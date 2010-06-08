//============================================================
//
//  winmain.c - Win32 main program
//
//============================================================
//
//  Copyright Aaron Giles
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or
//  without modification, are permitted provided that the
//  following conditions are met:
//
//    * Redistributions of source code must retain the above
//      copyright notice, this list of conditions and the
//      following disclaimer.
//    * Redistributions in binary form must reproduce the
//      above copyright notice, this list of conditions and
//      the following disclaimer in the documentation and/or
//      other materials provided with the distribution.
//    * Neither the name 'MAME' nor the names of its
//      contributors may be used to endorse or promote
//      products derived from this software without specific
//      prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND
//  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
//  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
//  EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
//  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//  DAMAGE (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
//  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
//  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//============================================================

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <mmsystem.h>
#include <tchar.h>

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
#include "sound.h"
#include "input.h"
#include "output.h"
#include "config.h"
#include "osdepend.h"
#include "strconv.h"
#include "winutf8.h"
#include "winutil.h"
#include "debugger.h"

#define ENABLE_PROFILER		0
#define DEBUG_SLOW_LOCKS	0


//============================================================
//  TYPE DEFINITIONS
//============================================================

#ifdef UNICODE
#define UNICODE_POSTFIX "W"
#else
#define UNICODE_POSTFIX "A"
#endif

typedef BOOL (WINAPI *try_enter_critical_section_ptr)(LPCRITICAL_SECTION lpCriticalSection);

typedef HANDLE (WINAPI *av_set_mm_thread_characteristics_ptr)(LPCTSTR TaskName, LPDWORD TaskIndex);
typedef HANDLE (WINAPI *av_set_mm_max_thread_characteristics_ptr)(LPCTSTR FirstTask, LPCTSTR SecondTask, LPDWORD TaskIndex);
typedef BOOL (WINAPI *av_revert_mm_thread_characteristics_ptr)(HANDLE AvrtHandle);

// from dbghelp.dll
typedef PIMAGE_NT_HEADERS (WINAPI *image_nt_header_ptr)(PVOID ImageBase);
typedef BOOL (WINAPI *stack_walk_64_ptr)(DWORD MachineType, HANDLE hProcess, HANDLE hThread, LPSTACKFRAME64 StackFrame, PVOID ContextRecord, PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemoryRoutine, PFUNCTION_TABLE_ACCESS_ROUTINE64 FunctionTableAccessRoutine, PGET_MODULE_BASE_ROUTINE64 GetModuleBaseRoutine, PTRANSLATE_ADDRESS_ROUTINE64 TranslateAddress);
typedef BOOL (WINAPI *sym_initialize_ptr)(HANDLE hProcess, LPCTSTR UserSearchPath, BOOL fInvadeProcess);
typedef PVOID (WINAPI *sym_function_table_access_64_ptr)(HANDLE hProcess, DWORD64 AddrBase);
typedef DWORD64 (WINAPI *sym_get_module_base_64_ptr)(HANDLE hProcess, DWORD64 dwAddr);
typedef BOOL (WINAPI *sym_from_addr_ptr)(HANDLE hProcess, DWORD64 Address, PDWORD64 Displacement, PSYMBOL_INFO Symbol);
typedef BOOL (WINAPI *sym_get_line_from_addr_64_ptr)(HANDLE hProcess, DWORD64 dwAddr, PDWORD pdwDisplacement, PIMAGEHLP_LINE64 Line);

// from psapi.dll
typedef BOOL (WINAPI *get_module_information_ptr)(HANDLE hProcess, HMODULE hModule, LPMODULEINFO lpmodinfo, DWORD cb);



//============================================================
//  GLOBAL VARIABLES
//============================================================

// this line prevents globbing on the command line
int _CRT_glob = 0;


//============================================================
//  LOCAL VARIABLES
//============================================================

static try_enter_critical_section_ptr try_enter_critical_section;

static av_set_mm_thread_characteristics_ptr av_set_mm_thread_characteristics;
static av_set_mm_max_thread_characteristics_ptr av_set_mm_max_thread_characteristics;
static av_revert_mm_thread_characteristics_ptr av_revert_mm_thread_characteristics;

static image_nt_header_ptr image_nt_header;
static stack_walk_64_ptr stack_walk_64;
static sym_initialize_ptr sym_initialize;
static sym_function_table_access_64_ptr sym_function_table_access_64;
static sym_get_module_base_64_ptr sym_get_module_base_64;
static sym_from_addr_ptr sym_from_addr;
static sym_get_line_from_addr_64_ptr sym_get_line_from_addr_64;

static get_module_information_ptr get_module_information;

static char mapfile_name[MAX_PATH];
static LPTOP_LEVEL_EXCEPTION_FILTER pass_thru_filter;

static HANDLE watchdog_reset_event;
static HANDLE watchdog_exit_event;
static HANDLE watchdog_thread;


#ifndef MESS
static const TCHAR helpfile[] = TEXT("docs\\windows.txt");
#else
static const TCHAR helpfile[] = TEXT("mess.chm");
#endif

//static HANDLE mm_task = NULL;
//static DWORD task_index = 0;
static int timeresult;
//static MMRESULT result;
static TIMECAPS caps;



//============================================================
//  PROTOTYPES
//============================================================

static void osd_exit(running_machine *machine);

static void soft_link_functions(void);
static int is_double_click_start(int argc);
static DWORD WINAPI watchdog_thread_entry(LPVOID lpParameter);
static LONG WINAPI exception_filter(struct _EXCEPTION_POINTERS *info);
static const char *lookup_symbol(FPTR address);

static void start_profiler(void);
static void stop_profiler(void);



//============================================================
//  OPTIONS
//============================================================

// struct definitions
const options_entry mame_win_options[] =
{
	// debugging options
	{ NULL,                       NULL,       OPTION_HEADER,     "WINDOWS DEBUGGING OPTIONS" },
	{ "oslog",                    "0",        OPTION_BOOLEAN,    "output error.log data to the system debugger" },
	{ "watchdog;wdog",            "0",        0,                 "force the program to terminate if no updates within specified number of seconds" },
	{ "debugger_font;dfont",      "Lucida Console", 0,           "specifies the font to use for debugging; defaults to Lucida Console" },
	{ "debugger_font_size;dfontsize", "9",    0,                 "specifies the font size to use for debugging; defaults to 9 pt" },

	// performance options
	{ NULL,                       NULL,       OPTION_HEADER,     "WINDOWS PERFORMANCE OPTIONS" },
	{ "priority(-15-1)",          "0",        0,                 "thread priority for the main game thread; range from -15 to 1" },
	{ "multithreading;mt",        "0",        OPTION_BOOLEAN,    "enable multithreading; this enables rendering and blitting on a separate thread" },
	{ "numprocessors;np",         "auto",     0,				 "number of processors; this overrides the number the system reports" },

	// video options
	{ NULL,                       NULL,       OPTION_HEADER,     "WINDOWS VIDEO OPTIONS" },
	{ "video",                    "d3d",      0,                 "video output method: none, gdi, ddraw, or d3d" },
	{ "numscreens(1-4)",          "1",        0,                 "number of screens to create; usually, you want just one" },
	{ "window;w",                 "0",        OPTION_BOOLEAN,    "enable window mode; otherwise, full screen mode is assumed" },
	{ "maximize;max",             "1",        OPTION_BOOLEAN,    "default to maximized windows; otherwise, windows will be minimized" },
	{ "keepaspect;ka",            "1",        OPTION_BOOLEAN,    "constrain to the proper aspect ratio" },
	{ "prescale",                 "1",        0,                 "scale screen rendering by this amount in software" },
	{ "effect",                   "none",     0,                 "name of a PNG file to use for visual effects, or 'none'" },
	{ "waitvsync",                "0",        OPTION_BOOLEAN,    "enable waiting for the start of VBLANK before flipping screens; reduces tearing effects" },
	{ "syncrefresh",              "0",        OPTION_BOOLEAN,    "enable using the start of VBLANK for throttling instead of the game time" },

	// DirectDraw-specific options
	{ NULL,                       NULL,       OPTION_HEADER,     "DIRECTDRAW-SPECIFIC OPTIONS" },
	{ "hwstretch;hws",            "1",        OPTION_BOOLEAN,    "enable hardware stretching" },

	// Direct3D-specific options
	{ NULL,                       NULL,       OPTION_HEADER,     "DIRECT3D-SPECIFIC OPTIONS" },
	{ "d3dversion(8-9)",          "9",        0,                 "specify the preferred Direct3D version (8 or 9)" },
	{ "filter;d3dfilter;flt",     "1",        OPTION_BOOLEAN,    "enable bilinear filtering on screen output" },

	// per-window options
	{ NULL,                       NULL,       OPTION_HEADER,     "PER-WINDOW VIDEO OPTIONS" },
	{ "screen",                   "auto",     0,                 "explicit name of all screens; 'auto' here will try to make a best guess" },
	{ "aspect;screen_aspect",     "auto",     0,                 "aspect ratio for all screens; 'auto' here will try to make a best guess" },
	{ "resolution;r",             "auto",     0,                 "preferred resolution for all screens; format is <width>x<height>[@<refreshrate>] or 'auto'" },
	{ "view",                     "auto",     0,                 "preferred view for all screens" },

	{ "screen0",                  "auto",     0,                 "explicit name of the first screen; 'auto' here will try to make a best guess" },
	{ "aspect0",                  "auto",     0,                 "aspect ratio of the first screen; 'auto' here will try to make a best guess" },
	{ "resolution0;r0",           "auto",     0,                 "preferred resolution of the first screen; format is <width>x<height>[@<refreshrate>] or 'auto'" },
	{ "view0",                    "auto",     0,                 "preferred view for the first screen" },

	{ "screen1",                  "auto",     0,                 "explicit name of the second screen; 'auto' here will try to make a best guess" },
	{ "aspect1",                  "auto",     0,                 "aspect ratio of the second screen; 'auto' here will try to make a best guess" },
	{ "resolution1;r1",           "auto",     0,                 "preferred resolution of the second screen; format is <width>x<height>[@<refreshrate>] or 'auto'" },
	{ "view1",                    "auto",     0,                 "preferred view for the second screen" },

	{ "screen2",                  "auto",     0,                 "explicit name of the third screen; 'auto' here will try to make a best guess" },
	{ "aspect2",                  "auto",     0,                 "aspect ratio of the third screen; 'auto' here will try to make a best guess" },
	{ "resolution2;r2",           "auto",     0,                 "preferred resolution of the third screen; format is <width>x<height>[@<refreshrate>] or 'auto'" },
	{ "view2",                    "auto",     0,                 "preferred view for the third screen" },

	{ "screen3",                  "auto",     0,                 "explicit name of the fourth screen; 'auto' here will try to make a best guess" },
	{ "aspect3",                  "auto",     0,                 "aspect ratio of the fourth screen; 'auto' here will try to make a best guess" },
	{ "resolution3;r3",           "auto",     0,                 "preferred resolution of the fourth screen; format is <width>x<height>[@<refreshrate>] or 'auto'" },
	{ "view3",                    "auto",     0,                 "preferred view for the fourth screen" },

	// full screen options
	{ NULL,                       NULL,       OPTION_HEADER,     "FULL SCREEN OPTIONS" },
	{ "triplebuffer;tb",          "0",        OPTION_BOOLEAN,    "enable triple buffering" },
	{ "switchres",                "0",        OPTION_BOOLEAN,    "enable resolution switching" },
	{ "full_screen_brightness;fsb(0.1-2.0)","1.0",     0,        "brightness value in full screen mode" },
	{ "full_screen_contrast;fsc(0.1-2.0)", "1.0",      0,        "contrast value in full screen mode" },
	{ "full_screen_gamma;fsg(0.1-3.0)",    "1.0",      0,        "gamma value in full screen mode" },

	// sound options
	{ NULL,                       NULL,       OPTION_HEADER,     "WINDOWS SOUND OPTIONS" },
	{ "audio_latency(1-5)",       "2",        0,                 "set audio latency (increase to reduce glitches)" },

	// input options
	{ NULL,                       NULL,       OPTION_HEADER,     "INPUT DEVICE OPTIONS" },
	{ "dual_lightgun;dual",       "0",        OPTION_BOOLEAN,    "enable dual lightgun input" },

	{ NULL }
};


//============================================================
//  winui_output_error
//============================================================

static void winui_output_error(void *param, const char *format, va_list argptr)
{
	char buffer[1024];

	// if we are in fullscreen mode, go to windowed mode
	if ((video_config.windowed == 0) && (win_window_list != NULL))
		winwindow_toggle_full_screen();

	vsnprintf(buffer, ARRAY_LENGTH(buffer), format, argptr);
	win_message_box_utf8(win_window_list ? win_window_list->hwnd : NULL, buffer, APPNAME, MB_OK);
}



//============================================================
//  utf8_main
//============================================================

int main(int argc, char *argv[])
{
	char *ext;

	// initialize common controls
	InitCommonControls();

	// set up exception handling
	pass_thru_filter = SetUnhandledExceptionFilter(exception_filter);
	SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);

	// if we're a GUI app, out errors to message boxes
	if (win_is_gui_application() || is_double_click_start(argc))
	{
		// if we are a GUI app, output errors to message boxes
		mame_set_output_channel(OUTPUT_CHANNEL_ERROR, winui_output_error, NULL, NULL, NULL);

		// make sure any console window that opened on our behalf is nuked
		FreeConsole();
	}

	// soft-link optional functions
	soft_link_functions();

	// compute the name of the mapfile
	strcpy(mapfile_name, argv[0]);
	ext = strchr(mapfile_name, '.');
	if (ext != NULL)
		strcpy(ext, ".map");
	else
		strcat(mapfile_name, ".map");

	// parse config and cmdline options
	return cli_execute(argc, argv, mame_win_options);
}


//============================================================
//  output_oslog
//============================================================

static void output_oslog(running_machine *machine, const char *buffer)
{
	win_output_debug_string_utf8(buffer);
}


//============================================================
//  osd_init
//============================================================

void osd_init(running_machine *machine)
{
	int watchdog = options_get_int(mame_options(), WINOPTION_WATCHDOG);
	const char *stemp;

	// thread priority
	if (!(machine->debug_flags & DEBUG_FLAG_OSD_ENABLED))
		SetThreadPriority(GetCurrentThread(), options_get_int(mame_options(), WINOPTION_PRIORITY));

	// ensure we get called on the way out
	add_exit_callback(machine, osd_exit);

	// get number of processors
	stemp = options_get_string(mame_options(), WINOPTION_NUMPROCESSORS);

	osd_num_processors = 0;

	if (strcmp(stemp, "auto") != 0)
	{
		osd_num_processors = atoi(stemp);
		if (osd_num_processors < 1)
		{
			mame_printf_warning("Warning: numprocessors < 1 doesn't make much sense. Assuming auto ...\n");
			osd_num_processors = 0;
		}
	}

	// initialize the subsystems
	winvideo_init(machine);
	winsound_init(machine);
	wininput_init(machine);
	winoutput_init(machine);

	// hook up the debugger log
	if (options_get_bool(mame_options(), WINOPTION_OSLOG))
		add_logerror_callback(machine, output_oslog);

	// crank up the multimedia timer resolution to its max
	// this gives the system much finer timeslices
	timeresult = timeGetDevCaps(&caps, sizeof(caps));
	if (timeresult == TIMERR_NOERROR)
		timeBeginPeriod(caps.wPeriodMin);

	// set our multimedia tasks if we can
//      if (av_set_mm_thread_characteristics != NULL)
//          mm_task = (*av_set_mm_thread_characteristics)(TEXT("Playback"), &task_index);

	start_profiler();

	// if a watchdog thread is requested, create one
	if (watchdog != 0)
	{
		watchdog_reset_event = CreateEvent(NULL, FALSE, FALSE, NULL);
		assert_always(watchdog_reset_event != NULL, "Failed to create watchdog reset event");
		watchdog_exit_event = CreateEvent(NULL, TRUE, FALSE, NULL);
		assert_always(watchdog_exit_event != NULL, "Failed to create watchdog exit event");
		watchdog_thread = CreateThread(NULL, 0, watchdog_thread_entry, (LPVOID)(FPTR)watchdog, 0, NULL);
		assert_always(watchdog_thread != NULL, "Failed to create watchdog thread");
	}
}


//============================================================
//  osd_exit
//============================================================

static void osd_exit(running_machine *machine)
{
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

	stop_profiler();

	// turn off our multimedia tasks
//      if (av_revert_mm_thread_characteristics != NULL)
//          (*av_revert_mm_thread_characteristics)(mm_task);

	// restore the timer resolution
	if (timeresult == TIMERR_NOERROR)
		timeEndPeriod(caps.wPeriodMin);

	// one last pass at events
	winwindow_process_events(machine, 0);
}



//============================================================
//  soft_link_functions
//============================================================

static void soft_link_functions(void)
{
	HMODULE library;

	// see if we can use TryEnterCriticalSection
	library = LoadLibrary(TEXT("kernel32.dll"));
	if (library != NULL)
		try_enter_critical_section = (try_enter_critical_section_ptr)GetProcAddress(library, "TryEnterCriticalSection");

	// see if we can use the multimedia scheduling functions in Vista
	library = LoadLibrary(TEXT("avrt.dll"));
	if (library != NULL)
	{
		av_set_mm_thread_characteristics = (av_set_mm_thread_characteristics_ptr)GetProcAddress(library, "AvSetMmThreadCharacteristics" UNICODE_POSTFIX);
		av_set_mm_max_thread_characteristics = (av_set_mm_max_thread_characteristics_ptr)GetProcAddress(library, "AvSetMmMaxThreadCharacteristics" UNICODE_POSTFIX);
		av_revert_mm_thread_characteristics = (av_revert_mm_thread_characteristics_ptr)GetProcAddress(library, "AvRevertMmThreadCharacteristics");
	}

	// psapi helpers (not available in win9x)
	library = LoadLibrary(TEXT("psapi.dll"));
	if (library != NULL)
		get_module_information = (get_module_information_ptr)GetProcAddress(library, "GetModuleInformation");

	// dbghelp helpers
	library = LoadLibrary(TEXT("dbghelp.dll"));
	if (library != NULL)
	{
		image_nt_header = (image_nt_header_ptr)GetProcAddress(library, "ImageNtHeader");
		stack_walk_64 = (stack_walk_64_ptr)GetProcAddress(library, "StackWalk64");
		sym_initialize = (sym_initialize_ptr)GetProcAddress(library, "SymInitialize");
		sym_function_table_access_64 = (sym_function_table_access_64_ptr)GetProcAddress(library, "SymFunctionTableAccess64");
		sym_get_module_base_64 = (sym_get_module_base_64_ptr)GetProcAddress(library, "SymGetModuleBase64");
		sym_from_addr = (sym_from_addr_ptr)GetProcAddress(library, "SymFromAddr");
		sym_get_line_from_addr_64 = (sym_get_line_from_addr_64_ptr)GetProcAddress(library, "SymGetLineFromAddr64");
	}
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
		{ EXCEPTION_ACCESS_VIOLATION,		"ACCESS VIOLATION" },
		{ EXCEPTION_DATATYPE_MISALIGNMENT,	"DATATYPE MISALIGNMENT" },
		{ EXCEPTION_BREAKPOINT, 			"BREAKPOINT" },
		{ EXCEPTION_SINGLE_STEP,			"SINGLE STEP" },
		{ EXCEPTION_ARRAY_BOUNDS_EXCEEDED,	"ARRAY BOUNDS EXCEEDED" },
		{ EXCEPTION_FLT_DENORMAL_OPERAND,	"FLOAT DENORMAL OPERAND" },
		{ EXCEPTION_FLT_DIVIDE_BY_ZERO,		"FLOAT DIVIDE BY ZERO" },
		{ EXCEPTION_FLT_INEXACT_RESULT,		"FLOAT INEXACT RESULT" },
		{ EXCEPTION_FLT_INVALID_OPERATION,	"FLOAT INVALID OPERATION" },
		{ EXCEPTION_FLT_OVERFLOW,			"FLOAT OVERFLOW" },
		{ EXCEPTION_FLT_STACK_CHECK,		"FLOAT STACK CHECK" },
		{ EXCEPTION_FLT_UNDERFLOW,			"FLOAT UNDERFLOW" },
		{ EXCEPTION_INT_DIVIDE_BY_ZERO,		"INTEGER DIVIDE BY ZERO" },
		{ EXCEPTION_INT_OVERFLOW,			"INTEGER OVERFLOW" },
		{ EXCEPTION_PRIV_INSTRUCTION,		"PRIVILEGED INSTRUCTION" },
		{ EXCEPTION_IN_PAGE_ERROR,			"IN PAGE ERROR" },
		{ EXCEPTION_ILLEGAL_INSTRUCTION,	"ILLEGAL INSTRUCTION" },
		{ EXCEPTION_NONCONTINUABLE_EXCEPTION,"NONCONTINUABLE EXCEPTION" },
		{ EXCEPTION_STACK_OVERFLOW, 		"STACK OVERFLOW" },
		{ EXCEPTION_INVALID_DISPOSITION,	"INVALID DISPOSITION" },
		{ EXCEPTION_GUARD_PAGE, 			"GUARD PAGE VIOLATION" },
		{ EXCEPTION_INVALID_HANDLE, 		"INVALID HANDLE" },
		{ 0,								"UNKNOWN EXCEPTION" }
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
			lookup_symbol((FPTR)info->ExceptionRecord->ExceptionAddress), exception_table[i].string);

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

	if (image_nt_header != NULL &&
		stack_walk_64 != NULL &&
		sym_initialize != NULL &&
		sym_function_table_access_64 != NULL &&
		sym_get_module_base_64 != NULL &&
		sym_from_addr != NULL &&
		get_module_information != NULL)
	{
		CONTEXT context = *info->ContextRecord;
		STACKFRAME64 stackframe;

		// initialize the symbol lookup
		sym_initialize(GetCurrentProcess(), NULL, TRUE);

		// reprint the actual exception address
		fprintf(stderr, "-----------------------------------------------------\n");
		fprintf(stderr, "Stack crawl:\n");

		memset(&stackframe, 0, sizeof(stackframe));
#ifdef PTR64
		stackframe.AddrPC.Offset = context.Rip;
		stackframe.AddrFrame.Offset = context.Rsp;
		stackframe.AddrStack.Offset = context.Rsp;
#else
		stackframe.AddrPC.Offset = context.Eip;
		stackframe.AddrFrame.Offset = context.Ebp;
		stackframe.AddrStack.Offset = context.Esp;
#endif
		stackframe.AddrPC.Mode = AddrModeFlat;
		stackframe.AddrFrame.Mode = AddrModeFlat;
		stackframe.AddrStack.Mode = AddrModeFlat;

		while (stack_walk_64(
#ifdef PTR64
				IMAGE_FILE_MACHINE_AMD64,
#else
				IMAGE_FILE_MACHINE_I386,
#endif
				GetCurrentProcess(),
				GetCurrentThread(),
				&stackframe,
				&context,
				NULL,
				sym_function_table_access_64,
				sym_get_module_base_64,
				NULL))
		{
			fprintf(stderr, "  %p: %p%s\n", (void *)stackframe.AddrFrame.Offset, (void *)stackframe.AddrPC.Offset, lookup_symbol((FPTR)stackframe.AddrPC.Offset));
		}
	}

	// exit
	return EXCEPTION_CONTINUE_SEARCH;
}


//============================================================
//  line_to_symbol
//============================================================

static const char *line_to_symbol(const char *line, FPTR &address)
{
#ifdef __GNUC__
/*
    32-bit gcc map line:
                0x0089cb00                nbmj9195_palette_r(_address_space const*, unsigned int)

    64-bit gcc map line:
                0x0000000000961afc                nbmj9195_palette_r(_address_space const*, unsigned int)
*/
	char symbol[1024];
	void *temp;

	// find a matching start
	if (strncmp(line, "                0x", 18) == 0)
		if (sscanf(line, " 0x%p %s", &temp, symbol) == 2)
			if (symbol[0] != '0' && symbol[1] != 'x')
			{
				address = reinterpret_cast<FPTR>(temp);
				return strstr(line, symbol);
			}
#endif

#ifdef _MSC_VER
/*
    32-bit MSVC map line:
 0001:00387890       ?nbmj9195_palette_r@@YAEPBU_address_space@@I@Z 00788890 f   nichibut:nbmj9195.o

    64-bit MSVC map line:
 0001:004d7510       ?nbmj9195_palette_r@@YAEPEBU_address_space@@I@Z 00000001404d8510 f   nichibut:nbmj9195.o
*/
	static char symbol[1024];
	int dummy1, dummy2;
	void *temp;

	symbol[0] = 0;
	if (line[0] == ' ' && line[5] == ':')
		if (sscanf(line, " %04x:%08x %s %p", &dummy1, &dummy2, symbol, &temp) == 4)
		{
			address = reinterpret_cast<FPTR>(temp);
			return symbol;
		}
#endif

	// nope, not a symbol line
	return NULL;
}


//============================================================
//  lookup_symbol
//============================================================

static const char *lookup_symbol(FPTR address)
{
	static char buffer[1024];

	// first try to do it formally
	BYTE info_buffer[sizeof(SYMBOL_INFO) + 256];
	SYMBOL_INFO &info = *reinterpret_cast<SYMBOL_INFO *>(&info_buffer[0]);
	DWORD64 displacement;

	// even through the struct says TCHAR, we actually get back an ANSI string here
	memset(info_buffer, 0, sizeof(info_buffer));
	info.SizeOfStruct = sizeof(info);
	info.MaxNameLen = sizeof(info_buffer) - sizeof(info);
	if (sym_from_addr(GetCurrentProcess(), address, &displacement, &info))
	{
		IMAGEHLP_LINE64 lineinfo = { sizeof(lineinfo) };
		DWORD linedisp;

		// try to get source info as well; again we are returned an ANSI string
		if (sym_get_line_from_addr_64 != NULL && sym_get_line_from_addr_64(GetCurrentProcess(), address, &linedisp, &lineinfo))
			sprintf(buffer, " (%s+0x%04x, %s:%d)", info.Name, (UINT32)displacement, lineinfo.FileName, (int)lineinfo.LineNumber);
		else
			sprintf(buffer, " (%s+0x%04x)", info.Name, (UINT32)displacement);
		return buffer;
	}

	// see if we have a map file
	FILE *map = fopen(mapfile_name, "r");
	if (map == NULL)
		return " (no map)";

	// reset the bests
	astring best_symbol;
	FPTR best_addr = 0;

	// parse the file, looking for map entries
	char line[1024];
	while (fgets(line, sizeof(line) - 1, map))
	{
		FPTR addr;
		const char *symbol = line_to_symbol(line, addr);
		if (symbol != NULL && addr <= address && addr > best_addr)
		{
			best_addr = addr;
			best_symbol.cpy(symbol);
		}
	}
	fclose(map);

	// create the final result
	if (address - best_addr > 0x10000)
		return " (unknown)";
	sprintf(buffer, " (%s+0x%04x)", best_symbol.trimspace().cstr(), (UINT32)(address - best_addr));
	return buffer;
}



#if ENABLE_PROFILER

//============================================================
//
//  profiler.c - Sampling profiler
//
//============================================================

//============================================================
//  TYPE DEFINITIONS
//============================================================

#define MAX_SYMBOLS		65536

typedef struct _map_entry map_entry;

struct _map_entry
{
	FPTR start;
	FPTR end;
	UINT64 hits;
	char *name;
};



//============================================================
//  LOCAL VARIABLES
//============================================================

static map_entry symbol_map[MAX_SYMBOLS];
static int map_entries;

static HANDLE profiler_thread;
static DWORD profiler_thread_id;
static volatile UINT8 profiler_thread_exit;



//============================================================
//  compare_base
//  compare_hits -- qsort callbacks to sort on
//============================================================

static int CLIB_DECL compare_start(const void *item1, const void *item2)
{
	return ((const map_entry *)item1)->start - ((const map_entry *)item2)->start;
}

static int CLIB_DECL compare_hits(const void *item1, const void *item2)
{
	return ((const map_entry *)item2)->hits - ((const map_entry *)item1)->hits;
}


//============================================================
//  add_symbol_map_entry
//  parse_map_file
//============================================================

static void add_symbol_map_entry(UINT32 start, const char *name)
{
	if (map_entries == MAX_SYMBOLS)
		fatalerror("Symbol table full");
	symbol_map[map_entries].start = start;
	symbol_map[map_entries].name = core_strdup(name);
	if (symbol_map[map_entries].name == NULL)
		fatalerror("Out of memory");
	map_entries++;
}

static void parse_map_file(void)
{
	int got_text = 0;
	char line[1024];
	FILE *map;
	int i;

	// open the map file
	map = fopen(mapfile_name, "r");
	if (!map)
		return;

	// parse out the various symbols into map entries
	map_entries = 0;
	while (fgets(line, sizeof(line) - 1, map))
	{
		/* look for the code boundaries */
		if (!got_text && !strncmp(line, ".text           0x", 18))
		{
			UINT32 base, size;
			if (sscanf(line, ".text           0x%08x 0x%x", &base, &size) == 2)
			{
				add_symbol_map_entry(base, "Code start");
				add_symbol_map_entry(base+size, "Other");
				got_text = 1;
			}
		}

		/* look for symbols */
		FPTR addr;
		const char *symbol = line_to_symbol(line, addr);
		if (symbol != NULL)
			add_symbol_map_entry(addr, symbol);
	}

	/* add a symbol for end-of-memory */
	add_symbol_map_entry(~0, "<end>");
	map_entries--;

	/* close the file */
	fclose(map);

	/* sort by address */
	qsort(symbol_map, map_entries, sizeof(symbol_map[0]), compare_start);

	/* fill in the end of each bucket */
	for (i = 0; i < map_entries; i++)
		symbol_map[i].end = symbol_map[i+1].start ? (symbol_map[i+1].start - 1) : 0;
}


//============================================================
//  free_symbol_map
//============================================================

static void free_symbol_map(void)
{
	int i;

	for (i = 0; i <= map_entries; i++)
	{
		free(symbol_map[i].name);
		symbol_map[i].name = NULL;
	}
}


//============================================================
//  output_symbol_list
//============================================================

static void output_symbol_list(FILE *f)
{
	map_entry *entry;
	int i;

	/* sort by hits */
	qsort(symbol_map, map_entries, sizeof(symbol_map[0]), compare_hits);

	for (i = 0, entry = symbol_map; i < map_entries; i++, entry++)
		if (entry->hits > 0)
			fprintf(f, "%10d  %08X-%08X  %s\n", entry->hits, entry->start, entry->end, entry->name);
}



//============================================================
//  increment_bucket
//============================================================

static void increment_bucket(UINT32 addr)
{
	int i;

	for (i = 0; i < map_entries; i++)
		if (addr <= symbol_map[i].end)
		{
			symbol_map[i].hits++;
			return;
		}
}



//============================================================
//  profiler_thread
//============================================================

static DWORD WINAPI profiler_thread_entry(LPVOID lpParameter)
{
	HANDLE mainThread = (HANDLE)lpParameter;
	CONTEXT context;

	/* loop until done */
	memset(&context, 0, sizeof(context));
	while (!profiler_thread_exit)
	{
		/* pause the main thread and get its context */
		SuspendThread(mainThread);
		context.ContextFlags = CONTEXT_FULL;
		GetThreadContext(mainThread, &context);
		ResumeThread(mainThread);

		/* add to the bucket */
		increment_bucket(context.Eip);

		/* sleep */
		Sleep(1);
	}

	return 0;
}



//============================================================
//  start_profiler
//============================================================

static void start_profiler(void)
{
	HANDLE currentThread;
	BOOL result;

	// parse the map file, if present
	parse_map_file();

	// do the dance to get a handle to ourself
	result = DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &currentThread,
			THREAD_GET_CONTEXT | THREAD_SUSPEND_RESUME | THREAD_QUERY_INFORMATION, FALSE, 0);
	assert_always(result, "Failed to get thread handle for main thread");

	profiler_thread_exit = 0;

	// start the thread
	profiler_thread = CreateThread(NULL, 0, profiler_thread_entry, (LPVOID)currentThread, 0, &profiler_thread_id);
	assert_always(profiler_thread, "Failed to create profiler thread\n");

	// max out the priority
	SetThreadPriority(profiler_thread, THREAD_PRIORITY_TIME_CRITICAL);
}



//============================================================
//  stop_profiler
//============================================================

static void stop_profiler(void)
{
	profiler_thread_exit = 1;
	WaitForSingleObject(profiler_thread, 2000);
	output_symbol_list(stderr);
	free_symbol_map();
}

#else

static void start_profiler(void) {}
static void stop_profiler(void) {}

#endif
