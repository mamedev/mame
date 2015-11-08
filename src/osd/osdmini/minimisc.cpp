// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  minimisc.c - Minimal core miscellaneous functions
//
//============================================================

#include "osdcore.h"
#include <stdlib.h>


//============================================================
//  osd_malloc
//============================================================

void *osd_malloc(size_t size)
{
	return malloc(size);
}


//============================================================
//  osd_malloc_array
//============================================================

void *osd_malloc_array(size_t size)
{
	return malloc(size);
}


//============================================================
//  osd_free
//============================================================

void osd_free(void *ptr)
{
	free(ptr);
}


//============================================================
//  osd_alloc_executable
//============================================================

void *osd_alloc_executable(size_t size)
{
	// to use this version of the code, we have to assume that
	// code injected into a malloc'ed region can be safely executed
	return malloc(size);
}


//============================================================
//  osd_free_executable
//============================================================

void osd_free_executable(void *ptr, size_t size)
{
	free(ptr);
}


//============================================================
//  osd_break_into_debugger
//============================================================

void osd_break_into_debugger(const char *message)
{
	// there is no standard way to do this, so ignore it
}


//============================================================
//  osd_get_clipboard_text
//============================================================

char *osd_get_clipboard_text(void)
{
	// can't support clipboards generically
	return NULL;
}

//============================================================
//  osd_getenv
//============================================================

const char *osd_getenv(const char *name)
{
	return NULL;
}

//============================================================
//  osd_setenv
//============================================================

int osd_setenv(const char *name, const char *value, int overwrite)
{
	return 0;
}
