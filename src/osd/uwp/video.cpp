// license:BSD-3-Clause
// copyright-holders:Brad Hughes, Aaron Giles
//============================================================
//
//  video.c - UWP video handling
//
//============================================================

#include <exception>

// standard windows headers
#include <windows.h>
#include <dxgi1_2.h>
#include <wrl\client.h>
#undef interface

// MAME headers
#include "emu.h"
#include "emuopts.h"
#include "render.h"
#include "uiinput.h"

// MAMEOS headers
#include "winmain.h"
#include "video.h"
#include "window.h"
#include "strconv.h"

#include "modules/osdwindow.h"

using namespace Platform;
using namespace Microsoft::WRL;
using namespace Windows::UI::Core;

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
	windows_options &options = downcast<windows_options &>(machine().options());
	for (int index = 0; index < video_config.numscreens; index++)
	{
		uwp_window_info::create(machine(), index, m_monitor_module->pick_monitor(options, index), &windows[index]);
	}

	if (video_config.mode != VIDEO_MODE_NONE)
	{
		auto win = std::static_pointer_cast<uwp_window_info>(osd_common_t::s_window_list.front());
		win->platform_window()->Activate();
	}

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
		for (auto window : osd_common_t::s_window_list)
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

void windows_osd_interface::input_update()
{
	// poll the joystick values here
	winwindow_process_events(machine(), true, false);
	poll_input(machine());
	check_osd_inputs();
}

//============================================================
//  check_osd_inputs
//============================================================

void windows_osd_interface::check_osd_inputs()
{
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
	if (strcmp(stemp, "auto") == 0)
		video_config.mode = VIDEO_MODE_BGFX;
	else if (strcmp(stemp, "bgfx") == 0)
		video_config.mode = VIDEO_MODE_BGFX;
	else if (strcmp(stemp, "none") == 0)
	{
		video_config.mode = VIDEO_MODE_NONE;
		if (!emulator_info::standalone() && options().seconds_to_run() == 0)
			osd_printf_warning("Warning: -video none doesn't make much sense without -seconds_to_run\n");
	}
	else
	{
		osd_printf_warning("Invalid video value %s; reverting to gdi\n", stemp);
		video_config.mode = VIDEO_MODE_GDI;
	}
	video_config.waitvsync     = options().wait_vsync();
	video_config.syncrefresh   = options().sync_refresh();
	video_config.triplebuf     = options().triple_buffer();
	video_config.switchres     = options().switch_res();

	if (video_config.prescale < 1 || video_config.prescale > 3)
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
