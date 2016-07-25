// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  strconv.cpp - Win32 string conversion
//
//============================================================

#if defined(SDLMAME_WIN32) || defined(OSD_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

// MAMEOS headers
#include "strconv.h"

#if defined(SDLMAME_WIN32) || defined(OSD_WINDOWS)
//============================================================
//  astring_from_utf8
//============================================================

std::string &astring_from_utf8(std::string &dst, const char *s)
{
	// convert MAME string (UTF-8) to UTF-16
	std::basic_string<WCHAR> wstring = wstring_from_utf8(s);

	// convert UTF-16 to "ANSI code page" string
	int char_count = WideCharToMultiByte(CP_ACP, 0, wstring.c_str(), wstring.size(), nullptr, 0, nullptr, nullptr);
	dst.resize(char_count);
	WideCharToMultiByte(CP_ACP, 0, wstring.c_str(), wstring.size(), &dst[0], char_count, nullptr, nullptr);

	return dst;
}


//============================================================
//  astring_from_utf8
//============================================================

std::string astring_from_utf8(const char *s)
{
	std::string result;
	astring_from_utf8(result, s);
	return result;
}


//============================================================
//  utf8_from_astring
//============================================================

std::string &utf8_from_astring(std::string &dst, const CHAR *s)
{
	// convert "ANSI code page" string to UTF-16
	int char_count = MultiByteToWideChar(CP_ACP, 0, s, -1, nullptr, 0);
	std::wstring wstring(char_count - 1, 0);
	MultiByteToWideChar(CP_ACP, 0, s, -1, &wstring[0], char_count - 1);

	// convert UTF-16 to MAME string (UTF-8)
	return utf8_from_wstring(dst, wstring.c_str());
}


//============================================================
//  utf8_from_astring
//============================================================

std::string utf8_from_astring(const CHAR *s)
{
	std::string result;
	utf8_from_astring(result, s);
	return result;
}


//============================================================
//  wstring_from_utf8
//============================================================

std::wstring &wstring_from_utf8(std::wstring &dst, const char *s)
{
	// convert MAME string (UTF-8) to UTF-16
	int char_count = MultiByteToWideChar(CP_UTF8, 0, s, -1, nullptr, 0);
	dst.resize(char_count - 1);
	MultiByteToWideChar(CP_UTF8, 0, s, -1, &dst[0], char_count - 1);

	return dst;
}


//============================================================
//  wstring_from_utf8
//============================================================

std::wstring wstring_from_utf8(const char *s)
{
	std::wstring result;
	wstring_from_utf8(result, s);
	return result;
}


//============================================================
//  utf8_from_wstring
//============================================================

std::string &utf8_from_wstring(std::string &dst, const WCHAR *s)
{
	// convert UTF-16 to MAME string (UTF-8)
	int char_count = WideCharToMultiByte(CP_UTF8, 0, s, -1, nullptr, 0, nullptr, nullptr);
	dst.resize(char_count - 1);
	WideCharToMultiByte(CP_UTF8, 0, s, -1, &dst[0], char_count - 1, nullptr, nullptr);

	return dst;
}


//============================================================
//  utf8_from_wstring
//============================================================

std::string utf8_from_wstring(const WCHAR *s)
{
	std::string result;
	utf8_from_wstring(result, s);
	return result;
}


//============================================================
//  osd_uchar_from_osdchar
//============================================================

int osd_uchar_from_osdchar(UINT32 *uchar, const char *osdchar, size_t count)
{
	WCHAR wch;
	CPINFO cp;

	if (!GetCPInfo(CP_ACP, &cp))
		goto error;

	// The multibyte char can't be bigger than the max character size
	count = MIN(count, cp.MaxCharSize);

	if (MultiByteToWideChar(CP_ACP, 0, osdchar, static_cast<DWORD>(count), &wch, 1) == 0)
		goto error;

	*uchar = wch;
	return static_cast<int>(count);

error:
	*uchar = 0;
	return static_cast<int>(count);
}

#else
#include "unicode.h"
//============================================================
//  osd_uchar_from_osdchar
//============================================================

int osd_uchar_from_osdchar(unicode_char *uchar, const char *osdchar, size_t count)
{
	wchar_t wch;

	count = mbstowcs(&wch, (char *)osdchar, 1);
	if (count != -1)
		*uchar = wch;
	else
		*uchar = 0;

	return count;
}

#endif
