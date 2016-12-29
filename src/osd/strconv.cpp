// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  strconv.cpp - Win32 string conversion
//
//============================================================
#if defined(SDLMAME_WIN32) || defined(OSD_WINDOWS) || defined(OSD_UWP)
#define WIN32_LEAN_AND_MEAN
#define WINVER 0x600
#include <windows.h>
#endif
#undef min
#undef max
#include <algorithm>
#include <assert.h>
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
//  normalize_unicode
//============================================================

std::string osd::text::normalize_unicode(const std::string &s, osd::text::unicode_normalization_form normalization_form)
{
	// convert this enum to its Win32 equivalent
	NORM_FORM win32_normalization_form;
	switch (normalization_form)
	{
	case unicode_normalization_form::C:
		win32_normalization_form = NormalizationC;
		break;
	case unicode_normalization_form::D:
		win32_normalization_form = NormalizationD;
		break;
	case unicode_normalization_form::KC:
		win32_normalization_form = NormalizationKC;
		break;
	case unicode_normalization_form::KD:
		win32_normalization_form = NormalizationKD;
		break;
	default:
		throw false;
	}

	// convert to wide string for use by Win32 APIs
	std::wstring wide_string = to_wstring(s);
	
	// get the length of the buffer required to do the conversion
	int required_length = NormalizeString(win32_normalization_form, wide_string.c_str(), wide_string.length(), nullptr, 0);

	// set up a string
	std::wstring result(required_length, '\0');

	// normalize the string
	int actual_length = NormalizeString(win32_normalization_form, wide_string.c_str(), wide_string.length(), &result[0], required_length);

	// resize the string - actual_length can be less than required_length
	result.resize(actual_length);

	// and return it
	return from_wstring(result);
}


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


//============================================================
//  normalize_unicode
//============================================================

// NOTE - For obvious reasons, not all Win32 platforms use Qt!  This
// needs to be conditionally compiled
#include <QtCore/QString>

namespace osd {
namespace text {
std::string normalize_unicode(const std::string &s, osd::text::unicode_normalization_form normalization_form)
{
	// convert this enum to the Qt equivalent
	QString::NormalizationForm normalizationForm;
	switch (normalization_form)
	{
	case unicode_normalization_form::C:
		normalizationForm = QString::NormalizationForm_C;
		break;
	case unicode_normalization_form::D:
		normalizationForm = QString::NormalizationForm_D;
		break;
	case unicode_normalization_form::KC:
		normalizationForm = QString::NormalizationForm_KC;
		break;
	case unicode_normalization_form::KD:
		normalizationForm = QString::NormalizationForm_KD;
		break;
	default:
		throw false;
	}

	QString qstring = QString::fromUtf8(s.c_str(), s.length());
	QString normalizedQstring = qstring.normalized(normalizationForm);
	QByteArray utf8ByteArray = normalizedQstring.toUtf8();
	return std::string(utf8ByteArray.data(), utf8ByteArray.size());
}
};
};

#endif
