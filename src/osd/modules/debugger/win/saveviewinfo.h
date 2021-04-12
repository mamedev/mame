// license:BSD-3-Clause
// copyright-holders:Samuele Zannoli
//============================================================
//
//  saveviewinfo.h - Win32 debug save window handling
//
//============================================================
#ifndef MAME_DEBUGGER_WIN_SAVEVIEWINFO_H
#define MAME_DEBUGGER_WIN_SAVEVIEWINFO_H

#pragma once

#include "debugwin.h"

#include "debugviewinfo.h"


class saveview_info : public debugview_info
{
public:
	saveview_info(debugger_windows_interface &debugger, debugwin_info &owner, HWND parent);
	virtual ~saveview_info();

	void clear();
};

#endif // MAME_DEBUGGER_WIN_SAVEVIEWINFO_H
