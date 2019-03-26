// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  winutf8.h - Win32 UTF-8 wrappers
//
//============================================================

#ifndef MAME_OSD_WINDOWS_WINUTF8_H
#define MAME_OSD_WINDOWS_WINUTF8_H

#pragma once

#include "osdcore.h"

#include <windows.h>

// wrappers for kernel32.dll
void win_output_debug_string_utf8(const char *string);

// wrappers for user32.dll
int win_message_box_utf8(HWND window, const char *text, const char *caption, UINT type);
BOOL win_set_window_text_utf8(HWND window, const char *text);
std::string win_get_window_text_utf8(HWND window);
HWND win_create_window_ex_utf8(
		DWORD exstyle,
		const char *classname,
		const char *windowname,
		DWORD style,
		int x, int y, int width, int height,
		HWND wndparent,
		HMENU menu,
		HINSTANCE instance,
		void *param);

#endif // MAME_OSD_WINDOWS_WINUTF8_H
