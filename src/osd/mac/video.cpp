// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  video.cpp - Mac video handling
//
//  Mac OSD by R. Belmont
//
//============================================================

// MAME headers
#include "emu.h"
#include "rendutil.h"
#include "ui/uimain.h"
#include "emuopts.h"
#include "uiinput.h"


// MAMEOS headers
#include "video.h"
#include "window.h"
#include "osdmac.h"
#include "modules/lib/osdlib.h"
#include "modules/monitor/monitor_module.h"

extern void MacPollInputs(); // in windowcontroller.mm

//============================================================
//  CONSTANTS
//============================================================


//============================================================
//  GLOBAL VARIABLES
//============================================================

osd_video_config video_config;

//============================================================
//  LOCAL VARIABLES
//============================================================


//============================================================
//  PROTOTYPES
//============================================================
static void get_resolution(const char *defdata, const char *data, osd_window_config *config, int report_error);


//============================================================
//  video_init
//============================================================

bool mac_osd_interface::video_init()
{
	int index;

	// extract data from the options
	extract_video_config();

	// we need the beam width in a float, contrary to what the core does.
	video_config.beamwidth = options().beam_width_min();

	// initialize the window system so we can make windows
	if (!window_init())
		return false;

	// create the windows
	for (index = 0; index < video_config.numscreens; index++)
	{
		osd_window_config conf;
		memset(&conf, 0, sizeof(conf));
		get_resolution(options().resolution(), options().resolution(index), &conf, true);

		// create window ...
		auto win = std::make_unique<mac_window_info>(machine(), *m_render, index, m_monitor_module->pick_monitor(reinterpret_cast<osd_options &>(options()), index), &conf);

		if (win->window_init())
			return false;

		s_window_list.emplace_back(std::move(win));
	}

	return true;
}

//============================================================
//  video_exit
//============================================================

void mac_osd_interface::video_exit()
{
	window_exit();
}

//============================================================
//  update
//============================================================

void mac_osd_interface::update(bool skip_redraw)
{
	osd_common_t::update(skip_redraw);

	// if we're not skipping this redraw, update all windows
	if (!skip_redraw)
	{
//      profiler_mark(PROFILER_BLIT);
		for (auto const &window : osd_common_t::window_list())
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

void mac_osd_interface::input_update(bool relative_reset)
{
	// poll the joystick values here
	process_events_buf();
	MacPollInputs();
	poll_input_modules(relative_reset);
}

//============================================================
//  check_osd_inputs
//============================================================

void mac_osd_interface::check_osd_inputs()
{
	// check for toggling fullscreen mode (don't do this in debug mode)
	if (machine().ui_input().pressed(IPT_OSD_1) && !(machine().debug_flags & DEBUG_FLAG_OSD_ENABLED))
	{
		// destroy the renderers first so that the render module can bounce if it depends on having a window handle
		for (auto it = window_list().rbegin(); window_list().rend() != it; ++it)
			(*it)->renderer_reset();
		for (auto const &curwin : window_list())
			dynamic_cast<mac_window_info &>(*curwin).toggle_full_screen();
	}

	auto const &window = window_list().front();

	//FIXME: on a per window basis
	if (machine().ui_input().pressed(IPT_OSD_5))
	{
		video_config.filter = !video_config.filter;
		machine().ui().popup_time(1, "Filter %s", video_config.filter? "enabled":"disabled");
	}

	if (machine().ui_input().pressed(IPT_OSD_6))
		dynamic_cast<mac_window_info &>(*window).modify_prescale(-1);

	if (machine().ui_input().pressed(IPT_OSD_7))
		dynamic_cast<mac_window_info &>(*window).modify_prescale(1);

	if (machine().ui_input().pressed(IPT_OSD_8))
		window->renderer().record();
}

//============================================================
//  extract_video_config
//============================================================

void mac_osd_interface::extract_video_config()
{
	// global options: extract the data
	video_config.windowed      = options().window();
	video_config.prescale      = options().prescale();
	video_config.filter        = options().filter();
	video_config.numscreens    = options().numscreens();

	// if we are in debug mode, never go full screen
	if (machine().debug_flags & DEBUG_FLAG_OSD_ENABLED)
		video_config.windowed = true;

	video_config.switchres     = options().switch_res();
	video_config.waitvsync     = options().wait_vsync();
	video_config.syncrefresh   = options().sync_refresh();
	if (!video_config.waitvsync && video_config.syncrefresh)
	{
		osd_printf_warning("-syncrefresh specified without -waitvsync. Reverting to -nosyncrefresh\n");
		video_config.syncrefresh = 0;
	}

	if (video_config.prescale < 1 || video_config.prescale > 8)
	{
		osd_printf_warning("Invalid prescale option, reverting to '1'\n");
		video_config.prescale = 1;
	}

	// misc options: sanity check values

	// global options: sanity check values
	if (video_config.numscreens < 1 || video_config.numscreens > MAX_VIDEO_WINDOWS)
	{
		osd_printf_warning("Invalid numscreens value %d; reverting to 1\n", video_config.numscreens);
		video_config.numscreens = 1;
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
