// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  main.c - Win32 main program
//
//============================================================

// standard windows headers
#ifdef OSD_SDL
#define _WIN32_WINNT 0x0501
#endif
#include <windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <vector>
#include <string>

#include "strconv.h"

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

extern int utf8_main(std::vector<std::string> &args);
//============================================================
//  main
//============================================================

#ifdef UNICODE
extern "C" int _tmain(int argc, TCHAR **argv)
{
	int i;
	std::vector<std::string> argv_vectors(argc);

	// convert arguments to UTF-8
	for (i = 0; i < argc; i++)
		argv_vectors[i] = osd::text::from_tstring(argv[i]);

	// run utf8_main
	return utf8_main(argv_vectors);
}
#endif

#else

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
