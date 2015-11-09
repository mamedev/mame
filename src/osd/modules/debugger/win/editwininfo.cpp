// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  editwininfo.c - Win32 debug window handling
//
//============================================================

#include "editwininfo.h"

#include "debugviewinfo.h"
#include "uimetrics.h"

#include "strconv.h"

#include "winutil.h"


// edit box styles
#define EDIT_BOX_STYLE      WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL
#define EDIT_BOX_STYLE_EX   0


editwin_info::editwin_info(debugger_windows_interface &debugger, bool is_main_console, LPCSTR title, WNDPROC handler) :
	debugwin_info(debugger, is_main_console, title, handler),
	m_editwnd(NULL),
	m_edit_defstr(),
	m_original_editproc(NULL),
	m_history_count(0),
	m_last_history(0)
{
	if (window() == NULL)
		return;

	// create an edit box and override its key handling
	m_editwnd = CreateWindowEx(EDIT_BOX_STYLE_EX, TEXT("EDIT"), NULL, EDIT_BOX_STYLE,
			0, 0, 100, 100, window(), NULL, GetModuleHandleUni(), NULL);
	m_original_editproc = (WNDPROC)(FPTR)GetWindowLongPtr(m_editwnd, GWLP_WNDPROC);
	SetWindowLongPtr(m_editwnd, GWLP_USERDATA, (LONG_PTR)this);
	SetWindowLongPtr(m_editwnd, GWLP_WNDPROC, (LONG_PTR)&editwin_info::static_edit_proc);
	SendMessage(m_editwnd, WM_SETFONT, (WPARAM)metrics().debug_font(), (LPARAM)FALSE);
	SendMessage(m_editwnd, EM_LIMITTEXT, (WPARAM)MAX_EDIT_STRING, (LPARAM)0);
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
	TCHAR *tc_buffer = tstring_from_utf8(text);
	if (tc_buffer != NULL)
	{
		SendMessage(m_editwnd, WM_SETTEXT, (WPARAM)0, (LPARAM)tc_buffer);
		osd_free(tc_buffer);
	}
}


void editwin_info::editwnd_select_all()
{
	SendMessage(m_editwnd, EM_SETSEL, (WPARAM)0, (LPARAM)-1);
}


void editwin_info::draw_contents(HDC dc)
{
	debugwin_info::draw_contents(dc);
	if (m_editwnd)
		draw_border(dc, m_editwnd);
}


LRESULT editwin_info::edit_proc(UINT message, WPARAM wparam, LPARAM lparam)
{
	TCHAR buffer[MAX_EDIT_STRING];

	// handle a few messages
	switch (message)
	{
	// key down: handle navigation in the attached view
	case WM_SYSKEYDOWN:
		if (wparam != VK_F10)
			return CallWindowProc(m_original_editproc, m_editwnd, message, wparam, lparam);
		// (fall through)
	case WM_KEYDOWN:
		switch (wparam)
		{
		case VK_UP:
			if (m_last_history < (m_history_count - 1))
				m_last_history++;
			else
				m_last_history = 0;
			SendMessage(m_editwnd, WM_SETTEXT, (WPARAM)0, (LPARAM)&m_history[m_last_history][0]);
			SendMessage(m_editwnd, EM_SETSEL, (WPARAM)MAX_EDIT_STRING, (LPARAM)MAX_EDIT_STRING);
			break;

		case VK_DOWN:
			if (m_last_history > 0)
				m_last_history--;
			else if (m_history_count > 0)
				m_last_history = m_history_count - 1;
			else
				m_last_history = 0;
			SendMessage(m_editwnd, WM_SETTEXT, (WPARAM)0, (LPARAM)&m_history[m_last_history][0]);
			SendMessage(m_editwnd, EM_SETSEL, (WPARAM)MAX_EDIT_STRING, (LPARAM)MAX_EDIT_STRING);
			break;

		case VK_PRIOR:
			if (m_views[0] != NULL)
				m_views[0]->send_pageup();
			break;

		case VK_NEXT:
			if (m_views[0] != NULL)
				m_views[0]->send_pagedown();
			break;

		case VK_TAB:
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
				prev_view(NULL);
			else
				next_view(NULL);
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
				switch (wparam)
				{
				case 13:
					{
						// fetch the text
						SendMessage(m_editwnd, WM_GETTEXT, (WPARAM)ARRAY_LENGTH(buffer), (LPARAM)buffer);

						// add to the history if it's not a repeat of the last one
						if (buffer[0] != 0 && _tcscmp(buffer, &m_history[0][0]))
						{
							memmove(&m_history[1][0], &m_history[0][0], (HISTORY_LENGTH - 1) * MAX_EDIT_STRING * sizeof(TCHAR));
							_tcscpy(&m_history[0][0], buffer);
							if (m_history_count < HISTORY_LENGTH)
								m_history_count++;
						}
						m_last_history = m_history_count - 1;

						// process
						char *utf8_buffer = utf8_from_tstring(buffer);
						if (utf8_buffer != NULL)
						{
							process_string(utf8_buffer);
							osd_free(utf8_buffer);
						}
						break;
					}

				case 27:
					{
						// fetch the text
						SendMessage(m_editwnd, WM_GETTEXT, (WPARAM)sizeof(buffer), (LPARAM)buffer);

						// if it's not empty, clear the text
						if (_tcslen(buffer) > 0)
						{
							set_ignore_char_lparam(lparam);
							set_editwnd_text(m_edit_defstr.c_str());
							editwnd_select_all();
						}
						break;
					}

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
	editwin_info *const info = (editwin_info *)(FPTR)GetWindowLongPtr(wnd, GWLP_USERDATA);
	assert(info->m_editwnd == wnd);
	return info->edit_proc(message, wparam, lparam);
}
