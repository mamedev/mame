// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  memorywininfo.h - Win32 debug window handling
//
//============================================================

#ifndef __DEBUG_WIN_MEMORY_WIN_INFO_H__
#define __DEBUG_WIN_MEMORY_WIN_INFO_H__

#include "debugwin.h"

#include "editwininfo.h"


class memorywin_info : public editwin_info
{
public:
	memorywin_info(debugger_windows_interface &debugger);
	virtual ~memorywin_info();

	virtual bool handle_key(WPARAM wparam, LPARAM lparam);

protected:
	virtual void recompute_children();
	virtual void update_menu();
	virtual bool handle_command(WPARAM wparam, LPARAM lparam);
	virtual void draw_contents(HDC dc);

private:
	virtual void process_string(char const *string);

	void update_caption();

	HWND    m_combownd;
};

#endif
