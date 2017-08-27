// license:BSD-3-Clause
// copyright-holders:Samuele Zannoli
//============================================================
//
//  logviewinfo.c - Win32 debug log window handling
//
//============================================================

#include "emu.h"
#include "logviewinfo.h"

#include "debug/dvtext.h"


logview_info::logview_info(debugger_windows_interface &debugger, debugwin_info &owner, HWND parent) :
	debugview_info(debugger, owner, parent, DVT_LOG)
{
}


logview_info::~logview_info()
{
}


void logview_info::clear()
{
	view<debug_view_log>()->clear();
}
