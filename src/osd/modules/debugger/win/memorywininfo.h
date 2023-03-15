// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  memorywininfo.h - Win32 debug window handling
//
//============================================================
#ifndef MAME_DEBUGGER_WIN_MEMORYWININFO_H
#define MAME_DEBUGGER_WIN_MEMORYWININFO_H

#pragma once

#include "debugwin.h"

#include "editwininfo.h"


namespace osd::debugger::win {

class memorywin_info : public editwin_info
{
public:
	memorywin_info(debugger_windows_interface &debugger);
	virtual ~memorywin_info();

	virtual bool handle_key(WPARAM wparam, LPARAM lparam) override;
	virtual void restore_configuration_from_node(util::xml::data_node const &node) override;

protected:
	virtual void recompute_children() override;
	virtual void update_menu() override;
	virtual bool handle_command(WPARAM wparam, LPARAM lparam) override;
	virtual void draw_contents(HDC dc) override;
	virtual void save_configuration_to_node(util::xml::data_node &node) override;

private:
	virtual void process_string(const std::string &string) override;

	void update_caption();

	HWND    m_combownd;
};

} // namespace osd::debugger::win

#endif // MAME_DEBUGGER_WIN_MEMORYWININFO_H
