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

#if defined(_WIN32)

#include <string_view>

#include <windows.h>

namespace osd::text {

std::string to_astring(std::string_view s);
std::string to_astring(const char *s);
std::string &to_astring(std::string &dst, std::string_view s);
std::string &to_astring(std::string &dst, const char *s);
std::string from_astring(const std::string_view s);
std::string from_astring(const CHAR *s);
std::string &from_astring(std::string &dst, std::string_view s);
std::string &from_astring(std::string &dst, const CHAR *s);

std::wstring to_wstring(std::string_view s);
std::wstring to_wstring(const char *s);
std::wstring &to_wstring(std::wstring &dst, std::string_view s);
std::wstring &to_wstring(std::wstring &dst, const char *s);
std::string from_wstring(const std::wstring_view s);
std::string from_wstring(const WCHAR *s);
std::string &from_wstring(std::string &dst, std::wstring_view s);
std::string &from_wstring(std::string &dst, const WCHAR *s);

#ifdef UNICODE
typedef std::wstring tstring;
#define to_tstring   to_wstring
#define from_tstring   from_wstring
#else // !UNICODE
typedef std::string tstring;
#define to_tstring   to_astring
#define from_tstring   from_astring
#endif // UNICODE

} // namespace osd::text

#endif // defined(_WIN32)


#endif // MAME_OSD_STRCONV_H
