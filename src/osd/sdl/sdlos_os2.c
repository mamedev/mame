// license:???
// copyright-holders:???
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
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/stat.h>

#define INCL_DOS
#include <os2.h>

// MAME headers
#include "osdcore.h"

//============================================================
//  osd_get_clipboard_text
//    - used in MESS
//============================================================

char *osd_get_clipboard_text(void)
{
	char *result = NULL;

	return result;
}
