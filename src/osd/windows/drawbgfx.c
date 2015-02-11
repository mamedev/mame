// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
//============================================================
//
//  drawbgfx.c - BGFX drawer
//
//============================================================

// standard windows headers
#define __STDC_LIMIT_MACROS
#define __STDC_FORMAT_MACROS
#define __STDC_CONSTANT_MACROS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// MAME headers
#include "emu.h"

// MAMEOS headers
#include "window.h"

#include <bgfxplatform.h> 
#include <bgfx.h>

class renderer_bgfx : public osd_renderer
{
public:
	renderer_bgfx(osd_window *window)
	: osd_renderer(window, FLAG_NONE) { }

	virtual ~renderer_bgfx() { }

	virtual int create();
	virtual render_primitive_list *get_primitives();
	virtual int draw(HDC dc, int update);
	virtual void save() {};
	virtual void record() {};
	virtual void toggle_fsfx() {};
	virtual void destroy();

private:
};


//============================================================
//  PROTOTYPES
//============================================================

// core functions
static void drawbgfx_exit(void);

//============================================================
//  drawnone_create
//============================================================

osd_renderer *drawbgfx_create(osd_window *window)
{
	return global_alloc(renderer_bgfx(window));
}

//============================================================
//  drawbgfx_init
//============================================================

int drawbgfx_init(running_machine &machine, osd_draw_callbacks *callbacks)
{
	// fill in the callbacks
	memset(callbacks, 0, sizeof(*callbacks));
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
//  drawbgfx_window_init
//============================================================

int renderer_bgfx::create()
{
	RECT client;
	GetClientRect(window().m_hwnd, &client);

	bgfx::winSetHwnd(window().m_hwnd);
	bgfx::init();
	bgfx::reset(rect_width(&client), rect_height(&client), BGFX_RESET_VSYNC);
	
	// Enable debug text.
	bgfx::setDebug(BGFX_DEBUG_STATS);// BGFX_DEBUG_TEXT);

	return 0;
}



//============================================================
//  drawbgfx_window_destroy
//============================================================

void renderer_bgfx::destroy()
{
	// Shutdown bgfx.
	bgfx::shutdown();
}



//============================================================
//  drawbgfx_window_get_primitives
//============================================================

render_primitive_list *renderer_bgfx::get_primitives()
{
	RECT client;
	GetClientRect(window().m_hwnd, &client);
	window().target()->set_bounds(rect_width(&client), rect_height(&client), window().m_monitor->get_aspect());
	return &window().target()->get_primitives();
}



//============================================================
//  drawbgfx_window_draw
//============================================================

int renderer_bgfx::draw(HDC dc, int update)
{
	RECT client;
	GetClientRect(window().m_hwnd, &client);

	bgfx::setViewClear(0
		, BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH
		, 0x000000ff
		, 1.0f
		, 0
		);
	// Set view 0 default viewport.
	bgfx::setViewRect(0, 0, 0, rect_width(&client), rect_height(&client));

	// This dummy draw call is here to make sure that view 0 is cleared
	// if no other draw calls are submitted to view 0.
	bgfx::submit(0);

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
