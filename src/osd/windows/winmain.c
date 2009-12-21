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

// MAME headers
#include "driver.h"
#include "clifront.h"

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

static char mapfile_name[MAX_PATH];
static LPTOP_LEVEL_EXCEPTION_FILTER pass_thru_filter;

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
static LONG CALLBACK exception_filter(struct _EXCEPTION_POINTERS *info);
static const char *lookup_symbol(UINT32 address);
static int get_code_base_size(UINT32 *base, UINT32 *size);

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

int main(int argc, char **argv)
{
	char *ext;

	// initialize common controls
	InitCommonControls();

	// set up exception handling
	pass_thru_filter = SetUnhandledExceptionFilter(exception_filter);

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
	if (!options_get_bool(mame_options(), OPTION_DEBUG))
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
		watchdog_exit_event = CreateEvent(NULL, TRUE, FALSE, NULL);
		assert_always(watchdog_exit_event != NULL, "Failed to create watchdog event");
		watchdog_thread = CreateThread(NULL, 0, watchdog_thread_entry, (LPVOID)watchdog, 0, NULL);
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
	try_enter_critical_section = NULL;
	library = LoadLibrary(TEXT("kernel32.dll"));
	if (library != NULL)
		try_enter_critical_section = (try_enter_critical_section_ptr)GetProcAddress(library, "TryEnterCriticalSection");

	// see if we can use the multimedia scheduling functions in Vista
	av_set_mm_thread_characteristics = NULL;
	av_set_mm_max_thread_characteristics = NULL;
	av_revert_mm_thread_characteristics = NULL;
	library = LoadLibrary(TEXT("avrt.dll"));
	if (library != NULL)
	{
		av_set_mm_thread_characteristics = (av_set_mm_thread_characteristics_ptr)GetProcAddress(library, "AvSetMmThreadCharacteristics" UNICODE_POSTFIX);
		av_set_mm_max_thread_characteristics = (av_set_mm_max_thread_characteristics_ptr)GetProcAddress(library, "AvSetMmMaxThreadCharacteristics" UNICODE_POSTFIX);
		av_revert_mm_thread_characteristics = (av_revert_mm_thread_characteristics_ptr)GetProcAddress(library, "AvRevertMmThreadCharacteristics");
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
	DWORD wait_result;

	wait_result = WaitForSingleObject(watchdog_exit_event, timeout);
	if (wait_result == WAIT_TIMEOUT)
		TerminateProcess(GetCurrentProcess(), -1);
	return 0;
}


//============================================================
//  exception_filter
//============================================================

static LONG CALLBACK exception_filter(struct _EXCEPTION_POINTERS *info)
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
		{ EXCEPTION_INT_OVERFLOW, 			"INTEGER OVERFLOW" },
		{ EXCEPTION_PRIV_INSTRUCTION, 		"PRIVILEGED INSTRUCTION" },
		{ EXCEPTION_IN_PAGE_ERROR, 			"IN PAGE ERROR" },
		{ EXCEPTION_ILLEGAL_INSTRUCTION, 	"ILLEGAL INSTRUCTION" },
		{ EXCEPTION_NONCONTINUABLE_EXCEPTION,"NONCONTINUABLE EXCEPTION" },
		{ EXCEPTION_STACK_OVERFLOW, 		"STACK OVERFLOW" },
		{ EXCEPTION_INVALID_DISPOSITION, 	"INVALID DISPOSITION" },
		{ EXCEPTION_GUARD_PAGE, 			"GUARD PAGE VIOLATION" },
		{ EXCEPTION_INVALID_HANDLE, 		"INVALID HANDLE" },
		{ 0,								"UNKNOWN EXCEPTION" }
	};
	static int already_hit = 0;
#ifndef PTR64
	UINT32 code_start, code_size;
#endif
	int i;

	// if we're hitting this recursively, just exit
	if (already_hit)
		ExitProcess(100);
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
		fprintf(stderr, "While attempting to %s memory at %08X\n",
				info->ExceptionRecord->ExceptionInformation[0] ? "write" : "read",
				(UINT32)info->ExceptionRecord->ExceptionInformation[1]);

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

#ifndef PTR64
	// crawl the stack for a while
	if (get_code_base_size(&code_start, &code_size))
	{
		char prev_symbol[1024], curr_symbol[1024];
		UINT32 last_call = (UINT32)info->ExceptionRecord->ExceptionAddress;
		UINT32 esp_start = info->ContextRecord->Esp;
		UINT32 esp_end = (esp_start | 0xffff) + 1;
		UINT32 esp;

		// reprint the actual exception address
		fprintf(stderr, "-----------------------------------------------------\n");
		fprintf(stderr, "Stack crawl:\n");
		fprintf(stderr, "exception-> %08X%s\n", last_call, strcpy(prev_symbol, lookup_symbol(last_call)));

		// crawl the stack until we hit the next 64k boundary
		for (esp = esp_start; esp < esp_end; esp += 4)
		{
			UINT32 stack_val = *(UINT32 *)esp;

			// if the value on the stack points within the code block, check it out
			if (stack_val >= code_start && stack_val < code_start + code_size)
			{
				UINT8 *return_addr = (UINT8 *)stack_val;
				UINT32 call_target = 0;

				// make sure the code that we think got us here is actually a CALL instruction
				if (return_addr[-5] == 0xe8)
					call_target = stack_val - 5 + *(INT32 *)&return_addr[-4];
				if ((return_addr[-2] == 0xff && (return_addr[-1] & 0x38) == 0x10) ||
					(return_addr[-3] == 0xff && (return_addr[-2] & 0x38) == 0x10) ||
					(return_addr[-4] == 0xff && (return_addr[-3] & 0x38) == 0x10) ||
					(return_addr[-5] == 0xff && (return_addr[-4] & 0x38) == 0x10) ||
					(return_addr[-6] == 0xff && (return_addr[-5] & 0x38) == 0x10) ||
					(return_addr[-7] == 0xff && (return_addr[-6] & 0x38) == 0x10))
					call_target = 1;

				// make sure it points somewhere a little before the last call
				if (call_target == 1 || (call_target < last_call && call_target >= last_call - 0x1000))
				{
					char *stop_compare = strchr(prev_symbol, '+');

					// don't print duplicate hits in the same routine
					strcpy(curr_symbol, lookup_symbol(stack_val));
					if (stop_compare == NULL || strncmp(curr_symbol, prev_symbol, stop_compare - prev_symbol))
					{
						strcpy(prev_symbol, curr_symbol);
						fprintf(stderr, "  %08X: %08X%s\n", esp, stack_val, curr_symbol);
						last_call = stack_val;
					}
				}
			}
		}
	}
#endif

	// exit
	ExitProcess(100);
}



//============================================================
//  lookup_symbol
//============================================================

static const char *lookup_symbol(UINT32 address)
{
	static char buffer[1024];
	FILE *	map = fopen(mapfile_name, "r");
	char	symbol[1024], best_symbol[1024];
	UINT32	addr, best_addr = 0;
	char	line[1024];

	// if no file, return nothing
	if (map == NULL)
		return "";

	// reset the bests
	*best_symbol = 0;
	best_addr = 0;

	// parse the file, looking for map entries
	while (fgets(line, sizeof(line) - 1, map))
		if (strncmp(line, "                0x", 18) == 0)
			if (sscanf(line, "                0x%08x %s", &addr, symbol) == 2)
				if (addr <= address && addr > best_addr)
				{
					best_addr = addr;
					strcpy(best_symbol, symbol);
				}
	fclose(map);

	// create the final result
	if (address - best_addr > 0x10000)
		return "";
	sprintf(buffer, " (%s+0x%04x)", best_symbol, address - best_addr);
	return buffer;
}



//============================================================
//  get_code_base_size
//============================================================

static int get_code_base_size(UINT32 *base, UINT32 *size)
{
	FILE *	map = fopen(mapfile_name, "r");
	char	line[1024];
	int result = 0;

	// if no file, return nothing
	if (map == NULL)
		return 0;

	// parse the file, looking for .text entry
	while (fgets(line, sizeof(line) - 1, map))
		if (!strncmp(line, ".text           0x", 18))
			if (sscanf(line, ".text           0x%08x 0x%x", base, size) == 2)
			{
				result = 1;
				break;
			}

	fclose(map);
	return result;
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
	UINT32 start;
	UINT32 end;
	UINT32 hits;
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
		else if (!strncmp(line, "                0x", 18))
		{
			char symbol[1024];
			UINT32 addr;
			if (sscanf(line, "                0x%08x %s", &addr, symbol) == 2)
			{
				add_symbol_map_entry(addr, symbol);
			}
		}
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

	/* do the dance to get a handle to ourself */
	result = DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &currentThread,
			THREAD_GET_CONTEXT | THREAD_SUSPEND_RESUME | THREAD_QUERY_INFORMATION, FALSE, 0);
	assert_always(result, "Failed to get thread handle for main thread");

	profiler_thread_exit = 0;

	/* start the thread */
	profiler_thread = CreateThread(NULL, 0, profiler_thread_entry, (LPVOID)currentThread, 0, &profiler_thread_id);
	assert_always(profiler_thread, "Failed to create profiler thread\n");

	/* max out the priority */
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
