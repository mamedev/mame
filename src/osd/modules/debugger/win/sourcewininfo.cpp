// license:BSD-3-Clause
// copyright-holders:David Broman
//============================================================
//
//  sourcewininfo.cpp - Win32 debug window handling
//
//============================================================

#include "emu.h"
#include "sourcewininfo.h"
#include "uimetrics.h"
#include "debugger.h"

#include "sourceviewinfo.h"

#include "debug/dvsourcecode.h"
#include "debug/debugcon.h"

namespace osd::debugger::win {

sourcewin_info::sourcewin_info(debugger_windows_interface &debugger, bool is_main_console, LPCSTR title, WNDPROC handler) :
	disasmbasewin_info(debugger, is_main_console, title, handler),
	m_filecombownd(nullptr)
{
	if (!window())
		return;

	m_views[VIEW_IDX_SOURCE].reset(new sourceview_info(debugger, *this, window() /* , DVT_SOURCE */));
	if ((m_views[VIEW_IDX_SOURCE] == nullptr) || !m_views[VIEW_IDX_SOURCE]->is_valid())
	{
		m_views[VIEW_IDX_SOURCE].reset();
		return;
	}
	m_views[VIEW_IDX_SOURCE]->set_source_for_visible_cpu();

	m_filecombownd = downcast<sourceview_info *>(m_views[VIEW_IDX_SOURCE].get())->
		create_source_file_combobox(window(), (LONG_PTR)this);

	// Add source-debugging commands to options menu
	AppendMenu(m_optionsmenu, MF_DISABLED | MF_SEPARATOR, 0, TEXT(""));
	AppendMenu(m_optionsmenu, MF_ENABLED, ID_SHOW_SOURCE, TEXT("Show source\tCtrl+U"));
	AppendMenu(m_optionsmenu, MF_ENABLED, ID_SHOW_DISASM, TEXT("Show disassembly\tCtrl+Shift+U"));
}


sourcewin_info::~sourcewin_info()
{
}


void sourcewin_info::set_srcwnd_bounds(RECT const &bounds)
{
	// combo box gets full width
	RECT comborect;
	comborect.top = bounds.top + EDGE_WIDTH;
	comborect.bottom = comborect.top + metrics().debug_font_height() + 4;
	comborect.left = bounds.left;
	comborect.right = bounds.right;

	// source view gets the rest
	RECT srcrect;
	srcrect.top = comborect.bottom + (2 * EDGE_WIDTH);
	srcrect.bottom = bounds.bottom - EDGE_WIDTH;
	srcrect.left = bounds.left;
	srcrect.right = bounds.right;

	// set the bounds of things
	smart_set_window_bounds(m_filecombownd, window(), comborect);
	m_views[VIEW_IDX_SOURCE]->set_bounds(srcrect);
}


bool sourcewin_info::show_src_window()
{
	m_views[VIEW_IDX_SOURCE]->show();
	if (machine().debugger().srcdbg_provider() != nullptr)
	{
		smart_show_window(m_filecombownd, true);
	}
	return true;
}


void sourcewin_info::hide_src_window()
{
	m_views[VIEW_IDX_SOURCE]->hide();
	smart_show_window(m_filecombownd, false);
}


void sourcewin_info::draw_contents(HDC dc)
{
	disasmbasewin_info::draw_contents(dc);
	draw_border(dc, m_filecombownd);
}


bool sourcewin_info::handle_key(WPARAM wparam, LPARAM lparam)
{
	if (GetAsyncKeyState(VK_CONTROL) & 0x8000 && wparam == 'U')
	{
		if (GetAsyncKeyState(VK_SHIFT))
		{
			SendMessage(window(), WM_COMMAND, ID_SHOW_DISASM, 0);
		}
		else
		{
			SendMessage(window(), WM_COMMAND, ID_SHOW_SOURCE, 0);

		}
		return true;
	}

	return disasmbasewin_info::handle_key(wparam, lparam);
}


// Helper called by handle_command to handle commands specific to
// only the source-code window
bool sourcewin_info::handle_sourcewin_command(WPARAM wparam, LPARAM lparam)
{
	if (HIWORD(wparam) == 0)
	{
		switch (LOWORD(wparam))
		{
		case ID_STEP:
			machine().debugger().console().get_visible_cpu()->debug()->single_step(1, true /* source stepping */);
			return true;

		case ID_STEP_OVER:
			machine().debugger().console().get_visible_cpu()->debug()->single_step_over(1, true /* source stepping */);
			return true;

		case ID_STEP_OUT:
			machine().debugger().console().get_visible_cpu()->debug()->single_step_out(true /* source stepping */);
			return true;
		}
	}

	switch (HIWORD(wparam))
	{
	case CBN_SELCHANGE:
		// Source file combo box selection changed
		{
			int const sel = SendMessage((HWND)lparam, CB_GETCURSEL, 0, 0);
			if (sel == CB_ERR)
				break;

			downcast<sourceview_info *>(m_views[VIEW_IDX_SOURCE].get())->set_src_index(u16(sel));

			// reset the focus
			set_default_focus();
			return true;
		}
	}

	return false;
}



bool sourcewin_info::handle_command(WPARAM wparam, LPARAM lparam)
{
	if (machine().debugger().srcdbg_provider() != nullptr &&
		m_views[VIEW_IDX_SOURCE] != nullptr &&
		m_views[VIEW_IDX_SOURCE]->is_visible() &&
		handle_sourcewin_command(wparam, lparam))
	{
		return true;
	}

	return disasmbasewin_info::handle_command(wparam, lparam);
}


} // namespace osd::debugger::win
