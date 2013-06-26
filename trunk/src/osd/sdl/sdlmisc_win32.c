//============================================================
//
//  sdlos_*.c - OS specific low level code
//
//  Copyright (c) 1996-2010, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

// standard sdl header
#include "sdlinc.h"


#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// MAME headers
#include "osdcore.h"

//============================================================
//  osd_alloc_executable
//
//  allocates "size" bytes of executable memory.  this must take
//  things like NX support into account.
//============================================================

void *osd_alloc_executable(size_t size)
{
	return VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
}

//============================================================
//  osd_free_executable
//
//  frees memory allocated with osd_alloc_executable
//============================================================

void osd_free_executable(void *ptr, size_t size)
{
	VirtualFree(ptr, 0, MEM_RELEASE);
}

//============================================================
//  osd_break_into_debugger
//============================================================

void osd_break_into_debugger(const char *message)
{
	if (IsDebuggerPresent())
	{
		OutputDebugStringA(message);
		DebugBreak();
	}
}
