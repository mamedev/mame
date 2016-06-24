// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  winutil.h - Win32 OSD core utility functions
//
//============================================================

#ifndef __WINUTIL__
#define __WINUTIL__

#include "osdcore.h"
#include <string>
#include <vector>
#include <chrono>

// Shared code
osd::directory::entry::entry_type win_attributes_to_entry_type(DWORD attributes);
std::chrono::system_clock::time_point win_time_point_from_filetime(LPFILETIME file_time);
BOOL win_is_gui_application(void);
HMODULE WINAPI GetModuleHandleUni();

#endif // __WINUTIL__
