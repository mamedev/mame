// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  strconv.cpp - Win32 string conversion
//
//============================================================

#include "strconv.h"

#include "unicode.h"

#include <algorithm>
#include <cassert>


#if defined(_WIN32)

#include <windows.h>


namespace osd::text {

//============================================================
//  mbstring_from_wstring
//============================================================

inline std::string &mbstring_from_wstring(std::string &dst, UINT code_page, const std::wstring_view &src)
{
	// convert UTF-16 to the specified code page
	const int dst_char_count = WideCharToMultiByte(code_page, 0, src.data(), src.length(), nullptr, 0, nullptr, nullptr);
	dst.resize(dst_char_count);
	WideCharToMultiByte(code_page, 0, src.data(), src.length(), dst.data(), dst_char_count, nullptr, nullptr);

	return dst;
}


//============================================================
//  wstring_from_mbstring
//============================================================

inline std::wstring &wstring_from_mbstring(std::wstring &dst, const std::string_view &src, UINT code_page)
{
	// convert multibyte string (in specified code page) to UTF-16
	const int dst_char_count = MultiByteToWideChar(code_page, 0, src.data(), src.length(), nullptr, 0);
	dst.resize(dst_char_count);
	MultiByteToWideChar(code_page, 0, src.data(), src.length(), dst.data(), dst_char_count);

	return dst;
}


//============================================================
//  to_astring
//============================================================

std::string &to_astring(std::string &dst, std::string_view s)
{
	// convert MAME string (UTF-8) to UTF-16
	std::wstring wstring = to_wstring(s);

	// convert UTF-16 to "ANSI code page" string
	return mbstring_from_wstring(dst, CP_ACP, wstring);
}



//============================================================
//  to_astring
//============================================================

std::string &to_astring(std::string &dst, const char *s)
{
	// convert MAME string (UTF-8) to UTF-16
	std::wstring wstring = to_wstring(s);

	// convert UTF-16 to "ANSI code page" string
	return mbstring_from_wstring(dst, CP_ACP, wstring);
}


//============================================================
//  to_astring
//============================================================

std::string to_astring(std::string_view s)
{
	std::string result;
	to_astring(result, s);
	return result;
}


//============================================================
//  to_astring
//============================================================

std::string to_astring(const char *s)
{
	std::string result;
	to_astring(result, s);
	return result;
}


//============================================================
//  from_astring
//============================================================

std::string &from_astring(std::string &dst, std::string_view s)
{
	// convert "ANSI code page" string to UTF-16
	std::wstring wstring;
	wstring_from_mbstring(wstring, s, CP_ACP);

	// convert UTF-16 to MAME string (UTF-8)
	return from_wstring(dst, wstring);
}


//============================================================
//  from_astring
//============================================================

std::string &from_astring(std::string &dst, const CHAR *s)
{
	// convert "ANSI code page" string to UTF-16
	std::wstring wstring;
	wstring_from_mbstring(wstring, s, CP_ACP);

	// convert UTF-16 to MAME string (UTF-8)
	return from_wstring(dst, wstring);
}


//============================================================
//  from_astring
//============================================================

std::string from_astring(std::string_view s)
{
	std::string result;
	from_astring(result, s);
	return result;
}


//============================================================
//  from_astring
//============================================================

std::string from_astring(const CHAR *s)
{
	std::string result;
	from_astring(result, s);
	return result;
}


//============================================================
//  to_wstring
//============================================================

std::wstring &to_wstring(std::wstring &dst, std::string_view s)
{
	// convert MAME string (UTF-8) to UTF-16
	return wstring_from_mbstring(dst, s, CP_UTF8);
}


//============================================================
//  to_wstring
//============================================================

std::wstring &to_wstring(std::wstring &dst, const char *s)
{
	// convert MAME string (UTF-8) to UTF-16
	return wstring_from_mbstring(dst, s, CP_UTF8);
}


//============================================================
//  to_wstring
//============================================================

std::wstring to_wstring(std::string_view s)
{
	std::wstring result;
	to_wstring(result, s);
	return result;
}


//============================================================
//  to_wstring
//============================================================

std::wstring to_wstring(const char *s)
{
	std::wstring result;
	to_wstring(result, s);
	return result;
}


//============================================================
//  from_wstring
//============================================================

std::string &from_wstring(std::string &dst, std::wstring_view s)
{
	// convert UTF-16 to MAME string (UTF-8)
	return mbstring_from_wstring(dst, CP_UTF8, s);
}


//============================================================
//  from_wstring
//============================================================

std::string &from_wstring(std::string &dst, const WCHAR *s)
{
	// convert UTF-16 to MAME string (UTF-8)
	return mbstring_from_wstring(dst, CP_UTF8, s);
}


//============================================================
//  from_wstring
//============================================================

std::string from_wstring(std::wstring_view s)
{
	std::string result;
	from_wstring(result, s);
	return result;
}


//============================================================
//  from_wstring
//============================================================

std::string from_wstring(const WCHAR *s)
{
	std::string result;
	from_wstring(result, s);
	return result;
}

} // namespace osd::text


//============================================================
//  osd_uchar_from_osdchar
//============================================================

int osd_uchar_from_osdchar(char32_t *uchar, const char *osdchar, size_t count)
{
	// handle UTF-8 internally
	if (GetACP() == CP_UTF8)
		return uchar_from_utf8(uchar, osdchar, count);

	// FIXME: This makes multiple bad assumptions:
	// * Assumes any character can be converted to a single wchar_t.
	// * Assumes characters are either one byte or maximum number of bytes.
	// * Doesn't handle nominal single-byte encodings with combining characters.
	CPINFO cp;
	if (!GetCPInfo(CP_ACP, &cp))
		goto error;

	// The multibyte char can't be bigger than the max character size
	count = std::min(count, size_t(IsDBCSLeadByte(*osdchar) ? cp.MaxCharSize : 1));

	WCHAR wch;
	if (MultiByteToWideChar(CP_ACP, 0, osdchar, static_cast<DWORD>(count), &wch, 1) == 0)
		goto error;

	*uchar = wch;
	return static_cast<int>(count);

error:
	*uchar = 0;
	return static_cast<int>(count);
}


#else // _WIN32

//============================================================
//  osd_uchar_from_osdchar
//============================================================

int osd_uchar_from_osdchar(char32_t *uchar, const char *osdchar, size_t count)
{
	// FIXME: mbstowcs depends on global state
	wchar_t wch;
	count = mbstowcs(&wch, (char *)osdchar, 1);
	if (count != size_t(-1))
	{
		*uchar = wch;
		return int(count);
	}
	else
	{
		*uchar = 0;
		return -1;
	}
}

#endif // _WIN32
