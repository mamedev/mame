//============================================================
//
//  winutf8.h - Win32 UTF-8 wrappers
//
//============================================================
//
//  Copyright Aaron Giles
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or
//  without modification, are permitted provided that the
//  following conditions are met:
//
//    * Redistributions of source code must retain the above
//      copyright notice, this list of conditions and the
//      following disclaimer.
//    * Redistributions in binary form must reproduce the
//      above copyright notice, this list of conditions and
//      the following disclaimer in the documentation and/or
//      other materials provided with the distribution.
//    * Neither the name 'MAME' nor the names of its
//      contributors may be used to endorse or promote
//      products derived from this software without specific
//      prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND
//  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
//  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
//  EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
//  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//  DAMAGE (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
//  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
//  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
