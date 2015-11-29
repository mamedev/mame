// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  memorywininfo.c - Win32 debug window handling
//
//============================================================

#include "memorywininfo.h"

#include "debugviewinfo.h"
#include "memoryviewinfo.h"
#include "uimetrics.h"

#include "winutf8.h"


memorywin_info::memorywin_info(debugger_windows_interface &debugger) :
	editwin_info(debugger, false, "Memory", NULL),
	m_combownd(NULL)
{
	if (!window())
		return;

	m_views[0].reset(global_alloc(memoryview_info(debugger, *this, window())));
	if ((m_views[0] == NULL) || !m_views[0]->is_valid())
	{
		m_views[0].reset();
		return;
	}

	// create the options menu
	HMENU const optionsmenu = CreatePopupMenu();
	AppendMenu(optionsmenu, MF_ENABLED, ID_1_BYTE_CHUNKS, TEXT("1-byte chunks\tCtrl+1"));
	AppendMenu(optionsmenu, MF_ENABLED, ID_2_BYTE_CHUNKS, TEXT("2-byte chunks\tCtrl+2"));
	AppendMenu(optionsmenu, MF_ENABLED, ID_4_BYTE_CHUNKS, TEXT("4-byte chunks\tCtrl+4"));
	AppendMenu(optionsmenu, MF_ENABLED, ID_8_BYTE_CHUNKS, TEXT("8-byte chunks\tCtrl+8"));
	AppendMenu(optionsmenu, MF_ENABLED, ID_FLOATING_POINT_32BIT, TEXT("32 bit floating point\tCtrl+9"));
	AppendMenu(optionsmenu, MF_DISABLED | MF_SEPARATOR, 0, TEXT(""));
	AppendMenu(optionsmenu, MF_ENABLED, ID_LOGICAL_ADDRESSES, TEXT("Logical Addresses\tCtrl+L"));
	AppendMenu(optionsmenu, MF_ENABLED, ID_PHYSICAL_ADDRESSES, TEXT("Physical Addresses\tCtrl+Y"));
	AppendMenu(optionsmenu, MF_DISABLED | MF_SEPARATOR, 0, TEXT(""));
	AppendMenu(optionsmenu, MF_ENABLED, ID_REVERSE_VIEW, TEXT("Reverse View\tCtrl+R"));
	AppendMenu(optionsmenu, MF_DISABLED | MF_SEPARATOR, 0, TEXT(""));
	AppendMenu(optionsmenu, MF_ENABLED, ID_INCREASE_MEM_WIDTH, TEXT("Increase bytes per line\tCtrl+P"));
	AppendMenu(optionsmenu, MF_ENABLED, ID_DECREASE_MEM_WIDTH, TEXT("Decrease bytes per line\tCtrl+O"));
	AppendMenu(GetMenu(window()), MF_ENABLED | MF_POPUP, (UINT_PTR)optionsmenu, TEXT("Options"));

	// set up the view to track the initial expression
	downcast<memoryview_info *>(m_views[0].get())->set_expression("0");
	set_edit_defstr("0");
	set_editwnd_text("0");
	editwnd_select_all();

	// create a combo box
	m_views[0]->set_source_for_visible_cpu();
	m_combownd = m_views[0]->create_source_combobox(window(), (LONG_PTR)this);

	// set the caption
	update_caption();

	// recompute the children once to get the maxwidth
	memorywin_info::recompute_children();

	// position the window and recompute children again
	SetWindowPos(window(), HWND_TOP, 100, 100, maxwidth(), 200, SWP_SHOWWINDOW);
	memorywin_info::recompute_children();

	// mark the edit box as the default focus and set it
	editwin_info::set_default_focus();
}


memorywin_info::~memorywin_info()
{
}


bool memorywin_info::handle_key(WPARAM wparam, LPARAM lparam)
{
	if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
	{
		switch (wparam)
		{
		case '1':
			SendMessage(window(), WM_COMMAND, ID_1_BYTE_CHUNKS, 0);
			return true;

		case '2':
			SendMessage(window(), WM_COMMAND, ID_2_BYTE_CHUNKS, 0);
			return true;

		case '4':
			SendMessage(window(), WM_COMMAND, ID_4_BYTE_CHUNKS, 0);
			return true;

		case '8':
			SendMessage(window(), WM_COMMAND, ID_8_BYTE_CHUNKS, 0);
			return true;

		case '9':
			SendMessage(window(), WM_COMMAND, ID_FLOATING_POINT_32BIT, 0);
			return true;

		case 'L':
			SendMessage(window(), WM_COMMAND, ID_LOGICAL_ADDRESSES, 0);
			return true;

		case 'Y':
			SendMessage(window(), WM_COMMAND, ID_PHYSICAL_ADDRESSES, 0);
			return true;

		case 'R':
			SendMessage(window(), WM_COMMAND, ID_REVERSE_VIEW, 0);
			return true;

		case 'P':
			SendMessage(window(), WM_COMMAND, ID_INCREASE_MEM_WIDTH, 0);
			return true;

		case 'O':
			SendMessage(window(), WM_COMMAND, ID_DECREASE_MEM_WIDTH, 0);
			return true;
		}
	}
	return editwin_info::handle_key(wparam, lparam);
}


void memorywin_info::recompute_children()
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

	// memory view gets the rest
	RECT memrect;
	memrect.top = editrect.bottom + (2 * EDGE_WIDTH);
	memrect.bottom = parent.bottom - EDGE_WIDTH;
	memrect.left = parent.left + EDGE_WIDTH;
	memrect.right = parent.right - EDGE_WIDTH;

	// set the bounds of things
	m_views[0]->set_bounds(memrect);
	set_editwnd_bounds(editrect);
	smart_set_window_bounds(m_combownd, window(), comborect);
}


void memorywin_info::update_menu()
{
	editwin_info::update_menu();

	memoryview_info *const memview = downcast<memoryview_info *>(m_views[0].get());
	HMENU const menu = GetMenu(window());
	CheckMenuItem(menu, ID_1_BYTE_CHUNKS, MF_BYCOMMAND | (memview->data_format() == 1 ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(menu, ID_2_BYTE_CHUNKS, MF_BYCOMMAND | (memview->data_format() == 2 ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(menu, ID_4_BYTE_CHUNKS, MF_BYCOMMAND | (memview->data_format() == 4 ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(menu, ID_8_BYTE_CHUNKS, MF_BYCOMMAND | (memview->data_format() == 8 ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(menu, ID_FLOATING_POINT_32BIT, MF_BYCOMMAND | (memview->data_format() == 9 ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(menu, ID_LOGICAL_ADDRESSES, MF_BYCOMMAND | (memview->physical() ? MF_UNCHECKED : MF_CHECKED));
	CheckMenuItem(menu, ID_PHYSICAL_ADDRESSES, MF_BYCOMMAND | (memview->physical() ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(menu, ID_REVERSE_VIEW, MF_BYCOMMAND | (memview->reverse() ? MF_CHECKED : MF_UNCHECKED));
	EnableMenuItem(menu, ID_DECREASE_MEM_WIDTH, MF_BYCOMMAND | ((memview->chunks_per_row() > 1) ? MF_ENABLED : MF_GRAYED));
}


bool memorywin_info::handle_command(WPARAM wparam, LPARAM lparam)
{
	memoryview_info *const memview = downcast<memoryview_info *>(m_views[0].get());
	switch (HIWORD(wparam))
	{
	// combo box selection changed
	case CBN_SELCHANGE:
		{
			int const sel = SendMessage((HWND)lparam, CB_GETCURSEL, 0, 0);
			if (sel != CB_ERR)
			{
				memview->set_source_index(sel);
				update_caption();

				// reset the focus
				set_default_focus();
				return true;
			}
			break;
		}

	// menu selections
	case 0:
		switch (LOWORD(wparam))
		{
		case ID_1_BYTE_CHUNKS:
			memview->set_data_format(1);
			return true;

		case ID_2_BYTE_CHUNKS:
			memview->set_data_format(2);
			return true;

		case ID_4_BYTE_CHUNKS:
			memview->set_data_format(4);
			return true;

		case ID_8_BYTE_CHUNKS:
			memview->set_data_format(8);
			return true;

		case ID_FLOATING_POINT_32BIT:
			memview->set_data_format(9);
			return true;

		case ID_LOGICAL_ADDRESSES:
			memview->set_physical(false);
			return true;

		case ID_PHYSICAL_ADDRESSES:
			memview->set_physical(true);
			return true;

		case ID_REVERSE_VIEW:
			memview->set_reverse(!memview->reverse());
			return true;

		case ID_INCREASE_MEM_WIDTH:
			memview->set_chunks_per_row(memview->chunks_per_row() + 1);
			recompute_children();
			return true;

		case ID_DECREASE_MEM_WIDTH:
			memview->set_chunks_per_row(memview->chunks_per_row() - 1);
			recompute_children();
			return true;
		}
		break;
	}
	return editwin_info::handle_command(wparam, lparam);
}


void memorywin_info::draw_contents(HDC dc)
{
	editwin_info::draw_contents(dc);
	if (m_combownd)
		draw_border(dc, m_combownd);
}


void memorywin_info::process_string(char const *string)
{
	// set the string to the memory view
	downcast<memoryview_info *>(m_views[0].get())->set_expression(string);

	// select everything in the edit text box
	editwnd_select_all();

	// update the default string to match
	set_edit_defstr(string);
}


void memorywin_info::update_caption()
{
	win_set_window_text_utf8(window(), std::string("Memory: ").append(m_views[0]->source_name()).c_str());
}
