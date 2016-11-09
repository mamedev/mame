// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  drawnone.cpp - stub "nothing" drawer
//
//============================================================

// standard windows headers
#define WIN32_LEAN_AND_MEAN
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
	GetClientRect(win->platform_window<HWND>(), &client);
#elif defined(OSD_UWP)
	auto bounds = win->m_window->Bounds;
	client.left = bounds.Left;
	client.right = bounds.Right;
	client.top = bounds.Top;
	client.bottom = bounds.Bottom;
#endif
	win->target()->set_bounds(rect_width(&client), rect_height(&client), win->pixel_aspect());
	return &win->target()->get_primitives();
}
