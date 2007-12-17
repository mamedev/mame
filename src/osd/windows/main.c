//============================================================
//
//  main.c - Win32 main program
//
//  Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

// standard windows headers
#define _WIN32_WINNT 0x0400
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>

// MAMEOS headers
#include "strconv.h"

extern int utf8_main(int argc, char **argv);



//============================================================
//  main
//============================================================

// undo the command-line #define that maps main to utf8_main in all other cases
#ifndef WINUI
#undef main
#endif

#ifdef __GNUC__
int main(int argc, char **a_argv)
#else // !__GNUC__
int _tmain(int argc, TCHAR **argv)
#endif // __GNUC__
{
	int i, rc;
	char **utf8_argv;

#ifdef __GNUC__
	TCHAR **argv;
#ifdef UNICODE
	// MinGW doesn't support wmain() directly, so we have to jump through some hoops
	extern void __wgetmainargs(int *argc, wchar_t ***wargv, wchar_t ***wenviron, int expand_wildcards, int *startupinfo);
	WCHAR **wenviron;
	int startupinfo;
	__wgetmainargs(&argc, &argv, &wenviron, 0, &startupinfo);
#else // !UNICODE
	argv = a_argv;
#endif // UNICODE
#endif // __GNUC__

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
		free(utf8_argv[i]);
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
