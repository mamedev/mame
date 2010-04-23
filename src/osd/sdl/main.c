//============================================================
//
//  main.c - Win32 main program
//
//  Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

// standard windows headers
#define _WIN32_WINNT 0x0400
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#include <stdlib.h>

// MAMEOS headers
#include "strconv.h"

extern int utf8_main(int argc, char *argv[]);



//============================================================
//  main
//============================================================

// undo the command-line #define that maps main to utf8_main in all other cases
#ifndef WINUI
#undef main
#undef wmain
#endif

extern "C" int _tmain(int argc, TCHAR **argv)
{
	int i, rc;
	char **utf8_argv;

#ifdef MALLOC_DEBUG
{
	extern int winalloc_in_main_code;
	winalloc_in_main_code = TRUE;
#endif

	/* convert arguments to UTF-8 */
	utf8_argv = (char **) malloc(argc * sizeof(*argv));
	if (utf8_argv == NULL)
		return 999;
	for (i = 0; i < argc; i++)
	{
		utf8_argv[i] = utf8_from_tstring(argv[i]);
		if (utf8_argv[i] == NULL)
			return 999;
	}

	/* run utf8_main */
	rc = utf8_main(argc, utf8_argv);

	/* free arguments */
	for (i = 0; i < argc; i++)
		osd_free(utf8_argv[i]);
	free(utf8_argv);

#ifdef MALLOC_DEBUG
	{
		void check_unfreed_mem(void);
		check_unfreed_mem();
	}
	winalloc_in_main_code = FALSE;
}
#endif

	return rc;
}
