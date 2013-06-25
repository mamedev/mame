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

#include "precompiled.h"
#include <Rocket/Core/SystemInterface.h>
#include <Rocket/Core/Log.h>

#ifdef ROCKET_PLATFORM_WIN32
#include <windows.h>
#endif

namespace Rocket {
namespace Core {

SystemInterface::SystemInterface() : ReferenceCountable(0)
{
}

SystemInterface::~SystemInterface()
{
}

bool SystemInterface::LogMessage(Log::Type logtype, const String& message)
{
	// By default we just send a platform message
#ifdef ROCKET_PLATFORM_WIN32
	if (logtype == Log::LT_ASSERT)
	{
		Core::String message(1024, "%s\nWould you like to interrupt execution?", message.CString());	

		// Return TRUE if the user presses NO (continue execution)
		return (IDNO == MessageBoxA(NULL, message.CString(), "Assertion Failure", MB_YESNO | MB_ICONSTOP | MB_DEFBUTTON2 | MB_TASKMODAL));
	}
	else
	{
		OutputDebugStringA(message.CString());
		OutputDebugStringA("\r\n");
	}
#else
	(logtype);
	fprintf(stderr,"%s\n", message.CString());
#endif	
	return true;
}

int SystemInterface::TranslateString(String& translated, const String& input)
{
	translated = input;
	return 0;
}

// Joins the path of an RML or RCSS file with the path of a resource specified within the file.
void SystemInterface::JoinPath(String& translated_path, const String& document_path, const String& path)
{
	// If the path is absolute, strip the leading / and return it.
	if (path.Substring(0, 1) == "/")
	{
		translated_path = path.Substring(1);
		return;
	}

	// If the path is a Windows-style absolute path, return it directly.
	size_t drive_pos = path.Find(":");
	size_t slash_pos = Math::Min(path.Find("/"), path.Find("\\"));
	if (drive_pos != String::npos &&
		drive_pos < slash_pos)
	{
		translated_path = path;
		return;
	}

	// Strip off the referencing document name.
	translated_path = document_path;
	translated_path = translated_path.Replace("\\", "/");
	size_t file_start = translated_path.RFind("/");
	if (file_start != String::npos)
		translated_path.Resize(file_start + 1);
	else
		translated_path.Clear();

	// Append the paths and send through URL to removing any '..'.
	URL url(translated_path.Replace(":", "|") + path.Replace("\\", "/"));
	translated_path = url.GetPathedFileName().Replace("|", ":");
}
	
// Activate keyboard (for touchscreen devices)
void SystemInterface::ActivateKeyboard() 
{
}
	
// Deactivate keyboard (for touchscreen devices)
void SystemInterface::DeactivateKeyboard() 
{
}

// Called when this system interface is released.
void SystemInterface::Release()
{
}

void SystemInterface::OnReferenceDeactivate()
{
	Release();
}

}
}
