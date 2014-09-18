// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
//============================================================
//
//  drawbgfx.c - BGFX drawer
//
//============================================================
#define __STDC_LIMIT_MACROS
#define __STDC_FORMAT_MACROS
#define __STDC_CONSTANT_MACROS

// standard C headers
#include <math.h>
#include <stdio.h>

// MAME headers
#include "emu.h"
#include "options.h"

// standard SDL headers
#include "sdlinc.h"

// OSD headers
#include "osdsdl.h"
#include "window.h"

#include <bgfxplatform.h> 
#include <bgfx.h>



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
static render_primitive_list &drawbgfx_window_get_primitives(sdl_window_info *window);
static void drawbgfx_destroy_all_textures(sdl_window_info *window);
static void drawbgfx_window_clear(sdl_window_info *window);
static int drawbgfx_xy_to_render_target(sdl_window_info *window, int x, int y, int *xt, int *yt);


int drawbgfx_init(running_machine &machine, sdl_draw_info *callbacks)
{
	const char *stemp;

	// fill in the callbacks
	callbacks->exit = drawbgfx_exit;
	callbacks->attach = drawbgfx_attach;

	osd_printf_verbose("Using SDL native texturing driver (SDL 2.0+)\n");

	// Load the GL library now - else MT will fail
	stemp = downcast<sdl_options &>(machine.options()).gl_lib();
	if (stemp != NULL && strcmp(stemp, SDLOPTVAL_AUTO) == 0)
		stemp = NULL;

	// No fatalerror here since not all video drivers support GL !
	if (SDL_GL_LoadLibrary(stemp) != 0) // Load library (default for e==NULL
		osd_printf_verbose("Warning: Unable to load opengl library: %s\n", stemp ? stemp : "<default>");
	else
		osd_printf_verbose("Loaded opengl shared library: %s\n", stemp ? stemp : "<default>");

	return 0;
		
}

static void drawbgfx_destroy_all_textures(sdl_window_info *window)
{
}
//============================================================
//  drawbgfx_exit
//============================================================

static void drawbgfx_exit(void)
{
}

//============================================================
//  drawbgfx_attach
//============================================================

static void drawbgfx_attach(sdl_draw_info *info, sdl_window_info *window)
{
	// fill in the callbacks
	window->create = drawbgfx_window_create;
	window->resize = drawbgfx_window_resize;
	window->get_primitives = drawbgfx_window_get_primitives;
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
	osd_printf_verbose("Enter drawbgfx_window_create\n");
	int extra_flags = (window->fullscreen ?
			SDL_WINDOW_BORDERLESS | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_FULLSCREEN : SDL_WINDOW_RESIZABLE);

	// create the SDL window
	window->sdl_window = SDL_CreateWindow(window->title, SDL_WINDOWPOS_UNDEFINED_DISPLAY(window->monitor->handle), SDL_WINDOWPOS_UNDEFINED,
			width, height, extra_flags);

	bgfx::sdlSetWindow(window->sdl_window);
	if (window->fullscreen && video_config.switchres)
	{
		SDL_DisplayMode mode;
		SDL_GetCurrentDisplayMode(window->monitor->handle, &mode);
		mode.w = width;
		mode.h = height;
		if (window->refresh)
			mode.refresh_rate = window->refresh;
		if (window->depth)
		{
			switch (window->depth)
			{
			case 15:
				mode.format = SDL_PIXELFORMAT_RGB555;
				break;
			case 16:
				mode.format = SDL_PIXELFORMAT_RGB565;
				break;
			case 24:
				mode.format = SDL_PIXELFORMAT_RGB24;
				break;
			case 32:
				mode.format = SDL_PIXELFORMAT_RGB888;
				break;
			default:
				osd_printf_warning("Ignoring depth %d\n", window->depth);
			}
		}
		SDL_SetWindowDisplayMode(window->sdl_window, &mode);    // Try to set mode
	}
	else
		SDL_SetWindowDisplayMode(window->sdl_window, NULL); // Use desktop

	// create renderer

	//SDL_SelectRenderer(window->sdl_window);

	SDL_ShowWindow(window->sdl_window);
	//SDL_SetWindowFullscreen(window->window_id, window->fullscreen);
	SDL_RaiseWindow(window->sdl_window);
	SDL_GetWindowSize(window->sdl_window, &window->width, &window->height);
	
	
	bgfx::init();
	bgfx::reset(window->width, window->height, BGFX_RESET_VSYNC);
	
	// Enable debug text.
	bgfx::setDebug(BGFX_DEBUG_STATS);// BGFX_DEBUG_TEXT);
			
	
	osd_printf_verbose("Leave drawbgfx_window_create\n");
	return 0;
}

//============================================================
//  drawbgfx_window_resize
//============================================================

static void drawbgfx_window_resize(sdl_window_info *window, int width, int height)
{
	window->width = width;
	window->height = height;
}

//============================================================
//  drawsdl_xy_to_render_target
//============================================================

static int drawbgfx_xy_to_render_target(sdl_window_info *window, int x, int y, int *xt, int *yt)
{
	return 1;
}

//============================================================
//  drawbgfx_window_get_primitives
//============================================================

static render_primitive_list &drawbgfx_window_get_primitives(sdl_window_info *window)
{
	if ((!window->fullscreen) || (video_config.switchres))
	{
		sdlwindow_blit_surface_size(window, window->width, window->height);
	}
	else
	{
		sdlwindow_blit_surface_size(window, window->monitor->center_width, window->monitor->center_height);
	}
	window->target->set_bounds(window->blitwidth, window->blitheight, sdlvideo_monitor_get_aspect(window->monitor));
	return window->target->get_primitives();
}

//============================================================
//  drawbgfx_window_draw
//============================================================

static int drawbgfx_window_draw(sdl_window_info *window, UINT32 dc, int update)
{
	if (video_config.novideo)
	{
		return 0;
	}

	window->primlist->acquire_lock();
	
	bgfx::setViewClear(0
		, BGFX_CLEAR_COLOR_BIT|BGFX_CLEAR_DEPTH_BIT
		, 0x00000000
		, 1.0f
		, 0
		);
	// Set view 0 default viewport.
	bgfx::setViewRect(0, 0, 0, window->width, window->height);

	// This dummy draw call is here to make sure that view 0 is cleared
	// if no other draw calls are submitted to view 0.
	bgfx::submit(0);

	// now draw
	for (render_primitive *prim = window->primlist->first(); prim != NULL; prim = prim->next())
	{
		switch (prim->type)
		{
			case render_primitive::LINE:
				break;
			case render_primitive::QUAD:
				break;
			default:
				throw emu_fatalerror("Unexpected render_primitive type\n");
		}
	}

	window->primlist->release_lock();
	// Advance to next frame. Rendering thread will be kicked to 
	// process submitted rendering primitives.
	bgfx::frame();
	
	return 0;
}


//============================================================
//  drawbgfx_window_clear
//============================================================

static void drawbgfx_window_clear(sdl_window_info *window)
{
}


//============================================================
//  drawbgfx_window_destroy
//============================================================

static void drawbgfx_window_destroy(sdl_window_info *window)
{
	// free the memory in the window
	SDL_DestroyWindow(window->sdl_window);
	// Shutdown bgfx.
	bgfx::shutdown();
	
	window->dxdata = NULL;
}

