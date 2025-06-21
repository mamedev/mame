// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  disasmbasewininfo.cpp - Win32 debug window handling
//
//============================================================

#include "emu.h"
#include "disasmbasewininfo.h"

#include "debugviewinfo.h"
#include "disasmviewinfo.h"

#include "debugger.h"
#include "debug/debugcon.h"
#include "debug/debugcpu.h"
#include "debug/points.h"

//#include "winutf8.h"


namespace osd::debugger::win {

disasmbasewin_info::disasmbasewin_info(debugger_windows_interface &debugger, bool is_main_console, LPCSTR title, WNDPROC handler) :
	editwin_info(debugger, is_main_console, VIEW_IDX_DISASM, title, handler)
{
	if (!window())
		return;

	m_views[VIEW_IDX_DISASM].reset(new disasmview_info(debugger, *this, window()));
	if (!m_views[VIEW_IDX_DISASM] || !m_views[VIEW_IDX_DISASM]->is_valid())
	{
		m_views[VIEW_IDX_DISASM].reset();
		return;
	}

	// create the options menu
	m_optionsmenu = CreatePopupMenu();
	AppendMenu(m_optionsmenu, MF_ENABLED, ID_TOGGLE_BREAKPOINT, TEXT("Toggle breakpoint at cursor\tF9"));
	AppendMenu(m_optionsmenu, MF_ENABLED, ID_DISABLE_BREAKPOINT, TEXT("Disable breakpoint at cursor\tShift+F9"));
	AppendMenu(m_optionsmenu, MF_ENABLED, ID_RUN_TO_CURSOR, TEXT("Run to cursor\tF4"));
	AppendMenu(m_optionsmenu, MF_DISABLED | MF_SEPARATOR, 0, TEXT(""));
	AppendMenu(m_optionsmenu, MF_ENABLED, ID_SHOW_RAW, TEXT("Raw opcodes\tCtrl+R"));
	AppendMenu(m_optionsmenu, MF_ENABLED, ID_SHOW_ENCRYPTED, TEXT("Encrypted opcodes\tCtrl+E"));
	AppendMenu(m_optionsmenu, MF_ENABLED, ID_SHOW_COMMENTS, TEXT("Comments\tCtrl+N"));
	AppendMenu(GetMenu(window()), MF_ENABLED | MF_POPUP, (UINT_PTR)m_optionsmenu, TEXT("Options"));

	// set up the view to track the initial expression
	downcast<disasmview_info *>(m_views[VIEW_IDX_DISASM].get())->set_expression("curpc");
	m_views[VIEW_IDX_DISASM]->set_source_for_visible_cpu();
}


disasmbasewin_info::~disasmbasewin_info()
{
}


bool disasmbasewin_info::handle_key(WPARAM wparam, LPARAM lparam)
{
	if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
	{
		switch (wparam)
		{
		case 'R':
			SendMessage(window(), WM_COMMAND, ID_SHOW_RAW, 0);
			return true;

		case 'E':
			SendMessage(window(), WM_COMMAND, ID_SHOW_ENCRYPTED, 0);
			return true;

		case 'N':
			SendMessage(window(), WM_COMMAND, ID_SHOW_COMMENTS, 0);
			return true;
		}
	}

	switch (wparam)
	{
	// ajg - steals the F4 from the global key handler - but ALT+F4 didn't work anyways ;)
	case VK_F4:
		SendMessage(window(), WM_COMMAND, ID_RUN_TO_CURSOR, 0);
		return true;

	case VK_F9:
		if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
			SendMessage(window(), WM_COMMAND, ID_DISABLE_BREAKPOINT, 0);
		else
			SendMessage(window(), WM_COMMAND, ID_TOGGLE_BREAKPOINT, 0);
		return true;

	case VK_RETURN:
		if (m_views[VIEW_IDX_DISASM]->cursor_visible() && m_views[VIEW_IDX_DISASM]->source_is_visible_cpu())
		{
			SendMessage(window(), WM_COMMAND, ID_STEP, 0);
			return true;
		}
		break;
	}

	return editwin_info::handle_key(wparam, lparam);
}


void disasmbasewin_info::update_menu()
{
	editwin_info::update_menu();

	disasmview_info const * visible_view = get_visible_view_info();
	HMENU const menu = GetMenu(window());

	bool const cursor_visible = visible_view->cursor_visible();
	if (cursor_visible)
	{
		std::optional<offs_t> const addropt = visible_view->selected_address();
		const debug_breakpoint *bp;
		if (addropt.has_value())
		{
			offs_t address = addropt.value();
			device_debug *const debug = visible_view->source_device()->debug();

			// first find an existing breakpoint at this address
			bp = debug->breakpoint_find(address);
		}
		else
		{
			bp = nullptr;
		}

		if (!bp)
		{
			ModifyMenu(menu, ID_TOGGLE_BREAKPOINT, MF_BYCOMMAND, ID_TOGGLE_BREAKPOINT, TEXT("Set breakpoint at cursor\tF9"));
			ModifyMenu(menu, ID_DISABLE_BREAKPOINT, MF_BYCOMMAND, ID_DISABLE_BREAKPOINT, TEXT("Disable breakpoint at cursor\tShift+F9"));
		}
		else
		{
			ModifyMenu(menu, ID_TOGGLE_BREAKPOINT, MF_BYCOMMAND, ID_TOGGLE_BREAKPOINT, TEXT("Clear breakpoint at cursor\tF9"));
			if (bp->enabled())
				ModifyMenu(menu, ID_DISABLE_BREAKPOINT, MF_BYCOMMAND, ID_DISABLE_BREAKPOINT, TEXT("Disable breakpoint at cursor\tShift+F9"));
			else
				ModifyMenu(menu, ID_DISABLE_BREAKPOINT, MF_BYCOMMAND, ID_DISABLE_BREAKPOINT, TEXT("Enable breakpoint at cursor\tShift+F9"));
		}
		bool const available = bp && (!is_main_console() || visible_view->source_is_visible_cpu());
		EnableMenuItem(menu, ID_DISABLE_BREAKPOINT, MF_BYCOMMAND | (available ? MF_ENABLED : MF_GRAYED));
	}
	else
	{
		ModifyMenu(menu, ID_TOGGLE_BREAKPOINT, MF_BYCOMMAND, ID_TOGGLE_BREAKPOINT, TEXT("Toggle breakpoint at cursor\tF9"));
		ModifyMenu(menu, ID_DISABLE_BREAKPOINT, MF_BYCOMMAND, ID_DISABLE_BREAKPOINT, TEXT("Disable breakpoint at cursor\tShift+F9"));
		EnableMenuItem(menu, ID_DISABLE_BREAKPOINT, MF_BYCOMMAND | MF_GRAYED);
	}
	EnableMenuItem(menu, ID_TOGGLE_BREAKPOINT, MF_BYCOMMAND | (cursor_visible ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(menu, ID_RUN_TO_CURSOR, MF_BYCOMMAND | (cursor_visible ? MF_ENABLED : MF_GRAYED));

	disasm_right_column const rightcol = visible_view->right_column();
	CheckMenuItem(menu, ID_SHOW_RAW, MF_BYCOMMAND | (rightcol == DASM_RIGHTCOL_RAW ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(menu, ID_SHOW_ENCRYPTED, MF_BYCOMMAND | (rightcol == DASM_RIGHTCOL_ENCRYPTED ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(menu, ID_SHOW_COMMENTS, MF_BYCOMMAND | (rightcol == DASM_RIGHTCOL_COMMENTS ? MF_CHECKED : MF_UNCHECKED));
}


bool disasmbasewin_info::handle_command(WPARAM wparam, LPARAM lparam)
{
	if (m_views[VIEW_IDX_DISASM].get() != nullptr)
	{
		if (m_views[VIEW_IDX_DISASM]->is_visible() &&
			handle_disasm_command(wparam, lparam))
		{
			return true;
		}

		if (handle_common_command(wparam, lparam))
		{
			return true;
		}
	}

	return editwin_info::handle_command(wparam, lparam);
}


// Helper called by handle_command to handle commands specific to the
// disassembly window, and not to other child classes, like the source
// debugging window
bool disasmbasewin_info::handle_disasm_command(WPARAM wparam, LPARAM lparam)
{
	auto *const dasmview = downcast<disasmview_info *>(m_views[VIEW_IDX_DISASM].get());
	switch (HIWORD(wparam))
	{
	// menu selections
	case 0:
		switch (LOWORD(wparam))
		{
		case ID_SHOW_RAW:
			dasmview->set_right_column(DASM_RIGHTCOL_RAW);
			recompute_children();
			return true;

		case ID_SHOW_ENCRYPTED:
			dasmview->set_right_column(DASM_RIGHTCOL_ENCRYPTED);
			recompute_children();
			return true;

		case ID_SHOW_COMMENTS:
			dasmview->set_right_column(DASM_RIGHTCOL_COMMENTS);
			recompute_children();
			return true;
		}
		break;
	}
	return false;
}

// Return pointer to either the disasmview_info or its subclass
// sourceview_info, depending on which view is currently visible
// in this window
disasmview_info const * disasmbasewin_info::get_visible_view_info()
{
	disasmview_info const *  ret = downcast<disasmview_info *>(m_views[VIEW_IDX_DISASM].get());
	if (is_main_console() && !m_views[VIEW_IDX_DISASM]->is_visible())
	{
		ret = downcast<disasmview_info *>(m_views[VIEW_IDX_SOURCE].get());
	}

	return ret;
}


// Helper called by handle_command to handle commands that could apply
// to any subclasses (i.e., both disasembly and source debugging windows).
bool disasmbasewin_info::handle_common_command(WPARAM wparam, LPARAM lparam)
{
	disasmview_info const * visible_view = get_visible_view_info();

	switch (HIWORD(wparam))
	{
	// menu selections
	case 0:
		switch (LOWORD(wparam))
		{
		case ID_TOGGLE_BREAKPOINT:
			if (visible_view->cursor_visible())
			{
				std::optional<offs_t> const addropt = visible_view->selected_address();
				if (!addropt.has_value())
				{
					return true;
				}
				offs_t address = addropt.value();
				device_debug *const debug = visible_view->source_device()->debug();

				// first find an existing breakpoint at this address
				const debug_breakpoint *bp = debug->breakpoint_find(address);

				// if it doesn't exist, add a new one
				if (!is_main_console())
				{
					if (bp == nullptr)
					{
						int32_t bpindex = debug->breakpoint_set(address);
						machine().debugger().console().printf("Breakpoint %X set\n", bpindex);
					}
					else
					{
						int32_t bpindex = bp->index();
						debug->breakpoint_clear(bpindex);
						machine().debugger().console().printf("Breakpoint %X cleared\n", bpindex);
					}
					machine().debug_view().update_all();
					machine().debugger().refresh_display();
				}
				else if (visible_view->source_is_visible_cpu())
				{
					std::string command;
					if (bp == nullptr)
						command = string_format("bpset 0x%X", address);
					else
						command = string_format("bpclear 0x%X", bp->index());
					machine().debugger().console().execute_command(command, true);
				}
			}
			return true;

		case ID_DISABLE_BREAKPOINT:
			if (visible_view->cursor_visible())
			{
				std::optional<offs_t> const addropt = visible_view->selected_address();
				if (!addropt.has_value())
				{
					return true;
				}
				offs_t address = addropt.value();
				device_debug *const debug = visible_view->source_device()->debug();

				// first find an existing breakpoint at this address
				const debug_breakpoint *bp = debug->breakpoint_find(address);

				// if it doesn't exist, add a new one
				if (bp != nullptr)
				{
					if (!is_main_console())
					{
						debug->breakpoint_enable(bp->index(), !bp->enabled());
						machine().debugger().console().printf("Breakpoint %X %s\n", (uint32_t)bp->index(), bp->enabled() ? "enabled" : "disabled");
						machine().debug_view().update_all();
						machine().debugger().refresh_display();
					}
					else if (visible_view->source_is_visible_cpu())
					{
						std::string command;
						command = string_format(bp->enabled() ? "bpdisable 0x%X" : "bpenable 0x%X", (uint32_t)bp->index());
						machine().debugger().console().execute_command(command, true);
					}
				}
			}
			return true;

		case ID_RUN_TO_CURSOR:
			if (visible_view->cursor_visible())
			{
				std::optional<offs_t> const addropt = visible_view->selected_address();
				if (!addropt.has_value())
				{
					return true;
				}
				offs_t address = addropt.value();
				if (visible_view->source_is_visible_cpu())
				{
					std::string command;
					command = string_format("go 0x%X", address);
					machine().debugger().console().execute_command(command, true);
				}
				else
				{
					visible_view->source_device()->debug()->go(address);
				}
			}
			return true;
		}
	}
	return false;
}


void disasmbasewin_info::restore_configuration_from_node(util::xml::data_node const &node)
{
	editwin_info::restore_configuration_from_node(node);
	m_views[VIEW_IDX_DISASM]->restore_configuration_from_node(node);
}


void disasmbasewin_info::save_configuration_to_node(util::xml::data_node &node)
{
	editwin_info::save_configuration_to_node(node);
	m_views[VIEW_IDX_DISASM]->save_configuration_to_node(node);
}

} // namespace osd::debugger::win
