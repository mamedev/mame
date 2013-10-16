// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  strconv.h - Win32 string conversion
//
//============================================================

#ifndef __WIN_STRCONV__
#define __WIN_STRCONV__

#include "osdcore.h"



//============================================================
//  FUNCTION PROTOTYPES
//============================================================

// the result of these functions has to be released with osd_free()

CHAR *astring_from_utf8(const char *s);
char *utf8_from_astring(const CHAR *s);

WCHAR *wstring_from_utf8(const char *s);
char *utf8_from_wstring(const WCHAR *s);

#ifdef UNICODE
#define tstring_from_utf8   wstring_from_utf8
#define utf8_from_tstring   utf8_from_wstring
#else // !UNICODE
#define tstring_from_utf8   astring_from_utf8
#define utf8_from_tstring   utf8_from_astring
#endif // UNICODE



#endif // __WIN_STRCONV__
