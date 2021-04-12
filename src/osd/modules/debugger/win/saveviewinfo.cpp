// license:BSD-3-Clause
// copyright-holders:Samuele Zannoli
//============================================================
//
//  saveviewinfo.c - Win32 debug save window handling
//
//============================================================

#include "emu.h"
#include "saveviewinfo.h"

#include "debug/dvsave.h"


saveview_info::saveview_info(debugger_windows_interface &debugger, debugwin_info &owner, HWND parent) :
	debugview_info(debugger, owner, parent, DVT_SAVE)
{
}


saveview_info::~saveview_info()
{
}


void saveview_info::clear()
{
//	view<debug_view_save>()->clear();
}
