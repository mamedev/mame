// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  drawnone.cpp - stub "nothing" drawer
//
//============================================================

// standard windows headers
#include <windows.h>

// MAME headers
#include "emu.h"

#include "drawnone.h"

//============================================================
//  drawnone_window_get_primitives
//============================================================

render_primitive_list *renderer_none::get_primitives()
{
	auto win = try_getwindow();
	if (win == nullptr)
		return nullptr;

	RECT client;
	GetClientRect(std::static_pointer_cast<win_window_info>(win)->platform_window(), &client);
	if ((rect_width(&client) == 0) || (rect_height(&client) == 0))
		return nullptr;
	win->target()->set_bounds(rect_width(&client), rect_height(&client), win->pixel_aspect());
	return &win->target()->get_primitives();
}
