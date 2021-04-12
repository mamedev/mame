// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  savewininfo.h - Win32 debug window handling
//
//============================================================
#ifndef MAME_DEBUGGER_WIN_SAVEWININFO_H
#define MAME_DEBUGGER_WIN_SAVEWININFO_H

#pragma once

#include "debugwin.h"

#include "debugwininfo.h"


class savewin_info : public debugwin_info
{
public:
	savewin_info(debugger_windows_interface &debugger);
	virtual ~savewin_info();

protected:
	virtual bool handle_command(WPARAM wparam, LPARAM lparam) override;
};

#endif
