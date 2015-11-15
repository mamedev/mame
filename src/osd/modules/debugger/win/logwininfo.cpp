// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  logwininfo.c - Win32 debug window handling
//
//============================================================

#include "logwininfo.h"

#include "debugviewinfo.h"


logwin_info::logwin_info(debugger_windows_interface &debugger) :
	debugwin_info(debugger, false, std::string("Errorlog: ").append(debugger.machine().system().description).append(" [").append(debugger.machine().system().name).append("]").c_str(), NULL)
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
	bounds.right = m_views[0]->maxwidth() + (2 * EDGE_WIDTH);
	bounds.bottom = 200;
	AdjustWindowRectEx(&bounds, DEBUG_WINDOW_STYLE, FALSE, DEBUG_WINDOW_STYLE_EX);

	// clamp the min/max size
	set_maxwidth(bounds.right - bounds.left);

	// position the window at the bottom-right
	SetWindowPos(window(), HWND_TOP, 100, 100, bounds.right - bounds.left, bounds.bottom - bounds.top, SWP_SHOWWINDOW);

	// recompute the children
	debugwin_info::recompute_children();
}


logwin_info::~logwin_info()
{
}
