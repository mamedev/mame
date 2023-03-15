// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  winutf8.cpp - Win32 OSD core utility functions
//
//============================================================

// MAMEOS headers
#include "winutf8.h"
#include "strconv.h"

#include <cstdlib>

// standard windows headers
#include <shellapi.h>


//============================================================
//  win_output_debug_string_utf8
//============================================================

void win_output_debug_string_utf8(const char *string)
{
	auto t_string = osd::text::to_tstring(string);
	OutputDebugString(t_string.c_str());
}


//============================================================
//  win_message_box_utf8
//============================================================

int win_message_box_utf8(HWND window, const char *text, const char *caption, UINT type)
{
	LPCTSTR t_text = nullptr;
	LPCTSTR t_caption = nullptr;
	std::basic_string<TCHAR> ts_text;
	std::basic_string<TCHAR> ts_caption;

	if (text)
	{
		ts_text = osd::text::to_tstring(text);
		t_text = ts_text.c_str();
	}

	if (caption)
	{
		ts_caption = osd::text::to_tstring(caption);
		t_caption = ts_caption.c_str();
	}

	return MessageBox(window, t_text, t_caption, type);
}


//============================================================
//  win_set_window_text_utf8
//============================================================

BOOL win_set_window_text_utf8(HWND window, const char *text)
{
	BOOL result = FALSE;
	LPCTSTR t_text = nullptr;
	std::basic_string<TCHAR> ts_text;

	if (text)
	{
		ts_text = osd::text::to_tstring(text);
		t_text = ts_text.c_str();
	}

	result = SetWindowText(window, t_text);
	return result;
}



//============================================================
//  win_get_window_text_utf8
//============================================================

std::string win_get_window_text_utf8(HWND window)
{
	// invoke the core Win32 API
	int length = GetWindowTextLength(window);
	if (length <= 0)
		return std::string();

	auto *buffer = (TCHAR *) alloca((length + 1) * sizeof(TCHAR));
	GetWindowText(window, buffer, length + 1);
	return osd::text::from_tstring(buffer);
}


//============================================================
//  win_create_window_ex_utf8
//============================================================

HWND win_create_window_ex_utf8(DWORD exstyle, const char* classname, const char* windowname, DWORD style,
								int x, int y, int width, int height, HWND parent, HMENU menu,
								HINSTANCE instance, void* param)
{
	std::basic_string<TCHAR> ts_classname = osd::text::to_tstring(classname);

	LPCTSTR t_windowname = nullptr;
	std::basic_string<TCHAR> ts_windowname;
	if (windowname != nullptr)
	{
		ts_windowname = osd::text::to_tstring(windowname);
		t_windowname = ts_windowname.c_str();
	}

	return CreateWindowEx(exstyle, ts_classname.c_str(), t_windowname, style, x, y, width, height, parent,
		menu, instance, param);
}
