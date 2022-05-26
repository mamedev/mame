// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  winutil.h - Win32 OSD core utility functions
//
//============================================================

#ifndef MAME_OSD_WINDOWS_WINUTIL_H
#define MAME_OSD_WINDOWS_WINUTIL_H

#pragma once

#include "osdfile.h"

#include <algorithm>
#include <chrono>

#include <windows.h>


// Shared code
osd::directory::entry::entry_type win_attributes_to_entry_type(DWORD attributes);
std::chrono::system_clock::time_point win_time_point_from_filetime(LPFILETIME file_time);
BOOL win_is_gui_application();
HMODULE WINAPI GetModuleHandleUni();

#endif // MAME_OSD_WINDOWS_WINUTIL_H
