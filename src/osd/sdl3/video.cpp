// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  video.cpp - SDL video handling
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#include "window.h"

// MAMEOS headers
#include "osdsdl.h"
#include "modules/lib/osdlib.h"
#include "modules/monitor/monitor_module.h"
#include "modules/render/render_module.h"

// MAME headers
#include "emu.h"
#include "emuopts.h"
#include "main.h"
#include "rendutil.h"
#include "uiinput.h"

#include <SDL3/SDL.h>


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

bool sdl_osd_interface::video_init()
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
		get_resolution(options().resolution(), options().resolution(index), &conf, true);

		// create window ...
		auto win = std::make_unique<sdl_window_info>(machine(), *m_render, index, m_monitor_module->pick_monitor(reinterpret_cast<osd_options &>(options()), index), &conf);
		if (win->window_init())
			return false;

		s_window_list.emplace_back(std::move(win));
	}

	if (m_render->is_interactive())
		SDL_RaiseWindow(dynamic_cast<sdl_window_info &>(*osd_common_t::s_window_list.front()).platform_window());

	return true;
}

//============================================================
//  video_exit
//============================================================

void sdl_osd_interface::video_exit()
{
	window_exit();
}

//============================================================
//  update
//============================================================

void sdl_osd_interface::update(bool skip_redraw)
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
//  extract_video_config
//============================================================

void sdl_osd_interface::extract_video_config()
{
	video_config.perftest    = options().video_fps();

	// global options: extract the data
	video_config.windowed      = options().window();
	video_config.prescale      = options().prescale();
	video_config.filter        = options().filter();
	video_config.numscreens    = options().numscreens();
	#ifdef SDLMAME_X11
	video_config.restrictonemonitor = !options().use_all_heads();
	#endif

	// if we are in debug mode, never go full screen
	if (machine().debug_flags & DEBUG_FLAG_OSD_ENABLED)
		video_config.windowed = true;

	video_config.switchres     = options().switch_res();
	video_config.centerh       = options().centerh();
	video_config.centerv       = options().centerv();
	video_config.waitvsync     = options().wait_vsync();
	video_config.syncrefresh   = options().sync_refresh();
	if (!video_config.waitvsync && video_config.syncrefresh)
	{
		osd_printf_warning("-syncrefresh specified without -waitvsync. Reverting to -nosyncrefresh\n");
		video_config.syncrefresh = 0;
	}

	if (video_config.prescale < 1 || video_config.prescale > 20)
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
