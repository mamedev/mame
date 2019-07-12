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
#if defined(OSD_WINDOWS)
	GetClientRect(std::static_pointer_cast<win_window_info>(win)->platform_window(), &client);
#elif defined(OSD_UWP)
	auto bounds = std::static_pointer_cast<uwp_window_info>(win)->platform_window()->Bounds;
	client.left = bounds.Left;
	client.right = bounds.Right;
	client.top = bounds.Top;
	client.bottom = bounds.Bottom;
#endif
	if ((rect_width(&client) == 0) || (rect_height(&client) == 0))
		return nullptr;
	win->target()->set_bounds(rect_width(&client), rect_height(&client), win->pixel_aspect());
	return &win->target()->get_primitives();
}
