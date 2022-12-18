// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  winmain.cpp - Win32 main program
//
//============================================================

// MAME headers
#include "emu.h"
#include "emuopts.h"
#include "strconv.h"

// MAMEOS headers
#include "winmain.h"
#include "window.h"
#include "winutf8.h"
#include "winutil.h"
#include "winfile.h"
#include "modules/diagnostics/diagnostics_module.h"
#include "modules/monitor/monitor_common.h"

// standard C headers
#include <cctype>
#include <clocale>
#include <cstdarg>
#include <cstdio>
#include <mutex>
#include <optional>
#include <sstream>
#include <thread>

// standard windows headers
#include <windows.h>
#include <commctrl.h>
#include <mmsystem.h>
#include <tchar.h>
#include <io.h>

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

//============================================================
//  winui_output_error
//============================================================

class winui_output_error : public osd_output
{
private:
	struct ui_state
	{
		~ui_state()
		{
			if (thread)
				thread->join();
		}

		std::ostringstream buffer;
		std::optional<std::thread> thread;
		std::mutex mutex;
		bool active;
	};

	static ui_state &get_state()
	{
		static ui_state state;
		return state;
	}

public:
	virtual void output_callback(osd_output_channel channel, const util::format_argument_pack<std::ostream> &args) override
	{
		if (channel == OSD_OUTPUT_CHANNEL_ERROR)
		{
			// if we are in fullscreen mode, go to windowed mode
			if ((video_config.windowed == 0) && !osd_common_t::s_window_list.empty())
				winwindow_toggle_full_screen();

			auto &state(get_state());
			std::lock_guard<std::mutex> guard(state.mutex);
			util::stream_format(state.buffer, args);
			if (!state.active)
			{
				if (state.thread)
				{
					state.thread->join();
					state.thread = std::nullopt;
				}
				state.thread.emplace(
						[] ()
						{
							auto &state(get_state());
							std::string message;
							while (true)
							{
								{
									std::lock_guard<std::mutex> guard(state.mutex);
									message = std::move(state.buffer).str();
									if (message.empty())
									{
										state.active = false;
										return;
									}
									state.buffer.str(std::string());
								}
								// Don't hold any locks lock while calling MessageBox.
								// Parent window isn't set because MAME could destroy
								// the window out from under us on a fatal error.
								win_message_box_utf8(nullptr, message.c_str(), emulator_info::get_appname(), MB_OK);
							}
						});
				state.active = true;
			}
		}
		else
		{
			chain_output(channel, args);
		}
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

static int timeresult = !TIMERR_NOERROR;
static TIMECAPS timecaps;

static running_machine *g_current_machine;


//**************************************************************************
//  FUNCTION PROTOTYPES
//**************************************************************************

static BOOL WINAPI control_handler(DWORD type);
static int is_double_click_start(int argc);


//**************************************************************************
//  OPTIONS
//**************************************************************************

// struct definitions
const options_entry windows_options::s_option_entries[] =
{
	// performance options
	{ nullptr,                                        nullptr,    core_options::option_type::HEADER,     "WINDOWS PERFORMANCE OPTIONS" },
	{ WINOPTION_PRIORITY "(-15-1)",                   "0",        core_options::option_type::INTEGER,    "thread priority for the main game thread; range from -15 to 1" },
	{ WINOPTION_PROFILE,                              "0",        core_options::option_type::INTEGER,    "enables profiling, specifying the stack depth to track" },

	// video options
	{ nullptr,                                        nullptr,    core_options::option_type::HEADER,     "WINDOWS VIDEO OPTIONS" },
	{ WINOPTION_MENU,                                 "0",        core_options::option_type::BOOLEAN,    "enables menu bar if available by UI implementation" },
	{ WINOPTION_ATTACH_WINDOW,                        "",         core_options::option_type::STRING,     "attach to arbitrary window" },

	// post-processing options
	{ nullptr,                                                  nullptr,             core_options::option_type::HEADER,     "DIRECT3D POST-PROCESSING OPTIONS" },
	{ WINOPTION_HLSLPATH,                                       "hlsl",              core_options::option_type::PATH,       "path to HLSL support files" },
	{ WINOPTION_HLSL_ENABLE";hlsl",                             "0",                 core_options::option_type::BOOLEAN,    "enable HLSL post-processing (PS3.0 required)" },
	{ WINOPTION_HLSL_OVERSAMPLING,                              "0",                 core_options::option_type::BOOLEAN,    "enable HLSL oversampling" },
	{ WINOPTION_HLSL_WRITE,                                     OSDOPTVAL_AUTO,      core_options::option_type::PATH,       "enable HLSL AVI writing (huge disk bandwidth suggested)" },
	{ WINOPTION_HLSL_SNAP_WIDTH,                                "2048",              core_options::option_type::STRING,     "HLSL upscaled-snapshot width" },
	{ WINOPTION_HLSL_SNAP_HEIGHT,                               "1536",              core_options::option_type::STRING,     "HLSL upscaled-snapshot height" },
	{ WINOPTION_SHADOW_MASK_TILE_MODE,                          "0",                 core_options::option_type::INTEGER,    "shadow mask tile mode (0 for screen based, 1 for source based)" },
	{ WINOPTION_SHADOW_MASK_ALPHA";fs_shadwa(0.0-1.0)",         "0.0",               core_options::option_type::FLOAT,      "shadow mask alpha-blend value (1.0 is fully blended, 0.0 is no mask)" },
	{ WINOPTION_SHADOW_MASK_TEXTURE";fs_shadwt(0.0-1.0)",       "shadow-mask.png",   core_options::option_type::STRING,     "shadow mask texture name" },
	{ WINOPTION_SHADOW_MASK_COUNT_X";fs_shadww",                "6",                 core_options::option_type::INTEGER,    "shadow mask tile width, in screen dimensions" },
	{ WINOPTION_SHADOW_MASK_COUNT_Y";fs_shadwh",                "4",                 core_options::option_type::INTEGER,    "shadow mask tile height, in screen dimensions" },
	{ WINOPTION_SHADOW_MASK_USIZE";fs_shadwu(0.0-1.0)",         "0.1875",            core_options::option_type::FLOAT,      "shadow mask texture width, in U/V dimensions" },
	{ WINOPTION_SHADOW_MASK_VSIZE";fs_shadwv(0.0-1.0)",         "0.25",              core_options::option_type::FLOAT,      "shadow mask texture height, in U/V dimensions" },
	{ WINOPTION_SHADOW_MASK_UOFFSET";fs_shadwou(-1.0-1.0)",     "0.0",               core_options::option_type::FLOAT,      "shadow mask texture offset, in U direction" },
	{ WINOPTION_SHADOW_MASK_VOFFSET";fs_shadwov(-1.0-1.0)",     "0.0",               core_options::option_type::FLOAT,      "shadow mask texture offset, in V direction" },
	{ WINOPTION_DISTORTION";fs_dist(-1.0-1.0)",                 "0.0",               core_options::option_type::FLOAT,      "screen distortion amount" },
	{ WINOPTION_CUBIC_DISTORTION";fs_cubedist(-1.0-1.0)",       "0.0",               core_options::option_type::FLOAT,      "screen cubic distortion amount" },
	{ WINOPTION_DISTORT_CORNER";fs_distc(0.0-1.0)",             "0.0",               core_options::option_type::FLOAT,      "screen distort corner amount" },
	{ WINOPTION_ROUND_CORNER";fs_rndc(0.0-1.0)",                "0.0",               core_options::option_type::FLOAT,      "screen round corner amount" },
	{ WINOPTION_SMOOTH_BORDER";fs_smob(0.0-1.0)",               "0.0",               core_options::option_type::FLOAT,      "screen smooth border amount" },
	{ WINOPTION_REFLECTION";fs_ref(0.0-1.0)",                   "0.0",               core_options::option_type::FLOAT,      "screen reflection amount" },
	{ WINOPTION_VIGNETTING";fs_vig(0.0-1.0)",                   "0.0",               core_options::option_type::FLOAT,      "image vignetting amount" },
	/* Beam-related values below this line*/
	{ WINOPTION_SCANLINE_AMOUNT";fs_scanam(0.0-4.0)",           "0.0",               core_options::option_type::FLOAT,      "overall alpha scaling value for scanlines" },
	{ WINOPTION_SCANLINE_SCALE";fs_scansc(0.0-4.0)",            "1.0",               core_options::option_type::FLOAT,      "overall height scaling value for scanlines" },
	{ WINOPTION_SCANLINE_HEIGHT";fs_scanh(0.0-4.0)",            "1.0",               core_options::option_type::FLOAT,      "individual height scaling value for scanlines" },
	{ WINOPTION_SCANLINE_VARIATION";fs_scanv(0.0-4.0)",         "1.0",               core_options::option_type::FLOAT,      "individual height varying value for scanlines" },
	{ WINOPTION_SCANLINE_BRIGHT_SCALE";fs_scanbs(0.0-2.0)",     "1.0",               core_options::option_type::FLOAT,      "overall brightness scaling value for scanlines (multiplicative)" },
	{ WINOPTION_SCANLINE_BRIGHT_OFFSET";fs_scanbo(0.0-1.0)",    "0.0",               core_options::option_type::FLOAT,      "overall brightness offset value for scanlines (additive)" },
	{ WINOPTION_SCANLINE_JITTER";fs_scanjt(0.0-4.0)",           "0.0",               core_options::option_type::FLOAT,      "overall interlace jitter scaling value for scanlines" },
	{ WINOPTION_HUM_BAR_ALPHA";fs_humba(0.0-1.0)",              "0.0",               core_options::option_type::FLOAT,      "overall alpha scaling value for hum bar" },
	{ WINOPTION_DEFOCUS";fs_focus",                             "0.0,0.0",           core_options::option_type::STRING,     "overall defocus value in screen-relative coords" },
	{ WINOPTION_CONVERGE_X";fs_convx",                          "0.0,0.0,0.0",       core_options::option_type::STRING,     "convergence in screen-relative X direction" },
	{ WINOPTION_CONVERGE_Y";fs_convy",                          "0.0,0.0,0.0",       core_options::option_type::STRING,     "convergence in screen-relative Y direction" },
	{ WINOPTION_RADIAL_CONVERGE_X";fs_rconvx",                  "0.0,0.0,0.0",       core_options::option_type::STRING,     "radial convergence in screen-relative X direction" },
	{ WINOPTION_RADIAL_CONVERGE_Y";fs_rconvy",                  "0.0,0.0,0.0",       core_options::option_type::STRING,     "radial convergence in screen-relative Y direction" },
	/* RGB colorspace convolution below this line */
	{ WINOPTION_RED_RATIO";fs_redratio",                        "1.0,0.0,0.0",       core_options::option_type::STRING,     "red output signal generated by input signal" },
	{ WINOPTION_GRN_RATIO";fs_grnratio",                        "0.0,1.0,0.0",       core_options::option_type::STRING,     "green output signal generated by input signal" },
	{ WINOPTION_BLU_RATIO";fs_bluratio",                        "0.0,0.0,1.0",       core_options::option_type::STRING,     "blue output signal generated by input signal" },
	{ WINOPTION_SATURATION";fs_sat(0.0-4.0)",                   "1.0",               core_options::option_type::FLOAT,      "saturation scaling value" },
	{ WINOPTION_OFFSET";fs_offset",                             "0.0,0.0,0.0",       core_options::option_type::STRING,     "signal offset value (additive)" },
	{ WINOPTION_SCALE";fs_scale",                               "1.0,1.0,1.0",       core_options::option_type::STRING,     "signal scaling value (multiplicative)" },
	{ WINOPTION_POWER";fs_power",                               "1.0,1.0,1.0",       core_options::option_type::STRING,     "signal power value (exponential)" },
	{ WINOPTION_FLOOR";fs_floor",                               "0.0,0.0,0.0",       core_options::option_type::STRING,     "signal floor level" },
	{ WINOPTION_PHOSPHOR";fs_phosphor",                         "0.0,0.0,0.0",       core_options::option_type::STRING,     "phosphorescence decay rate (0.0 is instant, 1.0 is forever)" },
	{ WINOPTION_CHROMA_MODE,                                    "3",                 core_options::option_type::INTEGER,    "number of phosphors to use: 1 - monochrome, 2 - dichrome, 3 - trichrome (color)" },
	{ WINOPTION_CHROMA_CONVERSION_GAIN,                         "0.299,0.587,0.114", core_options::option_type::STRING,     "gain to be applied when summing RGB signal for monochrome and dichrome modes" },
	{ WINOPTION_CHROMA_A,                                       "0.64,0.33",         core_options::option_type::STRING,     "chromaticity coordinate for first phosphor" },
	{ WINOPTION_CHROMA_B,                                       "0.30,0.60",         core_options::option_type::STRING,     "chromaticity coordinate for second phosphor" },
	{ WINOPTION_CHROMA_C,                                       "0.15,0.06",         core_options::option_type::STRING,     "chromaticity coordinate for third phosphor" },
	{ WINOPTION_CHROMA_Y_GAIN,                                  "0.2126,0.7152,0.0722", core_options::option_type::STRING,  "gain to be applied for each phosphor" },
	/* NTSC simulation below this line */
	{ nullptr,                                                  nullptr,             core_options::option_type::HEADER,     "NTSC POST-PROCESSING OPTIONS" },
	{ WINOPTION_YIQ_ENABLE";yiq",                               "0",                 core_options::option_type::BOOLEAN,    "enable YIQ-space HLSL post-processing" },
	{ WINOPTION_YIQ_JITTER";yiqj",                              "0.0",               core_options::option_type::FLOAT,      "jitter for the NTSC signal processing" },
	{ WINOPTION_YIQ_CCVALUE";yiqcc",                            "3.57954545",        core_options::option_type::FLOAT,      "color carrier frequency for NTSC signal processing" },
	{ WINOPTION_YIQ_AVALUE";yiqa",                              "0.5",               core_options::option_type::FLOAT,      "A value for NTSC signal processing" },
	{ WINOPTION_YIQ_BVALUE";yiqb",                              "0.5",               core_options::option_type::FLOAT,      "B value for NTSC signal processing" },
	{ WINOPTION_YIQ_OVALUE";yiqo",                              "0.0",               core_options::option_type::FLOAT,      "outgoing Color Carrier phase offset for NTSC signal processing" },
	{ WINOPTION_YIQ_PVALUE";yiqp",                              "1.0",               core_options::option_type::FLOAT,      "incoming Pixel Clock scaling value for NTSC signal processing" },
	{ WINOPTION_YIQ_NVALUE";yiqn",                              "1.0",               core_options::option_type::FLOAT,      "Y filter notch width for NTSC signal processing" },
	{ WINOPTION_YIQ_YVALUE";yiqy",                              "6.0",               core_options::option_type::FLOAT,      "Y filter cutoff frequency for NTSC signal processing" },
	{ WINOPTION_YIQ_IVALUE";yiqi",                              "1.2",               core_options::option_type::FLOAT,      "I filter cutoff frequency for NTSC signal processing" },
	{ WINOPTION_YIQ_QVALUE";yiqq",                              "0.6",               core_options::option_type::FLOAT,      "Q filter cutoff frequency for NTSC signal processing" },
	{ WINOPTION_YIQ_SCAN_TIME";yiqsc",                          "52.6",              core_options::option_type::FLOAT,      "horizontal scanline duration for NTSC signal processing (microseconds)" },
	{ WINOPTION_YIQ_PHASE_COUNT";yiqpc",                        "2",                 core_options::option_type::INTEGER,    "phase count value for NTSC signal processing" },
	/* Vector simulation below this line */
	{ nullptr,                                                  nullptr,             core_options::option_type::HEADER,     "VECTOR POST-PROCESSING OPTIONS" },
	{ WINOPTION_VECTOR_BEAM_SMOOTH";vecsmooth",                 "0.0",               core_options::option_type::FLOAT,      "vector beam smoothness" },
	{ WINOPTION_VECTOR_LENGTH_SCALE";vecscale",                 "0.5",               core_options::option_type::FLOAT,      "maximum vector attenuation" },
	{ WINOPTION_VECTOR_LENGTH_RATIO";vecratio",                 "0.5",               core_options::option_type::FLOAT,      "minimum vector length affected by attenuation (vector length to screen size ratio)" },
	/* Bloom below this line */
	{ nullptr,                                                  nullptr,             core_options::option_type::HEADER,     "BLOOM POST-PROCESSING OPTIONS" },
	{ WINOPTION_BLOOM_BLEND_MODE,                               "0",                 core_options::option_type::INTEGER,    "bloom blend mode (0 for brighten, 1 for darken)" },
	{ WINOPTION_BLOOM_SCALE,                                    "0.0",               core_options::option_type::FLOAT,      "intensity factor for bloom" },
	{ WINOPTION_BLOOM_OVERDRIVE,                                "1.0,1.0,1.0",       core_options::option_type::STRING,     "overdrive factor for bloom" },
	{ WINOPTION_BLOOM_LEVEL0_WEIGHT,                            "1.0",               core_options::option_type::FLOAT,      "bloom level 0 weight (full-size target)" },
	{ WINOPTION_BLOOM_LEVEL1_WEIGHT,                            "0.64",              core_options::option_type::FLOAT,      "bloom level 1 weight (1/4 smaller that level 0 target)" },
	{ WINOPTION_BLOOM_LEVEL2_WEIGHT,                            "0.32",              core_options::option_type::FLOAT,      "bloom level 2 weight (1/4 smaller that level 1 target)" },
	{ WINOPTION_BLOOM_LEVEL3_WEIGHT,                            "0.16",              core_options::option_type::FLOAT,      "bloom level 3 weight (1/4 smaller that level 2 target)" },
	{ WINOPTION_BLOOM_LEVEL4_WEIGHT,                            "0.08",              core_options::option_type::FLOAT,      "bloom level 4 weight (1/4 smaller that level 3 target)" },
	{ WINOPTION_BLOOM_LEVEL5_WEIGHT,                            "0.06",              core_options::option_type::FLOAT,      "bloom level 5 weight (1/4 smaller that level 4 target)" },
	{ WINOPTION_BLOOM_LEVEL6_WEIGHT,                            "0.04",              core_options::option_type::FLOAT,      "bloom level 6 weight (1/4 smaller that level 5 target)" },
	{ WINOPTION_BLOOM_LEVEL7_WEIGHT,                            "0.02",              core_options::option_type::FLOAT,      "bloom level 7 weight (1/4 smaller that level 6 target)" },
	{ WINOPTION_BLOOM_LEVEL8_WEIGHT,                            "0.01",              core_options::option_type::FLOAT,      "bloom level 8 weight (1/4 smaller that level 7 target)" },
	{ WINOPTION_LUT_TEXTURE,                                    "lut-default.png",   core_options::option_type::PATH,       "3D LUT texture filename for screen, PNG format" },
	{ WINOPTION_LUT_ENABLE,                                     "0",                 core_options::option_type::BOOLEAN,    "Enables 3D LUT to be applied to screen after post-processing" },
	{ WINOPTION_UI_LUT_TEXTURE,                                 "lut-default.png",   core_options::option_type::PATH,       "3D LUT texture filename of UI, PNG format" },
	{ WINOPTION_UI_LUT_ENABLE,                                  "0",                 core_options::option_type::BOOLEAN,    "enable 3D LUT to be applied to UI and artwork after post-processing" },

	// full screen options
	{ nullptr,                                        nullptr,    core_options::option_type::HEADER,     "FULL SCREEN OPTIONS" },
	{ WINOPTION_TRIPLEBUFFER ";tb",                   "0",        core_options::option_type::BOOLEAN,    "enable triple buffering" },
	{ WINOPTION_FULLSCREENBRIGHTNESS ";fsb(0.1-2.0)", "1.0",      core_options::option_type::FLOAT,      "brightness value in full screen mode" },
	{ WINOPTION_FULLSCREENCONTRAST ";fsc(0.1-2.0)",   "1.0",      core_options::option_type::FLOAT,      "contrast value in full screen mode" },
	{ WINOPTION_FULLSCREENGAMMA ";fsg(0.1-3.0)",      "1.0",      core_options::option_type::FLOAT,      "gamma value in full screen mode" },

	// input options
	{ nullptr,                                        nullptr,    core_options::option_type::HEADER,     "INPUT DEVICE OPTIONS" },
	{ WINOPTION_GLOBAL_INPUTS,                        "0",        core_options::option_type::BOOLEAN,    "enable global inputs" },
	{ WINOPTION_DUAL_LIGHTGUN ";dual",                "0",        core_options::option_type::BOOLEAN,    "enable dual lightgun input" },

	{ nullptr }
};

//**************************************************************************
//  MAIN ENTRY POINT
//**************************************************************************

//============================================================
//  main
//============================================================

int main(int argc, char *argv[])
{
	std::setlocale(LC_ALL, "");
	std::vector<std::string> args = osd_get_command_line(argc, argv);

	// use small output buffers on non-TTYs (i.e. pipes)
	if (!isatty(fileno(stdout)))
		setvbuf(stdout, (char *) nullptr, _IOFBF, 64);
	if (!isatty(fileno(stderr)))
		setvbuf(stderr, (char *) nullptr, _IOFBF, 64);

	// initialize common controls
	InitCommonControls();

	// set a handler to catch ctrl-c
	SetConsoleCtrlHandler(control_handler, TRUE);

	// Initialize crash diagnostics
	diagnostics_module::get_instance()->init_crash_diagnostics();

	// parse config and cmdline options
	DWORD result;
	{
		windows_options options;
		windows_osd_interface osd(options);
		// if we're a GUI app, out errors to message boxes
		// Initialize this after the osd interface so that we are first in the
		// output order
		winui_output_error winerror;
		if (win_is_gui_application() || is_double_click_start(args.size()))
		{
			// if we are a GUI app, output errors to message boxes
			osd_output::push(&winerror);
			// make sure any console window that opened on our behalf is nuked
			FreeConsole();
		}
		osd.register_options();
		result = emulator_info::start_frontend(options, osd, args);
		osd_output::pop(&winerror);
	}

	return result;
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
	if (g_current_machine == nullptr || type == CTRL_C_EVENT || type == CTRL_BREAK_EVENT)
	{
		fprintf(stderr, ", exiting\n");
		TerminateProcess(GetCurrentProcess(), EMU_ERR_FATALERROR);
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
//  windows_options
//============================================================

windows_options::windows_options()
: osd_options()
{
	add_entries(s_option_entries);
}


//============================================================
//  output_oslog
//============================================================

void windows_osd_interface::output_oslog(const char *buffer)
{
	if (IsDebuggerPresent())
		win_output_debug_string_utf8(buffer);
	else
		fputs(buffer, stderr);
}


//============================================================
//  constructor
//============================================================

windows_osd_interface::windows_osd_interface(windows_options &options)
	: osd_common_t(options)
	, m_options(options)
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
	video_options_add("gdi", nullptr);
	video_options_add("d3d", nullptr);
#if USE_OPENGL
	video_options_add("opengl", nullptr);
#endif
	video_options_add("bgfx", nullptr);
	//video_options_add("auto", nullptr); // making d3d video default one
}

//============================================================
//  init
//============================================================

void windows_osd_interface::init(running_machine &machine)
{
	// call our parent
	osd_common_t::init(machine);

	const char *stemp;
	auto &options = downcast<windows_options &>(machine.options());

	// determine if we are benchmarking, and adjust options appropriately
	int bench = options.bench();
	if (bench > 0)
	{
		options.set_value(OPTION_SLEEP, false, OPTION_PRIORITY_MAXIMUM);
		options.set_value(OPTION_THROTTLE, false, OPTION_PRIORITY_MAXIMUM);
		options.set_value(OSDOPTION_SOUND, "none", OPTION_PRIORITY_MAXIMUM);
		options.set_value(OSDOPTION_VIDEO, "none", OPTION_PRIORITY_MAXIMUM);
		options.set_value(OPTION_SECONDS_TO_RUN, bench, OPTION_PRIORITY_MAXIMUM);
	}

	// determine if we are profiling, and adjust options appropriately
	int profile = options.profile();
	if (profile > 0)
	{
		options.set_value(OPTION_THROTTLE, false, OPTION_PRIORITY_MAXIMUM);
		options.set_value(OSDOPTION_NUMPROCESSORS, 1, OPTION_PRIORITY_MAXIMUM);
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
	for (const auto &info : osd_common_t::s_window_list)
	{
		machine.output().set_value(string_format("Orientation(%s)", info->monitor()->devicename()), std::static_pointer_cast<win_window_info>(info)->m_targetorient);
	}

	// hook up the debugger log
	if (options.oslog())
		machine.add_logerror_callback(&windows_osd_interface::output_oslog);

	// crank up the multimedia timer resolution to its max
	// this gives the system much finer timeslices
	timeresult = timeGetDevCaps(&timecaps, sizeof(timecaps));
	if (timeresult == TIMERR_NOERROR)
		timeBeginPeriod(timecaps.wPeriodMin);

	// create and start the profiler
	if (profile > 0)
	{
		diagnostics_module::get_instance()->start_profiler(1000, profile - 1);
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
	g_current_machine = nullptr;

	// cleanup sockets
	win_cleanup_sockets();

	osd_common_t::osd_exit();

	// stop the profiler
	diagnostics_module::get_instance()->stop_profiler();
	diagnostics_module::get_instance()->print_profiler_results();

	// restore the timer resolution
	if (timeresult == TIMERR_NOERROR)
		timeEndPeriod(timecaps.wPeriodMin);

	// one last pass at events
	winwindow_process_events(machine(), false, false);
}


//============================================================
//  osd_setup_osd_specific_emu_options
//============================================================

void osd_setup_osd_specific_emu_options(emu_options &opts)
{
	opts.add_entries(osd_options::s_option_entries);
	opts.add_entries(windows_options::s_option_entries);
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
