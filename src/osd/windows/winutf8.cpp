// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  winutf8.c - Win32 OSD core utility functions
//
//============================================================

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <stdlib.h>

// MAMEOS headers
#include "winutf8.h"
#include "strconv.h"


//============================================================
//  win_output_debug_string_utf8
//============================================================

void win_output_debug_string_utf8(const char *string)
{
	auto t_string = tstring_from_utf8(string);
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
		ts_text = tstring_from_utf8(text);
		t_text = ts_text.c_str();
	}

	if (caption)
	{
		ts_caption = tstring_from_utf8(caption);
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
		ts_text = tstring_from_utf8(text);
		t_text = ts_text.c_str();
	}

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
	result = SetWindowText(window, t_text);
#else
	Windows::UI::ViewManagement::ApplicationView::GetForCurrentView()->Title = ref new Platform::String(t_text);
	result = TRUE;
#endif

	return result;
}



//============================================================
//  win_get_window_text_utf8
//============================================================

int win_get_window_text_utf8(HWND window, char *buffer, size_t buffer_size)
{
	int result = 0;
	TCHAR t_buffer[256];

	t_buffer[0] = '\0';

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
	// invoke the core Win32 API
	GetWindowText(window, t_buffer, ARRAY_LENGTH(t_buffer));
#else
	auto title = Windows::UI::ViewManagement::ApplicationView::GetForCurrentView()->Title;
	wcsncpy(t_buffer, title->Data(), ARRAY_LENGTH(t_buffer));
#endif

	std::string utf8_buffer = utf8_from_tstring(t_buffer);

	result = snprintf(buffer, buffer_size, "%s", utf8_buffer.c_str());

	return result;
}



//============================================================
//  win_create_window_ex_utf8
//============================================================

HWND win_create_window_ex_utf8(DWORD exstyle, const char* classname, const char* windowname, DWORD style,
								int x, int y, int width, int height, HWND parent, HMENU menu,
								HINSTANCE instance, void* param)
{
	std::basic_string<TCHAR> ts_classname = tstring_from_utf8(classname);

	LPCTSTR t_windowname = nullptr;
	std::basic_string<TCHAR> ts_windowname;
	if (windowname != nullptr)
	{
		ts_windowname = tstring_from_utf8(windowname);
		t_windowname = ts_windowname.c_str();
	}

	return CreateWindowEx(exstyle, ts_classname.c_str(), t_windowname, style, x, y, width, height, parent,
		menu, instance, param);
}
