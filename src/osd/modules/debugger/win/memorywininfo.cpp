// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  memorywininfo.cpp - Win32 debug window handling
//
//============================================================

#include "emu.h"
#include "memorywininfo.h"

#include "debugviewinfo.h"
#include "memoryviewinfo.h"
#include "uimetrics.h"

#include "util/xmlfile.h"

#include "winutf8.h"


namespace osd::debugger::win {

memorywin_info::memorywin_info(debugger_windows_interface &debugger) :
	editwin_info(debugger, false, 0 /* view index */, "Memory", nullptr),
	m_combownd(nullptr)
{
	if (!window())
		return;

	m_views[0].reset(new memoryview_info(debugger, *this, window()));
	if ((m_views[0] == nullptr) || !m_views[0]->is_valid())
	{
		m_views[0].reset();
		return;
	}

	// create the options menu
	HMENU const optionsmenu = CreatePopupMenu();
	AppendMenu(optionsmenu, MF_ENABLED, ID_1_BYTE_CHUNKS_HEX, TEXT("1-byte Chunks (Hex)\tCtrl+1"));
	AppendMenu(optionsmenu, MF_ENABLED, ID_2_BYTE_CHUNKS_HEX, TEXT("2-byte Chunks (Hex)\tCtrl+2"));
	AppendMenu(optionsmenu, MF_ENABLED, ID_4_BYTE_CHUNKS_HEX, TEXT("4-byte Chunks (Hex)\tCtrl+4"));
	AppendMenu(optionsmenu, MF_ENABLED, ID_8_BYTE_CHUNKS_HEX, TEXT("8-byte Chunks (Hex)\tCtrl+8"));
	AppendMenu(optionsmenu, MF_ENABLED, ID_1_BYTE_CHUNKS_OCT, TEXT("1-byte Chunks (Octal)\tCtrl+3"));
	AppendMenu(optionsmenu, MF_ENABLED, ID_2_BYTE_CHUNKS_OCT, TEXT("2-byte Chunks (Octal)\tCtrl+5"));
	AppendMenu(optionsmenu, MF_ENABLED, ID_4_BYTE_CHUNKS_OCT, TEXT("4-byte Chunks (Octal)\tCtrl+7"));
	AppendMenu(optionsmenu, MF_ENABLED, ID_8_BYTE_CHUNKS_OCT, TEXT("8-byte Chunks (Octal)\tCtrl+9"));
	AppendMenu(optionsmenu, MF_ENABLED, ID_FLOAT_32BIT, TEXT("32-bit Floating Point\tCtrl+Shift+F"));
	AppendMenu(optionsmenu, MF_ENABLED, ID_FLOAT_64BIT, TEXT("64-bit Floating Point\tCtrl+Shift+D"));
	AppendMenu(optionsmenu, MF_ENABLED, ID_FLOAT_80BIT, TEXT("80-bit Floating Point\tCtrl+Shift+E"));
	AppendMenu(optionsmenu, MF_DISABLED | MF_SEPARATOR, 0, TEXT(""));
	AppendMenu(optionsmenu, MF_ENABLED, ID_HEX_ADDRESSES, TEXT("Hexadecimal Addresses\tCtrl+Shift+H"));
	AppendMenu(optionsmenu, MF_ENABLED, ID_DEC_ADDRESSES, TEXT("Decimal Addresses"));
	AppendMenu(optionsmenu, MF_ENABLED, ID_OCT_ADDRESSES, TEXT("Octal Addresses\tCtrl+Shift+O"));
	AppendMenu(optionsmenu, MF_DISABLED | MF_SEPARATOR, 0, TEXT(""));
	AppendMenu(optionsmenu, MF_ENABLED, ID_LOGICAL_ADDRESSES, TEXT("Logical Addresses\tCtrl+L"));
	AppendMenu(optionsmenu, MF_ENABLED, ID_PHYSICAL_ADDRESSES, TEXT("Physical Addresses\tCtrl+Y"));
	AppendMenu(optionsmenu, MF_DISABLED | MF_SEPARATOR, 0, TEXT(""));
	AppendMenu(optionsmenu, MF_ENABLED, ID_REVERSE_VIEW, TEXT("Reverse View\tCtrl+R"));
	AppendMenu(optionsmenu, MF_DISABLED | MF_SEPARATOR, 0, TEXT(""));
	AppendMenu(optionsmenu, MF_ENABLED, ID_INCREASE_MEM_WIDTH, TEXT("Increase Bytes Per Line\tCtrl+P"));
	AppendMenu(optionsmenu, MF_ENABLED, ID_DECREASE_MEM_WIDTH, TEXT("Decrease Bytes Per Line\tCtrl+O"));
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
	recompute_children();

	// position the window and recompute children again
	debugger.stagger_window(window(), maxwidth(), 200);
	recompute_children();

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
		if (GetAsyncKeyState(VK_SHIFT))
		{
			switch (wparam)
			{
			case 'F':
				SendMessage(window(), WM_COMMAND, ID_FLOAT_32BIT, 0);
				return true;

			case 'D':
				SendMessage(window(), WM_COMMAND, ID_FLOAT_64BIT, 0);
				return true;

			case 'E':
				SendMessage(window(), WM_COMMAND, ID_FLOAT_80BIT, 0);
				return true;

			case 'H':
				SendMessage(window(), WM_COMMAND, ID_HEX_ADDRESSES, 0);
				return true;

			case 'O':
				SendMessage(window(), WM_COMMAND, ID_OCT_ADDRESSES, 0);
				return true;
			}
		}
		else
		{
			switch (wparam)
			{
			case '1':
				SendMessage(window(), WM_COMMAND, ID_1_BYTE_CHUNKS_HEX, 0);
				return true;

			case '2':
				SendMessage(window(), WM_COMMAND, ID_2_BYTE_CHUNKS_HEX, 0);
				return true;

			case '4':
				SendMessage(window(), WM_COMMAND, ID_4_BYTE_CHUNKS_HEX, 0);
				return true;

			case '8':
				SendMessage(window(), WM_COMMAND, ID_8_BYTE_CHUNKS_HEX, 0);
				return true;

			case '3':
				SendMessage(window(), WM_COMMAND, ID_1_BYTE_CHUNKS_OCT, 0);
				return true;

			case '5':
				SendMessage(window(), WM_COMMAND, ID_2_BYTE_CHUNKS_OCT, 0);
				return true;

			case '7':
				SendMessage(window(), WM_COMMAND, ID_4_BYTE_CHUNKS_OCT, 0);
				return true;

			case '9':
				SendMessage(window(), WM_COMMAND, ID_8_BYTE_CHUNKS_OCT, 0);
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

	auto *const memview = downcast<memoryview_info *>(m_views[0].get());
	HMENU const menu = GetMenu(window());

	CheckMenuItem(menu, ID_1_BYTE_CHUNKS_HEX, MF_BYCOMMAND | ((memview->data_format() == debug_view_memory::data_format::HEX_8BIT) ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(menu, ID_2_BYTE_CHUNKS_HEX, MF_BYCOMMAND | ((memview->data_format() == debug_view_memory::data_format::HEX_16BIT) ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(menu, ID_4_BYTE_CHUNKS_HEX, MF_BYCOMMAND | ((memview->data_format() == debug_view_memory::data_format::HEX_32BIT) ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(menu, ID_8_BYTE_CHUNKS_HEX, MF_BYCOMMAND | ((memview->data_format() == debug_view_memory::data_format::HEX_64BIT) ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(menu, ID_1_BYTE_CHUNKS_OCT, MF_BYCOMMAND | ((memview->data_format() == debug_view_memory::data_format::OCTAL_8BIT) ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(menu, ID_2_BYTE_CHUNKS_OCT, MF_BYCOMMAND | ((memview->data_format() == debug_view_memory::data_format::OCTAL_16BIT) ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(menu, ID_4_BYTE_CHUNKS_OCT, MF_BYCOMMAND | ((memview->data_format() == debug_view_memory::data_format::OCTAL_32BIT) ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(menu, ID_8_BYTE_CHUNKS_OCT, MF_BYCOMMAND | ((memview->data_format() == debug_view_memory::data_format::OCTAL_64BIT) ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(menu, ID_FLOAT_32BIT, MF_BYCOMMAND | ((memview->data_format() == debug_view_memory::data_format::FLOAT_32BIT) ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(menu, ID_FLOAT_64BIT, MF_BYCOMMAND | ((memview->data_format() == debug_view_memory::data_format::FLOAT_64BIT) ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(menu, ID_FLOAT_80BIT, MF_BYCOMMAND | ((memview->data_format() == debug_view_memory::data_format::FLOAT_80BIT) ? MF_CHECKED : MF_UNCHECKED));

	CheckMenuItem(menu, ID_HEX_ADDRESSES, MF_BYCOMMAND | ((memview->address_radix() == 16) ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(menu, ID_DEC_ADDRESSES, MF_BYCOMMAND | ((memview->address_radix() == 10) ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(menu, ID_OCT_ADDRESSES, MF_BYCOMMAND | ((memview->address_radix() == 8) ? MF_CHECKED : MF_UNCHECKED));

	CheckMenuItem(menu, ID_LOGICAL_ADDRESSES, MF_BYCOMMAND | (memview->physical() ? MF_UNCHECKED : MF_CHECKED));
	CheckMenuItem(menu, ID_PHYSICAL_ADDRESSES, MF_BYCOMMAND | (memview->physical() ? MF_CHECKED : MF_UNCHECKED));

	CheckMenuItem(menu, ID_REVERSE_VIEW, MF_BYCOMMAND | (memview->reverse() ? MF_CHECKED : MF_UNCHECKED));

	EnableMenuItem(menu, ID_DECREASE_MEM_WIDTH, MF_BYCOMMAND | ((memview->chunks_per_row() > 1) ? MF_ENABLED : MF_GRAYED));
}


bool memorywin_info::handle_command(WPARAM wparam, LPARAM lparam)
{
	auto *const memview = downcast<memoryview_info *>(m_views[0].get());
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
		case ID_1_BYTE_CHUNKS_HEX:
			memview->set_data_format(debug_view_memory::data_format::HEX_8BIT);
			return true;

		case ID_2_BYTE_CHUNKS_HEX:
			memview->set_data_format(debug_view_memory::data_format::HEX_16BIT);
			return true;

		case ID_4_BYTE_CHUNKS_HEX:
			memview->set_data_format(debug_view_memory::data_format::HEX_32BIT);
			return true;

		case ID_8_BYTE_CHUNKS_HEX:
			memview->set_data_format(debug_view_memory::data_format::HEX_64BIT);
			return true;

		case ID_1_BYTE_CHUNKS_OCT:
			memview->set_data_format(debug_view_memory::data_format::OCTAL_8BIT);
			return true;

		case ID_2_BYTE_CHUNKS_OCT:
			memview->set_data_format(debug_view_memory::data_format::OCTAL_16BIT);
			return true;

		case ID_4_BYTE_CHUNKS_OCT:
			memview->set_data_format(debug_view_memory::data_format::OCTAL_32BIT);
			return true;

		case ID_8_BYTE_CHUNKS_OCT:
			memview->set_data_format(debug_view_memory::data_format::OCTAL_64BIT);
			return true;

		case ID_FLOAT_32BIT:
			memview->set_data_format(debug_view_memory::data_format::FLOAT_32BIT);
			return true;

		case ID_FLOAT_64BIT:
			memview->set_data_format(debug_view_memory::data_format::FLOAT_64BIT);
			return true;

		case ID_FLOAT_80BIT:
			memview->set_data_format(debug_view_memory::data_format::FLOAT_80BIT);
			return true;

		case ID_HEX_ADDRESSES:
			memview->set_address_radix(16);
			return true;

		case ID_DEC_ADDRESSES:
			memview->set_address_radix(10);
			return true;

		case ID_OCT_ADDRESSES:
			memview->set_address_radix(8);
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


void memorywin_info::process_string(const std::string &string)
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


void memorywin_info::restore_configuration_from_node(util::xml::data_node const &node)
{
	m_views[0]->set_source_index(node.get_attribute_int(ATTR_WINDOW_MEMORY_REGION, m_views[0]->source_index()));
	int const cursource = m_views[0]->source_index();
	if (0 <= cursource)
		SendMessage(m_combownd, CB_SETCURSEL, cursource, 0);
	update_caption();

	util::xml::data_node const *const expr = node.get_child(NODE_WINDOW_EXPRESSION);
	if (expr && expr->get_value())
	{
		set_editwnd_text(expr->get_value());
		process_string(expr->get_value());
	}

	editwin_info::restore_configuration_from_node(node);

	m_views[0]->restore_configuration_from_node(node);
}


void memorywin_info::save_configuration_to_node(util::xml::data_node &node)
{
	editwin_info::save_configuration_to_node(node);

	node.set_attribute_int(ATTR_WINDOW_TYPE, WINDOW_TYPE_MEMORY_VIEWER);
	node.set_attribute_int(ATTR_WINDOW_MEMORY_REGION, m_views[0]->source_index());
	node.add_child(NODE_WINDOW_EXPRESSION, downcast<memoryview_info *>(m_views[0].get())->expression());
	m_views[0]->save_configuration_to_node(node);
}

} // namespace osd::debugger::win
