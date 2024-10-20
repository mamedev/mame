// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  winmain.cpp - Win32 main program
//
//============================================================

#include "winmain.h"

#include "window.h"
#include "winopts.h"

// MAME headers
#include "emu.h"
#include "main.h"

// MAMEOS headers
#include "strconv.h"
#include "winutf8.h"
#include "winutil.h"
#include "winfile.h"
#include "modules/diagnostics/diagnostics_module.h"
#include "modules/lib/osdlib.h"
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
#include <objbase.h>
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
	virtual void output_callback(osd_output_channel channel, const util::format_argument_pack<char> &args) override
	{
		if (channel == OSD_OUTPUT_CHANNEL_ERROR)
		{
			// if we are in fullscreen mode, go to windowed mode
			if ((video_config.windowed == 0) && !osd_common_t::window_list().empty())
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

	{
		// Disable legacy mouse to pointer event translation - it's broken:
		// * No WM_POINTERLEAVE event when mouse pointer moves directly to an
		//   overlapping window from the same process.
		// * Still receive occasional WM_MOUSEMOVE events.
		OSD_DYNAMIC_API(user32, "User32.dll", "User32.dll");
		OSD_DYNAMIC_API_FN(user32, BOOL, WINAPI, EnableMouseInPointer, BOOL);
		if (OSD_DYNAMIC_API_TEST(EnableMouseInPointer))
			OSD_DYNAMIC_CALL(EnableMouseInPointer, FALSE);
	}

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
	, m_com_status(SUCCEEDED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED)))
	, m_last_event_check(std::chrono::steady_clock::time_point::min())
{
}


//============================================================
//  destructor
//============================================================

windows_osd_interface::~windows_osd_interface()
{
	if (m_com_status)
		CoUninitialize();
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
	for (const auto &info : osd_common_t::window_list())
	{
		machine.output().set_value(string_format("Orientation(%s)", info->monitor()->devicename()), dynamic_cast<win_window_info &>(*info).m_targetorient);
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
	process_events(false, false);
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
