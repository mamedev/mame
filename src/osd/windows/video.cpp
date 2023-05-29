// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  video.cpp - Win32 video handling
//
//============================================================

#include "modules/osdwindow.h"
#include "modules/monitor/monitor_module.h"
#include "modules/render/render_module.h"

// MAME headers
#include "emu.h"
#include "emuopts.h"
#include "main.h"
#include "render.h"
#include "uiinput.h"

// MAMEOS headers
#include "winmain.h"
#include "window.h"
#include "strconv.h"

// standard windows headers
#include <windows.h>

//============================================================
//  CONSTANTS
//============================================================


//============================================================
//  GLOBAL VARIABLES
//============================================================

osd_video_config video_config;


//============================================================
//  PROTOTYPES
//============================================================

static void get_resolution(const char *defdata, const char *data, osd_window_config *config, int report_error);


//============================================================
//  video_init
//============================================================

// FIXME: Temporary workaround
static osd_window_config   windows[MAX_VIDEO_WINDOWS];        // configuration data per-window

bool windows_osd_interface::video_init()
{
	// extract data from the options
	extract_video_config();

	// initialize the window system so we can make windows
	window_init();

	// create the windows
	auto &options = downcast<windows_options &>(machine().options());
	for (int index = 0; index < video_config.numscreens; index++)
	{
		auto window = win_window_info::create(
				machine(),
				*m_render,
				index,
				m_monitor_module->pick_monitor(options, index),
				&windows[index]);
		s_window_list.emplace_back(std::move(window));
	}

	if (m_render->is_interactive())
		SetForegroundWindow(dynamic_cast<win_window_info &>(*osd_common_t::s_window_list.front()).platform_window());

	return true;
}

//============================================================
//  video_exit
//============================================================

void windows_osd_interface::video_exit()
{
	window_exit();
}

//============================================================
//  update
//============================================================

void windows_osd_interface::update(bool skip_redraw)
{
	osd_common_t::update(skip_redraw);

	// if we're not skipping this redraw, update all windows
	if (!skip_redraw)
	{
//      profiler_mark(PROFILER_BLIT);
		for (const auto &window : osd_common_t::s_window_list)
			window->update();
//      profiler_mark(PROFILER_END);
	}

	// if we're running, disable some parts of the debugger
	if ((machine().debug_flags & DEBUG_FLAG_OSD_ENABLED) != 0)
		debugger_update();
}


//============================================================
//  input_update
//============================================================

void windows_osd_interface::input_update(bool relative_reset)
{
	// poll the joystick values here
	process_events(true, false);
	poll_input_modules(relative_reset);
}

//============================================================
//  check_osd_inputs
//============================================================

void windows_osd_interface::check_osd_inputs()
{
	// check for toggling fullscreen mode
	if (machine().ui_input().pressed(IPT_OSD_1))
		winwindow_toggle_full_screen();

	// check for taking fullscreen snap
	if (machine().ui_input().pressed(IPT_OSD_2))
		winwindow_take_snap();

	// check for taking fullscreen video
	if (machine().ui_input().pressed(IPT_OSD_3))
		winwindow_take_video();

	// check for taking fullscreen video
	if (machine().ui_input().pressed(IPT_OSD_4))
		winwindow_toggle_fsfx();
}



//============================================================
//  extract_video_config
//============================================================

void windows_osd_interface::extract_video_config()
{
	// global options: extract the data
	video_config.windowed      = options().window();
	video_config.prescale      = options().prescale();
	video_config.filter        = options().filter();
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

	video_config.waitvsync     = options().wait_vsync();
	video_config.syncrefresh   = options().sync_refresh();
	video_config.triplebuf     = options().triple_buffer();
	video_config.switchres     = options().switch_res();

	if (video_config.prescale < 1 || video_config.prescale > 20)
	{
		osd_printf_warning("Invalid prescale option, reverting to '1'\n");
		video_config.prescale = 1;
	}
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
