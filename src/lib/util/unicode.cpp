// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*********************************************************************

    unicode.cpp

    Unicode related functions

***************************************************************************/

#include "unicode.h"

#include "osdcomm.h"

#ifdef _WIN32
#include "strconv.h"
#define UTF8PROC_DLLEXPORT
#endif

#include <utf8proc.h>

#include <codecvt>
#include <locale>


namespace {

//-------------------------------------------------
//  internal_normalize_unicode - uses utf8proc to
//  normalize unicode
//-------------------------------------------------

std::string internal_normalize_unicode(
		char const *s,
		size_t length,
		unicode_normalization_form normalization_form,
		bool fold_case,
		bool null_terminated)
{
	// convert the normalization form
	int options;
	switch (normalization_form)
	{
	case unicode_normalization_form::C:
		options = UTF8PROC_STABLE | UTF8PROC_COMPOSE;
		break;
	case unicode_normalization_form::D:
		options = UTF8PROC_STABLE | UTF8PROC_DECOMPOSE;
		break;
	case unicode_normalization_form::KC:
		options = UTF8PROC_STABLE | UTF8PROC_COMPOSE | UTF8PROC_COMPAT;
		break;
	case unicode_normalization_form::KD:
		options = UTF8PROC_STABLE | UTF8PROC_DECOMPOSE | UTF8PROC_COMPAT;
		break;
	default:
		throw false;
	}

	// perform case folding?
	if (fold_case)
		options |= UTF8PROC_CASEFOLD;

	// use NUL terminator to determine length?
	if (null_terminated)
		options |= UTF8PROC_NULLTERM;

	// invoke utf8proc
	utf8proc_uint8_t *utf8proc_result(nullptr);
	utf8proc_ssize_t const utf8proc_result_length(utf8proc_map(reinterpret_cast<utf8proc_uint8_t const *>(s), length, &utf8proc_result, utf8proc_option_t(options)));

	// conver the result
	std::string result;
	if (utf8proc_result)
	{
		if (utf8proc_result_length > 0)
			result.assign(reinterpret_cast<char const *>(utf8proc_result), utf8proc_result_length);
		free(utf8proc_result);
	}

	return result;
}

} // anonymous namespace


//-------------------------------------------------
//  uchar_isvalid - return true if a given
//  character is a legitimate unicode character
//-------------------------------------------------

bool uchar_isvalid(char32_t uchar)
{
	return (uchar < 0x110000) && !((uchar >= 0xd800) && (uchar <= 0xdfff));
}


//-------------------------------------------------
//  uchar_is_printable - tests to see if a unicode
//  char is printable
//-------------------------------------------------

bool uchar_is_printable(char32_t uchar)
{
	return
		!(0x0001f >= uchar) &&                            // C0 control
		!((0x0007f <= uchar) && (0x0009f >= uchar)) &&    // DEL and C1 control
		!((0x0fdd0 <= uchar) && (0x0fddf >= uchar)) &&    // noncharacters
		!(0x0fffe == (uchar & 0x0ffff)) &&                // byte-order detection noncharacter
		!(0x0ffff == (uchar & 0x0ffff));                  // the other noncharacter
}


//-------------------------------------------------
//  uchar_is_digit - tests to see if a unicode
//  char is a digit
//-------------------------------------------------

bool uchar_is_digit(char32_t uchar)
{
	return uchar >= '0' && uchar <= '9';
}


//-------------------------------------------------
//  uchar_from_utf8 - convert a UTF-8 sequence
//  into a unicode character
//-----------------------------------------------

int uchar_from_utf8(char32_t *uchar, std::string_view utf8str)
{
	return uchar_from_utf8(uchar, utf8str.data(), utf8str.length());
}


//-------------------------------------------------
//  uchar_from_utf8 - convert a UTF-8 sequence
//  into a unicode character
//-----------------------------------------------

int uchar_from_utf8(char32_t *uchar, const char *utf8char, size_t count)
{
	// validate parameters
	if (!utf8char || !count)
		return 0;

	// start with the first byte
	char32_t c = (unsigned char)*utf8char;
	count--;
	utf8char++;

	// based on that, determine how many additional bytes we need
	char32_t minchar;
	int auxlen;
	if ((c & 0x80) == 0x00)
	{
		// unicode char 0x00000000 - 0x0000007F
		auxlen = 0;
		minchar = 0x00000000;
	}
	else if ((c & 0xe0) == 0xc0)
	{
		// unicode char 0x00000080 - 0x000007FF
		c &= 0x1f;
		auxlen = 1;
		minchar = 0x00000080;
	}
	else if ((c & 0xf0) == 0xe0)
	{
		// unicode char 0x00000800 - 0x0000FFFF
		c &= 0x0f;
		auxlen = 2;
		minchar = 0x00000800;
	}
	else if ((c & 0xf8) == 0xf0)
	{
		// unicode char 0x00010000 - 0x001FFFFF
		c &= 0x07;
		auxlen = 3;
		minchar = 0x00010000;
	}
	else if ((c & 0xfc) == 0xf8)
	{
		// unicode char 0x00200000 - 0x03FFFFFF
		c &= 0x03;
		auxlen = 4;
		minchar = 0x00200000;
	}
	else if ((c & 0xfe) == 0xfc)
	{
		// unicode char 0x04000000 - 0x7FFFFFFF
		c &= 0x01;
		auxlen = 5;
		minchar = 0x04000000;
	}
	else
	{
		// invalid
		return -1;
	}

	// exceeds the count?
	if (auxlen > count)
		return -1;

	// we now know how long the char is, now compute it
	for (int i = 0; i < auxlen; i++)
	{
		char32_t const auxchar = (unsigned char)utf8char[i];

		// all auxiliary chars must be between 0x80-0xbf
		if ((auxchar & 0xc0) != 0x80)
			return -1;

		c = c << 6;
		c |= auxchar & 0x3f;
	}

	// make sure that this char is above the minimum
	if (c < minchar)
		return -1;

	*uchar = c;
	return auxlen + 1;
}


//-------------------------------------------------
//  uchar_from_utf16 - convert a UTF-16 sequence
//  into a unicode character
//-------------------------------------------------

int uchar_from_utf16(char32_t *uchar, const char16_t *utf16char, size_t count)
{
	int rc = -1;

	// validate parameters
	if (utf16char == nullptr || count == 0)
	{
		rc = 0;
	}
	if (utf16char[0] >= 0xd800 && utf16char[0] <= 0xdbff)
	{
		// handle the two-byte case
		if (count > 1 && utf16char[1] >= 0xdc00 && utf16char[1] <= 0xdfff)
		{
			*uchar = 0x10000 + ((utf16char[0] & 0x3ff) * 0x400) + (utf16char[1] & 0x3ff);
			rc = 2;
		}
	}
	else if (utf16char[0] < 0xdc00 || utf16char[0] > 0xdfff)
	{
		// handle the one-byte case
		*uchar = utf16char[0];
		rc = 1;
	}

	return rc;
}


//-------------------------------------------------
//  uchar_from_utf16f - convert a UTF-16 sequence
//  into a unicode character from a flipped
//  byte order
//-------------------------------------------------

int uchar_from_utf16f(char32_t *uchar, const char16_t *utf16char, size_t count)
{
	char16_t buf[2] = {0};
	if (count > 0)
		buf[0] = swapendian_int16(utf16char[0]);
	if (count > 1)
		buf[1] = swapendian_int16(utf16char[1]);
	return uchar_from_utf16(uchar, buf, count);
}


//-------------------------------------------------
//  ustr_from_utf8 - convert a UTF-8 sequence into
//  into a Unicode string
//-------------------------------------------------

std::u32string ustr_from_utf8(const std::string &utf8str)
{
	std::u32string result;
	char const *utf8char(utf8str.c_str());
	size_t remaining(utf8str.length());
	while (remaining)
	{
		char32_t ch;
		int const consumed(uchar_from_utf8(&ch, utf8char, remaining));
		result.append(1, (consumed > 0) ? ch : char32_t(0x00fffdU));
		utf8char += (consumed > 0) ? consumed : 1;
		remaining -= (consumed > 0) ? consumed : 1;
	}
	return result;
}


//-------------------------------------------------
//  utf8_from_uchar - convert a unicode character
//  into a UTF-8 sequence
//-------------------------------------------------

int utf8_from_uchar(char *utf8string, size_t count, char32_t uchar)
{
	int rc = 0;

	// error on invalid characters
	if (!uchar_isvalid(uchar))
		return -1;

	// based on the value, output the appropriate number of bytes
	if (uchar < 0x80)
	{
		// unicode char 0x00000000 - 0x0000007F
		if (count < 1)
			return -1;
		utf8string[rc++] = (char) uchar;
	}
	else if (uchar < 0x800)
	{
		// unicode char 0x00000080 - 0x000007FF
		if (count < 2)
			return -1;
		utf8string[rc++] = ((char) (uchar >> 6)) | 0xC0;
		utf8string[rc++] = ((char) ((uchar >> 0) & 0x3F)) | 0x80;
	}
	else if (uchar < 0x10000)
	{
		// unicode char 0x00000800 - 0x0000FFFF
		if (count < 3)
			return -1;
		utf8string[rc++] = ((char) (uchar >> 12)) | 0xE0;
		utf8string[rc++] = ((char) ((uchar >> 6) & 0x3F)) | 0x80;
		utf8string[rc++] = ((char) ((uchar >> 0) & 0x3F)) | 0x80;
	}
	else if (uchar < 0x00200000)
	{
		// unicode char 0x00010000 - 0x001FFFFF
		if (count < 4)
			return -1;
		utf8string[rc++] = ((char) (uchar >> 18)) | 0xF0;
		utf8string[rc++] = ((char) ((uchar >> 12) & 0x3F)) | 0x80;
		utf8string[rc++] = ((char) ((uchar >> 6) & 0x3F)) | 0x80;
		utf8string[rc++] = ((char) ((uchar >> 0) & 0x3F)) | 0x80;
	}
	else if (uchar < 0x04000000)
	{
		// unicode char 0x00200000 - 0x03FFFFFF
		if (count < 5)
			return -1;
		utf8string[rc++] = ((char) (uchar >> 24)) | 0xF8;
		utf8string[rc++] = ((char) ((uchar >> 18) & 0x3F)) | 0x80;
		utf8string[rc++] = ((char) ((uchar >> 12) & 0x3F)) | 0x80;
		utf8string[rc++] = ((char) ((uchar >> 6) & 0x3F)) | 0x80;
		utf8string[rc++] = ((char) ((uchar >> 0) & 0x3F)) | 0x80;
	}
	else if (uchar < 0x80000000)
	{
		// unicode char 0x04000000 - 0x7FFFFFFF
		if (count < 6)
			return -1;
		utf8string[rc++] = ((char) (uchar >> 30)) | 0xFC;
		utf8string[rc++] = ((char) ((uchar >> 24) & 0x3F)) | 0x80;
		utf8string[rc++] = ((char) ((uchar >> 18) & 0x3F)) | 0x80;
		utf8string[rc++] = ((char) ((uchar >> 12) & 0x3F)) | 0x80;
		utf8string[rc++] = ((char) ((uchar >> 6) & 0x3F)) | 0x80;
		utf8string[rc++] = ((char) ((uchar >> 0) & 0x3F)) | 0x80;
	}
	else
		rc = -1;

	return rc;
}


//-------------------------------------------------
//  utf8_from_uchar - convert a unicode character
//  into a UTF-8 sequence
//-------------------------------------------------

std::string utf8_from_uchar(char32_t uchar)
{
	char buffer[UTF8_CHAR_MAX];
	auto len = utf8_from_uchar(buffer, ARRAY_LENGTH(buffer), uchar);
	return std::string(buffer, len);
}


//-------------------------------------------------
//  utf16_from_uchar - convert a unicode character
//  into a UTF-16 sequence
//-------------------------------------------------

int utf16_from_uchar(char16_t *utf16string, size_t count, char32_t uchar)
{
	int rc;

	// error on invalid characters
	if (!uchar_isvalid(uchar))
		return -1;

	if (uchar < 0x10000)
	{
		// single word case
		if (count < 1)
			return -1;
		utf16string[0] = (char16_t) uchar;
		rc = 1;
	}
	else if (uchar < 0x100000)
	{
		// double word case
		if (count < 2)
			return -1;
		uchar -= 0x10000;
		utf16string[0] = ((uchar >> 10) & 0x03ff) | 0xd800;
		utf16string[1] = ((uchar >>  0) & 0x03ff) | 0xdc00;
		rc = 2;
	}
	else
	{
		return -1;
	}
	return rc;
}


//-------------------------------------------------
//  utf16_from_uchar - convert a unicode character
//  into a UTF-16 sequence with flipped endianness
//-------------------------------------------------

int utf16f_from_uchar(char16_t *utf16string, size_t count, char32_t uchar)
{
	int rc;
	char16_t buf[2] = { 0, 0 };

	rc = utf16_from_uchar(buf, count, uchar);

	if (rc >= 1)
		utf16string[0] = swapendian_int16(buf[0]);
	if (rc >= 2)
		utf16string[1] = swapendian_int16(buf[1]);
	return rc;
}


//-------------------------------------------------
// wstring_from_utf8
//-------------------------------------------------

std::wstring wstring_from_utf8(const std::string &utf8string)
{
#ifdef WIN32
	// for some reason, using codecvt yields bad results on MinGW (but not MSVC)
	return osd::text::to_wstring(utf8string);
#else
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.from_bytes(utf8string);
#endif
}


//-------------------------------------------------
// utf8_from_wstring
//-------------------------------------------------

std::string utf8_from_wstring(const std::wstring &string)
{
#ifdef WIN32
	// for some reason, using codecvt yields bad results on MinGW (but not MSVC)
	return osd::text::from_wstring(string);
#else
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.to_bytes(string);
#endif
}


//-------------------------------------------------
//  normalize_unicode - uses utf8proc to normalize
//  unicode
//-------------------------------------------------

std::string normalize_unicode(const std::string &s, unicode_normalization_form normalization_form, bool fold_case)
{
	return internal_normalize_unicode(s.c_str(), s.length(), normalization_form, fold_case, false);
}


//-------------------------------------------------
//  normalize_unicode - uses utf8proc to normalize
//  unicode
//-------------------------------------------------

std::string normalize_unicode(const char *s, unicode_normalization_form normalization_form, bool fold_case)
{
	return internal_normalize_unicode(s, 0, normalization_form, fold_case, true);
}


//-------------------------------------------------
//  normalize_unicode - uses utf8proc to normalize
//  unicode
//-------------------------------------------------

std::string normalize_unicode(std::string_view s, unicode_normalization_form normalization_form, bool fold_case)
{
	return internal_normalize_unicode(s.data(), s.length(), normalization_form, fold_case, false);
}


//-------------------------------------------------
//  uchar_toupper - uses utf8proc to convert to
//  upper case
//-------------------------------------------------

char32_t uchar_toupper(char32_t ch)
{
	return utf8proc_toupper(ch);
}


//-------------------------------------------------
//  uchar_tolower - uses utf8proc to convert to
//  lower case
//-------------------------------------------------

char32_t uchar_tolower(char32_t ch)
{
	return utf8proc_tolower(ch);
}


//-------------------------------------------------
//  utf8_previous_char - return a pointer to the
//  previous character in a string
//-------------------------------------------------

/**
 * @fn  const char *utf8_previous_char(const char *utf8string)
 *
 * @brief   UTF 8 previous character.
 *
 * @param   utf8string  The UTF 8string.
 *
 * @return  null if it fails, else a char*.
 */

const char *utf8_previous_char(const char *utf8string)
{
	while ((*--utf8string & 0xc0) == 0x80)
		;
	return utf8string;
}


//-------------------------------------------------
//  utf8_is_valid_string - return true if the
//  given string is a properly formed sequence of
//    UTF-8 characters
//-------------------------------------------------

/**
 * @fn  int utf8_is_valid_string(const char *utf8string)
 *
 * @brief   UTF 8 is valid string.
 *
 * @param   utf8string  The UTF 8string.
 *
 * @return  An int.
 */

bool utf8_is_valid_string(const char *utf8string)
{
	int remaining_length = strlen(utf8string);

	while (*utf8string != 0)
	{
		char32_t uchar = 0;
		int charlen;

		// extract the current character and verify it
		charlen = uchar_from_utf8(&uchar, utf8string, remaining_length);
		if (charlen <= 0 || uchar == 0 || !uchar_isvalid(uchar))
			return false;

		// advance
		utf8string += charlen;
		remaining_length -= charlen;
	}

	return true;
}
