// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  drawnone.c - stub "nothing" drawer
//
//============================================================

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// MAME headers
#include "emu.h"

// MAMEOS headers
#include "window.h"


class renderer_none : public osd_renderer
{
public:
	renderer_none(osd_window *window)
	: osd_renderer(window, FLAG_NONE) { }

	virtual ~renderer_none() { }

	virtual int create();
	virtual render_primitive_list *get_primitives();
	virtual int draw(const int update);
	virtual void save() { };
	virtual void record() { };
	virtual void toggle_fsfx() { };
	virtual void destroy();
};

//============================================================
//  PROTOTYPES
//============================================================

// core functions
static void drawnone_exit(void);

//============================================================
//  drawnone_create
//============================================================

osd_renderer *drawnone_create(osd_window *window)
{
	return global_alloc(renderer_none(window));
}

//============================================================
//  drawnone_init
//============================================================

int drawnone_init(running_machine &machine, osd_draw_callbacks *callbacks)
{
	// fill in the callbacks
	memset(callbacks, 0, sizeof(*callbacks));
	callbacks->exit = drawnone_exit;
	callbacks->create = drawnone_create;
	return 0;
}



//============================================================
//  drawnone_exit
//============================================================

static void drawnone_exit(void)
{
}



//============================================================
//  drawnone_window_init
//============================================================

int renderer_none::create()
{
	return 0;
}



//============================================================
//  drawnone_window_destroy
//============================================================

void renderer_none::destroy()
{
}



//============================================================
//  drawnone_window_get_primitives
//============================================================

render_primitive_list *renderer_none::get_primitives()
{
	RECT client;
	GetClientRect(window().m_hwnd, &client);
	window().target()->set_bounds(rect_width(&client), rect_height(&client), window().aspect());
	return &window().target()->get_primitives();
}



//============================================================
//  drawnone_window_draw
//============================================================

int renderer_none::draw(const int update)
{
	return 0;
}
