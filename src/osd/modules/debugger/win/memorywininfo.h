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

	virtual bool handle_key(WPARAM wparam, LPARAM lparam) override;

protected:
	virtual void recompute_children() override;
	virtual void update_menu() override;
	virtual bool handle_command(WPARAM wparam, LPARAM lparam) override;
	virtual void draw_contents(HDC dc) override;

private:
	virtual void process_string(char const *string) override;

	void update_caption();

	HWND    m_combownd;
};

#endif
