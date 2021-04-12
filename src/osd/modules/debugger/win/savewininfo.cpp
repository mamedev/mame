// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  savewininfo.c - Win32 debug window handling
//
//============================================================

#include "emu.h"
#include "savewininfo.h"

#include "debugviewinfo.h"
#include "saveviewinfo.h"


savewin_info::savewin_info(debugger_windows_interface &debugger) :
	debugwin_info(debugger, false, "Save State Data", nullptr)
{
	if (!window())
		return;

	m_views[0].reset(new saveview_info(debugger, *this, window()));
	if ((m_views[0] == nullptr) || !m_views[0]->is_valid())
	{
		m_views[0].reset();
		return;
	}

	// create the save menu
	HMENU const savemenu = CreatePopupMenu();
	AppendMenu(GetMenu(window()), MF_ENABLED | MF_POPUP, (UINT_PTR)savemenu, TEXT("Save"));

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


savewin_info::~savewin_info()
{
}

bool savewin_info::handle_command(WPARAM wparam, LPARAM lparam)
{
	return debugwin_info::handle_command(wparam, lparam);
}
