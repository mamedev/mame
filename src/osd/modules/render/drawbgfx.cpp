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

#if defined(SDLMAME_WIN32) || defined(OSD_WINDOWS)
// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#if defined(SDLMAME_WIN32)
#if (SDLMAME_SDL2)
#include <SDL2/SDL_syswm.h>
#else
#include <SDL/SDL_syswm.h>
#endif
#endif
#else
#include "sdlinc.h"
#endif

// MAMEOS headers
#include "emu.h"
#include "window.h"

#include <bgfx/bgfxplatform.h>
#include <bgfx/bgfx.h>

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
//  INLINES
//============================================================


//============================================================
//  TYPES
//============================================================


/* sdl_info is the information about SDL for the current screen */
class renderer_bgfx : public osd_renderer
{
public:
	renderer_bgfx(osd_window *w)
	: osd_renderer(w, FLAG_NONE), m_blittimer(0),
		m_blit_dim(0, 0),
		m_last_hofs(0), m_last_vofs(0),
		m_last_blit_time(0), m_last_blit_pixels(0)
	{}

	/* virtual */ int create();
	/* virtual */ int draw(const int update);
#ifdef OSD_SDL
	/* virtual */ int xy_to_render_target(const int x, const int y, int *xt, int *yt);
#else
	/* virtual */ void save() { }
	/* virtual */ void record() { }
	/* virtual */ void toggle_fsfx() { }
#endif
	/* virtual */ void destroy();
	/* virtual */ render_primitive_list *get_primitives()
	{
#ifdef OSD_WINDOWS
		RECT client;
		GetClientRect(window().m_hwnd, &client);
		window().target()->set_bounds(rect_width(&client), rect_height(&client), window().aspect());
		return &window().target()->get_primitives();
#else
		osd_dim nd = window().blit_surface_size();
		if (nd != m_blit_dim)
		{
			m_blit_dim = nd;
			notify_changed();
		}
		window().target()->set_bounds(m_blit_dim.width(), m_blit_dim.height(), window().aspect());
		return &window().target()->get_primitives();
#endif
	}

	// void render_quad(texture_info *texture, const render_primitive *prim, const int x, const int y);

	//texture_info *texture_find(const render_primitive &prim, const quad_setup_data &setup);
	//texture_info *texture_update(const render_primitive &prim);

	INT32           m_blittimer;

	//simple_list<texture_info>  m_texlist;                // list of active textures

	osd_dim         m_blit_dim;
	float           m_last_hofs;
	float           m_last_vofs;

	// Stats
	INT64           m_last_blit_time;
	INT64           m_last_blit_pixels;

	// Original display_mode
};


//============================================================
//  PROTOTYPES
//============================================================

// core functions
static void drawbgfx_exit(void);

//============================================================
//  drawnone_create
//============================================================

static osd_renderer *drawbgfx_create(osd_window *window)
{
	return global_alloc(renderer_bgfx(window));
}

//============================================================
//  drawbgfx_init
//============================================================

int drawbgfx_init(running_machine &machine, osd_draw_callbacks *callbacks)
{
	// fill in the callbacks
	//memset(callbacks, 0, sizeof(*callbacks));
	callbacks->exit = drawbgfx_exit;
	callbacks->create = drawbgfx_create;

	return 0;
}

//============================================================
//  drawbgfx_exit
//============================================================

static void drawbgfx_exit(void)
{
}

//============================================================
//  renderer_bgfx::create
//============================================================

int renderer_bgfx::create()
{
	// create renderer

#ifdef OSD_WINDOWS
	RECT client;
	GetClientRect(window().m_hwnd, &client);

	bgfx::winSetHwnd(window().m_hwnd);
	bgfx::init();
	bgfx::reset(rect_width(&client), rect_height(&client), BGFX_RESET_VSYNC);
#else
	osd_dim d = window().get_size();
	m_blittimer = 3;

	bgfx::sdlSetWindow(window().sdl_window());
	bgfx::init();
	bgfx::reset(d.width(), d.height(), BGFX_RESET_VSYNC);
#endif

	// Enable debug text.
	bgfx::setDebug(BGFX_DEBUG_STATS);// BGFX_DEBUG_TEXT);

	osd_printf_verbose("Leave drawsdl2_window_create\n");
	return 0;
}

//============================================================
//  drawbgfx_window_destroy
//============================================================

void renderer_bgfx::destroy()
{
	// free the memory in the window

	// destroy_all_textures();

	// Shutdown bgfx.
	bgfx::shutdown();
}


//============================================================
//  drawsdl_xy_to_render_target
//============================================================

#ifdef OSD_SDL
int renderer_bgfx::xy_to_render_target(int x, int y, int *xt, int *yt)
{
	*xt = x - m_last_hofs;
	*yt = y - m_last_vofs;
	if (*xt<0 || *xt >= m_blit_dim.width())
		return 0;
	if (*yt<0 || *yt >= m_blit_dim.height())
		return 0;
	return 1;
}
#endif

//============================================================
//  drawbgfx_window_draw
//============================================================

int renderer_bgfx::draw(int update)
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
#ifdef OSD_WINDOWS
	RECT client;
	GetClientRect(window().m_hwnd, &client);
	bgfx::setViewRect(0, 0, 0, rect_width(&client), rect_height(&client));
#else
	bgfx::setViewRect(0, 0, 0, m_blit_dim.width(), m_blit_dim.height());
#endif
	// This dummy draw call is here to make sure that view 0 is cleared
	// if no other draw calls are submitted to view 0.
	bgfx::touch(0);

	window().m_primlist->acquire_lock();

	// now draw
	for (render_primitive *prim = window().m_primlist->first(); prim != NULL; prim = prim->next())
	{
		switch (prim->type)
		{
			/**
			 * Try to stay in one Begin/End block as long as possible,
			 * since entering and leaving one is most expensive..
			 */
			case render_primitive::LINE:
				// check if it's really a point
/*
                if (((prim->bounds.x1 - prim->bounds.x0) == 0) && ((prim->bounds.y1 - prim->bounds.y0) == 0))
                {
                    curPrimitive=GL_POINTS;
                } else {
                    curPrimitive=GL_LINES;
                }

                if(pendingPrimitive!=GL_NO_PRIMITIVE && pendingPrimitive!=curPrimitive)
                {
                    glEnd();
                    pendingPrimitive=GL_NO_PRIMITIVE;
                }

                if ( pendingPrimitive==GL_NO_PRIMITIVE )
                {
                            set_blendmode(sdl, PRIMFLAG_GET_BLENDMODE(prim->flags));
                }

                glColor4f(prim->color.r, prim->color.g, prim->color.b, prim->color.a);

                if(pendingPrimitive!=curPrimitive)
                {
                    glBegin(curPrimitive);
                    pendingPrimitive=curPrimitive;
                }

                // check if it's really a point
                if (curPrimitive==GL_POINTS)
                {
                    glVertex2f(prim->bounds.x0+hofs, prim->bounds.y0+vofs);
                }
                else
                {
                    glVertex2f(prim->bounds.x0+hofs, prim->bounds.y0+vofs);
                    glVertex2f(prim->bounds.x1+hofs, prim->bounds.y1+vofs);
                }*/
				break;

			case render_primitive::QUAD:
/*
                if(pendingPrimitive!=GL_NO_PRIMITIVE)
                {
                    glEnd();
                    pendingPrimitive=GL_NO_PRIMITIVE;
                }

                glColor4f(prim->color.r, prim->color.g, prim->color.b, prim->color.a);

                set_blendmode(sdl, PRIMFLAG_GET_BLENDMODE(prim->flags));

                texture = texture_update(window, prim, 0);


                sdl->texVerticex[0]=prim->bounds.x0 + hofs;
                sdl->texVerticex[1]=prim->bounds.y0 + vofs;
                sdl->texVerticex[2]=prim->bounds.x1 + hofs;
                sdl->texVerticex[3]=prim->bounds.y0 + vofs;
                sdl->texVerticex[4]=prim->bounds.x1 + hofs;
                sdl->texVerticex[5]=prim->bounds.y1 + vofs;
                sdl->texVerticex[6]=prim->bounds.x0 + hofs;
                sdl->texVerticex[7]=prim->bounds.y1 + vofs;

                glDrawArrays(GL_QUADS, 0, 4);
*/
				break;

			default:
				throw emu_fatalerror("Unexpected render_primitive type");
		}
	}

	window().m_primlist->release_lock();
	// Advance to next frame. Rendering thread will be kicked to
	// process submitted rendering primitives.
	bgfx::frame();

	return 0;
}
