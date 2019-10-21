// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  logwininfo.h - Win32 debug window handling
//
//============================================================

#ifndef __DEBUG_WIN_LOG_WIN_INFO_H__
#define __DEBUG_WIN_LOG_WIN_INFO_H__

#include "debugwin.h"

#include "debugwininfo.h"


class logwin_info : public debugwin_info
{
public:
	logwin_info(debugger_windows_interface &debugger);
	virtual ~logwin_info();

protected:
	virtual bool handle_command(WPARAM wparam, LPARAM lparam) override;
};

#endif
