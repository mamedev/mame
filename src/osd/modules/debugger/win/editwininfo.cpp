// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  editwininfo.cpp - Win32 debug window handling
//
//============================================================

#include "emu.h"
#include "editwininfo.h"

#include "debugviewinfo.h"
#include "uimetrics.h"

#include "xmlfile.h"

#include "strconv.h"

#include "winutil.h"


namespace osd::debugger::win {

namespace {

constexpr DWORD EDIT_BOX_STYLE      = WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL;
constexpr DWORD EDIT_BOX_STYLE_EX   = 0;

constexpr int   MAX_EDIT_STRING     = 256;
constexpr int   HISTORY_LENGTH      = 100;

} // anonymous namespace


editwin_info::editwin_info(debugger_windows_interface &debugger, bool is_main_console, int viewidx, LPCSTR title, WNDPROC handler) :
	debugwin_info(debugger, is_main_console, title, handler),
	m_editwnd(nullptr),
	m_viewidx(viewidx),
	m_edit_defstr(),
	m_original_editproc(nullptr),
	m_history(),
	m_last_history(-1)
{
	if (window() == nullptr)
		return;

	// create an edit box and override its key handling
	m_editwnd = CreateWindowEx(EDIT_BOX_STYLE_EX, TEXT("EDIT"), nullptr, EDIT_BOX_STYLE,
			0, 0, 100, 100, window(), nullptr, GetModuleHandleUni(), nullptr);
	m_original_editproc = WNDPROC(uintptr_t(GetWindowLongPtr(m_editwnd, GWLP_WNDPROC)));
	SetWindowLongPtr(m_editwnd, GWLP_USERDATA, LONG_PTR(this));
	SetWindowLongPtr(m_editwnd, GWLP_WNDPROC, LONG_PTR(&editwin_info::static_edit_proc));
	SendMessage(m_editwnd, WM_SETFONT, WPARAM(metrics().debug_font()), LPARAM(FALSE));
	SendMessage(m_editwnd, EM_LIMITTEXT, WPARAM(MAX_EDIT_STRING), LPARAM(0));
	set_editwnd_text("");
}


editwin_info::~editwin_info()
{
}


bool editwin_info::restore_field(HWND wnd)
{
	if (wnd == m_editwnd)
	{
		set_editwnd_text(m_edit_defstr.c_str());
		editwnd_select_all();
		return true;
	}
	else
	{
		return false;
	}
}


bool editwin_info::set_default_focus()
{
	SetFocus(m_editwnd);
	return true;
}


void editwin_info::set_editwnd_bounds(RECT const &bounds)
{
	smart_set_window_bounds(m_editwnd, window(), bounds);
}


void editwin_info::set_editwnd_text(char const *text)
{
	auto tc_buffer = osd::text::to_tstring(text);
	SendMessage(m_editwnd, WM_SETTEXT, WPARAM(0), LPARAM(tc_buffer.c_str()));
}


void editwin_info::editwnd_select_all()
{
	SendMessage(m_editwnd, EM_SETSEL, WPARAM(0), LPARAM(-1));
}


void editwin_info::draw_contents(HDC dc)
{
	debugwin_info::draw_contents(dc);
	if (m_editwnd)
		draw_border(dc, m_editwnd);
}


void editwin_info::restore_configuration_from_node(util::xml::data_node const &node)
{
	m_history.clear();
	util::xml::data_node const *const hist = node.get_child(NODE_WINDOW_HISTORY);
	if (hist)
	{
		util::xml::data_node const *item = hist->get_child(NODE_HISTORY_ITEM);
		while (item)
		{
			if (item->get_value() && *item->get_value())
			{
				while (m_history.size() >= HISTORY_LENGTH)
					m_history.pop_back();
				m_history.emplace_front(osd::text::to_tstring(item->get_value()));
			}
			item = item->get_next_sibling(NODE_HISTORY_ITEM);
		}
	}
	m_last_history = -1;

	debugwin_info::restore_configuration_from_node(node);
}


void editwin_info::save_configuration_to_node(util::xml::data_node &node)
{
	debugwin_info::save_configuration_to_node(node);

	util::xml::data_node *const hist = node.add_child(NODE_WINDOW_HISTORY, nullptr);
	if (hist)
	{
		for (auto it = m_history.crbegin(); m_history.crend() != it; ++it)
			hist->add_child(NODE_HISTORY_ITEM, osd::text::from_tstring(*it).c_str());
	}
}


LRESULT editwin_info::edit_proc(UINT message, WPARAM wparam, LPARAM lparam)
{
	// handle a few messages
	switch (message)
	{
	// key down: handle navigation in the attached view
	case WM_SYSKEYDOWN:
		if (wparam != VK_F10)
			return CallWindowProc(m_original_editproc, m_editwnd, message, wparam, lparam);
		[[fallthrough]];
	case WM_KEYDOWN:
		switch (wparam)
		{
		case VK_UP:
			if (!m_history.empty())
			{
				m_last_history++;
				if (m_last_history >= m_history.size())
					m_last_history = 0;
				auto const &entry(m_history[m_last_history]);
				SendMessage(m_editwnd, WM_SETTEXT, WPARAM(0), LPARAM(entry.c_str()));
				SendMessage(m_editwnd, EM_SETSEL, WPARAM(entry.length()), LPARAM(entry.length()));
			}
			break;

		case VK_DOWN:
			if (!m_history.empty())
			{
				if (m_last_history > 0)
					m_last_history--;
				else
					m_last_history = m_history.size() - 1;
				auto const &entry(m_history[m_last_history]);
				SendMessage(m_editwnd, WM_SETTEXT, WPARAM(0), LPARAM(entry.c_str()));
				SendMessage(m_editwnd, EM_SETSEL, WPARAM(entry.length()), LPARAM(entry.length()));
			}
			break;

		case VK_PRIOR:
			if (m_views[m_viewidx] != nullptr)
				m_views[m_viewidx]->send_pageup();
			break;

		case VK_NEXT:
			if (m_views[m_viewidx] != nullptr)
				m_views[m_viewidx]->send_pagedown();
			break;

		case VK_TAB:
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
				prev_view(nullptr);
			else
				next_view(nullptr);
			set_ignore_char_lparam(lparam);
			break;

		default:
			if (handle_key(wparam, lparam))
				set_ignore_char_lparam(lparam);
			else
				return CallWindowProc(m_original_editproc, m_editwnd, message, wparam, lparam);
			break;
		}
		break;

	// char: handle the return key
	case WM_CHAR:

		// ignore chars associated with keys we've handled
		if (check_ignore_char_lparam(lparam))
		{
			if (waiting_for_debugger() || !seq_pressed())
			{
				TCHAR buffer[MAX_EDIT_STRING];

				switch (wparam)
				{
				case 13: // carriage return
					{
						// fetch the text
						SendMessage(m_editwnd, WM_GETTEXT, WPARAM(std::size(buffer)), LPARAM(buffer));

						// add to the history if it's not a repeat of the last one
						if (buffer[0] && (m_history.empty() || _tcscmp(buffer, m_history[0].c_str())))
						{
							while (m_history.size() >= HISTORY_LENGTH)
								m_history.pop_back();
							m_history.emplace_front(buffer);
						}
						m_last_history = -1;

						// process
						{
							auto utf8_buffer = osd::text::from_tstring(buffer);
							process_string(utf8_buffer);
						}
					}
					break;

				case 27: // escape
					{
						// fetch the text
						SendMessage(m_editwnd, WM_GETTEXT, WPARAM(sizeof(buffer)), LPARAM(buffer));

						// if it's not empty, clear the text
						if (_tcslen(buffer) > 0)
						{
							set_ignore_char_lparam(lparam);
							set_editwnd_text(m_edit_defstr.c_str());
							editwnd_select_all();
						}
					}
					break;

				default:
					return CallWindowProc(m_original_editproc, m_editwnd, message, wparam, lparam);
				}
			}
		}
		break;

	// everything else: defaults
	default:
		return CallWindowProc(m_original_editproc, m_editwnd, message, wparam, lparam);
	}

	return 0;
}


LRESULT CALLBACK editwin_info::static_edit_proc(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	auto *const info = (editwin_info *)uintptr_t(GetWindowLongPtr(wnd, GWLP_USERDATA));
	assert(info->m_editwnd == wnd);
	return info->edit_proc(message, wparam, lparam);
}

} // namespace osd::debugger::win
