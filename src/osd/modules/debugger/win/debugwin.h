// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  debugwin.h - Win32 debug window handling
//
//============================================================
#ifndef MAME_DEBUGGER_WIN_DEBUGWIN_H
#define MAME_DEBUGGER_WIN_DEBUGWIN_H

#pragma once

#include "../xmlconfig.h"

// standard windows headers
#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <commdlg.h>
#ifdef _MSC_VER
#include <zmouse.h>
#endif


namespace osd::debugger::win {

class debugview_info;
class debugwin_info;
class ui_metrics;


class debugger_windows_interface
{
public:
	virtual ~debugger_windows_interface() { }

	virtual running_machine &machine() const = 0;

	virtual ui_metrics &metrics() const = 0;
	virtual void set_color_theme(int index) = 0;
	virtual bool get_save_window_arrangement() const = 0;
	virtual void set_save_window_arrangement(bool save) = 0;

	virtual bool const &waiting_for_debugger() const = 0;
	virtual bool seq_pressed() const = 0;

	virtual void create_memory_window() = 0;
	virtual void create_disasm_window() = 0;
	virtual void create_log_window() = 0;
	virtual void create_points_window() = 0;
	virtual void remove_window(debugwin_info &info) = 0;

	virtual void show_all() = 0;
	virtual void hide_all() = 0;

	virtual void stagger_window(HWND window, int width, int height) = 0;
};

} // namespace osd::debugger::win

#endif // MAME_DEBUGGER_WIN_DEBUGWIN_H
