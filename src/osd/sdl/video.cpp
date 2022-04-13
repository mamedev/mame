// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  video.c - SDL video handling
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================
#include <SDL2/SDL.h>

// MAME headers
#include "emu.h"
#include "rendutil.h"
#include "ui/uimain.h"
#include "emuopts.h"
#include "uiinput.h"


// MAMEOS headers
#include "video.h"
#include "window.h"
#include "osdsdl.h"
#include "modules/lib/osdlib.h"

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

static void check_osd_inputs(running_machine &machine);

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
		memset(&conf, 0, sizeof(conf));
		get_resolution(options().resolution(), options().resolution(index), &conf, true);

		// create window ...
		std::shared_ptr<sdl_window_info> win = std::make_shared<sdl_window_info>(machine(), index, m_monitor_module->pick_monitor(reinterpret_cast<osd_options &>(options()), index), &conf);

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

void sdl_osd_interface::input_update()
{
	// poll the joystick values here
	process_events_buf();
	poll_inputs(machine());
	check_osd_inputs(machine());
}

//============================================================
//  check_osd_inputs
//============================================================

static void check_osd_inputs(running_machine &machine)
{
	// check for toggling fullscreen mode
	if (machine.ui_input().pressed(IPT_OSD_1))
	{
		for (auto curwin : osd_common_t::s_window_list)
			std::static_pointer_cast<sdl_window_info>(curwin)->toggle_full_screen();
	}

	auto window = osd_common_t::s_window_list.front();

	if (USE_OPENGL)
	{
		//FIXME: on a per window basis
		if (machine.ui_input().pressed(IPT_OSD_5))
		{
			video_config.filter = !video_config.filter;
			machine.ui().popup_time(1, "Filter %s", video_config.filter? "enabled":"disabled");
		}
	}

	if (machine.ui_input().pressed(IPT_OSD_6))
		std::static_pointer_cast<sdl_window_info>(window)->modify_prescale(-1);

	if (machine.ui_input().pressed(IPT_OSD_7))
		std::static_pointer_cast<sdl_window_info>(window)->modify_prescale(1);

	if (machine.ui_input().pressed(IPT_OSD_8))
		window->renderer().record();
}

//============================================================
//  extract_video_config
//============================================================

void sdl_osd_interface::extract_video_config()
{
	const char *stemp;

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

	// default to working video please
	video_config.novideo = 0;

	// video options: extract the data
	stemp = options().video();
	if (strcmp(stemp, "auto") == 0)
	{
#if (defined SDLMAME_EMSCRIPTEN)
		stemp = "soft";
#else
		stemp = "bgfx";
#endif
	}
	if (strcmp(stemp, SDLOPTVAL_SOFT) == 0)
		video_config.mode = VIDEO_MODE_SOFT;
	else if (strcmp(stemp, OSDOPTVAL_NONE) == 0)
	{
		video_config.mode = VIDEO_MODE_SOFT;
		video_config.novideo = 1;

		if (!emulator_info::standalone() && options().seconds_to_run() == 0)
			osd_printf_warning("Warning: -video none doesn't make much sense without -seconds_to_run\n");
	}
#if (USE_OPENGL)
	else if (strcmp(stemp, SDLOPTVAL_OPENGL) == 0)
		video_config.mode = VIDEO_MODE_OPENGL;
#endif
	else if ((strcmp(stemp, SDLOPTVAL_SDL2ACCEL) == 0))
	{
		video_config.mode = VIDEO_MODE_SDL2ACCEL;
	}
	else if (strcmp(stemp, SDLOPTVAL_BGFX) == 0)
	{
		video_config.mode = VIDEO_MODE_BGFX;
	}
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
		osd_printf_warning("-syncrefresh specified without -waitvsync. Reverting to -nosyncrefresh\n");
		video_config.syncrefresh = 0;
	}

	if (video_config.prescale < 1 || video_config.prescale > 8)
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
					video_config.glsl_shader_mamebm[i] = nullptr;
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
					video_config.glsl_shader_scrn[i] = nullptr;
				}
			}
		} else {
			int i;
			video_config.glsl_filter = 0;
			video_config.glsl_shader_mamebm_num=0;
			for(i=0; i<GLSL_SHADER_MAX; i++)
			{
				video_config.glsl_shader_mamebm[i] = nullptr;
			}
			video_config.glsl_shader_scrn_num=0;
			for(i=0; i<GLSL_SHADER_MAX; i++)
			{
				video_config.glsl_shader_scrn[i] = nullptr;
			}
		}

	#endif /* USE_OPENGL */
	// misc options: sanity check values

	// global options: sanity check values
	if (video_config.numscreens < 1 || video_config.numscreens > MAX_VIDEO_WINDOWS)
	{
		osd_printf_warning("Invalid numscreens value %d; reverting to 1\n", video_config.numscreens);
		video_config.numscreens = 1;
	}
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
