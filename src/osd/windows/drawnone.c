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



//============================================================
//  PROTOTYPES
//============================================================

// core functions
static void drawnone_exit(void);
static int drawnone_window_init(win_window_info *window);
static void drawnone_window_destroy(win_window_info *window);
static render_primitive_list *drawnone_window_get_primitives(win_window_info *window);
static int drawnone_window_draw(win_window_info *window, HDC dc, int update);



//============================================================
//  drawnone_init
//============================================================

int drawnone_init(running_machine &machine, win_draw_callbacks *callbacks)
{
	// fill in the callbacks
	memset(callbacks, 0, sizeof(*callbacks));
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

static render_primitive_list *drawnone_window_get_primitives(win_window_info *window)
{
	RECT client;
	GetClientRect(window->hwnd, &client);
	window->target->set_bounds(rect_width(&client), rect_height(&client), winvideo_monitor_get_aspect(window->monitor));
	return &window->target->get_primitives();
}



//============================================================
//  drawnone_window_draw
//============================================================

static int drawnone_window_draw(win_window_info *window, HDC dc, int update)
{
	return 0;
}
