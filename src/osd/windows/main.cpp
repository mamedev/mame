// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  main.cpp - Win32 main program
//
//============================================================

// standard windows headers
#ifdef OSD_SDL
#define _WIN32_WINNT 0x0501
#endif
#include <windows.h>
#include <tchar.h>
#include <cstdlib>
#include <vector>
#include <string>

#include "strconv.h"

#if !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

#include "winmain.h"

// The main function is only used to initialize our IFrameworkView class.
[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
	auto app_source = ref new MameViewSource();
	Windows::ApplicationModel::Core::CoreApplication::Run(app_source);
	return 0;
}

#endif
