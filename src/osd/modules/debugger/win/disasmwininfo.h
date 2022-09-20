// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  disasmwininfo.h - Win32 debug window handling
//
//============================================================
#ifndef MAME_DEBUGGER_WIN_DISASMWININFO_H
#define MAME_DEBUGGER_WIN_DISASMWININFO_H

#pragma once

#include "debugwin.h"

#include "disasmbasewininfo.h"


namespace osd::debugger::win {

class disasmwin_info : public disasmbasewin_info
{
public:
	disasmwin_info(debugger_windows_interface &debugger);
	virtual ~disasmwin_info();

	virtual void restore_configuration_from_node(util::xml::data_node const &node) override;

protected:
	virtual void recompute_children() override;
	virtual bool handle_command(WPARAM wparam, LPARAM lparam) override;
	virtual void draw_contents(HDC dc) override;
	virtual void save_configuration_to_node(util::xml::data_node &node) override;

private:
	virtual void process_string(const std::string &string) override;

	void update_caption();

	HWND    m_combownd;
};

} // namespace osd::debugger::win

#endif // MAME_DEBUGGER_WIN_DISASMWININFO_H
