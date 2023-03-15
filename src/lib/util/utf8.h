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
#define n_TILDE                 "\xc3\xb1"          /* small n with a tilde */

#define A_RING                  "\xc3\x85"          /* capital A with a ring */
#define A_UMLAUT                "\xc3\x84"          /* capital A with an umlaut */
#define O_UMLAUT                "\xc3\x96"          /* capital O with an umlaut */
#define U_UMLAUT                "\xc3\x9c"          /* capital U with an umlaut */
#define E_ACUTE                 "\xc3\x89"          /* capital E with an acute */
#define N_TILDE                 "\xc3\x91"          /* capital N with a tilde */

#define UTF8_LEFT               "\xe2\x86\x90"      /* cursor left */
#define UTF8_RIGHT              "\xe2\x86\x92"      /* cursor right */
#define UTF8_UP                 "\xe2\x86\x91"      /* cursor up */
#define UTF8_DOWN               "\xe2\x86\x93"      /* cursor down */

#endif // MAME_LIB_UTIL_UTF8_H
