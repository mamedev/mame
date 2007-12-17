//============================================================
//
//  video.c - Win32 video handling
//
//  Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

// needed for multimonitor
#define _WIN32_WINNT 0x501

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// standard C headers
#include <math.h>

// Windows 95/NT4 multimonitor stubs
#ifdef WIN95_MULTIMON
#include "multidef.h"
#endif

// MAME headers
#include "osdepend.h"
#include "driver.h"
#include "profiler.h"
#include "video/vector.h"
#include "render.h"
#include "rendutil.h"
#include "ui.h"

// MAMEOS headers
#include "winmain.h"
#include "video.h"
#include "window.h"
#include "input.h"
#include "debugwin.h"
#include "strconv.h"
#include "config.h"

#ifdef MESS
#include "menu.h"
#endif


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

static mame_bitmap *effect_bitmap;



//============================================================
//  PROTOTYPES
//============================================================

static void video_exit(running_machine *machine);
static void init_monitors(void);
static BOOL CALLBACK monitor_enum_callback(HMONITOR handle, HDC dc, LPRECT rect, LPARAM data);
static win_monitor_info *pick_monitor(int index);

static void check_osd_inputs(void);

static void extract_video_config(void);
static void load_effect_overlay(const char *filename);
static float get_aspect(const char *name, int report_error);
static void get_resolution(const char *name, win_window_config *config, int report_error);



//============================================================
//  winvideo_init
//============================================================

void winvideo_init(running_machine *machine)
{
	int index;

	// ensure we get called on the way out
	add_exit_callback(machine, video_exit);

	// extract data from the options
	extract_video_config();

	// set up monitors first
	init_monitors();

	// initialize the window system so we can make windows
	winwindow_init(machine);

	// create the windows
	for (index = 0; index < video_config.numscreens; index++)
		winwindow_video_window_create(index, pick_monitor(index), &video_config.window[index]);
	if (video_config.mode != VIDEO_MODE_NONE)
		SetForegroundWindow(win_window_list->hwnd);

	// possibly create the debug window, but don't show it yet
#ifdef MAME_DEBUG
	if (options_get_bool(mame_options(), OPTION_DEBUG))
		debugwin_init_windows();
#endif
}


//============================================================
//  video_exit
//============================================================

static void video_exit(running_machine *machine)
{
	// free the overlay effect
	if (effect_bitmap != NULL)
		bitmap_free(effect_bitmap);
	effect_bitmap = NULL;

	// possibly kill the debug window
#ifdef MAME_DEBUG
	if (options_get_bool(mame_options(), OPTION_DEBUG))
		debugwin_destroy_windows();
#endif

	// free all of our monitor information
	while (win_monitor_list != NULL)
	{
		win_monitor_info *temp = win_monitor_list;
		win_monitor_list = temp->next;
		free(temp);
	}
}



//============================================================
//  winvideo_monitor_refresh
//============================================================

void winvideo_monitor_refresh(win_monitor_info *monitor)
{
	HRESULT result;

	// fetch the latest info about the monitor
	monitor->info.cbSize = sizeof(monitor->info);
	result = GetMonitorInfo(monitor->handle, (LPMONITORINFO)&monitor->info);
	assert(result);
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
//  osd_update
//============================================================

void osd_update(int skip_redraw)
{
	win_window_info *window;

	// if we're not skipping this redraw, update all windows
	if (!skip_redraw)
		for (window = win_window_list; window != NULL; window = window->next)
			winwindow_video_window_update(window);

	// poll the joystick values here
	winwindow_process_events(TRUE);
	wininput_poll();
	check_osd_inputs();
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
				free(utf8_device);
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

	// allocate a new monitor info
	monitor = malloc_or_die(sizeof(*monitor));
	memset(monitor, 0, sizeof(*monitor));

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

static win_monitor_info *pick_monitor(int index)
{
	const char *scrname, *scrname2;
	win_monitor_info *monitor;
	int moncount = 0;
	char option[20];
	float aspect;

	// get the screen option
	scrname = options_get_string(mame_options(), WINOPTION_SCREEN);
	sprintf(option, "screen%d", index);
	scrname2 = options_get_string(mame_options(), option);

	// decide which one we want to use
	if (strcmp(scrname2, "auto") != 0)
		scrname = scrname2;

	// get the aspect ratio
	sprintf(option, "aspect%d", index);
	aspect = get_aspect(option, TRUE);

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
				free(utf8_device);
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

static void check_osd_inputs(void)
{
	// check for toggling fullscreen mode
	if (input_ui_pressed(IPT_OSD_1))
		winwindow_toggle_full_screen();

#ifdef MESS
	// check for toggling menu bar
	if (input_ui_pressed(IPT_OSD_2))
		win_toggle_menubar();
#endif
}



//============================================================
//  extract_video_config
//============================================================

static void extract_video_config(void)
{
	const char *stemp;

	// global options: extract the data
	video_config.windowed      = options_get_bool(mame_options(), WINOPTION_WINDOW);
	video_config.prescale      = options_get_int(mame_options(), WINOPTION_PRESCALE);
	video_config.keepaspect    = options_get_bool(mame_options(), WINOPTION_KEEPASPECT);
	video_config.numscreens    = options_get_int(mame_options(), WINOPTION_NUMSCREENS);
#ifdef MAME_DEBUG
	// if we are in debug mode, never go full screen
	if (options_get_bool(mame_options(), OPTION_DEBUG))
		video_config.windowed = TRUE;
#endif
	stemp                      = options_get_string(mame_options(), WINOPTION_EFFECT);
	if (strcmp(stemp, "none") != 0)
		load_effect_overlay(stemp);

	// per-window options: extract the data
	get_resolution(WINOPTION_RESOLUTION0, &video_config.window[0], TRUE);
	get_resolution(WINOPTION_RESOLUTION1, &video_config.window[1], TRUE);
	get_resolution(WINOPTION_RESOLUTION2, &video_config.window[2], TRUE);
	get_resolution(WINOPTION_RESOLUTION3, &video_config.window[3], TRUE);

	// video options: extract the data
	stemp = options_get_string(mame_options(), WINOPTION_VIDEO);
	if (strcmp(stemp, "d3d") == 0)
		video_config.mode = VIDEO_MODE_D3D;
	else if (strcmp(stemp, "ddraw") == 0)
		video_config.mode = VIDEO_MODE_DDRAW;
	else if (strcmp(stemp, "gdi") == 0)
		video_config.mode = VIDEO_MODE_GDI;
	else if (strcmp(stemp, "none") == 0)
	{
		video_config.mode = VIDEO_MODE_NONE;
		if (options_get_int(mame_options(), OPTION_SECONDS_TO_RUN) == 0)
			mame_printf_warning("Warning: -video none doesn't make much sense without -seconds_to_run\n");
	}
	else
	{
		mame_printf_warning("Invalid video value %s; reverting to gdi\n", stemp);
		video_config.mode = VIDEO_MODE_GDI;
	}
	video_config.waitvsync     = options_get_bool(mame_options(), WINOPTION_WAITVSYNC);
	video_config.syncrefresh   = options_get_bool(mame_options(), WINOPTION_SYNCREFRESH);
	video_config.triplebuf     = options_get_bool(mame_options(), WINOPTION_TRIPLEBUFFER);
	video_config.switchres     = options_get_bool(mame_options(), WINOPTION_SWITCHRES);

	// ddraw options: extract the data
	video_config.hwstretch     = options_get_bool(mame_options(), WINOPTION_HWSTRETCH);

	// d3d options: extract the data
	video_config.filter        = options_get_bool(mame_options(), WINOPTION_FILTER);
	if (video_config.prescale == 0)
		video_config.prescale = 1;

	// misc options: sanity check values

	// per-window options: sanity check values

	// d3d options: sanity check values
	options_get_int(mame_options(), WINOPTION_D3DVERSION);

	options_get_float(mame_options(), WINOPTION_FULLSCREENBRIGHTNESS);
	options_get_float(mame_options(), WINOPTION_FULLLSCREENCONTRAST);
	options_get_float(mame_options(), WINOPTION_FULLSCREENGAMMA);
}



//============================================================
//  load_effect_overlay
//============================================================

static void load_effect_overlay(const char *filename)
{
	char *tempstr = malloc_or_die(strlen(filename) + 5);
	int scrnum;
	char *dest;

	// append a .PNG extension
	strcpy(tempstr, filename);
	dest = strrchr(tempstr, '.');
	if (dest == NULL)
		dest = &tempstr[strlen(tempstr)];
	strcpy(dest, ".png");

	// load the file
	effect_bitmap = render_load_png(NULL, tempstr, NULL, NULL);
	if (effect_bitmap == NULL)
	{
		mame_printf_error("Unable to load PNG file '%s'\n", tempstr);
		free(tempstr);
		return;
	}

	// set the overlay on all screens
	for (scrnum = 0; scrnum < MAX_SCREENS; scrnum++)
		if (Machine->drv->screen[scrnum].tag != NULL)
			render_container_set_overlay(render_container_get_screen(scrnum), effect_bitmap);

	free(tempstr);
}



//============================================================
//  get_aspect
//============================================================

static float get_aspect(const char *name, int report_error)
{
	const char *defdata = options_get_string(mame_options(), WINOPTION_ASPECT);
	const char *data = options_get_string(mame_options(), name);
	int num = 0, den = 1;

	if (strcmp(data, "auto") == 0)
	{
		if (strcmp(defdata, "auto") == 0)
			return 0;
		data = defdata;
	}
	if (sscanf(data, "%d:%d", &num, &den) != 2 && report_error)
		mame_printf_error("Illegal aspect ratio value for %s = %s\n", name, data);
	return (float)num / (float)den;
}



//============================================================
//  get_resolution
//============================================================

static void get_resolution(const char *name, win_window_config *config, int report_error)
{
	const char *defdata = options_get_string(mame_options(), WINOPTION_RESOLUTION);
	const char *data = options_get_string(mame_options(), name);

	config->width = config->height = config->refresh = 0;
	if (strcmp(data, "auto") == 0)
	{
		if (strcmp(defdata, "auto") == 0)
			return;
		data = defdata;
	}
	if (sscanf(data, "%dx%d@%d", &config->width, &config->height, &config->refresh) < 2 && report_error)
		mame_printf_error("Illegal resolution value for %s = %s\n", name, data);
}
