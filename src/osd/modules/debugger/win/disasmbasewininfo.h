// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  disasmbasewininfo.h - Win32 debug window handling
//
//============================================================
#ifndef MAME_DEBUGGER_WIN_DISASMBASEWININFO_H
#define MAME_DEBUGGER_WIN_DISASMBASEWININFO_H

#pragma once

#include "debugwin.h"

#include "editwininfo.h"


namespace osd::debugger::win {

class disasmview_info;

class disasmbasewin_info : public editwin_info
{
public:
	disasmbasewin_info(debugger_windows_interface &debugger, bool is_main_console, LPCSTR title, WNDPROC handler);
	virtual ~disasmbasewin_info();

	virtual bool handle_key(WPARAM wparam, LPARAM lparam) override;
	virtual void restore_configuration_from_node(util::xml::data_node const &node) override;

protected:
	virtual void update_menu() override;
	virtual bool handle_command(WPARAM wparam, LPARAM lparam) override;
	virtual void save_configuration_to_node(util::xml::data_node &node) override;

private:
	disasmview_info const * get_visible_view_info();
	bool handle_common_command(WPARAM wparam, LPARAM lparam);
	bool handle_disasm_command(WPARAM wparam, LPARAM lparam);

protected:
	HMENU m_optionsmenu;
};

} // namespace osd::debugger::win

#endif // MAME_DEBUGGER_WIN_DISASMBASEWININFO_H
