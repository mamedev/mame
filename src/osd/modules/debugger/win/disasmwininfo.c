// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  disasmwininfo.c - Win32 debug window handling
//
//============================================================

#include "disasmwininfo.h"

#include "debugviewinfo.h"
#include "disasmviewinfo.h"
#include "uimetrics.h"

#include "debugger.h"
#include "debug/debugcon.h"
#include "debug/debugcpu.h"

#include "winutf8.h"


disasmwin_info::disasmwin_info(debugger_windows_interface &debugger) :
	disasmbasewin_info(debugger, false, "Disassembly", NULL),
	m_combownd(NULL)
{
	if ((window() == NULL) || (m_views[0] == NULL))
		return;

	// set up the view to track the initial expression
	set_edit_defstr("curpc");
	set_editwnd_text("curpc");
	editwnd_select_all();

	// create a combo box
	m_combownd = m_views[0]->create_source_combobox(window(), (LONG_PTR)this);

	// set the caption
	update_caption();

	// recompute the children once to get the maxwidth
	recompute_children();

	// position the window and recompute children again
	SetWindowPos(window(), HWND_TOP, 100, 100, maxwidth(), 200, SWP_SHOWWINDOW);
	recompute_children();

	// mark the edit box as the default focus and set it
	set_default_focus();
}


disasmwin_info::~disasmwin_info()
{
}


void disasmwin_info::recompute_children()
{
	// compute a client rect
	RECT bounds;
	bounds.top = bounds.left = 0;
	bounds.right = m_views[0]->prefwidth() + (2 * EDGE_WIDTH);
	bounds.bottom = 200;
	AdjustWindowRectEx(&bounds, DEBUG_WINDOW_STYLE, FALSE, DEBUG_WINDOW_STYLE_EX);

	// clamp the min/max size
	set_maxwidth(bounds.right - bounds.left);

	// get the parent's dimensions
	RECT parent;
	GetClientRect(window(), &parent);

	// edit box gets half of the width
	RECT editrect;
	editrect.top = parent.top + EDGE_WIDTH;
	editrect.bottom = editrect.top + metrics().debug_font_height() + 4;
	editrect.left = parent.left + EDGE_WIDTH;
	editrect.right = parent.left + ((parent.right - parent.left) / 2) - EDGE_WIDTH;

	// combo box gets the other half of the width
	RECT comborect;
	comborect.top = editrect.top;
	comborect.bottom = editrect.bottom;
	comborect.left = editrect.right + (2 * EDGE_WIDTH);
	comborect.right = parent.right - EDGE_WIDTH;

	// disasm view gets the rest
	RECT dasmrect;
	dasmrect.top = editrect.bottom + (2 * EDGE_WIDTH);
	dasmrect.bottom = parent.bottom - EDGE_WIDTH;
	dasmrect.left = parent.left + EDGE_WIDTH;
	dasmrect.right = parent.right - EDGE_WIDTH;

	// set the bounds of things
	m_views[0]->set_bounds(dasmrect);
	set_editwnd_bounds(editrect);
	smart_set_window_bounds(m_combownd, window(), comborect);
}


bool disasmwin_info::handle_command(WPARAM wparam, LPARAM lparam)
{
	switch (HIWORD(wparam))
	{
	// combo box selection changed
	case CBN_SELCHANGE:
		{
			int const sel = SendMessage((HWND)lparam, CB_GETCURSEL, 0, 0);
			if (sel != CB_ERR)
			{
				m_views[0]->set_source_index(sel);
				update_caption();

				// reset the focus
				set_default_focus();
				return true;
			}
			break;
		}
	}
	return disasmbasewin_info::handle_command(wparam, lparam);
}


void disasmwin_info::draw_contents(HDC dc)
{
	disasmbasewin_info::draw_contents(dc);
	if (m_combownd)
		draw_border(dc, m_combownd);
}


void disasmwin_info::process_string(char const *string)
{
	// set the string to the disasm view
	downcast<disasmview_info *>(m_views[0].get())->set_expression(string);

	// select everything in the edit text box
	editwnd_select_all();

	// update the default string to match
	set_edit_defstr(string);
}


void disasmwin_info::update_caption()
{
	win_set_window_text_utf8(window(), std::string("Disassembly: ").append(m_views[0]->source_name()).c_str());
}
