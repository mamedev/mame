//============================================================
//
//  strconv.h - SDL string conversion
//
//  Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

#ifndef __SDLSTRCONV__
#define __SDLSTRCONV__

#include "osdcore.h"



//============================================================
//  FUNCTION PROTOTYPES
//============================================================

#ifdef SDLMAME_WIN32

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

#endif //SDLMAME_WIN32

#endif // __SDLSTRCONV__

