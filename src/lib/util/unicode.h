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
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include "osdcore.h"



/***************************************************************************
	CONSTANTS
***************************************************************************/

//! size of the first 17 Unicode planes
#define	UNICODE_PLANESIZE		0x110000

#ifndef	NEED_UNICODE_RANGES
#define	NEED_UNICODE_RANGES		1		//!< define to 1, if the name, first or last of the range of a code is needed
#endif
#ifndef	NEED_UNICODE_NAME
#define	NEED_UNICODE_NAME		1		//!< define to 1, if the name of a code is needed
#endif
#ifndef	NEED_UNICODE_NAME10
#define	NEED_UNICODE_NAME10		1		//!< define to 1, if the short name of a code is needed
#endif
#ifndef	NEED_UNICODE_GCAT
#define	NEED_UNICODE_GCAT		1		//!< define to 1, if the general category of a code is needed
#endif
#ifndef	NEED_UNICODE_CCOM
#define	NEED_UNICODE_CCOM		1		//!< define to 1, if the canonical combining (name) of a code is needed
#endif
#ifndef	NEED_UNICODE_BIDI
#define	NEED_UNICODE_BIDI		1		//!< define to 1, if the bidirectional category of a code is needed
#endif
#ifndef	NEED_UNICODE_DECO
#define	NEED_UNICODE_DECO		1		//!< define to 1, if the decomposition codes of a code are needed
#endif
#ifndef	NEED_UNICODE_DECIMAL
#define	NEED_UNICODE_DECIMAL	1		//!< define to 1, if the decimal value of a code is needed
#endif
#ifndef	NEED_UNICODE_DIGIT
#define	NEED_UNICODE_DIGIT		1		//!< define to 1, if the digit value of a code is needed
#endif
#ifndef	NEED_UNICODE_NUMERIC
#define	NEED_UNICODE_NUMERIC	1		//!< define to 1, if the numeric value of a code is needed
#endif
#ifndef	NEED_UNICODE_MIRRORED
#define	NEED_UNICODE_MIRRORED	1		//!< define to 1, if the mirrored flag of a code is needed
#endif
#ifndef	NEED_UNICODE_DECN
#define	NEED_UNICODE_DECN		1		//!< define to 1, if access to decomposed code [n] of a code is needed
#endif
#ifndef	NEED_UNICODE_UCASE
#define	NEED_UNICODE_UCASE		1		//!< define to 1, if the upper case value of a code is needed
#endif
#ifndef	NEED_UNICODE_LCASE
#define	NEED_UNICODE_LCASE		1		//!< define to 1, if the lower case value of a code is needed
#endif
#ifndef	NEED_UNICODE_TCASE
#define	NEED_UNICODE_TCASE		1		//!< define to 1, if the title case value of a code is needed
#endif
#ifndef	NEED_UNICODE_WIDTH
#define	NEED_UNICODE_WIDTH		1		//!< define to 1, if the glyph width of a code is needed
#endif

#define UTF8_CHAR_MAX			6		//!< maximum size of Unicode UTF-8 encoding
#define UTF16_CHAR_MAX			2		//!< maximum size of Unicode UTF-16 encoding

/* these are UTF-8 encoded strings for common characters */
#define UTF8_NBSP           "\xc2\xa0"          //!< non-breaking space
#define UTF8_MULTIPLY       "\xc3\x97"          //!< multiplication symbol
#define UTF8_DEGREES        "\xc2\xb0"          //!< degrees symbol

#define a_RING              "\xc3\xa5"          //!< small a with a ring
#define a_UMLAUT            "\xc3\xa4"          //!< small a with an umlaut
#define o_UMLAUT            "\xc3\xb6"          //!< small o with an umlaut
#define u_UMLAUT            "\xc3\xbc"          //!< small u with an umlaut
#define e_ACUTE             "\xc3\xa9"          //!< small e with an acute

#define A_RING              "\xc3\x85"          //!< capital A with a ring
#define A_UMLAUT            "\xc3\x84"          //!< capital A with an umlaut
#define O_UMLAUT            "\xc3\x96"          //!< capital O with an umlaut
#define U_UMLAUT            "\xc3\x9c"          //!< capital U with an umlaut
#define E_ACUTE             "\xc3\x89"          //!< capital E with an acute

#define UTF8_LEFT           "\xe2\x86\x90"      //!< cursor left
#define UTF8_RIGHT          "\xe2\x86\x92"		//!< cursor right
#define UTF8_UP             "\xe2\x86\x91"      //!< cursor up
#define UTF8_DOWN           "\xe2\x86\x93"      //!< cursor down



/***************************************************************************
	TYPE DEFINITIONS
***************************************************************************/

typedef UINT16 utf16_char;						//!< type used for UTF-16 encoded values
typedef UINT32 unicode_char;					//!< type used for full width Unicode values



/***************************************************************************
	FUNCTION PROTOTYPES
***************************************************************************/

//! tests to see if a unicode char is a valid code point
int uchar_isvalid(unicode_char uchar);

//! convert an UTF-8 sequence into an unicode character
int uchar_from_utf8(unicode_char *uchar, const char *utf8char, size_t count);

//! convert a UTF-16 sequence into an unicode character
int uchar_from_utf16(unicode_char *uchar, const utf16_char *utf16char, size_t count);

//! convert a UTF-16 sequence into an unicode character from a flipped byte order
int uchar_from_utf16f(unicode_char *uchar, const utf16_char *utf16char, size_t count);

//! convert an unicode character into a UTF-8 sequence
int utf8_from_uchar(char *utf8string, size_t count, unicode_char uchar);

//! convert an unicode character into a UTF-16 sequence
int utf16_from_uchar(utf16_char *utf16string, size_t count, unicode_char uchar);

//! convert an unicode character into a UTF-16 sequence with flipped endianness
int utf16f_from_uchar(utf16_char *utf16string, size_t count, unicode_char uchar);

/* misc UTF-8 helpers */
//! return a pointer to the previous character in a string
const char *utf8_previous_char(const char *utf8string);

//! return true if the given string is a properly formed sequence of UTF-8 characters
int utf8_is_valid_string(const char *utf8string);

//! return the number of decoded Unicode values in UTF-8 encoded string
size_t utf8_strlen(const char* src);

/* 8 bit code to Unicode value lookup table handling (e.g. ISO-8859-1 aka Latin1) */
//! load a table translating UINT8 (unsigned char) to Unicode values
unicode_char * uchar_table_load(const char* name);

//! reverse lookup of uchar in a Unicode table
UINT8 uchar_table_index(unicode_char* table, unicode_char uchar);

//! free a unicode table
void uchar_table_free(unicode_char* table);

/* unicode_char array functions - string.h like */
//! return the unicode_char array length
size_t uchar_strlen(const unicode_char* src);

//! compare two unicode_char arrays
int uchar_strcmp(const unicode_char* dst, const unicode_char* src);

//! compare two unicode_char arrays with length limiting
int uchar_strncmp(const unicode_char* dst, const unicode_char* src, size_t len);

//! print a formatted string of ASCII characters to an unicode_char array (max 256 characters)
int uchar_sprintf(unicode_char* dst, const char* format, ...);

//! copy an array of unicode_char from source to destination
unicode_char* uchar_strcpy(unicode_char* dst, const unicode_char* src);

//! copy a length limited array of unicode_char from source to destination
unicode_char* uchar_strncpy(unicode_char* dst, const unicode_char* src, size_t len);

/***************************************************************************
 *	unicode.org published UnicodeData.txt
 *  Parser and property accessors
 ***************************************************************************/

//! load the specified UnicodeData.txt file an parse it
int unicode_data_load(const char* name);

//! free the UnicodeData.txt table memory
void unicode_data_free();

#if	NEED_UNICODE_GCAT
/**
 * @brief enumeration of the possible general categories
 */
typedef enum {
	gcat_0,		//!< invalid value
	gcat_Lu,	//!< Letter, Uppercase
	gcat_Ll,	//!< Letter, Lowercase
	gcat_Lt,	//!< Letter, Titlecase
	gcat_Mn,	//!< Mark, Non-Spacing
	gcat_Mc,	//!< Mark, Spacing Combining
	gcat_Me,	//!< Mark, Enclosing
	gcat_Nd,	//!< Number, Decimal Digit
	gcat_Nl,	//!< Number, Letter
	gcat_No,	//!< Number, Other
	gcat_Zs,	//!< Separator, Space
	gcat_Zl,	//!< Separator, Line
	gcat_Zp,	//!< Separator, Paragraph
	gcat_Cc,	//!< Other, Control
	gcat_Cf,	//!< Other, Format
	gcat_Cs,	//!< Other, Surrogate
	gcat_Co,	//!< Other, Private Use
	gcat_Cn,	//!< Other, Not Assigned (no characters have this property)
	gcat_Lm,	//!< Letter, Modifier
	gcat_Lo,	//!< Letter, Other
	gcat_Pc,	//!< Punctuation, Connector
	gcat_Pd,	//!< Punctuation, Dash
	gcat_Ps,	//!< Punctuation, Open
	gcat_Pe,	//!< Punctuation, Close
	gcat_Pi,	//!< Punctuation, Initial quote (may behave like Ps or Pe depending on usage)
	gcat_Pf,	//!< Punctuation, Final quote (may behave like Ps or Pe depending on usage)
	gcat_Po,	//!< Punctuation, Other
	gcat_Sm,	//!< Symbol, Math
	gcat_Sc,	//!< Symbol, Currency
	gcat_Sk,	//!< Symbol, Modifier
	gcat_So		//!< Symbol, Other
}	unicode_general_category;
#endif

#if	NEED_UNICODE_BIDI
/**
 * @brief enumeration of the possible bidirectional categories
 */
typedef enum {
	bidi_0,		//!< invalid value
	bidi_L,		//!< Left-to-Right
	bidi_LRE,	//!< Left-to-Right Embedding
	bidi_LRO,	//!< Left-to-Right Override
	bidi_R,		//!< Right-to-Left
	bidi_AL,	//!< Right-to-Left Arabic
	bidi_RLE,	//!< Right-to-Left Embedding
	bidi_RLO,	//!< Right-to-Left Override
	bidi_PDF,	//!< Pop Directional Format
	bidi_EN,	//!< European Number
	bidi_ES,	//!< European Number Separator
	bidi_ET,	//!< European Number Terminator
	bidi_AN,	//!< Arabic Number
	bidi_CS,	//!< Common Number Separator
	bidi_NSM,	//!< Non-Spacing Mark
	bidi_BN,	//!< Boundary Neutral
	bidi_B,		//!< Paragraph Separator
	bidi_S,		//!< Segment Separator
	bidi_WS,	//!< Whitespace
	bidi_ON		//!< Other Neutrals
}	unicode_bidirectional_category;
#endif

#if	NEED_UNICODE_DECO
/**
 * @brief enumeration of the possible decomposition mappings
 */
typedef enum {
	deco_0,			//!< invalid value
	deco_canonical, //!< canonical mapping
	deco_font,		//!< A font variant (e.g. a blackletter form)
	deco_noBreak,	//!< A no-break version of a space or hyphen
	deco_initial,	//!< An initial presentation form (Arabic)
	deco_medial,	//!< A medial presentation form (Arabic)
	deco_final, 	//!< A final presentation form (Arabic)
	deco_isolated,	//!< An isolated presentation form (Arabic)
	deco_circle,	//!< An encircled form
	deco_super, 	//!< A superscript form
	deco_sub,		//!< A subscript form
	deco_vertical,	//!< A vertical layout presentation form
	deco_wide,		//!< A wide (or zenkaku) compatibility character
	deco_narrow,	//!< A narrow (or hankaku) compatibility character
	deco_small, 	//!< A small variant form (CNS compatibility)
	deco_square,	//!< A CJK squared font variant
	deco_fraction,	//!< A vulgar fraction form
	deco_compat 	//!< Otherwise unspecified compatibility character
}	unicode_decomposition_mapping;
#endif

#if	NEED_UNICODE_RANGES
//! return the name of a range for a unicode char
const char * unicode_range_name(unicode_char uchar);
//! return the first value of a range for a unicode char
unicode_char unicode_range_first(unicode_char uchar);
//! return the last value of a range for a unicode char
unicode_char unicode_range_last(unicode_char uchar);
#endif

#if	NEED_UNICODE_NAME
//! return the name for a unicode char
const char * unicode_name(unicode_char uchar);
#endif

#if	NEED_UNICODE_NAME10
//! return the name10 for a unicode char
const char * unicode_name10(unicode_char uchar);
#endif

#if	NEED_UNICODE_GCAT
//! return the general category for a unicode char
unicode_general_category unicode_gcat(unicode_char uchar);
//! return a name for the general category for a unicode char
const char * unicode_gcat_name(unicode_char uchar);
#endif

#if	NEED_UNICODE_CCOM
//! return the type of canonical combining that is needed
UINT8 unicode_ccom(unicode_char uchar);
//! return a name for the type of canonical combining that is needed
const char * unicode_ccom_name(unicode_char uchar);
#endif

#if	NEED_UNICODE_BIDI
//! return the bidirectional category for a unicode char
unicode_bidirectional_category unicode_bidi(unicode_char uchar);
//! return a name for the bidirectional category for a unicode char
const char * unicode_bidi_name(unicode_char uchar);
#endif

#if	NEED_UNICODE_DECO
//! return the decomposition mapping for a unicode char
unicode_decomposition_mapping unicode_deco(unicode_char uchar);
//! return a name for the decomposition mapping for a unicode char
const char * unicode_deco_name(unicode_char uchar);
#if	NEED_UNICODE_DECN
//! return the n'th decomposed unicode character for uchar
unicode_char unicode_deco_n(unicode_char uchar, int n);
#endif
#endif

#if	NEED_UNICODE_DECIMAL
#define	UNICODE_NOT_DECIMAL	255
//! return the decimal value for a unicode char
UINT8 unicode_decimal(unicode_char uchar);
#endif

#if	NEED_UNICODE_DIGIT
#define	UNICODE_NOT_DIGIT	255
//! return the digit value for a unicode char
UINT8 unicode_digit(unicode_char uchar);
#endif

#if	NEED_UNICODE_NUMERIC
#define	UNICODE_NOT_NUMERIC	255
//! return the numeric value for a unicode char
UINT8 unicode_numeric(unicode_char uchar);
#endif

#if	NEED_UNICODE_MIRRORED
//! return the is-mirrored flag for a unicode char
bool unicode_mirrored(unicode_char uchar);
#endif

#if	NEED_UNICODE_UCASE
//! return the upper case value for a unicode char
unicode_char unicode_ucase(unicode_char uchar);
#endif

#if	NEED_UNICODE_LCASE
//! return the lower case value for a unicode char
unicode_char unicode_lcase(unicode_char uchar);
#endif

#if	NEED_UNICODE_TCASE
//! return the title case value for a unicode char
unicode_char unicode_tcase(unicode_char uchar);
#endif

#if	NEED_UNICODE_WIDTH
//! return (printing) glyph width for a unicode char
int unicode_width(unicode_char uchar);
#endif

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
