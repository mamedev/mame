/*
 * This source file is part of libRocket, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://www.librocket.com
 *
 * Copyright (c) 2008-2010 CodePoint Ltd, Shift Technology Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <Rocket/Controls/Clipboard.h>
#include <Rocket/Core/Types.h>
#include <Rocket/Core/WString.h>
#if defined ROCKET_PLATFORM_WIN32
#include <windows.h>
#endif

namespace Rocket {
namespace Controls {

#if defined ROCKET_PLATFORM_WIN32
static HWND application_hwnd = NULL;

static BOOL CALLBACK FindApplicationWindow(HWND hwnd, LPARAM process_id)
{
	DWORD hwnd_pid;
	GetWindowThreadProcessId(hwnd, &hwnd_pid);
	if (hwnd_pid == (DWORD) process_id)
	{
		application_hwnd = hwnd;
		return FALSE;
	}

	return TRUE;
}

static HWND GetHWND()
{
	if (application_hwnd != NULL)
		return application_hwnd;

	EnumWindows(FindApplicationWindow, GetCurrentProcessId());
	return application_hwnd;
}
#endif

static Core::WString content;

// Get the current contents of the clipboard.
Core::WString Clipboard::Get()
{
	#if defined ROCKET_PLATFORM_WIN32
	if (GetHWND())
	{
		Core::WString clipboard_content;

		if (!OpenClipboard(GetHWND()))
			return clipboard_content;

		HANDLE clipboard_data = GetClipboardData(CF_UNICODETEXT);
		if (clipboard_data == NULL)
			return clipboard_content;

		const Rocket::Core::word* clipboard_text = (const Rocket::Core::word*) GlobalLock(clipboard_data);
		if (clipboard_text)
			clipboard_content.Assign(clipboard_text);
		GlobalUnlock(clipboard_data);

		return clipboard_content;
	}
	else
		return content;
	#else
	return content;
	#endif
}

// Set the contents of the clipboard.
void Clipboard::Set(const Core::WString& _content)
{
	#if defined ROCKET_PLATFORM_WIN32
	if (GetHWND())
	{
		if (!OpenClipboard(GetHWND()))
			return;

		EmptyClipboard();

		Rocket::Core::String win32_content;
		_content.ToUTF8(win32_content);

		HGLOBAL clipboard_data = GlobalAlloc(GMEM_FIXED, win32_content.Length() + 1);
		// Replaced strcpy_s with a simple strcpy, because we know for sure it's big enough.
		strcpy((char*) clipboard_data, win32_content.CString());

		if (SetClipboardData(CF_TEXT, clipboard_data) == NULL)
		{
			CloseClipboard();
			GlobalFree(clipboard_data);
		}
		else
			CloseClipboard();
	}
	else
		content = _content;
	#else
	content = _content;
	#endif
}

#if defined ROCKET_PLATFORM_WIN32
// Set the window handle of the application.
void Clipboard::SetHWND(void* hwnd)
{
	application_hwnd = (HWND) hwnd;
}
#endif

}
}
