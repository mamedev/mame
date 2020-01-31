// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  pointswininfo.h - Win32 debug window handling
//
//============================================================
#ifndef MAME_DEBUGGER_WIN_POINTSWININFO_H
#define MAME_DEBUGGER_WIN_POINTSWININFO_H

#pragma once

#include "debugwin.h"

#include "debugwininfo.h"


class pointswin_info : public debugwin_info
{
public:
	pointswin_info(debugger_windows_interface &debugger);
	virtual ~pointswin_info();

	virtual bool handle_key(WPARAM wparam, LPARAM lparam) override;

protected:
	virtual void update_menu() override;
	virtual bool handle_command(WPARAM wparam, LPARAM lparam) override;
};

#endif
