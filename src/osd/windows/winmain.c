// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  winmain.c - Win32 main program
//
//============================================================

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <mmsystem.h>
#include <tchar.h>
#include <io.h>

// standard C headers
#include <ctype.h>
#include <stdarg.h>
#include <psapi.h>
#include <dbghelp.h>

// MAME headers
#include "emu.h"
#include "clifront.h"
#include "emuopts.h"

// MAMEOS headers
#include "winmain.h"
#include "window.h"
#include "video.h"
#include "input.h"
#include "output.h"
#include "config.h"
#include "osdepend.h"
#include "strconv.h"
#include "winutf8.h"
#include "winutil.h"
#include "debugger.h"
#include "winfile.h"

#define DEBUG_SLOW_LOCKS    0

//**************************************************************************
//  MACROS
//**************************************************************************

#ifdef UNICODE
#define UNICODE_POSTFIX "W"
#else
#define UNICODE_POSTFIX "A"
#endif



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

template<typename _FunctionPtr>
class dynamic_bind
{
public:
	// constructor which looks up the function
	dynamic_bind(const TCHAR *dll, const char *symbol)
		: m_function(NULL)
	{
		HMODULE module = LoadLibrary(dll);
		if (module != NULL)
			m_function = reinterpret_cast<_FunctionPtr>(GetProcAddress(module, symbol));
	}

	// bool to test if the function is NULL or not
	operator bool() const { return (m_function != NULL); }

	// dereference to get the underlying pointer
	_FunctionPtr operator *() const { return m_function; }

private:
	_FunctionPtr    m_function;
};


class stack_walker
{
public:
	stack_walker();

	FPTR ip() const { return m_stackframe.AddrPC.Offset; }
	FPTR sp() const { return m_stackframe.AddrStack.Offset; }
	FPTR frame() const { return m_stackframe.AddrFrame.Offset; }

	bool reset();
	void reset(CONTEXT &context, HANDLE thread);
	bool unwind();

private:
	HANDLE          m_process;
	HANDLE          m_thread;
	STACKFRAME64    m_stackframe;
	CONTEXT         m_context;
	bool            m_first;

	dynamic_bind<BOOL (WINAPI *)(DWORD, HANDLE, HANDLE, LPSTACKFRAME64, PVOID, PREAD_PROCESS_MEMORY_ROUTINE64, PFUNCTION_TABLE_ACCESS_ROUTINE64, PGET_MODULE_BASE_ROUTINE64, PTRANSLATE_ADDRESS_ROUTINE64)>
			m_stack_walk_64;
	dynamic_bind<BOOL (WINAPI *)(HANDLE, LPCTSTR, BOOL)> m_sym_initialize;
	dynamic_bind<PVOID (WINAPI *)(HANDLE, DWORD64)> m_sym_function_table_access_64;
	dynamic_bind<DWORD64 (WINAPI *)(HANDLE, DWORD64)> m_sym_get_module_base_64;
	dynamic_bind<VOID (WINAPI *)(PCONTEXT)> m_rtl_capture_context;

	static bool     s_initialized;
};


class symbol_manager
{
public:
	// construction/destruction
	symbol_manager(const char *argv0);
	~symbol_manager();

	// getters
	FPTR last_base() const { return m_last_base; }

	// core symbol lookup
	const char *symbol_for_address(FPTR address);
	const char *symbol_for_address(PVOID address) { return symbol_for_address(reinterpret_cast<FPTR>(address)); }

	// force symbols to be cached
	void cache_symbols() { scan_file_for_address(0, true); }

	void reset_cache() { m_cache.reset(); }
private:
	// internal helpers
	bool query_system_for_address(FPTR address);
	void scan_file_for_address(FPTR address, bool create_cache);
	bool parse_sym_line(const char *line, FPTR &address, std::string &symbol);
	bool parse_map_line(const char *line, FPTR &address, std::string &symbol);
	void scan_cache_for_address(FPTR address);
	void format_symbol(const char *name, UINT32 displacement, const char *filename = NULL, int linenumber = 0);

	static FPTR get_text_section_base();

	struct cache_entry
	{
		cache_entry(FPTR address, const char *symbol) :
			m_next(NULL), m_address(address), m_name(symbol) { }
		cache_entry *next() const { return m_next; }

		cache_entry *   m_next;
		FPTR            m_address;
		std::string     m_name;
	};
	simple_list<cache_entry> m_cache;

	std::string     m_mapfile;
	std::string     m_symfile;
	std::string     m_buffer;
	HANDLE          m_process;
	FPTR            m_last_base;
	FPTR            m_text_base;

	dynamic_bind<BOOL (WINAPI *)(HANDLE, DWORD64, PDWORD64, PSYMBOL_INFO)> m_sym_from_addr;
	dynamic_bind<BOOL (WINAPI *)(HANDLE, DWORD64, PDWORD, PIMAGEHLP_LINE64)> m_sym_get_line_from_addr_64;
};


class sampling_profiler
{
public:
	sampling_profiler(UINT32 max_seconds, UINT8 stack_depth);
	~sampling_profiler();

	void start();
	void stop();

//  void reset();
	void print_results(symbol_manager &symbols);

private:
	static DWORD WINAPI thread_entry(LPVOID lpParameter);
	void thread_run();

	static int CLIB_DECL compare_address(const void *item1, const void *item2);
	static int CLIB_DECL compare_frequency(const void *item1, const void *item2);

	HANDLE          m_target_thread;

	HANDLE          m_thread;
	DWORD           m_thread_id;
	volatile bool   m_thread_exit;

	UINT8           m_stack_depth;
	UINT8           m_entry_stride;
	std::vector<FPTR>    m_buffer;
	FPTR *          m_buffer_ptr;
	FPTR *          m_buffer_end;
};

//============================================================
//  winui_output_error
//============================================================

class winui_output_error : public osd_output
{
public:
	virtual void output_callback(osd_output_channel channel, const char *msg, va_list args)
	{
		if (channel == OSD_OUTPUT_CHANNEL_ERROR)
		{
			char buffer[1024];

			// if we are in fullscreen mode, go to windowed mode
			if ((video_config.windowed == 0) && (win_window_list != NULL))
				winwindow_toggle_full_screen();

			vsnprintf(buffer, ARRAY_LENGTH(buffer), msg, args);
			win_message_box_utf8(win_window_list ? win_window_list->m_hwnd : NULL, buffer, emulator_info::get_appname(), MB_OK);
		}
		else
			chain_output(channel, msg, args);
	}
};




//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// this line prevents globbing on the command line
int _CRT_glob = 0;

//**************************************************************************
//  LOCAL VARIABLES
//**************************************************************************

static LPTOP_LEVEL_EXCEPTION_FILTER pass_thru_filter;

static HANDLE watchdog_reset_event;
static HANDLE watchdog_exit_event;
static HANDLE watchdog_thread;

static running_machine *g_current_machine;

static int timeresult = !TIMERR_NOERROR;
static TIMECAPS timecaps;

static sampling_profiler *profiler = NULL;
static symbol_manager *symbols = NULL;

bool stack_walker::s_initialized = false;


//**************************************************************************
//  FUNCTION PROTOTYPES
//**************************************************************************

static BOOL WINAPI control_handler(DWORD type);
static int is_double_click_start(int argc);
static DWORD WINAPI watchdog_thread_entry(LPVOID lpParameter);
static LONG WINAPI exception_filter(struct _EXCEPTION_POINTERS *info);



//**************************************************************************
//  OPTIONS
//**************************************************************************

// struct definitions
const options_entry windows_options::s_option_entries[] =
{
	// performance options
	{ NULL,                                           NULL,       OPTION_HEADER,     "WINDOWS PERFORMANCE OPTIONS" },
	{ WINOPTION_PRIORITY "(-15-1)",                   "0",        OPTION_INTEGER,    "thread priority for the main game thread; range from -15 to 1" },
	{ WINOPTION_PROFILE,                              "0",        OPTION_INTEGER,    "enable profiling, specifying the stack depth to track" },

	// video options
	{ NULL,                                           NULL,       OPTION_HEADER,     "WINDOWS VIDEO OPTIONS" },
	{ WINOPTION_MENU,                                 "0",        OPTION_BOOLEAN,    "enable menu bar if available by UI implementation" },

	// DirectDraw-specific options
	{ NULL,                                           NULL,       OPTION_HEADER,     "DIRECTDRAW-SPECIFIC OPTIONS" },
	{ WINOPTION_HWSTRETCH ";hws",                     "1",        OPTION_BOOLEAN,    "enable hardware stretching" },

	// post-processing options
	{ NULL,                                                     NULL,        OPTION_HEADER,     "DIRECT3D POST-PROCESSING OPTIONS" },
	{ WINOPTION_HLSL_ENABLE";hlsl",                             "0",         OPTION_BOOLEAN,    "enable HLSL post-processing (PS3.0 required)" },
	{ WINOPTION_HLSLPATH,                                       "hlsl",      OPTION_STRING,     "path to hlsl files" },
	{ WINOPTION_HLSL_PRESCALE_X,                                "0",         OPTION_INTEGER,    "HLSL pre-scale override factor for X (0 for auto)" },
	{ WINOPTION_HLSL_PRESCALE_Y,                                "0",         OPTION_INTEGER,    "HLSL pre-scale override factor for Y (0 for auto)" },
	{ WINOPTION_HLSL_PRESET";(-1-3)",                           "-1",        OPTION_INTEGER,    "HLSL preset to use (0-3)" },
	{ WINOPTION_HLSL_WRITE,                                     NULL,        OPTION_STRING,     "enable HLSL AVI writing (huge disk bandwidth suggested)" },
	{ WINOPTION_HLSL_SNAP_WIDTH,                                "2048",      OPTION_STRING,     "HLSL upscaled-snapshot width" },
	{ WINOPTION_HLSL_SNAP_HEIGHT,                               "1536",      OPTION_STRING,     "HLSL upscaled-snapshot height" },
	{ WINOPTION_SHADOW_MASK_ALPHA";fs_shadwa(0.0-1.0)",         "0.0",       OPTION_FLOAT,      "shadow mask alpha-blend value (1.0 is fully blended, 0.0 is no mask)" },
	{ WINOPTION_SHADOW_MASK_TEXTURE";fs_shadwt(0.0-1.0)",       "aperture.png", OPTION_STRING,  "shadow mask texture name" },
	{ WINOPTION_SHADOW_MASK_COUNT_X";fs_shadww",                "6",         OPTION_INTEGER,    "shadow mask width, in phosphor dots" },
	{ WINOPTION_SHADOW_MASK_COUNT_Y";fs_shadwh",                "6",         OPTION_INTEGER,    "shadow mask height, in phosphor dots" },
	{ WINOPTION_SHADOW_MASK_USIZE";fs_shadwu(0.0-1.0)",         "0.1875",    OPTION_FLOAT,      "shadow mask texture size in U direction" },
	{ WINOPTION_SHADOW_MASK_VSIZE";fs_shadwv(0.0-1.0)",         "0.1875",    OPTION_FLOAT,      "shadow mask texture size in V direction" },
	{ WINOPTION_SHADOW_MASK_UOFFSET";fs_shadwou(-1.0-1.0)",     "0.0",       OPTION_FLOAT,      "shadow mask texture offset in U direction" },
	{ WINOPTION_SHADOW_MASK_VOFFSET";fs_shadwov(-1.0-1.0)",     "0.0",       OPTION_FLOAT,      "shadow mask texture offset in V direction" },
	{ WINOPTION_CURVATURE";fs_curv(0.0-1.0)",                   "0.0",       OPTION_FLOAT,      "screen curvature amount" },
	{ WINOPTION_ROUND_CORNER";fs_rndc(0.0-1.0)",                "0.0",       OPTION_FLOAT,      "screen round corner amount" },
	{ WINOPTION_SMOOTH_BORDER";fs_smob(0.0-1.0)",               "0.0",       OPTION_FLOAT,      "screen smooth border amount" },
	{ WINOPTION_REFLECTION";fs_ref(0.0-1.0)",                   "0.0",       OPTION_FLOAT,      "screen reflection amount" },
	{ WINOPTION_VIGNETTING";fs_vig(0.0-1.0)",                   "0.0",       OPTION_FLOAT,      "image vignetting amount" },
	/* Beam-related values below this line*/
	{ WINOPTION_SCANLINE_AMOUNT";fs_scanam(0.0-4.0)",           "1.0",       OPTION_FLOAT,      "overall alpha scaling value for scanlines" },
	{ WINOPTION_SCANLINE_SCALE";fs_scansc(0.0-4.0)",            "1.0",       OPTION_FLOAT,      "overall height scaling value for scanlines" },
	{ WINOPTION_SCANLINE_HEIGHT";fs_scanh(0.0-4.0)",            "1.0",       OPTION_FLOAT,      "individual height scaling value for scanlines" },
	{ WINOPTION_SCANLINE_BRIGHT_SCALE";fs_scanbs(0.0-2.0)",     "1.0",       OPTION_FLOAT,      "overall brightness scaling value for scanlines (multiplicative)" },
	{ WINOPTION_SCANLINE_BRIGHT_OFFSET";fs_scanbo(0.0-1.0)",    "0.0",       OPTION_FLOAT,      "overall brightness offset value for scanlines (additive)" },
	{ WINOPTION_SCANLINE_OFFSET";fs_scanjt(0.0-4.0)",           "0.0",       OPTION_FLOAT,      "overall interlace jitter scaling value for scanlines" },
	{ WINOPTION_DEFOCUS";fs_focus",                             "0.0,0.0",   OPTION_STRING,     "overall defocus value in screen-relative coords" },
	{ WINOPTION_CONVERGE_X";fs_convx",                          "0.3,0.0,-0.3",OPTION_STRING,   "convergence in screen-relative X direction" },
	{ WINOPTION_CONVERGE_Y";fs_convy",                          "0.0,0.3,-0.3",OPTION_STRING,   "convergence in screen-relative Y direction" },
	{ WINOPTION_RADIAL_CONVERGE_X";fs_rconvx",                  "0.0,0.0,0.0",OPTION_STRING,    "radial convergence in screen-relative X direction" },
	{ WINOPTION_RADIAL_CONVERGE_Y";fs_rconvy",                  "0.0,0.0,0.0",OPTION_STRING,    "radial convergence in screen-relative Y direction" },
	/* RGB colorspace convolution below this line */
	{ WINOPTION_RED_RATIO";fs_redratio",                        "1.0,0.0,0.0",OPTION_STRING,    "red output signal generated by input signal" },
	{ WINOPTION_GRN_RATIO";fs_grnratio",                        "0.0,1.0,0.0",OPTION_STRING,    "green output signal generated by input signal" },
	{ WINOPTION_BLU_RATIO";fs_bluratio",                        "0.0,0.0,1.0",OPTION_STRING,    "blue output signal generated by input signal" },
	{ WINOPTION_SATURATION";fs_sat(0.0-4.0)",                   "1.4",        OPTION_FLOAT,     "saturation scaling value" },
	{ WINOPTION_OFFSET";fs_offset",                             "0.0,0.0,0.0",OPTION_STRING,    "signal offset value (additive)" },
	{ WINOPTION_SCALE";fs_scale",                               "0.95,0.95,0.95",OPTION_STRING, "signal scaling value (multiplicative)" },
	{ WINOPTION_POWER";fs_power",                               "0.8,0.8,0.8",OPTION_STRING,    "signal power value (exponential)" },
	{ WINOPTION_FLOOR";fs_floor",                               "0.05,0.05,0.05",OPTION_STRING, "signal floor level" },
	{ WINOPTION_PHOSPHOR";fs_phosphor",                         "0.4,0.4,0.4",OPTION_STRING,    "phosphorescence decay rate (0.0 is instant, 1.0 is forever)" },
	/* NTSC simulation below this line */
	{ NULL,                                                     NULL,        OPTION_HEADER,     "NTSC POST-PROCESSING OPTIONS" },
	{ WINOPTION_YIQ_ENABLE";yiq",                               "0",         OPTION_BOOLEAN,    "enable YIQ-space HLSL post-processing" },
	{ WINOPTION_YIQ_CCVALUE";yiqcc",                            "3.59754545",OPTION_FLOAT,      "Color Carrier frequency for NTSC signal processing" },
	{ WINOPTION_YIQ_AVALUE";yiqa",                              "0.5",       OPTION_FLOAT,      "A value for NTSC signal processing" },
	{ WINOPTION_YIQ_BVALUE";yiqb",                              "0.5",       OPTION_FLOAT,      "B value for NTSC signal processing" },
	{ WINOPTION_YIQ_OVALUE";yiqo",                              "1.570796325",OPTION_FLOAT,     "Outgoing Color Carrier phase offset for NTSC signal processing" },
	{ WINOPTION_YIQ_PVALUE";yiqp",                              "1.0",       OPTION_FLOAT,      "Incoming Pixel Clock scaling value for NTSC signal processing" },
	{ WINOPTION_YIQ_NVALUE";yiqn",                              "1.0",       OPTION_FLOAT,      "Y filter notch width for NTSC signal processing" },
	{ WINOPTION_YIQ_YVALUE";yiqy",                              "6.0",       OPTION_FLOAT,      "Y filter cutoff frequency for NTSC signal processing" },
	{ WINOPTION_YIQ_IVALUE";yiqi",                              "1.2",       OPTION_FLOAT,      "I filter cutoff frequency for NTSC signal processing" },
	{ WINOPTION_YIQ_QVALUE";yiqq",                              "0.6",       OPTION_FLOAT,      "Q filter cutoff frequency for NTSC signal processing" },
	{ WINOPTION_YIQ_SCAN_TIME";yiqsc",                          "52.6",      OPTION_FLOAT,      "Horizontal scanline duration for NTSC signal processing (in usec)" },
	{ WINOPTION_YIQ_PHASE_COUNT";yiqp",                         "2",         OPTION_INTEGER,    "Phase Count value for NTSC signal processing" },
	{ WINOPTION_YIQ_SCAN_TIME";yiqsc",                          "52.6",      OPTION_FLOAT,      "Horizontal scanline duration for NTSC signal processing (in usec)" },
	{ WINOPTION_YIQ_PHASE_COUNT";yiqp",                         "2",         OPTION_INTEGER,    "Phase Count value for NTSC signal processing" },
	/* Vector simulation below this line */
	{ NULL,                                                     NULL,        OPTION_HEADER,     "VECTOR POST-PROCESSING OPTIONS" },
	{ WINOPTION_VECTOR_LENGTH_SCALE";veclength",                "0.8",       OPTION_FLOAT,      "How much length affects vector fade" },
	{ WINOPTION_VECTOR_LENGTH_RATIO";vecsize",                  "500.0",     OPTION_FLOAT,      "Vector fade length (4.0 - vectors fade the most at and above 4 pixels, etc.)" },
	/* Bloom below this line */
	{ NULL,                                                     NULL,        OPTION_HEADER,     "BLOOM POST-PROCESSING OPTIONS" },
	{ WINOPTION_BLOOM_SCALE,                                    "0.25",      OPTION_FLOAT,      "Intensity factor for bloom" },
	{ WINOPTION_BLOOM_OVERDRIVE,                                "0.0,0.0,0.0",OPTION_STRING,    "Overdrive factor for bloom" },
	{ WINOPTION_BLOOM_LEVEL0_WEIGHT,                            "1.0",       OPTION_FLOAT,      "Bloom level 0  (full-size target) weight" },
	{ WINOPTION_BLOOM_LEVEL1_WEIGHT,                            "0.21",      OPTION_FLOAT,      "Bloom level 1  (half-size target) weight" },
	{ WINOPTION_BLOOM_LEVEL2_WEIGHT,                            "0.19",      OPTION_FLOAT,      "Bloom level 2  (quarter-size target) weight" },
	{ WINOPTION_BLOOM_LEVEL3_WEIGHT,                            "0.17",      OPTION_FLOAT,      "Bloom level 3  (.) weight" },
	{ WINOPTION_BLOOM_LEVEL4_WEIGHT,                            "0.15",      OPTION_FLOAT,      "Bloom level 4  (.) weight" },
	{ WINOPTION_BLOOM_LEVEL5_WEIGHT,                            "0.14",      OPTION_FLOAT,      "Bloom level 5  (.) weight" },
	{ WINOPTION_BLOOM_LEVEL6_WEIGHT,                            "0.13",      OPTION_FLOAT,      "Bloom level 6  (.) weight" },
	{ WINOPTION_BLOOM_LEVEL7_WEIGHT,                            "0.12",      OPTION_FLOAT,      "Bloom level 7  (.) weight" },
	{ WINOPTION_BLOOM_LEVEL8_WEIGHT,                            "0.11",      OPTION_FLOAT,      "Bloom level 8  (.) weight" },
	{ WINOPTION_BLOOM_LEVEL9_WEIGHT,                            "0.10",      OPTION_FLOAT,      "Bloom level 9  (.) weight" },
	{ WINOPTION_BLOOM_LEVEL10_WEIGHT,                           "0.09",      OPTION_FLOAT,      "Bloom level 10 (1x1 target) weight" },

	// full screen options
	{ NULL,                                           NULL,       OPTION_HEADER,     "FULL SCREEN OPTIONS" },
	{ WINOPTION_TRIPLEBUFFER ";tb",                   "0",        OPTION_BOOLEAN,    "enable triple buffering" },
	{ WINOPTION_FULLSCREENBRIGHTNESS ";fsb(0.1-2.0)", "1.0",      OPTION_FLOAT,      "brightness value in full screen mode" },
	{ WINOPTION_FULLSCREENCONTRAST ";fsc(0.1-2.0)",   "1.0",      OPTION_FLOAT,      "contrast value in full screen mode" },
	{ WINOPTION_FULLSCREENGAMMA ";fsg(0.1-3.0)",      "1.0",      OPTION_FLOAT,      "gamma value in full screen mode" },

	// input options
	{ NULL,                                           NULL,       OPTION_HEADER,     "INPUT DEVICE OPTIONS" },
	{ WINOPTION_GLOBAL_INPUTS ";global_inputs",       "0",        OPTION_BOOLEAN,    "enable global inputs" },
	{ WINOPTION_DUAL_LIGHTGUN ";dual",                "0",        OPTION_BOOLEAN,    "enable dual lightgun input" },

	{ NULL }
};

//**************************************************************************
//  MAIN ENTRY POINT
//**************************************************************************


//============================================================
//  utf8_main
//============================================================

int main(int argc, char *argv[])
{
	// use small output buffers on non-TTYs (i.e. pipes)
	if (!isatty(fileno(stdout)))
		setvbuf(stdout, (char *) NULL, _IOFBF, 64);
	if (!isatty(fileno(stderr)))
		setvbuf(stderr, (char *) NULL, _IOFBF, 64);

	// initialize common controls
	InitCommonControls();

	// set a handler to catch ctrl-c
	SetConsoleCtrlHandler(control_handler, TRUE);

	// allocate symbols
	symbol_manager local_symbols(argv[0]);
	symbols = &local_symbols;

	// set up exception handling
	pass_thru_filter = SetUnhandledExceptionFilter(exception_filter);
	SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);

	// enable stack crawls for asserts
	extern void (*s_debugger_stack_crawler)();
	s_debugger_stack_crawler = winmain_dump_stack;


	// parse config and cmdline options
	DWORD result = 0;
	{
		windows_options options;
		windows_osd_interface osd(options);
		// if we're a GUI app, out errors to message boxes
		// Initialize this after the osd interface so that we are first in the
		// output order
		winui_output_error winerror;
		if (win_is_gui_application() || is_double_click_start(argc))
		{
			// if we are a GUI app, output errors to message boxes
			osd_output::push(&winerror);
			// make sure any console window that opened on our behalf is nuked
			FreeConsole();
		}
		osd.register_options();
		cli_frontend frontend(options, osd);
		result = frontend.execute(argc, argv);
		osd_output::pop(&winerror);
	}
	// free symbols
	symbols = NULL;
	return result;
}


//============================================================
//  windows_options
//============================================================

windows_options::windows_options()
: osd_options()
{
	add_entries(s_option_entries);
}


//============================================================
//  control_handler
//============================================================

static BOOL WINAPI control_handler(DWORD type)
{
	// indicate to the user that we detected something
	switch (type)
	{
		case CTRL_C_EVENT:          fprintf(stderr, "Caught Ctrl+C");                   break;
		case CTRL_BREAK_EVENT:      fprintf(stderr, "Caught Ctrl+break");               break;
		case CTRL_CLOSE_EVENT:      fprintf(stderr, "Caught console close");            break;
		case CTRL_LOGOFF_EVENT:     fprintf(stderr, "Caught logoff");                   break;
		case CTRL_SHUTDOWN_EVENT:   fprintf(stderr, "Caught shutdown");                 break;
		default:                    fprintf(stderr, "Caught unexpected console event"); break;
	}

	// if we don't have a machine yet, or if we are handling ctrl+c/ctrl+break,
	// just terminate hard, without throwing or handling any atexit stuff
	if (g_current_machine == NULL || type == CTRL_C_EVENT || type == CTRL_BREAK_EVENT)
	{
		fprintf(stderr, ", exiting\n");
		TerminateProcess(GetCurrentProcess(), MAMERR_FATALERROR);
	}

	// all other situations attempt to do a clean exit
	else
	{
		fprintf(stderr, ", exit requested\n");
		g_current_machine->schedule_exit();
	}

	// in all cases we handled it
	return TRUE;
}




//============================================================
//  output_oslog
//============================================================

static void output_oslog(running_machine &machine, const char *buffer)
{
	if (IsDebuggerPresent())
		win_output_debug_string_utf8(buffer);
}


//============================================================
//  constructor
//============================================================

windows_osd_interface::windows_osd_interface(windows_options &options)
: osd_common_t(options), m_options(options)
{
}


//============================================================
//  destructor
//============================================================

windows_osd_interface::~windows_osd_interface()
{
}


//============================================================
//  video_register
//============================================================

void windows_osd_interface::video_register()
{
	video_options_add("gdi", NULL);
	video_options_add("ddraw", NULL);
	video_options_add("d3d", NULL);
	video_options_add("bgfx", NULL);
	//video_options_add("auto", NULL); // making d3d video default one
}

//============================================================
//  init
//============================================================

void windows_osd_interface::init(running_machine &machine)
{
	// call our parent
	osd_common_t::init(machine);

	const char *stemp;
	windows_options &options = downcast<windows_options &>(machine.options());

	// determine if we are benchmarking, and adjust options appropriately
	int bench = options.bench();
	std::string error_string;
	if (bench > 0)
	{
		options.set_value(OPTION_THROTTLE, false, OPTION_PRIORITY_MAXIMUM, error_string);
		options.set_value(OSDOPTION_SOUND, "none", OPTION_PRIORITY_MAXIMUM, error_string);
		options.set_value(OSDOPTION_VIDEO, "none", OPTION_PRIORITY_MAXIMUM, error_string);
		options.set_value(OPTION_SECONDS_TO_RUN, bench, OPTION_PRIORITY_MAXIMUM, error_string);
		assert(error_string.empty());
	}

	// determine if we are profiling, and adjust options appropriately
	int profile = options.profile();
	if (profile > 0)
	{
		options.set_value(OPTION_THROTTLE, false, OPTION_PRIORITY_MAXIMUM, error_string);
		options.set_value(OSDOPTION_MULTITHREADING, false, OPTION_PRIORITY_MAXIMUM, error_string);
		options.set_value(OSDOPTION_NUMPROCESSORS, 1, OPTION_PRIORITY_MAXIMUM, error_string);
		assert(error_string.empty());
	}

	// thread priority
	if (!(machine.debug_flags & DEBUG_FLAG_OSD_ENABLED))
		SetThreadPriority(GetCurrentThread(), options.priority());

	// get number of processors
	stemp = options.numprocessors();

	osd_num_processors = 0;

	if (strcmp(stemp, "auto") != 0)
	{
		osd_num_processors = atoi(stemp);
		if (osd_num_processors < 1)
		{
			osd_printf_warning("Warning: numprocessors < 1 doesn't make much sense. Assuming auto ...\n");
			osd_num_processors = 0;
		}
	}

	// initialize the subsystems
	osd_common_t::init_subsystems();

	// notify listeners of screen configuration
	std::string tempstring;
	for (win_window_info *info = win_window_list; info != NULL; info = info->m_next)
	{
		strprintf(tempstring, "Orientation(%s)", info->m_monitor->devicename());
		output_set_value(tempstring.c_str(), info->m_targetorient);
	}

	// hook up the debugger log
	if (options.oslog())
		machine.add_logerror_callback(output_oslog);

	// crank up the multimedia timer resolution to its max
	// this gives the system much finer timeslices
	timeresult = timeGetDevCaps(&timecaps, sizeof(timecaps));
	if (timeresult == TIMERR_NOERROR)
		timeBeginPeriod(timecaps.wPeriodMin);

	// if a watchdog thread is requested, create one
	int watchdog = options.watchdog();
	if (watchdog != 0)
	{
		watchdog_reset_event = CreateEvent(NULL, FALSE, FALSE, NULL);
		assert_always(watchdog_reset_event != NULL, "Failed to create watchdog reset event");
		watchdog_exit_event = CreateEvent(NULL, TRUE, FALSE, NULL);
		assert_always(watchdog_exit_event != NULL, "Failed to create watchdog exit event");
		watchdog_thread = CreateThread(NULL, 0, watchdog_thread_entry, (LPVOID)(FPTR)watchdog, 0, NULL);
		assert_always(watchdog_thread != NULL, "Failed to create watchdog thread");
	}

	// create and start the profiler
	if (profile > 0)
	{
		profiler = global_alloc(sampling_profiler(1000, profile - 1));
		profiler->start();
	}

	// initialize sockets
	win_init_sockets();

	// note the existence of a machine
	g_current_machine = &machine;
}


//============================================================
//  osd_exit
//============================================================

void windows_osd_interface::osd_exit()
{
	// no longer have a machine
	g_current_machine = NULL;

	// cleanup sockets
	win_cleanup_sockets();

	osd_common_t::osd_exit();

	// take down the watchdog thread if it exists
	if (watchdog_thread != NULL)
	{
		SetEvent(watchdog_exit_event);
		WaitForSingleObject(watchdog_thread, INFINITE);
		CloseHandle(watchdog_reset_event);
		CloseHandle(watchdog_exit_event);
		CloseHandle(watchdog_thread);
		watchdog_reset_event = NULL;
		watchdog_exit_event = NULL;
		watchdog_thread = NULL;
	}

	// stop the profiler
	if (profiler != NULL)
	{
		profiler->stop();
		profiler->print_results(*symbols);
		global_free(profiler);
	}

	// restore the timer resolution
	if (timeresult == TIMERR_NOERROR)
		timeEndPeriod(timecaps.wPeriodMin);

	// one last pass at events
	winwindow_process_events(machine(), 0, 0);
}

//============================================================
//  winmain_dump_stack
//============================================================

void winmain_dump_stack()
{
	// set up the stack walker
	stack_walker walker;
	if (!walker.reset())
		return;

	// walk the stack
	while (walker.unwind())
		fprintf(stderr, "  %p: %p%s\n", (void *)walker.frame(), (void *)walker.ip(), (symbols == NULL) ? "" : symbols->symbol_for_address(walker.ip()));
}


//============================================================
//  check_for_double_click_start
//============================================================

static int is_double_click_start(int argc)
{
	STARTUPINFO startup_info = { sizeof(STARTUPINFO) };

	// determine our startup information
	GetStartupInfo(&startup_info);

	// try to determine if MAME was simply double-clicked
	return (argc <= 1 && startup_info.dwFlags && !(startup_info.dwFlags & STARTF_USESTDHANDLES));
}


//============================================================
//  watchdog_thread_entry
//============================================================

static DWORD WINAPI watchdog_thread_entry(LPVOID lpParameter)
{
	DWORD timeout = (int)(FPTR)lpParameter * 1000;

	while (TRUE)
	{
		HANDLE handle_list[2];
		DWORD wait_result;

		// wait for either a reset or an exit, or a timeout
		handle_list[0] = watchdog_reset_event;
		handle_list[1] = watchdog_exit_event;
		wait_result = WaitForMultipleObjects(2, handle_list, FALSE, timeout);

		// on a reset, just loop around and re-wait
		if (wait_result == WAIT_OBJECT_0 + 0)
			continue;

		// on an exit, break out
		if (wait_result == WAIT_OBJECT_0 + 1)
			break;

		// on a timeout, kill the process
		if (wait_result == WAIT_TIMEOUT)
		{
			fprintf(stderr, "Terminating due to watchdog timeout\n");
			fflush(stderr);
			TerminateProcess(GetCurrentProcess(), -1);
		}
	}
	return EXCEPTION_CONTINUE_SEARCH;
}


//============================================================
//  winmain_watchdog_ping
//============================================================

void winmain_watchdog_ping(void)
{
	// if we have a watchdog, reset it
	if (watchdog_reset_event != NULL)
		SetEvent(watchdog_reset_event);
}


//============================================================
//  exception_filter
//============================================================

static LONG WINAPI exception_filter(struct _EXCEPTION_POINTERS *info)
{
	static const struct
	{
		DWORD code;
		const char *string;
	} exception_table[] =
	{
		{ EXCEPTION_ACCESS_VIOLATION,       "ACCESS VIOLATION" },
		{ EXCEPTION_DATATYPE_MISALIGNMENT,  "DATATYPE MISALIGNMENT" },
		{ EXCEPTION_BREAKPOINT,             "BREAKPOINT" },
		{ EXCEPTION_SINGLE_STEP,            "SINGLE STEP" },
		{ EXCEPTION_ARRAY_BOUNDS_EXCEEDED,  "ARRAY BOUNDS EXCEEDED" },
		{ EXCEPTION_FLT_DENORMAL_OPERAND,   "FLOAT DENORMAL OPERAND" },
		{ EXCEPTION_FLT_DIVIDE_BY_ZERO,     "FLOAT DIVIDE BY ZERO" },
		{ EXCEPTION_FLT_INEXACT_RESULT,     "FLOAT INEXACT RESULT" },
		{ EXCEPTION_FLT_INVALID_OPERATION,  "FLOAT INVALID OPERATION" },
		{ EXCEPTION_FLT_OVERFLOW,           "FLOAT OVERFLOW" },
		{ EXCEPTION_FLT_STACK_CHECK,        "FLOAT STACK CHECK" },
		{ EXCEPTION_FLT_UNDERFLOW,          "FLOAT UNDERFLOW" },
		{ EXCEPTION_INT_DIVIDE_BY_ZERO,     "INTEGER DIVIDE BY ZERO" },
		{ EXCEPTION_INT_OVERFLOW,           "INTEGER OVERFLOW" },
		{ EXCEPTION_PRIV_INSTRUCTION,       "PRIVILEGED INSTRUCTION" },
		{ EXCEPTION_IN_PAGE_ERROR,          "IN PAGE ERROR" },
		{ EXCEPTION_ILLEGAL_INSTRUCTION,    "ILLEGAL INSTRUCTION" },
		{ EXCEPTION_NONCONTINUABLE_EXCEPTION,"NONCONTINUABLE EXCEPTION" },
		{ EXCEPTION_STACK_OVERFLOW,         "STACK OVERFLOW" },
		{ EXCEPTION_INVALID_DISPOSITION,    "INVALID DISPOSITION" },
		{ EXCEPTION_GUARD_PAGE,             "GUARD PAGE VIOLATION" },
		{ EXCEPTION_INVALID_HANDLE,         "INVALID HANDLE" },
		{ 0,                                "UNKNOWN EXCEPTION" }
	};
	static int already_hit = 0;
	int i;

	// if we're hitting this recursively, just exit
	if (already_hit)
		return EXCEPTION_CONTINUE_SEARCH;
	already_hit = 1;

	// flush any debugging traces that were live
	debugger_flush_all_traces_on_abnormal_exit();

	// find our man
	for (i = 0; exception_table[i].code != 0; i++)
		if (info->ExceptionRecord->ExceptionCode == exception_table[i].code)
			break;

	// print the exception type and address
	fprintf(stderr, "\n-----------------------------------------------------\n");
	fprintf(stderr, "Exception at EIP=%p%s: %s\n", info->ExceptionRecord->ExceptionAddress,
			symbols->symbol_for_address((FPTR)info->ExceptionRecord->ExceptionAddress), exception_table[i].string);

	// for access violations, print more info
	if (info->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
		fprintf(stderr, "While attempting to %s memory at %p\n",
				info->ExceptionRecord->ExceptionInformation[0] ? "write" : "read",
				(void *)info->ExceptionRecord->ExceptionInformation[1]);

	// print the state of the CPU
	fprintf(stderr, "-----------------------------------------------------\n");
#ifdef PTR64
	fprintf(stderr, "RAX=%p RBX=%p RCX=%p RDX=%p\n",
			(void *)info->ContextRecord->Rax,
			(void *)info->ContextRecord->Rbx,
			(void *)info->ContextRecord->Rcx,
			(void *)info->ContextRecord->Rdx);
	fprintf(stderr, "RSI=%p RDI=%p RBP=%p RSP=%p\n",
			(void *)info->ContextRecord->Rsi,
			(void *)info->ContextRecord->Rdi,
			(void *)info->ContextRecord->Rbp,
			(void *)info->ContextRecord->Rsp);
	fprintf(stderr, " R8=%p  R9=%p R10=%p R11=%p\n",
			(void *)info->ContextRecord->R8,
			(void *)info->ContextRecord->R9,
			(void *)info->ContextRecord->R10,
			(void *)info->ContextRecord->R11);
	fprintf(stderr, "R12=%p R13=%p R14=%p R15=%p\n",
			(void *)info->ContextRecord->R12,
			(void *)info->ContextRecord->R13,
			(void *)info->ContextRecord->R14,
			(void *)info->ContextRecord->R15);
#else
	fprintf(stderr, "EAX=%p EBX=%p ECX=%p EDX=%p\n",
			(void *)info->ContextRecord->Eax,
			(void *)info->ContextRecord->Ebx,
			(void *)info->ContextRecord->Ecx,
			(void *)info->ContextRecord->Edx);
	fprintf(stderr, "ESI=%p EDI=%p EBP=%p ESP=%p\n",
			(void *)info->ContextRecord->Esi,
			(void *)info->ContextRecord->Edi,
			(void *)info->ContextRecord->Ebp,
			(void *)info->ContextRecord->Esp);
#endif

	stack_walker walker;
	walker.reset(*info->ContextRecord, GetCurrentThread());

	// reprint the actual exception address
	fprintf(stderr, "-----------------------------------------------------\n");
	fprintf(stderr, "Stack crawl:\n");

	// walk the stack
	while (walker.unwind())
		fprintf(stderr, "  %p: %p%s\n", (void *)walker.frame(), (void *)walker.ip(), (symbols == NULL) ? "" : symbols->symbol_for_address(walker.ip()));

	// flush stderr, so the data is actually written when output is being redirected
	fflush(stderr);

	// exit
	return EXCEPTION_CONTINUE_SEARCH;
}


//**************************************************************************
//  STACK WALKER
//**************************************************************************

//-------------------------------------------------
//  stack_walker - constructor
//-------------------------------------------------

stack_walker::stack_walker()
	: m_process(GetCurrentProcess()),
		m_thread(GetCurrentThread()),
		m_first(true),
		m_stack_walk_64(TEXT("dbghelp.dll"), "StackWalk64"),
		m_sym_initialize(TEXT("dbghelp.dll"), "SymInitialize"),
		m_sym_function_table_access_64(TEXT("dbghelp.dll"), "SymFunctionTableAccess64"),
		m_sym_get_module_base_64(TEXT("dbghelp.dll"), "SymGetModuleBase64"),
		m_rtl_capture_context(TEXT("kernel32.dll"), "RtlCaptureContext")
{
	// zap the structs
	memset(&m_stackframe, 0, sizeof(m_stackframe));
	memset(&m_context, 0, sizeof(m_context));

	// initialize the symbols
	if (!s_initialized && m_sym_initialize && m_stack_walk_64 && m_sym_function_table_access_64 && m_sym_get_module_base_64)
	{
		(*m_sym_initialize)(m_process, NULL, TRUE);
		s_initialized = true;
	}
}


//-------------------------------------------------
//  reset - set up a new context
//-------------------------------------------------

bool stack_walker::reset()
{
	// set up the initial state
	if (!m_rtl_capture_context)
		return false;
	(*m_rtl_capture_context)(&m_context);
	m_thread = GetCurrentThread();
	m_first = true;

	// initialize the stackframe
	memset(&m_stackframe, 0, sizeof(m_stackframe));
	m_stackframe.AddrPC.Mode = AddrModeFlat;
	m_stackframe.AddrFrame.Mode = AddrModeFlat;
	m_stackframe.AddrStack.Mode = AddrModeFlat;

	// pull architecture-specific fields from the context
#ifdef PTR64
	m_stackframe.AddrPC.Offset = m_context.Rip;
	m_stackframe.AddrFrame.Offset = m_context.Rsp;
	m_stackframe.AddrStack.Offset = m_context.Rsp;
#else
	m_stackframe.AddrPC.Offset = m_context.Eip;
	m_stackframe.AddrFrame.Offset = m_context.Ebp;
	m_stackframe.AddrStack.Offset = m_context.Esp;
#endif
	return true;
}

void stack_walker::reset(CONTEXT &initial, HANDLE thread)
{
	// set up the initial state
	m_context = initial;
	m_thread = thread;
	m_first = true;

	// initialize the stackframe
	memset(&m_stackframe, 0, sizeof(m_stackframe));
	m_stackframe.AddrPC.Mode = AddrModeFlat;
	m_stackframe.AddrFrame.Mode = AddrModeFlat;
	m_stackframe.AddrStack.Mode = AddrModeFlat;

	// pull architecture-specific fields from the context
#ifdef PTR64
	m_stackframe.AddrPC.Offset = m_context.Rip;
	m_stackframe.AddrFrame.Offset = m_context.Rsp;
	m_stackframe.AddrStack.Offset = m_context.Rsp;
#else
	m_stackframe.AddrPC.Offset = m_context.Eip;
	m_stackframe.AddrFrame.Offset = m_context.Ebp;
	m_stackframe.AddrStack.Offset = m_context.Esp;
#endif
}


//-------------------------------------------------
//  unwind - unwind a single level
//-------------------------------------------------

bool stack_walker::unwind()
{
	// if we were able to initialize, then we have everything we need
	if (s_initialized)
	{
#ifdef PTR64
		return (*m_stack_walk_64)(IMAGE_FILE_MACHINE_AMD64, m_process, m_thread, &m_stackframe, &m_context, NULL, *m_sym_function_table_access_64, *m_sym_get_module_base_64, NULL);
#else
		return (*m_stack_walk_64)(IMAGE_FILE_MACHINE_I386, m_process, m_thread, &m_stackframe, &m_context, NULL, *m_sym_function_table_access_64, *m_sym_get_module_base_64, NULL);
#endif
	}

	// otherwise, fake the first unwind, which will just return info from the context
	else
	{
		bool result = m_first;
		m_first = false;
		return result;
	}
}



//**************************************************************************
//  SYMBOL MANAGER
//**************************************************************************

//-------------------------------------------------
//  symbol_manager - constructor
//-------------------------------------------------

symbol_manager::symbol_manager(const char *argv0)
	: m_mapfile(argv0),
		m_symfile(argv0),
		m_process(GetCurrentProcess()),
		m_last_base(0),
		m_text_base(0),
		m_sym_from_addr(TEXT("dbghelp.dll"), "SymFromAddr"),
		m_sym_get_line_from_addr_64(TEXT("dbghelp.dll"), "SymGetLineFromAddr64")
{
#ifdef __GNUC__
	// compute the name of the mapfile
	int extoffs = m_mapfile.find_last_of('.');
	if (extoffs != -1)
		m_mapfile.substr(0, extoffs);
	m_mapfile.append(".map");

	// and the name of the symfile
	extoffs = m_symfile.find_last_of('.');
	if (extoffs != -1)
		m_symfile = m_symfile.substr(0, extoffs);
	m_symfile.append(".sym");

	// figure out the base of the .text section
	m_text_base = get_text_section_base();
#endif

	// expand the buffer to be decently large up front
	strprintf(m_buffer,"%500s", "");
}


//-------------------------------------------------
//  ~symbol_manager - destructor
//-------------------------------------------------

symbol_manager::~symbol_manager()
{
}


//-------------------------------------------------
//  symbol_for_address - return a symbol by looking
//  it up either in the cache or by scanning the
//  file
//-------------------------------------------------

const char *symbol_manager::symbol_for_address(FPTR address)
{
	// default the buffer
	m_buffer.assign(" (not found)");
	m_last_base = 0;

	// first try to do it using system APIs
	if (!query_system_for_address(address))
	{
		// if that fails, scan the cache if we have one
		if (m_cache.first() != NULL)
			scan_cache_for_address(address);

		// or else try to open a sym/map file and find it there
		else
			scan_file_for_address(address, false);
	}
	return m_buffer.c_str();
}


//-------------------------------------------------
//  query_system_for_address - ask the system to
//  look up our address
//-------------------------------------------------

bool symbol_manager::query_system_for_address(FPTR address)
{
	// need at least the sym_from_addr API
	if (!m_sym_from_addr)
		return false;

	BYTE info_buffer[sizeof(SYMBOL_INFO) + 256] = { 0 };
	SYMBOL_INFO &info = *reinterpret_cast<SYMBOL_INFO *>(&info_buffer[0]);
	DWORD64 displacement;

	// even through the struct says TCHAR, we actually get back an ANSI string here
	info.SizeOfStruct = sizeof(info);
	info.MaxNameLen = sizeof(info_buffer) - sizeof(info);
	if ((*m_sym_from_addr)(m_process, address, &displacement, &info))
	{
		// try to get source info as well; again we are returned an ANSI string
		IMAGEHLP_LINE64 lineinfo = { sizeof(lineinfo) };
		DWORD linedisp;
		if (m_sym_get_line_from_addr_64 && (*m_sym_get_line_from_addr_64)(m_process, address, &linedisp, &lineinfo))
			format_symbol(info.Name, displacement, lineinfo.FileName, lineinfo.LineNumber);
		else
			format_symbol(info.Name, displacement);

		// set the last base
		m_last_base = address - displacement;
		return true;
	}
	return false;
}


//-------------------------------------------------
//  scan_file_for_address - walk either the map
//  or symbol files and find the best match for
//  the given address, optionally creating a cache
//  along the way
//-------------------------------------------------

void symbol_manager::scan_file_for_address(FPTR address, bool create_cache)
{
	bool is_symfile = false;
	FILE *srcfile = NULL;

#ifdef __GNUC__
	// see if we have a symbol file (gcc only)
	srcfile = fopen(m_symfile.c_str(), "r");
	is_symfile = (srcfile != NULL);
#endif

	// if not, see if we have a map file
	if (srcfile == NULL)
		srcfile = fopen(m_mapfile.c_str(), "r");

	// if not, fail
	if (srcfile == NULL)
		return;

	// reset the best info
	std::string best_symbol;
	FPTR best_addr = 0;

	// parse the file, looking for valid entries
	std::string symbol;
	char line[1024];
	while (fgets(line, sizeof(line) - 1, srcfile))
	{
		// parse the line looking for an interesting symbol
		FPTR addr = 0;
		bool valid = is_symfile ? parse_sym_line(line, addr, symbol) : parse_map_line(line, addr, symbol);

		// if we got one, see if this is the best
		if (valid)
		{
			// if this is the best one so far, remember it
			if (addr <= address && addr > best_addr)
			{
				best_addr = addr;
				best_symbol = symbol;
			}

			// also create a cache entry if we can
			if (create_cache)
				m_cache.append(*global_alloc(cache_entry(addr, symbol.c_str())));
		}
	}

	// close the file
	fclose(srcfile);

	// format the symbol and remember the last base
	format_symbol(best_symbol.c_str(), address - best_addr);
	m_last_base = best_addr;
}


//-------------------------------------------------
//  scan_cache_for_address - walk the cache to
//  find the best match for the given address
//-------------------------------------------------

void symbol_manager::scan_cache_for_address(FPTR address)
{
	// reset the best info
	std::string best_symbol;
	FPTR best_addr = 0;

	// walk the cache, looking for valid entries
	for (cache_entry *entry = m_cache.first(); entry != NULL; entry = entry->next())

		// if this is the best one so far, remember it
		if (entry->m_address <= address && entry->m_address > best_addr)
		{
			best_addr = entry->m_address;
			best_symbol = entry->m_name;
		}

	// format the symbol and remember the last base
	format_symbol(best_symbol.c_str(), address - best_addr);
	m_last_base = best_addr;
}


//-------------------------------------------------
//  parse_sym_line - parse a line from a sym file
//  which is just the output of objdump
//-------------------------------------------------

bool symbol_manager::parse_sym_line(const char *line, FPTR &address, std::string &symbol)
{
#ifdef __GNUC__
/*
    32-bit gcc symbol line:
[271778](sec  1)(fl 0x00)(ty  20)(scl   3) (nx 0) 0x007df675 line_to_symbol(char const*, unsigned int&, bool)

    64-bit gcc symbol line:
[271775](sec  1)(fl 0x00)(ty  20)(scl   3) (nx 0) 0x00000000008dd1e9 line_to_symbol(char const*, unsigned long long&, bool)
*/

	// first look for a (ty) entry
	const char *type = strstr(line, "(ty  20)");
	if (type == NULL)
		return false;

	// scan forward in the line to find the address
	bool in_parens = false;
	for (const char *chptr = type; *chptr != 0; chptr++)
	{
		// track open/close parentheses
		if (*chptr == '(')
			in_parens = true;
		else if (*chptr == ')')
			in_parens = false;

		// otherwise, look for an 0x address
		else if (!in_parens && *chptr == '0' && chptr[1] == 'x')
		{
			// make sure we can get an address
			void *temp;
			if (sscanf(chptr, "0x%p", &temp) != 1)
				return false;
			address = m_text_base + reinterpret_cast<FPTR>(temp);

			// skip forward until we're past the space
			while (*chptr != 0 && !isspace(*chptr))
				chptr++;

			// extract the symbol name
			strtrimspace(symbol.assign(chptr));
			return (symbol.length() > 0);
		}
	}
#endif
	return false;
}


//-------------------------------------------------
//  parse_map_line - parse a line from a linker-
//  generated map file
//-------------------------------------------------

bool symbol_manager::parse_map_line(const char *line, FPTR &address, std::string &symbol)
{
#ifdef __GNUC__
/*
    32-bit gcc map line:
                0x0089cb00                nbmj9195_palette_r(_address_space const*, unsigned int)

    64-bit gcc map line:
                0x0000000000961afc                nbmj9195_palette_r(_address_space const*, unsigned int)
*/

	// find a matching start
	if (strncmp(line, "                0x", 18) == 0)
	{
		// make sure we can get an address
		void *temp;
		if (sscanf(&line[16], "0x%p", &temp) != 1)
			return false;
		address = reinterpret_cast<FPTR>(temp);

		// skip forward until we're past the space
		const char *chptr = &line[16];
		while (*chptr != 0 && !isspace(*chptr))
			chptr++;

		// extract the symbol name
		strtrimspace(symbol.assign(chptr));
		return (symbol.length() > 0);
	}
#endif
	return false;
}


//-------------------------------------------------
//  format_symbol - common symbol formatting
//-------------------------------------------------

void symbol_manager::format_symbol(const char *name, UINT32 displacement, const char *filename, int linenumber)
{
	// start with the address and offset
	strprintf(m_buffer, " (%s", name);
	if (displacement != 0)
		strcatprintf(m_buffer, "+0x%04x", (UINT32)displacement);

	// append file/line if present
	if (filename != NULL)
		strcatprintf(m_buffer, ", %s:%d", filename, linenumber);

	// close up the string
	m_buffer.append(")");
}


//-------------------------------------------------
//  get_text_section_base - figure out the base
//  of the .text section
//-------------------------------------------------

FPTR symbol_manager::get_text_section_base()
{
	dynamic_bind<PIMAGE_SECTION_HEADER (WINAPI *)(PIMAGE_NT_HEADERS, PVOID, ULONG)> image_rva_to_section(TEXT("dbghelp.dll"), "ImageRvaToSection");
	dynamic_bind<PIMAGE_NT_HEADERS (WINAPI *)(PVOID)> image_nt_header(TEXT("dbghelp.dll"), "ImageNtHeader");

	// start with the image base
	PVOID base = reinterpret_cast<PVOID>(GetModuleHandleUni());
	assert(base != NULL);

	// make sure we have the functions we need
	if (image_nt_header && image_rva_to_section)
	{
		// get the NT header
		PIMAGE_NT_HEADERS headers = (*image_nt_header)(base);
		assert(headers != NULL);

		// look ourself up (assuming we are in the .text section)
		PIMAGE_SECTION_HEADER section = (*image_rva_to_section)(headers, base, reinterpret_cast<FPTR>(get_text_section_base) - reinterpret_cast<FPTR>(base));
		if (section != NULL)
			return reinterpret_cast<FPTR>(base) + section->VirtualAddress;
	}

	// fallback to returning the image base (wrong)
	return reinterpret_cast<FPTR>(base);
}



//**************************************************************************
//  SAMPLING PROFILER
//**************************************************************************

//-------------------------------------------------
//  sampling_profiler - constructor
//-------------------------------------------------

sampling_profiler::sampling_profiler(UINT32 max_seconds, UINT8 stack_depth = 0)
	: m_thread(NULL),
		m_thread_id(0),
		m_thread_exit(false),
		m_stack_depth(stack_depth),
		m_entry_stride(stack_depth + 2),
		m_buffer(max_seconds * 1000 * m_entry_stride),
		m_buffer_ptr(&m_buffer[0]),
		m_buffer_end(&m_buffer[0] + max_seconds * 1000 * m_entry_stride)
{
}


//-------------------------------------------------
//  sampling_profiler - destructor
//-------------------------------------------------

sampling_profiler::~sampling_profiler()
{
}


//-------------------------------------------------
//  start - begin gathering profiling information
//-------------------------------------------------

void sampling_profiler::start()
{
	// do the dance to get a handle to ourself
	BOOL result = DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &m_target_thread,
			THREAD_GET_CONTEXT | THREAD_SUSPEND_RESUME | THREAD_QUERY_INFORMATION, FALSE, 0);
	assert_always(result, "Failed to get thread handle for main thread");

	// reset the exit flag
	m_thread_exit = false;

	// start the thread
	m_thread = CreateThread(NULL, 0, thread_entry, (LPVOID)this, 0, &m_thread_id);
	assert_always(m_thread != NULL, "Failed to create profiler thread\n");

	// max out the priority
	SetThreadPriority(m_thread, THREAD_PRIORITY_TIME_CRITICAL);
}


//-------------------------------------------------
//  stop - stop gathering profiling information
//-------------------------------------------------

void sampling_profiler::stop()
{
	// set the flag and wait a couple of seconds (max)
	m_thread_exit = true;
	WaitForSingleObject(m_thread, 2000);

	// regardless, close the handle
	CloseHandle(m_thread);
}


//-------------------------------------------------
//  compare_address - compare two entries by their
//  bucket address
//-------------------------------------------------

int CLIB_DECL sampling_profiler::compare_address(const void *item1, const void *item2)
{
	const FPTR *entry1 = reinterpret_cast<const FPTR *>(item1);
	const FPTR *entry2 = reinterpret_cast<const FPTR *>(item2);
	int mincount = MIN(entry1[0], entry2[0]);

	// sort in order of: bucket, caller, caller's caller, etc.
	for (int index = 1; index <= mincount; index++)
		if (entry1[index] != entry2[index])
			return entry1[index] - entry2[index];

	// if we match to the end, sort by the depth of the stack
	return entry1[0] - entry2[0];
}


//-------------------------------------------------
//  compare_frequency - compare two entries by
//  their frequency of occurrence
//-------------------------------------------------

int CLIB_DECL sampling_profiler::compare_frequency(const void *item1, const void *item2)
{
	const FPTR *entry1 = reinterpret_cast<const FPTR *>(item1);
	const FPTR *entry2 = reinterpret_cast<const FPTR *>(item2);

	// sort by frequency, then by address
	if (entry1[0] != entry2[0])
		return entry2[0] - entry1[0];
	return entry1[1] - entry2[1];
}


//-------------------------------------------------
//  print_results - output the results
//-------------------------------------------------

void sampling_profiler::print_results(symbol_manager &symbols)
{
	// cache the symbols
	symbols.cache_symbols();

	// step 1: find the base of each entry
	for (FPTR *current = &m_buffer[0]; current < m_buffer_ptr; current += m_entry_stride)
	{
		assert(current[0] >= 1 && current[0] < m_entry_stride);

		// convert the sampled PC to its function base as a bucket
		symbols.symbol_for_address(current[1]);
		current[1] = symbols.last_base();
	}

	// step 2: sort the results
	qsort(&m_buffer[0], (m_buffer_ptr - &m_buffer[0]) / m_entry_stride, m_entry_stride * sizeof(FPTR), compare_address);

	// step 3: count and collapse unique entries
	UINT32 total_count = 0;
	for (FPTR *current = &m_buffer[0]; current < m_buffer_ptr; )
	{
		int count = 1;
		FPTR *scan;
		for (scan = current + m_entry_stride; scan < m_buffer_ptr; scan += m_entry_stride)
		{
			if (compare_address(current, scan) != 0)
				break;
			scan[0] = 0;
			count++;
		}
		current[0] = count;
		total_count += count;
		current = scan;
	}

	// step 4: sort the results again, this time by frequency
	qsort(&m_buffer[0], (m_buffer_ptr - &m_buffer[0]) / m_entry_stride, m_entry_stride * sizeof(FPTR), compare_frequency);

	// step 5: print the results
	UINT32 num_printed = 0;
	for (FPTR *current = &m_buffer[0]; current < m_buffer_ptr && num_printed < 30; current += m_entry_stride)
	{
		// once we hit 0 frequency, we're done
		if (current[0] == 0)
			break;

		// output the result
		printf("%4.1f%% - %6d : %p%s\n", (double)current[0] * 100.0 / (double)total_count, (UINT32)current[0], reinterpret_cast<void *>(current[1]), symbols.symbol_for_address(current[1]));
		for (int index = 2; index < m_entry_stride; index++)
		{
			if (current[index] == 0)
				break;
			printf("                 %p%s\n", reinterpret_cast<void *>(current[index]), symbols.symbol_for_address(current[index]));
		}
		printf("\n");
		num_printed++;
	}
	symbols.reset_cache();
}


//-------------------------------------------------
//  thread_entry - thread entry stub
//-------------------------------------------------

DWORD WINAPI sampling_profiler::thread_entry(LPVOID lpParameter)
{
	reinterpret_cast<sampling_profiler *>(lpParameter)->thread_run();
	return 0;
}


//-------------------------------------------------
//  thread_run - sampling thread
//-------------------------------------------------

void sampling_profiler::thread_run()
{
	CONTEXT context;
	memset(&context, 0, sizeof(context));

	// loop until done
	stack_walker walker;
	while (!m_thread_exit && m_buffer_ptr < m_buffer_end)
	{
		// pause the main thread and get its context
		SuspendThread(m_target_thread);
		context.ContextFlags = CONTEXT_FULL;
		GetThreadContext(m_target_thread, &context);

		// first entry is a count
		FPTR *count = m_buffer_ptr++;
		*count = 0;

		// iterate over the frames until we run out or hit an error
		walker.reset(context, m_target_thread);
		int frame;
		for (frame = 0; frame <= m_stack_depth && walker.unwind(); frame++)
		{
			*m_buffer_ptr++ = walker.ip();
			*count += 1;
		}

		// fill in any missing parts with NULLs
		for (; frame <= m_stack_depth; frame++)
			*m_buffer_ptr++ = 0;

		// resume the thread
		ResumeThread(m_target_thread);

		// sleep for 1ms
		Sleep(1);
	}
}
