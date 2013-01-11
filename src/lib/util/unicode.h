/*********************************************************************

    unicode.h

    Unicode related functions

    This code is for converting between UTF-8, UTF-16, and 32-bit
    Unicode strings.  These functions roughly parallel C runtime
    library functions like mbtowc() and similar functions, but are
    specific for these Unicode encodings.  Specifically, there are
    functions that convert UTF-8 and UTF-16 char clusters to and from
    singular 32-bit Unicode chars.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

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
#define UTF8_CHAR_MAX   6
#define UTF16_CHAR_MAX  2

/* these are UTF-8 encoded strings for common characters */
#define UTF8_NBSP           "\xc2\xa0"          /* non-breaking space */
#define UTF8_MULTIPLY       "\xc3\x97"          /* multiplication symbol */
#define UTF8_DEGREES        "\xc2\xb0"          /* degrees symbol */

#define a_RING              "\xc3\xa5"          /* small a with a ring */
#define a_UMLAUT            "\xc3\xa4"          /* small a with an umlaut */
#define o_UMLAUT            "\xc3\xb6"          /* small o with an umlaut */
#define u_UMLAUT            "\xc3\xbc"          /* small u with an umlaut */
#define e_ACUTE             "\xc3\xa9"          /* small e with an acute */

#define A_RING              "\xc3\x85"          /* capital A with a ring */
#define A_UMLAUT            "\xc3\x84"          /* capital A with an umlaut */
#define O_UMLAUT            "\xc3\x96"          /* capital O with an umlaut */
#define U_UMLAUT            "\xc3\x9c"          /* capital U with an umlaut */
#define E_ACUTE             "\xc3\x89"          /* capital E with an acute */

#define UTF8_LEFT           "\xe2\x86\x90"      /* cursor left */
#define UTF8_RIGHT          "\xe2\x86\x92"      /* cursor right */
#define UTF8_UP             "\xe2\x86\x91"      /* cursor up */
#define UTF8_DOWN           "\xe2\x86\x93"      /* cursor down */



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
