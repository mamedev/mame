/*********************************************************************

    unicode.h

    Unicode related functions

    This code is for converting between UTF-8, UTF-16, and 32-bit
    Unicode strings.  These functions roughly parallel C runtime
    library functions like mbtowc() and similar functions, but are
    specific for these Unicode encodings.  Specifically, there are
    functions that convert UTF-8 and UTF-16 char clusters to and from
    singular 32-bit Unicode chars.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#pragma once

#ifndef UNICODE_H
#define UNICODE_H

#include <stdlib.h>
#include "osdcore.h"



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* these defines specify the maximum size of different types of Unicode
 * character encodings */
#define UTF8_CHAR_MAX	6
#define UTF16_CHAR_MAX	2

/* these are UTF-8 encoded strings for common characters */
#define UTF8_NBSP			"\xc2\xa0"			/* non-breaking space */
#define UTF8_MULTIPLY		"\xc3\x97"			/* multiplication symbol */



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef UINT16 utf16_char;
typedef UINT32 unicode_char;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* tests to see if a unicode char is a valid code point */
int uchar_isvalid(unicode_char uchar);

/* converting strings to 32-bit Unicode chars */
int uchar_from_utf8(unicode_char *uchar, const char *utf8char, size_t count);
int uchar_from_utf16(unicode_char *uchar, const utf16_char *utf16char, size_t count);
int uchar_from_utf16f(unicode_char *uchar, const utf16_char *utf16char, size_t count);

/* converting 32-bit Unicode chars to strings */
int utf8_from_uchar(char *utf8string, size_t count, unicode_char uchar);
int utf16_from_uchar(utf16_char *utf16string, size_t count, unicode_char uchar);
int utf16f_from_uchar(utf16_char *utf16string, size_t count, unicode_char uchar);

/* misc UTF-8 helpers */
const char *utf8_previous_char(const char *utf8string);
int utf8_is_valid_string(const char *utf8string);



/***************************************************************************
    MACROS
***************************************************************************/

#ifdef LSB_FIRST
#define uchar_from_utf16be	uchar_from_utf16f
#define uchar_from_utf16le	uchar_from_utf16
#define utf16be_from_uchar	utf16f_from_uchar
#define utf16le_from_uchar	utf16_from_uchar
#else
#define uchar_from_utf16be	uchar_from_utf16
#define uchar_from_utf16le	uchar_from_utf16f
#define utf16be_from_uchar	utf16_from_uchar
#define utf16le_from_uchar	utf16f_from_uchar
#endif

#endif /* UNICODE_H */
