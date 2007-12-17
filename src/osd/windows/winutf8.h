//============================================================
//
//  winutf8.h - Win32 UTF-8 wrappers
//
//  Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

#ifndef __WINUTF8__
#define __WINUTF8__

#include "osdcore.h"

// wrappers for kernel32.dll
void win_output_debug_string_utf8(const char *string);
DWORD win_get_module_file_name_utf8(HMODULE module, char *filename, DWORD size);
BOOL win_set_file_attributes_utf8(const char* filename, DWORD fileattributes);
BOOL win_copy_file_utf8(const char* existingfilename, const char* newfilename, BOOL failifexists);
BOOL win_move_file_utf8(const char* existingfilename, const char* newfilename);

// wrappers for user32.dll
int win_message_box_utf8(HWND window, const char *text, const char *caption, UINT type);
BOOL win_set_window_text_utf8(HWND window, const char *text);
int win_get_window_text_utf8(HWND window, char *buffer, size_t buffer_size);
HWND win_create_window_ex_utf8(DWORD exstyle, const char* classname, const char* windowname, DWORD style,
							   int x, int y, int width, int height, HWND wndparent, HMENU menu,
							   HINSTANCE instance, void* param);

#endif // __WINUTF8__
