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

#pragma once

#ifndef UNICODE_H
#define UNICODE_H

#include <stdlib.h>
#include "osdcore.h"



/***************************************************************************
    CONSTANTS
***************************************************************************/

// these defines specify the maximum size of different types of Unicode
// character encodings
#define UTF8_CHAR_MAX   6
#define UTF16_CHAR_MAX  2

// these are UTF-8 encoded strings for common characters
#define UTF8_NBSP               "\xc2\xa0"          /* non-breaking space */

#define UTF8_MULTIPLY           "\xc3\x97"          /* multiplication sign */
#define UTF8_DIVIDE             "\xc3\xb7"          /* division sign */
#define UTF8_SQUAREROOT         "\xe2\x88\x9a"      /* square root symbol */
#define UTF8_PLUSMINUS          "\xc2\xb1"          /* plusminus symbol */

#define UTF8_POW_2              "\xc2\xb2"          /* superscript 2 */
#define UTF8_POW_X              "\xcb\xa3"          /* superscript x */
#define UTF8_POW_Y              "\xca\xb8"          /* superscript y */
#define UTF8_PRIME              "\xca\xb9"          /* prime symbol */
#define UTF8_DEGREES            "\xc2\xb0"          /* degrees symbol */

#define UTF8_SMALL_PI           "\xcf\x80"          /* Greek small letter pi */
#define UTF8_CAPITAL_SIGMA      "\xce\xa3"          /* Greek capital letter sigma */
#define UTF8_CAPITAL_DELTA      "\xce\x94"          /* Greek capital letter delta */

#define UTF8_MACRON             "\xc2\xaf"          /* macron symbol */
#define UTF8_NONSPACE_MACRON    "\xcc\x84"          /* nonspace macron, use after another char */

#define a_RING                  "\xc3\xa5"          /* small a with a ring */
#define a_UMLAUT                "\xc3\xa4"          /* small a with an umlaut */
#define o_UMLAUT                "\xc3\xb6"          /* small o with an umlaut */
#define u_UMLAUT                "\xc3\xbc"          /* small u with an umlaut */
#define e_ACUTE                 "\xc3\xa9"          /* small e with an acute */

#define A_RING                  "\xc3\x85"          /* capital A with a ring */
#define A_UMLAUT                "\xc3\x84"          /* capital A with an umlaut */
#define O_UMLAUT                "\xc3\x96"          /* capital O with an umlaut */
#define U_UMLAUT                "\xc3\x9c"          /* capital U with an umlaut */
#define E_ACUTE                 "\xc3\x89"          /* capital E with an acute */

#define UTF8_LEFT               "\xe2\x86\x90"      /* cursor left */
#define UTF8_RIGHT              "\xe2\x86\x92"      /* cursor right */
#define UTF8_UP                 "\xe2\x86\x91"      /* cursor up */
#define UTF8_DOWN               "\xe2\x86\x93"      /* cursor down */



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef UINT16 utf16_char;
typedef UINT32 unicode_char;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

// tests to see if a unicode char is a valid code point
bool uchar_isvalid(unicode_char uchar);

// converting strings to 32-bit Unicode chars
int uchar_from_utf8(unicode_char *uchar, const char *utf8char, size_t count);
int uchar_from_utf16(unicode_char *uchar, const utf16_char *utf16char, size_t count);
int uchar_from_utf16f(unicode_char *uchar, const utf16_char *utf16char, size_t count);

// converting 32-bit Unicode chars to strings
int utf8_from_uchar(char *utf8string, size_t count, unicode_char uchar);
std::string utf8_from_uchar(unicode_char uchar);
int utf16_from_uchar(utf16_char *utf16string, size_t count, unicode_char uchar);
int utf16f_from_uchar(utf16_char *utf16string, size_t count, unicode_char uchar);

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

#endif /* UNICODE_H */
