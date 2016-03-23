// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  winutf8.h - Win32 UTF-8 wrappers
//
//============================================================

#ifndef __WINUTF8__
#define __WINUTF8__

#include "osdcore.h"

// wrappers for kernel32.dll
void win_output_debug_string_utf8(const char *string);

// wrappers for user32.dll
int win_message_box_utf8(HWND window, const char *text, const char *caption, UINT type);
BOOL win_set_window_text_utf8(HWND window, const char *text);
int win_get_window_text_utf8(HWND window, char *buffer, size_t buffer_size);
HWND win_create_window_ex_utf8(DWORD exstyle, const char* classname, const char* windowname, DWORD style,
								int x, int y, int width, int height, HWND wndparent, HMENU menu,
								HINSTANCE instance, void* param);

#endif // __WINUTF8__
