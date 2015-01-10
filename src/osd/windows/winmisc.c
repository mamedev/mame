// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  winmisc.c - Win32 OSD core miscellaneous functions
//
//============================================================

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>

// MAME headers
#include "osdcore.h"

// MAMEOS headers
#include "winutf8.h"
#include "strconv.h"
#include "strconv.h"


//============================================================
//  GLOBAL VARIABLES
//============================================================

void (*s_debugger_stack_crawler)() = NULL;


//============================================================
//  osd_alloc_executable
//============================================================

void *osd_alloc_executable(size_t size)
{
	return VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
}


//============================================================
//  osd_free_executable
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
		win_output_debug_string_utf8(message);
		DebugBreak();
	}
	else if (s_debugger_stack_crawler != NULL)
		(*s_debugger_stack_crawler)();
}
