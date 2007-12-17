//============================================================
//
//  drawnone.c - stub "nothing" drawer
//
//  Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// MAME headers
#include "mamecore.h"

// MAMEOS headers
#include "window.h"



//============================================================
//  PROTOTYPES
//============================================================

// core functions
static void drawnone_exit(void);
static int drawnone_window_init(win_window_info *window);
static void drawnone_window_destroy(win_window_info *window);
static const render_primitive_list *drawnone_window_get_primitives(win_window_info *window);
static int drawnone_window_draw(win_window_info *window, HDC dc, int update);



//============================================================
//  drawnone_init
//============================================================

int drawnone_init(win_draw_callbacks *callbacks)
{
	// fill in the callbacks
	callbacks->exit = drawnone_exit;
	callbacks->window_init = drawnone_window_init;
	callbacks->window_get_primitives = drawnone_window_get_primitives;
	callbacks->window_draw = drawnone_window_draw;
	callbacks->window_destroy = drawnone_window_destroy;
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

static int drawnone_window_init(win_window_info *window)
{
	return 0;
}



//============================================================
//  drawnone_window_destroy
//============================================================

static void drawnone_window_destroy(win_window_info *window)
{
}



//============================================================
//  drawnone_window_get_primitives
//============================================================

static const render_primitive_list *drawnone_window_get_primitives(win_window_info *window)
{
	RECT client;
	GetClientRect(window->hwnd, &client);
	render_target_set_bounds(window->target, rect_width(&client), rect_height(&client), winvideo_monitor_get_aspect(window->monitor));
	return render_target_get_primitives(window->target);
}



//============================================================
//  drawnone_window_draw
//============================================================

static int drawnone_window_draw(win_window_info *window, HDC dc, int update)
{
	return 0;
}
