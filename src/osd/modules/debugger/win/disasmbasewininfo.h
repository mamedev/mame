// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  disasmbasewininfo.h - Win32 debug window handling
//
//============================================================

#ifndef __DEBUG_WIN_DISASM_BASE_WIN_INFO_H__
#define __DEBUG_WIN_DISASM_BASE_WIN_INFO_H__

#include "debugwin.h"

#include "editwininfo.h"


class disasmbasewin_info : public editwin_info
{
public:
	disasmbasewin_info(debugger_windows_interface &debugger, bool is_main_console, LPCSTR title, WNDPROC handler);
	virtual ~disasmbasewin_info();

	virtual bool handle_key(WPARAM wparam, LPARAM lparam) override;

protected:
	virtual void update_menu() override;
	virtual bool handle_command(WPARAM wparam, LPARAM lparam) override;
};

#endif
