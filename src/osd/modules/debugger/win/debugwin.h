// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  debugwin.h - Win32 debug window handling
//
//============================================================

#ifndef __DEBUG_WIN_DEBUG_WIN_H__
#define __DEBUG_WIN_DEBUG_WIN_H__

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <commdlg.h>
#ifdef _MSC_VER
#include <zmouse.h>
#endif
#undef min
#undef max

#include "emu.h"


class debugview_info;
class debugwin_info;
class ui_metrics;


class debugger_windows_interface
{
public:
	virtual ~debugger_windows_interface() { }

	virtual running_machine &machine() const = 0;

	virtual ui_metrics &metrics() const = 0;

	virtual bool const &waiting_for_debugger() const = 0;
	virtual bool seq_pressed() const = 0;

	virtual void create_memory_window() = 0;
	virtual void create_disasm_window() = 0;
	virtual void create_log_window() = 0;
	virtual void create_points_window() = 0;
	virtual void remove_window(debugwin_info &info) = 0;

	virtual void show_all() = 0;
	virtual void hide_all() = 0;
};

#endif
