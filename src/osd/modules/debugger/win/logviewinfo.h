// license:BSD-3-Clause
// copyright-holders:Samuele Zannoli
//============================================================
//
//  logviewinfo.h - Win32 debug log window handling
//
//============================================================

#ifndef __DEBUG_WIN_LOG_VIEW_INFO_H__
#define __DEBUG_WIN_LOG_VIEW_INFO_H__

#include "debugwin.h"

#include "debugviewinfo.h"


class logview_info : public debugview_info
{
public:
	logview_info(debugger_windows_interface &debugger, debugwin_info &owner, HWND parent);
	virtual ~logview_info();

	void clear();
};

#endif
