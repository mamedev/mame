// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//	logwininfo.c - Win32 debug window handling
//
//============================================================

#include "logwininfo.h"

#include "debugviewinfo.h"


logwin_info::logwin_info(debugger_windows_interface &debugger) :
	debugwin_info(debugger, false, astring("Errorlog: ", debugger.machine().system().description, " [", debugger.machine().system().name, "]"), NULL)
{
	if (!window())
		return;

	m_views[0].reset(global_alloc(debugview_info(debugger, *this, window(), DVT_LOG)));
	if ((m_views[0] == NULL) || !m_views[0]->is_valid())
	{
		m_views[0].reset();
		return;
	}

	// compute a client rect
	RECT bounds;
	bounds.top = bounds.left = 0;
	bounds.right = m_views[0]->prefwidth() + (2 * EDGE_WIDTH);
	bounds.bottom = 200;
	AdjustWindowRectEx(&bounds, DEBUG_WINDOW_STYLE, FALSE, DEBUG_WINDOW_STYLE_EX);

	// clamp the min/max size
	set_maxwidth(bounds.right - bounds.left);

	// position the window at the bottom-right
	SetWindowPos(window(), HWND_TOP, 100, 100, bounds.right - bounds.left, bounds.bottom - bounds.top, SWP_SHOWWINDOW);

	// recompute the children
	recompute_children();
}


logwin_info::~logwin_info()
{
}


void logwin_info::recompute_children()
{
	// compute a client rect
	RECT bounds;
	bounds.top = bounds.left = 0;
	bounds.right = m_views[0]->maxwidth() + (2 * EDGE_WIDTH);
	bounds.bottom = 200;
	AdjustWindowRectEx(&bounds, DEBUG_WINDOW_STYLE, FALSE, DEBUG_WINDOW_STYLE_EX);

	// clamp the min/max size
	set_maxwidth(bounds.right - bounds.left);

	// get the parent's dimensions
	RECT parent;
	GetClientRect(window(), &parent);

	// view gets the remaining space
	InflateRect(&parent, -EDGE_WIDTH, -EDGE_WIDTH);
	m_views[0]->set_bounds(parent);
}
