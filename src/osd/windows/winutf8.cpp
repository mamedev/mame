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
	TCHAR *t_string = tstring_from_utf8(string);
	if (t_string != NULL)
	{
		OutputDebugString(t_string);
		osd_free(t_string);
	}
}



//============================================================
//  win_message_box_utf8
//============================================================

int win_message_box_utf8(HWND window, const char *text, const char *caption, UINT type)
{
	int result = IDNO;
	TCHAR *t_text = NULL;
	TCHAR *t_caption = NULL;

	if (text)
	{
		t_text = tstring_from_utf8(text);
		if (!t_text)
			goto done;
	}

	if (caption)
	{
		t_caption = tstring_from_utf8(caption);
		if (!t_caption)
			goto done;
	}

	result = MessageBox(window, t_text, t_caption, type);

done:
	if (t_text)
		osd_free(t_text);
	if (t_caption)
		osd_free(t_caption);
	return result;
}



//============================================================
//  win_set_window_text_utf8
//============================================================

BOOL win_set_window_text_utf8(HWND window, const char *text)
{
	BOOL result = FALSE;
	TCHAR *t_text = NULL;

	if (text)
	{
		t_text = tstring_from_utf8(text);
		if (!t_text)
			goto done;
	}

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
	result = SetWindowText(window, t_text);
#else
	Windows::UI::ViewManagement::ApplicationView::GetForCurrentView()->Title = ref new Platform::String(t_text);
	result = TRUE;
#endif

done:
	if (t_text)
		osd_free(t_text);
	return result;
}



//============================================================
//  win_get_window_text_utf8
//============================================================

int win_get_window_text_utf8(HWND window, char *buffer, size_t buffer_size)
{
	int result = 0;
	char *utf8_buffer = NULL;
	TCHAR t_buffer[256];

	t_buffer[0] = '\0';

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
	// invoke the core Win32 API
	GetWindowText(window, t_buffer, ARRAY_LENGTH(t_buffer));
#else
	auto title = Windows::UI::ViewManagement::ApplicationView::GetForCurrentView()->Title;
	wcsncpy(t_buffer, title->Data(), ARRAY_LENGTH(t_buffer));
#endif

	utf8_buffer = utf8_from_tstring(t_buffer);
	if (!utf8_buffer)
		goto done;

	result = snprintf(buffer, buffer_size, "%s", utf8_buffer);

done:
	if (utf8_buffer)
		osd_free(utf8_buffer);
	return result;
}



//============================================================
//  win_create_window_ex_utf8
//============================================================

HWND win_create_window_ex_utf8(DWORD exstyle, const char* classname, const char* windowname, DWORD style,
								int x, int y, int width, int height, HWND parent, HMENU menu,
								HINSTANCE instance, void* param)
{
	TCHAR* t_classname;
	TCHAR* t_windowname = NULL;
	HWND result = 0;

	t_classname = tstring_from_utf8(classname);
	if( !t_classname )
		return result;

	if( windowname ) {
		t_windowname = tstring_from_utf8(windowname);
		if( !t_windowname ) {
			osd_free(t_classname);
			return result;
		}
	}

	result = CreateWindowEx(exstyle, t_classname, t_windowname, style, x, y, width, height, parent,
							menu, instance, param);

	if( t_windowname )
		osd_free(t_windowname);
	osd_free(t_classname);

	return result;
}
