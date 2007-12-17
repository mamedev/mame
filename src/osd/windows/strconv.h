//============================================================
//
//  strconv.h - Win32 string conversion
//
//  Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

#ifndef __WIN_STRCONV__
#define __WIN_STRCONV__

#include "mamecore.h"



//============================================================
//  FUNCTION PROTOTYPES
//============================================================

CHAR *astring_from_utf8(const char *s) ATTR_MALLOC;
char *utf8_from_astring(const CHAR *s) ATTR_MALLOC;

WCHAR *wstring_from_utf8(const char *s) ATTR_MALLOC;
char *utf8_from_wstring(const WCHAR *s) ATTR_MALLOC;

#ifdef UNICODE
#define tstring_from_utf8	wstring_from_utf8
#define utf8_from_tstring	utf8_from_wstring
#else // !UNICODE
#define tstring_from_utf8	astring_from_utf8
#define utf8_from_tstring	utf8_from_astring
#endif // UNICODE



#endif // __WIN_STRCONV__
