//============================================================
//
//  video.c - SDL video handling
//
//  Copyright (c) 1996-2014, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#ifdef SDLMAME_X11
#include <X11/extensions/Xinerama.h>
#endif

#ifdef SDLMAME_MACOSX
#undef Status
#include <Carbon/Carbon.h>
#endif

#ifdef SDLMAME_WIN32
// for multimonitor
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x501
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "strconv.h"
#endif

#ifdef SDLMAME_OS2
#define INCL_WINSYS
#include <os2.h>
#endif

#include "sdlinc.h"

// MAME headers
#include "emu.h"
#include "rendutil.h"
#include "ui/ui.h"
#include "emuopts.h"
#include "uiinput.h"


// MAMEOS headers
#include "video.h"
#include "window.h"
#include "input.h"
#include "osdsdl.h"
#include "modules/lib/osdlib.h"

//============================================================
//  CONSTANTS
//============================================================


//============================================================
//  GLOBAL VARIABLES
//============================================================

sdl_video_config video_config;

#ifndef NO_OPENGL
#ifdef USE_DISPATCH_GL
osd_gl_dispatch *gl_dispatch;
#endif
#endif

sdl_monitor_info *sdl_monitor_info::primary_monitor = NULL;
sdl_monitor_info *sdl_monitor_info::list = NULL;

//============================================================
//  LOCAL VARIABLES
//============================================================

//============================================================
//  PROTOTYPES
//============================================================

static void check_osd_inputs(running_machine &machine);

static float get_aspect(const char *defdata, const char *data, int report_error);
static void get_resolution(const char *defdata, const char *data, sdl_window_config *config, int report_error);


//============================================================
//  video_init
//============================================================

bool sdl_osd_interface::video_init()
{
	int index;

	// extract data from the options
	extract_video_config(machine());

	// set up monitors first
	sdl_monitor_info::init();

	// we need the beam width in a float, contrary to what the core does.
	video_config.beamwidth = options().beam();

	// initialize the window system so we can make windows
	if (!window_init())
		return false;

	// create the windows
	for (index = 0; index < video_config.numscreens; index++)
	{
		sdl_window_config conf;
		memset(&conf, 0, sizeof(conf));
		get_resolution(options().resolution(), options().resolution(index), &conf, TRUE);

		// create window ...
		sdl_window_info *win = global_alloc(sdl_window_info(machine(), index, sdl_monitor_info::pick_monitor(options(), index), &conf));

		if (win->window_init())
			return false;
	}

	return true;
}


//============================================================
//  video_exit
//============================================================

void sdl_osd_interface::video_exit()
{
	window_exit();
	sdl_monitor_info::exit();
}


//============================================================
//  sdlvideo_monitor_refresh
//============================================================

void sdl_monitor_info::refresh()
{
	#if (SDLMAME_SDL2)
	SDL_DisplayMode dmode;

	#if defined(SDLMAME_WIN32)
	SDL_GetDesktopDisplayMode(m_handle, &dmode);
	#else
	SDL_GetCurrentDisplayMode(m_handle, &dmode);
	#endif
	SDL_GetDisplayBounds(m_handle, &m_dimensions);
	m_center_width = m_dimensions.w;
	m_center_height = m_dimensions.h;

	// FIXME: Use SDL_GetDisplayBounds(monitor->handle, &tt) to update monitor_x
	// SDL_Rect tt;
	#else
	#if defined(SDLMAME_WIN32)  // Win32 version
	MONITORINFOEX info;
	info.cbSize = sizeof(info);
	GetMonitorInfo((HMONITOR)m_handle, (LPMONITORINFO)&info);
	m_dimensions.x = m_dimensions.y = 0;
	m_center_width = m_dimensions.w = info.rcMonitor.right - info.rcMonitor.left;
	m_center_height = m_dimensions.h = info.rcMonitor.bottom - info.rcMonitor.top;
	char *temp = utf8_from_wstring(info.szDevice);
	strcpy(m_monitor_device, temp);
	osd_free(temp);
	#elif defined(SDLMAME_MACOSX)   // Mac OS X Core Imaging version
	CGDirectDisplayID primary;
	CGRect dbounds;

	// get the main display
	primary = CGMainDisplayID();
	dbounds = CGDisplayBounds(primary);

	m_dimensions.x = m_dimensions.y = 0;
	m_center_width = m_dimensions.w = dbounds.size.width - dbounds.origin.x;
	m_center_height = m_dimensions.h = dbounds.size.height - dbounds.origin.y;
	strcpy(m_monitor_device, "Mac OS X display");
	#elif defined(SDLMAME_X11) || defined(SDLMAME_NO_X11)       // X11 version
	{
		#if defined(SDLMAME_X11)
		// X11 version
		int screen;
		SDL_SysWMinfo info;
		SDL_VERSION(&info.version);

		if ( SDL_GetWMInfo(&info) && (info.subsystem == SDL_SYSWM_X11) )
		{
			screen = DefaultScreen(info.info.x11.display);
			SDL_VideoDriverName(m_monitor_device, sizeof(m_monitor_device)-1);
			m_dimensions.x = m_dimensions.y = 0;
			m_dimensions.w = DisplayWidth(info.info.x11.display, screen);
			m_dimensions.h = DisplayHeight(info.info.x11.display, screen);

			if ((XineramaIsActive(info.info.x11.display)) && video_config.restrictonemonitor)
			{
				XineramaScreenInfo *xineinfo;
				int numscreens;

					xineinfo = XineramaQueryScreens(info.info.x11.display, &numscreens);

				m_center_width = xineinfo[0].width;
				m_center_height = xineinfo[0].height;

				XFree(xineinfo);
			}
			else
			{
				m_center_width = m_dimensions.w;
				m_center_height = m_dimensions.h;
			}
		}
		else
		#endif // defined(SDLMAME_X11)
		{
			static int first_call=0;
			static int cw = 0, ch = 0;

			SDL_VideoDriverName(m_monitor_device, sizeof(m_monitor_device)-1);
			if (first_call==0)
			{
				const char *dimstr = osd_getenv(SDLENV_DESKTOPDIM);
				const SDL_VideoInfo *sdl_vi;

				sdl_vi = SDL_GetVideoInfo();
				#if (SDL_VERSION_ATLEAST(1,2,10))
				cw = sdl_vi->current_w;
				ch = sdl_vi->current_h;
				#endif
				first_call=1;
				if ((cw==0) || (ch==0))
				{
					if (dimstr != NULL)
					{
						sscanf(dimstr, "%dx%d", &cw, &ch);
					}
					if ((cw==0) || (ch==0))
					{
						osd_printf_warning("WARNING: SDL_GetVideoInfo() for driver <%s> is broken.\n", m_monitor_device);
						osd_printf_warning("         You should set SDLMAME_DESKTOPDIM to your desktop size.\n");
						osd_printf_warning("            e.g. export SDLMAME_DESKTOPDIM=800x600\n");
						osd_printf_warning("         Assuming 1024x768 now!\n");
						cw=1024;
						ch=768;
					}
				}
			}
			m_dimensions.w = cw;
			m_dimensions.h = ch;
			m_center_width = cw;
			m_center_height = ch;
		}
	}
	#elif defined(SDLMAME_OS2)      // OS2 version
	m_dimensions.x = m_dimensions.y = 0;
	m_center_width = m_dimensions.w = WinQuerySysValue( HWND_DESKTOP, SV_CXSCREEN );
	m_center_height = m_dimensions.h = WinQuerySysValue( HWND_DESKTOP, SV_CYSCREEN );
	strcpy(m_monitor_device, "OS/2 display");
	#else
	#error Unknown SDLMAME_xx OS type!
	#endif

	{
		static int info_shown=0;
		if (!info_shown)
		{
			osd_printf_verbose("SDL Device Driver     : %s\n", m_monitor_device);
			osd_printf_verbose("SDL Monitor Dimensions: %d x %d\n", m_dimensions.w, m_dimensions.h);
			info_shown = 1;
		}
	}
	#endif //  (SDLMAME_SDL2)
}



//============================================================
//  sdlvideo_monitor_get_aspect
//============================================================

float sdl_monitor_info::aspect()
{
	// refresh the monitor information and compute the aspect
	refresh();
	// FIXME: returning 0 looks odd, video_config is bad
	if (video_config.keepaspect)
	{
		return m_aspect / ((float)m_dimensions.w / (float)m_dimensions.h);
	}
	return 0.0f;
}




//============================================================
//  update
//============================================================

void sdl_osd_interface::update(bool skip_redraw)
{
	sdl_window_info *window;

	if (m_watchdog != NULL)
		m_watchdog->reset();

	// if we're not skipping this redraw, update all windows
	if (!skip_redraw)
	{
//      profiler_mark(PROFILER_BLIT);
		for (window = sdl_window_list; window != NULL; window = window->m_next)
			window->update();
//      profiler_mark(PROFILER_END);
	}

	// poll the joystick values here
	sdlinput_poll(machine());
	check_osd_inputs(machine());

	if ((machine().debug_flags & DEBUG_FLAG_OSD_ENABLED) != 0)
		debugger_update();
}


//============================================================
//  add_primary_monitor
//============================================================

#if !defined(SDLMAME_WIN32) && !(SDLMAME_SDL2)
void sdl_monitor_info::add_primary_monitor(void *data)
{
	sdl_monitor_info ***tailptr = (sdl_monitor_info ***)data;
	sdl_monitor_info *monitor;

	// allocate a new monitor info
	monitor = global_alloc_clear(sdl_monitor_info);

	// copy in the data
	monitor->m_handle = 1;

	monitor->refresh();

	// guess the aspect ratio assuming square pixels
	monitor->m_aspect = (float)(monitor->m_dimensions.w) / (float)(monitor->m_dimensions.h);

	// save the primary monitor handle
	primary_monitor = monitor;

	// hook us into the list
	**tailptr = monitor;
	*tailptr = &monitor->m_next;
}
#endif


//============================================================
//  monitor_enum_callback
//============================================================

#if defined(SDLMAME_WIN32) && !(SDLMAME_SDL2)
static BOOL CALLBACK monitor_enum_callback(HMONITOR handle, HDC dc, LPRECT rect, LPARAM data)
{
	sdl_monitor_info ***tailptr = (sdl_monitor_info ***)data;
	sdl_monitor_info *monitor;
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
	char *temp = utf8_from_wstring(info.szDevice);
	// copy in the data
	monitor = global_alloc(sdl_monitor_info((UINT64) handle, temp, aspect));
	osd_free(temp);

	// save the primary monitor handle
	if (info.dwFlags & MONITORINFOF_PRIMARY)
		sdl_monitor_info::primary_monitor = monitor;

	// hook us into the list
	**tailptr = monitor;
	*tailptr = &monitor->m_next;

	// enumerate all the available monitors so to list their names in verbose mode
	return TRUE;
}
#endif


//============================================================
//  init_monitors
//============================================================

void sdl_monitor_info::init()
{
	sdl_monitor_info **tailptr;

	// make a list of monitors
	sdl_monitor_info::list = NULL;
	tailptr = &sdl_monitor_info::list;

	#if (SDLMAME_SDL2)
	{
		int i, monx = 0;

		osd_printf_verbose("Enter init_monitors\n");

		for (i = 0; i < SDL_GetNumVideoDisplays(); i++)
		{
			sdl_monitor_info *monitor;
			SDL_DisplayMode dmode;

			// allocate a new monitor info
			monitor = global_alloc_clear(sdl_monitor_info);
			monitor->m_handle = i;

			snprintf(monitor->m_monitor_device, sizeof(monitor->m_monitor_device)-1, "%s%d", OSDOPTION_SCREEN,i);

			SDL_GetDesktopDisplayMode(i, &dmode);
			SDL_GetDisplayBounds(i, &monitor->m_dimensions);
			monitor->m_center_width = monitor->m_dimensions.w;
			monitor->m_center_height = monitor->m_dimensions.h;

			// guess the aspect ratio assuming square pixels
			monitor->m_aspect = (float)(dmode.w) / (float)(dmode.h);

			osd_printf_verbose("Adding monitor %s (%d x %d)\n", monitor->m_monitor_device, dmode.w, dmode.h);

			monx += dmode.w;

			// save the primary monitor handle
			if (i == 0)
				primary_monitor = monitor;

			// hook us into the list
			*tailptr = monitor;
			tailptr = &monitor->m_next;
		}
	}
	osd_printf_verbose("Leave init_monitors\n");
	#elif defined(SDLMAME_WIN32)
	EnumDisplayMonitors(NULL, NULL, monitor_enum_callback, (LPARAM)&tailptr);
	#else
	add_primary_monitor((void *)&tailptr);
	#endif
}

void sdl_monitor_info::exit()
{
	// free all of our monitor information
	while (sdl_monitor_info::list != NULL)
	{
		sdl_monitor_info *temp = sdl_monitor_info::list;
		sdl_monitor_info::list = temp->next();
		global_free(temp);
	}
}


//============================================================
//  pick_monitor
//============================================================

#if (SDLMAME_SDL2) || defined(SDLMAME_WIN32)
sdl_monitor_info *sdl_monitor_info::pick_monitor(sdl_options &options, int index)
{
	sdl_monitor_info *monitor;
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
	if (scrname != NULL)
	{
		for (monitor = sdl_monitor_info::list; monitor != NULL; monitor = monitor->next())
		{
			moncount++;
			if (strcmp(scrname, monitor->device()) == 0)
				goto finishit;
		}
	}

	// didn't find it; alternate monitors until we hit the jackpot
	index %= moncount;
	for (monitor = sdl_monitor_info::list; monitor != NULL; monitor = monitor->next())
		if (index-- == 0)
			goto finishit;

	// return the primary just in case all else fails
	monitor = primary_monitor;

finishit:
	if (aspect != 0)
	{
		monitor->set_aspect(aspect);
	}
	return monitor;
}
#else
sdl_monitor_info *sdl_monitor_info::pick_monitor(sdl_options &options, int index)
{
	sdl_monitor_info *monitor;
	float aspect;

	// get the aspect ratio
	aspect = get_aspect(options.aspect(), options.aspect(index), TRUE);

	// return the primary just in case all else fails
	monitor = primary_monitor;

	if (aspect != 0)
	{
		monitor->m_aspect = aspect;
	}
	return monitor;
}
#endif


//============================================================
//  check_osd_inputs
//============================================================

static void check_osd_inputs(running_machine &machine)
{
	sdl_window_info *window = sdlinput_get_focus_window();

	// check for toggling fullscreen mode
	if (ui_input_pressed(machine, IPT_OSD_1))
	{
		sdl_window_info *curwin = sdl_window_list;

		while (curwin != (sdl_window_info *)NULL)
		{
			curwin->toggle_full_screen();
			curwin = curwin->m_next;
		}
	}

	if (ui_input_pressed(machine, IPT_OSD_2))
	{
		//FIXME: on a per window basis
		video_config.fullstretch = !video_config.fullstretch;
		machine.ui().popup_time(1, "Uneven stretch %s", video_config.fullstretch? "enabled":"disabled");
	}

	if (ui_input_pressed(machine, IPT_OSD_4))
	{
		//FIXME: on a per window basis
		video_config.keepaspect = !video_config.keepaspect;
		machine.ui().popup_time(1, "Keepaspect %s", video_config.keepaspect? "enabled":"disabled");
	}

	#if (USE_OPENGL || SDLMAME_SDL2)
		//FIXME: on a per window basis
		if (ui_input_pressed(machine, IPT_OSD_5))
		{
			video_config.filter = !video_config.filter;
			machine.ui().popup_time(1, "Filter %s", video_config.filter? "enabled":"disabled");
		}
	#endif

	if (ui_input_pressed(machine, IPT_OSD_6))
		window->modify_prescale(-1);

	if (ui_input_pressed(machine, IPT_OSD_7))
		window->modify_prescale(1);
}

//============================================================
//  extract_video_config
//============================================================

void sdl_osd_interface::extract_video_config(running_machine &machine)
{
	const char *stemp;

	video_config.perftest    = options().video_fps();

	// global options: extract the data
	video_config.windowed      = options().window();
	video_config.keepaspect    = options().keep_aspect();
	video_config.numscreens    = options().numscreens();
	video_config.fullstretch   = options().uneven_stretch();
	#ifdef SDLMAME_X11
	video_config.restrictonemonitor = !options().use_all_heads();
	#endif


	if (machine.debug_flags & DEBUG_FLAG_OSD_ENABLED)
		video_config.windowed = TRUE;

	// default to working video please
	video_config.novideo = 0;

	// d3d options: extract the data
	stemp = options().video();
	if (strcmp(stemp, "auto") == 0)
	{
#if (defined SDLMAME_MACOSX || defined SDLMAME_WIN32)
		stemp = "opengl";
#else
		stemp = "soft";
#endif
	}
	if (strcmp(stemp, SDLOPTVAL_SOFT) == 0)
		video_config.mode = VIDEO_MODE_SOFT;
	else if (strcmp(stemp, SDLOPTVAL_NONE) == 0)
	{
		video_config.mode = VIDEO_MODE_SOFT;
		video_config.novideo = 1;

		if (options().seconds_to_run() == 0)
			osd_printf_warning("Warning: -video none doesn't make much sense without -seconds_to_run\n");
	}
	else if (USE_OPENGL && (strcmp(stemp, SDLOPTVAL_OPENGL) == 0))
		video_config.mode = VIDEO_MODE_OPENGL;
	else if (SDLMAME_SDL2 && (strcmp(stemp, SDLOPTVAL_SDL2ACCEL) == 0))
	{
		video_config.mode = VIDEO_MODE_SDL2ACCEL;
	}
#ifdef USE_BGFX
	else if (strcmp(stemp, SDLOPTVAL_BGFX) == 0)
	{
		video_config.mode = VIDEO_MODE_BGFX;
	}
#endif
	else
	{
		osd_printf_warning("Invalid video value %s; reverting to software\n", stemp);
		video_config.mode = VIDEO_MODE_SOFT;
	}

	video_config.switchres     = options().switch_res();
	video_config.centerh       = options().centerh();
	video_config.centerv       = options().centerv();
	video_config.waitvsync     = options().wait_vsync();
	video_config.syncrefresh   = options().sync_refresh();
	if (!video_config.waitvsync && video_config.syncrefresh)
	{
		osd_printf_warning("-syncrefresh specified without -waitsync. Reverting to -nosyncrefresh\n");
		video_config.syncrefresh = 0;
	}

	#if (USE_OPENGL || SDLMAME_SDL2)
		video_config.filter        = options().filter();
	#endif

	#if (USE_OPENGL)
		video_config.prescale      = options().prescale();
		if (video_config.prescale < 1 || video_config.prescale > 3)
		{
			osd_printf_warning("Invalid prescale option, reverting to '1'\n");
			video_config.prescale = 1;
		}
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
				if (stemp && strcmp(stemp, SDLOPTVAL_NONE) != 0 && strlen(stemp)>0)
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
				if (stemp && strcmp(stemp, SDLOPTVAL_NONE) != 0 && strlen(stemp)>0)
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
	// misc options: sanity check values

	// global options: sanity check values
#if (!SDLMAME_SDL2)
	if (video_config.numscreens < 1 || video_config.numscreens > 1) //MAX_VIDEO_WINDOWS)
	{
		osd_printf_warning("Invalid numscreens value %d; reverting to 1\n", video_config.numscreens);
		video_config.numscreens = 1;
	}
#else
	if (video_config.numscreens < 1 || video_config.numscreens > MAX_VIDEO_WINDOWS)
	{
		osd_printf_warning("Invalid numscreens value %d; reverting to 1\n", video_config.numscreens);
		video_config.numscreens = 1;
	}
#endif
	// yuv settings ...
	stemp = options().scale_mode();
	video_config.scale_mode = drawsdl_scale_mode(stemp);
	if (video_config.scale_mode < 0)
	{
		osd_printf_warning("Invalid yuvmode value %s; reverting to none\n", stemp);
		video_config.scale_mode = VIDEO_SCALE_MODE_NONE;
	}
	if ( (video_config.mode != VIDEO_MODE_SOFT) && (video_config.scale_mode != VIDEO_SCALE_MODE_NONE) )
	{
		osd_printf_warning("scalemode is only for -video soft, overriding\n");
		video_config.scale_mode = VIDEO_SCALE_MODE_NONE;
	}
}


//============================================================
//  get_aspect
//============================================================

static float get_aspect(const char *defdata, const char *data, int report_error)
{
	int num = 0, den = 1;

	if (strcmp(data, SDLOPTVAL_AUTO) == 0)
	{
		if (strcmp(defdata,SDLOPTVAL_AUTO) == 0)
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

static void get_resolution(const char *defdata, const char *data, sdl_window_config *config, int report_error)
{
	config->width = config->height = config->depth = config->refresh = 0;
	if (strcmp(data, SDLOPTVAL_AUTO) == 0)
	{
		if (strcmp(defdata, SDLOPTVAL_AUTO) == 0)
		{
			return;
		}

		data = defdata;
	}

	if (sscanf(data, "%dx%dx%d@%d", &config->width, &config->height, &config->depth, &config->refresh) < 2 && report_error)
		osd_printf_error("Illegal resolution value = %s\n", data);
}
