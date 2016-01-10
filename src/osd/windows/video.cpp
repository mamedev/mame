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

// MAME headers
#include "emu.h"
#include "emuopts.h"
#include "render.h"
#include "uiinput.h"

// MAMEOS headers
#include "winmain.h"
#include "video.h"
#include "window.h"
#include "input.h"
#include "strconv.h"

//============================================================
//  CONSTANTS
//============================================================


//============================================================
//  GLOBAL VARIABLES
//============================================================

osd_video_config video_config;

// monitor info
osd_monitor_info *osd_monitor_info::list = NULL;


//============================================================
//  LOCAL VARIABLES
//============================================================


//============================================================
//  PROTOTYPES
//============================================================

static void init_monitors(void);

static void check_osd_inputs(running_machine &machine);

static float get_aspect(const char *defdata, const char *data, int report_error);
static void get_resolution(const char *defdata, const char *data, osd_window_config *config, int report_error);


//============================================================
//  video_init
//============================================================

// FIXME: Temporary workaround
static osd_window_config   windows[MAX_WINDOWS];        // configuration data per-window

bool windows_osd_interface::video_init()
{
	int index;

	// extract data from the options
	extract_video_config();

	// set up monitors first
	init_monitors();

	// initialize the window system so we can make windows
	window_init();

	// create the windows
	windows_options &options = downcast<windows_options &>(machine().options());
	for (index = 0; index < video_config.numscreens; index++)
		win_window_info::create(machine(), index, osd_monitor_info::pick_monitor(options, index), &windows[index]);
	if (video_config.mode != VIDEO_MODE_NONE)
		SetForegroundWindow(win_window_list->m_hwnd);

	return true;
}


//============================================================
//  video_exit
//============================================================

void windows_osd_interface::video_exit()
{
	window_exit();

	// free all of our monitor information
	while (osd_monitor_info::list != NULL)
	{
		osd_monitor_info *temp = osd_monitor_info::list;
		osd_monitor_info::list = temp->m_next;
		global_free(temp);
	}
}



win_monitor_info::win_monitor_info(const HMONITOR handle, const char *monitor_device, float aspect)
	: osd_monitor_info(&m_handle, monitor_device, aspect), m_handle(handle)
{
	win_monitor_info::refresh();
}

win_monitor_info::~win_monitor_info()
{
}

//============================================================
//  winvideo_monitor_refresh
//============================================================

void win_monitor_info::refresh()
{
	BOOL result;

	// fetch the latest info about the monitor
	m_info.cbSize = sizeof(m_info);
	result = GetMonitorInfo(m_handle, (LPMONITORINFO)&m_info);
	assert(result);
	char *temp = utf8_from_tstring(m_info.szDevice);

	if (temp) strncpy(m_name, temp, sizeof(m_name));

	osd_free(temp);

	m_pos_size = RECT_to_osd_rect(m_info.rcMonitor);
	m_usuable_pos_size = RECT_to_osd_rect(m_info.rcWork);
	m_is_primary = ((m_info.dwFlags & MONITORINFOF_PRIMARY) != 0);
	(void)result; // to silence gcc 4.6
}



//============================================================
//  sdlvideo_monitor_get_aspect
//============================================================

float osd_monitor_info::aspect()
{
	// FIXME: returning 0 looks odd, video_config is bad
	if (video_config.keepaspect)
	{
		return m_aspect / ((float)m_pos_size.width() / (float)m_pos_size.height());
	}
	return 0.0f;
}

//============================================================
//  winvideo_monitor_from_handle
//============================================================

osd_monitor_info *win_monitor_info::monitor_from_handle(HMONITOR hmonitor)
{
	osd_monitor_info *monitor;

	// find the matching monitor
	for (monitor = osd_monitor_info::list; monitor != NULL; monitor = monitor->m_next)
		if (*((HMONITOR *)monitor->oshandle()) == hmonitor)
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
	{
//      profiler_mark(PROFILER_BLIT);
		for (win_window_info *window = win_window_list; window != NULL; window = window->m_next)
			window->update();
//      profiler_mark(PROFILER_END);
	}

	// poll the joystick values here
	winwindow_process_events(machine(), TRUE, FALSE);
	wininput_poll(machine());
	check_osd_inputs(machine());
	// if we're running, disable some parts of the debugger
	if ((machine().debug_flags & DEBUG_FLAG_OSD_ENABLED) != 0)
		debugger_update();
}





//============================================================
//  monitor_enum_callback
//============================================================

BOOL CALLBACK win_monitor_info::monitor_enum_callback(HMONITOR handle, HDC dc, LPRECT rect, LPARAM data)
{
	osd_monitor_info ***tailptr = (osd_monitor_info ***)data;
	osd_monitor_info *monitor;
	MONITORINFOEX info;
	BOOL result;

	// get the monitor info
	info.cbSize = sizeof(info);
	result = GetMonitorInfo(handle, (LPMONITORINFO)&info);
	assert(result);
	(void)result; // to silence gcc 4.6

	// guess the aspect ratio assuming square pixels
	float aspect = (float)(info.rcMonitor.right - info.rcMonitor.left) / (float)(info.rcMonitor.bottom - info.rcMonitor.top);

	// allocate a new monitor info
	char *temp = utf8_from_tstring(info.szDevice);
	// copy in the data
	monitor = global_alloc(win_monitor_info(handle, temp, aspect));
	osd_free(temp);

	// hook us into the list
	**tailptr = monitor;
	*tailptr = &monitor->m_next;

	// enumerate all the available monitors so to list their names in verbose mode
	return TRUE;
}


//============================================================
//  init_monitors
//============================================================

static void init_monitors(void)
{
	osd_monitor_info **tailptr;

	// make a list of monitors
	osd_monitor_info::list = NULL;
	tailptr = &osd_monitor_info::list;
	EnumDisplayMonitors(NULL, NULL, win_monitor_info::monitor_enum_callback, (LPARAM)&tailptr);

	// if we're verbose, print the list of monitors
	{
		osd_monitor_info *monitor;
		for (monitor = osd_monitor_info::list; monitor != NULL; monitor = monitor->m_next)
		{
			osd_printf_verbose("Video: Monitor %p = \"%s\" %s\n", monitor->oshandle(), monitor->devicename(), monitor->is_primary() ? "(primary)" : "");
		}
	}
}


//============================================================
//  pick_monitor
//============================================================

osd_monitor_info *osd_monitor_info::pick_monitor(windows_options &options, int index)
{
	osd_monitor_info *monitor;
	const char *scrname, *scrname2;
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
	if (scrname != NULL && (scrname[0] != 0))
	{
		for (monitor = osd_monitor_info::list; monitor != NULL; monitor = monitor->next())
		{
			moncount++;
			if (strcmp(scrname, monitor->devicename()) == 0)
				goto finishit;
		}
	}

	// didn't find it; alternate monitors until we hit the jackpot
	index %= moncount;
	for (monitor = osd_monitor_info::list; monitor != NULL; monitor = monitor->next())
		if (index-- == 0)
			goto finishit;

	// return the primary just in case all else fails
	for (monitor = osd_monitor_info::list; monitor != NULL; monitor = monitor->next())
		if (monitor->is_primary())
			goto finishit;

	// FIXME: FatalError?
finishit:
	if (aspect != 0)
	{
		monitor->set_aspect(aspect);
	}
	return monitor;
}


//============================================================
//  check_osd_inputs
//============================================================

static void check_osd_inputs(running_machine &machine)
{
	// check for toggling fullscreen mode
	if (machine.ui_input().pressed(IPT_OSD_1))
		winwindow_toggle_full_screen();

	// check for taking fullscreen snap
	if (machine.ui_input().pressed(IPT_OSD_2))
		winwindow_take_snap();

	// check for taking fullscreen video
	if (machine.ui_input().pressed(IPT_OSD_3))
		winwindow_take_video();

	// check for taking fullscreen video
	if (machine.ui_input().pressed(IPT_OSD_4))
		winwindow_toggle_fsfx();
}



//============================================================
//  extract_video_config
//============================================================

void windows_osd_interface::extract_video_config()
{
	const char *stemp;

	// global options: extract the data
	video_config.windowed      = options().window();
	video_config.prescale      = options().prescale();
	video_config.filter        = options().filter();
	video_config.keepaspect    = options().keep_aspect();
	video_config.numscreens    = options().numscreens();

	// if we are in debug mode, never go full screen
	if (machine().debug_flags & DEBUG_FLAG_OSD_ENABLED)
		video_config.windowed = TRUE;

	// per-window options: extract the data
	const char *default_resolution = options().resolution();
	get_resolution(default_resolution, options().resolution(0), &windows[0], TRUE);
	get_resolution(default_resolution, options().resolution(1), &windows[1], TRUE);
	get_resolution(default_resolution, options().resolution(2), &windows[2], TRUE);
	get_resolution(default_resolution, options().resolution(3), &windows[3], TRUE);

	// video options: extract the data
	stemp = options().video();
	if (strcmp(stemp, "d3d") == 0)
		video_config.mode = VIDEO_MODE_D3D;
	else if (strcmp(stemp, "auto") == 0)
		video_config.mode = VIDEO_MODE_D3D;
	else if (strcmp(stemp, "ddraw") == 0)
		video_config.mode = VIDEO_MODE_DDRAW;
	else if (strcmp(stemp, "gdi") == 0)
		video_config.mode = VIDEO_MODE_GDI;
#if defined (USE_BGFX)
	else if (strcmp(stemp, "bgfx") == 0)
		video_config.mode = VIDEO_MODE_BGFX;
#endif
	else if (strcmp(stemp, "none") == 0)
	{
		video_config.mode = VIDEO_MODE_NONE;
		if (options().seconds_to_run() == 0)
			osd_printf_warning("Warning: -video none doesn't make much sense without -seconds_to_run\n");
	}
#if (USE_OPENGL)
	else if (strcmp(stemp, "opengl") == 0)
		video_config.mode = VIDEO_MODE_OPENGL;
#endif
	else
	{
		osd_printf_warning("Invalid video value %s; reverting to gdi\n", stemp);
		video_config.mode = VIDEO_MODE_GDI;
	}
	video_config.waitvsync     = options().wait_vsync();
	video_config.syncrefresh   = options().sync_refresh();
	video_config.triplebuf     = options().triple_buffer();
	video_config.switchres     = options().switch_res();

	// ddraw options: extract the data
	video_config.hwstretch     = options().hwstretch();

	if (video_config.prescale < 1 || video_config.prescale > 3)
	{
		osd_printf_warning("Invalid prescale option, reverting to '1'\n");
		video_config.prescale = 1;
	}
	#if (USE_OPENGL)
		// default to working video please
		video_config.forcepow2texture = options().gl_force_pow2_texture();
		video_config.allowtexturerect = !(options().gl_no_texture_rect());
		video_config.vbo         = options().gl_vbo();
		video_config.pbo         = options().gl_pbo();
		video_config.glsl        = options().gl_glsl();
		if ( video_config.glsl )
		{
			int i;

			video_config.glsl_filter = options().glsl_filter();

			video_config.glsl_shader_mamebm_num=0;

			for(i=0; i<GLSL_SHADER_MAX; i++)
			{
				stemp = options().shader_mame(i);
				if (stemp && strcmp(stemp, OSDOPTVAL_NONE) != 0 && strlen(stemp)>0)
				{
					video_config.glsl_shader_mamebm[i] = (char *) malloc(strlen(stemp)+1);
					strcpy(video_config.glsl_shader_mamebm[i], stemp);
					video_config.glsl_shader_mamebm_num++;
				} else {
					video_config.glsl_shader_mamebm[i] = NULL;
				}
			}

			video_config.glsl_shader_scrn_num=0;

			for(i=0; i<GLSL_SHADER_MAX; i++)
			{
				stemp = options().shader_screen(i);
				if (stemp && strcmp(stemp, OSDOPTVAL_NONE) != 0 && strlen(stemp)>0)
				{
					video_config.glsl_shader_scrn[i] = (char *) malloc(strlen(stemp)+1);
					strcpy(video_config.glsl_shader_scrn[i], stemp);
					video_config.glsl_shader_scrn_num++;
				} else {
					video_config.glsl_shader_scrn[i] = NULL;
				}
			}
		} else {
			int i;
			video_config.glsl_filter = 0;
			video_config.glsl_shader_mamebm_num=0;
			for(i=0; i<GLSL_SHADER_MAX; i++)
			{
				video_config.glsl_shader_mamebm[i] = NULL;
			}
			video_config.glsl_shader_scrn_num=0;
			for(i=0; i<GLSL_SHADER_MAX; i++)
			{
				video_config.glsl_shader_scrn[i] = NULL;
			}
		}

	#endif /* USE_OPENGL */

}



//============================================================
//  get_aspect
//============================================================

static float get_aspect(const char *defdata, const char *data, int report_error)
{
	int num = 0, den = 1;

	if (strcmp(data, OSDOPTVAL_AUTO) == 0)
	{
		if (strcmp(defdata,OSDOPTVAL_AUTO) == 0)
			return 0;
		data = defdata;
	}
	if (sscanf(data, "%d:%d", &num, &den) != 2 && report_error)
		osd_printf_error("Illegal aspect ratio value = %s\n", data);
	return (float)num / (float)den;
}


//============================================================
//  get_resolution
//============================================================

static void get_resolution(const char *defdata, const char *data, osd_window_config *config, int report_error)
{
	config->width = config->height = config->depth = config->refresh = 0;
	if (strcmp(data, OSDOPTVAL_AUTO) == 0)
	{
		if (strcmp(defdata, OSDOPTVAL_AUTO) == 0)
			return;
		data = defdata;
	}

	if (sscanf(data, "%dx%dx%d", &config->width, &config->height, &config->depth) < 2 && report_error)
		osd_printf_error("Illegal resolution value = %s\n", data);

	const char * at_pos = strchr(data, '@');
	if (at_pos)
		if (sscanf(at_pos + 1, "%d", &config->refresh) < 1 && report_error)
			osd_printf_error("Illegal refresh rate in resolution value = %s\n", data);
}
