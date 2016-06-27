// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  strconv.h - String conversion
//
//============================================================

#ifndef MAME_OSD_STRCONV_H
#define MAME_OSD_STRCONV_H

#include "osdcore.h"



//============================================================
//  FUNCTION PROTOTYPES
//============================================================

#if defined(WIN32)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

// the result of these functions has to be released with osd_free()

CHAR *astring_from_utf8(const char *s);
char *utf8_from_astring(const CHAR *s);

WCHAR *wstring_from_utf8(const char *s);
char *utf8_from_wstring(const WCHAR *s);

struct osd_wstr_deleter
{
	void operator () (wchar_t* wstr) const
	{
		if (wstr != nullptr)
			osd_free(wstr);
	}
};

struct osd_str_deleter
{
	void operator () (char* str) const
	{
		if (str != nullptr)
			osd_free(str);
	}
};

typedef std::unique_ptr<wchar_t, osd_wstr_deleter> osd_unique_wstr;
typedef std::unique_ptr<char, osd_str_deleter> osd_unique_str;

#ifdef UNICODE
#define tstring_from_utf8   wstring_from_utf8
#define utf8_from_tstring   utf8_from_wstring
#else // !UNICODE
#define tstring_from_utf8   astring_from_utf8
#define utf8_from_tstring   utf8_from_astring
#endif // UNICODE

#endif // defined(WIN32)


#endif // MAME_OSD_STRCONV_H
