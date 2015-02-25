// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  disasmwininfo.h - Win32 debug window handling
//
//============================================================

#ifndef __DEBUG_WIN_DISASM_WIN_INFO_H__
#define __DEBUG_WIN_DISASM_WIN_INFO_H__

#include "debugwin.h"

#include "disasmbasewininfo.h"


class disasmwin_info : public disasmbasewin_info
{
public:
	disasmwin_info(debugger_windows_interface &debugger);
	virtual ~disasmwin_info();

protected:
	virtual void recompute_children();
	virtual bool handle_command(WPARAM wparam, LPARAM lparam);
	virtual void draw_contents(HDC dc);

private:
	virtual void process_string(char const *string);

	void update_caption();

	HWND    m_combownd;
};

#endif
