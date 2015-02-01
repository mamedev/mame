// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
//============================================================
//
//  drawbgfx.c - BGFX drawer
//
//============================================================

#ifdef SDLMAME_WIN32
// standard windows headers
#define __STDC_LIMIT_MACROS
#define __STDC_FORMAT_MACROS
#define __STDC_CONSTANT_MACROS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

// standard C headers
#include <math.h>
#include <stdio.h>

// MAME headers
#include "osdcomm.h"
#include "emu.h"
#include "options.h"
#include "emuopts.h"

// standard SDL headers
#include "sdlinc.h"
#include "modules/lib/osdlib.h"

// OSD headers
#include "osdsdl.h"
#include "window.h"

#include <bgfxplatform.h> 
#include <bgfx.h>

//============================================================
//  DEBUGGING
//============================================================

//============================================================
//  CONSTANTS
//============================================================

//============================================================
//  MACROS
//============================================================

//============================================================
//  TYPES
//============================================================

//============================================================
//  INLINES
//============================================================


//============================================================
//  PROTOTYPES
//============================================================

// core functions

static void drawbgfx_exit(void);

//============================================================
//  Textures
//============================================================

/* sdl_info is the information about SDL for the current screen */
class sdl_info_bgfx : public osd_renderer
{
public:
    sdl_info_bgfx(sdl_window_info *w)
    : osd_renderer(w), m_blittimer(0), m_renderer(NULL),
      m_last_hofs(0), m_last_vofs(0),
      m_resize_pending(0), m_resize_width(0), m_resize_height(0),
      m_last_blit_time(0), m_last_blit_pixels(0)
    {}

	/* virtual */ int create(int width, int height);
	/* virtual */ void resize(int width, int height);
	/* virtual */ int draw(UINT32 dc, int update);
	/* virtual */ void set_target_bounds();
	/* virtual */ int xy_to_render_target(int x, int y, int *xt, int *yt);
	/* virtual */ void destroy_all_textures();
	/* virtual */ void destroy();
	/* virtual */ void clear();

   // void render_quad(texture_info *texture, const render_primitive *prim, const int x, const int y);

    //texture_info *texture_find(const render_primitive &prim, const quad_setup_data &setup);
    //texture_info *texture_update(const render_primitive &prim);

	INT32           m_blittimer;

	SDL_Renderer *  m_renderer;
	//simple_list<texture_info>  m_texlist;                // list of active textures

	float           m_last_hofs;
	float           m_last_vofs;

	// resize information

	UINT8           m_resize_pending;
	UINT32          m_resize_width;
	UINT32          m_resize_height;

	// Stats
	INT64           m_last_blit_time;
	INT64           m_last_blit_pixels;

	// Original display_mode
	SDL_DisplayMode m_original_mode;
};

//============================================================
//  Static Variables
//============================================================

//============================================================
//  drawbgfx_init
//============================================================

static osd_renderer *drawbgfx_create(sdl_window_info *window)
{
	return global_alloc(sdl_info_bgfx(window));
}


int drawbgfx_init(running_machine &machine, sdl_draw_info *callbacks)
{
	// fill in the callbacks
	callbacks->exit = drawbgfx_exit;
	callbacks->create = drawbgfx_create;
		
	return 0;
}

//============================================================
//  sdl_info_bgfx::create
//============================================================

int sdl_info_bgfx::create(int width, int height)
{
	/* FIXME: On Ubuntu and potentially other Linux OS you should use
	 * to disable panning. This has to be done before every invocation of mame.
	 *
	 * xrandr --output HDMI-0 --panning 0x0+0+0 --fb 0x0
	 *
	 */

	osd_printf_verbose("Enter drawsdl2_window_create\n");

	UINT32 extra_flags = (window().fullscreen() ?
			SDL_WINDOW_BORDERLESS | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_FULLSCREEN : SDL_WINDOW_RESIZABLE);

#if defined(SDLMAME_WIN32)
	SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");
#endif
	// create the SDL window
	window().m_sdl_window = SDL_CreateWindow(window().m_title,
			window().monitor()->position_size().x, window().monitor()->position_size().y,
			width, height, extra_flags);

	if (window().fullscreen() && video_config.switchres)
	{
		SDL_DisplayMode mode;
		//SDL_GetCurrentDisplayMode(window().monitor()->handle, &mode);
		SDL_GetWindowDisplayMode(window().m_sdl_window, &mode);
		m_original_mode = mode;
		mode.w = width;
		mode.h = height;
		if (window().m_refresh)
			mode.refresh_rate = window().m_refresh;

		SDL_SetWindowDisplayMode(window().m_sdl_window, &mode);    // Try to set mode
#ifndef SDLMAME_WIN32
		/* FIXME: Warp the mouse to 0,0 in case a virtual desktop resolution
		 * is in place after the mode switch - which will most likely be the case
		 * This is a hack to work around a deficiency in SDL2
		 */
		SDL_WarpMouseInWindow(window().m_sdl_window, 1, 1);
#endif
	}
	else
	{
		//SDL_SetWindowDisplayMode(window().m_sdl_window, NULL); // Use desktop
	}
	// create renderer

	if (video_config.waitvsync)
		m_renderer = SDL_CreateRenderer(window().m_sdl_window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
	else
		m_renderer = SDL_CreateRenderer(window().m_sdl_window, -1, SDL_RENDERER_ACCELERATED);

	if (!m_renderer)
	{
		fatalerror("Error on creating renderer: %s\n", SDL_GetError());
	}

	//SDL_SelectRenderer(window().m_sdl_window);
	SDL_ShowWindow(window().m_sdl_window);
	//SDL_SetWindowFullscreen(window().window_id, window().fullscreen);
	SDL_RaiseWindow(window().m_sdl_window);

	SDL_GetWindowSize(window().m_sdl_window, &window().m_width, &window().m_height);

	m_blittimer = 3;

	SDL_RenderPresent(m_renderer);
	
	bgfx::sdlSetWindow(window().m_sdl_window);
	bgfx::init();
	bgfx::reset(window().m_width, window().m_height, BGFX_RESET_VSYNC);
	
	// Enable debug text.
	bgfx::setDebug(BGFX_DEBUG_STATS);// BGFX_DEBUG_TEXT);
	osd_printf_verbose("Leave drawsdl2_window_create\n");
	return 0;
}

//============================================================
//  sdl_info_bgfx::resize
//============================================================

void sdl_info_bgfx::resize(int width, int height)
{
	m_resize_pending = 1;
	m_resize_height = height;
	m_resize_width = width;

	window().m_width = width;
	window().m_height = height;

	m_blittimer = 3;
}

//============================================================
//  drawsdl_xy_to_render_target
//============================================================

int sdl_info_bgfx::xy_to_render_target(int x, int y, int *xt, int *yt)
{
	*xt = x - m_last_hofs;
	*yt = y - m_last_vofs;
	if (*xt<0 || *xt >= window().m_blitwidth)
		return 0;
	if (*yt<0 || *yt >= window().m_blitheight)
		return 0;
	return 1;
}

//============================================================
//  sdl_info_bgfx::get_primitives
//============================================================

void sdl_info_bgfx::set_target_bounds()
{
	window().m_target->set_bounds(window().m_blitwidth, window().m_blitheight, window().monitor()->aspect());
}

//============================================================
//  sdl_info_bgfx::draw
//============================================================

int sdl_info_bgfx::draw(UINT32 dc, int update)
{
	bgfx::setViewClear(0
		, BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH
		, 0x000000ff
		, 1.0f
		, 0
		);
	// Set view 0 default viewport.
	bgfx::setViewRect(0, 0, 0, window().m_blitwidth, window().m_blitheight);

	// This dummy draw call is here to make sure that view 0 is cleared
	// if no other draw calls are submitted to view 0.
	bgfx::submit(0);

	window().m_primlist->acquire_lock();
	window().m_primlist->release_lock();
	// Advance to next frame. Rendering thread will be kicked to 
	// process submitted rendering primitives.
	bgfx::frame();
	
	return 0;
}

//============================================================
//  drawbgfx_exit
//============================================================

static void drawbgfx_exit(void)
{
}

//============================================================
//  sdl_info_bgfx::destroy
//============================================================

void sdl_info_bgfx::destroy()
{
	// free the memory in the window

	destroy_all_textures();

	if (window().fullscreen() && video_config.switchres)
	{
		SDL_SetWindowFullscreen(window().m_sdl_window, 0);    // Try to set mode
		SDL_SetWindowDisplayMode(window().m_sdl_window, &m_original_mode);    // Try to set mode
		SDL_SetWindowFullscreen(window().m_sdl_window, SDL_WINDOW_FULLSCREEN);    // Try to set mode
	}

	SDL_DestroyWindow(window().m_sdl_window);
	
	// Shutdown bgfx.
	bgfx::shutdown();
}

void sdl_info_bgfx::destroy_all_textures()
{
}

//============================================================
//  TEXCOPY FUNCS
//============================================================

void sdl_info_bgfx::clear()
{
	m_blittimer = 2;
}
