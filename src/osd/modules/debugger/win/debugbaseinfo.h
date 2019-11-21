// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  debugbaseinfo.h - Win32 debug window handling
//
//============================================================
#ifndef MAME_DEBUGGER_WIN_DEBUGBASEINFO_H
#define MAME_DEBUGGER_WIN_DEBUGBASEINFO_H

#pragma once

#include "debugwin.h"


class debugbase_info
{
protected:
	debugbase_info(debugger_windows_interface &debugger);

	debugger_windows_interface &debugger() const { return m_debugger; }
	running_machine &machine() const { return m_machine; }
	ui_metrics const &metrics() const { return m_metrics; }

	bool waiting_for_debugger() const { return m_waiting_for_debugger; }
	bool seq_pressed() const { return m_debugger.seq_pressed(); }

	static void smart_set_window_bounds(HWND wnd, HWND parent, RECT const &bounds);
	static void smart_show_window(HWND wnd, bool show);

private:
	debugger_windows_interface  &m_debugger;
	running_machine             &m_machine;
	ui_metrics const            &m_metrics;
	bool const                  &m_waiting_for_debugger;
};

#endif
