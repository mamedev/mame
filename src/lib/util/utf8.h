// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Curt Coder, hap
/***************************************************************************

    utf8.h

    UTF-8 string literals.

***************************************************************************/

#ifndef MAME_LIB_UTIL_UTF8_H
#define MAME_LIB_UTIL_UTF8_H

// these are UTF-8 encoded strings for common characters
#define UTF8_NBSP               "\xc2\xa0"          /* non-breaking space */

#define UTF8_MULTIPLY           "\xc3\x97"          /* multiplication sign */
#define UTF8_DEGREES            "\xc2\xb0"          /* degrees symbol */

#define UTF8_LEFT               "\xe2\x86\x90"      /* cursor left */
#define UTF8_RIGHT              "\xe2\x86\x92"      /* cursor right */
#define UTF8_UP                 "\xe2\x86\x91"      /* cursor up */
#define UTF8_DOWN               "\xe2\x86\x93"      /* cursor down */

#endif // MAME_LIB_UTIL_UTF8_H
