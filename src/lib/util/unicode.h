// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*********************************************************************

    unicode.h

    Unicode related functions

    This code is for converting between UTF-8, UTF-16, and 32-bit
    Unicode strings.  These functions roughly parallel C runtime
    library functions like mbtowc() and similar functions, but are
    specific for these Unicode encodings.  Specifically, there are
    functions that convert UTF-8 and UTF-16 char clusters to and from
    singular 32-bit Unicode chars.

***************************************************************************/
#ifndef MAME_LIB_UTIL_UNICODE_H
#define MAME_LIB_UTIL_UNICODE_H

#pragma once

#include "osdcore.h"

#include <string>
#include <string_view>

#include <cstdlib>



/***************************************************************************
    CONSTANTS
***************************************************************************/

// these defines specify the maximum size of different types of Unicode
// character encodings
#define UTF8_CHAR_MAX   6
#define UTF16_CHAR_MAX  2

enum class unicode_normalization_form { C, D, KC, KD };



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

// tests to see if a unicode char is a valid code point
bool uchar_isvalid(char32_t uchar);

// tests to see if a unicode char is printable
bool uchar_is_printable(char32_t uchar);

// tests to see if a unicode char is a digit
bool uchar_is_digit(char32_t uchar);

// converting strings to 32-bit Unicode chars
int uchar_from_utf8(char32_t *uchar, const char *utf8char, size_t count);
int uchar_from_utf8(char32_t *uchar, std::string_view utf8str);
int uchar_from_utf16(char32_t *uchar, const char16_t *utf16char, size_t count);
int uchar_from_utf16f(char32_t *uchar, const char16_t *utf16char, size_t count);
std::u32string ustr_from_utf8(const std::string &utf8str);

// converting 32-bit Unicode chars to strings
int utf8_from_uchar(char *utf8string, size_t count, char32_t uchar);
std::string utf8_from_uchar(char32_t uchar);
int utf16_from_uchar(char16_t *utf16string, size_t count, char32_t uchar);
int utf16f_from_uchar(char16_t *utf16string, size_t count, char32_t uchar);

// converting UTF-8 strings to/from "wide" strings
std::wstring wstring_from_utf8(const std::string &utf8string);
std::string utf8_from_wstring(const std::wstring &string);

// unicode normalization
std::string normalize_unicode(const std::string &s, unicode_normalization_form normalization_form, bool fold_case = false);
std::string normalize_unicode(const char *s, unicode_normalization_form normalization_form, bool fold_case = false);
std::string normalize_unicode(std::string_view s, unicode_normalization_form normalization_form, bool fold_case = false);

// upper and lower case
char32_t uchar_toupper(char32_t ch);
char32_t uchar_tolower(char32_t ch);

// misc UTF-8 helpers
const char *utf8_previous_char(const char *utf8string);
bool utf8_is_valid_string(const char *utf8string);



/***************************************************************************
    MACROS
***************************************************************************/

#ifdef LSB_FIRST
#define uchar_from_utf16be  uchar_from_utf16f
#define uchar_from_utf16le  uchar_from_utf16
#define utf16be_from_uchar  utf16f_from_uchar
#define utf16le_from_uchar  utf16_from_uchar
#else
#define uchar_from_utf16be  uchar_from_utf16
#define uchar_from_utf16le  uchar_from_utf16f
#define utf16be_from_uchar  utf16_from_uchar
#define utf16le_from_uchar  utf16f_from_uchar
#endif

#endif // MAME_LIB_UTIL_UNICODE_H
