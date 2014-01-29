// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  video.c - Win32 video handling
//
//============================================================

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// Windows 95/NT4 multimonitor stubs
#ifdef WIN95_MULTIMON
#include "multidef.h"
#endif

// MAME headers
#include "emu.h"
#include "emuopts.h"
#include "osdepend.h"
#include "video/vector.h"
#include "render.h"
#include "rendutil.h"
#include "ui/ui.h"
#include "uiinput.h"

// MAMEOS headers
#include "winmain.h"
#include "video.h"
#include "window.h"
#include "input.h"
#include "debugwin.h"
#include "strconv.h"
#include "config.h"

//============================================================
//  GLOBAL VARIABLES
//============================================================

win_video_config video_config;



//============================================================
//  LOCAL VARIABLES
//============================================================

// monitor info
win_monitor_info *win_monitor_list;
static win_monitor_info *primary_monitor;



//============================================================
//  PROTOTYPES
//============================================================

static void winvideo_exit(running_machine &machine);
static void init_monitors(void);
static BOOL CALLBACK monitor_enum_callback(HMONITOR handle, HDC dc, LPRECT rect, LPARAM data);
static win_monitor_info *pick_monitor(windows_options &options, int index);

static void check_osd_inputs(running_machine &machine);

static void extract_video_config(running_machine &machine);
static float get_aspect(const char *defdata, const char *data, int report_error);
static void get_resolution(const char *defdata, const char *data, win_window_config *config, int report_error);



//============================================================
//  winvideo_init
//============================================================

void winvideo_init(running_machine &machine)
{
	int index;

	// ensure we get called on the way out
	machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(winvideo_exit), &machine));

	// extract data from the options
	extract_video_config(machine);

	// set up monitors first
	init_monitors();

	// initialize the window system so we can make windows
	winwindow_init(machine);

	// create the windows
	windows_options &options = downcast<windows_options &>(machine.options());
	for (index = 0; index < video_config.numscreens; index++)
		winwindow_video_window_create(machine, index, pick_monitor(options, index), &video_config.window[index]);
	if (video_config.mode != VIDEO_MODE_NONE)
		SetForegroundWindow(win_window_list->hwnd);

	// possibly create the debug window, but don't show it yet
	if (machine.debug_flags & DEBUG_FLAG_OSD_ENABLED)
		machine.osd().init_debugger();
}


//============================================================
//  winvideo_exit
//============================================================

static void winvideo_exit(running_machine &machine)
{
	// free all of our monitor information
	while (win_monitor_list != NULL)
	{
		win_monitor_info *temp = win_monitor_list;
		win_monitor_list = temp->next;
		global_free(temp);
	}
}



//============================================================
//  winvideo_monitor_refresh
//============================================================

void winvideo_monitor_refresh(win_monitor_info *monitor)
{
	BOOL result;

	// fetch the latest info about the monitor
	monitor->info.cbSize = sizeof(monitor->info);
	result = GetMonitorInfo(monitor->handle, (LPMONITORINFO)&monitor->info);
	assert(result);
	(void)result; // to silence gcc 4.6
}



//============================================================
//  winvideo_monitor_get_aspect
//============================================================

float winvideo_monitor_get_aspect(win_monitor_info *monitor)
{
	// refresh the monitor information and compute the aspect
	if (video_config.keepaspect)
	{
		int width, height;
		winvideo_monitor_refresh(monitor);
		width = rect_width(&monitor->info.rcMonitor);
		height = rect_height(&monitor->info.rcMonitor);
		return monitor->aspect / ((float)width / (float)height);
	}
	return 0.0f;
}



//============================================================
//  winvideo_monitor_from_handle
//============================================================

win_monitor_info *winvideo_monitor_from_handle(HMONITOR hmonitor)
{
	win_monitor_info *monitor;

	// find the matching monitor
	for (monitor = win_monitor_list; monitor != NULL; monitor = monitor->next)
		if (monitor->handle == hmonitor)
			return monitor;
	return NULL;
}



//============================================================
//  update
//============================================================

void windows_osd_interface::update(bool skip_redraw)
{
	// ping the watchdog on each update
	winmain_watchdog_ping();

	// if we're not skipping this redraw, update all windows
	if (!skip_redraw)
		for (win_window_info *window = win_window_list; window != NULL; window = window->next)
			winwindow_video_window_update(window);

	// poll the joystick values here
	winwindow_process_events(machine(), TRUE, FALSE);
	wininput_poll(machine());
	check_osd_inputs(machine());
}



//============================================================
//  init_monitors
//============================================================

static void init_monitors(void)
{
	win_monitor_info **tailptr;

	// make a list of monitors
	win_monitor_list = NULL;
	tailptr = &win_monitor_list;
	EnumDisplayMonitors(NULL, NULL, monitor_enum_callback, (LPARAM)&tailptr);

	// if we're verbose, print the list of monitors
	{
		win_monitor_info *monitor;
		for (monitor = win_monitor_list; monitor != NULL; monitor = monitor->next)
		{
			char *utf8_device = utf8_from_tstring(monitor->info.szDevice);
			if (utf8_device != NULL)
			{
				mame_printf_verbose("Video: Monitor %p = \"%s\" %s\n", monitor->handle, utf8_device, (monitor == primary_monitor) ? "(primary)" : "");
				osd_free(utf8_device);
			}
		}
	}
}



//============================================================
//  monitor_enum_callback
//============================================================

static BOOL CALLBACK monitor_enum_callback(HMONITOR handle, HDC dc, LPRECT rect, LPARAM data)
{
	win_monitor_info ***tailptr = (win_monitor_info ***)data;
	win_monitor_info *monitor;
	MONITORINFOEX info;
	BOOL result;

	// get the monitor info
	info.cbSize = sizeof(info);
	result = GetMonitorInfo(handle, (LPMONITORINFO)&info);
	assert(result);
	(void)result; // to silence gcc 4.6

	// allocate a new monitor info
	monitor = global_alloc_clear(win_monitor_info);

	// copy in the data
	monitor->handle = handle;
	monitor->info = info;

	// guess the aspect ratio assuming square pixels
	monitor->aspect = (float)(info.rcMonitor.right - info.rcMonitor.left) / (float)(info.rcMonitor.bottom - info.rcMonitor.top);

	// save the primary monitor handle
	if (monitor->info.dwFlags & MONITORINFOF_PRIMARY)
		primary_monitor = monitor;

	// hook us into the list
	**tailptr = monitor;
	*tailptr = &monitor->next;

	// enumerate all the available monitors so to list their names in verbose mode
	return TRUE;
}



//============================================================
//  pick_monitor
//============================================================

static win_monitor_info *pick_monitor(windows_options &options, int index)
{
	const char *scrname, *scrname2;
	win_monitor_info *monitor;
	int moncount = 0;
	float aspect;

	// get the screen option
	scrname = options.screen();
	scrname2 = options.screen(index);

	// decide which one we want to use
	if (strcmp(scrname2, "auto") != 0)
		scrname = scrname2;

	// get the aspect ratio
	aspect = get_aspect(options.aspect(), options.aspect(index), TRUE);

	// look for a match in the name first
	if (scrname[0] != 0)
		for (monitor = win_monitor_list; monitor != NULL; monitor = monitor->next)
		{
			char *utf8_device;
			int rc = 1;

			moncount++;

			utf8_device = utf8_from_tstring(monitor->info.szDevice);
			if (utf8_device != NULL)
			{
				rc = strcmp(scrname, utf8_device);
				osd_free(utf8_device);
			}
			if (rc == 0)
				goto finishit;
		}

	// didn't find it; alternate monitors until we hit the jackpot
	index %= moncount;
	for (monitor = win_monitor_list; monitor != NULL; monitor = monitor->next)
		if (index-- == 0)
			goto finishit;

	// return the primary just in case all else fails
	monitor = primary_monitor;

finishit:
	if (aspect != 0)
		monitor->aspect = aspect;
	return monitor;
}



//============================================================
//  check_osd_inputs
//============================================================

static void check_osd_inputs(running_machine &machine)
{
	// check for toggling fullscreen mode
	if (ui_input_pressed(machine, IPT_OSD_1))
		winwindow_toggle_full_screen();

	// check for taking fullscreen snap
	if (ui_input_pressed(machine, IPT_OSD_2))
		winwindow_take_snap();

	// check for taking fullscreen video
	if (ui_input_pressed(machine, IPT_OSD_3))
		winwindow_take_video();

	// check for taking fullscreen video
	if (ui_input_pressed(machine, IPT_OSD_4))
		winwindow_toggle_fsfx();
}



//============================================================
//  extract_video_config
//============================================================

static void extract_video_config(running_machine &machine)
{
	windows_options &options = downcast<windows_options &>(machine.options());
	const char *stemp;

	// global options: extract the data
	video_config.windowed      = options.window();
	video_config.prescale      = options.prescale();
	video_config.keepaspect    = options.keep_aspect();
	video_config.numscreens    = options.numscreens();

	// if we are in debug mode, never go full screen
	if (machine.debug_flags & DEBUG_FLAG_OSD_ENABLED)
		video_config.windowed = TRUE;

	// per-window options: extract the data
	const char *default_resolution = options.resolution();
	get_resolution(default_resolution, options.resolution(0), &video_config.window[0], TRUE);
	get_resolution(default_resolution, options.resolution(1), &video_config.window[1], TRUE);
	get_resolution(default_resolution, options.resolution(2), &video_config.window[2], TRUE);
	get_resolution(default_resolution, options.resolution(3), &video_config.window[3], TRUE);

	// video options: extract the data
	stemp = options.video();
	if (strcmp(stemp, "d3d") == 0)
		video_config.mode = VIDEO_MODE_D3D;
	else if (strcmp(stemp, "ddraw") == 0)
		video_config.mode = VIDEO_MODE_DDRAW;
	else if (strcmp(stemp, "gdi") == 0)
		video_config.mode = VIDEO_MODE_GDI;
	else if (strcmp(stemp, "none") == 0)
	{
		video_config.mode = VIDEO_MODE_NONE;
		if (options.seconds_to_run() == 0)
			mame_printf_warning("Warning: -video none doesn't make much sense without -seconds_to_run\n");
	}
	else
	{
		mame_printf_warning("Invalid video value %s; reverting to gdi\n", stemp);
		video_config.mode = VIDEO_MODE_GDI;
	}
	video_config.waitvsync     = options.wait_vsync();
	video_config.syncrefresh   = options.sync_refresh();
	video_config.triplebuf     = options.triple_buffer();
	video_config.switchres     = options.switch_res();

	// ddraw options: extract the data
	video_config.hwstretch     = options.hwstretch();

	// d3d options: extract the data
	video_config.filter        = options.filter();
	if (video_config.prescale == 0)
		video_config.prescale = 1;
}



//============================================================
//  get_aspect
//============================================================

static float get_aspect(const char *defdata, const char *data, int report_error)
{
	int num = 0, den = 1;

	if (strcmp(data, "auto") == 0)
	{
		if (strcmp(defdata, "auto") == 0)
			return 0;
		data = defdata;
	}
	if (sscanf(data, "%d:%d", &num, &den) != 2 && report_error)
		mame_printf_error("Illegal aspect ratio value = %s\n", data);
	return (float)num / (float)den;
}



//============================================================
//  get_resolution
//============================================================

static void get_resolution(const char *defdata, const char *data, win_window_config *config, int report_error)
{
	config->width = config->height = config->refresh = 0;
	if (strcmp(data, "auto") == 0)
	{
		if (strcmp(defdata, "auto") == 0)
			return;
		data = defdata;
	}
	if (sscanf(data, "%dx%d@%d", &config->width, &config->height, &config->refresh) < 2 && report_error)
		mame_printf_error("Illegal resolution value = %s\n", data);
}
