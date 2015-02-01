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
static void drawbgfx_attach(sdl_draw_info *info, sdl_window_info *window);
static int drawbgfx_window_create(sdl_window_info *window, int width, int height);
static void drawbgfx_window_resize(sdl_window_info *window, int width, int height);
static void drawbgfx_window_destroy(sdl_window_info *window);
static int drawbgfx_window_draw(sdl_window_info *window, UINT32 dc, int update);
static void drawbgfx_set_target_bounds(sdl_window_info *window);
static void drawbgfx_destroy_all_textures(sdl_window_info *window);
static void drawbgfx_window_clear(sdl_window_info *window);
static int drawbgfx_xy_to_render_target(sdl_window_info *window, int x, int y, int *xt, int *yt);

//============================================================
//  Textures
//============================================================

/* sdl_info is the information about SDL for the current screen */
struct sdl_info13
{
    sdl_info13()
    : m_blittimer(0), m_renderer(NULL),
      m_hofs(0), m_vofs(0),
      m_resize_pending(0), m_resize_width(0), m_resize_height(0),
      m_last_blit_time(0), m_last_blit_pixels(0)
    {}

   // void render_quad(texture_info *texture, const render_primitive *prim, const int x, const int y);

    //texture_info *texture_find(const render_primitive &prim, const quad_setup_data &setup);
    //texture_info *texture_update(const render_primitive &prim);

	INT32           m_blittimer;

	SDL_Renderer *  m_renderer;
	//simple_list<texture_info>  m_texlist;                // list of active textures

	float           m_hofs;
	float           m_vofs;

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

int drawbgfx_init(running_machine &machine, sdl_draw_info *callbacks)
{
	// fill in the callbacks
	callbacks->exit = drawbgfx_exit;
	callbacks->attach = drawbgfx_attach;
		
	return 0;
}


//============================================================
//  drawbgfx_attach
//============================================================

static void drawbgfx_attach(sdl_draw_info *info, sdl_window_info *window)
{
	// fill in the callbacks
	window->create = drawbgfx_window_create;
	window->resize = drawbgfx_window_resize;
	window->set_target_bounds = drawbgfx_set_target_bounds;
	window->draw = drawbgfx_window_draw;
	window->destroy = drawbgfx_window_destroy;
	window->destroy_all_textures = drawbgfx_destroy_all_textures;
	window->clear = drawbgfx_window_clear;
	window->xy_to_render_target = drawbgfx_xy_to_render_target;
}

//============================================================
//  drawbgfx_window_create
//============================================================

static int drawbgfx_window_create(sdl_window_info *window, int width, int height)
{
	// allocate memory for our structures
	sdl_info13 *sdl = global_alloc(sdl_info13);

	/* FIXME: On Ubuntu and potentially other Linux OS you should use
	 * to disable panning. This has to be done before every invocation of mame.
	 *
	 * xrandr --output HDMI-0 --panning 0x0+0+0 --fb 0x0
	 *
	 */

	osd_printf_verbose("Enter drawsdl2_window_create\n");

	window->dxdata = sdl;

	UINT32 extra_flags = (window->fullscreen() ?
			SDL_WINDOW_BORDERLESS | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_FULLSCREEN : SDL_WINDOW_RESIZABLE);

#if defined(SDLMAME_WIN32)
	SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");
#endif
	// create the SDL window
	window->sdl_window = SDL_CreateWindow(window->title,
			window->monitor()->position_size().x, window->monitor()->position_size().y,
			width, height, extra_flags);

	if (window->fullscreen() && video_config.switchres)
	{
		SDL_DisplayMode mode;
		//SDL_GetCurrentDisplayMode(window->monitor()->handle, &mode);
		SDL_GetWindowDisplayMode(window->sdl_window, &mode);
		sdl->m_original_mode = mode;
		mode.w = width;
		mode.h = height;
		if (window->refresh)
			mode.refresh_rate = window->refresh;

		SDL_SetWindowDisplayMode(window->sdl_window, &mode);    // Try to set mode
#ifndef SDLMAME_WIN32
		/* FIXME: Warp the mouse to 0,0 in case a virtual desktop resolution
		 * is in place after the mode switch - which will most likely be the case
		 * This is a hack to work around a deficiency in SDL2
		 */
		SDL_WarpMouseInWindow(window->sdl_window, 1, 1);
#endif
	}
	else
	{
		//SDL_SetWindowDisplayMode(window->sdl_window, NULL); // Use desktop
	}
	// create renderer

	if (video_config.waitvsync)
		sdl->m_renderer = SDL_CreateRenderer(window->sdl_window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
	else
		sdl->m_renderer = SDL_CreateRenderer(window->sdl_window, -1, SDL_RENDERER_ACCELERATED);

	if (!sdl->m_renderer)
	{
		fatalerror("Error on creating renderer: %s\n", SDL_GetError());
	}

	//SDL_SelectRenderer(window->sdl_window);
	SDL_ShowWindow(window->sdl_window);
	//SDL_SetWindowFullscreen(window->window_id, window->fullscreen);
	SDL_RaiseWindow(window->sdl_window);

	SDL_GetWindowSize(window->sdl_window, &window->width, &window->height);

	sdl->m_blittimer = 3;

	SDL_RenderPresent(sdl->m_renderer);
	
	bgfx::sdlSetWindow(window->sdl_window);
	bgfx::init();
	bgfx::reset(window->width, window->height, BGFX_RESET_VSYNC);
	
	// Enable debug text.
	bgfx::setDebug(BGFX_DEBUG_STATS);// BGFX_DEBUG_TEXT);
	osd_printf_verbose("Leave drawsdl2_window_create\n");
	return 0;
}

//============================================================
//  drawbgfx_window_resize
//============================================================

static void drawbgfx_window_resize(sdl_window_info *window, int width, int height)
{
	sdl_info13 *sdl = (sdl_info13 *) window->dxdata;

	sdl->m_resize_pending = 1;
	sdl->m_resize_height = height;
	sdl->m_resize_width = width;

	window->width = width;
	window->height = height;

	sdl->m_blittimer = 3;
}

//============================================================
//  drawsdl_xy_to_render_target
//============================================================

static int drawbgfx_xy_to_render_target(sdl_window_info *window, int x, int y, int *xt, int *yt)
{
	sdl_info13 *sdl = (sdl_info13 *) window->dxdata;

	*xt = x - sdl->m_hofs;
	*yt = y - sdl->m_vofs;
	if (*xt<0 || *xt >= window->blitwidth)
		return 0;
	if (*yt<0 || *yt >= window->blitheight)
		return 0;
	return 1;
}

//============================================================
//  drawbgfx_window_get_primitives
//============================================================

static void drawbgfx_set_target_bounds(sdl_window_info *window)
{
	window->target->set_bounds(window->blitwidth, window->blitheight, window->monitor()->aspect());
}

//============================================================
//  drawbgfx_window_draw
//============================================================

static int drawbgfx_window_draw(sdl_window_info *window, UINT32 dc, int update)
{
	bgfx::setViewClear(0
		, BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH
		, 0x000000ff
		, 1.0f
		, 0
		);
	// Set view 0 default viewport.
	bgfx::setViewRect(0, 0, 0, window->blitwidth, window->blitheight);

	// This dummy draw call is here to make sure that view 0 is cleared
	// if no other draw calls are submitted to view 0.
	bgfx::submit(0);

	window->primlist->acquire_lock();
	window->primlist->release_lock();	
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
//  drawbgfx_window_destroy
//============================================================

static void drawbgfx_window_destroy(sdl_window_info *window)
{
	sdl_info13 *sdl = (sdl_info13 *) window->dxdata;

	// skip if nothing
	if (sdl == NULL)
		return;

	// free the memory in the window

	drawbgfx_destroy_all_textures(window);

	if (window->fullscreen() && video_config.switchres)
	{
		SDL_SetWindowFullscreen(window->sdl_window, 0);    // Try to set mode
		SDL_SetWindowDisplayMode(window->sdl_window, &sdl->m_original_mode);    // Try to set mode
		SDL_SetWindowFullscreen(window->sdl_window, SDL_WINDOW_FULLSCREEN);    // Try to set mode
	}

	SDL_DestroyWindow(window->sdl_window);

	global_free(sdl);
	window->dxdata = NULL;
	
	// Shutdown bgfx.
	bgfx::shutdown();
}

static void drawbgfx_destroy_all_textures(sdl_window_info *window)
{
}

//============================================================
//  TEXCOPY FUNCS
//============================================================

static void drawbgfx_window_clear(sdl_window_info *window)
{
	sdl_info13 *sdl = (sdl_info13 *) window->dxdata;

	sdl->m_blittimer = 2;
}
