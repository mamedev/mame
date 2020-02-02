// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  strconv.cpp - Win32 string conversion
//
//============================================================
#if defined(SDLMAME_WIN32) || defined(OSD_WINDOWS) || defined(OSD_UWP)
#include <windows.h>
#endif
#include <algorithm>
#include <cassert>
// MAMEOS headers
#include "strconv.h"

#if defined(SDLMAME_WIN32) || defined(OSD_WINDOWS) || defined(OSD_UWP)

namespace
{
	// class designed to provide inputs to WideCharToMultiByte() and MultiByteToWideChar()
	template<typename T>
	class string_source
	{
	public:
		string_source(const T *str) : m_str(str), m_char_count(-1)
		{
			assert(str);
		}

		string_source(const std::basic_string<T> &str) : m_str(str.c_str()), m_char_count((int)str.size() + 1)
		{
		}

		const T *string() const { return m_str; };  // returns pointer to actual characters
		int char_count() const { return m_char_count; }     // returns the character count (including NUL terminater), or -1 if NUL terminated

	private:
		const T *m_str;
		int m_char_count;
	};
};

namespace osd {
namespace text {

//============================================================
//  mbstring_from_wstring
//============================================================

static std::string &mbstring_from_wstring(std::string &dst, UINT code_page, const string_source<wchar_t> &src)
{
	// convert UTF-16 to the specified code page
	int dst_char_count = WideCharToMultiByte(code_page, 0, src.string(), src.char_count(), nullptr, 0, nullptr, nullptr);
	dst.resize(dst_char_count - 1);
	WideCharToMultiByte(code_page, 0, src.string(), src.char_count(), &dst[0], dst_char_count, nullptr, nullptr);

	return dst;
}


//============================================================
//  wstring_from_mbstring
//============================================================

static std::wstring &wstring_from_mbstring(std::wstring &dst, const string_source<char> &src, UINT code_page)
{
	// convert multibyte string (in specified code page) to UTF-16
	int dst_char_count = MultiByteToWideChar(code_page, 0, src.string(), src.char_count(), nullptr, 0);
	dst.resize(dst_char_count - 1);
	MultiByteToWideChar(CP_UTF8, 0, src.string(), src.char_count(), &dst[0], dst_char_count - 1);

	return dst;
}


//============================================================
//  to_astring
//============================================================

std::string &to_astring(std::string &dst, const std::string &s)
{
	// convert MAME string (UTF-8) to UTF-16
	std::wstring wstring = to_wstring(s);

	// convert UTF-16 to "ANSI code page" string
	return mbstring_from_wstring(dst, CP_ACP, string_source<wchar_t>(wstring));
}



//============================================================
//  to_astring
//============================================================

std::string &to_astring(std::string &dst, const char *s)
{
	// convert MAME string (UTF-8) to UTF-16
	std::wstring wstring = to_wstring(s);

	// convert UTF-16 to "ANSI code page" string
	return mbstring_from_wstring(dst, CP_ACP, string_source<wchar_t>(wstring));
}


//============================================================
//  to_astring
//============================================================

std::string to_astring(const std::string &s)
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

std::string &from_astring(std::string &dst, const std::string &s)
{
	// convert "ANSI code page" string to UTF-16
	std::wstring wstring;
	wstring_from_mbstring(wstring, string_source<char>(s), CP_ACP);

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
	wstring_from_mbstring(wstring, string_source<char>(s), CP_ACP);

	// convert UTF-16 to MAME string (UTF-8)
	return from_wstring(dst, wstring);
}


//============================================================
//  from_astring
//============================================================

std::string from_astring(const std::string &s)
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

std::wstring &to_wstring(std::wstring &dst, const std::string &s)
{
	// convert MAME string (UTF-8) to UTF-16
	return wstring_from_mbstring(dst, string_source<char>(s), CP_UTF8);
}


//============================================================
//  to_wstring
//============================================================

std::wstring &to_wstring(std::wstring &dst, const char *s)
{
	// convert MAME string (UTF-8) to UTF-16
	return wstring_from_mbstring(dst, string_source<char>(s), CP_UTF8);
}


//============================================================
//  to_wstring
//============================================================

std::wstring to_wstring(const std::string &s)
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

std::string &from_wstring(std::string &dst, const std::wstring &s)
{
	// convert UTF-16 to MAME string (UTF-8)
	return mbstring_from_wstring(dst, CP_UTF8, string_source<wchar_t>(s));
}


//============================================================
//  from_wstring
//============================================================

std::string &from_wstring(std::string &dst, const WCHAR *s)
{
	// convert UTF-16 to MAME string (UTF-8)
	return mbstring_from_wstring(dst, CP_UTF8, string_source<wchar_t>(s));
}


//============================================================
//  from_wstring
//============================================================

std::string from_wstring(const std::wstring &s)
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

}; // namespace text
}; // namespace osd


//============================================================
//  osd_uchar_from_osdchar
//============================================================

int osd_uchar_from_osdchar(char32_t *uchar, const char *osdchar, size_t count)
{
	WCHAR wch;
	CPINFO cp;

	if (!GetCPInfo(CP_ACP, &cp))
		goto error;

	// The multibyte char can't be bigger than the max character size
	count = std::min(count, size_t(cp.MaxCharSize));

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

int osd_uchar_from_osdchar(char32_t *uchar, const char *osdchar, size_t count)
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
