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
    sdl_info_bgfx(osd_window *w)
    : osd_renderer(w, FLAG_NONE), m_blittimer(0), m_renderer(NULL),
      m_blitwidth(0), m_blitheight(0),
      m_last_hofs(0), m_last_vofs(0),
      m_last_blit_time(0), m_last_blit_pixels(0)
    {}

	/* virtual */ int create();
	/* virtual */ int draw(const UINT32 dc, const int update);
	/* virtual */ int xy_to_render_target(const int x, const int y, int *xt, int *yt);
	/* virtual */ void destroy();
	/* virtual */ render_primitive_list *get_primitives()
	{
		int nw = 0; int nh = 0;
		window().blit_surface_size(nw, nh);
		if (nw != m_blitwidth || nh != m_blitheight)
		{
			m_blitwidth = nw; m_blitheight = nh;
			notify_changed();
		}
		window().target()->set_bounds(m_blitwidth, m_blitheight, window().aspect());
		return &window().target()->get_primitives();
	}

   // void render_quad(texture_info *texture, const render_primitive *prim, const int x, const int y);

    //texture_info *texture_find(const render_primitive &prim, const quad_setup_data &setup);
    //texture_info *texture_update(const render_primitive &prim);

	INT32           m_blittimer;

	SDL_Renderer *  m_renderer;
	//simple_list<texture_info>  m_texlist;                // list of active textures

	int				m_blitwidth;
	int				m_blitheight;
	float           m_last_hofs;
	float           m_last_vofs;

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

static osd_renderer *drawbgfx_create(osd_window *window)
{
	return global_alloc(sdl_info_bgfx(window));
}


int drawbgfx_init(running_machine &machine, osd_draw_callbacks *callbacks)
{
	// fill in the callbacks
	callbacks->exit = drawbgfx_exit;
	callbacks->create = drawbgfx_create;
		
	return 0;
}

//============================================================
//  sdl_info_bgfx::create
//============================================================

int sdl_info_bgfx::create()
{
	// create renderer

	int width = 0; int height = 0;

	window().get_size(width, height);
	m_blittimer = 3;

	bgfx::sdlSetWindow(window().sdl_window());
	bgfx::init();
	bgfx::reset(width, height, BGFX_RESET_VSYNC);
	
	// Enable debug text.
	bgfx::setDebug(BGFX_DEBUG_STATS);// BGFX_DEBUG_TEXT);
	osd_printf_verbose("Leave drawsdl2_window_create\n");
	return 0;
}

//============================================================
//  drawsdl_xy_to_render_target
//============================================================

int sdl_info_bgfx::xy_to_render_target(int x, int y, int *xt, int *yt)
{
	*xt = x - m_last_hofs;
	*yt = y - m_last_vofs;
	if (*xt<0 || *xt >= m_blitwidth)
		return 0;
	if (*yt<0 || *yt >= m_blitheight)
		return 0;
	return 1;
}

//============================================================
//  sdl_info_bgfx::draw
//============================================================

int sdl_info_bgfx::draw(UINT32 dc, int update)
{

	//if (has_flags(FI_CHANGED) || (window().width() != m_last_width) || (window().height() != m_last_height))
		// do something
	//clear_flags(FI_CHANGED);

	bgfx::setViewClear(0
		, BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH
		, 0x000000ff
		, 1.0f
		, 0
		);
	// Set view 0 default viewport.
	bgfx::setViewRect(0, 0, 0, m_blitwidth, m_blitheight);

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

	// destroy_all_textures();

	// Shutdown bgfx.
	bgfx::shutdown();
}
