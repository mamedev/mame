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
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <vector>
#include <string>

#include "strconv.h"

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

extern int utf8_main(int argc, char *argv[]);
//============================================================
//  main
//============================================================

#ifdef UNICODE
extern "C" int _tmain(int argc, TCHAR **argv)
{
	int i;
	std::vector<std::string> argv_vectors(argc);
	char **utf8_argv = (char **) alloca(argc * sizeof(char *));

	// convert arguments to UTF-8
	for (i = 0; i < argc; i++)
	{
		argv_vectors[i] = utf8_from_tstring(argv[i]);
		utf8_argv[i] = (char *) argv_vectors[i].c_str();
	}

	// run utf8_main
	return utf8_main(argc, utf8_argv);
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
