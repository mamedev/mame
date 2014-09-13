//============================================================
//
//  winos.c - Win32 OS specific low level code
//
//============================================================

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// MAME headers
#include "osdcore.h"

//============================================================
//  osd_num_processors
//============================================================

int osd_get_num_processors(void)
{
	SYSTEM_INFO info;

	// otherwise, fetch the info from the system
	GetSystemInfo(&info);

	// max out at 4 for now since scaling above that seems to do poorly
	return MIN(info.dwNumberOfProcessors, 4);
}

//============================================================
//  osd_getenv
//============================================================

char *osd_getenv(const char *name)
{
	return getenv(name);
}