// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  pointswininfo.cpp - Win32 debug window handling
//
//============================================================

#include "emu.h"
#include "pointswininfo.h"

#include "debugviewinfo.h"

#include "util/xmlfile.h"

#include "winutf8.h"


namespace osd::debugger::win {

pointswin_info::pointswin_info(debugger_windows_interface &debugger) :
	debugwin_info(debugger, false, std::string("All Breakpoints").c_str(), nullptr)
{
	if (!window())
		return;

	m_views[0].reset(new debugview_info(debugger, *this, window(), DVT_BREAK_POINTS));
	if ((m_views[0] == nullptr) || !m_views[0]->is_valid())
	{
		m_views[0].reset();
		return;
	}

	// create the options menu
	HMENU const optionsmenu = CreatePopupMenu();
	AppendMenu(optionsmenu, MF_ENABLED, ID_SHOW_BREAKPOINTS, TEXT("Breakpoints\tCtrl+1"));
	AppendMenu(optionsmenu, MF_ENABLED, ID_SHOW_WATCHPOINTS, TEXT("Watchpoints\tCtrl+2"));
	AppendMenu(optionsmenu, MF_ENABLED, ID_SHOW_REGISTERPOINTS, TEXT("Registerpoints\tCtrl+3"));
	AppendMenu(optionsmenu, MF_ENABLED, ID_SHOW_EXCEPTIONPOINTS, TEXT("Exceptionpoints\tCtrl+4"));
	AppendMenu(GetMenu(window()), MF_ENABLED | MF_POPUP, (UINT_PTR)optionsmenu, TEXT("Options"));

	// compute a client rect
	RECT bounds;
	bounds.top = bounds.left = 0;
	bounds.right = m_views[0]->maxwidth() + (2 * EDGE_WIDTH);
	bounds.bottom = 200;
	AdjustWindowRectEx(&bounds, DEBUG_WINDOW_STYLE, FALSE, DEBUG_WINDOW_STYLE_EX);

	// clamp the min/max size
	set_maxwidth(bounds.right - bounds.left);

	// position the window and recompute children
	debugger.stagger_window(window(), bounds.right - bounds.left, bounds.bottom - bounds.top);
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

		case '3':
			SendMessage(window(), WM_COMMAND, ID_SHOW_REGISTERPOINTS, 0);
			return true;

		case '4':
			SendMessage(window(), WM_COMMAND, ID_SHOW_EXCEPTIONPOINTS, 0);
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
	CheckMenuItem(menu, ID_SHOW_REGISTERPOINTS, MF_BYCOMMAND | (m_views[0]->type() == DVT_REGISTER_POINTS ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(menu, ID_SHOW_EXCEPTIONPOINTS, MF_BYCOMMAND | (m_views[0]->type() == DVT_EXCEPTION_POINTS ? MF_CHECKED : MF_UNCHECKED));
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
			m_views[0].reset(new debugview_info(debugger(), *this, window(), DVT_BREAK_POINTS));
			if (!m_views[0]->is_valid())
				m_views[0].reset();
			win_set_window_text_utf8(window(), "All Breakpoints");
			recompute_children();
			return true;

		case ID_SHOW_WATCHPOINTS:
			m_views[0].reset();
			m_views[0].reset(new debugview_info(debugger(), *this, window(), DVT_WATCH_POINTS));
			if (!m_views[0]->is_valid())
				m_views[0].reset();
			win_set_window_text_utf8(window(), "All Watchpoints");
			recompute_children();
			return true;

		case ID_SHOW_REGISTERPOINTS:
			m_views[0].reset();
			m_views[0].reset(new debugview_info(debugger(), *this, window(), DVT_REGISTER_POINTS));
			if (!m_views[0]->is_valid())
				m_views[0].reset();
			win_set_window_text_utf8(window(), "All Registerpoints");
			recompute_children();
			return true;

		case ID_SHOW_EXCEPTIONPOINTS:
			m_views[0].reset();
			m_views[0].reset(new debugview_info(debugger(), *this, window(), DVT_EXCEPTION_POINTS));
			if (!m_views[0]->is_valid())
				m_views[0].reset();
			win_set_window_text_utf8(window(), "All Exceptionpoints");
			recompute_children();
			return true;
		}
		break;
	}
	return debugwin_info::handle_command(wparam, lparam);
}


void pointswin_info::restore_configuration_from_node(util::xml::data_node const &node)
{
	switch (node.get_attribute_int(ATTR_WINDOW_POINTS_TYPE, -1))
	{
	case 0:
		SendMessage(window(), WM_COMMAND, ID_SHOW_BREAKPOINTS, 0);
		break;
	case 1:
		SendMessage(window(), WM_COMMAND, ID_SHOW_WATCHPOINTS, 0);
		break;
	case 2:
		SendMessage(window(), WM_COMMAND, ID_SHOW_REGISTERPOINTS, 0);
		break;
	case 3:
		SendMessage(window(), WM_COMMAND, ID_SHOW_EXCEPTIONPOINTS, 0);
		break;
	}

	debugwin_info::restore_configuration_from_node(node);
}


void pointswin_info::save_configuration_to_node(util::xml::data_node &node)
{
	debugwin_info::save_configuration_to_node(node);

	node.set_attribute_int(ATTR_WINDOW_TYPE, WINDOW_TYPE_POINTS_VIEWER);
	switch (m_views[0]->type())
	{
	case DVT_BREAK_POINTS:
		node.set_attribute_int(ATTR_WINDOW_POINTS_TYPE, 0);
		break;
	case DVT_WATCH_POINTS:
		node.set_attribute_int(ATTR_WINDOW_POINTS_TYPE, 1);
		break;
	case DVT_REGISTER_POINTS:
		node.set_attribute_int(ATTR_WINDOW_POINTS_TYPE, 2);
		break;
	case DVT_EXCEPTION_POINTS:
		node.set_attribute_int(ATTR_WINDOW_POINTS_TYPE, 3);
		break;
	default:
		break;
	}
}

} // namespace osd::debugger::win
