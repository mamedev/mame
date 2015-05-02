// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  pointswininfo.c - Win32 debug window handling
//
//============================================================

#include "pointswininfo.h"

#include "debugviewinfo.h"

#include "winutf8.h"


pointswin_info::pointswin_info(debugger_windows_interface &debugger) :
	debugwin_info(debugger, false, std::string("All Breakpoints").c_str(), NULL)
{
	if (!window())
		return;

	m_views[0].reset(global_alloc(debugview_info(debugger, *this, window(), DVT_BREAK_POINTS)));
	if ((m_views[0] == NULL) || !m_views[0]->is_valid())
	{
		m_views[0].reset();
		return;
	}

	// create the options menu
	HMENU const optionsmenu = CreatePopupMenu();
	AppendMenu(optionsmenu, MF_ENABLED, ID_SHOW_BREAKPOINTS, TEXT("Breakpoints\tCtrl+1"));
	AppendMenu(optionsmenu, MF_ENABLED, ID_SHOW_WATCHPOINTS, TEXT("Watchpoints\tCtrl+2"));
	AppendMenu(GetMenu(window()), MF_ENABLED | MF_POPUP, (UINT_PTR)optionsmenu, TEXT("Options"));

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
	recompute_children();
}


pointswin_info::~pointswin_info()
{
}


bool pointswin_info::handle_key(WPARAM wparam, LPARAM lparam)
{
	if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
	{
		switch (wparam)
		{
		case '1':
			SendMessage(window(), WM_COMMAND, ID_SHOW_BREAKPOINTS, 0);
			return true;

		case '2':
			SendMessage(window(), WM_COMMAND, ID_SHOW_WATCHPOINTS, 0);
			return true;
		}
	}

	return debugwin_info::handle_key(wparam, lparam);
}


void pointswin_info::update_menu()
{
	debugwin_info::update_menu();

	HMENU const menu = GetMenu(window());
	CheckMenuItem(menu, ID_SHOW_BREAKPOINTS, MF_BYCOMMAND | (m_views[0]->type() == DVT_BREAK_POINTS ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(menu, ID_SHOW_WATCHPOINTS, MF_BYCOMMAND | (m_views[0]->type() == DVT_WATCH_POINTS ? MF_CHECKED : MF_UNCHECKED));
}


bool pointswin_info::handle_command(WPARAM wparam, LPARAM lparam)
{
	switch (HIWORD(wparam))
	{
	// menu selections
	case 0:
		switch (LOWORD(wparam))
		{
		case ID_SHOW_BREAKPOINTS:
			m_views[0].reset();
			m_views[0].reset(global_alloc(debugview_info(debugger(), *this, window(), DVT_BREAK_POINTS)));
			if (!m_views[0]->is_valid())
				m_views[0].reset();
			win_set_window_text_utf8(window(), "All Breakpoints");
			recompute_children();
			return true;

		case ID_SHOW_WATCHPOINTS:
			m_views[0].reset();
			m_views[0].reset(global_alloc(debugview_info(debugger(), *this, window(), DVT_WATCH_POINTS)));
			if (!m_views[0]->is_valid())
				m_views[0].reset();
			win_set_window_text_utf8(window(), "All Watchpoints");
			recompute_children();
			return true;
		}
		break;
	}
	return debugwin_info::handle_command(wparam, lparam);
}
