// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*********************************************************************

	unicode.c

	Unicode related functions

***************************************************************************/

#include "unicode.h"


/*-------------------------------------------------
	uchar_isvalid - return true if a given
	character is a legitimate unicode character
-------------------------------------------------*/

int uchar_isvalid(unicode_char uchar)
{
	return (uchar < 0x110000) && !((uchar >= 0xd800) && (uchar <= 0xdfff));
}


/*-------------------------------------------------
	uchar_from_utf8 - convert a UTF-8 sequence
	into a unicode character
-------------------------------------------------*/

int uchar_from_utf8(unicode_char *uchar, const char *utf8char, size_t count)
{
	unicode_char c, minchar;
	int auxlen, i;
	char auxchar;

	/* validate parameters */
	if (utf8char == NULL || count == 0)
		return 0;

	/* start with the first byte */
	c = (unsigned char) *utf8char;
	count--;
	utf8char++;

	/* based on that, determine how many additional bytes we need */
	if (c < 0x80)
	{
		/* unicode char 0x00000000 - 0x0000007F */
		c &= 0x7f;
		auxlen = 0;
		minchar = 0x00000000;
	}
	else if (c >= 0xc0 && c < 0xe0)
	{
		/* unicode char 0x00000080 - 0x000007FF */
		c &= 0x1f;
		auxlen = 1;
		minchar = 0x00000080;
	}
	else if (c >= 0xe0 && c < 0xf0)
	{
		/* unicode char 0x00000800 - 0x0000FFFF */
		c &= 0x0f;
		auxlen = 2;
		minchar = 0x00000800;
	}
	else if (c >= 0xf0 && c < 0xf8)
	{
		/* unicode char 0x00010000 - 0x001FFFFF */
		c &= 0x07;
		auxlen = 3;
		minchar = 0x00010000;
	}
	else if (c >= 0xf8 && c < 0xfc)
	{
		/* unicode char 0x00200000 - 0x03FFFFFF */
		c &= 0x03;
		auxlen = 4;
		minchar = 0x00200000;
	}
	else if (c >= 0xfc && c < 0xfe)
	{
		/* unicode char 0x04000000 - 0x7FFFFFFF */
		c &= 0x01;
		auxlen = 5;
		minchar = 0x04000000;
	}
	else
	{
		/* invalid */
		return -1;
	}

	/* exceeds the count? */
	if (auxlen > count)
		return -1;

	/* we now know how long the char is, now compute it */
	for (i = 0; i < auxlen; i++)
	{
		auxchar = utf8char[i];

		/* all auxillary chars must be between 0x80-0xbf */
		if ((auxchar & 0xc0) != 0x80)
			return -1;

		c = c << 6;
		c |= auxchar & 0x3f;
	}

	/* make sure that this char is above the minimum */
	if (c < minchar)
		return -1;

	*uchar = c;
	return auxlen + 1;
}


/*-------------------------------------------------
	uchar_from_utf16 - convert a UTF-16 sequence
	into a unicode character
-------------------------------------------------*/

int uchar_from_utf16(unicode_char *uchar, const utf16_char *utf16char, size_t count)
{
	int rc = -1;

	/* validate parameters */
	if (utf16char == NULL || count == 0)
		return 0;

	/* handle the two-byte case */
	if (utf16char[0] >= 0xd800 && utf16char[0] <= 0xdbff)
	{
		if (count > 1 && utf16char[1] >= 0xdc00 && utf16char[1] <= 0xdfff)
		{
			*uchar = 0x10000 + ((utf16char[0] & 0x3ff) * 0x400) + (utf16char[1] & 0x3ff);
			rc = 2;
		}
	}

	/* handle the one-byte case */
	else if (utf16char[0] < 0xdc00 || utf16char[0] > 0xdfff)
	{
		*uchar = utf16char[0];
		rc = 1;
	}

	return rc;
}


/*-------------------------------------------------
	uchar_from_utf16f - convert a UTF-16 sequence
	into a unicode character from a flipped
	byte order
-------------------------------------------------*/

int uchar_from_utf16f(unicode_char *uchar, const utf16_char *utf16char, size_t count)
{
	utf16_char buf[2] = {0};
	if (count > 0)
		buf[0] = FLIPENDIAN_INT16(utf16char[0]);
	if (count > 1)
		buf[1] = FLIPENDIAN_INT16(utf16char[1]);
	return uchar_from_utf16(uchar, buf, count);
}


/*-------------------------------------------------
	utf8_from_uchar - convert a unicode character
	into a UTF-8 sequence
-------------------------------------------------*/

int utf8_from_uchar(char *utf8string, size_t count, unicode_char uchar)
{
	int rc = 0;

	/* error on invalid characters */
	if (!uchar_isvalid(uchar))
		return -1;

	/* based on the value, output the appropriate number of bytes */
	if (uchar < 0x80)
	{
		/* unicode char 0x00000000 - 0x0000007F */
		if (count < 1)
			return -1;
		utf8string[rc++] = (char) uchar;
	}
	else if (uchar < 0x800)
	{
		/* unicode char 0x00000080 - 0x000007FF */
		if (count < 2)
			return -1;
		utf8string[rc++] = ((char) (uchar >> 6)) | 0xC0;
		utf8string[rc++] = ((char) ((uchar >> 0) & 0x3F)) | 0x80;
	}
	else if (uchar < 0x10000)
	{
		/* unicode char 0x00000800 - 0x0000FFFF */
		if (count < 3)
			return -1;
		utf8string[rc++] = ((char) (uchar >> 12)) | 0xE0;
		utf8string[rc++] = ((char) ((uchar >> 6) & 0x3F)) | 0x80;
		utf8string[rc++] = ((char) ((uchar >> 0) & 0x3F)) | 0x80;
	}
	else if (uchar < 0x00200000)
	{
		/* unicode char 0x00010000 - 0x001FFFFF */
		if (count < 4)
			return -1;
		utf8string[rc++] = ((char) (uchar >> 18)) | 0xF0;
		utf8string[rc++] = ((char) ((uchar >> 12) & 0x3F)) | 0x80;
		utf8string[rc++] = ((char) ((uchar >> 6) & 0x3F)) | 0x80;
		utf8string[rc++] = ((char) ((uchar >> 0) & 0x3F)) | 0x80;
	}
	else if (uchar < 0x04000000)
	{
		/* unicode char 0x00200000 - 0x03FFFFFF */
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
		/* unicode char 0x04000000 - 0x7FFFFFFF */
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


/*-------------------------------------------------
	utf16_from_uchar - convert a unicode character
	into a UTF-16 sequence
-------------------------------------------------*/

int utf16_from_uchar(utf16_char *utf16string, size_t count, unicode_char uchar)
{
	int rc;

	/* error on invalid characters */
	if (!uchar_isvalid(uchar))
		return -1;

	/* single word case */
	if (uchar < 0x10000)
	{
		if (count < 1)
			return -1;
		utf16string[0] = (utf16_char) uchar;
		rc = 1;
	}

	/* double word case */
	else if (uchar < 0x100000)
	{
		if (count < 2)
			return -1;
		utf16string[0] = ((uchar >> 10) & 0x03ff) | 0xd800;
		utf16string[1] = ((uchar >>  0) & 0x03ff) | 0xdc00;
		rc = 2;
	}
	else
		return -1;
	return rc;
}


/*-------------------------------------------------
	utf16_from_uchar - convert a unicode character
	into a UTF-16 sequence with flipped endianness
-------------------------------------------------*/

int utf16f_from_uchar(utf16_char *utf16string, size_t count, unicode_char uchar)
{
	int rc;
	utf16_char buf[2] = { 0, 0 };

	rc = utf16_from_uchar(buf, count, uchar);

	if (rc >= 1)
		utf16string[0] = FLIPENDIAN_INT16(buf[0]);
	if (rc >= 2)
		utf16string[1] = FLIPENDIAN_INT16(buf[1]);
	return rc;
}


/*-------------------------------------------------
	utf8_previous_char - return a pointer to the
	previous character in a string
-------------------------------------------------*/

const char *utf8_previous_char(const char *utf8string)
{
	while ((*--utf8string & 0xc0) == 0x80)
		;
	return utf8string;
}


/*-------------------------------------------------
	utf8_is_valid_string - return true if the
	given string is a properly formed sequence of
	UTF-8 characters
-------------------------------------------------*/

int utf8_is_valid_string(const char *utf8string)
{
	int remaining_length = strlen(utf8string);

	while (*utf8string != 0)
	{
		unicode_char uchar = 0;
		int charlen;

		/* extract the current character and verify it */
		charlen = uchar_from_utf8(&uchar, utf8string, remaining_length);
		if (charlen <= 0 || uchar == 0 || !uchar_isvalid(uchar))
			return FALSE;

		/* advance */
		utf8string += charlen;
		remaining_length -= charlen;
	}

	return TRUE;
}

//**************************************************************************
//  ISO-8859 to Unicode lookup tables
//  Source: http://www.unicode.org/Public/MAPPINGS/ISO8859/
//**************************************************************************

/***************************************************************************
#
#	Name:             ISO/IEC 8859-1:1998 to Unicode
#	Unicode version:  3.0
#	Table version:    1.0
#	Table format:     Format A
#	Date:             1999 July 27
#	Authors:          Ken Whistler <kenw@sybase.com>
#
#	Copyright (c) 1991-1999 Unicode, Inc.  All Rights reserved.
#
#	This file is provided as-is by Unicode, Inc. (The Unicode Consortium).
#	No claims are made as to fitness for any particular purpose.  No
#	warranties of any kind are expressed or implied.  The recipient
#	agrees to determine applicability of information provided.  If this
#	file has been provided on optical media by Unicode, Inc., the sole
#	remedy for any claim will be exchange of defective media within 90
#	days of receipt.
#
#	Unicode, Inc. hereby grants the right to freely use the information
#	supplied in this file in the creation of products supporting the
#	Unicode Standard, and to make copies of this file in any form for
#	internal or external distribution as long as this notice remains
#	attached.
#
#	General notes:
#
#	This table contains the data the Unicode Consortium has on how
#       ISO/IEC 8859-1:1998 characters map into Unicode.
#
#	Format:  Three tab-separated columns
#		 Column #1 is the ISO/IEC 8859-1 code (in hex as 0xXX)
#		 Column #2 is the Unicode (in hex as 0xXXXX)
#		 Column #3 the Unicode name (follows a comment sign, '#')
#
#	The entries are in ISO/IEC 8859-1 order.
#
#	Version history
#	1.0 version updates 0.1 version by adding mappings for all
#	control characters.
#
#	Updated versions of this file may be found in:
#		<ftp://ftp.unicode.org/Public/MAPPINGS/>
#
#	Any comments or problems, contact <errata@unicode.org>
#	Please note that <errata@unicode.org> is an archival address;
#	notices will be checked, but do not expect an immediate response.
#
 ***************************************************************************/
static const unicode_char lookup_iso8859_1[256] = {
/* 00 */	0x0000,	//	NULL
/* 01 */	0x0001,	//	START OF HEADING
/* 02 */	0x0002,	//	START OF TEXT
/* 03 */	0x0003,	//	END OF TEXT
/* 04 */	0x0004,	//	END OF TRANSMISSION
/* 05 */	0x0005,	//	ENQUIRY
/* 06 */	0x0006,	//	ACKNOWLEDGE
/* 07 */	0x0007,	//	BELL
/* 08 */	0x0008,	//	BACKSPACE
/* 09 */	0x0009,	//	HORIZONTAL TABULATION
/* 0A */	0x000A,	//	LINE FEED
/* 0B */	0x000B,	//	VERTICAL TABULATION
/* 0C */	0x000C,	//	FORM FEED
/* 0D */	0x000D,	//	CARRIAGE RETURN
/* 0E */	0x000E,	//	SHIFT OUT
/* 0F */	0x000F,	//	SHIFT IN
/* 10 */	0x0010,	//	DATA LINK ESCAPE
/* 11 */	0x0011,	//	DEVICE CONTROL ONE
/* 12 */	0x0012,	//	DEVICE CONTROL TWO
/* 13 */	0x0013,	//	DEVICE CONTROL THREE
/* 14 */	0x0014,	//	DEVICE CONTROL FOUR
/* 15 */	0x0015,	//	NEGATIVE ACKNOWLEDGE
/* 16 */	0x0016,	//	SYNCHRONOUS IDLE
/* 17 */	0x0017,	//	END OF TRANSMISSION BLOCK
/* 18 */	0x0018,	//	CANCEL
/* 19 */	0x0019,	//	END OF MEDIUM
/* 1A */	0x001A,	//	SUBSTITUTE
/* 1B */	0x001B,	//	ESCAPE
/* 1C */	0x001C,	//	FILE SEPARATOR
/* 1D */	0x001D,	//	GROUP SEPARATOR
/* 1E */	0x001E,	//	RECORD SEPARATOR
/* 1F */	0x001F,	//	UNIT SEPARATOR
/* 20 */	0x0020,	//	SPACE
/* 21 */	0x0021,	//	EXCLAMATION MARK
/* 22 */	0x0022,	//	QUOTATION MARK
/* 23 */	0x0023,	//	NUMBER SIGN
/* 24 */	0x0024,	//	DOLLAR SIGN
/* 25 */	0x0025,	//	PERCENT SIGN
/* 26 */	0x0026,	//	AMPERSAND
/* 27 */	0x0027,	//	APOSTROPHE
/* 28 */	0x0028,	//	LEFT PARENTHESIS
/* 29 */	0x0029,	//	RIGHT PARENTHESIS
/* 2A */	0x002A,	//	ASTERISK
/* 2B */	0x002B,	//	PLUS SIGN
/* 2C */	0x002C,	//	COMMA
/* 2D */	0x002D,	//	HYPHEN-MINUS
/* 2E */	0x002E,	//	FULL STOP
/* 2F */	0x002F,	//	SOLIDUS
/* 30 */	0x0030,	//	DIGIT ZERO
/* 31 */	0x0031,	//	DIGIT ONE
/* 32 */	0x0032,	//	DIGIT TWO
/* 33 */	0x0033,	//	DIGIT THREE
/* 34 */	0x0034,	//	DIGIT FOUR
/* 35 */	0x0035,	//	DIGIT FIVE
/* 36 */	0x0036,	//	DIGIT SIX
/* 37 */	0x0037,	//	DIGIT SEVEN
/* 38 */	0x0038,	//	DIGIT EIGHT
/* 39 */	0x0039,	//	DIGIT NINE
/* 3A */	0x003A,	//	COLON
/* 3B */	0x003B,	//	SEMICOLON
/* 3C */	0x003C,	//	LESS-THAN SIGN
/* 3D */	0x003D,	//	EQUALS SIGN
/* 3E */	0x003E,	//	GREATER-THAN SIGN
/* 3F */	0x003F,	//	QUESTION MARK
/* 40 */	0x0040,	//	COMMERCIAL AT
/* 41 */	0x0041,	//	LATIN CAPITAL LETTER A
/* 42 */	0x0042,	//	LATIN CAPITAL LETTER B
/* 43 */	0x0043,	//	LATIN CAPITAL LETTER C
/* 44 */	0x0044,	//	LATIN CAPITAL LETTER D
/* 45 */	0x0045,	//	LATIN CAPITAL LETTER E
/* 46 */	0x0046,	//	LATIN CAPITAL LETTER F
/* 47 */	0x0047,	//	LATIN CAPITAL LETTER G
/* 48 */	0x0048,	//	LATIN CAPITAL LETTER H
/* 49 */	0x0049,	//	LATIN CAPITAL LETTER I
/* 4A */	0x004A,	//	LATIN CAPITAL LETTER J
/* 4B */	0x004B,	//	LATIN CAPITAL LETTER K
/* 4C */	0x004C,	//	LATIN CAPITAL LETTER L
/* 4D */	0x004D,	//	LATIN CAPITAL LETTER M
/* 4E */	0x004E,	//	LATIN CAPITAL LETTER N
/* 4F */	0x004F,	//	LATIN CAPITAL LETTER O
/* 50 */	0x0050,	//	LATIN CAPITAL LETTER P
/* 51 */	0x0051,	//	LATIN CAPITAL LETTER Q
/* 52 */	0x0052,	//	LATIN CAPITAL LETTER R
/* 53 */	0x0053,	//	LATIN CAPITAL LETTER S
/* 54 */	0x0054,	//	LATIN CAPITAL LETTER T
/* 55 */	0x0055,	//	LATIN CAPITAL LETTER U
/* 56 */	0x0056,	//	LATIN CAPITAL LETTER V
/* 57 */	0x0057,	//	LATIN CAPITAL LETTER W
/* 58 */	0x0058,	//	LATIN CAPITAL LETTER X
/* 59 */	0x0059,	//	LATIN CAPITAL LETTER Y
/* 5A */	0x005A,	//	LATIN CAPITAL LETTER Z
/* 5B */	0x005B,	//	LEFT SQUARE BRACKET
/* 5C */	0x005C,	//	REVERSE SOLIDUS
/* 5D */	0x005D,	//	RIGHT SQUARE BRACKET
/* 5E */	0x005E,	//	CIRCUMFLEX ACCENT
/* 5F */	0x005F,	//	LOW LINE
/* 60 */	0x0060,	//	GRAVE ACCENT
/* 61 */	0x0061,	//	LATIN SMALL LETTER A
/* 62 */	0x0062,	//	LATIN SMALL LETTER B
/* 63 */	0x0063,	//	LATIN SMALL LETTER C
/* 64 */	0x0064,	//	LATIN SMALL LETTER D
/* 65 */	0x0065,	//	LATIN SMALL LETTER E
/* 66 */	0x0066,	//	LATIN SMALL LETTER F
/* 67 */	0x0067,	//	LATIN SMALL LETTER G
/* 68 */	0x0068,	//	LATIN SMALL LETTER H
/* 69 */	0x0069,	//	LATIN SMALL LETTER I
/* 6A */	0x006A,	//	LATIN SMALL LETTER J
/* 6B */	0x006B,	//	LATIN SMALL LETTER K
/* 6C */	0x006C,	//	LATIN SMALL LETTER L
/* 6D */	0x006D,	//	LATIN SMALL LETTER M
/* 6E */	0x006E,	//	LATIN SMALL LETTER N
/* 6F */	0x006F,	//	LATIN SMALL LETTER O
/* 70 */	0x0070,	//	LATIN SMALL LETTER P
/* 71 */	0x0071,	//	LATIN SMALL LETTER Q
/* 72 */	0x0072,	//	LATIN SMALL LETTER R
/* 73 */	0x0073,	//	LATIN SMALL LETTER S
/* 74 */	0x0074,	//	LATIN SMALL LETTER T
/* 75 */	0x0075,	//	LATIN SMALL LETTER U
/* 76 */	0x0076,	//	LATIN SMALL LETTER V
/* 77 */	0x0077,	//	LATIN SMALL LETTER W
/* 78 */	0x0078,	//	LATIN SMALL LETTER X
/* 79 */	0x0079,	//	LATIN SMALL LETTER Y
/* 7A */	0x007A,	//	LATIN SMALL LETTER Z
/* 7B */	0x007B,	//	LEFT CURLY BRACKET
/* 7C */	0x007C,	//	VERTICAL LINE
/* 7D */	0x007D,	//	RIGHT CURLY BRACKET
/* 7E */	0x007E,	//	TILDE
/* 7F */	0x007F,	//	DELETE
/* 80 */	0x0080,	//	<control>
/* 81 */	0x0081,	//	<control>
/* 82 */	0x0082,	//	<control>
/* 83 */	0x0083,	//	<control>
/* 84 */	0x0084,	//	<control>
/* 85 */	0x0085,	//	<control>
/* 86 */	0x0086,	//	<control>
/* 87 */	0x0087,	//	<control>
/* 88 */	0x0088,	//	<control>
/* 89 */	0x0089,	//	<control>
/* 8A */	0x008A,	//	<control>
/* 8B */	0x008B,	//	<control>
/* 8C */	0x008C,	//	<control>
/* 8D */	0x008D,	//	<control>
/* 8E */	0x008E,	//	<control>
/* 8F */	0x008F,	//	<control>
/* 90 */	0x0090,	//	<control>
/* 91 */	0x0091,	//	<control>
/* 92 */	0x0092,	//	<control>
/* 93 */	0x0093,	//	<control>
/* 94 */	0x0094,	//	<control>
/* 95 */	0x0095,	//	<control>
/* 96 */	0x0096,	//	<control>
/* 97 */	0x0097,	//	<control>
/* 98 */	0x0098,	//	<control>
/* 99 */	0x0099,	//	<control>
/* 9A */	0x009A,	//	<control>
/* 9B */	0x009B,	//	<control>
/* 9C */	0x009C,	//	<control>
/* 9D */	0x009D,	//	<control>
/* 9E */	0x009E,	//	<control>
/* 9F */	0x009F,	//	<control>
/* A0 */	0x00A0,	//	NO-BREAK SPACE
/* A1 */	0x00A1,	//	INVERTED EXCLAMATION MARK
/* A2 */	0x00A2,	//	CENT SIGN
/* A3 */	0x00A3,	//	POUND SIGN
/* A4 */	0x00A4,	//	CURRENCY SIGN
/* A5 */	0x00A5,	//	YEN SIGN
/* A6 */	0x00A6,	//	BROKEN BAR
/* A7 */	0x00A7,	//	SECTION SIGN
/* A8 */	0x00A8,	//	DIAERESIS
/* A9 */	0x00A9,	//	COPYRIGHT SIGN
/* AA */	0x00AA,	//	FEMININE ORDINAL INDICATOR
/* AB */	0x00AB,	//	LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
/* AC */	0x00AC,	//	NOT SIGN
/* AD */	0x00AD,	//	SOFT HYPHEN
/* AE */	0x00AE,	//	REGISTERED SIGN
/* AF */	0x00AF,	//	MACRON
/* B0 */	0x00B0,	//	DEGREE SIGN
/* B1 */	0x00B1,	//	PLUS-MINUS SIGN
/* B2 */	0x00B2,	//	SUPERSCRIPT TWO
/* B3 */	0x00B3,	//	SUPERSCRIPT THREE
/* B4 */	0x00B4,	//	ACUTE ACCENT
/* B5 */	0x00B5,	//	MICRO SIGN
/* B6 */	0x00B6,	//	PILCROW SIGN
/* B7 */	0x00B7,	//	MIDDLE DOT
/* B8 */	0x00B8,	//	CEDILLA
/* B9 */	0x00B9,	//	SUPERSCRIPT ONE
/* BA */	0x00BA,	//	MASCULINE ORDINAL INDICATOR
/* BB */	0x00BB,	//	RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
/* BC */	0x00BC,	//	VULGAR FRACTION ONE QUARTER
/* BD */	0x00BD,	//	VULGAR FRACTION ONE HALF
/* BE */	0x00BE,	//	VULGAR FRACTION THREE QUARTERS
/* BF */	0x00BF,	//	INVERTED QUESTION MARK
/* C0 */	0x00C0,	//	LATIN CAPITAL LETTER A WITH GRAVE
/* C1 */	0x00C1,	//	LATIN CAPITAL LETTER A WITH ACUTE
/* C2 */	0x00C2,	//	LATIN CAPITAL LETTER A WITH CIRCUMFLEX
/* C3 */	0x00C3,	//	LATIN CAPITAL LETTER A WITH TILDE
/* C4 */	0x00C4,	//	LATIN CAPITAL LETTER A WITH DIAERESIS
/* C5 */	0x00C5,	//	LATIN CAPITAL LETTER A WITH RING ABOVE
/* C6 */	0x00C6,	//	LATIN CAPITAL LETTER AE
/* C7 */	0x00C7,	//	LATIN CAPITAL LETTER C WITH CEDILLA
/* C8 */	0x00C8,	//	LATIN CAPITAL LETTER E WITH GRAVE
/* C9 */	0x00C9,	//	LATIN CAPITAL LETTER E WITH ACUTE
/* CA */	0x00CA,	//	LATIN CAPITAL LETTER E WITH CIRCUMFLEX
/* CB */	0x00CB,	//	LATIN CAPITAL LETTER E WITH DIAERESIS
/* CC */	0x00CC,	//	LATIN CAPITAL LETTER I WITH GRAVE
/* CD */	0x00CD,	//	LATIN CAPITAL LETTER I WITH ACUTE
/* CE */	0x00CE,	//	LATIN CAPITAL LETTER I WITH CIRCUMFLEX
/* CF */	0x00CF,	//	LATIN CAPITAL LETTER I WITH DIAERESIS
/* D0 */	0x00D0,	//	LATIN CAPITAL LETTER ETH (Icelandic)
/* D1 */	0x00D1,	//	LATIN CAPITAL LETTER N WITH TILDE
/* D2 */	0x00D2,	//	LATIN CAPITAL LETTER O WITH GRAVE
/* D3 */	0x00D3,	//	LATIN CAPITAL LETTER O WITH ACUTE
/* D4 */	0x00D4,	//	LATIN CAPITAL LETTER O WITH CIRCUMFLEX
/* D5 */	0x00D5,	//	LATIN CAPITAL LETTER O WITH TILDE
/* D6 */	0x00D6,	//	LATIN CAPITAL LETTER O WITH DIAERESIS
/* D7 */	0x00D7,	//	MULTIPLICATION SIGN
/* D8 */	0x00D8,	//	LATIN CAPITAL LETTER O WITH STROKE
/* D9 */	0x00D9,	//	LATIN CAPITAL LETTER U WITH GRAVE
/* DA */	0x00DA,	//	LATIN CAPITAL LETTER U WITH ACUTE
/* DB */	0x00DB,	//	LATIN CAPITAL LETTER U WITH CIRCUMFLEX
/* DC */	0x00DC,	//	LATIN CAPITAL LETTER U WITH DIAERESIS
/* DD */	0x00DD,	//	LATIN CAPITAL LETTER Y WITH ACUTE
/* DE */	0x00DE,	//	LATIN CAPITAL LETTER THORN (Icelandic)
/* DF */	0x00DF,	//	LATIN SMALL LETTER SHARP S (German)
/* E0 */	0x00E0,	//	LATIN SMALL LETTER A WITH GRAVE
/* E1 */	0x00E1,	//	LATIN SMALL LETTER A WITH ACUTE
/* E2 */	0x00E2,	//	LATIN SMALL LETTER A WITH CIRCUMFLEX
/* E3 */	0x00E3,	//	LATIN SMALL LETTER A WITH TILDE
/* E4 */	0x00E4,	//	LATIN SMALL LETTER A WITH DIAERESIS
/* E5 */	0x00E5,	//	LATIN SMALL LETTER A WITH RING ABOVE
/* E6 */	0x00E6,	//	LATIN SMALL LETTER AE
/* E7 */	0x00E7,	//	LATIN SMALL LETTER C WITH CEDILLA
/* E8 */	0x00E8,	//	LATIN SMALL LETTER E WITH GRAVE
/* E9 */	0x00E9,	//	LATIN SMALL LETTER E WITH ACUTE
/* EA */	0x00EA,	//	LATIN SMALL LETTER E WITH CIRCUMFLEX
/* EB */	0x00EB,	//	LATIN SMALL LETTER E WITH DIAERESIS
/* EC */	0x00EC,	//	LATIN SMALL LETTER I WITH GRAVE
/* ED */	0x00ED,	//	LATIN SMALL LETTER I WITH ACUTE
/* EE */	0x00EE,	//	LATIN SMALL LETTER I WITH CIRCUMFLEX
/* EF */	0x00EF,	//	LATIN SMALL LETTER I WITH DIAERESIS
/* F0 */	0x00F0,	//	LATIN SMALL LETTER ETH (Icelandic)
/* F1 */	0x00F1,	//	LATIN SMALL LETTER N WITH TILDE
/* F2 */	0x00F2,	//	LATIN SMALL LETTER O WITH GRAVE
/* F3 */	0x00F3,	//	LATIN SMALL LETTER O WITH ACUTE
/* F4 */	0x00F4,	//	LATIN SMALL LETTER O WITH CIRCUMFLEX
/* F5 */	0x00F5,	//	LATIN SMALL LETTER O WITH TILDE
/* F6 */	0x00F6,	//	LATIN SMALL LETTER O WITH DIAERESIS
/* F7 */	0x00F7,	//	DIVISION SIGN
/* F8 */	0x00F8,	//	LATIN SMALL LETTER O WITH STROKE
/* F9 */	0x00F9,	//	LATIN SMALL LETTER U WITH GRAVE
/* FA */	0x00FA,	//	LATIN SMALL LETTER U WITH ACUTE
/* FB */	0x00FB,	//	LATIN SMALL LETTER U WITH CIRCUMFLEX
/* FC */	0x00FC,	//	LATIN SMALL LETTER U WITH DIAERESIS
/* FD */	0x00FD,	//	LATIN SMALL LETTER Y WITH ACUTE
/* FE */	0x00FE,	//	LATIN SMALL LETTER THORN (Icelandic)
/* FF */	0x00FF	//	LATIN SMALL LETTER Y WITH DIAERESIS
};

/***************************************************************************
#
#	Name:             ISO 8859-2:1999 to Unicode
#	Unicode version:  3.0
#	Table version:    1.0
#	Table format:     Format A
#	Date:             1999 July 27
#	Authors:          Ken Whistler <kenw@sybase.com>
#
#	Copyright (c) 1991-1999 Unicode, Inc.  All Rights reserved.
#
#	This file is provided as-is by Unicode, Inc. (The Unicode Consortium).
#	No claims are made as to fitness for any particular purpose.  No
#	warranties of any kind are expressed or implied.  The recipient
#	agrees to determine applicability of information provided.  If this
#	file has been provided on optical media by Unicode, Inc., the sole
#	remedy for any claim will be exchange of defective media within 90
#	days of receipt.
#
#	Unicode, Inc. hereby grants the right to freely use the information
#	supplied in this file in the creation of products supporting the
#	Unicode Standard, and to make copies of this file in any form for
#	internal or external distribution as long as this notice remains
#	attached.
#
#	General notes:
#
#	This table contains the data the Unicode Consortium has on how
#       ISO/IEC 8859-2:1999 characters map into Unicode.
#
#	Format:  Three tab-separated columns
#		 Column #1 is the ISO/IEC 8859-2 code (in hex as 0xXX)
#		 Column #2 is the Unicode (in hex as 0xXXXX)
#		 Column #3 the Unicode name (follows a comment sign, '#')
#
#	The entries are in ISO/IEC 8859-2 order.
#
#	Version history
#	1.0 version updates 0.1 version by adding mappings for all
#	control characters.
#
#	Updated versions of this file may be found in:
#		<ftp://ftp.unicode.org/Public/MAPPINGS/>
#
#	Any comments or problems, contact <errata@unicode.org>
#	Please note that <errata@unicode.org> is an archival address;
#	notices will be checked, but do not expect an immediate response.
#
***************************************************************************/
static const unicode_char lookup_iso8859_2[256] = {
/* 00 */	0x0000,	//	NULL
/* 01 */	0x0001,	//	START OF HEADING
/* 02 */	0x0002,	//	START OF TEXT
/* 03 */	0x0003,	//	END OF TEXT
/* 04 */	0x0004,	//	END OF TRANSMISSION
/* 05 */	0x0005,	//	ENQUIRY
/* 06 */	0x0006,	//	ACKNOWLEDGE
/* 07 */	0x0007,	//	BELL
/* 08 */	0x0008,	//	BACKSPACE
/* 09 */	0x0009,	//	HORIZONTAL TABULATION
/* 0A */	0x000A,	//	LINE FEED
/* 0B */	0x000B,	//	VERTICAL TABULATION
/* 0C */	0x000C,	//	FORM FEED
/* 0D */	0x000D,	//	CARRIAGE RETURN
/* 0E */	0x000E,	//	SHIFT OUT
/* 0F */	0x000F,	//	SHIFT IN
/* 10 */	0x0010,	//	DATA LINK ESCAPE
/* 11 */	0x0011,	//	DEVICE CONTROL ONE
/* 12 */	0x0012,	//	DEVICE CONTROL TWO
/* 13 */	0x0013,	//	DEVICE CONTROL THREE
/* 14 */	0x0014,	//	DEVICE CONTROL FOUR
/* 15 */	0x0015,	//	NEGATIVE ACKNOWLEDGE
/* 16 */	0x0016,	//	SYNCHRONOUS IDLE
/* 17 */	0x0017,	//	END OF TRANSMISSION BLOCK
/* 18 */	0x0018,	//	CANCEL
/* 19 */	0x0019,	//	END OF MEDIUM
/* 1A */	0x001A,	//	SUBSTITUTE
/* 1B */	0x001B,	//	ESCAPE
/* 1C */	0x001C,	//	FILE SEPARATOR
/* 1D */	0x001D,	//	GROUP SEPARATOR
/* 1E */	0x001E,	//	RECORD SEPARATOR
/* 1F */	0x001F,	//	UNIT SEPARATOR
/* 20 */	0x0020,	//	SPACE
/* 21 */	0x0021,	//	EXCLAMATION MARK
/* 22 */	0x0022,	//	QUOTATION MARK
/* 23 */	0x0023,	//	NUMBER SIGN
/* 24 */	0x0024,	//	DOLLAR SIGN
/* 25 */	0x0025,	//	PERCENT SIGN
/* 26 */	0x0026,	//	AMPERSAND
/* 27 */	0x0027,	//	APOSTROPHE
/* 28 */	0x0028,	//	LEFT PARENTHESIS
/* 29 */	0x0029,	//	RIGHT PARENTHESIS
/* 2A */	0x002A,	//	ASTERISK
/* 2B */	0x002B,	//	PLUS SIGN
/* 2C */	0x002C,	//	COMMA
/* 2D */	0x002D,	//	HYPHEN-MINUS
/* 2E */	0x002E,	//	FULL STOP
/* 2F */	0x002F,	//	SOLIDUS
/* 30 */	0x0030,	//	DIGIT ZERO
/* 31 */	0x0031,	//	DIGIT ONE
/* 32 */	0x0032,	//	DIGIT TWO
/* 33 */	0x0033,	//	DIGIT THREE
/* 34 */	0x0034,	//	DIGIT FOUR
/* 35 */	0x0035,	//	DIGIT FIVE
/* 36 */	0x0036,	//	DIGIT SIX
/* 37 */	0x0037,	//	DIGIT SEVEN
/* 38 */	0x0038,	//	DIGIT EIGHT
/* 39 */	0x0039,	//	DIGIT NINE
/* 3A */	0x003A,	//	COLON
/* 3B */	0x003B,	//	SEMICOLON
/* 3C */	0x003C,	//	LESS-THAN SIGN
/* 3D */	0x003D,	//	EQUALS SIGN
/* 3E */	0x003E,	//	GREATER-THAN SIGN
/* 3F */	0x003F,	//	QUESTION MARK
/* 40 */	0x0040,	//	COMMERCIAL AT
/* 41 */	0x0041,	//	LATIN CAPITAL LETTER A
/* 42 */	0x0042,	//	LATIN CAPITAL LETTER B
/* 43 */	0x0043,	//	LATIN CAPITAL LETTER C
/* 44 */	0x0044,	//	LATIN CAPITAL LETTER D
/* 45 */	0x0045,	//	LATIN CAPITAL LETTER E
/* 46 */	0x0046,	//	LATIN CAPITAL LETTER F
/* 47 */	0x0047,	//	LATIN CAPITAL LETTER G
/* 48 */	0x0048,	//	LATIN CAPITAL LETTER H
/* 49 */	0x0049,	//	LATIN CAPITAL LETTER I
/* 4A */	0x004A,	//	LATIN CAPITAL LETTER J
/* 4B */	0x004B,	//	LATIN CAPITAL LETTER K
/* 4C */	0x004C,	//	LATIN CAPITAL LETTER L
/* 4D */	0x004D,	//	LATIN CAPITAL LETTER M
/* 4E */	0x004E,	//	LATIN CAPITAL LETTER N
/* 4F */	0x004F,	//	LATIN CAPITAL LETTER O
/* 50 */	0x0050,	//	LATIN CAPITAL LETTER P
/* 51 */	0x0051,	//	LATIN CAPITAL LETTER Q
/* 52 */	0x0052,	//	LATIN CAPITAL LETTER R
/* 53 */	0x0053,	//	LATIN CAPITAL LETTER S
/* 54 */	0x0054,	//	LATIN CAPITAL LETTER T
/* 55 */	0x0055,	//	LATIN CAPITAL LETTER U
/* 56 */	0x0056,	//	LATIN CAPITAL LETTER V
/* 57 */	0x0057,	//	LATIN CAPITAL LETTER W
/* 58 */	0x0058,	//	LATIN CAPITAL LETTER X
/* 59 */	0x0059,	//	LATIN CAPITAL LETTER Y
/* 5A */	0x005A,	//	LATIN CAPITAL LETTER Z
/* 5B */	0x005B,	//	LEFT SQUARE BRACKET
/* 5C */	0x005C,	//	REVERSE SOLIDUS
/* 5D */	0x005D,	//	RIGHT SQUARE BRACKET
/* 5E */	0x005E,	//	CIRCUMFLEX ACCENT
/* 5F */	0x005F,	//	LOW LINE
/* 60 */	0x0060,	//	GRAVE ACCENT
/* 61 */	0x0061,	//	LATIN SMALL LETTER A
/* 62 */	0x0062,	//	LATIN SMALL LETTER B
/* 63 */	0x0063,	//	LATIN SMALL LETTER C
/* 64 */	0x0064,	//	LATIN SMALL LETTER D
/* 65 */	0x0065,	//	LATIN SMALL LETTER E
/* 66 */	0x0066,	//	LATIN SMALL LETTER F
/* 67 */	0x0067,	//	LATIN SMALL LETTER G
/* 68 */	0x0068,	//	LATIN SMALL LETTER H
/* 69 */	0x0069,	//	LATIN SMALL LETTER I
/* 6A */	0x006A,	//	LATIN SMALL LETTER J
/* 6B */	0x006B,	//	LATIN SMALL LETTER K
/* 6C */	0x006C,	//	LATIN SMALL LETTER L
/* 6D */	0x006D,	//	LATIN SMALL LETTER M
/* 6E */	0x006E,	//	LATIN SMALL LETTER N
/* 6F */	0x006F,	//	LATIN SMALL LETTER O
/* 70 */	0x0070,	//	LATIN SMALL LETTER P
/* 71 */	0x0071,	//	LATIN SMALL LETTER Q
/* 72 */	0x0072,	//	LATIN SMALL LETTER R
/* 73 */	0x0073,	//	LATIN SMALL LETTER S
/* 74 */	0x0074,	//	LATIN SMALL LETTER T
/* 75 */	0x0075,	//	LATIN SMALL LETTER U
/* 76 */	0x0076,	//	LATIN SMALL LETTER V
/* 77 */	0x0077,	//	LATIN SMALL LETTER W
/* 78 */	0x0078,	//	LATIN SMALL LETTER X
/* 79 */	0x0079,	//	LATIN SMALL LETTER Y
/* 7A */	0x007A,	//	LATIN SMALL LETTER Z
/* 7B */	0x007B,	//	LEFT CURLY BRACKET
/* 7C */	0x007C,	//	VERTICAL LINE
/* 7D */	0x007D,	//	RIGHT CURLY BRACKET
/* 7E */	0x007E,	//	TILDE
/* 7F */	0x007F,	//	DELETE
/* 80 */	0x0080,	//	<control>
/* 81 */	0x0081,	//	<control>
/* 82 */	0x0082,	//	<control>
/* 83 */	0x0083,	//	<control>
/* 84 */	0x0084,	//	<control>
/* 85 */	0x0085,	//	<control>
/* 86 */	0x0086,	//	<control>
/* 87 */	0x0087,	//	<control>
/* 88 */	0x0088,	//	<control>
/* 89 */	0x0089,	//	<control>
/* 8A */	0x008A,	//	<control>
/* 8B */	0x008B,	//	<control>
/* 8C */	0x008C,	//	<control>
/* 8D */	0x008D,	//	<control>
/* 8E */	0x008E,	//	<control>
/* 8F */	0x008F,	//	<control>
/* 90 */	0x0090,	//	<control>
/* 91 */	0x0091,	//	<control>
/* 92 */	0x0092,	//	<control>
/* 93 */	0x0093,	//	<control>
/* 94 */	0x0094,	//	<control>
/* 95 */	0x0095,	//	<control>
/* 96 */	0x0096,	//	<control>
/* 97 */	0x0097,	//	<control>
/* 98 */	0x0098,	//	<control>
/* 99 */	0x0099,	//	<control>
/* 9A */	0x009A,	//	<control>
/* 9B */	0x009B,	//	<control>
/* 9C */	0x009C,	//	<control>
/* 9D */	0x009D,	//	<control>
/* 9E */	0x009E,	//	<control>
/* 9F */	0x009F,	//	<control>
/* A0 */	0x00A0,	//	NO-BREAK SPACE
/* A1 */	0x0104,	//	LATIN CAPITAL LETTER A WITH OGONEK
/* A2 */	0x02D8,	//	BREVE
/* A3 */	0x0141,	//	LATIN CAPITAL LETTER L WITH STROKE
/* A4 */	0x00A4,	//	CURRENCY SIGN
/* A5 */	0x013D,	//	LATIN CAPITAL LETTER L WITH CARON
/* A6 */	0x015A,	//	LATIN CAPITAL LETTER S WITH ACUTE
/* A7 */	0x00A7,	//	SECTION SIGN
/* A8 */	0x00A8,	//	DIAERESIS
/* A9 */	0x0160,	//	LATIN CAPITAL LETTER S WITH CARON
/* AA */	0x015E,	//	LATIN CAPITAL LETTER S WITH CEDILLA
/* AB */	0x0164,	//	LATIN CAPITAL LETTER T WITH CARON
/* AC */	0x0179,	//	LATIN CAPITAL LETTER Z WITH ACUTE
/* AD */	0x00AD,	//	SOFT HYPHEN
/* AE */	0x017D,	//	LATIN CAPITAL LETTER Z WITH CARON
/* AF */	0x017B,	//	LATIN CAPITAL LETTER Z WITH DOT ABOVE
/* B0 */	0x00B0,	//	DEGREE SIGN
/* B1 */	0x0105,	//	LATIN SMALL LETTER A WITH OGONEK
/* B2 */	0x02DB,	//	OGONEK
/* B3 */	0x0142,	//	LATIN SMALL LETTER L WITH STROKE
/* B4 */	0x00B4,	//	ACUTE ACCENT
/* B5 */	0x013E,	//	LATIN SMALL LETTER L WITH CARON
/* B6 */	0x015B,	//	LATIN SMALL LETTER S WITH ACUTE
/* B7 */	0x02C7,	//	CARON
/* B8 */	0x00B8,	//	CEDILLA
/* B9 */	0x0161,	//	LATIN SMALL LETTER S WITH CARON
/* BA */	0x015F,	//	LATIN SMALL LETTER S WITH CEDILLA
/* BB */	0x0165,	//	LATIN SMALL LETTER T WITH CARON
/* BC */	0x017A,	//	LATIN SMALL LETTER Z WITH ACUTE
/* BD */	0x02DD,	//	DOUBLE ACUTE ACCENT
/* BE */	0x017E,	//	LATIN SMALL LETTER Z WITH CARON
/* BF */	0x017C,	//	LATIN SMALL LETTER Z WITH DOT ABOVE
/* C0 */	0x0154,	//	LATIN CAPITAL LETTER R WITH ACUTE
/* C1 */	0x00C1,	//	LATIN CAPITAL LETTER A WITH ACUTE
/* C2 */	0x00C2,	//	LATIN CAPITAL LETTER A WITH CIRCUMFLEX
/* C3 */	0x0102,	//	LATIN CAPITAL LETTER A WITH BREVE
/* C4 */	0x00C4,	//	LATIN CAPITAL LETTER A WITH DIAERESIS
/* C5 */	0x0139,	//	LATIN CAPITAL LETTER L WITH ACUTE
/* C6 */	0x0106,	//	LATIN CAPITAL LETTER C WITH ACUTE
/* C7 */	0x00C7,	//	LATIN CAPITAL LETTER C WITH CEDILLA
/* C8 */	0x010C,	//	LATIN CAPITAL LETTER C WITH CARON
/* C9 */	0x00C9,	//	LATIN CAPITAL LETTER E WITH ACUTE
/* CA */	0x0118,	//	LATIN CAPITAL LETTER E WITH OGONEK
/* CB */	0x00CB,	//	LATIN CAPITAL LETTER E WITH DIAERESIS
/* CC */	0x011A,	//	LATIN CAPITAL LETTER E WITH CARON
/* CD */	0x00CD,	//	LATIN CAPITAL LETTER I WITH ACUTE
/* CE */	0x00CE,	//	LATIN CAPITAL LETTER I WITH CIRCUMFLEX
/* CF */	0x010E,	//	LATIN CAPITAL LETTER D WITH CARON
/* D0 */	0x0110,	//	LATIN CAPITAL LETTER D WITH STROKE
/* D1 */	0x0143,	//	LATIN CAPITAL LETTER N WITH ACUTE
/* D2 */	0x0147,	//	LATIN CAPITAL LETTER N WITH CARON
/* D3 */	0x00D3,	//	LATIN CAPITAL LETTER O WITH ACUTE
/* D4 */	0x00D4,	//	LATIN CAPITAL LETTER O WITH CIRCUMFLEX
/* D5 */	0x0150,	//	LATIN CAPITAL LETTER O WITH DOUBLE ACUTE
/* D6 */	0x00D6,	//	LATIN CAPITAL LETTER O WITH DIAERESIS
/* D7 */	0x00D7,	//	MULTIPLICATION SIGN
/* D8 */	0x0158,	//	LATIN CAPITAL LETTER R WITH CARON
/* D9 */	0x016E,	//	LATIN CAPITAL LETTER U WITH RING ABOVE
/* DA */	0x00DA,	//	LATIN CAPITAL LETTER U WITH ACUTE
/* DB */	0x0170,	//	LATIN CAPITAL LETTER U WITH DOUBLE ACUTE
/* DC */	0x00DC,	//	LATIN CAPITAL LETTER U WITH DIAERESIS
/* DD */	0x00DD,	//	LATIN CAPITAL LETTER Y WITH ACUTE
/* DE */	0x0162,	//	LATIN CAPITAL LETTER T WITH CEDILLA
/* DF */	0x00DF,	//	LATIN SMALL LETTER SHARP S
/* E0 */	0x0155,	//	LATIN SMALL LETTER R WITH ACUTE
/* E1 */	0x00E1,	//	LATIN SMALL LETTER A WITH ACUTE
/* E2 */	0x00E2,	//	LATIN SMALL LETTER A WITH CIRCUMFLEX
/* E3 */	0x0103,	//	LATIN SMALL LETTER A WITH BREVE
/* E4 */	0x00E4,	//	LATIN SMALL LETTER A WITH DIAERESIS
/* E5 */	0x013A,	//	LATIN SMALL LETTER L WITH ACUTE
/* E6 */	0x0107,	//	LATIN SMALL LETTER C WITH ACUTE
/* E7 */	0x00E7,	//	LATIN SMALL LETTER C WITH CEDILLA
/* E8 */	0x010D,	//	LATIN SMALL LETTER C WITH CARON
/* E9 */	0x00E9,	//	LATIN SMALL LETTER E WITH ACUTE
/* EA */	0x0119,	//	LATIN SMALL LETTER E WITH OGONEK
/* EB */	0x00EB,	//	LATIN SMALL LETTER E WITH DIAERESIS
/* EC */	0x011B,	//	LATIN SMALL LETTER E WITH CARON
/* ED */	0x00ED,	//	LATIN SMALL LETTER I WITH ACUTE
/* EE */	0x00EE,	//	LATIN SMALL LETTER I WITH CIRCUMFLEX
/* EF */	0x010F,	//	LATIN SMALL LETTER D WITH CARON
/* F0 */	0x0111,	//	LATIN SMALL LETTER D WITH STROKE
/* F1 */	0x0144,	//	LATIN SMALL LETTER N WITH ACUTE
/* F2 */	0x0148,	//	LATIN SMALL LETTER N WITH CARON
/* F3 */	0x00F3,	//	LATIN SMALL LETTER O WITH ACUTE
/* F4 */	0x00F4,	//	LATIN SMALL LETTER O WITH CIRCUMFLEX
/* F5 */	0x0151,	//	LATIN SMALL LETTER O WITH DOUBLE ACUTE
/* F6 */	0x00F6,	//	LATIN SMALL LETTER O WITH DIAERESIS
/* F7 */	0x00F7,	//	DIVISION SIGN
/* F8 */	0x0159,	//	LATIN SMALL LETTER R WITH CARON
/* F9 */	0x016F,	//	LATIN SMALL LETTER U WITH RING ABOVE
/* FA */	0x00FA,	//	LATIN SMALL LETTER U WITH ACUTE
/* FB */	0x0171,	//	LATIN SMALL LETTER U WITH DOUBLE ACUTE
/* FC */	0x00FC,	//	LATIN SMALL LETTER U WITH DIAERESIS
/* FD */	0x00FD,	//	LATIN SMALL LETTER Y WITH ACUTE
/* FE */	0x0163,	//	LATIN SMALL LETTER T WITH CEDILLA
/* FF */	0x02D9	//	DOT ABOVE
};

/***************************************************************************
#
#	Name:             ISO/IEC 8859-3:1999 to Unicode
#	Unicode version:  3.0
#	Table version:    1.0
#	Table format:     Format A
#	Date:             1999 July 27
#	Authors:          Ken Whistler <kenw@sybase.com>
#
#	Copyright (c) 1991-1999 Unicode, Inc.  All Rights reserved.
#
#	This file is provided as-is by Unicode, Inc. (The Unicode Consortium).
#	No claims are made as to fitness for any particular purpose.  No
#	warranties of any kind are expressed or implied.  The recipient
#	agrees to determine applicability of information provided.  If this
#	file has been provided on optical media by Unicode, Inc., the sole
#	remedy for any claim will be exchange of defective media within 90
#	days of receipt.
#
#	Unicode, Inc. hereby grants the right to freely use the information
#	supplied in this file in the creation of products supporting the
#	Unicode Standard, and to make copies of this file in any form for
#	internal or external distribution as long as this notice remains
#	attached.
#
#	General notes:
#
#	This table contains the data the Unicode Consortium has on how
#       ISO/IEC 8859-3:1999 characters map into Unicode.
#
#	Format:  Three tab-separated columns
#		 Column #1 is the ISO/IEC 8859-3 code (in hex as 0xXX)
#		 Column #2 is the Unicode (in hex as 0xXXXX)
#		 Column #3 the Unicode name (follows a comment sign, '#')
#
#	The entries are in ISO/IEC 8859-3 order.
#
#	Version history
#	1.0 version updates 0.1 version by adding mappings for all
#	control characters.
#
#	Updated versions of this file may be found in:
#		<ftp://ftp.unicode.org/Public/MAPPINGS/>
#
#	Any comments or problems, contact <errata@unicode.org>
#	Please note that <errata@unicode.org> is an archival address;
#	notices will be checked, but do not expect an immediate response.
#
***************************************************************************/
static const unicode_char lookup_iso8859_3[256] = {
/* 00 */	0x0000,	//	NULL
/* 01 */	0x0001,	//	START OF HEADING
/* 02 */	0x0002,	//	START OF TEXT
/* 03 */	0x0003,	//	END OF TEXT
/* 04 */	0x0004,	//	END OF TRANSMISSION
/* 05 */	0x0005,	//	ENQUIRY
/* 06 */	0x0006,	//	ACKNOWLEDGE
/* 07 */	0x0007,	//	BELL
/* 08 */	0x0008,	//	BACKSPACE
/* 09 */	0x0009,	//	HORIZONTAL TABULATION
/* 0A */	0x000A,	//	LINE FEED
/* 0B */	0x000B,	//	VERTICAL TABULATION
/* 0C */	0x000C,	//	FORM FEED
/* 0D */	0x000D,	//	CARRIAGE RETURN
/* 0E */	0x000E,	//	SHIFT OUT
/* 0F */	0x000F,	//	SHIFT IN
/* 10 */	0x0010,	//	DATA LINK ESCAPE
/* 11 */	0x0011,	//	DEVICE CONTROL ONE
/* 12 */	0x0012,	//	DEVICE CONTROL TWO
/* 13 */	0x0013,	//	DEVICE CONTROL THREE
/* 14 */	0x0014,	//	DEVICE CONTROL FOUR
/* 15 */	0x0015,	//	NEGATIVE ACKNOWLEDGE
/* 16 */	0x0016,	//	SYNCHRONOUS IDLE
/* 17 */	0x0017,	//	END OF TRANSMISSION BLOCK
/* 18 */	0x0018,	//	CANCEL
/* 19 */	0x0019,	//	END OF MEDIUM
/* 1A */	0x001A,	//	SUBSTITUTE
/* 1B */	0x001B,	//	ESCAPE
/* 1C */	0x001C,	//	FILE SEPARATOR
/* 1D */	0x001D,	//	GROUP SEPARATOR
/* 1E */	0x001E,	//	RECORD SEPARATOR
/* 1F */	0x001F,	//	UNIT SEPARATOR
/* 20 */	0x0020,	//	SPACE
/* 21 */	0x0021,	//	EXCLAMATION MARK
/* 22 */	0x0022,	//	QUOTATION MARK
/* 23 */	0x0023,	//	NUMBER SIGN
/* 24 */	0x0024,	//	DOLLAR SIGN
/* 25 */	0x0025,	//	PERCENT SIGN
/* 26 */	0x0026,	//	AMPERSAND
/* 27 */	0x0027,	//	APOSTROPHE
/* 28 */	0x0028,	//	LEFT PARENTHESIS
/* 29 */	0x0029,	//	RIGHT PARENTHESIS
/* 2A */	0x002A,	//	ASTERISK
/* 2B */	0x002B,	//	PLUS SIGN
/* 2C */	0x002C,	//	COMMA
/* 2D */	0x002D,	//	HYPHEN-MINUS
/* 2E */	0x002E,	//	FULL STOP
/* 2F */	0x002F,	//	SOLIDUS
/* 30 */	0x0030,	//	DIGIT ZERO
/* 31 */	0x0031,	//	DIGIT ONE
/* 32 */	0x0032,	//	DIGIT TWO
/* 33 */	0x0033,	//	DIGIT THREE
/* 34 */	0x0034,	//	DIGIT FOUR
/* 35 */	0x0035,	//	DIGIT FIVE
/* 36 */	0x0036,	//	DIGIT SIX
/* 37 */	0x0037,	//	DIGIT SEVEN
/* 38 */	0x0038,	//	DIGIT EIGHT
/* 39 */	0x0039,	//	DIGIT NINE
/* 3A */	0x003A,	//	COLON
/* 3B */	0x003B,	//	SEMICOLON
/* 3C */	0x003C,	//	LESS-THAN SIGN
/* 3D */	0x003D,	//	EQUALS SIGN
/* 3E */	0x003E,	//	GREATER-THAN SIGN
/* 3F */	0x003F,	//	QUESTION MARK
/* 40 */	0x0040,	//	COMMERCIAL AT
/* 41 */	0x0041,	//	LATIN CAPITAL LETTER A
/* 42 */	0x0042,	//	LATIN CAPITAL LETTER B
/* 43 */	0x0043,	//	LATIN CAPITAL LETTER C
/* 44 */	0x0044,	//	LATIN CAPITAL LETTER D
/* 45 */	0x0045,	//	LATIN CAPITAL LETTER E
/* 46 */	0x0046,	//	LATIN CAPITAL LETTER F
/* 47 */	0x0047,	//	LATIN CAPITAL LETTER G
/* 48 */	0x0048,	//	LATIN CAPITAL LETTER H
/* 49 */	0x0049,	//	LATIN CAPITAL LETTER I
/* 4A */	0x004A,	//	LATIN CAPITAL LETTER J
/* 4B */	0x004B,	//	LATIN CAPITAL LETTER K
/* 4C */	0x004C,	//	LATIN CAPITAL LETTER L
/* 4D */	0x004D,	//	LATIN CAPITAL LETTER M
/* 4E */	0x004E,	//	LATIN CAPITAL LETTER N
/* 4F */	0x004F,	//	LATIN CAPITAL LETTER O
/* 50 */	0x0050,	//	LATIN CAPITAL LETTER P
/* 51 */	0x0051,	//	LATIN CAPITAL LETTER Q
/* 52 */	0x0052,	//	LATIN CAPITAL LETTER R
/* 53 */	0x0053,	//	LATIN CAPITAL LETTER S
/* 54 */	0x0054,	//	LATIN CAPITAL LETTER T
/* 55 */	0x0055,	//	LATIN CAPITAL LETTER U
/* 56 */	0x0056,	//	LATIN CAPITAL LETTER V
/* 57 */	0x0057,	//	LATIN CAPITAL LETTER W
/* 58 */	0x0058,	//	LATIN CAPITAL LETTER X
/* 59 */	0x0059,	//	LATIN CAPITAL LETTER Y
/* 5A */	0x005A,	//	LATIN CAPITAL LETTER Z
/* 5B */	0x005B,	//	LEFT SQUARE BRACKET
/* 5C */	0x005C,	//	REVERSE SOLIDUS
/* 5D */	0x005D,	//	RIGHT SQUARE BRACKET
/* 5E */	0x005E,	//	CIRCUMFLEX ACCENT
/* 5F */	0x005F,	//	LOW LINE
/* 60 */	0x0060,	//	GRAVE ACCENT
/* 61 */	0x0061,	//	LATIN SMALL LETTER A
/* 62 */	0x0062,	//	LATIN SMALL LETTER B
/* 63 */	0x0063,	//	LATIN SMALL LETTER C
/* 64 */	0x0064,	//	LATIN SMALL LETTER D
/* 65 */	0x0065,	//	LATIN SMALL LETTER E
/* 66 */	0x0066,	//	LATIN SMALL LETTER F
/* 67 */	0x0067,	//	LATIN SMALL LETTER G
/* 68 */	0x0068,	//	LATIN SMALL LETTER H
/* 69 */	0x0069,	//	LATIN SMALL LETTER I
/* 6A */	0x006A,	//	LATIN SMALL LETTER J
/* 6B */	0x006B,	//	LATIN SMALL LETTER K
/* 6C */	0x006C,	//	LATIN SMALL LETTER L
/* 6D */	0x006D,	//	LATIN SMALL LETTER M
/* 6E */	0x006E,	//	LATIN SMALL LETTER N
/* 6F */	0x006F,	//	LATIN SMALL LETTER O
/* 70 */	0x0070,	//	LATIN SMALL LETTER P
/* 71 */	0x0071,	//	LATIN SMALL LETTER Q
/* 72 */	0x0072,	//	LATIN SMALL LETTER R
/* 73 */	0x0073,	//	LATIN SMALL LETTER S
/* 74 */	0x0074,	//	LATIN SMALL LETTER T
/* 75 */	0x0075,	//	LATIN SMALL LETTER U
/* 76 */	0x0076,	//	LATIN SMALL LETTER V
/* 77 */	0x0077,	//	LATIN SMALL LETTER W
/* 78 */	0x0078,	//	LATIN SMALL LETTER X
/* 79 */	0x0079,	//	LATIN SMALL LETTER Y
/* 7A */	0x007A,	//	LATIN SMALL LETTER Z
/* 7B */	0x007B,	//	LEFT CURLY BRACKET
/* 7C */	0x007C,	//	VERTICAL LINE
/* 7D */	0x007D,	//	RIGHT CURLY BRACKET
/* 7E */	0x007E,	//	TILDE
/* 7F */	0x007F,	//	DELETE
/* 80 */	0x0080,	//	<control>
/* 81 */	0x0081,	//	<control>
/* 82 */	0x0082,	//	<control>
/* 83 */	0x0083,	//	<control>
/* 84 */	0x0084,	//	<control>
/* 85 */	0x0085,	//	<control>
/* 86 */	0x0086,	//	<control>
/* 87 */	0x0087,	//	<control>
/* 88 */	0x0088,	//	<control>
/* 89 */	0x0089,	//	<control>
/* 8A */	0x008A,	//	<control>
/* 8B */	0x008B,	//	<control>
/* 8C */	0x008C,	//	<control>
/* 8D */	0x008D,	//	<control>
/* 8E */	0x008E,	//	<control>
/* 8F */	0x008F,	//	<control>
/* 90 */	0x0090,	//	<control>
/* 91 */	0x0091,	//	<control>
/* 92 */	0x0092,	//	<control>
/* 93 */	0x0093,	//	<control>
/* 94 */	0x0094,	//	<control>
/* 95 */	0x0095,	//	<control>
/* 96 */	0x0096,	//	<control>
/* 97 */	0x0097,	//	<control>
/* 98 */	0x0098,	//	<control>
/* 99 */	0x0099,	//	<control>
/* 9A */	0x009A,	//	<control>
/* 9B */	0x009B,	//	<control>
/* 9C */	0x009C,	//	<control>
/* 9D */	0x009D,	//	<control>
/* 9E */	0x009E,	//	<control>
/* 9F */	0x009F,	//	<control>
/* A0 */	0x00A0,	//	NO-BREAK SPACE
/* A1 */	0x0126,	//	LATIN CAPITAL LETTER H WITH STROKE
/* A2 */	0x02D8,	//	BREVE
/* A3 */	0x00A3,	//	POUND SIGN
/* A4 */	0x00A4,	//	CURRENCY SIGN
/* A6 */	0x0124,	//	LATIN CAPITAL LETTER H WITH CIRCUMFLEX
/* A7 */	0x00A7,	//	SECTION SIGN
/* A8 */	0x00A8,	//	DIAERESIS
/* A9 */	0x0130,	//	LATIN CAPITAL LETTER I WITH DOT ABOVE
/* AA */	0x015E,	//	LATIN CAPITAL LETTER S WITH CEDILLA
/* AB */	0x011E,	//	LATIN CAPITAL LETTER G WITH BREVE
/* AC */	0x0134,	//	LATIN CAPITAL LETTER J WITH CIRCUMFLEX
/* AD */	0x00AD,	//	SOFT HYPHEN
/* AF */	0x017B,	//	LATIN CAPITAL LETTER Z WITH DOT ABOVE
/* B0 */	0x00B0,	//	DEGREE SIGN
/* B1 */	0x0127,	//	LATIN SMALL LETTER H WITH STROKE
/* B2 */	0x00B2,	//	SUPERSCRIPT TWO
/* B3 */	0x00B3,	//	SUPERSCRIPT THREE
/* B4 */	0x00B4,	//	ACUTE ACCENT
/* B5 */	0x00B5,	//	MICRO SIGN
/* B6 */	0x0125,	//	LATIN SMALL LETTER H WITH CIRCUMFLEX
/* B7 */	0x00B7,	//	MIDDLE DOT
/* B8 */	0x00B8,	//	CEDILLA
/* B9 */	0x0131,	//	LATIN SMALL LETTER DOTLESS I
/* BA */	0x015F,	//	LATIN SMALL LETTER S WITH CEDILLA
/* BB */	0x011F,	//	LATIN SMALL LETTER G WITH BREVE
/* BC */	0x0135,	//	LATIN SMALL LETTER J WITH CIRCUMFLEX
/* BD */	0x00BD,	//	VULGAR FRACTION ONE HALF
/* BF */	0x017C,	//	LATIN SMALL LETTER Z WITH DOT ABOVE
/* C0 */	0x00C0,	//	LATIN CAPITAL LETTER A WITH GRAVE
/* C1 */	0x00C1,	//	LATIN CAPITAL LETTER A WITH ACUTE
/* C2 */	0x00C2,	//	LATIN CAPITAL LETTER A WITH CIRCUMFLEX
/* C4 */	0x00C4,	//	LATIN CAPITAL LETTER A WITH DIAERESIS
/* C5 */	0x010A,	//	LATIN CAPITAL LETTER C WITH DOT ABOVE
/* C6 */	0x0108,	//	LATIN CAPITAL LETTER C WITH CIRCUMFLEX
/* C7 */	0x00C7,	//	LATIN CAPITAL LETTER C WITH CEDILLA
/* C8 */	0x00C8,	//	LATIN CAPITAL LETTER E WITH GRAVE
/* C9 */	0x00C9,	//	LATIN CAPITAL LETTER E WITH ACUTE
/* CA */	0x00CA,	//	LATIN CAPITAL LETTER E WITH CIRCUMFLEX
/* CB */	0x00CB,	//	LATIN CAPITAL LETTER E WITH DIAERESIS
/* CC */	0x00CC,	//	LATIN CAPITAL LETTER I WITH GRAVE
/* CD */	0x00CD,	//	LATIN CAPITAL LETTER I WITH ACUTE
/* CE */	0x00CE,	//	LATIN CAPITAL LETTER I WITH CIRCUMFLEX
/* CF */	0x00CF,	//	LATIN CAPITAL LETTER I WITH DIAERESIS
/* D1 */	0x00D1,	//	LATIN CAPITAL LETTER N WITH TILDE
/* D2 */	0x00D2,	//	LATIN CAPITAL LETTER O WITH GRAVE
/* D3 */	0x00D3,	//	LATIN CAPITAL LETTER O WITH ACUTE
/* D4 */	0x00D4,	//	LATIN CAPITAL LETTER O WITH CIRCUMFLEX
/* D5 */	0x0120,	//	LATIN CAPITAL LETTER G WITH DOT ABOVE
/* D6 */	0x00D6,	//	LATIN CAPITAL LETTER O WITH DIAERESIS
/* D7 */	0x00D7,	//	MULTIPLICATION SIGN
/* D8 */	0x011C,	//	LATIN CAPITAL LETTER G WITH CIRCUMFLEX
/* D9 */	0x00D9,	//	LATIN CAPITAL LETTER U WITH GRAVE
/* DA */	0x00DA,	//	LATIN CAPITAL LETTER U WITH ACUTE
/* DB */	0x00DB,	//	LATIN CAPITAL LETTER U WITH CIRCUMFLEX
/* DC */	0x00DC,	//	LATIN CAPITAL LETTER U WITH DIAERESIS
/* DD */	0x016C,	//	LATIN CAPITAL LETTER U WITH BREVE
/* DE */	0x015C,	//	LATIN CAPITAL LETTER S WITH CIRCUMFLEX
/* DF */	0x00DF,	//	LATIN SMALL LETTER SHARP S
/* E0 */	0x00E0,	//	LATIN SMALL LETTER A WITH GRAVE
/* E1 */	0x00E1,	//	LATIN SMALL LETTER A WITH ACUTE
/* E2 */	0x00E2,	//	LATIN SMALL LETTER A WITH CIRCUMFLEX
/* E4 */	0x00E4,	//	LATIN SMALL LETTER A WITH DIAERESIS
/* E5 */	0x010B,	//	LATIN SMALL LETTER C WITH DOT ABOVE
/* E6 */	0x0109,	//	LATIN SMALL LETTER C WITH CIRCUMFLEX
/* E7 */	0x00E7,	//	LATIN SMALL LETTER C WITH CEDILLA
/* E8 */	0x00E8,	//	LATIN SMALL LETTER E WITH GRAVE
/* E9 */	0x00E9,	//	LATIN SMALL LETTER E WITH ACUTE
/* EA */	0x00EA,	//	LATIN SMALL LETTER E WITH CIRCUMFLEX
/* EB */	0x00EB,	//	LATIN SMALL LETTER E WITH DIAERESIS
/* EC */	0x00EC,	//	LATIN SMALL LETTER I WITH GRAVE
/* ED */	0x00ED,	//	LATIN SMALL LETTER I WITH ACUTE
/* EE */	0x00EE,	//	LATIN SMALL LETTER I WITH CIRCUMFLEX
/* EF */	0x00EF,	//	LATIN SMALL LETTER I WITH DIAERESIS
/* F1 */	0x00F1,	//	LATIN SMALL LETTER N WITH TILDE
/* F2 */	0x00F2,	//	LATIN SMALL LETTER O WITH GRAVE
/* F3 */	0x00F3,	//	LATIN SMALL LETTER O WITH ACUTE
/* F4 */	0x00F4,	//	LATIN SMALL LETTER O WITH CIRCUMFLEX
/* F5 */	0x0121,	//	LATIN SMALL LETTER G WITH DOT ABOVE
/* F6 */	0x00F6,	//	LATIN SMALL LETTER O WITH DIAERESIS
/* F7 */	0x00F7,	//	DIVISION SIGN
/* F8 */	0x011D,	//	LATIN SMALL LETTER G WITH CIRCUMFLEX
/* F9 */	0x00F9,	//	LATIN SMALL LETTER U WITH GRAVE
/* FA */	0x00FA,	//	LATIN SMALL LETTER U WITH ACUTE
/* FB */	0x00FB,	//	LATIN SMALL LETTER U WITH CIRCUMFLEX
/* FC */	0x00FC,	//	LATIN SMALL LETTER U WITH DIAERESIS
/* FD */	0x016D,	//	LATIN SMALL LETTER U WITH BREVE
/* FE */	0x015D,	//	LATIN SMALL LETTER S WITH CIRCUMFLEX
/* FF */	0x02D9	//	DOT ABOVE
};

/***************************************************************************
#
#	Name:             ISO/IEC 8859-4:1998 to Unicode
#	Unicode version:  3.0
#	Table version:    1.0
#	Table format:     Format A
#	Date:             1999 July 27
#	Authors:          Ken Whistler <kenw@sybase.com>
#
#	Copyright (c) 1991-1999 Unicode, Inc.  All Rights reserved.
#
#	This file is provided as-is by Unicode, Inc. (The Unicode Consortium).
#	No claims are made as to fitness for any particular purpose.  No
#	warranties of any kind are expressed or implied.  The recipient
#	agrees to determine applicability of information provided.  If this
#	file has been provided on optical media by Unicode, Inc., the sole
#	remedy for any claim will be exchange of defective media within 90
#	days of receipt.
#
#	Unicode, Inc. hereby grants the right to freely use the information
#	supplied in this file in the creation of products supporting the
#	Unicode Standard, and to make copies of this file in any form for
#	internal or external distribution as long as this notice remains
#	attached.
#
#	General notes:
#
#	This table contains the data the Unicode Consortium has on how
#       ISO/IEC 8859-4:1998 characters map into Unicode.
#
#	Format:  Three tab-separated columns
#		 Column #1 is the ISO/IEC 8859-4 code (in hex as 0xXX)
#		 Column #2 is the Unicode (in hex as 0xXXXX)
#		 Column #3 the Unicode name (follows a comment sign, '#')
#
#	The entries are in ISO/IEC 8859-4 order.
#
#	Version history
#	1.0 version updates 0.1 version by adding mappings for all
#	control characters.
#
#	Updated versions of this file may be found in:
#		<ftp://ftp.unicode.org/Public/MAPPINGS/>
#
#	Any comments or problems, contact <errata@unicode.org>
#	Please note that <errata@unicode.org> is an archival address;
#	notices will be checked, but do not expect an immediate response.
#
***************************************************************************/
static const unicode_char lookup_iso8859_4[256] = {
/* 00 */	0x0000,	//	NULL
/* 01 */	0x0001,	//	START OF HEADING
/* 02 */	0x0002,	//	START OF TEXT
/* 03 */	0x0003,	//	END OF TEXT
/* 04 */	0x0004,	//	END OF TRANSMISSION
/* 05 */	0x0005,	//	ENQUIRY
/* 06 */	0x0006,	//	ACKNOWLEDGE
/* 07 */	0x0007,	//	BELL
/* 08 */	0x0008,	//	BACKSPACE
/* 09 */	0x0009,	//	HORIZONTAL TABULATION
/* 0A */	0x000A,	//	LINE FEED
/* 0B */	0x000B,	//	VERTICAL TABULATION
/* 0C */	0x000C,	//	FORM FEED
/* 0D */	0x000D,	//	CARRIAGE RETURN
/* 0E */	0x000E,	//	SHIFT OUT
/* 0F */	0x000F,	//	SHIFT IN
/* 10 */	0x0010,	//	DATA LINK ESCAPE
/* 11 */	0x0011,	//	DEVICE CONTROL ONE
/* 12 */	0x0012,	//	DEVICE CONTROL TWO
/* 13 */	0x0013,	//	DEVICE CONTROL THREE
/* 14 */	0x0014,	//	DEVICE CONTROL FOUR
/* 15 */	0x0015,	//	NEGATIVE ACKNOWLEDGE
/* 16 */	0x0016,	//	SYNCHRONOUS IDLE
/* 17 */	0x0017,	//	END OF TRANSMISSION BLOCK
/* 18 */	0x0018,	//	CANCEL
/* 19 */	0x0019,	//	END OF MEDIUM
/* 1A */	0x001A,	//	SUBSTITUTE
/* 1B */	0x001B,	//	ESCAPE
/* 1C */	0x001C,	//	FILE SEPARATOR
/* 1D */	0x001D,	//	GROUP SEPARATOR
/* 1E */	0x001E,	//	RECORD SEPARATOR
/* 1F */	0x001F,	//	UNIT SEPARATOR
/* 20 */	0x0020,	//	SPACE
/* 21 */	0x0021,	//	EXCLAMATION MARK
/* 22 */	0x0022,	//	QUOTATION MARK
/* 23 */	0x0023,	//	NUMBER SIGN
/* 24 */	0x0024,	//	DOLLAR SIGN
/* 25 */	0x0025,	//	PERCENT SIGN
/* 26 */	0x0026,	//	AMPERSAND
/* 27 */	0x0027,	//	APOSTROPHE
/* 28 */	0x0028,	//	LEFT PARENTHESIS
/* 29 */	0x0029,	//	RIGHT PARENTHESIS
/* 2A */	0x002A,	//	ASTERISK
/* 2B */	0x002B,	//	PLUS SIGN
/* 2C */	0x002C,	//	COMMA
/* 2D */	0x002D,	//	HYPHEN-MINUS
/* 2E */	0x002E,	//	FULL STOP
/* 2F */	0x002F,	//	SOLIDUS
/* 30 */	0x0030,	//	DIGIT ZERO
/* 31 */	0x0031,	//	DIGIT ONE
/* 32 */	0x0032,	//	DIGIT TWO
/* 33 */	0x0033,	//	DIGIT THREE
/* 34 */	0x0034,	//	DIGIT FOUR
/* 35 */	0x0035,	//	DIGIT FIVE
/* 36 */	0x0036,	//	DIGIT SIX
/* 37 */	0x0037,	//	DIGIT SEVEN
/* 38 */	0x0038,	//	DIGIT EIGHT
/* 39 */	0x0039,	//	DIGIT NINE
/* 3A */	0x003A,	//	COLON
/* 3B */	0x003B,	//	SEMICOLON
/* 3C */	0x003C,	//	LESS-THAN SIGN
/* 3D */	0x003D,	//	EQUALS SIGN
/* 3E */	0x003E,	//	GREATER-THAN SIGN
/* 3F */	0x003F,	//	QUESTION MARK
/* 40 */	0x0040,	//	COMMERCIAL AT
/* 41 */	0x0041,	//	LATIN CAPITAL LETTER A
/* 42 */	0x0042,	//	LATIN CAPITAL LETTER B
/* 43 */	0x0043,	//	LATIN CAPITAL LETTER C
/* 44 */	0x0044,	//	LATIN CAPITAL LETTER D
/* 45 */	0x0045,	//	LATIN CAPITAL LETTER E
/* 46 */	0x0046,	//	LATIN CAPITAL LETTER F
/* 47 */	0x0047,	//	LATIN CAPITAL LETTER G
/* 48 */	0x0048,	//	LATIN CAPITAL LETTER H
/* 49 */	0x0049,	//	LATIN CAPITAL LETTER I
/* 4A */	0x004A,	//	LATIN CAPITAL LETTER J
/* 4B */	0x004B,	//	LATIN CAPITAL LETTER K
/* 4C */	0x004C,	//	LATIN CAPITAL LETTER L
/* 4D */	0x004D,	//	LATIN CAPITAL LETTER M
/* 4E */	0x004E,	//	LATIN CAPITAL LETTER N
/* 4F */	0x004F,	//	LATIN CAPITAL LETTER O
/* 50 */	0x0050,	//	LATIN CAPITAL LETTER P
/* 51 */	0x0051,	//	LATIN CAPITAL LETTER Q
/* 52 */	0x0052,	//	LATIN CAPITAL LETTER R
/* 53 */	0x0053,	//	LATIN CAPITAL LETTER S
/* 54 */	0x0054,	//	LATIN CAPITAL LETTER T
/* 55 */	0x0055,	//	LATIN CAPITAL LETTER U
/* 56 */	0x0056,	//	LATIN CAPITAL LETTER V
/* 57 */	0x0057,	//	LATIN CAPITAL LETTER W
/* 58 */	0x0058,	//	LATIN CAPITAL LETTER X
/* 59 */	0x0059,	//	LATIN CAPITAL LETTER Y
/* 5A */	0x005A,	//	LATIN CAPITAL LETTER Z
/* 5B */	0x005B,	//	LEFT SQUARE BRACKET
/* 5C */	0x005C,	//	REVERSE SOLIDUS
/* 5D */	0x005D,	//	RIGHT SQUARE BRACKET
/* 5E */	0x005E,	//	CIRCUMFLEX ACCENT
/* 5F */	0x005F,	//	LOW LINE
/* 60 */	0x0060,	//	GRAVE ACCENT
/* 61 */	0x0061,	//	LATIN SMALL LETTER A
/* 62 */	0x0062,	//	LATIN SMALL LETTER B
/* 63 */	0x0063,	//	LATIN SMALL LETTER C
/* 64 */	0x0064,	//	LATIN SMALL LETTER D
/* 65 */	0x0065,	//	LATIN SMALL LETTER E
/* 66 */	0x0066,	//	LATIN SMALL LETTER F
/* 67 */	0x0067,	//	LATIN SMALL LETTER G
/* 68 */	0x0068,	//	LATIN SMALL LETTER H
/* 69 */	0x0069,	//	LATIN SMALL LETTER I
/* 6A */	0x006A,	//	LATIN SMALL LETTER J
/* 6B */	0x006B,	//	LATIN SMALL LETTER K
/* 6C */	0x006C,	//	LATIN SMALL LETTER L
/* 6D */	0x006D,	//	LATIN SMALL LETTER M
/* 6E */	0x006E,	//	LATIN SMALL LETTER N
/* 6F */	0x006F,	//	LATIN SMALL LETTER O
/* 70 */	0x0070,	//	LATIN SMALL LETTER P
/* 71 */	0x0071,	//	LATIN SMALL LETTER Q
/* 72 */	0x0072,	//	LATIN SMALL LETTER R
/* 73 */	0x0073,	//	LATIN SMALL LETTER S
/* 74 */	0x0074,	//	LATIN SMALL LETTER T
/* 75 */	0x0075,	//	LATIN SMALL LETTER U
/* 76 */	0x0076,	//	LATIN SMALL LETTER V
/* 77 */	0x0077,	//	LATIN SMALL LETTER W
/* 78 */	0x0078,	//	LATIN SMALL LETTER X
/* 79 */	0x0079,	//	LATIN SMALL LETTER Y
/* 7A */	0x007A,	//	LATIN SMALL LETTER Z
/* 7B */	0x007B,	//	LEFT CURLY BRACKET
/* 7C */	0x007C,	//	VERTICAL LINE
/* 7D */	0x007D,	//	RIGHT CURLY BRACKET
/* 7E */	0x007E,	//	TILDE
/* 7F */	0x007F,	//	DELETE
/* 80 */	0x0080,	//	<control>
/* 81 */	0x0081,	//	<control>
/* 82 */	0x0082,	//	<control>
/* 83 */	0x0083,	//	<control>
/* 84 */	0x0084,	//	<control>
/* 85 */	0x0085,	//	<control>
/* 86 */	0x0086,	//	<control>
/* 87 */	0x0087,	//	<control>
/* 88 */	0x0088,	//	<control>
/* 89 */	0x0089,	//	<control>
/* 8A */	0x008A,	//	<control>
/* 8B */	0x008B,	//	<control>
/* 8C */	0x008C,	//	<control>
/* 8D */	0x008D,	//	<control>
/* 8E */	0x008E,	//	<control>
/* 8F */	0x008F,	//	<control>
/* 90 */	0x0090,	//	<control>
/* 91 */	0x0091,	//	<control>
/* 92 */	0x0092,	//	<control>
/* 93 */	0x0093,	//	<control>
/* 94 */	0x0094,	//	<control>
/* 95 */	0x0095,	//	<control>
/* 96 */	0x0096,	//	<control>
/* 97 */	0x0097,	//	<control>
/* 98 */	0x0098,	//	<control>
/* 99 */	0x0099,	//	<control>
/* 9A */	0x009A,	//	<control>
/* 9B */	0x009B,	//	<control>
/* 9C */	0x009C,	//	<control>
/* 9D */	0x009D,	//	<control>
/* 9E */	0x009E,	//	<control>
/* 9F */	0x009F,	//	<control>
/* A0 */	0x00A0,	//	NO-BREAK SPACE
/* A1 */	0x0104,	//	LATIN CAPITAL LETTER A WITH OGONEK
/* A2 */	0x0138,	//	LATIN SMALL LETTER KRA
/* A3 */	0x0156,	//	LATIN CAPITAL LETTER R WITH CEDILLA
/* A4 */	0x00A4,	//	CURRENCY SIGN
/* A5 */	0x0128,	//	LATIN CAPITAL LETTER I WITH TILDE
/* A6 */	0x013B,	//	LATIN CAPITAL LETTER L WITH CEDILLA
/* A7 */	0x00A7,	//	SECTION SIGN
/* A8 */	0x00A8,	//	DIAERESIS
/* A9 */	0x0160,	//	LATIN CAPITAL LETTER S WITH CARON
/* AA */	0x0112,	//	LATIN CAPITAL LETTER E WITH MACRON
/* AB */	0x0122,	//	LATIN CAPITAL LETTER G WITH CEDILLA
/* AC */	0x0166,	//	LATIN CAPITAL LETTER T WITH STROKE
/* AD */	0x00AD,	//	SOFT HYPHEN
/* AE */	0x017D,	//	LATIN CAPITAL LETTER Z WITH CARON
/* AF */	0x00AF,	//	MACRON
/* B0 */	0x00B0,	//	DEGREE SIGN
/* B1 */	0x0105,	//	LATIN SMALL LETTER A WITH OGONEK
/* B2 */	0x02DB,	//	OGONEK
/* B3 */	0x0157,	//	LATIN SMALL LETTER R WITH CEDILLA
/* B4 */	0x00B4,	//	ACUTE ACCENT
/* B5 */	0x0129,	//	LATIN SMALL LETTER I WITH TILDE
/* B6 */	0x013C,	//	LATIN SMALL LETTER L WITH CEDILLA
/* B7 */	0x02C7,	//	CARON
/* B8 */	0x00B8,	//	CEDILLA
/* B9 */	0x0161,	//	LATIN SMALL LETTER S WITH CARON
/* BA */	0x0113,	//	LATIN SMALL LETTER E WITH MACRON
/* BB */	0x0123,	//	LATIN SMALL LETTER G WITH CEDILLA
/* BC */	0x0167,	//	LATIN SMALL LETTER T WITH STROKE
/* BD */	0x014A,	//	LATIN CAPITAL LETTER ENG
/* BE */	0x017E,	//	LATIN SMALL LETTER Z WITH CARON
/* BF */	0x014B,	//	LATIN SMALL LETTER ENG
/* C0 */	0x0100,	//	LATIN CAPITAL LETTER A WITH MACRON
/* C1 */	0x00C1,	//	LATIN CAPITAL LETTER A WITH ACUTE
/* C2 */	0x00C2,	//	LATIN CAPITAL LETTER A WITH CIRCUMFLEX
/* C3 */	0x00C3,	//	LATIN CAPITAL LETTER A WITH TILDE
/* C4 */	0x00C4,	//	LATIN CAPITAL LETTER A WITH DIAERESIS
/* C5 */	0x00C5,	//	LATIN CAPITAL LETTER A WITH RING ABOVE
/* C6 */	0x00C6,	//	LATIN CAPITAL LETTER AE
/* C7 */	0x012E,	//	LATIN CAPITAL LETTER I WITH OGONEK
/* C8 */	0x010C,	//	LATIN CAPITAL LETTER C WITH CARON
/* C9 */	0x00C9,	//	LATIN CAPITAL LETTER E WITH ACUTE
/* CA */	0x0118,	//	LATIN CAPITAL LETTER E WITH OGONEK
/* CB */	0x00CB,	//	LATIN CAPITAL LETTER E WITH DIAERESIS
/* CC */	0x0116,	//	LATIN CAPITAL LETTER E WITH DOT ABOVE
/* CD */	0x00CD,	//	LATIN CAPITAL LETTER I WITH ACUTE
/* CE */	0x00CE,	//	LATIN CAPITAL LETTER I WITH CIRCUMFLEX
/* CF */	0x012A,	//	LATIN CAPITAL LETTER I WITH MACRON
/* D0 */	0x0110,	//	LATIN CAPITAL LETTER D WITH STROKE
/* D1 */	0x0145,	//	LATIN CAPITAL LETTER N WITH CEDILLA
/* D2 */	0x014C,	//	LATIN CAPITAL LETTER O WITH MACRON
/* D3 */	0x0136,	//	LATIN CAPITAL LETTER K WITH CEDILLA
/* D4 */	0x00D4,	//	LATIN CAPITAL LETTER O WITH CIRCUMFLEX
/* D5 */	0x00D5,	//	LATIN CAPITAL LETTER O WITH TILDE
/* D6 */	0x00D6,	//	LATIN CAPITAL LETTER O WITH DIAERESIS
/* D7 */	0x00D7,	//	MULTIPLICATION SIGN
/* D8 */	0x00D8,	//	LATIN CAPITAL LETTER O WITH STROKE
/* D9 */	0x0172,	//	LATIN CAPITAL LETTER U WITH OGONEK
/* DA */	0x00DA,	//	LATIN CAPITAL LETTER U WITH ACUTE
/* DB */	0x00DB,	//	LATIN CAPITAL LETTER U WITH CIRCUMFLEX
/* DC */	0x00DC,	//	LATIN CAPITAL LETTER U WITH DIAERESIS
/* DD */	0x0168,	//	LATIN CAPITAL LETTER U WITH TILDE
/* DE */	0x016A,	//	LATIN CAPITAL LETTER U WITH MACRON
/* DF */	0x00DF,	//	LATIN SMALL LETTER SHARP S
/* E0 */	0x0101,	//	LATIN SMALL LETTER A WITH MACRON
/* E1 */	0x00E1,	//	LATIN SMALL LETTER A WITH ACUTE
/* E2 */	0x00E2,	//	LATIN SMALL LETTER A WITH CIRCUMFLEX
/* E3 */	0x00E3,	//	LATIN SMALL LETTER A WITH TILDE
/* E4 */	0x00E4,	//	LATIN SMALL LETTER A WITH DIAERESIS
/* E5 */	0x00E5,	//	LATIN SMALL LETTER A WITH RING ABOVE
/* E6 */	0x00E6,	//	LATIN SMALL LETTER AE
/* E7 */	0x012F,	//	LATIN SMALL LETTER I WITH OGONEK
/* E8 */	0x010D,	//	LATIN SMALL LETTER C WITH CARON
/* E9 */	0x00E9,	//	LATIN SMALL LETTER E WITH ACUTE
/* EA */	0x0119,	//	LATIN SMALL LETTER E WITH OGONEK
/* EB */	0x00EB,	//	LATIN SMALL LETTER E WITH DIAERESIS
/* EC */	0x0117,	//	LATIN SMALL LETTER E WITH DOT ABOVE
/* ED */	0x00ED,	//	LATIN SMALL LETTER I WITH ACUTE
/* EE */	0x00EE,	//	LATIN SMALL LETTER I WITH CIRCUMFLEX
/* EF */	0x012B,	//	LATIN SMALL LETTER I WITH MACRON
/* F0 */	0x0111,	//	LATIN SMALL LETTER D WITH STROKE
/* F1 */	0x0146,	//	LATIN SMALL LETTER N WITH CEDILLA
/* F2 */	0x014D,	//	LATIN SMALL LETTER O WITH MACRON
/* F3 */	0x0137,	//	LATIN SMALL LETTER K WITH CEDILLA
/* F4 */	0x00F4,	//	LATIN SMALL LETTER O WITH CIRCUMFLEX
/* F5 */	0x00F5,	//	LATIN SMALL LETTER O WITH TILDE
/* F6 */	0x00F6,	//	LATIN SMALL LETTER O WITH DIAERESIS
/* F7 */	0x00F7,	//	DIVISION SIGN
/* F8 */	0x00F8,	//	LATIN SMALL LETTER O WITH STROKE
/* F9 */	0x0173,	//	LATIN SMALL LETTER U WITH OGONEK
/* FA */	0x00FA,	//	LATIN SMALL LETTER U WITH ACUTE
/* FB */	0x00FB,	//	LATIN SMALL LETTER U WITH CIRCUMFLEX
/* FC */	0x00FC,	//	LATIN SMALL LETTER U WITH DIAERESIS
/* FD */	0x0169,	//	LATIN SMALL LETTER U WITH TILDE
/* FE */	0x016B,	//	LATIN SMALL LETTER U WITH MACRON
/* FF */	0x02D9	//	DOT ABOVE
};

/***************************************************************************
#
#	Name:             ISO 8859-5:1999 to Unicode
#	Unicode version:  3.0
#	Table version:    1.0
#	Table format:     Format A
#	Date:             1999 July 27
#	Authors:          Ken Whistler <kenw@sybase.com>
#
#	Copyright (c) 1991-1999 Unicode, Inc.  All Rights reserved.
#
#	This file is provided as-is by Unicode, Inc. (The Unicode Consortium).
#	No claims are made as to fitness for any particular purpose.  No
#	warranties of any kind are expressed or implied.  The recipient
#	agrees to determine applicability of information provided.  If this
#	file has been provided on optical media by Unicode, Inc., the sole
#	remedy for any claim will be exchange of defective media within 90
#	days of receipt.
#
#	Unicode, Inc. hereby grants the right to freely use the information
#	supplied in this file in the creation of products supporting the
#	Unicode Standard, and to make copies of this file in any form for
#	internal or external distribution as long as this notice remains
#	attached.
#
#	General notes:
#
#	This table contains the data the Unicode Consortium has on how
#       ISO/IEC 8859-5:1999 characters map into Unicode.
#
#	Format:  Three tab-separated columns
#		 Column #1 is the ISO/IEC 8859-5 code (in hex as 0xXX)
#		 Column #2 is the Unicode (in hex as 0xXXXX)
#		 Column #3 the Unicode name (follows a comment sign, '#')
#
#	The entries are in ISO/IEC 8859-5 order.
#
#	Version history
#	1.0 version updates 0.1 version by adding mappings for all
#	control characters.
#
#	Updated versions of this file may be found in:
#		<ftp://ftp.unicode.org/Public/MAPPINGS/>
#
#	Any comments or problems, contact <errata@unicode.org>
#	Please note that <errata@unicode.org> is an archival address;
#	notices will be checked, but do not expect an immediate response.
#
***************************************************************************/
static const unicode_char lookup_iso8859_5[256] = {
/* 00 */	0x0000,	//	NULL
/* 01 */	0x0001,	//	START OF HEADING
/* 02 */	0x0002,	//	START OF TEXT
/* 03 */	0x0003,	//	END OF TEXT
/* 04 */	0x0004,	//	END OF TRANSMISSION
/* 05 */	0x0005,	//	ENQUIRY
/* 06 */	0x0006,	//	ACKNOWLEDGE
/* 07 */	0x0007,	//	BELL
/* 08 */	0x0008,	//	BACKSPACE
/* 09 */	0x0009,	//	HORIZONTAL TABULATION
/* 0A */	0x000A,	//	LINE FEED
/* 0B */	0x000B,	//	VERTICAL TABULATION
/* 0C */	0x000C,	//	FORM FEED
/* 0D */	0x000D,	//	CARRIAGE RETURN
/* 0E */	0x000E,	//	SHIFT OUT
/* 0F */	0x000F,	//	SHIFT IN
/* 10 */	0x0010,	//	DATA LINK ESCAPE
/* 11 */	0x0011,	//	DEVICE CONTROL ONE
/* 12 */	0x0012,	//	DEVICE CONTROL TWO
/* 13 */	0x0013,	//	DEVICE CONTROL THREE
/* 14 */	0x0014,	//	DEVICE CONTROL FOUR
/* 15 */	0x0015,	//	NEGATIVE ACKNOWLEDGE
/* 16 */	0x0016,	//	SYNCHRONOUS IDLE
/* 17 */	0x0017,	//	END OF TRANSMISSION BLOCK
/* 18 */	0x0018,	//	CANCEL
/* 19 */	0x0019,	//	END OF MEDIUM
/* 1A */	0x001A,	//	SUBSTITUTE
/* 1B */	0x001B,	//	ESCAPE
/* 1C */	0x001C,	//	FILE SEPARATOR
/* 1D */	0x001D,	//	GROUP SEPARATOR
/* 1E */	0x001E,	//	RECORD SEPARATOR
/* 1F */	0x001F,	//	UNIT SEPARATOR
/* 20 */	0x0020,	//	SPACE
/* 21 */	0x0021,	//	EXCLAMATION MARK
/* 22 */	0x0022,	//	QUOTATION MARK
/* 23 */	0x0023,	//	NUMBER SIGN
/* 24 */	0x0024,	//	DOLLAR SIGN
/* 25 */	0x0025,	//	PERCENT SIGN
/* 26 */	0x0026,	//	AMPERSAND
/* 27 */	0x0027,	//	APOSTROPHE
/* 28 */	0x0028,	//	LEFT PARENTHESIS
/* 29 */	0x0029,	//	RIGHT PARENTHESIS
/* 2A */	0x002A,	//	ASTERISK
/* 2B */	0x002B,	//	PLUS SIGN
/* 2C */	0x002C,	//	COMMA
/* 2D */	0x002D,	//	HYPHEN-MINUS
/* 2E */	0x002E,	//	FULL STOP
/* 2F */	0x002F,	//	SOLIDUS
/* 30 */	0x0030,	//	DIGIT ZERO
/* 31 */	0x0031,	//	DIGIT ONE
/* 32 */	0x0032,	//	DIGIT TWO
/* 33 */	0x0033,	//	DIGIT THREE
/* 34 */	0x0034,	//	DIGIT FOUR
/* 35 */	0x0035,	//	DIGIT FIVE
/* 36 */	0x0036,	//	DIGIT SIX
/* 37 */	0x0037,	//	DIGIT SEVEN
/* 38 */	0x0038,	//	DIGIT EIGHT
/* 39 */	0x0039,	//	DIGIT NINE
/* 3A */	0x003A,	//	COLON
/* 3B */	0x003B,	//	SEMICOLON
/* 3C */	0x003C,	//	LESS-THAN SIGN
/* 3D */	0x003D,	//	EQUALS SIGN
/* 3E */	0x003E,	//	GREATER-THAN SIGN
/* 3F */	0x003F,	//	QUESTION MARK
/* 40 */	0x0040,	//	COMMERCIAL AT
/* 41 */	0x0041,	//	LATIN CAPITAL LETTER A
/* 42 */	0x0042,	//	LATIN CAPITAL LETTER B
/* 43 */	0x0043,	//	LATIN CAPITAL LETTER C
/* 44 */	0x0044,	//	LATIN CAPITAL LETTER D
/* 45 */	0x0045,	//	LATIN CAPITAL LETTER E
/* 46 */	0x0046,	//	LATIN CAPITAL LETTER F
/* 47 */	0x0047,	//	LATIN CAPITAL LETTER G
/* 48 */	0x0048,	//	LATIN CAPITAL LETTER H
/* 49 */	0x0049,	//	LATIN CAPITAL LETTER I
/* 4A */	0x004A,	//	LATIN CAPITAL LETTER J
/* 4B */	0x004B,	//	LATIN CAPITAL LETTER K
/* 4C */	0x004C,	//	LATIN CAPITAL LETTER L
/* 4D */	0x004D,	//	LATIN CAPITAL LETTER M
/* 4E */	0x004E,	//	LATIN CAPITAL LETTER N
/* 4F */	0x004F,	//	LATIN CAPITAL LETTER O
/* 50 */	0x0050,	//	LATIN CAPITAL LETTER P
/* 51 */	0x0051,	//	LATIN CAPITAL LETTER Q
/* 52 */	0x0052,	//	LATIN CAPITAL LETTER R
/* 53 */	0x0053,	//	LATIN CAPITAL LETTER S
/* 54 */	0x0054,	//	LATIN CAPITAL LETTER T
/* 55 */	0x0055,	//	LATIN CAPITAL LETTER U
/* 56 */	0x0056,	//	LATIN CAPITAL LETTER V
/* 57 */	0x0057,	//	LATIN CAPITAL LETTER W
/* 58 */	0x0058,	//	LATIN CAPITAL LETTER X
/* 59 */	0x0059,	//	LATIN CAPITAL LETTER Y
/* 5A */	0x005A,	//	LATIN CAPITAL LETTER Z
/* 5B */	0x005B,	//	LEFT SQUARE BRACKET
/* 5C */	0x005C,	//	REVERSE SOLIDUS
/* 5D */	0x005D,	//	RIGHT SQUARE BRACKET
/* 5E */	0x005E,	//	CIRCUMFLEX ACCENT
/* 5F */	0x005F,	//	LOW LINE
/* 60 */	0x0060,	//	GRAVE ACCENT
/* 61 */	0x0061,	//	LATIN SMALL LETTER A
/* 62 */	0x0062,	//	LATIN SMALL LETTER B
/* 63 */	0x0063,	//	LATIN SMALL LETTER C
/* 64 */	0x0064,	//	LATIN SMALL LETTER D
/* 65 */	0x0065,	//	LATIN SMALL LETTER E
/* 66 */	0x0066,	//	LATIN SMALL LETTER F
/* 67 */	0x0067,	//	LATIN SMALL LETTER G
/* 68 */	0x0068,	//	LATIN SMALL LETTER H
/* 69 */	0x0069,	//	LATIN SMALL LETTER I
/* 6A */	0x006A,	//	LATIN SMALL LETTER J
/* 6B */	0x006B,	//	LATIN SMALL LETTER K
/* 6C */	0x006C,	//	LATIN SMALL LETTER L
/* 6D */	0x006D,	//	LATIN SMALL LETTER M
/* 6E */	0x006E,	//	LATIN SMALL LETTER N
/* 6F */	0x006F,	//	LATIN SMALL LETTER O
/* 70 */	0x0070,	//	LATIN SMALL LETTER P
/* 71 */	0x0071,	//	LATIN SMALL LETTER Q
/* 72 */	0x0072,	//	LATIN SMALL LETTER R
/* 73 */	0x0073,	//	LATIN SMALL LETTER S
/* 74 */	0x0074,	//	LATIN SMALL LETTER T
/* 75 */	0x0075,	//	LATIN SMALL LETTER U
/* 76 */	0x0076,	//	LATIN SMALL LETTER V
/* 77 */	0x0077,	//	LATIN SMALL LETTER W
/* 78 */	0x0078,	//	LATIN SMALL LETTER X
/* 79 */	0x0079,	//	LATIN SMALL LETTER Y
/* 7A */	0x007A,	//	LATIN SMALL LETTER Z
/* 7B */	0x007B,	//	LEFT CURLY BRACKET
/* 7C */	0x007C,	//	VERTICAL LINE
/* 7D */	0x007D,	//	RIGHT CURLY BRACKET
/* 7E */	0x007E,	//	TILDE
/* 7F */	0x007F,	//	DELETE
/* 80 */	0x0080,	//	<control>
/* 81 */	0x0081,	//	<control>
/* 82 */	0x0082,	//	<control>
/* 83 */	0x0083,	//	<control>
/* 84 */	0x0084,	//	<control>
/* 85 */	0x0085,	//	<control>
/* 86 */	0x0086,	//	<control>
/* 87 */	0x0087,	//	<control>
/* 88 */	0x0088,	//	<control>
/* 89 */	0x0089,	//	<control>
/* 8A */	0x008A,	//	<control>
/* 8B */	0x008B,	//	<control>
/* 8C */	0x008C,	//	<control>
/* 8D */	0x008D,	//	<control>
/* 8E */	0x008E,	//	<control>
/* 8F */	0x008F,	//	<control>
/* 90 */	0x0090,	//	<control>
/* 91 */	0x0091,	//	<control>
/* 92 */	0x0092,	//	<control>
/* 93 */	0x0093,	//	<control>
/* 94 */	0x0094,	//	<control>
/* 95 */	0x0095,	//	<control>
/* 96 */	0x0096,	//	<control>
/* 97 */	0x0097,	//	<control>
/* 98 */	0x0098,	//	<control>
/* 99 */	0x0099,	//	<control>
/* 9A */	0x009A,	//	<control>
/* 9B */	0x009B,	//	<control>
/* 9C */	0x009C,	//	<control>
/* 9D */	0x009D,	//	<control>
/* 9E */	0x009E,	//	<control>
/* 9F */	0x009F,	//	<control>
/* A0 */	0x00A0,	//	NO-BREAK SPACE
/* A1 */	0x0401,	//	CYRILLIC CAPITAL LETTER IO
/* A2 */	0x0402,	//	CYRILLIC CAPITAL LETTER DJE
/* A3 */	0x0403,	//	CYRILLIC CAPITAL LETTER GJE
/* A4 */	0x0404,	//	CYRILLIC CAPITAL LETTER UKRAINIAN IE
/* A5 */	0x0405,	//	CYRILLIC CAPITAL LETTER DZE
/* A6 */	0x0406,	//	CYRILLIC CAPITAL LETTER BYELORUSSIAN-UKRAINIAN I
/* A7 */	0x0407,	//	CYRILLIC CAPITAL LETTER YI
/* A8 */	0x0408,	//	CYRILLIC CAPITAL LETTER JE
/* A9 */	0x0409,	//	CYRILLIC CAPITAL LETTER LJE
/* AA */	0x040A,	//	CYRILLIC CAPITAL LETTER NJE
/* AB */	0x040B,	//	CYRILLIC CAPITAL LETTER TSHE
/* AC */	0x040C,	//	CYRILLIC CAPITAL LETTER KJE
/* AD */	0x00AD,	//	SOFT HYPHEN
/* AE */	0x040E,	//	CYRILLIC CAPITAL LETTER SHORT U
/* AF */	0x040F,	//	CYRILLIC CAPITAL LETTER DZHE
/* B0 */	0x0410,	//	CYRILLIC CAPITAL LETTER A
/* B1 */	0x0411,	//	CYRILLIC CAPITAL LETTER BE
/* B2 */	0x0412,	//	CYRILLIC CAPITAL LETTER VE
/* B3 */	0x0413,	//	CYRILLIC CAPITAL LETTER GHE
/* B4 */	0x0414,	//	CYRILLIC CAPITAL LETTER DE
/* B5 */	0x0415,	//	CYRILLIC CAPITAL LETTER IE
/* B6 */	0x0416,	//	CYRILLIC CAPITAL LETTER ZHE
/* B7 */	0x0417,	//	CYRILLIC CAPITAL LETTER ZE
/* B8 */	0x0418,	//	CYRILLIC CAPITAL LETTER I
/* B9 */	0x0419,	//	CYRILLIC CAPITAL LETTER SHORT I
/* BA */	0x041A,	//	CYRILLIC CAPITAL LETTER KA
/* BB */	0x041B,	//	CYRILLIC CAPITAL LETTER EL
/* BC */	0x041C,	//	CYRILLIC CAPITAL LETTER EM
/* BD */	0x041D,	//	CYRILLIC CAPITAL LETTER EN
/* BE */	0x041E,	//	CYRILLIC CAPITAL LETTER O
/* BF */	0x041F,	//	CYRILLIC CAPITAL LETTER PE
/* C0 */	0x0420,	//	CYRILLIC CAPITAL LETTER ER
/* C1 */	0x0421,	//	CYRILLIC CAPITAL LETTER ES
/* C2 */	0x0422,	//	CYRILLIC CAPITAL LETTER TE
/* C3 */	0x0423,	//	CYRILLIC CAPITAL LETTER U
/* C4 */	0x0424,	//	CYRILLIC CAPITAL LETTER EF
/* C5 */	0x0425,	//	CYRILLIC CAPITAL LETTER HA
/* C6 */	0x0426,	//	CYRILLIC CAPITAL LETTER TSE
/* C7 */	0x0427,	//	CYRILLIC CAPITAL LETTER CHE
/* C8 */	0x0428,	//	CYRILLIC CAPITAL LETTER SHA
/* C9 */	0x0429,	//	CYRILLIC CAPITAL LETTER SHCHA
/* CA */	0x042A,	//	CYRILLIC CAPITAL LETTER HARD SIGN
/* CB */	0x042B,	//	CYRILLIC CAPITAL LETTER YERU
/* CC */	0x042C,	//	CYRILLIC CAPITAL LETTER SOFT SIGN
/* CD */	0x042D,	//	CYRILLIC CAPITAL LETTER E
/* CE */	0x042E,	//	CYRILLIC CAPITAL LETTER YU
/* CF */	0x042F,	//	CYRILLIC CAPITAL LETTER YA
/* D0 */	0x0430,	//	CYRILLIC SMALL LETTER A
/* D1 */	0x0431,	//	CYRILLIC SMALL LETTER BE
/* D2 */	0x0432,	//	CYRILLIC SMALL LETTER VE
/* D3 */	0x0433,	//	CYRILLIC SMALL LETTER GHE
/* D4 */	0x0434,	//	CYRILLIC SMALL LETTER DE
/* D5 */	0x0435,	//	CYRILLIC SMALL LETTER IE
/* D6 */	0x0436,	//	CYRILLIC SMALL LETTER ZHE
/* D7 */	0x0437,	//	CYRILLIC SMALL LETTER ZE
/* D8 */	0x0438,	//	CYRILLIC SMALL LETTER I
/* D9 */	0x0439,	//	CYRILLIC SMALL LETTER SHORT I
/* DA */	0x043A,	//	CYRILLIC SMALL LETTER KA
/* DB */	0x043B,	//	CYRILLIC SMALL LETTER EL
/* DC */	0x043C,	//	CYRILLIC SMALL LETTER EM
/* DD */	0x043D,	//	CYRILLIC SMALL LETTER EN
/* DE */	0x043E,	//	CYRILLIC SMALL LETTER O
/* DF */	0x043F,	//	CYRILLIC SMALL LETTER PE
/* E0 */	0x0440,	//	CYRILLIC SMALL LETTER ER
/* E1 */	0x0441,	//	CYRILLIC SMALL LETTER ES
/* E2 */	0x0442,	//	CYRILLIC SMALL LETTER TE
/* E3 */	0x0443,	//	CYRILLIC SMALL LETTER U
/* E4 */	0x0444,	//	CYRILLIC SMALL LETTER EF
/* E5 */	0x0445,	//	CYRILLIC SMALL LETTER HA
/* E6 */	0x0446,	//	CYRILLIC SMALL LETTER TSE
/* E7 */	0x0447,	//	CYRILLIC SMALL LETTER CHE
/* E8 */	0x0448,	//	CYRILLIC SMALL LETTER SHA
/* E9 */	0x0449,	//	CYRILLIC SMALL LETTER SHCHA
/* EA */	0x044A,	//	CYRILLIC SMALL LETTER HARD SIGN
/* EB */	0x044B,	//	CYRILLIC SMALL LETTER YERU
/* EC */	0x044C,	//	CYRILLIC SMALL LETTER SOFT SIGN
/* ED */	0x044D,	//	CYRILLIC SMALL LETTER E
/* EE */	0x044E,	//	CYRILLIC SMALL LETTER YU
/* EF */	0x044F,	//	CYRILLIC SMALL LETTER YA
/* F0 */	0x2116,	//	NUMERO SIGN
/* F1 */	0x0451,	//	CYRILLIC SMALL LETTER IO
/* F2 */	0x0452,	//	CYRILLIC SMALL LETTER DJE
/* F3 */	0x0453,	//	CYRILLIC SMALL LETTER GJE
/* F4 */	0x0454,	//	CYRILLIC SMALL LETTER UKRAINIAN IE
/* F5 */	0x0455,	//	CYRILLIC SMALL LETTER DZE
/* F6 */	0x0456,	//	CYRILLIC SMALL LETTER BYELORUSSIAN-UKRAINIAN I
/* F7 */	0x0457,	//	CYRILLIC SMALL LETTER YI
/* F8 */	0x0458,	//	CYRILLIC SMALL LETTER JE
/* F9 */	0x0459,	//	CYRILLIC SMALL LETTER LJE
/* FA */	0x045A,	//	CYRILLIC SMALL LETTER NJE
/* FB */	0x045B,	//	CYRILLIC SMALL LETTER TSHE
/* FC */	0x045C,	//	CYRILLIC SMALL LETTER KJE
/* FD */	0x00A7,	//	SECTION SIGN
/* FE */	0x045E,	//	CYRILLIC SMALL LETTER SHORT U
/* FF */	0x045F	//	CYRILLIC SMALL LETTER DZHE
};

/***************************************************************************
#
#	Name:             ISO 8859-6:1999 to Unicode
#	Unicode version:  3.0
#	Table version:    1.0
#	Table format:     Format A
#	Date:             1999 July 27
#	Authors:          Ken Whistler <kenw@sybase.com>
#
#	Copyright (c) 1991-1999 Unicode, Inc.  All Rights reserved.
#
#	This file is provided as-is by Unicode, Inc. (The Unicode Consortium).
#	No claims are made as to fitness for any particular purpose.  No
#	warranties of any kind are expressed or implied.  The recipient
#	agrees to determine applicability of information provided.  If this
#	file has been provided on optical media by Unicode, Inc., the sole
#	remedy for any claim will be exchange of defective media within 90
#	days of receipt.
#
#	Unicode, Inc. hereby grants the right to freely use the information
#	supplied in this file in the creation of products supporting the
#	Unicode Standard, and to make copies of this file in any form for
#	internal or external distribution as long as this notice remains
#	attached.
#
#	General notes:
#
#	This table contains the data the Unicode Consortium has on how
#       ISO/IEC 8859-6:1999 characters map into Unicode.
#
#	Format:  Three tab-separated columns
#		 Column #1 is the ISO/IEC 8859-6 code (in hex as 0xXX)
#		 Column #2 is the Unicode (in hex as 0xXXXX)
#		 Column #3 the Unicode name (follows a comment sign, '#')
#
#	The entries are in ISO/IEC 8859-6 order.
#
#	Version history
#	1.0 version updates 0.1 version by adding mappings for all
#	control characters.
#	0x30..0x39 remapped to the ASCII digits (U+0030..U+0039) instead
#	of the Arabic digits (U+0660..U+0669).
#
#	Updated versions of this file may be found in:
#		<ftp://ftp.unicode.org/Public/MAPPINGS/>
#
#	Any comments or problems, contact <errata@unicode.org>
#	Please note that <errata@unicode.org> is an archival address;
#	notices will be checked, but do not expect an immediate response.
#
***************************************************************************/
static const unicode_char lookup_iso8859_6[256] = {
/* 00 */	0x0000,	//	NULL
/* 01 */	0x0001,	//	START OF HEADING
/* 02 */	0x0002,	//	START OF TEXT
/* 03 */	0x0003,	//	END OF TEXT
/* 04 */	0x0004,	//	END OF TRANSMISSION
/* 05 */	0x0005,	//	ENQUIRY
/* 06 */	0x0006,	//	ACKNOWLEDGE
/* 07 */	0x0007,	//	BELL
/* 08 */	0x0008,	//	BACKSPACE
/* 09 */	0x0009,	//	HORIZONTAL TABULATION
/* 0A */	0x000A,	//	LINE FEED
/* 0B */	0x000B,	//	VERTICAL TABULATION
/* 0C */	0x000C,	//	FORM FEED
/* 0D */	0x000D,	//	CARRIAGE RETURN
/* 0E */	0x000E,	//	SHIFT OUT
/* 0F */	0x000F,	//	SHIFT IN
/* 10 */	0x0010,	//	DATA LINK ESCAPE
/* 11 */	0x0011,	//	DEVICE CONTROL ONE
/* 12 */	0x0012,	//	DEVICE CONTROL TWO
/* 13 */	0x0013,	//	DEVICE CONTROL THREE
/* 14 */	0x0014,	//	DEVICE CONTROL FOUR
/* 15 */	0x0015,	//	NEGATIVE ACKNOWLEDGE
/* 16 */	0x0016,	//	SYNCHRONOUS IDLE
/* 17 */	0x0017,	//	END OF TRANSMISSION BLOCK
/* 18 */	0x0018,	//	CANCEL
/* 19 */	0x0019,	//	END OF MEDIUM
/* 1A */	0x001A,	//	SUBSTITUTE
/* 1B */	0x001B,	//	ESCAPE
/* 1C */	0x001C,	//	FILE SEPARATOR
/* 1D */	0x001D,	//	GROUP SEPARATOR
/* 1E */	0x001E,	//	RECORD SEPARATOR
/* 1F */	0x001F,	//	UNIT SEPARATOR
/* 20 */	0x0020,	//	SPACE
/* 21 */	0x0021,	//	EXCLAMATION MARK
/* 22 */	0x0022,	//	QUOTATION MARK
/* 23 */	0x0023,	//	NUMBER SIGN
/* 24 */	0x0024,	//	DOLLAR SIGN
/* 25 */	0x0025,	//	PERCENT SIGN
/* 26 */	0x0026,	//	AMPERSAND
/* 27 */	0x0027,	//	APOSTROPHE
/* 28 */	0x0028,	//	LEFT PARENTHESIS
/* 29 */	0x0029,	//	RIGHT PARENTHESIS
/* 2A */	0x002A,	//	ASTERISK
/* 2B */	0x002B,	//	PLUS SIGN
/* 2C */	0x002C,	//	COMMA
/* 2D */	0x002D,	//	HYPHEN-MINUS
/* 2E */	0x002E,	//	FULL STOP
/* 2F */	0x002F,	//	SOLIDUS
/* 30 */	0x0030,	//	DIGIT ZERO
/* 31 */	0x0031,	//	DIGIT ONE
/* 32 */	0x0032,	//	DIGIT TWO
/* 33 */	0x0033,	//	DIGIT THREE
/* 34 */	0x0034,	//	DIGIT FOUR
/* 35 */	0x0035,	//	DIGIT FIVE
/* 36 */	0x0036,	//	DIGIT SIX
/* 37 */	0x0037,	//	DIGIT SEVEN
/* 38 */	0x0038,	//	DIGIT EIGHT
/* 39 */	0x0039,	//	DIGIT NINE
/* 3A */	0x003A,	//	COLON
/* 3B */	0x003B,	//	SEMICOLON
/* 3C */	0x003C,	//	LESS-THAN SIGN
/* 3D */	0x003D,	//	EQUALS SIGN
/* 3E */	0x003E,	//	GREATER-THAN SIGN
/* 3F */	0x003F,	//	QUESTION MARK
/* 40 */	0x0040,	//	COMMERCIAL AT
/* 41 */	0x0041,	//	LATIN CAPITAL LETTER A
/* 42 */	0x0042,	//	LATIN CAPITAL LETTER B
/* 43 */	0x0043,	//	LATIN CAPITAL LETTER C
/* 44 */	0x0044,	//	LATIN CAPITAL LETTER D
/* 45 */	0x0045,	//	LATIN CAPITAL LETTER E
/* 46 */	0x0046,	//	LATIN CAPITAL LETTER F
/* 47 */	0x0047,	//	LATIN CAPITAL LETTER G
/* 48 */	0x0048,	//	LATIN CAPITAL LETTER H
/* 49 */	0x0049,	//	LATIN CAPITAL LETTER I
/* 4A */	0x004A,	//	LATIN CAPITAL LETTER J
/* 4B */	0x004B,	//	LATIN CAPITAL LETTER K
/* 4C */	0x004C,	//	LATIN CAPITAL LETTER L
/* 4D */	0x004D,	//	LATIN CAPITAL LETTER M
/* 4E */	0x004E,	//	LATIN CAPITAL LETTER N
/* 4F */	0x004F,	//	LATIN CAPITAL LETTER O
/* 50 */	0x0050,	//	LATIN CAPITAL LETTER P
/* 51 */	0x0051,	//	LATIN CAPITAL LETTER Q
/* 52 */	0x0052,	//	LATIN CAPITAL LETTER R
/* 53 */	0x0053,	//	LATIN CAPITAL LETTER S
/* 54 */	0x0054,	//	LATIN CAPITAL LETTER T
/* 55 */	0x0055,	//	LATIN CAPITAL LETTER U
/* 56 */	0x0056,	//	LATIN CAPITAL LETTER V
/* 57 */	0x0057,	//	LATIN CAPITAL LETTER W
/* 58 */	0x0058,	//	LATIN CAPITAL LETTER X
/* 59 */	0x0059,	//	LATIN CAPITAL LETTER Y
/* 5A */	0x005A,	//	LATIN CAPITAL LETTER Z
/* 5B */	0x005B,	//	LEFT SQUARE BRACKET
/* 5C */	0x005C,	//	REVERSE SOLIDUS
/* 5D */	0x005D,	//	RIGHT SQUARE BRACKET
/* 5E */	0x005E,	//	CIRCUMFLEX ACCENT
/* 5F */	0x005F,	//	LOW LINE
/* 60 */	0x0060,	//	GRAVE ACCENT
/* 61 */	0x0061,	//	LATIN SMALL LETTER A
/* 62 */	0x0062,	//	LATIN SMALL LETTER B
/* 63 */	0x0063,	//	LATIN SMALL LETTER C
/* 64 */	0x0064,	//	LATIN SMALL LETTER D
/* 65 */	0x0065,	//	LATIN SMALL LETTER E
/* 66 */	0x0066,	//	LATIN SMALL LETTER F
/* 67 */	0x0067,	//	LATIN SMALL LETTER G
/* 68 */	0x0068,	//	LATIN SMALL LETTER H
/* 69 */	0x0069,	//	LATIN SMALL LETTER I
/* 6A */	0x006A,	//	LATIN SMALL LETTER J
/* 6B */	0x006B,	//	LATIN SMALL LETTER K
/* 6C */	0x006C,	//	LATIN SMALL LETTER L
/* 6D */	0x006D,	//	LATIN SMALL LETTER M
/* 6E */	0x006E,	//	LATIN SMALL LETTER N
/* 6F */	0x006F,	//	LATIN SMALL LETTER O
/* 70 */	0x0070,	//	LATIN SMALL LETTER P
/* 71 */	0x0071,	//	LATIN SMALL LETTER Q
/* 72 */	0x0072,	//	LATIN SMALL LETTER R
/* 73 */	0x0073,	//	LATIN SMALL LETTER S
/* 74 */	0x0074,	//	LATIN SMALL LETTER T
/* 75 */	0x0075,	//	LATIN SMALL LETTER U
/* 76 */	0x0076,	//	LATIN SMALL LETTER V
/* 77 */	0x0077,	//	LATIN SMALL LETTER W
/* 78 */	0x0078,	//	LATIN SMALL LETTER X
/* 79 */	0x0079,	//	LATIN SMALL LETTER Y
/* 7A */	0x007A,	//	LATIN SMALL LETTER Z
/* 7B */	0x007B,	//	LEFT CURLY BRACKET
/* 7C */	0x007C,	//	VERTICAL LINE
/* 7D */	0x007D,	//	RIGHT CURLY BRACKET
/* 7E */	0x007E,	//	TILDE
/* 7F */	0x007F,	//	DELETE
/* 80 */	0x0080,	//	<control>
/* 81 */	0x0081,	//	<control>
/* 82 */	0x0082,	//	<control>
/* 83 */	0x0083,	//	<control>
/* 84 */	0x0084,	//	<control>
/* 85 */	0x0085,	//	<control>
/* 86 */	0x0086,	//	<control>
/* 87 */	0x0087,	//	<control>
/* 88 */	0x0088,	//	<control>
/* 89 */	0x0089,	//	<control>
/* 8A */	0x008A,	//	<control>
/* 8B */	0x008B,	//	<control>
/* 8C */	0x008C,	//	<control>
/* 8D */	0x008D,	//	<control>
/* 8E */	0x008E,	//	<control>
/* 8F */	0x008F,	//	<control>
/* 90 */	0x0090,	//	<control>
/* 91 */	0x0091,	//	<control>
/* 92 */	0x0092,	//	<control>
/* 93 */	0x0093,	//	<control>
/* 94 */	0x0094,	//	<control>
/* 95 */	0x0095,	//	<control>
/* 96 */	0x0096,	//	<control>
/* 97 */	0x0097,	//	<control>
/* 98 */	0x0098,	//	<control>
/* 99 */	0x0099,	//	<control>
/* 9A */	0x009A,	//	<control>
/* 9B */	0x009B,	//	<control>
/* 9C */	0x009C,	//	<control>
/* 9D */	0x009D,	//	<control>
/* 9E */	0x009E,	//	<control>
/* 9F */	0x009F,	//	<control>
/* A0 */	0x00A0,	//	NO-BREAK SPACE
/* A4 */	0x00A4,	//	CURRENCY SIGN
/* AC */	0x060C,	//	ARABIC COMMA
/* AD */	0x00AD,	//	SOFT HYPHEN
/* BB */	0x061B,	//	ARABIC SEMICOLON
/* BF */	0x061F,	//	ARABIC QUESTION MARK
/* C1 */	0x0621,	//	ARABIC LETTER HAMZA
/* C2 */	0x0622,	//	ARABIC LETTER ALEF WITH MADDA ABOVE
/* C3 */	0x0623,	//	ARABIC LETTER ALEF WITH HAMZA ABOVE
/* C4 */	0x0624,	//	ARABIC LETTER WAW WITH HAMZA ABOVE
/* C5 */	0x0625,	//	ARABIC LETTER ALEF WITH HAMZA BELOW
/* C6 */	0x0626,	//	ARABIC LETTER YEH WITH HAMZA ABOVE
/* C7 */	0x0627,	//	ARABIC LETTER ALEF
/* C8 */	0x0628,	//	ARABIC LETTER BEH
/* C9 */	0x0629,	//	ARABIC LETTER TEH MARBUTA
/* CA */	0x062A,	//	ARABIC LETTER TEH
/* CB */	0x062B,	//	ARABIC LETTER THEH
/* CC */	0x062C,	//	ARABIC LETTER JEEM
/* CD */	0x062D,	//	ARABIC LETTER HAH
/* CE */	0x062E,	//	ARABIC LETTER KHAH
/* CF */	0x062F,	//	ARABIC LETTER DAL
/* D0 */	0x0630,	//	ARABIC LETTER THAL
/* D1 */	0x0631,	//	ARABIC LETTER REH
/* D2 */	0x0632,	//	ARABIC LETTER ZAIN
/* D3 */	0x0633,	//	ARABIC LETTER SEEN
/* D4 */	0x0634,	//	ARABIC LETTER SHEEN
/* D5 */	0x0635,	//	ARABIC LETTER SAD
/* D6 */	0x0636,	//	ARABIC LETTER DAD
/* D7 */	0x0637,	//	ARABIC LETTER TAH
/* D8 */	0x0638,	//	ARABIC LETTER ZAH
/* D9 */	0x0639,	//	ARABIC LETTER AIN
/* DA */	0x063A,	//	ARABIC LETTER GHAIN
/* E0 */	0x0640,	//	ARABIC TATWEEL
/* E1 */	0x0641,	//	ARABIC LETTER FEH
/* E2 */	0x0642,	//	ARABIC LETTER QAF
/* E3 */	0x0643,	//	ARABIC LETTER KAF
/* E4 */	0x0644,	//	ARABIC LETTER LAM
/* E5 */	0x0645,	//	ARABIC LETTER MEEM
/* E6 */	0x0646,	//	ARABIC LETTER NOON
/* E7 */	0x0647,	//	ARABIC LETTER HEH
/* E8 */	0x0648,	//	ARABIC LETTER WAW
/* E9 */	0x0649,	//	ARABIC LETTER ALEF MAKSURA
/* EA */	0x064A,	//	ARABIC LETTER YEH
/* EB */	0x064B,	//	ARABIC FATHATAN
/* EC */	0x064C,	//	ARABIC DAMMATAN
/* ED */	0x064D,	//	ARABIC KASRATAN
/* EE */	0x064E,	//	ARABIC FATHA
/* EF */	0x064F,	//	ARABIC DAMMA
/* F0 */	0x0650,	//	ARABIC KASRA
/* F1 */	0x0651,	//	ARABIC SHADDA
/* F2 */	0x0652,	//	ARABIC SUKUN
/* F3 */	0xFFFD,	//	INVALID
/* F4 */	0xFFFD,	//	INVALID
/* F5 */	0xFFFD,	//	INVALID
/* F6 */	0xFFFD,	//	INVALID
/* F7 */	0xFFFD,	//	INVALID
/* F8 */	0xFFFD,	//	INVALID
/* F9 */	0xFFFD,	//	INVALID
/* FA */	0xFFFD,	//	INVALID
/* FB */	0xFFFD,	//	INVALID
/* FC */	0xFFFD,	//	INVALID
/* FD */	0xFFFD,	//	INVALID
/* FE */	0xFFFD,	//	INVALID
/* FF */	0xFFFD	//	INVALID
};

/***************************************************************************
#
#	Name:             ISO 8859-7:2003 to Unicode
#	Unicode version:  4.0
#	Table version:    2.0
#	Table format:     Format A
#	Date:             2003-Nov-12
#	Authors:          Ken Whistler <kenw@sybase.com>
#
#	Copyright (c) 1991-2003 Unicode, Inc.  All Rights reserved.
#
#	This file is provided as-is by Unicode, Inc. (The Unicode Consortium).
#	No claims are made as to fitness for any particular purpose.  No
#	warranties of any kind are expressed or implied.  The recipient
#	agrees to determine applicability of information provided.  If this
#	file has been provided on optical media by Unicode, Inc., the sole
#	remedy for any claim will be exchange of defective media within 90
#	days of receipt.
#
#	Unicode, Inc. hereby grants the right to freely use the information
#	supplied in this file in the creation of products supporting the
#	Unicode Standard, and to make copies of this file in any form for
#	internal or external distribution as long as this notice remains
#	attached.
#
#	General notes:
#
#	This table contains the data the Unicode Consortium has on how
#       ISO 8859-7:2003 characters map into Unicode.
#
#	ISO 8859-7:1987 is equivalent to ISO-IR-126, ELOT 928,
#	and ECMA 118. ISO 8859-7:2003 adds two currency signs
#	and one other character not in the earlier standard.
#
#	Format:  Three tab-separated columns
#		 Column #1 is the ISO 8859-7 code (in hex as 0xXX)
#		 Column #2 is the Unicode (in hex as 0xXXXX)
#		 Column #3 the Unicode name (follows a comment sign, '#')
#
#	The entries are in ISO 8859-7 order.
#
#	Version history
#	1.0 version updates 0.1 version by adding mappings for all
#	control characters.
#	Remap 0xA1 to U+2018 (instead of 0x02BD) to match text of 8859-7
#	Remap 0xA2 to U+2019 (instead of 0x02BC) to match text of 8859-7
#
#	2.0 version updates 1.0 version by adding mappings for the
#	three newly added characters 0xA4, 0xA5, 0xAA.
#
#	Updated versions of this file may be found in:
#		<http://www.unicode.org/Public/MAPPINGS/>
#
#	Any comments or problems, contact the Unicode Consortium at:
#	        <http://www.unicode.org/reporting.html>
#
***************************************************************************/
static const unicode_char lookup_iso8859_7[256] = {
/* 00 */	0x0000,	//	NULL
/* 01 */	0x0001,	//	START OF HEADING
/* 02 */	0x0002,	//	START OF TEXT
/* 03 */	0x0003,	//	END OF TEXT
/* 04 */	0x0004,	//	END OF TRANSMISSION
/* 05 */	0x0005,	//	ENQUIRY
/* 06 */	0x0006,	//	ACKNOWLEDGE
/* 07 */	0x0007,	//	BELL
/* 08 */	0x0008,	//	BACKSPACE
/* 09 */	0x0009,	//	HORIZONTAL TABULATION
/* 0A */	0x000A,	//	LINE FEED
/* 0B */	0x000B,	//	VERTICAL TABULATION
/* 0C */	0x000C,	//	FORM FEED
/* 0D */	0x000D,	//	CARRIAGE RETURN
/* 0E */	0x000E,	//	SHIFT OUT
/* 0F */	0x000F,	//	SHIFT IN
/* 10 */	0x0010,	//	DATA LINK ESCAPE
/* 11 */	0x0011,	//	DEVICE CONTROL ONE
/* 12 */	0x0012,	//	DEVICE CONTROL TWO
/* 13 */	0x0013,	//	DEVICE CONTROL THREE
/* 14 */	0x0014,	//	DEVICE CONTROL FOUR
/* 15 */	0x0015,	//	NEGATIVE ACKNOWLEDGE
/* 16 */	0x0016,	//	SYNCHRONOUS IDLE
/* 17 */	0x0017,	//	END OF TRANSMISSION BLOCK
/* 18 */	0x0018,	//	CANCEL
/* 19 */	0x0019,	//	END OF MEDIUM
/* 1A */	0x001A,	//	SUBSTITUTE
/* 1B */	0x001B,	//	ESCAPE
/* 1C */	0x001C,	//	FILE SEPARATOR
/* 1D */	0x001D,	//	GROUP SEPARATOR
/* 1E */	0x001E,	//	RECORD SEPARATOR
/* 1F */	0x001F,	//	UNIT SEPARATOR
/* 20 */	0x0020,	//	SPACE
/* 21 */	0x0021,	//	EXCLAMATION MARK
/* 22 */	0x0022,	//	QUOTATION MARK
/* 23 */	0x0023,	//	NUMBER SIGN
/* 24 */	0x0024,	//	DOLLAR SIGN
/* 25 */	0x0025,	//	PERCENT SIGN
/* 26 */	0x0026,	//	AMPERSAND
/* 27 */	0x0027,	//	APOSTROPHE
/* 28 */	0x0028,	//	LEFT PARENTHESIS
/* 29 */	0x0029,	//	RIGHT PARENTHESIS
/* 2A */	0x002A,	//	ASTERISK
/* 2B */	0x002B,	//	PLUS SIGN
/* 2C */	0x002C,	//	COMMA
/* 2D */	0x002D,	//	HYPHEN-MINUS
/* 2E */	0x002E,	//	FULL STOP
/* 2F */	0x002F,	//	SOLIDUS
/* 30 */	0x0030,	//	DIGIT ZERO
/* 31 */	0x0031,	//	DIGIT ONE
/* 32 */	0x0032,	//	DIGIT TWO
/* 33 */	0x0033,	//	DIGIT THREE
/* 34 */	0x0034,	//	DIGIT FOUR
/* 35 */	0x0035,	//	DIGIT FIVE
/* 36 */	0x0036,	//	DIGIT SIX
/* 37 */	0x0037,	//	DIGIT SEVEN
/* 38 */	0x0038,	//	DIGIT EIGHT
/* 39 */	0x0039,	//	DIGIT NINE
/* 3A */	0x003A,	//	COLON
/* 3B */	0x003B,	//	SEMICOLON
/* 3C */	0x003C,	//	LESS-THAN SIGN
/* 3D */	0x003D,	//	EQUALS SIGN
/* 3E */	0x003E,	//	GREATER-THAN SIGN
/* 3F */	0x003F,	//	QUESTION MARK
/* 40 */	0x0040,	//	COMMERCIAL AT
/* 41 */	0x0041,	//	LATIN CAPITAL LETTER A
/* 42 */	0x0042,	//	LATIN CAPITAL LETTER B
/* 43 */	0x0043,	//	LATIN CAPITAL LETTER C
/* 44 */	0x0044,	//	LATIN CAPITAL LETTER D
/* 45 */	0x0045,	//	LATIN CAPITAL LETTER E
/* 46 */	0x0046,	//	LATIN CAPITAL LETTER F
/* 47 */	0x0047,	//	LATIN CAPITAL LETTER G
/* 48 */	0x0048,	//	LATIN CAPITAL LETTER H
/* 49 */	0x0049,	//	LATIN CAPITAL LETTER I
/* 4A */	0x004A,	//	LATIN CAPITAL LETTER J
/* 4B */	0x004B,	//	LATIN CAPITAL LETTER K
/* 4C */	0x004C,	//	LATIN CAPITAL LETTER L
/* 4D */	0x004D,	//	LATIN CAPITAL LETTER M
/* 4E */	0x004E,	//	LATIN CAPITAL LETTER N
/* 4F */	0x004F,	//	LATIN CAPITAL LETTER O
/* 50 */	0x0050,	//	LATIN CAPITAL LETTER P
/* 51 */	0x0051,	//	LATIN CAPITAL LETTER Q
/* 52 */	0x0052,	//	LATIN CAPITAL LETTER R
/* 53 */	0x0053,	//	LATIN CAPITAL LETTER S
/* 54 */	0x0054,	//	LATIN CAPITAL LETTER T
/* 55 */	0x0055,	//	LATIN CAPITAL LETTER U
/* 56 */	0x0056,	//	LATIN CAPITAL LETTER V
/* 57 */	0x0057,	//	LATIN CAPITAL LETTER W
/* 58 */	0x0058,	//	LATIN CAPITAL LETTER X
/* 59 */	0x0059,	//	LATIN CAPITAL LETTER Y
/* 5A */	0x005A,	//	LATIN CAPITAL LETTER Z
/* 5B */	0x005B,	//	LEFT SQUARE BRACKET
/* 5C */	0x005C,	//	REVERSE SOLIDUS
/* 5D */	0x005D,	//	RIGHT SQUARE BRACKET
/* 5E */	0x005E,	//	CIRCUMFLEX ACCENT
/* 5F */	0x005F,	//	LOW LINE
/* 60 */	0x0060,	//	GRAVE ACCENT
/* 61 */	0x0061,	//	LATIN SMALL LETTER A
/* 62 */	0x0062,	//	LATIN SMALL LETTER B
/* 63 */	0x0063,	//	LATIN SMALL LETTER C
/* 64 */	0x0064,	//	LATIN SMALL LETTER D
/* 65 */	0x0065,	//	LATIN SMALL LETTER E
/* 66 */	0x0066,	//	LATIN SMALL LETTER F
/* 67 */	0x0067,	//	LATIN SMALL LETTER G
/* 68 */	0x0068,	//	LATIN SMALL LETTER H
/* 69 */	0x0069,	//	LATIN SMALL LETTER I
/* 6A */	0x006A,	//	LATIN SMALL LETTER J
/* 6B */	0x006B,	//	LATIN SMALL LETTER K
/* 6C */	0x006C,	//	LATIN SMALL LETTER L
/* 6D */	0x006D,	//	LATIN SMALL LETTER M
/* 6E */	0x006E,	//	LATIN SMALL LETTER N
/* 6F */	0x006F,	//	LATIN SMALL LETTER O
/* 70 */	0x0070,	//	LATIN SMALL LETTER P
/* 71 */	0x0071,	//	LATIN SMALL LETTER Q
/* 72 */	0x0072,	//	LATIN SMALL LETTER R
/* 73 */	0x0073,	//	LATIN SMALL LETTER S
/* 74 */	0x0074,	//	LATIN SMALL LETTER T
/* 75 */	0x0075,	//	LATIN SMALL LETTER U
/* 76 */	0x0076,	//	LATIN SMALL LETTER V
/* 77 */	0x0077,	//	LATIN SMALL LETTER W
/* 78 */	0x0078,	//	LATIN SMALL LETTER X
/* 79 */	0x0079,	//	LATIN SMALL LETTER Y
/* 7A */	0x007A,	//	LATIN SMALL LETTER Z
/* 7B */	0x007B,	//	LEFT CURLY BRACKET
/* 7C */	0x007C,	//	VERTICAL LINE
/* 7D */	0x007D,	//	RIGHT CURLY BRACKET
/* 7E */	0x007E,	//	TILDE
/* 7F */	0x007F,	//	DELETE
/* 80 */	0x0080,	//	<control>
/* 81 */	0x0081,	//	<control>
/* 82 */	0x0082,	//	<control>
/* 83 */	0x0083,	//	<control>
/* 84 */	0x0084,	//	<control>
/* 85 */	0x0085,	//	<control>
/* 86 */	0x0086,	//	<control>
/* 87 */	0x0087,	//	<control>
/* 88 */	0x0088,	//	<control>
/* 89 */	0x0089,	//	<control>
/* 8A */	0x008A,	//	<control>
/* 8B */	0x008B,	//	<control>
/* 8C */	0x008C,	//	<control>
/* 8D */	0x008D,	//	<control>
/* 8E */	0x008E,	//	<control>
/* 8F */	0x008F,	//	<control>
/* 90 */	0x0090,	//	<control>
/* 91 */	0x0091,	//	<control>
/* 92 */	0x0092,	//	<control>
/* 93 */	0x0093,	//	<control>
/* 94 */	0x0094,	//	<control>
/* 95 */	0x0095,	//	<control>
/* 96 */	0x0096,	//	<control>
/* 97 */	0x0097,	//	<control>
/* 98 */	0x0098,	//	<control>
/* 99 */	0x0099,	//	<control>
/* 9A */	0x009A,	//	<control>
/* 9B */	0x009B,	//	<control>
/* 9C */	0x009C,	//	<control>
/* 9D */	0x009D,	//	<control>
/* 9E */	0x009E,	//	<control>
/* 9F */	0x009F,	//	<control>
/* A0 */	0x00A0,	//	NO-BREAK SPACE
/* A1 */	0x2018,	//	LEFT SINGLE QUOTATION MARK
/* A2 */	0x2019,	//	RIGHT SINGLE QUOTATION MARK
/* A3 */	0x00A3,	//	POUND SIGN
/* A4 */	0x20AC,	//	EURO SIGN
/* A5 */	0x20AF,	//	DRACHMA SIGN
/* A6 */	0x00A6,	//	BROKEN BAR
/* A7 */	0x00A7,	//	SECTION SIGN
/* A8 */	0x00A8,	//	DIAERESIS
/* A9 */	0x00A9,	//	COPYRIGHT SIGN
/* AA */	0x037A,	//	GREEK YPOGEGRAMMENI
/* AB */	0x00AB,	//	LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
/* AC */	0x00AC,	//	NOT SIGN
/* AD */	0x00AD,	//	SOFT HYPHEN
/* AF */	0x2015,	//	HORIZONTAL BAR
/* B0 */	0x00B0,	//	DEGREE SIGN
/* B1 */	0x00B1,	//	PLUS-MINUS SIGN
/* B2 */	0x00B2,	//	SUPERSCRIPT TWO
/* B3 */	0x00B3,	//	SUPERSCRIPT THREE
/* B4 */	0x0384,	//	GREEK TONOS
/* B5 */	0x0385,	//	GREEK DIALYTIKA TONOS
/* B6 */	0x0386,	//	GREEK CAPITAL LETTER ALPHA WITH TONOS
/* B7 */	0x00B7,	//	MIDDLE DOT
/* B8 */	0x0388,	//	GREEK CAPITAL LETTER EPSILON WITH TONOS
/* B9 */	0x0389,	//	GREEK CAPITAL LETTER ETA WITH TONOS
/* BA */	0x038A,	//	GREEK CAPITAL LETTER IOTA WITH TONOS
/* BB */	0x00BB,	//	RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
/* BC */	0x038C,	//	GREEK CAPITAL LETTER OMICRON WITH TONOS
/* BD */	0x00BD,	//	VULGAR FRACTION ONE HALF
/* BE */	0x038E,	//	GREEK CAPITAL LETTER UPSILON WITH TONOS
/* BF */	0x038F,	//	GREEK CAPITAL LETTER OMEGA WITH TONOS
/* C0 */	0x0390,	//	GREEK SMALL LETTER IOTA WITH DIALYTIKA AND TONOS
/* C1 */	0x0391,	//	GREEK CAPITAL LETTER ALPHA
/* C2 */	0x0392,	//	GREEK CAPITAL LETTER BETA
/* C3 */	0x0393,	//	GREEK CAPITAL LETTER GAMMA
/* C4 */	0x0394,	//	GREEK CAPITAL LETTER DELTA
/* C5 */	0x0395,	//	GREEK CAPITAL LETTER EPSILON
/* C6 */	0x0396,	//	GREEK CAPITAL LETTER ZETA
/* C7 */	0x0397,	//	GREEK CAPITAL LETTER ETA
/* C8 */	0x0398,	//	GREEK CAPITAL LETTER THETA
/* C9 */	0x0399,	//	GREEK CAPITAL LETTER IOTA
/* CA */	0x039A,	//	GREEK CAPITAL LETTER KAPPA
/* CB */	0x039B,	//	GREEK CAPITAL LETTER LAMDA
/* CC */	0x039C,	//	GREEK CAPITAL LETTER MU
/* CD */	0x039D,	//	GREEK CAPITAL LETTER NU
/* CE */	0x039E,	//	GREEK CAPITAL LETTER XI
/* CF */	0x039F,	//	GREEK CAPITAL LETTER OMICRON
/* D0 */	0x03A0,	//	GREEK CAPITAL LETTER PI
/* D1 */	0x03A1,	//	GREEK CAPITAL LETTER RHO
/* D3 */	0x03A3,	//	GREEK CAPITAL LETTER SIGMA
/* D4 */	0x03A4,	//	GREEK CAPITAL LETTER TAU
/* D5 */	0x03A5,	//	GREEK CAPITAL LETTER UPSILON
/* D6 */	0x03A6,	//	GREEK CAPITAL LETTER PHI
/* D7 */	0x03A7,	//	GREEK CAPITAL LETTER CHI
/* D8 */	0x03A8,	//	GREEK CAPITAL LETTER PSI
/* D9 */	0x03A9,	//	GREEK CAPITAL LETTER OMEGA
/* DA */	0x03AA,	//	GREEK CAPITAL LETTER IOTA WITH DIALYTIKA
/* DB */	0x03AB,	//	GREEK CAPITAL LETTER UPSILON WITH DIALYTIKA
/* DC */	0x03AC,	//	GREEK SMALL LETTER ALPHA WITH TONOS
/* DD */	0x03AD,	//	GREEK SMALL LETTER EPSILON WITH TONOS
/* DE */	0x03AE,	//	GREEK SMALL LETTER ETA WITH TONOS
/* DF */	0x03AF,	//	GREEK SMALL LETTER IOTA WITH TONOS
/* E0 */	0x03B0,	//	GREEK SMALL LETTER UPSILON WITH DIALYTIKA AND TONOS
/* E1 */	0x03B1,	//	GREEK SMALL LETTER ALPHA
/* E2 */	0x03B2,	//	GREEK SMALL LETTER BETA
/* E3 */	0x03B3,	//	GREEK SMALL LETTER GAMMA
/* E4 */	0x03B4,	//	GREEK SMALL LETTER DELTA
/* E5 */	0x03B5,	//	GREEK SMALL LETTER EPSILON
/* E6 */	0x03B6,	//	GREEK SMALL LETTER ZETA
/* E7 */	0x03B7,	//	GREEK SMALL LETTER ETA
/* E8 */	0x03B8,	//	GREEK SMALL LETTER THETA
/* E9 */	0x03B9,	//	GREEK SMALL LETTER IOTA
/* EA */	0x03BA,	//	GREEK SMALL LETTER KAPPA
/* EB */	0x03BB,	//	GREEK SMALL LETTER LAMDA
/* EC */	0x03BC,	//	GREEK SMALL LETTER MU
/* ED */	0x03BD,	//	GREEK SMALL LETTER NU
/* EE */	0x03BE,	//	GREEK SMALL LETTER XI
/* EF */	0x03BF,	//	GREEK SMALL LETTER OMICRON
/* F0 */	0x03C0,	//	GREEK SMALL LETTER PI
/* F1 */	0x03C1,	//	GREEK SMALL LETTER RHO
/* F2 */	0x03C2,	//	GREEK SMALL LETTER FINAL SIGMA
/* F3 */	0x03C3,	//	GREEK SMALL LETTER SIGMA
/* F4 */	0x03C4,	//	GREEK SMALL LETTER TAU
/* F5 */	0x03C5,	//	GREEK SMALL LETTER UPSILON
/* F6 */	0x03C6,	//	GREEK SMALL LETTER PHI
/* F7 */	0x03C7,	//	GREEK SMALL LETTER CHI
/* F8 */	0x03C8,	//	GREEK SMALL LETTER PSI
/* F9 */	0x03C9,	//	GREEK SMALL LETTER OMEGA
/* FA */	0x03CA,	//	GREEK SMALL LETTER IOTA WITH DIALYTIKA
/* FB */	0x03CB,	//	GREEK SMALL LETTER UPSILON WITH DIALYTIKA
/* FC */	0x03CC,	//	GREEK SMALL LETTER OMICRON WITH TONOS
/* FD */	0x03CD,	//	GREEK SMALL LETTER UPSILON WITH TONOS
/* FE */	0x03CE,	//	GREEK SMALL LETTER OMEGA WITH TONOS
/* FF */	0xFFFD,	//	INVALID
};

/***************************************************************************
#
#	Name:             ISO/IEC 8859-8:1999 to Unicode
#	Unicode version:  3.0
#	Table version:    1.1
#	Table format:     Format A
#	Date:             2000-Jan-03
#	Authors:          Ken Whistler <kenw@sybase.com>
#
#	Copyright (c) 1991-1999 Unicode, Inc.  All Rights reserved.
#
#	This file is provided as-is by Unicode, Inc. (The Unicode Consortium).
#	No claims are made as to fitness for any particular purpose.  No
#	warranties of any kind are expressed or implied.  The recipient
#	agrees to determine applicability of information provided.  If this
#	file has been provided on optical media by Unicode, Inc., the sole
#	remedy for any claim will be exchange of defective media within 90
#	days of receipt.
#
#	Unicode, Inc. hereby grants the right to freely use the information
#	supplied in this file in the creation of products supporting the
#	Unicode Standard, and to make copies of this file in any form for
#	internal or external distribution as long as this notice remains
#	attached.
#
#	General notes:
#
#	This table contains the data the Unicode Consortium has on how
#       ISO/IEC 8859-8:1999 characters map into Unicode.
#
#	Format:  Three tab-separated columns
#		 Column #1 is the ISO/IEC 8859-8 code (in hex as 0xXX)
#		 Column #2 is the Unicode (in hex as 0xXXXX)
#		 Column #3 the Unicode name (follows a comment sign, '#')
#
#	The entries are in ISO/IEC 8859-8 order.
#
#	Version history
#	1.0 version updates 0.1 version by adding mappings for all
#	control characters.
#       1.1 version updates to the published 8859-8:1999, correcting
#          the mapping of 0xAF and adding mappings for LRM and RLM.
#
#	Updated versions of this file may be found in:
#		<ftp://ftp.unicode.org/Public/MAPPINGS/>
#
#	Any comments or problems, contact <errata@unicode.org>
#	Please note that <errata@unicode.org> is an archival address;
#	notices will be checked, but do not expect an immediate response.
#
***************************************************************************/
static const unicode_char lookup_iso8859_8[256] = {
/* 00 */	0x0000,	//	NULL
/* 01 */	0x0001,	//	START OF HEADING
/* 02 */	0x0002,	//	START OF TEXT
/* 03 */	0x0003,	//	END OF TEXT
/* 04 */	0x0004,	//	END OF TRANSMISSION
/* 05 */	0x0005,	//	ENQUIRY
/* 06 */	0x0006,	//	ACKNOWLEDGE
/* 07 */	0x0007,	//	BELL
/* 08 */	0x0008,	//	BACKSPACE
/* 09 */	0x0009,	//	HORIZONTAL TABULATION
/* 0A */	0x000A,	//	LINE FEED
/* 0B */	0x000B,	//	VERTICAL TABULATION
/* 0C */	0x000C,	//	FORM FEED
/* 0D */	0x000D,	//	CARRIAGE RETURN
/* 0E */	0x000E,	//	SHIFT OUT
/* 0F */	0x000F,	//	SHIFT IN
/* 10 */	0x0010,	//	DATA LINK ESCAPE
/* 11 */	0x0011,	//	DEVICE CONTROL ONE
/* 12 */	0x0012,	//	DEVICE CONTROL TWO
/* 13 */	0x0013,	//	DEVICE CONTROL THREE
/* 14 */	0x0014,	//	DEVICE CONTROL FOUR
/* 15 */	0x0015,	//	NEGATIVE ACKNOWLEDGE
/* 16 */	0x0016,	//	SYNCHRONOUS IDLE
/* 17 */	0x0017,	//	END OF TRANSMISSION BLOCK
/* 18 */	0x0018,	//	CANCEL
/* 19 */	0x0019,	//	END OF MEDIUM
/* 1A */	0x001A,	//	SUBSTITUTE
/* 1B */	0x001B,	//	ESCAPE
/* 1C */	0x001C,	//	FILE SEPARATOR
/* 1D */	0x001D,	//	GROUP SEPARATOR
/* 1E */	0x001E,	//	RECORD SEPARATOR
/* 1F */	0x001F,	//	UNIT SEPARATOR
/* 20 */	0x0020,	//	SPACE
/* 21 */	0x0021,	//	EXCLAMATION MARK
/* 22 */	0x0022,	//	QUOTATION MARK
/* 23 */	0x0023,	//	NUMBER SIGN
/* 24 */	0x0024,	//	DOLLAR SIGN
/* 25 */	0x0025,	//	PERCENT SIGN
/* 26 */	0x0026,	//	AMPERSAND
/* 27 */	0x0027,	//	APOSTROPHE
/* 28 */	0x0028,	//	LEFT PARENTHESIS
/* 29 */	0x0029,	//	RIGHT PARENTHESIS
/* 2A */	0x002A,	//	ASTERISK
/* 2B */	0x002B,	//	PLUS SIGN
/* 2C */	0x002C,	//	COMMA
/* 2D */	0x002D,	//	HYPHEN-MINUS
/* 2E */	0x002E,	//	FULL STOP
/* 2F */	0x002F,	//	SOLIDUS
/* 30 */	0x0030,	//	DIGIT ZERO
/* 31 */	0x0031,	//	DIGIT ONE
/* 32 */	0x0032,	//	DIGIT TWO
/* 33 */	0x0033,	//	DIGIT THREE
/* 34 */	0x0034,	//	DIGIT FOUR
/* 35 */	0x0035,	//	DIGIT FIVE
/* 36 */	0x0036,	//	DIGIT SIX
/* 37 */	0x0037,	//	DIGIT SEVEN
/* 38 */	0x0038,	//	DIGIT EIGHT
/* 39 */	0x0039,	//	DIGIT NINE
/* 3A */	0x003A,	//	COLON
/* 3B */	0x003B,	//	SEMICOLON
/* 3C */	0x003C,	//	LESS-THAN SIGN
/* 3D */	0x003D,	//	EQUALS SIGN
/* 3E */	0x003E,	//	GREATER-THAN SIGN
/* 3F */	0x003F,	//	QUESTION MARK
/* 40 */	0x0040,	//	COMMERCIAL AT
/* 41 */	0x0041,	//	LATIN CAPITAL LETTER A
/* 42 */	0x0042,	//	LATIN CAPITAL LETTER B
/* 43 */	0x0043,	//	LATIN CAPITAL LETTER C
/* 44 */	0x0044,	//	LATIN CAPITAL LETTER D
/* 45 */	0x0045,	//	LATIN CAPITAL LETTER E
/* 46 */	0x0046,	//	LATIN CAPITAL LETTER F
/* 47 */	0x0047,	//	LATIN CAPITAL LETTER G
/* 48 */	0x0048,	//	LATIN CAPITAL LETTER H
/* 49 */	0x0049,	//	LATIN CAPITAL LETTER I
/* 4A */	0x004A,	//	LATIN CAPITAL LETTER J
/* 4B */	0x004B,	//	LATIN CAPITAL LETTER K
/* 4C */	0x004C,	//	LATIN CAPITAL LETTER L
/* 4D */	0x004D,	//	LATIN CAPITAL LETTER M
/* 4E */	0x004E,	//	LATIN CAPITAL LETTER N
/* 4F */	0x004F,	//	LATIN CAPITAL LETTER O
/* 50 */	0x0050,	//	LATIN CAPITAL LETTER P
/* 51 */	0x0051,	//	LATIN CAPITAL LETTER Q
/* 52 */	0x0052,	//	LATIN CAPITAL LETTER R
/* 53 */	0x0053,	//	LATIN CAPITAL LETTER S
/* 54 */	0x0054,	//	LATIN CAPITAL LETTER T
/* 55 */	0x0055,	//	LATIN CAPITAL LETTER U
/* 56 */	0x0056,	//	LATIN CAPITAL LETTER V
/* 57 */	0x0057,	//	LATIN CAPITAL LETTER W
/* 58 */	0x0058,	//	LATIN CAPITAL LETTER X
/* 59 */	0x0059,	//	LATIN CAPITAL LETTER Y
/* 5A */	0x005A,	//	LATIN CAPITAL LETTER Z
/* 5B */	0x005B,	//	LEFT SQUARE BRACKET
/* 5C */	0x005C,	//	REVERSE SOLIDUS
/* 5D */	0x005D,	//	RIGHT SQUARE BRACKET
/* 5E */	0x005E,	//	CIRCUMFLEX ACCENT
/* 5F */	0x005F,	//	LOW LINE
/* 60 */	0x0060,	//	GRAVE ACCENT
/* 61 */	0x0061,	//	LATIN SMALL LETTER A
/* 62 */	0x0062,	//	LATIN SMALL LETTER B
/* 63 */	0x0063,	//	LATIN SMALL LETTER C
/* 64 */	0x0064,	//	LATIN SMALL LETTER D
/* 65 */	0x0065,	//	LATIN SMALL LETTER E
/* 66 */	0x0066,	//	LATIN SMALL LETTER F
/* 67 */	0x0067,	//	LATIN SMALL LETTER G
/* 68 */	0x0068,	//	LATIN SMALL LETTER H
/* 69 */	0x0069,	//	LATIN SMALL LETTER I
/* 6A */	0x006A,	//	LATIN SMALL LETTER J
/* 6B */	0x006B,	//	LATIN SMALL LETTER K
/* 6C */	0x006C,	//	LATIN SMALL LETTER L
/* 6D */	0x006D,	//	LATIN SMALL LETTER M
/* 6E */	0x006E,	//	LATIN SMALL LETTER N
/* 6F */	0x006F,	//	LATIN SMALL LETTER O
/* 70 */	0x0070,	//	LATIN SMALL LETTER P
/* 71 */	0x0071,	//	LATIN SMALL LETTER Q
/* 72 */	0x0072,	//	LATIN SMALL LETTER R
/* 73 */	0x0073,	//	LATIN SMALL LETTER S
/* 74 */	0x0074,	//	LATIN SMALL LETTER T
/* 75 */	0x0075,	//	LATIN SMALL LETTER U
/* 76 */	0x0076,	//	LATIN SMALL LETTER V
/* 77 */	0x0077,	//	LATIN SMALL LETTER W
/* 78 */	0x0078,	//	LATIN SMALL LETTER X
/* 79 */	0x0079,	//	LATIN SMALL LETTER Y
/* 7A */	0x007A,	//	LATIN SMALL LETTER Z
/* 7B */	0x007B,	//	LEFT CURLY BRACKET
/* 7C */	0x007C,	//	VERTICAL LINE
/* 7D */	0x007D,	//	RIGHT CURLY BRACKET
/* 7E */	0x007E,	//	TILDE
/* 7F */	0x007F,	//	DELETE
/* 80 */	0x0080,	//	<control>
/* 81 */	0x0081,	//	<control>
/* 82 */	0x0082,	//	<control>
/* 83 */	0x0083,	//	<control>
/* 84 */	0x0084,	//	<control>
/* 85 */	0x0085,	//	<control>
/* 86 */	0x0086,	//	<control>
/* 87 */	0x0087,	//	<control>
/* 88 */	0x0088,	//	<control>
/* 89 */	0x0089,	//	<control>
/* 8A */	0x008A,	//	<control>
/* 8B */	0x008B,	//	<control>
/* 8C */	0x008C,	//	<control>
/* 8D */	0x008D,	//	<control>
/* 8E */	0x008E,	//	<control>
/* 8F */	0x008F,	//	<control>
/* 90 */	0x0090,	//	<control>
/* 91 */	0x0091,	//	<control>
/* 92 */	0x0092,	//	<control>
/* 93 */	0x0093,	//	<control>
/* 94 */	0x0094,	//	<control>
/* 95 */	0x0095,	//	<control>
/* 96 */	0x0096,	//	<control>
/* 97 */	0x0097,	//	<control>
/* 98 */	0x0098,	//	<control>
/* 99 */	0x0099,	//	<control>
/* 9A */	0x009A,	//	<control>
/* 9B */	0x009B,	//	<control>
/* 9C */	0x009C,	//	<control>
/* 9D */	0x009D,	//	<control>
/* 9E */	0x009E,	//	<control>
/* 9F */	0x009F,	//	<control>
/* A0 */	0x00A0,	//	NO-BREAK SPACE
/* A2 */	0x00A2,	//	CENT SIGN
/* A3 */	0x00A3,	//	POUND SIGN
/* A4 */	0x00A4,	//	CURRENCY SIGN
/* A5 */	0x00A5,	//	YEN SIGN
/* A6 */	0x00A6,	//	BROKEN BAR
/* A7 */	0x00A7,	//	SECTION SIGN
/* A8 */	0x00A8,	//	DIAERESIS
/* A9 */	0x00A9,	//	COPYRIGHT SIGN
/* AA */	0x00D7,	//	MULTIPLICATION SIGN
/* AB */	0x00AB,	//	LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
/* AC */	0x00AC,	//	NOT SIGN
/* AD */	0x00AD,	//	SOFT HYPHEN
/* AE */	0x00AE,	//	REGISTERED SIGN
/* AF */	0x00AF,	//	MACRON
/* B0 */	0x00B0,	//	DEGREE SIGN
/* B1 */	0x00B1,	//	PLUS-MINUS SIGN
/* B2 */	0x00B2,	//	SUPERSCRIPT TWO
/* B3 */	0x00B3,	//	SUPERSCRIPT THREE
/* B4 */	0x00B4,	//	ACUTE ACCENT
/* B5 */	0x00B5,	//	MICRO SIGN
/* B6 */	0x00B6,	//	PILCROW SIGN
/* B7 */	0x00B7,	//	MIDDLE DOT
/* B8 */	0x00B8,	//	CEDILLA
/* B9 */	0x00B9,	//	SUPERSCRIPT ONE
/* BA */	0x00F7,	//	DIVISION SIGN
/* BB */	0x00BB,	//	RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
/* BC */	0x00BC,	//	VULGAR FRACTION ONE QUARTER
/* BD */	0x00BD,	//	VULGAR FRACTION ONE HALF
/* BE */	0x00BE,	//	VULGAR FRACTION THREE QUARTERS
/* DF */	0x2017,	//	DOUBLE LOW LINE
/* E0 */	0x05D0,	//	HEBREW LETTER ALEF
/* E1 */	0x05D1,	//	HEBREW LETTER BET
/* E2 */	0x05D2,	//	HEBREW LETTER GIMEL
/* E3 */	0x05D3,	//	HEBREW LETTER DALET
/* E4 */	0x05D4,	//	HEBREW LETTER HE
/* E5 */	0x05D5,	//	HEBREW LETTER VAV
/* E6 */	0x05D6,	//	HEBREW LETTER ZAYIN
/* E7 */	0x05D7,	//	HEBREW LETTER HET
/* E8 */	0x05D8,	//	HEBREW LETTER TET
/* E9 */	0x05D9,	//	HEBREW LETTER YOD
/* EA */	0x05DA,	//	HEBREW LETTER FINAL KAF
/* EB */	0x05DB,	//	HEBREW LETTER KAF
/* EC */	0x05DC,	//	HEBREW LETTER LAMED
/* ED */	0x05DD,	//	HEBREW LETTER FINAL MEM
/* EE */	0x05DE,	//	HEBREW LETTER MEM
/* EF */	0x05DF,	//	HEBREW LETTER FINAL NUN
/* F0 */	0x05E0,	//	HEBREW LETTER NUN
/* F1 */	0x05E1,	//	HEBREW LETTER SAMEKH
/* F2 */	0x05E2,	//	HEBREW LETTER AYIN
/* F3 */	0x05E3,	//	HEBREW LETTER FINAL PE
/* F4 */	0x05E4,	//	HEBREW LETTER PE
/* F5 */	0x05E5,	//	HEBREW LETTER FINAL TSADI
/* F6 */	0x05E6,	//	HEBREW LETTER TSADI
/* F7 */	0x05E7,	//	HEBREW LETTER QOF
/* F8 */	0x05E8,	//	HEBREW LETTER RESH
/* F9 */	0x05E9,	//	HEBREW LETTER SHIN
/* FA */	0x05EA,	//	HEBREW LETTER TAV
/* FD */	0x200E,	//	LEFT-TO-RIGHT MARK
/* FE */	0x200F,	//	RIGHT-TO-LEFT MARK
/* FF */	0xFFFD	//	INVALID
};

/***************************************************************************
#
#	Name:             ISO/IEC 8859-9:1999 to Unicode
#	Unicode version:  3.0
#	Table version:    1.0
#	Table format:     Format A
#	Date:             1999 July 27
#	Authors:          Ken Whistler <kenw@sybase.com>
#
#	Copyright (c) 1991-1999 Unicode, Inc.  All Rights reserved.
#
#	This file is provided as-is by Unicode, Inc. (The Unicode Consortium).
#	No claims are made as to fitness for any particular purpose.  No
#	warranties of any kind are expressed or implied.  The recipient
#	agrees to determine applicability of information provided.  If this
#	file has been provided on magnetic media by Unicode, Inc., the sole
#	remedy for any claim will be exchange of defective media within 90
#	days of receipt.
#
#	Unicode, Inc. hereby grants the right to freely use the information
#	supplied in this file in the creation of products supporting the
#	Unicode Standard, and to make copies of this file in any form for
#	internal or external distribution as long as this notice remains
#	attached.
#
#	General notes:
#
#	This table contains the data the Unicode Consortium has on how
#       ISO/IEC 8859-9:1999 characters map into Unicode.
#
#	Format:  Three tab-separated columns
#		 Column #1 is the ISO/IEC 8859-9 code (in hex as 0xXX)
#		 Column #2 is the Unicode (in hex as 0xXXXX)
#		 Column #3 the Unicode name (follows a comment sign, '#')
#
#	The entries are in ISO/IEC 8859-9 order.
#
#	ISO/IEC 8859-9 is also equivalent to ISO-IR-148.
#
#	Version history
#	1.0 version updates 0.1 version by adding mappings for all
#	control characters.
#
#	Updated versions of this file may be found in:
#		<ftp://ftp.unicode.org/Public/MAPPINGS/>
#
#	Any comments or problems, contact <errata@unicode.org>
#	Please note that <errata@unicode.org> is an archival address;
#	notices will be checked, but do not expect an immediate response.
#
***************************************************************************/
static const unicode_char lookup_iso8859_9[256] = {
/* 00 */	0x0000,	//	NULL
/* 01 */	0x0001,	//	START OF HEADING
/* 02 */	0x0002,	//	START OF TEXT
/* 03 */	0x0003,	//	END OF TEXT
/* 04 */	0x0004,	//	END OF TRANSMISSION
/* 05 */	0x0005,	//	ENQUIRY
/* 06 */	0x0006,	//	ACKNOWLEDGE
/* 07 */	0x0007,	//	BELL
/* 08 */	0x0008,	//	BACKSPACE
/* 09 */	0x0009,	//	HORIZONTAL TABULATION
/* 0A */	0x000A,	//	LINE FEED
/* 0B */	0x000B,	//	VERTICAL TABULATION
/* 0C */	0x000C,	//	FORM FEED
/* 0D */	0x000D,	//	CARRIAGE RETURN
/* 0E */	0x000E,	//	SHIFT OUT
/* 0F */	0x000F,	//	SHIFT IN
/* 10 */	0x0010,	//	DATA LINK ESCAPE
/* 11 */	0x0011,	//	DEVICE CONTROL ONE
/* 12 */	0x0012,	//	DEVICE CONTROL TWO
/* 13 */	0x0013,	//	DEVICE CONTROL THREE
/* 14 */	0x0014,	//	DEVICE CONTROL FOUR
/* 15 */	0x0015,	//	NEGATIVE ACKNOWLEDGE
/* 16 */	0x0016,	//	SYNCHRONOUS IDLE
/* 17 */	0x0017,	//	END OF TRANSMISSION BLOCK
/* 18 */	0x0018,	//	CANCEL
/* 19 */	0x0019,	//	END OF MEDIUM
/* 1A */	0x001A,	//	SUBSTITUTE
/* 1B */	0x001B,	//	ESCAPE
/* 1C */	0x001C,	//	FILE SEPARATOR
/* 1D */	0x001D,	//	GROUP SEPARATOR
/* 1E */	0x001E,	//	RECORD SEPARATOR
/* 1F */	0x001F,	//	UNIT SEPARATOR
/* 20 */	0x0020,	//	SPACE
/* 21 */	0x0021,	//	EXCLAMATION MARK
/* 22 */	0x0022,	//	QUOTATION MARK
/* 23 */	0x0023,	//	NUMBER SIGN
/* 24 */	0x0024,	//	DOLLAR SIGN
/* 25 */	0x0025,	//	PERCENT SIGN
/* 26 */	0x0026,	//	AMPERSAND
/* 27 */	0x0027,	//	APOSTROPHE
/* 28 */	0x0028,	//	LEFT PARENTHESIS
/* 29 */	0x0029,	//	RIGHT PARENTHESIS
/* 2A */	0x002A,	//	ASTERISK
/* 2B */	0x002B,	//	PLUS SIGN
/* 2C */	0x002C,	//	COMMA
/* 2D */	0x002D,	//	HYPHEN-MINUS
/* 2E */	0x002E,	//	FULL STOP
/* 2F */	0x002F,	//	SOLIDUS
/* 30 */	0x0030,	//	DIGIT ZERO
/* 31 */	0x0031,	//	DIGIT ONE
/* 32 */	0x0032,	//	DIGIT TWO
/* 33 */	0x0033,	//	DIGIT THREE
/* 34 */	0x0034,	//	DIGIT FOUR
/* 35 */	0x0035,	//	DIGIT FIVE
/* 36 */	0x0036,	//	DIGIT SIX
/* 37 */	0x0037,	//	DIGIT SEVEN
/* 38 */	0x0038,	//	DIGIT EIGHT
/* 39 */	0x0039,	//	DIGIT NINE
/* 3A */	0x003A,	//	COLON
/* 3B */	0x003B,	//	SEMICOLON
/* 3C */	0x003C,	//	LESS-THAN SIGN
/* 3D */	0x003D,	//	EQUALS SIGN
/* 3E */	0x003E,	//	GREATER-THAN SIGN
/* 3F */	0x003F,	//	QUESTION MARK
/* 40 */	0x0040,	//	COMMERCIAL AT
/* 41 */	0x0041,	//	LATIN CAPITAL LETTER A
/* 42 */	0x0042,	//	LATIN CAPITAL LETTER B
/* 43 */	0x0043,	//	LATIN CAPITAL LETTER C
/* 44 */	0x0044,	//	LATIN CAPITAL LETTER D
/* 45 */	0x0045,	//	LATIN CAPITAL LETTER E
/* 46 */	0x0046,	//	LATIN CAPITAL LETTER F
/* 47 */	0x0047,	//	LATIN CAPITAL LETTER G
/* 48 */	0x0048,	//	LATIN CAPITAL LETTER H
/* 49 */	0x0049,	//	LATIN CAPITAL LETTER I
/* 4A */	0x004A,	//	LATIN CAPITAL LETTER J
/* 4B */	0x004B,	//	LATIN CAPITAL LETTER K
/* 4C */	0x004C,	//	LATIN CAPITAL LETTER L
/* 4D */	0x004D,	//	LATIN CAPITAL LETTER M
/* 4E */	0x004E,	//	LATIN CAPITAL LETTER N
/* 4F */	0x004F,	//	LATIN CAPITAL LETTER O
/* 50 */	0x0050,	//	LATIN CAPITAL LETTER P
/* 51 */	0x0051,	//	LATIN CAPITAL LETTER Q
/* 52 */	0x0052,	//	LATIN CAPITAL LETTER R
/* 53 */	0x0053,	//	LATIN CAPITAL LETTER S
/* 54 */	0x0054,	//	LATIN CAPITAL LETTER T
/* 55 */	0x0055,	//	LATIN CAPITAL LETTER U
/* 56 */	0x0056,	//	LATIN CAPITAL LETTER V
/* 57 */	0x0057,	//	LATIN CAPITAL LETTER W
/* 58 */	0x0058,	//	LATIN CAPITAL LETTER X
/* 59 */	0x0059,	//	LATIN CAPITAL LETTER Y
/* 5A */	0x005A,	//	LATIN CAPITAL LETTER Z
/* 5B */	0x005B,	//	LEFT SQUARE BRACKET
/* 5C */	0x005C,	//	REVERSE SOLIDUS
/* 5D */	0x005D,	//	RIGHT SQUARE BRACKET
/* 5E */	0x005E,	//	CIRCUMFLEX ACCENT
/* 5F */	0x005F,	//	LOW LINE
/* 60 */	0x0060,	//	GRAVE ACCENT
/* 61 */	0x0061,	//	LATIN SMALL LETTER A
/* 62 */	0x0062,	//	LATIN SMALL LETTER B
/* 63 */	0x0063,	//	LATIN SMALL LETTER C
/* 64 */	0x0064,	//	LATIN SMALL LETTER D
/* 65 */	0x0065,	//	LATIN SMALL LETTER E
/* 66 */	0x0066,	//	LATIN SMALL LETTER F
/* 67 */	0x0067,	//	LATIN SMALL LETTER G
/* 68 */	0x0068,	//	LATIN SMALL LETTER H
/* 69 */	0x0069,	//	LATIN SMALL LETTER I
/* 6A */	0x006A,	//	LATIN SMALL LETTER J
/* 6B */	0x006B,	//	LATIN SMALL LETTER K
/* 6C */	0x006C,	//	LATIN SMALL LETTER L
/* 6D */	0x006D,	//	LATIN SMALL LETTER M
/* 6E */	0x006E,	//	LATIN SMALL LETTER N
/* 6F */	0x006F,	//	LATIN SMALL LETTER O
/* 70 */	0x0070,	//	LATIN SMALL LETTER P
/* 71 */	0x0071,	//	LATIN SMALL LETTER Q
/* 72 */	0x0072,	//	LATIN SMALL LETTER R
/* 73 */	0x0073,	//	LATIN SMALL LETTER S
/* 74 */	0x0074,	//	LATIN SMALL LETTER T
/* 75 */	0x0075,	//	LATIN SMALL LETTER U
/* 76 */	0x0076,	//	LATIN SMALL LETTER V
/* 77 */	0x0077,	//	LATIN SMALL LETTER W
/* 78 */	0x0078,	//	LATIN SMALL LETTER X
/* 79 */	0x0079,	//	LATIN SMALL LETTER Y
/* 7A */	0x007A,	//	LATIN SMALL LETTER Z
/* 7B */	0x007B,	//	LEFT CURLY BRACKET
/* 7C */	0x007C,	//	VERTICAL LINE
/* 7D */	0x007D,	//	RIGHT CURLY BRACKET
/* 7E */	0x007E,	//	TILDE
/* 7F */	0x007F,	//	DELETE
/* 80 */	0x0080,	//	<control>
/* 81 */	0x0081,	//	<control>
/* 82 */	0x0082,	//	<control>
/* 83 */	0x0083,	//	<control>
/* 84 */	0x0084,	//	<control>
/* 85 */	0x0085,	//	<control>
/* 86 */	0x0086,	//	<control>
/* 87 */	0x0087,	//	<control>
/* 88 */	0x0088,	//	<control>
/* 89 */	0x0089,	//	<control>
/* 8A */	0x008A,	//	<control>
/* 8B */	0x008B,	//	<control>
/* 8C */	0x008C,	//	<control>
/* 8D */	0x008D,	//	<control>
/* 8E */	0x008E,	//	<control>
/* 8F */	0x008F,	//	<control>
/* 90 */	0x0090,	//	<control>
/* 91 */	0x0091,	//	<control>
/* 92 */	0x0092,	//	<control>
/* 93 */	0x0093,	//	<control>
/* 94 */	0x0094,	//	<control>
/* 95 */	0x0095,	//	<control>
/* 96 */	0x0096,	//	<control>
/* 97 */	0x0097,	//	<control>
/* 98 */	0x0098,	//	<control>
/* 99 */	0x0099,	//	<control>
/* 9A */	0x009A,	//	<control>
/* 9B */	0x009B,	//	<control>
/* 9C */	0x009C,	//	<control>
/* 9D */	0x009D,	//	<control>
/* 9E */	0x009E,	//	<control>
/* 9F */	0x009F,	//	<control>
/* A0 */	0x00A0,	//	NO-BREAK SPACE
/* A1 */	0x00A1,	//	INVERTED EXCLAMATION MARK
/* A2 */	0x00A2,	//	CENT SIGN
/* A3 */	0x00A3,	//	POUND SIGN
/* A4 */	0x00A4,	//	CURRENCY SIGN
/* A5 */	0x00A5,	//	YEN SIGN
/* A6 */	0x00A6,	//	BROKEN BAR
/* A7 */	0x00A7,	//	SECTION SIGN
/* A8 */	0x00A8,	//	DIAERESIS
/* A9 */	0x00A9,	//	COPYRIGHT SIGN
/* AA */	0x00AA,	//	FEMININE ORDINAL INDICATOR
/* AB */	0x00AB,	//	LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
/* AC */	0x00AC,	//	NOT SIGN
/* AD */	0x00AD,	//	SOFT HYPHEN
/* AE */	0x00AE,	//	REGISTERED SIGN
/* AF */	0x00AF,	//	MACRON
/* B0 */	0x00B0,	//	DEGREE SIGN
/* B1 */	0x00B1,	//	PLUS-MINUS SIGN
/* B2 */	0x00B2,	//	SUPERSCRIPT TWO
/* B3 */	0x00B3,	//	SUPERSCRIPT THREE
/* B4 */	0x00B4,	//	ACUTE ACCENT
/* B5 */	0x00B5,	//	MICRO SIGN
/* B6 */	0x00B6,	//	PILCROW SIGN
/* B7 */	0x00B7,	//	MIDDLE DOT
/* B8 */	0x00B8,	//	CEDILLA
/* B9 */	0x00B9,	//	SUPERSCRIPT ONE
/* BA */	0x00BA,	//	MASCULINE ORDINAL INDICATOR
/* BB */	0x00BB,	//	RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
/* BC */	0x00BC,	//	VULGAR FRACTION ONE QUARTER
/* BD */	0x00BD,	//	VULGAR FRACTION ONE HALF
/* BE */	0x00BE,	//	VULGAR FRACTION THREE QUARTERS
/* BF */	0x00BF,	//	INVERTED QUESTION MARK
/* C0 */	0x00C0,	//	LATIN CAPITAL LETTER A WITH GRAVE
/* C1 */	0x00C1,	//	LATIN CAPITAL LETTER A WITH ACUTE
/* C2 */	0x00C2,	//	LATIN CAPITAL LETTER A WITH CIRCUMFLEX
/* C3 */	0x00C3,	//	LATIN CAPITAL LETTER A WITH TILDE
/* C4 */	0x00C4,	//	LATIN CAPITAL LETTER A WITH DIAERESIS
/* C5 */	0x00C5,	//	LATIN CAPITAL LETTER A WITH RING ABOVE
/* C6 */	0x00C6,	//	LATIN CAPITAL LETTER AE
/* C7 */	0x00C7,	//	LATIN CAPITAL LETTER C WITH CEDILLA
/* C8 */	0x00C8,	//	LATIN CAPITAL LETTER E WITH GRAVE
/* C9 */	0x00C9,	//	LATIN CAPITAL LETTER E WITH ACUTE
/* CA */	0x00CA,	//	LATIN CAPITAL LETTER E WITH CIRCUMFLEX
/* CB */	0x00CB,	//	LATIN CAPITAL LETTER E WITH DIAERESIS
/* CC */	0x00CC,	//	LATIN CAPITAL LETTER I WITH GRAVE
/* CD */	0x00CD,	//	LATIN CAPITAL LETTER I WITH ACUTE
/* CE */	0x00CE,	//	LATIN CAPITAL LETTER I WITH CIRCUMFLEX
/* CF */	0x00CF,	//	LATIN CAPITAL LETTER I WITH DIAERESIS
/* D0 */	0x011E,	//	LATIN CAPITAL LETTER G WITH BREVE
/* D1 */	0x00D1,	//	LATIN CAPITAL LETTER N WITH TILDE
/* D2 */	0x00D2,	//	LATIN CAPITAL LETTER O WITH GRAVE
/* D3 */	0x00D3,	//	LATIN CAPITAL LETTER O WITH ACUTE
/* D4 */	0x00D4,	//	LATIN CAPITAL LETTER O WITH CIRCUMFLEX
/* D5 */	0x00D5,	//	LATIN CAPITAL LETTER O WITH TILDE
/* D6 */	0x00D6,	//	LATIN CAPITAL LETTER O WITH DIAERESIS
/* D7 */	0x00D7,	//	MULTIPLICATION SIGN
/* D8 */	0x00D8,	//	LATIN CAPITAL LETTER O WITH STROKE
/* D9 */	0x00D9,	//	LATIN CAPITAL LETTER U WITH GRAVE
/* DA */	0x00DA,	//	LATIN CAPITAL LETTER U WITH ACUTE
/* DB */	0x00DB,	//	LATIN CAPITAL LETTER U WITH CIRCUMFLEX
/* DC */	0x00DC,	//	LATIN CAPITAL LETTER U WITH DIAERESIS
/* DD */	0x0130,	//	LATIN CAPITAL LETTER I WITH DOT ABOVE
/* DE */	0x015E,	//	LATIN CAPITAL LETTER S WITH CEDILLA
/* DF */	0x00DF,	//	LATIN SMALL LETTER SHARP S
/* E0 */	0x00E0,	//	LATIN SMALL LETTER A WITH GRAVE
/* E1 */	0x00E1,	//	LATIN SMALL LETTER A WITH ACUTE
/* E2 */	0x00E2,	//	LATIN SMALL LETTER A WITH CIRCUMFLEX
/* E3 */	0x00E3,	//	LATIN SMALL LETTER A WITH TILDE
/* E4 */	0x00E4,	//	LATIN SMALL LETTER A WITH DIAERESIS
/* E5 */	0x00E5,	//	LATIN SMALL LETTER A WITH RING ABOVE
/* E6 */	0x00E6,	//	LATIN SMALL LETTER AE
/* E7 */	0x00E7,	//	LATIN SMALL LETTER C WITH CEDILLA
/* E8 */	0x00E8,	//	LATIN SMALL LETTER E WITH GRAVE
/* E9 */	0x00E9,	//	LATIN SMALL LETTER E WITH ACUTE
/* EA */	0x00EA,	//	LATIN SMALL LETTER E WITH CIRCUMFLEX
/* EB */	0x00EB,	//	LATIN SMALL LETTER E WITH DIAERESIS
/* EC */	0x00EC,	//	LATIN SMALL LETTER I WITH GRAVE
/* ED */	0x00ED,	//	LATIN SMALL LETTER I WITH ACUTE
/* EE */	0x00EE,	//	LATIN SMALL LETTER I WITH CIRCUMFLEX
/* EF */	0x00EF,	//	LATIN SMALL LETTER I WITH DIAERESIS
/* F0 */	0x011F,	//	LATIN SMALL LETTER G WITH BREVE
/* F1 */	0x00F1,	//	LATIN SMALL LETTER N WITH TILDE
/* F2 */	0x00F2,	//	LATIN SMALL LETTER O WITH GRAVE
/* F3 */	0x00F3,	//	LATIN SMALL LETTER O WITH ACUTE
/* F4 */	0x00F4,	//	LATIN SMALL LETTER O WITH CIRCUMFLEX
/* F5 */	0x00F5,	//	LATIN SMALL LETTER O WITH TILDE
/* F6 */	0x00F6,	//	LATIN SMALL LETTER O WITH DIAERESIS
/* F7 */	0x00F7,	//	DIVISION SIGN
/* F8 */	0x00F8,	//	LATIN SMALL LETTER O WITH STROKE
/* F9 */	0x00F9,	//	LATIN SMALL LETTER U WITH GRAVE
/* FA */	0x00FA,	//	LATIN SMALL LETTER U WITH ACUTE
/* FB */	0x00FB,	//	LATIN SMALL LETTER U WITH CIRCUMFLEX
/* FC */	0x00FC,	//	LATIN SMALL LETTER U WITH DIAERESIS
/* FD */	0x0131,	//	LATIN SMALL LETTER DOTLESS I
/* FE */	0x015F,	//	LATIN SMALL LETTER S WITH CEDILLA
/* FF */	0x00FF	//	LATIN SMALL LETTER Y WITH DIAERESIS
};

/***************************************************************************
#
#	Name:             ISO/IEC 8859-10:1998 to Unicode
#	Unicode version:  3.0
#	Table version:    1.1
#	Table format:     Format A
#	Date:             1999 October 11
#	Authors:          Ken Whistler <kenw@sybase.com>
#
#	Copyright (c) 1999 Unicode, Inc.  All Rights reserved.
#
#	This file is provided as-is by Unicode, Inc. (The Unicode Consortium).
#	No claims are made as to fitness for any particular purpose.  No
#	warranties of any kind are expressed or implied.  The recipient
#	agrees to determine applicability of information provided.  If this
#	file has been provided on optical media by Unicode, Inc., the sole
#	remedy for any claim will be exchange of defective media within 90
#	days of receipt.
#
#	Unicode, Inc. hereby grants the right to freely use the information
#	supplied in this file in the creation of products supporting the
#	Unicode Standard, and to make copies of this file in any form for
#	internal or external distribution as long as this notice remains
#	attached.
#
#	General notes:
#
#	This table contains the data the Unicode Consortium has on how
#       ISO/IEC 8859-10:1998 characters map into Unicode.
#
#	Format:  Three tab-separated columns
#		 Column #1 is the ISO/IEC 8859-10 code (in hex as 0xXX)
#		 Column #2 is the Unicode (in hex as 0xXXXX)
#		 Column #3 the Unicode name (follows a comment sign, '#')
#
#	The entries are in ISO/IEC 8859-10 order.
#
#	Version history
#	1.0 version new.
#       1.1 corrected mistake in mapping of 0xA4
#
#	Updated versions of this file may be found in:
#		<ftp://ftp.unicode.org/Public/MAPPINGS/>
#
#	Any comments or problems, contact <errata@unicode.org>
#	Please note that <errata@unicode.org> is an archival address;
#	notices will be checked, but do not expect an immediate response.
#
***************************************************************************/
static const unicode_char lookup_iso8859_10[256] = {
/* 00 */	0x0000,	//	NULL
/* 01 */	0x0001,	//	START OF HEADING
/* 02 */	0x0002,	//	START OF TEXT
/* 03 */	0x0003,	//	END OF TEXT
/* 04 */	0x0004,	//	END OF TRANSMISSION
/* 05 */	0x0005,	//	ENQUIRY
/* 06 */	0x0006,	//	ACKNOWLEDGE
/* 07 */	0x0007,	//	BELL
/* 08 */	0x0008,	//	BACKSPACE
/* 09 */	0x0009,	//	HORIZONTAL TABULATION
/* 0A */	0x000A,	//	LINE FEED
/* 0B */	0x000B,	//	VERTICAL TABULATION
/* 0C */	0x000C,	//	FORM FEED
/* 0D */	0x000D,	//	CARRIAGE RETURN
/* 0E */	0x000E,	//	SHIFT OUT
/* 0F */	0x000F,	//	SHIFT IN
/* 10 */	0x0010,	//	DATA LINK ESCAPE
/* 11 */	0x0011,	//	DEVICE CONTROL ONE
/* 12 */	0x0012,	//	DEVICE CONTROL TWO
/* 13 */	0x0013,	//	DEVICE CONTROL THREE
/* 14 */	0x0014,	//	DEVICE CONTROL FOUR
/* 15 */	0x0015,	//	NEGATIVE ACKNOWLEDGE
/* 16 */	0x0016,	//	SYNCHRONOUS IDLE
/* 17 */	0x0017,	//	END OF TRANSMISSION BLOCK
/* 18 */	0x0018,	//	CANCEL
/* 19 */	0x0019,	//	END OF MEDIUM
/* 1A */	0x001A,	//	SUBSTITUTE
/* 1B */	0x001B,	//	ESCAPE
/* 1C */	0x001C,	//	FILE SEPARATOR
/* 1D */	0x001D,	//	GROUP SEPARATOR
/* 1E */	0x001E,	//	RECORD SEPARATOR
/* 1F */	0x001F,	//	UNIT SEPARATOR
/* 20 */	0x0020,	//	SPACE
/* 21 */	0x0021,	//	EXCLAMATION MARK
/* 22 */	0x0022,	//	QUOTATION MARK
/* 23 */	0x0023,	//	NUMBER SIGN
/* 24 */	0x0024,	//	DOLLAR SIGN
/* 25 */	0x0025,	//	PERCENT SIGN
/* 26 */	0x0026,	//	AMPERSAND
/* 27 */	0x0027,	//	APOSTROPHE
/* 28 */	0x0028,	//	LEFT PARENTHESIS
/* 29 */	0x0029,	//	RIGHT PARENTHESIS
/* 2A */	0x002A,	//	ASTERISK
/* 2B */	0x002B,	//	PLUS SIGN
/* 2C */	0x002C,	//	COMMA
/* 2D */	0x002D,	//	HYPHEN-MINUS
/* 2E */	0x002E,	//	FULL STOP
/* 2F */	0x002F,	//	SOLIDUS
/* 30 */	0x0030,	//	DIGIT ZERO
/* 31 */	0x0031,	//	DIGIT ONE
/* 32 */	0x0032,	//	DIGIT TWO
/* 33 */	0x0033,	//	DIGIT THREE
/* 34 */	0x0034,	//	DIGIT FOUR
/* 35 */	0x0035,	//	DIGIT FIVE
/* 36 */	0x0036,	//	DIGIT SIX
/* 37 */	0x0037,	//	DIGIT SEVEN
/* 38 */	0x0038,	//	DIGIT EIGHT
/* 39 */	0x0039,	//	DIGIT NINE
/* 3A */	0x003A,	//	COLON
/* 3B */	0x003B,	//	SEMICOLON
/* 3C */	0x003C,	//	LESS-THAN SIGN
/* 3D */	0x003D,	//	EQUALS SIGN
/* 3E */	0x003E,	//	GREATER-THAN SIGN
/* 3F */	0x003F,	//	QUESTION MARK
/* 40 */	0x0040,	//	COMMERCIAL AT
/* 41 */	0x0041,	//	LATIN CAPITAL LETTER A
/* 42 */	0x0042,	//	LATIN CAPITAL LETTER B
/* 43 */	0x0043,	//	LATIN CAPITAL LETTER C
/* 44 */	0x0044,	//	LATIN CAPITAL LETTER D
/* 45 */	0x0045,	//	LATIN CAPITAL LETTER E
/* 46 */	0x0046,	//	LATIN CAPITAL LETTER F
/* 47 */	0x0047,	//	LATIN CAPITAL LETTER G
/* 48 */	0x0048,	//	LATIN CAPITAL LETTER H
/* 49 */	0x0049,	//	LATIN CAPITAL LETTER I
/* 4A */	0x004A,	//	LATIN CAPITAL LETTER J
/* 4B */	0x004B,	//	LATIN CAPITAL LETTER K
/* 4C */	0x004C,	//	LATIN CAPITAL LETTER L
/* 4D */	0x004D,	//	LATIN CAPITAL LETTER M
/* 4E */	0x004E,	//	LATIN CAPITAL LETTER N
/* 4F */	0x004F,	//	LATIN CAPITAL LETTER O
/* 50 */	0x0050,	//	LATIN CAPITAL LETTER P
/* 51 */	0x0051,	//	LATIN CAPITAL LETTER Q
/* 52 */	0x0052,	//	LATIN CAPITAL LETTER R
/* 53 */	0x0053,	//	LATIN CAPITAL LETTER S
/* 54 */	0x0054,	//	LATIN CAPITAL LETTER T
/* 55 */	0x0055,	//	LATIN CAPITAL LETTER U
/* 56 */	0x0056,	//	LATIN CAPITAL LETTER V
/* 57 */	0x0057,	//	LATIN CAPITAL LETTER W
/* 58 */	0x0058,	//	LATIN CAPITAL LETTER X
/* 59 */	0x0059,	//	LATIN CAPITAL LETTER Y
/* 5A */	0x005A,	//	LATIN CAPITAL LETTER Z
/* 5B */	0x005B,	//	LEFT SQUARE BRACKET
/* 5C */	0x005C,	//	REVERSE SOLIDUS
/* 5D */	0x005D,	//	RIGHT SQUARE BRACKET
/* 5E */	0x005E,	//	CIRCUMFLEX ACCENT
/* 5F */	0x005F,	//	LOW LINE
/* 60 */	0x0060,	//	GRAVE ACCENT
/* 61 */	0x0061,	//	LATIN SMALL LETTER A
/* 62 */	0x0062,	//	LATIN SMALL LETTER B
/* 63 */	0x0063,	//	LATIN SMALL LETTER C
/* 64 */	0x0064,	//	LATIN SMALL LETTER D
/* 65 */	0x0065,	//	LATIN SMALL LETTER E
/* 66 */	0x0066,	//	LATIN SMALL LETTER F
/* 67 */	0x0067,	//	LATIN SMALL LETTER G
/* 68 */	0x0068,	//	LATIN SMALL LETTER H
/* 69 */	0x0069,	//	LATIN SMALL LETTER I
/* 6A */	0x006A,	//	LATIN SMALL LETTER J
/* 6B */	0x006B,	//	LATIN SMALL LETTER K
/* 6C */	0x006C,	//	LATIN SMALL LETTER L
/* 6D */	0x006D,	//	LATIN SMALL LETTER M
/* 6E */	0x006E,	//	LATIN SMALL LETTER N
/* 6F */	0x006F,	//	LATIN SMALL LETTER O
/* 70 */	0x0070,	//	LATIN SMALL LETTER P
/* 71 */	0x0071,	//	LATIN SMALL LETTER Q
/* 72 */	0x0072,	//	LATIN SMALL LETTER R
/* 73 */	0x0073,	//	LATIN SMALL LETTER S
/* 74 */	0x0074,	//	LATIN SMALL LETTER T
/* 75 */	0x0075,	//	LATIN SMALL LETTER U
/* 76 */	0x0076,	//	LATIN SMALL LETTER V
/* 77 */	0x0077,	//	LATIN SMALL LETTER W
/* 78 */	0x0078,	//	LATIN SMALL LETTER X
/* 79 */	0x0079,	//	LATIN SMALL LETTER Y
/* 7A */	0x007A,	//	LATIN SMALL LETTER Z
/* 7B */	0x007B,	//	LEFT CURLY BRACKET
/* 7C */	0x007C,	//	VERTICAL LINE
/* 7D */	0x007D,	//	RIGHT CURLY BRACKET
/* 7E */	0x007E,	//	TILDE
/* 7F */	0x007F,	//	DELETE
/* 80 */	0x0080,	//	<control>
/* 81 */	0x0081,	//	<control>
/* 82 */	0x0082,	//	<control>
/* 83 */	0x0083,	//	<control>
/* 84 */	0x0084,	//	<control>
/* 85 */	0x0085,	//	<control>
/* 86 */	0x0086,	//	<control>
/* 87 */	0x0087,	//	<control>
/* 88 */	0x0088,	//	<control>
/* 89 */	0x0089,	//	<control>
/* 8A */	0x008A,	//	<control>
/* 8B */	0x008B,	//	<control>
/* 8C */	0x008C,	//	<control>
/* 8D */	0x008D,	//	<control>
/* 8E */	0x008E,	//	<control>
/* 8F */	0x008F,	//	<control>
/* 90 */	0x0090,	//	<control>
/* 91 */	0x0091,	//	<control>
/* 92 */	0x0092,	//	<control>
/* 93 */	0x0093,	//	<control>
/* 94 */	0x0094,	//	<control>
/* 95 */	0x0095,	//	<control>
/* 96 */	0x0096,	//	<control>
/* 97 */	0x0097,	//	<control>
/* 98 */	0x0098,	//	<control>
/* 99 */	0x0099,	//	<control>
/* 9A */	0x009A,	//	<control>
/* 9B */	0x009B,	//	<control>
/* 9C */	0x009C,	//	<control>
/* 9D */	0x009D,	//	<control>
/* 9E */	0x009E,	//	<control>
/* 9F */	0x009F,	//	<control>
/* A0 */	0x00A0,	//	NO-BREAK SPACE
/* A1 */	0x0104,	//	LATIN CAPITAL LETTER A WITH OGONEK
/* A2 */	0x0112,	//	LATIN CAPITAL LETTER E WITH MACRON
/* A3 */	0x0122,	//	LATIN CAPITAL LETTER G WITH CEDILLA
/* A4 */	0x012A,	//	LATIN CAPITAL LETTER I WITH MACRON
/* A5 */	0x0128,	//	LATIN CAPITAL LETTER I WITH TILDE
/* A6 */	0x0136,	//	LATIN CAPITAL LETTER K WITH CEDILLA
/* A7 */	0x00A7,	//	SECTION SIGN
/* A8 */	0x013B,	//	LATIN CAPITAL LETTER L WITH CEDILLA
/* A9 */	0x0110,	//	LATIN CAPITAL LETTER D WITH STROKE
/* AA */	0x0160,	//	LATIN CAPITAL LETTER S WITH CARON
/* AB */	0x0166,	//	LATIN CAPITAL LETTER T WITH STROKE
/* AC */	0x017D,	//	LATIN CAPITAL LETTER Z WITH CARON
/* AD */	0x00AD,	//	SOFT HYPHEN
/* AE */	0x016A,	//	LATIN CAPITAL LETTER U WITH MACRON
/* AF */	0x014A,	//	LATIN CAPITAL LETTER ENG
/* B0 */	0x00B0,	//	DEGREE SIGN
/* B1 */	0x0105,	//	LATIN SMALL LETTER A WITH OGONEK
/* B2 */	0x0113,	//	LATIN SMALL LETTER E WITH MACRON
/* B3 */	0x0123,	//	LATIN SMALL LETTER G WITH CEDILLA
/* B4 */	0x012B,	//	LATIN SMALL LETTER I WITH MACRON
/* B5 */	0x0129,	//	LATIN SMALL LETTER I WITH TILDE
/* B6 */	0x0137,	//	LATIN SMALL LETTER K WITH CEDILLA
/* B7 */	0x00B7,	//	MIDDLE DOT
/* B8 */	0x013C,	//	LATIN SMALL LETTER L WITH CEDILLA
/* B9 */	0x0111,	//	LATIN SMALL LETTER D WITH STROKE
/* BA */	0x0161,	//	LATIN SMALL LETTER S WITH CARON
/* BB */	0x0167,	//	LATIN SMALL LETTER T WITH STROKE
/* BC */	0x017E,	//	LATIN SMALL LETTER Z WITH CARON
/* BD */	0x2015,	//	HORIZONTAL BAR
/* BE */	0x016B,	//	LATIN SMALL LETTER U WITH MACRON
/* BF */	0x014B,	//	LATIN SMALL LETTER ENG
/* C0 */	0x0100,	//	LATIN CAPITAL LETTER A WITH MACRON
/* C1 */	0x00C1,	//	LATIN CAPITAL LETTER A WITH ACUTE
/* C2 */	0x00C2,	//	LATIN CAPITAL LETTER A WITH CIRCUMFLEX
/* C3 */	0x00C3,	//	LATIN CAPITAL LETTER A WITH TILDE
/* C4 */	0x00C4,	//	LATIN CAPITAL LETTER A WITH DIAERESIS
/* C5 */	0x00C5,	//	LATIN CAPITAL LETTER A WITH RING ABOVE
/* C6 */	0x00C6,	//	LATIN CAPITAL LETTER AE
/* C7 */	0x012E,	//	LATIN CAPITAL LETTER I WITH OGONEK
/* C8 */	0x010C,	//	LATIN CAPITAL LETTER C WITH CARON
/* C9 */	0x00C9,	//	LATIN CAPITAL LETTER E WITH ACUTE
/* CA */	0x0118,	//	LATIN CAPITAL LETTER E WITH OGONEK
/* CB */	0x00CB,	//	LATIN CAPITAL LETTER E WITH DIAERESIS
/* CC */	0x0116,	//	LATIN CAPITAL LETTER E WITH DOT ABOVE
/* CD */	0x00CD,	//	LATIN CAPITAL LETTER I WITH ACUTE
/* CE */	0x00CE,	//	LATIN CAPITAL LETTER I WITH CIRCUMFLEX
/* CF */	0x00CF,	//	LATIN CAPITAL LETTER I WITH DIAERESIS
/* D0 */	0x00D0,	//	LATIN CAPITAL LETTER ETH (Icelandic)
/* D1 */	0x0145,	//	LATIN CAPITAL LETTER N WITH CEDILLA
/* D2 */	0x014C,	//	LATIN CAPITAL LETTER O WITH MACRON
/* D3 */	0x00D3,	//	LATIN CAPITAL LETTER O WITH ACUTE
/* D4 */	0x00D4,	//	LATIN CAPITAL LETTER O WITH CIRCUMFLEX
/* D5 */	0x00D5,	//	LATIN CAPITAL LETTER O WITH TILDE
/* D6 */	0x00D6,	//	LATIN CAPITAL LETTER O WITH DIAERESIS
/* D7 */	0x0168,	//	LATIN CAPITAL LETTER U WITH TILDE
/* D8 */	0x00D8,	//	LATIN CAPITAL LETTER O WITH STROKE
/* D9 */	0x0172,	//	LATIN CAPITAL LETTER U WITH OGONEK
/* DA */	0x00DA,	//	LATIN CAPITAL LETTER U WITH ACUTE
/* DB */	0x00DB,	//	LATIN CAPITAL LETTER U WITH CIRCUMFLEX
/* DC */	0x00DC,	//	LATIN CAPITAL LETTER U WITH DIAERESIS
/* DD */	0x00DD,	//	LATIN CAPITAL LETTER Y WITH ACUTE
/* DE */	0x00DE,	//	LATIN CAPITAL LETTER THORN (Icelandic)
/* DF */	0x00DF,	//	LATIN SMALL LETTER SHARP S (German)
/* E0 */	0x0101,	//	LATIN SMALL LETTER A WITH MACRON
/* E1 */	0x00E1,	//	LATIN SMALL LETTER A WITH ACUTE
/* E2 */	0x00E2,	//	LATIN SMALL LETTER A WITH CIRCUMFLEX
/* E3 */	0x00E3,	//	LATIN SMALL LETTER A WITH TILDE
/* E4 */	0x00E4,	//	LATIN SMALL LETTER A WITH DIAERESIS
/* E5 */	0x00E5,	//	LATIN SMALL LETTER A WITH RING ABOVE
/* E6 */	0x00E6,	//	LATIN SMALL LETTER AE
/* E7 */	0x012F,	//	LATIN SMALL LETTER I WITH OGONEK
/* E8 */	0x010D,	//	LATIN SMALL LETTER C WITH CARON
/* E9 */	0x00E9,	//	LATIN SMALL LETTER E WITH ACUTE
/* EA */	0x0119,	//	LATIN SMALL LETTER E WITH OGONEK
/* EB */	0x00EB,	//	LATIN SMALL LETTER E WITH DIAERESIS
/* EC */	0x0117,	//	LATIN SMALL LETTER E WITH DOT ABOVE
/* ED */	0x00ED,	//	LATIN SMALL LETTER I WITH ACUTE
/* EE */	0x00EE,	//	LATIN SMALL LETTER I WITH CIRCUMFLEX
/* EF */	0x00EF,	//	LATIN SMALL LETTER I WITH DIAERESIS
/* F0 */	0x00F0,	//	LATIN SMALL LETTER ETH (Icelandic)
/* F1 */	0x0146,	//	LATIN SMALL LETTER N WITH CEDILLA
/* F2 */	0x014D,	//	LATIN SMALL LETTER O WITH MACRON
/* F3 */	0x00F3,	//	LATIN SMALL LETTER O WITH ACUTE
/* F4 */	0x00F4,	//	LATIN SMALL LETTER O WITH CIRCUMFLEX
/* F5 */	0x00F5,	//	LATIN SMALL LETTER O WITH TILDE
/* F6 */	0x00F6,	//	LATIN SMALL LETTER O WITH DIAERESIS
/* F7 */	0x0169,	//	LATIN SMALL LETTER U WITH TILDE
/* F8 */	0x00F8,	//	LATIN SMALL LETTER O WITH STROKE
/* F9 */	0x0173,	//	LATIN SMALL LETTER U WITH OGONEK
/* FA */	0x00FA,	//	LATIN SMALL LETTER U WITH ACUTE
/* FB */	0x00FB,	//	LATIN SMALL LETTER U WITH CIRCUMFLEX
/* FC */	0x00FC,	//	LATIN SMALL LETTER U WITH DIAERESIS
/* FD */	0x00FD,	//	LATIN SMALL LETTER Y WITH ACUTE
/* FE */	0x00FE,	//	LATIN SMALL LETTER THORN (Icelandic)
/* FF */	0x0138	//	LATIN SMALL LETTER KRA
};

/***************************************************************************
#
#	Name:             ISO/IEC 8859-11:2001 to Unicode
#	Unicode version:  3.2
#	Table version:    1.0
#	Table format:     Format A
#	Date:             2002 October 7
#	Authors:          Ken Whistler <kenw@sybase.com>
#
#	Copyright (c) 2002 Unicode, Inc.  All Rights reserved.
#
#	This file is provided as-is by Unicode, Inc. (The Unicode Consortium).
#	No claims are made as to fitness for any particular purpose.  No
#	warranties of any kind are expressed or implied.  The recipient
#	agrees to determine applicability of information provided.  If this
#	file has been provided on optical media by Unicode, Inc., the sole
#	remedy for any claim will be exchange of defective media within 90
#	days of receipt.
#
#	Unicode, Inc. hereby grants the right to freely use the information
#	supplied in this file in the creation of products supporting the
#	Unicode Standard, and to make copies of this file in any form for
#	internal or external distribution as long as this notice remains
#	attached.
#
#	General notes:
#
#	This table contains the data the Unicode Consortium has on how
#       ISO/IEC 8859-11:2001 characters map into Unicode.
#
#	ISO/IEC 8859-11:2001 is equivalent to TIS 620-2533 (1990) with
#	the addition of 0xA0 NO-BREAK SPACE.
#
#	Format:  Three tab-separated columns
#		 Column #1 is the ISO/IEC 8859-11 code (in hex as 0xXX)
#		 Column #2 is the Unicode (in hex as 0xXXXX)
#		 Column #3 the Unicode name (follows a comment sign, '#')
#
#	The entries are in ISO/IEC 8859-11 order.
#
#	Version history:
#		2002 October 7  Created
#
#	Updated versions of this file may be found in:
#		<ftp://ftp.unicode.org/Public/MAPPINGS/>
#
#	For any comments or problems, please use the Unicode
#	web contact form at:
#		http://www.unicode.org/unicode/reporting.html
#
***************************************************************************/
static const unicode_char lookup_iso8859_11[256] = {
/* 00 */	0x0000,	//	NULL
/* 01 */	0x0001,	//	START OF HEADING
/* 02 */	0x0002,	//	START OF TEXT
/* 03 */	0x0003,	//	END OF TEXT
/* 04 */	0x0004,	//	END OF TRANSMISSION
/* 05 */	0x0005,	//	ENQUIRY
/* 06 */	0x0006,	//	ACKNOWLEDGE
/* 07 */	0x0007,	//	BELL
/* 08 */	0x0008,	//	BACKSPACE
/* 09 */	0x0009,	//	HORIZONTAL TABULATION
/* 0A */	0x000A,	//	LINE FEED
/* 0B */	0x000B,	//	VERTICAL TABULATION
/* 0C */	0x000C,	//	FORM FEED
/* 0D */	0x000D,	//	CARRIAGE RETURN
/* 0E */	0x000E,	//	SHIFT OUT
/* 0F */	0x000F,	//	SHIFT IN
/* 10 */	0x0010,	//	DATA LINK ESCAPE
/* 11 */	0x0011,	//	DEVICE CONTROL ONE
/* 12 */	0x0012,	//	DEVICE CONTROL TWO
/* 13 */	0x0013,	//	DEVICE CONTROL THREE
/* 14 */	0x0014,	//	DEVICE CONTROL FOUR
/* 15 */	0x0015,	//	NEGATIVE ACKNOWLEDGE
/* 16 */	0x0016,	//	SYNCHRONOUS IDLE
/* 17 */	0x0017,	//	END OF TRANSMISSION BLOCK
/* 18 */	0x0018,	//	CANCEL
/* 19 */	0x0019,	//	END OF MEDIUM
/* 1A */	0x001A,	//	SUBSTITUTE
/* 1B */	0x001B,	//	ESCAPE
/* 1C */	0x001C,	//	FILE SEPARATOR
/* 1D */	0x001D,	//	GROUP SEPARATOR
/* 1E */	0x001E,	//	RECORD SEPARATOR
/* 1F */	0x001F,	//	UNIT SEPARATOR
/* 20 */	0x0020,	//	SPACE
/* 21 */	0x0021,	//	EXCLAMATION MARK
/* 22 */	0x0022,	//	QUOTATION MARK
/* 23 */	0x0023,	//	NUMBER SIGN
/* 24 */	0x0024,	//	DOLLAR SIGN
/* 25 */	0x0025,	//	PERCENT SIGN
/* 26 */	0x0026,	//	AMPERSAND
/* 27 */	0x0027,	//	APOSTROPHE
/* 28 */	0x0028,	//	LEFT PARENTHESIS
/* 29 */	0x0029,	//	RIGHT PARENTHESIS
/* 2A */	0x002A,	//	ASTERISK
/* 2B */	0x002B,	//	PLUS SIGN
/* 2C */	0x002C,	//	COMMA
/* 2D */	0x002D,	//	HYPHEN-MINUS
/* 2E */	0x002E,	//	FULL STOP
/* 2F */	0x002F,	//	SOLIDUS
/* 30 */	0x0030,	//	DIGIT ZERO
/* 31 */	0x0031,	//	DIGIT ONE
/* 32 */	0x0032,	//	DIGIT TWO
/* 33 */	0x0033,	//	DIGIT THREE
/* 34 */	0x0034,	//	DIGIT FOUR
/* 35 */	0x0035,	//	DIGIT FIVE
/* 36 */	0x0036,	//	DIGIT SIX
/* 37 */	0x0037,	//	DIGIT SEVEN
/* 38 */	0x0038,	//	DIGIT EIGHT
/* 39 */	0x0039,	//	DIGIT NINE
/* 3A */	0x003A,	//	COLON
/* 3B */	0x003B,	//	SEMICOLON
/* 3C */	0x003C,	//	LESS-THAN SIGN
/* 3D */	0x003D,	//	EQUALS SIGN
/* 3E */	0x003E,	//	GREATER-THAN SIGN
/* 3F */	0x003F,	//	QUESTION MARK
/* 40 */	0x0040,	//	COMMERCIAL AT
/* 41 */	0x0041,	//	LATIN CAPITAL LETTER A
/* 42 */	0x0042,	//	LATIN CAPITAL LETTER B
/* 43 */	0x0043,	//	LATIN CAPITAL LETTER C
/* 44 */	0x0044,	//	LATIN CAPITAL LETTER D
/* 45 */	0x0045,	//	LATIN CAPITAL LETTER E
/* 46 */	0x0046,	//	LATIN CAPITAL LETTER F
/* 47 */	0x0047,	//	LATIN CAPITAL LETTER G
/* 48 */	0x0048,	//	LATIN CAPITAL LETTER H
/* 49 */	0x0049,	//	LATIN CAPITAL LETTER I
/* 4A */	0x004A,	//	LATIN CAPITAL LETTER J
/* 4B */	0x004B,	//	LATIN CAPITAL LETTER K
/* 4C */	0x004C,	//	LATIN CAPITAL LETTER L
/* 4D */	0x004D,	//	LATIN CAPITAL LETTER M
/* 4E */	0x004E,	//	LATIN CAPITAL LETTER N
/* 4F */	0x004F,	//	LATIN CAPITAL LETTER O
/* 50 */	0x0050,	//	LATIN CAPITAL LETTER P
/* 51 */	0x0051,	//	LATIN CAPITAL LETTER Q
/* 52 */	0x0052,	//	LATIN CAPITAL LETTER R
/* 53 */	0x0053,	//	LATIN CAPITAL LETTER S
/* 54 */	0x0054,	//	LATIN CAPITAL LETTER T
/* 55 */	0x0055,	//	LATIN CAPITAL LETTER U
/* 56 */	0x0056,	//	LATIN CAPITAL LETTER V
/* 57 */	0x0057,	//	LATIN CAPITAL LETTER W
/* 58 */	0x0058,	//	LATIN CAPITAL LETTER X
/* 59 */	0x0059,	//	LATIN CAPITAL LETTER Y
/* 5A */	0x005A,	//	LATIN CAPITAL LETTER Z
/* 5B */	0x005B,	//	LEFT SQUARE BRACKET
/* 5C */	0x005C,	//	REVERSE SOLIDUS
/* 5D */	0x005D,	//	RIGHT SQUARE BRACKET
/* 5E */	0x005E,	//	CIRCUMFLEX ACCENT
/* 5F */	0x005F,	//	LOW LINE
/* 60 */	0x0060,	//	GRAVE ACCENT
/* 61 */	0x0061,	//	LATIN SMALL LETTER A
/* 62 */	0x0062,	//	LATIN SMALL LETTER B
/* 63 */	0x0063,	//	LATIN SMALL LETTER C
/* 64 */	0x0064,	//	LATIN SMALL LETTER D
/* 65 */	0x0065,	//	LATIN SMALL LETTER E
/* 66 */	0x0066,	//	LATIN SMALL LETTER F
/* 67 */	0x0067,	//	LATIN SMALL LETTER G
/* 68 */	0x0068,	//	LATIN SMALL LETTER H
/* 69 */	0x0069,	//	LATIN SMALL LETTER I
/* 6A */	0x006A,	//	LATIN SMALL LETTER J
/* 6B */	0x006B,	//	LATIN SMALL LETTER K
/* 6C */	0x006C,	//	LATIN SMALL LETTER L
/* 6D */	0x006D,	//	LATIN SMALL LETTER M
/* 6E */	0x006E,	//	LATIN SMALL LETTER N
/* 6F */	0x006F,	//	LATIN SMALL LETTER O
/* 70 */	0x0070,	//	LATIN SMALL LETTER P
/* 71 */	0x0071,	//	LATIN SMALL LETTER Q
/* 72 */	0x0072,	//	LATIN SMALL LETTER R
/* 73 */	0x0073,	//	LATIN SMALL LETTER S
/* 74 */	0x0074,	//	LATIN SMALL LETTER T
/* 75 */	0x0075,	//	LATIN SMALL LETTER U
/* 76 */	0x0076,	//	LATIN SMALL LETTER V
/* 77 */	0x0077,	//	LATIN SMALL LETTER W
/* 78 */	0x0078,	//	LATIN SMALL LETTER X
/* 79 */	0x0079,	//	LATIN SMALL LETTER Y
/* 7A */	0x007A,	//	LATIN SMALL LETTER Z
/* 7B */	0x007B,	//	LEFT CURLY BRACKET
/* 7C */	0x007C,	//	VERTICAL LINE
/* 7D */	0x007D,	//	RIGHT CURLY BRACKET
/* 7E */	0x007E,	//	TILDE
/* 7F */	0x007F,	//	DELETE
/* 80 */	0x0080,	//	<control>
/* 81 */	0x0081,	//	<control>
/* 82 */	0x0082,	//	<control>
/* 83 */	0x0083,	//	<control>
/* 84 */	0x0084,	//	<control>
/* 85 */	0x0085,	//	<control>
/* 86 */	0x0086,	//	<control>
/* 87 */	0x0087,	//	<control>
/* 88 */	0x0088,	//	<control>
/* 89 */	0x0089,	//	<control>
/* 8A */	0x008A,	//	<control>
/* 8B */	0x008B,	//	<control>
/* 8C */	0x008C,	//	<control>
/* 8D */	0x008D,	//	<control>
/* 8E */	0x008E,	//	<control>
/* 8F */	0x008F,	//	<control>
/* 90 */	0x0090,	//	<control>
/* 91 */	0x0091,	//	<control>
/* 92 */	0x0092,	//	<control>
/* 93 */	0x0093,	//	<control>
/* 94 */	0x0094,	//	<control>
/* 95 */	0x0095,	//	<control>
/* 96 */	0x0096,	//	<control>
/* 97 */	0x0097,	//	<control>
/* 98 */	0x0098,	//	<control>
/* 99 */	0x0099,	//	<control>
/* 9A */	0x009A,	//	<control>
/* 9B */	0x009B,	//	<control>
/* 9C */	0x009C,	//	<control>
/* 9D */	0x009D,	//	<control>
/* 9E */	0x009E,	//	<control>
/* 9F */	0x009F,	//	<control>
/* A0 */	0x00A0,	//	NO-BREAK SPACE
/* A1 */	0x0E01,	//	THAI CHARACTER KO KAI
/* A2 */	0x0E02,	//	THAI CHARACTER KHO KHAI
/* A3 */	0x0E03,	//	THAI CHARACTER KHO KHUAT
/* A4 */	0x0E04,	//	THAI CHARACTER KHO KHWAI
/* A5 */	0x0E05,	//	THAI CHARACTER KHO KHON
/* A6 */	0x0E06,	//	THAI CHARACTER KHO RAKHANG
/* A7 */	0x0E07,	//	THAI CHARACTER NGO NGU
/* A8 */	0x0E08,	//	THAI CHARACTER CHO CHAN
/* A9 */	0x0E09,	//	THAI CHARACTER CHO CHING
/* AA */	0x0E0A,	//	THAI CHARACTER CHO CHANG
/* AB */	0x0E0B,	//	THAI CHARACTER SO SO
/* AC */	0x0E0C,	//	THAI CHARACTER CHO CHOE
/* AD */	0x0E0D,	//	THAI CHARACTER YO YING
/* AE */	0x0E0E,	//	THAI CHARACTER DO CHADA
/* AF */	0x0E0F,	//	THAI CHARACTER TO PATAK
/* B0 */	0x0E10,	//	THAI CHARACTER THO THAN
/* B1 */	0x0E11,	//	THAI CHARACTER THO NANGMONTHO
/* B2 */	0x0E12,	//	THAI CHARACTER THO PHUTHAO
/* B3 */	0x0E13,	//	THAI CHARACTER NO NEN
/* B4 */	0x0E14,	//	THAI CHARACTER DO DEK
/* B5 */	0x0E15,	//	THAI CHARACTER TO TAO
/* B6 */	0x0E16,	//	THAI CHARACTER THO THUNG
/* B7 */	0x0E17,	//	THAI CHARACTER THO THAHAN
/* B8 */	0x0E18,	//	THAI CHARACTER THO THONG
/* B9 */	0x0E19,	//	THAI CHARACTER NO NU
/* BA */	0x0E1A,	//	THAI CHARACTER BO BAIMAI
/* BB */	0x0E1B,	//	THAI CHARACTER PO PLA
/* BC */	0x0E1C,	//	THAI CHARACTER PHO PHUNG
/* BD */	0x0E1D,	//	THAI CHARACTER FO FA
/* BE */	0x0E1E,	//	THAI CHARACTER PHO PHAN
/* BF */	0x0E1F,	//	THAI CHARACTER FO FAN
/* C0 */	0x0E20,	//	THAI CHARACTER PHO SAMPHAO
/* C1 */	0x0E21,	//	THAI CHARACTER MO MA
/* C2 */	0x0E22,	//	THAI CHARACTER YO YAK
/* C3 */	0x0E23,	//	THAI CHARACTER RO RUA
/* C4 */	0x0E24,	//	THAI CHARACTER RU
/* C5 */	0x0E25,	//	THAI CHARACTER LO LING
/* C6 */	0x0E26,	//	THAI CHARACTER LU
/* C7 */	0x0E27,	//	THAI CHARACTER WO WAEN
/* C8 */	0x0E28,	//	THAI CHARACTER SO SALA
/* C9 */	0x0E29,	//	THAI CHARACTER SO RUSI
/* CA */	0x0E2A,	//	THAI CHARACTER SO SUA
/* CB */	0x0E2B,	//	THAI CHARACTER HO HIP
/* CC */	0x0E2C,	//	THAI CHARACTER LO CHULA
/* CD */	0x0E2D,	//	THAI CHARACTER O ANG
/* CE */	0x0E2E,	//	THAI CHARACTER HO NOKHUK
/* CF */	0x0E2F,	//	THAI CHARACTER PAIYANNOI
/* D0 */	0x0E30,	//	THAI CHARACTER SARA A
/* D1 */	0x0E31,	//	THAI CHARACTER MAI HAN-AKAT
/* D2 */	0x0E32,	//	THAI CHARACTER SARA AA
/* D3 */	0x0E33,	//	THAI CHARACTER SARA AM
/* D4 */	0x0E34,	//	THAI CHARACTER SARA I
/* D5 */	0x0E35,	//	THAI CHARACTER SARA II
/* D6 */	0x0E36,	//	THAI CHARACTER SARA UE
/* D7 */	0x0E37,	//	THAI CHARACTER SARA UEE
/* D8 */	0x0E38,	//	THAI CHARACTER SARA U
/* D9 */	0x0E39,	//	THAI CHARACTER SARA UU
/* DA */	0x0E3A,	//	THAI CHARACTER PHINTHU
/* DF */	0x0E3F,	//	THAI CURRENCY SYMBOL BAHT
/* E0 */	0x0E40,	//	THAI CHARACTER SARA E
/* E1 */	0x0E41,	//	THAI CHARACTER SARA AE
/* E2 */	0x0E42,	//	THAI CHARACTER SARA O
/* E3 */	0x0E43,	//	THAI CHARACTER SARA AI MAIMUAN
/* E4 */	0x0E44,	//	THAI CHARACTER SARA AI MAIMALAI
/* E5 */	0x0E45,	//	THAI CHARACTER LAKKHANGYAO
/* E6 */	0x0E46,	//	THAI CHARACTER MAIYAMOK
/* E7 */	0x0E47,	//	THAI CHARACTER MAITAIKHU
/* E8 */	0x0E48,	//	THAI CHARACTER MAI EK
/* E9 */	0x0E49,	//	THAI CHARACTER MAI THO
/* EA */	0x0E4A,	//	THAI CHARACTER MAI TRI
/* EB */	0x0E4B,	//	THAI CHARACTER MAI CHATTAWA
/* EC */	0x0E4C,	//	THAI CHARACTER THANTHAKHAT
/* ED */	0x0E4D,	//	THAI CHARACTER NIKHAHIT
/* EE */	0x0E4E,	//	THAI CHARACTER YAMAKKAN
/* EF */	0x0E4F,	//	THAI CHARACTER FONGMAN
/* F0 */	0x0E50,	//	THAI DIGIT ZERO
/* F1 */	0x0E51,	//	THAI DIGIT ONE
/* F2 */	0x0E52,	//	THAI DIGIT TWO
/* F3 */	0x0E53,	//	THAI DIGIT THREE
/* F4 */	0x0E54,	//	THAI DIGIT FOUR
/* F5 */	0x0E55,	//	THAI DIGIT FIVE
/* F6 */	0x0E56,	//	THAI DIGIT SIX
/* F7 */	0x0E57,	//	THAI DIGIT SEVEN
/* F8 */	0x0E58,	//	THAI DIGIT EIGHT
/* F9 */	0x0E59,	//	THAI DIGIT NINE
/* FA */	0x0E5A,	//	THAI CHARACTER ANGKHANKHU
/* FB */	0x0E5B,	//	THAI CHARACTER KHOMUT
/* FC */	0xFFFD,	//	INVALID
/* FD */	0xFFFD,	//	INVALID
/* FE */	0xFFFD,	//	INVALID
/* FF */	0xFFFD	//	INVALID
};

/***************************************************************************
#
#	Name:             ISO/IEC 8859-13:1998  to Unicode
#	Unicode version:  3.0
#	Table version:    1.0
#	Table format:     Format A
#	Date:             1999 July 27
#	Authors:          Ken Whistler <kenw@sybase.com>
#
#	Copyright (c) 1998 - 1999 Unicode, Inc.  All Rights reserved.
#
#	This file is provided as-is by Unicode, Inc. (The Unicode Consortium).
#	No claims are made as to fitness for any particular purpose.  No
#	warranties of any kind are expressed or implied.  The recipient
#	agrees to determine applicability of information provided.  If this
#	file has been provided on optical media by Unicode, Inc., the sole
#	remedy for any claim will be exchange of defective media within 90
#	days of receipt.
#
#	Unicode, Inc. hereby grants the right to freely use the information
#	supplied in this file in the creation of products supporting the
#	Unicode Standard, and to make copies of this file in any form for
#	internal or external distribution as long as this notice remains
#	attached.
#
#	General notes:
#
#	This table contains the data the Unicode Consortium has on how
#       ISO/IEC 8859-13:1998 characters map into Unicode.
#
#	Format:  Three tab-separated columns
#		 Column #1 is the ISO/IEC 8859-13 code (in hex as 0xXX)
#		 Column #2 is the Unicode (in hex as 0xXXXX)
#		 Column #3 the Unicode name (follows a comment sign, '#')
#
#	The entries are in ISO/IEC 8859-13 order.
#
#	Updated versions of this file may be found in:
#		<ftp://ftp.unicode.org/Public/MAPPINGS/>
#
#	Any comments or problems, contact <errata@unicode.org>
#	Please note that <errata@unicode.org> is an archival address;
#	notices will be checked, but do not expect an immediate response.
#
***************************************************************************/
static const unicode_char lookup_iso8859_13[256] = {
/* 00 */	0x0000,	//	NULL
/* 01 */	0x0001,	//	START OF HEADING
/* 02 */	0x0002,	//	START OF TEXT
/* 03 */	0x0003,	//	END OF TEXT
/* 04 */	0x0004,	//	END OF TRANSMISSION
/* 05 */	0x0005,	//	ENQUIRY
/* 06 */	0x0006,	//	ACKNOWLEDGE
/* 07 */	0x0007,	//	BELL
/* 08 */	0x0008,	//	BACKSPACE
/* 09 */	0x0009,	//	HORIZONTAL TABULATION
/* 0A */	0x000A,	//	LINE FEED
/* 0B */	0x000B,	//	VERTICAL TABULATION
/* 0C */	0x000C,	//	FORM FEED
/* 0D */	0x000D,	//	CARRIAGE RETURN
/* 0E */	0x000E,	//	SHIFT OUT
/* 0F */	0x000F,	//	SHIFT IN
/* 10 */	0x0010,	//	DATA LINK ESCAPE
/* 11 */	0x0011,	//	DEVICE CONTROL ONE
/* 12 */	0x0012,	//	DEVICE CONTROL TWO
/* 13 */	0x0013,	//	DEVICE CONTROL THREE
/* 14 */	0x0014,	//	DEVICE CONTROL FOUR
/* 15 */	0x0015,	//	NEGATIVE ACKNOWLEDGE
/* 16 */	0x0016,	//	SYNCHRONOUS IDLE
/* 17 */	0x0017,	//	END OF TRANSMISSION BLOCK
/* 18 */	0x0018,	//	CANCEL
/* 19 */	0x0019,	//	END OF MEDIUM
/* 1A */	0x001A,	//	SUBSTITUTE
/* 1B */	0x001B,	//	ESCAPE
/* 1C */	0x001C,	//	FILE SEPARATOR
/* 1D */	0x001D,	//	GROUP SEPARATOR
/* 1E */	0x001E,	//	RECORD SEPARATOR
/* 1F */	0x001F,	//	UNIT SEPARATOR
/* 20 */	0x0020,	//	SPACE
/* 21 */	0x0021,	//	EXCLAMATION MARK
/* 22 */	0x0022,	//	QUOTATION MARK
/* 23 */	0x0023,	//	NUMBER SIGN
/* 24 */	0x0024,	//	DOLLAR SIGN
/* 25 */	0x0025,	//	PERCENT SIGN
/* 26 */	0x0026,	//	AMPERSAND
/* 27 */	0x0027,	//	APOSTROPHE
/* 28 */	0x0028,	//	LEFT PARENTHESIS
/* 29 */	0x0029,	//	RIGHT PARENTHESIS
/* 2A */	0x002A,	//	ASTERISK
/* 2B */	0x002B,	//	PLUS SIGN
/* 2C */	0x002C,	//	COMMA
/* 2D */	0x002D,	//	HYPHEN-MINUS
/* 2E */	0x002E,	//	FULL STOP
/* 2F */	0x002F,	//	SOLIDUS
/* 30 */	0x0030,	//	DIGIT ZERO
/* 31 */	0x0031,	//	DIGIT ONE
/* 32 */	0x0032,	//	DIGIT TWO
/* 33 */	0x0033,	//	DIGIT THREE
/* 34 */	0x0034,	//	DIGIT FOUR
/* 35 */	0x0035,	//	DIGIT FIVE
/* 36 */	0x0036,	//	DIGIT SIX
/* 37 */	0x0037,	//	DIGIT SEVEN
/* 38 */	0x0038,	//	DIGIT EIGHT
/* 39 */	0x0039,	//	DIGIT NINE
/* 3A */	0x003A,	//	COLON
/* 3B */	0x003B,	//	SEMICOLON
/* 3C */	0x003C,	//	LESS-THAN SIGN
/* 3D */	0x003D,	//	EQUALS SIGN
/* 3E */	0x003E,	//	GREATER-THAN SIGN
/* 3F */	0x003F,	//	QUESTION MARK
/* 40 */	0x0040,	//	COMMERCIAL AT
/* 41 */	0x0041,	//	LATIN CAPITAL LETTER A
/* 42 */	0x0042,	//	LATIN CAPITAL LETTER B
/* 43 */	0x0043,	//	LATIN CAPITAL LETTER C
/* 44 */	0x0044,	//	LATIN CAPITAL LETTER D
/* 45 */	0x0045,	//	LATIN CAPITAL LETTER E
/* 46 */	0x0046,	//	LATIN CAPITAL LETTER F
/* 47 */	0x0047,	//	LATIN CAPITAL LETTER G
/* 48 */	0x0048,	//	LATIN CAPITAL LETTER H
/* 49 */	0x0049,	//	LATIN CAPITAL LETTER I
/* 4A */	0x004A,	//	LATIN CAPITAL LETTER J
/* 4B */	0x004B,	//	LATIN CAPITAL LETTER K
/* 4C */	0x004C,	//	LATIN CAPITAL LETTER L
/* 4D */	0x004D,	//	LATIN CAPITAL LETTER M
/* 4E */	0x004E,	//	LATIN CAPITAL LETTER N
/* 4F */	0x004F,	//	LATIN CAPITAL LETTER O
/* 50 */	0x0050,	//	LATIN CAPITAL LETTER P
/* 51 */	0x0051,	//	LATIN CAPITAL LETTER Q
/* 52 */	0x0052,	//	LATIN CAPITAL LETTER R
/* 53 */	0x0053,	//	LATIN CAPITAL LETTER S
/* 54 */	0x0054,	//	LATIN CAPITAL LETTER T
/* 55 */	0x0055,	//	LATIN CAPITAL LETTER U
/* 56 */	0x0056,	//	LATIN CAPITAL LETTER V
/* 57 */	0x0057,	//	LATIN CAPITAL LETTER W
/* 58 */	0x0058,	//	LATIN CAPITAL LETTER X
/* 59 */	0x0059,	//	LATIN CAPITAL LETTER Y
/* 5A */	0x005A,	//	LATIN CAPITAL LETTER Z
/* 5B */	0x005B,	//	LEFT SQUARE BRACKET
/* 5C */	0x005C,	//	REVERSE SOLIDUS
/* 5D */	0x005D,	//	RIGHT SQUARE BRACKET
/* 5E */	0x005E,	//	CIRCUMFLEX ACCENT
/* 5F */	0x005F,	//	LOW LINE
/* 60 */	0x0060,	//	GRAVE ACCENT
/* 61 */	0x0061,	//	LATIN SMALL LETTER A
/* 62 */	0x0062,	//	LATIN SMALL LETTER B
/* 63 */	0x0063,	//	LATIN SMALL LETTER C
/* 64 */	0x0064,	//	LATIN SMALL LETTER D
/* 65 */	0x0065,	//	LATIN SMALL LETTER E
/* 66 */	0x0066,	//	LATIN SMALL LETTER F
/* 67 */	0x0067,	//	LATIN SMALL LETTER G
/* 68 */	0x0068,	//	LATIN SMALL LETTER H
/* 69 */	0x0069,	//	LATIN SMALL LETTER I
/* 6A */	0x006A,	//	LATIN SMALL LETTER J
/* 6B */	0x006B,	//	LATIN SMALL LETTER K
/* 6C */	0x006C,	//	LATIN SMALL LETTER L
/* 6D */	0x006D,	//	LATIN SMALL LETTER M
/* 6E */	0x006E,	//	LATIN SMALL LETTER N
/* 6F */	0x006F,	//	LATIN SMALL LETTER O
/* 70 */	0x0070,	//	LATIN SMALL LETTER P
/* 71 */	0x0071,	//	LATIN SMALL LETTER Q
/* 72 */	0x0072,	//	LATIN SMALL LETTER R
/* 73 */	0x0073,	//	LATIN SMALL LETTER S
/* 74 */	0x0074,	//	LATIN SMALL LETTER T
/* 75 */	0x0075,	//	LATIN SMALL LETTER U
/* 76 */	0x0076,	//	LATIN SMALL LETTER V
/* 77 */	0x0077,	//	LATIN SMALL LETTER W
/* 78 */	0x0078,	//	LATIN SMALL LETTER X
/* 79 */	0x0079,	//	LATIN SMALL LETTER Y
/* 7A */	0x007A,	//	LATIN SMALL LETTER Z
/* 7B */	0x007B,	//	LEFT CURLY BRACKET
/* 7C */	0x007C,	//	VERTICAL LINE
/* 7D */	0x007D,	//	RIGHT CURLY BRACKET
/* 7E */	0x007E,	//	TILDE
/* 7F */	0x007F,	//	DELETE
/* 80 */	0x0080,	//	<control>
/* 81 */	0x0081,	//	<control>
/* 82 */	0x0082,	//	<control>
/* 83 */	0x0083,	//	<control>
/* 84 */	0x0084,	//	<control>
/* 85 */	0x0085,	//	<control>
/* 86 */	0x0086,	//	<control>
/* 87 */	0x0087,	//	<control>
/* 88 */	0x0088,	//	<control>
/* 89 */	0x0089,	//	<control>
/* 8A */	0x008A,	//	<control>
/* 8B */	0x008B,	//	<control>
/* 8C */	0x008C,	//	<control>
/* 8D */	0x008D,	//	<control>
/* 8E */	0x008E,	//	<control>
/* 8F */	0x008F,	//	<control>
/* 90 */	0x0090,	//	<control>
/* 91 */	0x0091,	//	<control>
/* 92 */	0x0092,	//	<control>
/* 93 */	0x0093,	//	<control>
/* 94 */	0x0094,	//	<control>
/* 95 */	0x0095,	//	<control>
/* 96 */	0x0096,	//	<control>
/* 97 */	0x0097,	//	<control>
/* 98 */	0x0098,	//	<control>
/* 99 */	0x0099,	//	<control>
/* 9A */	0x009A,	//	<control>
/* 9B */	0x009B,	//	<control>
/* 9C */	0x009C,	//	<control>
/* 9D */	0x009D,	//	<control>
/* 9E */	0x009E,	//	<control>
/* 9F */	0x009F,	//	<control>
/* A0 */	0x00A0,	//	NO-BREAK SPACE
/* A1 */	0x201D,	//	RIGHT DOUBLE QUOTATION MARK
/* A2 */	0x00A2,	//	CENT SIGN
/* A3 */	0x00A3,	//	POUND SIGN
/* A4 */	0x00A4,	//	CURRENCY SIGN
/* A5 */	0x201E,	//	DOUBLE LOW-9 QUOTATION MARK
/* A6 */	0x00A6,	//	BROKEN BAR
/* A7 */	0x00A7,	//	SECTION SIGN
/* A8 */	0x00D8,	//	LATIN CAPITAL LETTER O WITH STROKE
/* A9 */	0x00A9,	//	COPYRIGHT SIGN
/* AA */	0x0156,	//	LATIN CAPITAL LETTER R WITH CEDILLA
/* AB */	0x00AB,	//	LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
/* AC */	0x00AC,	//	NOT SIGN
/* AD */	0x00AD,	//	SOFT HYPHEN
/* AE */	0x00AE,	//	REGISTERED SIGN
/* AF */	0x00C6,	//	LATIN CAPITAL LETTER AE
/* B0 */	0x00B0,	//	DEGREE SIGN
/* B1 */	0x00B1,	//	PLUS-MINUS SIGN
/* B2 */	0x00B2,	//	SUPERSCRIPT TWO
/* B3 */	0x00B3,	//	SUPERSCRIPT THREE
/* B4 */	0x201C,	//	LEFT DOUBLE QUOTATION MARK
/* B5 */	0x00B5,	//	MICRO SIGN
/* B6 */	0x00B6,	//	PILCROW SIGN
/* B7 */	0x00B7,	//	MIDDLE DOT
/* B8 */	0x00F8,	//	LATIN SMALL LETTER O WITH STROKE
/* B9 */	0x00B9,	//	SUPERSCRIPT ONE
/* BA */	0x0157,	//	LATIN SMALL LETTER R WITH CEDILLA
/* BB */	0x00BB,	//	RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
/* BC */	0x00BC,	//	VULGAR FRACTION ONE QUARTER
/* BD */	0x00BD,	//	VULGAR FRACTION ONE HALF
/* BE */	0x00BE,	//	VULGAR FRACTION THREE QUARTERS
/* BF */	0x00E6,	//	LATIN SMALL LETTER AE
/* C0 */	0x0104,	//	LATIN CAPITAL LETTER A WITH OGONEK
/* C1 */	0x012E,	//	LATIN CAPITAL LETTER I WITH OGONEK
/* C2 */	0x0100,	//	LATIN CAPITAL LETTER A WITH MACRON
/* C3 */	0x0106,	//	LATIN CAPITAL LETTER C WITH ACUTE
/* C4 */	0x00C4,	//	LATIN CAPITAL LETTER A WITH DIAERESIS
/* C5 */	0x00C5,	//	LATIN CAPITAL LETTER A WITH RING ABOVE
/* C6 */	0x0118,	//	LATIN CAPITAL LETTER E WITH OGONEK
/* C7 */	0x0112,	//	LATIN CAPITAL LETTER E WITH MACRON
/* C8 */	0x010C,	//	LATIN CAPITAL LETTER C WITH CARON
/* C9 */	0x00C9,	//	LATIN CAPITAL LETTER E WITH ACUTE
/* CA */	0x0179,	//	LATIN CAPITAL LETTER Z WITH ACUTE
/* CB */	0x0116,	//	LATIN CAPITAL LETTER E WITH DOT ABOVE
/* CC */	0x0122,	//	LATIN CAPITAL LETTER G WITH CEDILLA
/* CD */	0x0136,	//	LATIN CAPITAL LETTER K WITH CEDILLA
/* CE */	0x012A,	//	LATIN CAPITAL LETTER I WITH MACRON
/* CF */	0x013B,	//	LATIN CAPITAL LETTER L WITH CEDILLA
/* D0 */	0x0160,	//	LATIN CAPITAL LETTER S WITH CARON
/* D1 */	0x0143,	//	LATIN CAPITAL LETTER N WITH ACUTE
/* D2 */	0x0145,	//	LATIN CAPITAL LETTER N WITH CEDILLA
/* D3 */	0x00D3,	//	LATIN CAPITAL LETTER O WITH ACUTE
/* D4 */	0x014C,	//	LATIN CAPITAL LETTER O WITH MACRON
/* D5 */	0x00D5,	//	LATIN CAPITAL LETTER O WITH TILDE
/* D6 */	0x00D6,	//	LATIN CAPITAL LETTER O WITH DIAERESIS
/* D7 */	0x00D7,	//	MULTIPLICATION SIGN
/* D8 */	0x0172,	//	LATIN CAPITAL LETTER U WITH OGONEK
/* D9 */	0x0141,	//	LATIN CAPITAL LETTER L WITH STROKE
/* DA */	0x015A,	//	LATIN CAPITAL LETTER S WITH ACUTE
/* DB */	0x016A,	//	LATIN CAPITAL LETTER U WITH MACRON
/* DC */	0x00DC,	//	LATIN CAPITAL LETTER U WITH DIAERESIS
/* DD */	0x017B,	//	LATIN CAPITAL LETTER Z WITH DOT ABOVE
/* DE */	0x017D,	//	LATIN CAPITAL LETTER Z WITH CARON
/* DF */	0x00DF,	//	LATIN SMALL LETTER SHARP S (German)
/* E0 */	0x0105,	//	LATIN SMALL LETTER A WITH OGONEK
/* E1 */	0x012F,	//	LATIN SMALL LETTER I WITH OGONEK
/* E2 */	0x0101,	//	LATIN SMALL LETTER A WITH MACRON
/* E3 */	0x0107,	//	LATIN SMALL LETTER C WITH ACUTE
/* E4 */	0x00E4,	//	LATIN SMALL LETTER A WITH DIAERESIS
/* E5 */	0x00E5,	//	LATIN SMALL LETTER A WITH RING ABOVE
/* E6 */	0x0119,	//	LATIN SMALL LETTER E WITH OGONEK
/* E7 */	0x0113,	//	LATIN SMALL LETTER E WITH MACRON
/* E8 */	0x010D,	//	LATIN SMALL LETTER C WITH CARON
/* E9 */	0x00E9,	//	LATIN SMALL LETTER E WITH ACUTE
/* EA */	0x017A,	//	LATIN SMALL LETTER Z WITH ACUTE
/* EB */	0x0117,	//	LATIN SMALL LETTER E WITH DOT ABOVE
/* EC */	0x0123,	//	LATIN SMALL LETTER G WITH CEDILLA
/* ED */	0x0137,	//	LATIN SMALL LETTER K WITH CEDILLA
/* EE */	0x012B,	//	LATIN SMALL LETTER I WITH MACRON
/* EF */	0x013C,	//	LATIN SMALL LETTER L WITH CEDILLA
/* F0 */	0x0161,	//	LATIN SMALL LETTER S WITH CARON
/* F1 */	0x0144,	//	LATIN SMALL LETTER N WITH ACUTE
/* F2 */	0x0146,	//	LATIN SMALL LETTER N WITH CEDILLA
/* F3 */	0x00F3,	//	LATIN SMALL LETTER O WITH ACUTE
/* F4 */	0x014D,	//	LATIN SMALL LETTER O WITH MACRON
/* F5 */	0x00F5,	//	LATIN SMALL LETTER O WITH TILDE
/* F6 */	0x00F6,	//	LATIN SMALL LETTER O WITH DIAERESIS
/* F7 */	0x00F7,	//	DIVISION SIGN
/* F8 */	0x0173,	//	LATIN SMALL LETTER U WITH OGONEK
/* F9 */	0x0142,	//	LATIN SMALL LETTER L WITH STROKE
/* FA */	0x015B,	//	LATIN SMALL LETTER S WITH ACUTE
/* FB */	0x016B,	//	LATIN SMALL LETTER U WITH MACRON
/* FC */	0x00FC,	//	LATIN SMALL LETTER U WITH DIAERESIS
/* FD */	0x017C,	//	LATIN SMALL LETTER Z WITH DOT ABOVE
/* FE */	0x017E,	//	LATIN SMALL LETTER Z WITH CARON
/* FF */	0x2019	//	RIGHT SINGLE QUOTATION MARK
};

/***************************************************************************
#
#	Name:             ISO/IEC 8859-14:1998 to Unicode
#	Unicode version:  3.0
#	Table version:    1.0
#	Table format:     Format A
#	Date:             1999 July 27
#	Authors:          Markus Kuhn <http://www.cl.cam.ac.uk/~mgk25/>
#			  Ken Whistler <kenw@sybase.com>
#
#	Copyright (c) 1998 - 1999 Unicode, Inc.  All Rights reserved.
#
#	This file is provided as-is by Unicode, Inc. (The Unicode Consortium).
#	No claims are made as to fitness for any particular purpose.  No
#	warranties of any kind are expressed or implied.  The recipient
#	agrees to determine applicability of information provided.  If this
#	file has been provided on optical media by Unicode, Inc., the sole
#	remedy for any claim will be exchange of defective media within 90
#	days of receipt.
#
#	Unicode, Inc. hereby grants the right to freely use the information
#	supplied in this file in the creation of products supporting the
#	Unicode Standard, and to make copies of this file in any form for
#	internal or external distribution as long as this notice remains
#	attached.
#
#	General notes:
#
#	This table contains the data the Unicode Consortium has on how
#       ISO/IEC 8859-14:1998 characters map into Unicode.
#
#	Format:  Three tab-separated columns
#		 Column #1 is the ISO/IEC 8859-14 code (in hex as 0xXX)
#		 Column #2 is the Unicode (in hex as 0xXXXX)
#		 Column #3 the Unicode name (follows a comment sign, '#')
#
#	The entries are in ISO/IEC 8859-14 order.
#
#	Updated versions of this file may be found in:
#		<ftp://ftp.unicode.org/Public/MAPPINGS/>
#
#	Any comments or problems, contact <errata@unicode.org>
#	Please note that <errata@unicode.org> is an archival address;
#	notices will be checked, but do not expect an immediate response.
#
***************************************************************************/
static const unicode_char lookup_iso8859_14[256] = {
/* 00 */	0x0000,	//	NULL
/* 01 */	0x0001,	//	START OF HEADING
/* 02 */	0x0002,	//	START OF TEXT
/* 03 */	0x0003,	//	END OF TEXT
/* 04 */	0x0004,	//	END OF TRANSMISSION
/* 05 */	0x0005,	//	ENQUIRY
/* 06 */	0x0006,	//	ACKNOWLEDGE
/* 07 */	0x0007,	//	BELL
/* 08 */	0x0008,	//	BACKSPACE
/* 09 */	0x0009,	//	HORIZONTAL TABULATION
/* 0A */	0x000A,	//	LINE FEED
/* 0B */	0x000B,	//	VERTICAL TABULATION
/* 0C */	0x000C,	//	FORM FEED
/* 0D */	0x000D,	//	CARRIAGE RETURN
/* 0E */	0x000E,	//	SHIFT OUT
/* 0F */	0x000F,	//	SHIFT IN
/* 10 */	0x0010,	//	DATA LINK ESCAPE
/* 11 */	0x0011,	//	DEVICE CONTROL ONE
/* 12 */	0x0012,	//	DEVICE CONTROL TWO
/* 13 */	0x0013,	//	DEVICE CONTROL THREE
/* 14 */	0x0014,	//	DEVICE CONTROL FOUR
/* 15 */	0x0015,	//	NEGATIVE ACKNOWLEDGE
/* 16 */	0x0016,	//	SYNCHRONOUS IDLE
/* 17 */	0x0017,	//	END OF TRANSMISSION BLOCK
/* 18 */	0x0018,	//	CANCEL
/* 19 */	0x0019,	//	END OF MEDIUM
/* 1A */	0x001A,	//	SUBSTITUTE
/* 1B */	0x001B,	//	ESCAPE
/* 1C */	0x001C,	//	FILE SEPARATOR
/* 1D */	0x001D,	//	GROUP SEPARATOR
/* 1E */	0x001E,	//	RECORD SEPARATOR
/* 1F */	0x001F,	//	UNIT SEPARATOR
/* 20 */	0x0020,	//	SPACE
/* 21 */	0x0021,	//	EXCLAMATION MARK
/* 22 */	0x0022,	//	QUOTATION MARK
/* 23 */	0x0023,	//	NUMBER SIGN
/* 24 */	0x0024,	//	DOLLAR SIGN
/* 25 */	0x0025,	//	PERCENT SIGN
/* 26 */	0x0026,	//	AMPERSAND
/* 27 */	0x0027,	//	APOSTROPHE
/* 28 */	0x0028,	//	LEFT PARENTHESIS
/* 29 */	0x0029,	//	RIGHT PARENTHESIS
/* 2A */	0x002A,	//	ASTERISK
/* 2B */	0x002B,	//	PLUS SIGN
/* 2C */	0x002C,	//	COMMA
/* 2D */	0x002D,	//	HYPHEN-MINUS
/* 2E */	0x002E,	//	FULL STOP
/* 2F */	0x002F,	//	SOLIDUS
/* 30 */	0x0030,	//	DIGIT ZERO
/* 31 */	0x0031,	//	DIGIT ONE
/* 32 */	0x0032,	//	DIGIT TWO
/* 33 */	0x0033,	//	DIGIT THREE
/* 34 */	0x0034,	//	DIGIT FOUR
/* 35 */	0x0035,	//	DIGIT FIVE
/* 36 */	0x0036,	//	DIGIT SIX
/* 37 */	0x0037,	//	DIGIT SEVEN
/* 38 */	0x0038,	//	DIGIT EIGHT
/* 39 */	0x0039,	//	DIGIT NINE
/* 3A */	0x003A,	//	COLON
/* 3B */	0x003B,	//	SEMICOLON
/* 3C */	0x003C,	//	LESS-THAN SIGN
/* 3D */	0x003D,	//	EQUALS SIGN
/* 3E */	0x003E,	//	GREATER-THAN SIGN
/* 3F */	0x003F,	//	QUESTION MARK
/* 40 */	0x0040,	//	COMMERCIAL AT
/* 41 */	0x0041,	//	LATIN CAPITAL LETTER A
/* 42 */	0x0042,	//	LATIN CAPITAL LETTER B
/* 43 */	0x0043,	//	LATIN CAPITAL LETTER C
/* 44 */	0x0044,	//	LATIN CAPITAL LETTER D
/* 45 */	0x0045,	//	LATIN CAPITAL LETTER E
/* 46 */	0x0046,	//	LATIN CAPITAL LETTER F
/* 47 */	0x0047,	//	LATIN CAPITAL LETTER G
/* 48 */	0x0048,	//	LATIN CAPITAL LETTER H
/* 49 */	0x0049,	//	LATIN CAPITAL LETTER I
/* 4A */	0x004A,	//	LATIN CAPITAL LETTER J
/* 4B */	0x004B,	//	LATIN CAPITAL LETTER K
/* 4C */	0x004C,	//	LATIN CAPITAL LETTER L
/* 4D */	0x004D,	//	LATIN CAPITAL LETTER M
/* 4E */	0x004E,	//	LATIN CAPITAL LETTER N
/* 4F */	0x004F,	//	LATIN CAPITAL LETTER O
/* 50 */	0x0050,	//	LATIN CAPITAL LETTER P
/* 51 */	0x0051,	//	LATIN CAPITAL LETTER Q
/* 52 */	0x0052,	//	LATIN CAPITAL LETTER R
/* 53 */	0x0053,	//	LATIN CAPITAL LETTER S
/* 54 */	0x0054,	//	LATIN CAPITAL LETTER T
/* 55 */	0x0055,	//	LATIN CAPITAL LETTER U
/* 56 */	0x0056,	//	LATIN CAPITAL LETTER V
/* 57 */	0x0057,	//	LATIN CAPITAL LETTER W
/* 58 */	0x0058,	//	LATIN CAPITAL LETTER X
/* 59 */	0x0059,	//	LATIN CAPITAL LETTER Y
/* 5A */	0x005A,	//	LATIN CAPITAL LETTER Z
/* 5B */	0x005B,	//	LEFT SQUARE BRACKET
/* 5C */	0x005C,	//	REVERSE SOLIDUS
/* 5D */	0x005D,	//	RIGHT SQUARE BRACKET
/* 5E */	0x005E,	//	CIRCUMFLEX ACCENT
/* 5F */	0x005F,	//	LOW LINE
/* 60 */	0x0060,	//	GRAVE ACCENT
/* 61 */	0x0061,	//	LATIN SMALL LETTER A
/* 62 */	0x0062,	//	LATIN SMALL LETTER B
/* 63 */	0x0063,	//	LATIN SMALL LETTER C
/* 64 */	0x0064,	//	LATIN SMALL LETTER D
/* 65 */	0x0065,	//	LATIN SMALL LETTER E
/* 66 */	0x0066,	//	LATIN SMALL LETTER F
/* 67 */	0x0067,	//	LATIN SMALL LETTER G
/* 68 */	0x0068,	//	LATIN SMALL LETTER H
/* 69 */	0x0069,	//	LATIN SMALL LETTER I
/* 6A */	0x006A,	//	LATIN SMALL LETTER J
/* 6B */	0x006B,	//	LATIN SMALL LETTER K
/* 6C */	0x006C,	//	LATIN SMALL LETTER L
/* 6D */	0x006D,	//	LATIN SMALL LETTER M
/* 6E */	0x006E,	//	LATIN SMALL LETTER N
/* 6F */	0x006F,	//	LATIN SMALL LETTER O
/* 70 */	0x0070,	//	LATIN SMALL LETTER P
/* 71 */	0x0071,	//	LATIN SMALL LETTER Q
/* 72 */	0x0072,	//	LATIN SMALL LETTER R
/* 73 */	0x0073,	//	LATIN SMALL LETTER S
/* 74 */	0x0074,	//	LATIN SMALL LETTER T
/* 75 */	0x0075,	//	LATIN SMALL LETTER U
/* 76 */	0x0076,	//	LATIN SMALL LETTER V
/* 77 */	0x0077,	//	LATIN SMALL LETTER W
/* 78 */	0x0078,	//	LATIN SMALL LETTER X
/* 79 */	0x0079,	//	LATIN SMALL LETTER Y
/* 7A */	0x007A,	//	LATIN SMALL LETTER Z
/* 7B */	0x007B,	//	LEFT CURLY BRACKET
/* 7C */	0x007C,	//	VERTICAL LINE
/* 7D */	0x007D,	//	RIGHT CURLY BRACKET
/* 7E */	0x007E,	//	TILDE
/* 7F */	0x007F,	//	DELETE
/* 80 */	0x0080,	//	<control>
/* 81 */	0x0081,	//	<control>
/* 82 */	0x0082,	//	<control>
/* 83 */	0x0083,	//	<control>
/* 84 */	0x0084,	//	<control>
/* 85 */	0x0085,	//	<control>
/* 86 */	0x0086,	//	<control>
/* 87 */	0x0087,	//	<control>
/* 88 */	0x0088,	//	<control>
/* 89 */	0x0089,	//	<control>
/* 8A */	0x008A,	//	<control>
/* 8B */	0x008B,	//	<control>
/* 8C */	0x008C,	//	<control>
/* 8D */	0x008D,	//	<control>
/* 8E */	0x008E,	//	<control>
/* 8F */	0x008F,	//	<control>
/* 90 */	0x0090,	//	<control>
/* 91 */	0x0091,	//	<control>
/* 92 */	0x0092,	//	<control>
/* 93 */	0x0093,	//	<control>
/* 94 */	0x0094,	//	<control>
/* 95 */	0x0095,	//	<control>
/* 96 */	0x0096,	//	<control>
/* 97 */	0x0097,	//	<control>
/* 98 */	0x0098,	//	<control>
/* 99 */	0x0099,	//	<control>
/* 9A */	0x009A,	//	<control>
/* 9B */	0x009B,	//	<control>
/* 9C */	0x009C,	//	<control>
/* 9D */	0x009D,	//	<control>
/* 9E */	0x009E,	//	<control>
/* 9F */	0x009F,	//	<control>
/* A0 */	0x00A0,	//	NO-BREAK SPACE
/* A1 */	0x1E02,	//	LATIN CAPITAL LETTER B WITH DOT ABOVE
/* A2 */	0x1E03,	//	LATIN SMALL LETTER B WITH DOT ABOVE
/* A3 */	0x00A3,	//	POUND SIGN
/* A4 */	0x010A,	//	LATIN CAPITAL LETTER C WITH DOT ABOVE
/* A5 */	0x010B,	//	LATIN SMALL LETTER C WITH DOT ABOVE
/* A6 */	0x1E0A,	//	LATIN CAPITAL LETTER D WITH DOT ABOVE
/* A7 */	0x00A7,	//	SECTION SIGN
/* A8 */	0x1E80,	//	LATIN CAPITAL LETTER W WITH GRAVE
/* A9 */	0x00A9,	//	COPYRIGHT SIGN
/* AA */	0x1E82,	//	LATIN CAPITAL LETTER W WITH ACUTE
/* AB */	0x1E0B,	//	LATIN SMALL LETTER D WITH DOT ABOVE
/* AC */	0x1EF2,	//	LATIN CAPITAL LETTER Y WITH GRAVE
/* AD */	0x00AD,	//	SOFT HYPHEN
/* AE */	0x00AE,	//	REGISTERED SIGN
/* AF */	0x0178,	//	LATIN CAPITAL LETTER Y WITH DIAERESIS
/* B0 */	0x1E1E,	//	LATIN CAPITAL LETTER F WITH DOT ABOVE
/* B1 */	0x1E1F,	//	LATIN SMALL LETTER F WITH DOT ABOVE
/* B2 */	0x0120,	//	LATIN CAPITAL LETTER G WITH DOT ABOVE
/* B3 */	0x0121,	//	LATIN SMALL LETTER G WITH DOT ABOVE
/* B4 */	0x1E40,	//	LATIN CAPITAL LETTER M WITH DOT ABOVE
/* B5 */	0x1E41,	//	LATIN SMALL LETTER M WITH DOT ABOVE
/* B6 */	0x00B6,	//	PILCROW SIGN
/* B7 */	0x1E56,	//	LATIN CAPITAL LETTER P WITH DOT ABOVE
/* B8 */	0x1E81,	//	LATIN SMALL LETTER W WITH GRAVE
/* B9 */	0x1E57,	//	LATIN SMALL LETTER P WITH DOT ABOVE
/* BA */	0x1E83,	//	LATIN SMALL LETTER W WITH ACUTE
/* BB */	0x1E60,	//	LATIN CAPITAL LETTER S WITH DOT ABOVE
/* BC */	0x1EF3,	//	LATIN SMALL LETTER Y WITH GRAVE
/* BD */	0x1E84,	//	LATIN CAPITAL LETTER W WITH DIAERESIS
/* BE */	0x1E85,	//	LATIN SMALL LETTER W WITH DIAERESIS
/* BF */	0x1E61,	//	LATIN SMALL LETTER S WITH DOT ABOVE
/* C0 */	0x00C0,	//	LATIN CAPITAL LETTER A WITH GRAVE
/* C1 */	0x00C1,	//	LATIN CAPITAL LETTER A WITH ACUTE
/* C2 */	0x00C2,	//	LATIN CAPITAL LETTER A WITH CIRCUMFLEX
/* C3 */	0x00C3,	//	LATIN CAPITAL LETTER A WITH TILDE
/* C4 */	0x00C4,	//	LATIN CAPITAL LETTER A WITH DIAERESIS
/* C5 */	0x00C5,	//	LATIN CAPITAL LETTER A WITH RING ABOVE
/* C6 */	0x00C6,	//	LATIN CAPITAL LETTER AE
/* C7 */	0x00C7,	//	LATIN CAPITAL LETTER C WITH CEDILLA
/* C8 */	0x00C8,	//	LATIN CAPITAL LETTER E WITH GRAVE
/* C9 */	0x00C9,	//	LATIN CAPITAL LETTER E WITH ACUTE
/* CA */	0x00CA,	//	LATIN CAPITAL LETTER E WITH CIRCUMFLEX
/* CB */	0x00CB,	//	LATIN CAPITAL LETTER E WITH DIAERESIS
/* CC */	0x00CC,	//	LATIN CAPITAL LETTER I WITH GRAVE
/* CD */	0x00CD,	//	LATIN CAPITAL LETTER I WITH ACUTE
/* CE */	0x00CE,	//	LATIN CAPITAL LETTER I WITH CIRCUMFLEX
/* CF */	0x00CF,	//	LATIN CAPITAL LETTER I WITH DIAERESIS
/* D0 */	0x0174,	//	LATIN CAPITAL LETTER W WITH CIRCUMFLEX
/* D1 */	0x00D1,	//	LATIN CAPITAL LETTER N WITH TILDE
/* D2 */	0x00D2,	//	LATIN CAPITAL LETTER O WITH GRAVE
/* D3 */	0x00D3,	//	LATIN CAPITAL LETTER O WITH ACUTE
/* D4 */	0x00D4,	//	LATIN CAPITAL LETTER O WITH CIRCUMFLEX
/* D5 */	0x00D5,	//	LATIN CAPITAL LETTER O WITH TILDE
/* D6 */	0x00D6,	//	LATIN CAPITAL LETTER O WITH DIAERESIS
/* D7 */	0x1E6A,	//	LATIN CAPITAL LETTER T WITH DOT ABOVE
/* D8 */	0x00D8,	//	LATIN CAPITAL LETTER O WITH STROKE
/* D9 */	0x00D9,	//	LATIN CAPITAL LETTER U WITH GRAVE
/* DA */	0x00DA,	//	LATIN CAPITAL LETTER U WITH ACUTE
/* DB */	0x00DB,	//	LATIN CAPITAL LETTER U WITH CIRCUMFLEX
/* DC */	0x00DC,	//	LATIN CAPITAL LETTER U WITH DIAERESIS
/* DD */	0x00DD,	//	LATIN CAPITAL LETTER Y WITH ACUTE
/* DE */	0x0176,	//	LATIN CAPITAL LETTER Y WITH CIRCUMFLEX
/* DF */	0x00DF,	//	LATIN SMALL LETTER SHARP S
/* E0 */	0x00E0,	//	LATIN SMALL LETTER A WITH GRAVE
/* E1 */	0x00E1,	//	LATIN SMALL LETTER A WITH ACUTE
/* E2 */	0x00E2,	//	LATIN SMALL LETTER A WITH CIRCUMFLEX
/* E3 */	0x00E3,	//	LATIN SMALL LETTER A WITH TILDE
/* E4 */	0x00E4,	//	LATIN SMALL LETTER A WITH DIAERESIS
/* E5 */	0x00E5,	//	LATIN SMALL LETTER A WITH RING ABOVE
/* E6 */	0x00E6,	//	LATIN SMALL LETTER AE
/* E7 */	0x00E7,	//	LATIN SMALL LETTER C WITH CEDILLA
/* E8 */	0x00E8,	//	LATIN SMALL LETTER E WITH GRAVE
/* E9 */	0x00E9,	//	LATIN SMALL LETTER E WITH ACUTE
/* EA */	0x00EA,	//	LATIN SMALL LETTER E WITH CIRCUMFLEX
/* EB */	0x00EB,	//	LATIN SMALL LETTER E WITH DIAERESIS
/* EC */	0x00EC,	//	LATIN SMALL LETTER I WITH GRAVE
/* ED */	0x00ED,	//	LATIN SMALL LETTER I WITH ACUTE
/* EE */	0x00EE,	//	LATIN SMALL LETTER I WITH CIRCUMFLEX
/* EF */	0x00EF,	//	LATIN SMALL LETTER I WITH DIAERESIS
/* F0 */	0x0175,	//	LATIN SMALL LETTER W WITH CIRCUMFLEX
/* F1 */	0x00F1,	//	LATIN SMALL LETTER N WITH TILDE
/* F2 */	0x00F2,	//	LATIN SMALL LETTER O WITH GRAVE
/* F3 */	0x00F3,	//	LATIN SMALL LETTER O WITH ACUTE
/* F4 */	0x00F4,	//	LATIN SMALL LETTER O WITH CIRCUMFLEX
/* F5 */	0x00F5,	//	LATIN SMALL LETTER O WITH TILDE
/* F6 */	0x00F6,	//	LATIN SMALL LETTER O WITH DIAERESIS
/* F7 */	0x1E6B,	//	LATIN SMALL LETTER T WITH DOT ABOVE
/* F8 */	0x00F8,	//	LATIN SMALL LETTER O WITH STROKE
/* F9 */	0x00F9,	//	LATIN SMALL LETTER U WITH GRAVE
/* FA */	0x00FA,	//	LATIN SMALL LETTER U WITH ACUTE
/* FB */	0x00FB,	//	LATIN SMALL LETTER U WITH CIRCUMFLEX
/* FC */	0x00FC,	//	LATIN SMALL LETTER U WITH DIAERESIS
/* FD */	0x00FD,	//	LATIN SMALL LETTER Y WITH ACUTE
/* FE */	0x0177,	//	LATIN SMALL LETTER Y WITH CIRCUMFLEX
/* FF */	0x00FF	//	LATIN SMALL LETTER Y WITH DIAERESIS
};

/***************************************************************************
#
#	Name:             ISO/IEC 8859-15:1999 to Unicode
#	Unicode version:  3.0
#	Table version:    1.0
#	Table format:     Format A
#	Date:             1999 July 27
#	Authors:          Markus Kuhn <http://www.cl.cam.ac.uk/~mgk25/>
#			  Ken Whistler <kenw@sybase.com>
#
#	Copyright (c) 1998 - 1999 Unicode, Inc.  All Rights reserved.
#
#	This file is provided as-is by Unicode, Inc. (The Unicode Consortium).
#	No claims are made as to fitness for any particular purpose.  No
#	warranties of any kind are expressed or implied.  The recipient
#	agrees to determine applicability of information provided.  If this
#	file has been provided on optical media by Unicode, Inc., the sole
#	remedy for any claim will be exchange of defective media within 90
#	days of receipt.
#
#	Unicode, Inc. hereby grants the right to freely use the information
#	supplied in this file in the creation of products supporting the
#	Unicode Standard, and to make copies of this file in any form for
#	internal or external distribution as long as this notice remains
#	attached.
#
#	General notes:
#
#	This table contains the data the Unicode Consortium has on how
#       ISO/IEC 8859-15:1999 characters map into Unicode.
#
#	Format:  Three tab-separated columns
#		 Column #1 is the ISO/IEC 8859-15 code (in hex as 0xXX)
#		 Column #2 is the Unicode (in hex as 0xXXXX)
#		 Column #3 the Unicode name (follows a comment sign, '#')
#
#	The entries are in ISO/IEC 8859-15 order.
#
#	Version history
#
#	Updated versions of this file may be found in:
#		<ftp://ftp.unicode.org/Public/MAPPINGS/>
#
#	Any comments or problems, contact <errata@unicode.org>
#	Please note that <errata@unicode.org> is an archival address;
#	notices will be checked, but do not expect an immediate response.
#
***************************************************************************/
static const unicode_char lookup_iso8859_15[256] = {
/* 00 */	0x0000,	//	NULL
/* 01 */	0x0001,	//	START OF HEADING
/* 02 */	0x0002,	//	START OF TEXT
/* 03 */	0x0003,	//	END OF TEXT
/* 04 */	0x0004,	//	END OF TRANSMISSION
/* 05 */	0x0005,	//	ENQUIRY
/* 06 */	0x0006,	//	ACKNOWLEDGE
/* 07 */	0x0007,	//	BELL
/* 08 */	0x0008,	//	BACKSPACE
/* 09 */	0x0009,	//	HORIZONTAL TABULATION
/* 0A */	0x000A,	//	LINE FEED
/* 0B */	0x000B,	//	VERTICAL TABULATION
/* 0C */	0x000C,	//	FORM FEED
/* 0D */	0x000D,	//	CARRIAGE RETURN
/* 0E */	0x000E,	//	SHIFT OUT
/* 0F */	0x000F,	//	SHIFT IN
/* 10 */	0x0010,	//	DATA LINK ESCAPE
/* 11 */	0x0011,	//	DEVICE CONTROL ONE
/* 12 */	0x0012,	//	DEVICE CONTROL TWO
/* 13 */	0x0013,	//	DEVICE CONTROL THREE
/* 14 */	0x0014,	//	DEVICE CONTROL FOUR
/* 15 */	0x0015,	//	NEGATIVE ACKNOWLEDGE
/* 16 */	0x0016,	//	SYNCHRONOUS IDLE
/* 17 */	0x0017,	//	END OF TRANSMISSION BLOCK
/* 18 */	0x0018,	//	CANCEL
/* 19 */	0x0019,	//	END OF MEDIUM
/* 1A */	0x001A,	//	SUBSTITUTE
/* 1B */	0x001B,	//	ESCAPE
/* 1C */	0x001C,	//	FILE SEPARATOR
/* 1D */	0x001D,	//	GROUP SEPARATOR
/* 1E */	0x001E,	//	RECORD SEPARATOR
/* 1F */	0x001F,	//	UNIT SEPARATOR
/* 20 */	0x0020,	//	SPACE
/* 21 */	0x0021,	//	EXCLAMATION MARK
/* 22 */	0x0022,	//	QUOTATION MARK
/* 23 */	0x0023,	//	NUMBER SIGN
/* 24 */	0x0024,	//	DOLLAR SIGN
/* 25 */	0x0025,	//	PERCENT SIGN
/* 26 */	0x0026,	//	AMPERSAND
/* 27 */	0x0027,	//	APOSTROPHE
/* 28 */	0x0028,	//	LEFT PARENTHESIS
/* 29 */	0x0029,	//	RIGHT PARENTHESIS
/* 2A */	0x002A,	//	ASTERISK
/* 2B */	0x002B,	//	PLUS SIGN
/* 2C */	0x002C,	//	COMMA
/* 2D */	0x002D,	//	HYPHEN-MINUS
/* 2E */	0x002E,	//	FULL STOP
/* 2F */	0x002F,	//	SOLIDUS
/* 30 */	0x0030,	//	DIGIT ZERO
/* 31 */	0x0031,	//	DIGIT ONE
/* 32 */	0x0032,	//	DIGIT TWO
/* 33 */	0x0033,	//	DIGIT THREE
/* 34 */	0x0034,	//	DIGIT FOUR
/* 35 */	0x0035,	//	DIGIT FIVE
/* 36 */	0x0036,	//	DIGIT SIX
/* 37 */	0x0037,	//	DIGIT SEVEN
/* 38 */	0x0038,	//	DIGIT EIGHT
/* 39 */	0x0039,	//	DIGIT NINE
/* 3A */	0x003A,	//	COLON
/* 3B */	0x003B,	//	SEMICOLON
/* 3C */	0x003C,	//	LESS-THAN SIGN
/* 3D */	0x003D,	//	EQUALS SIGN
/* 3E */	0x003E,	//	GREATER-THAN SIGN
/* 3F */	0x003F,	//	QUESTION MARK
/* 40 */	0x0040,	//	COMMERCIAL AT
/* 41 */	0x0041,	//	LATIN CAPITAL LETTER A
/* 42 */	0x0042,	//	LATIN CAPITAL LETTER B
/* 43 */	0x0043,	//	LATIN CAPITAL LETTER C
/* 44 */	0x0044,	//	LATIN CAPITAL LETTER D
/* 45 */	0x0045,	//	LATIN CAPITAL LETTER E
/* 46 */	0x0046,	//	LATIN CAPITAL LETTER F
/* 47 */	0x0047,	//	LATIN CAPITAL LETTER G
/* 48 */	0x0048,	//	LATIN CAPITAL LETTER H
/* 49 */	0x0049,	//	LATIN CAPITAL LETTER I
/* 4A */	0x004A,	//	LATIN CAPITAL LETTER J
/* 4B */	0x004B,	//	LATIN CAPITAL LETTER K
/* 4C */	0x004C,	//	LATIN CAPITAL LETTER L
/* 4D */	0x004D,	//	LATIN CAPITAL LETTER M
/* 4E */	0x004E,	//	LATIN CAPITAL LETTER N
/* 4F */	0x004F,	//	LATIN CAPITAL LETTER O
/* 50 */	0x0050,	//	LATIN CAPITAL LETTER P
/* 51 */	0x0051,	//	LATIN CAPITAL LETTER Q
/* 52 */	0x0052,	//	LATIN CAPITAL LETTER R
/* 53 */	0x0053,	//	LATIN CAPITAL LETTER S
/* 54 */	0x0054,	//	LATIN CAPITAL LETTER T
/* 55 */	0x0055,	//	LATIN CAPITAL LETTER U
/* 56 */	0x0056,	//	LATIN CAPITAL LETTER V
/* 57 */	0x0057,	//	LATIN CAPITAL LETTER W
/* 58 */	0x0058,	//	LATIN CAPITAL LETTER X
/* 59 */	0x0059,	//	LATIN CAPITAL LETTER Y
/* 5A */	0x005A,	//	LATIN CAPITAL LETTER Z
/* 5B */	0x005B,	//	LEFT SQUARE BRACKET
/* 5C */	0x005C,	//	REVERSE SOLIDUS
/* 5D */	0x005D,	//	RIGHT SQUARE BRACKET
/* 5E */	0x005E,	//	CIRCUMFLEX ACCENT
/* 5F */	0x005F,	//	LOW LINE
/* 60 */	0x0060,	//	GRAVE ACCENT
/* 61 */	0x0061,	//	LATIN SMALL LETTER A
/* 62 */	0x0062,	//	LATIN SMALL LETTER B
/* 63 */	0x0063,	//	LATIN SMALL LETTER C
/* 64 */	0x0064,	//	LATIN SMALL LETTER D
/* 65 */	0x0065,	//	LATIN SMALL LETTER E
/* 66 */	0x0066,	//	LATIN SMALL LETTER F
/* 67 */	0x0067,	//	LATIN SMALL LETTER G
/* 68 */	0x0068,	//	LATIN SMALL LETTER H
/* 69 */	0x0069,	//	LATIN SMALL LETTER I
/* 6A */	0x006A,	//	LATIN SMALL LETTER J
/* 6B */	0x006B,	//	LATIN SMALL LETTER K
/* 6C */	0x006C,	//	LATIN SMALL LETTER L
/* 6D */	0x006D,	//	LATIN SMALL LETTER M
/* 6E */	0x006E,	//	LATIN SMALL LETTER N
/* 6F */	0x006F,	//	LATIN SMALL LETTER O
/* 70 */	0x0070,	//	LATIN SMALL LETTER P
/* 71 */	0x0071,	//	LATIN SMALL LETTER Q
/* 72 */	0x0072,	//	LATIN SMALL LETTER R
/* 73 */	0x0073,	//	LATIN SMALL LETTER S
/* 74 */	0x0074,	//	LATIN SMALL LETTER T
/* 75 */	0x0075,	//	LATIN SMALL LETTER U
/* 76 */	0x0076,	//	LATIN SMALL LETTER V
/* 77 */	0x0077,	//	LATIN SMALL LETTER W
/* 78 */	0x0078,	//	LATIN SMALL LETTER X
/* 79 */	0x0079,	//	LATIN SMALL LETTER Y
/* 7A */	0x007A,	//	LATIN SMALL LETTER Z
/* 7B */	0x007B,	//	LEFT CURLY BRACKET
/* 7C */	0x007C,	//	VERTICAL LINE
/* 7D */	0x007D,	//	RIGHT CURLY BRACKET
/* 7E */	0x007E,	//	TILDE
/* 7F */	0x007F,	//	DELETE
/* 80 */	0x0080,	//	<control>
/* 81 */	0x0081,	//	<control>
/* 82 */	0x0082,	//	<control>
/* 83 */	0x0083,	//	<control>
/* 84 */	0x0084,	//	<control>
/* 85 */	0x0085,	//	<control>
/* 86 */	0x0086,	//	<control>
/* 87 */	0x0087,	//	<control>
/* 88 */	0x0088,	//	<control>
/* 89 */	0x0089,	//	<control>
/* 8A */	0x008A,	//	<control>
/* 8B */	0x008B,	//	<control>
/* 8C */	0x008C,	//	<control>
/* 8D */	0x008D,	//	<control>
/* 8E */	0x008E,	//	<control>
/* 8F */	0x008F,	//	<control>
/* 90 */	0x0090,	//	<control>
/* 91 */	0x0091,	//	<control>
/* 92 */	0x0092,	//	<control>
/* 93 */	0x0093,	//	<control>
/* 94 */	0x0094,	//	<control>
/* 95 */	0x0095,	//	<control>
/* 96 */	0x0096,	//	<control>
/* 97 */	0x0097,	//	<control>
/* 98 */	0x0098,	//	<control>
/* 99 */	0x0099,	//	<control>
/* 9A */	0x009A,	//	<control>
/* 9B */	0x009B,	//	<control>
/* 9C */	0x009C,	//	<control>
/* 9D */	0x009D,	//	<control>
/* 9E */	0x009E,	//	<control>
/* 9F */	0x009F,	//	<control>
/* A0 */	0x00A0,	//	NO-BREAK SPACE
/* A1 */	0x00A1,	//	INVERTED EXCLAMATION MARK
/* A2 */	0x00A2,	//	CENT SIGN
/* A3 */	0x00A3,	//	POUND SIGN
/* A4 */	0x20AC,	//	EURO SIGN
/* A5 */	0x00A5,	//	YEN SIGN
/* A6 */	0x0160,	//	LATIN CAPITAL LETTER S WITH CARON
/* A7 */	0x00A7,	//	SECTION SIGN
/* A8 */	0x0161,	//	LATIN SMALL LETTER S WITH CARON
/* A9 */	0x00A9,	//	COPYRIGHT SIGN
/* AA */	0x00AA,	//	FEMININE ORDINAL INDICATOR
/* AB */	0x00AB,	//	LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
/* AC */	0x00AC,	//	NOT SIGN
/* AD */	0x00AD,	//	SOFT HYPHEN
/* AE */	0x00AE,	//	REGISTERED SIGN
/* AF */	0x00AF,	//	MACRON
/* B0 */	0x00B0,	//	DEGREE SIGN
/* B1 */	0x00B1,	//	PLUS-MINUS SIGN
/* B2 */	0x00B2,	//	SUPERSCRIPT TWO
/* B3 */	0x00B3,	//	SUPERSCRIPT THREE
/* B4 */	0x017D,	//	LATIN CAPITAL LETTER Z WITH CARON
/* B5 */	0x00B5,	//	MICRO SIGN
/* B6 */	0x00B6,	//	PILCROW SIGN
/* B7 */	0x00B7,	//	MIDDLE DOT
/* B8 */	0x017E,	//	LATIN SMALL LETTER Z WITH CARON
/* B9 */	0x00B9,	//	SUPERSCRIPT ONE
/* BA */	0x00BA,	//	MASCULINE ORDINAL INDICATOR
/* BB */	0x00BB,	//	RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
/* BC */	0x0152,	//	LATIN CAPITAL LIGATURE OE
/* BD */	0x0153,	//	LATIN SMALL LIGATURE OE
/* BE */	0x0178,	//	LATIN CAPITAL LETTER Y WITH DIAERESIS
/* BF */	0x00BF,	//	INVERTED QUESTION MARK
/* C0 */	0x00C0,	//	LATIN CAPITAL LETTER A WITH GRAVE
/* C1 */	0x00C1,	//	LATIN CAPITAL LETTER A WITH ACUTE
/* C2 */	0x00C2,	//	LATIN CAPITAL LETTER A WITH CIRCUMFLEX
/* C3 */	0x00C3,	//	LATIN CAPITAL LETTER A WITH TILDE
/* C4 */	0x00C4,	//	LATIN CAPITAL LETTER A WITH DIAERESIS
/* C5 */	0x00C5,	//	LATIN CAPITAL LETTER A WITH RING ABOVE
/* C6 */	0x00C6,	//	LATIN CAPITAL LETTER AE
/* C7 */	0x00C7,	//	LATIN CAPITAL LETTER C WITH CEDILLA
/* C8 */	0x00C8,	//	LATIN CAPITAL LETTER E WITH GRAVE
/* C9 */	0x00C9,	//	LATIN CAPITAL LETTER E WITH ACUTE
/* CA */	0x00CA,	//	LATIN CAPITAL LETTER E WITH CIRCUMFLEX
/* CB */	0x00CB,	//	LATIN CAPITAL LETTER E WITH DIAERESIS
/* CC */	0x00CC,	//	LATIN CAPITAL LETTER I WITH GRAVE
/* CD */	0x00CD,	//	LATIN CAPITAL LETTER I WITH ACUTE
/* CE */	0x00CE,	//	LATIN CAPITAL LETTER I WITH CIRCUMFLEX
/* CF */	0x00CF,	//	LATIN CAPITAL LETTER I WITH DIAERESIS
/* D0 */	0x00D0,	//	LATIN CAPITAL LETTER ETH
/* D1 */	0x00D1,	//	LATIN CAPITAL LETTER N WITH TILDE
/* D2 */	0x00D2,	//	LATIN CAPITAL LETTER O WITH GRAVE
/* D3 */	0x00D3,	//	LATIN CAPITAL LETTER O WITH ACUTE
/* D4 */	0x00D4,	//	LATIN CAPITAL LETTER O WITH CIRCUMFLEX
/* D5 */	0x00D5,	//	LATIN CAPITAL LETTER O WITH TILDE
/* D6 */	0x00D6,	//	LATIN CAPITAL LETTER O WITH DIAERESIS
/* D7 */	0x00D7,	//	MULTIPLICATION SIGN
/* D8 */	0x00D8,	//	LATIN CAPITAL LETTER O WITH STROKE
/* D9 */	0x00D9,	//	LATIN CAPITAL LETTER U WITH GRAVE
/* DA */	0x00DA,	//	LATIN CAPITAL LETTER U WITH ACUTE
/* DB */	0x00DB,	//	LATIN CAPITAL LETTER U WITH CIRCUMFLEX
/* DC */	0x00DC,	//	LATIN CAPITAL LETTER U WITH DIAERESIS
/* DD */	0x00DD,	//	LATIN CAPITAL LETTER Y WITH ACUTE
/* DE */	0x00DE,	//	LATIN CAPITAL LETTER THORN
/* DF */	0x00DF,	//	LATIN SMALL LETTER SHARP S
/* E0 */	0x00E0,	//	LATIN SMALL LETTER A WITH GRAVE
/* E1 */	0x00E1,	//	LATIN SMALL LETTER A WITH ACUTE
/* E2 */	0x00E2,	//	LATIN SMALL LETTER A WITH CIRCUMFLEX
/* E3 */	0x00E3,	//	LATIN SMALL LETTER A WITH TILDE
/* E4 */	0x00E4,	//	LATIN SMALL LETTER A WITH DIAERESIS
/* E5 */	0x00E5,	//	LATIN SMALL LETTER A WITH RING ABOVE
/* E6 */	0x00E6,	//	LATIN SMALL LETTER AE
/* E7 */	0x00E7,	//	LATIN SMALL LETTER C WITH CEDILLA
/* E8 */	0x00E8,	//	LATIN SMALL LETTER E WITH GRAVE
/* E9 */	0x00E9,	//	LATIN SMALL LETTER E WITH ACUTE
/* EA */	0x00EA,	//	LATIN SMALL LETTER E WITH CIRCUMFLEX
/* EB */	0x00EB,	//	LATIN SMALL LETTER E WITH DIAERESIS
/* EC */	0x00EC,	//	LATIN SMALL LETTER I WITH GRAVE
/* ED */	0x00ED,	//	LATIN SMALL LETTER I WITH ACUTE
/* EE */	0x00EE,	//	LATIN SMALL LETTER I WITH CIRCUMFLEX
/* EF */	0x00EF,	//	LATIN SMALL LETTER I WITH DIAERESIS
/* F0 */	0x00F0,	//	LATIN SMALL LETTER ETH
/* F1 */	0x00F1,	//	LATIN SMALL LETTER N WITH TILDE
/* F2 */	0x00F2,	//	LATIN SMALL LETTER O WITH GRAVE
/* F3 */	0x00F3,	//	LATIN SMALL LETTER O WITH ACUTE
/* F4 */	0x00F4,	//	LATIN SMALL LETTER O WITH CIRCUMFLEX
/* F5 */	0x00F5,	//	LATIN SMALL LETTER O WITH TILDE
/* F6 */	0x00F6,	//	LATIN SMALL LETTER O WITH DIAERESIS
/* F7 */	0x00F7,	//	DIVISION SIGN
/* F8 */	0x00F8,	//	LATIN SMALL LETTER O WITH STROKE
/* F9 */	0x00F9,	//	LATIN SMALL LETTER U WITH GRAVE
/* FA */	0x00FA,	//	LATIN SMALL LETTER U WITH ACUTE
/* FB */	0x00FB,	//	LATIN SMALL LETTER U WITH CIRCUMFLEX
/* FC */	0x00FC,	//	LATIN SMALL LETTER U WITH DIAERESIS
/* FD */	0x00FD,	//	LATIN SMALL LETTER Y WITH ACUTE
/* FE */	0x00FE,	//	LATIN SMALL LETTER THORN
/* FF */	0x00FF	//	LATIN SMALL LETTER Y WITH DIAERESIS
};

/***************************************************************************
#
#	Name:             ISO/IEC 8859-16:2001 to Unicode
#	Unicode version:  3.0
#	Table version:    1.0
#	Table format:     Format A
#	Date:             2001 July 26
#	Authors:          Markus Kuhn <http://www.cl.cam.ac.uk/~mgk25/>
#
#	Copyright (c) 1999-2001 Unicode, Inc.  All Rights reserved.
#
#	This file is provided as-is by Unicode, Inc. (The Unicode Consortium).
#	No claims are made as to fitness for any particular purpose.  No
#	warranties of any kind are expressed or implied.  The recipient
#	agrees to determine applicability of information provided.  If this
#	file has been provided on optical media by Unicode, Inc., the sole
#	remedy for any claim will be exchange of defective media within 90
#	days of receipt.
#
#	Unicode, Inc. hereby grants the right to freely use the information
#	supplied in this file in the creation of products supporting the
#	Unicode Standard, and to make copies of this file in any form for
#	internal or external distribution as long as this notice remains
#	attached.
#
#	General notes:
#
#	This table contains the data the Unicode Consortium has on how
#       ISO/IEC 8859-16:2001 characters map into Unicode.
#
#	Format:  Three tab-separated columns
#		 Column #1 is the ISO/IEC 8859-16 code (in hex as 0xXX)
#		 Column #2 is the Unicode (in hex as 0xXXXX)
#		 Column #3 the Unicode name (follows a comment sign, '#')
#
#	The entries are in ISO/IEC 8859-16 order.
#
#	Updated versions of this file may be found in:
#		<ftp://ftp.unicode.org/Public/MAPPINGS/>
#
#	Any comments or problems, contact <errata@unicode.org>
#	Please note that <errata@unicode.org> is an archival address;
#	notices will be checked, but do not expect an immediate response.
#
***************************************************************************/
static const unicode_char lookup_iso8859_16[256] = {
/* 00 */	0x0000,	//	NULL
/* 01 */	0x0001,	//	START OF HEADING
/* 02 */	0x0002,	//	START OF TEXT
/* 03 */	0x0003,	//	END OF TEXT
/* 04 */	0x0004,	//	END OF TRANSMISSION
/* 05 */	0x0005,	//	ENQUIRY
/* 06 */	0x0006,	//	ACKNOWLEDGE
/* 07 */	0x0007,	//	BELL
/* 08 */	0x0008,	//	BACKSPACE
/* 09 */	0x0009,	//	HORIZONTAL TABULATION
/* 0A */	0x000A,	//	LINE FEED
/* 0B */	0x000B,	//	VERTICAL TABULATION
/* 0C */	0x000C,	//	FORM FEED
/* 0D */	0x000D,	//	CARRIAGE RETURN
/* 0E */	0x000E,	//	SHIFT OUT
/* 0F */	0x000F,	//	SHIFT IN
/* 10 */	0x0010,	//	DATA LINK ESCAPE
/* 11 */	0x0011,	//	DEVICE CONTROL ONE
/* 12 */	0x0012,	//	DEVICE CONTROL TWO
/* 13 */	0x0013,	//	DEVICE CONTROL THREE
/* 14 */	0x0014,	//	DEVICE CONTROL FOUR
/* 15 */	0x0015,	//	NEGATIVE ACKNOWLEDGE
/* 16 */	0x0016,	//	SYNCHRONOUS IDLE
/* 17 */	0x0017,	//	END OF TRANSMISSION BLOCK
/* 18 */	0x0018,	//	CANCEL
/* 19 */	0x0019,	//	END OF MEDIUM
/* 1A */	0x001A,	//	SUBSTITUTE
/* 1B */	0x001B,	//	ESCAPE
/* 1C */	0x001C,	//	FILE SEPARATOR
/* 1D */	0x001D,	//	GROUP SEPARATOR
/* 1E */	0x001E,	//	RECORD SEPARATOR
/* 1F */	0x001F,	//	UNIT SEPARATOR
/* 20 */	0x0020,	//	SPACE
/* 21 */	0x0021,	//	EXCLAMATION MARK
/* 22 */	0x0022,	//	QUOTATION MARK
/* 23 */	0x0023,	//	NUMBER SIGN
/* 24 */	0x0024,	//	DOLLAR SIGN
/* 25 */	0x0025,	//	PERCENT SIGN
/* 26 */	0x0026,	//	AMPERSAND
/* 27 */	0x0027,	//	APOSTROPHE
/* 28 */	0x0028,	//	LEFT PARENTHESIS
/* 29 */	0x0029,	//	RIGHT PARENTHESIS
/* 2A */	0x002A,	//	ASTERISK
/* 2B */	0x002B,	//	PLUS SIGN
/* 2C */	0x002C,	//	COMMA
/* 2D */	0x002D,	//	HYPHEN-MINUS
/* 2E */	0x002E,	//	FULL STOP
/* 2F */	0x002F,	//	SOLIDUS
/* 30 */	0x0030,	//	DIGIT ZERO
/* 31 */	0x0031,	//	DIGIT ONE
/* 32 */	0x0032,	//	DIGIT TWO
/* 33 */	0x0033,	//	DIGIT THREE
/* 34 */	0x0034,	//	DIGIT FOUR
/* 35 */	0x0035,	//	DIGIT FIVE
/* 36 */	0x0036,	//	DIGIT SIX
/* 37 */	0x0037,	//	DIGIT SEVEN
/* 38 */	0x0038,	//	DIGIT EIGHT
/* 39 */	0x0039,	//	DIGIT NINE
/* 3A */	0x003A,	//	COLON
/* 3B */	0x003B,	//	SEMICOLON
/* 3C */	0x003C,	//	LESS-THAN SIGN
/* 3D */	0x003D,	//	EQUALS SIGN
/* 3E */	0x003E,	//	GREATER-THAN SIGN
/* 3F */	0x003F,	//	QUESTION MARK
/* 40 */	0x0040,	//	COMMERCIAL AT
/* 41 */	0x0041,	//	LATIN CAPITAL LETTER A
/* 42 */	0x0042,	//	LATIN CAPITAL LETTER B
/* 43 */	0x0043,	//	LATIN CAPITAL LETTER C
/* 44 */	0x0044,	//	LATIN CAPITAL LETTER D
/* 45 */	0x0045,	//	LATIN CAPITAL LETTER E
/* 46 */	0x0046,	//	LATIN CAPITAL LETTER F
/* 47 */	0x0047,	//	LATIN CAPITAL LETTER G
/* 48 */	0x0048,	//	LATIN CAPITAL LETTER H
/* 49 */	0x0049,	//	LATIN CAPITAL LETTER I
/* 4A */	0x004A,	//	LATIN CAPITAL LETTER J
/* 4B */	0x004B,	//	LATIN CAPITAL LETTER K
/* 4C */	0x004C,	//	LATIN CAPITAL LETTER L
/* 4D */	0x004D,	//	LATIN CAPITAL LETTER M
/* 4E */	0x004E,	//	LATIN CAPITAL LETTER N
/* 4F */	0x004F,	//	LATIN CAPITAL LETTER O
/* 50 */	0x0050,	//	LATIN CAPITAL LETTER P
/* 51 */	0x0051,	//	LATIN CAPITAL LETTER Q
/* 52 */	0x0052,	//	LATIN CAPITAL LETTER R
/* 53 */	0x0053,	//	LATIN CAPITAL LETTER S
/* 54 */	0x0054,	//	LATIN CAPITAL LETTER T
/* 55 */	0x0055,	//	LATIN CAPITAL LETTER U
/* 56 */	0x0056,	//	LATIN CAPITAL LETTER V
/* 57 */	0x0057,	//	LATIN CAPITAL LETTER W
/* 58 */	0x0058,	//	LATIN CAPITAL LETTER X
/* 59 */	0x0059,	//	LATIN CAPITAL LETTER Y
/* 5A */	0x005A,	//	LATIN CAPITAL LETTER Z
/* 5B */	0x005B,	//	LEFT SQUARE BRACKET
/* 5C */	0x005C,	//	REVERSE SOLIDUS
/* 5D */	0x005D,	//	RIGHT SQUARE BRACKET
/* 5E */	0x005E,	//	CIRCUMFLEX ACCENT
/* 5F */	0x005F,	//	LOW LINE
/* 60 */	0x0060,	//	GRAVE ACCENT
/* 61 */	0x0061,	//	LATIN SMALL LETTER A
/* 62 */	0x0062,	//	LATIN SMALL LETTER B
/* 63 */	0x0063,	//	LATIN SMALL LETTER C
/* 64 */	0x0064,	//	LATIN SMALL LETTER D
/* 65 */	0x0065,	//	LATIN SMALL LETTER E
/* 66 */	0x0066,	//	LATIN SMALL LETTER F
/* 67 */	0x0067,	//	LATIN SMALL LETTER G
/* 68 */	0x0068,	//	LATIN SMALL LETTER H
/* 69 */	0x0069,	//	LATIN SMALL LETTER I
/* 6A */	0x006A,	//	LATIN SMALL LETTER J
/* 6B */	0x006B,	//	LATIN SMALL LETTER K
/* 6C */	0x006C,	//	LATIN SMALL LETTER L
/* 6D */	0x006D,	//	LATIN SMALL LETTER M
/* 6E */	0x006E,	//	LATIN SMALL LETTER N
/* 6F */	0x006F,	//	LATIN SMALL LETTER O
/* 70 */	0x0070,	//	LATIN SMALL LETTER P
/* 71 */	0x0071,	//	LATIN SMALL LETTER Q
/* 72 */	0x0072,	//	LATIN SMALL LETTER R
/* 73 */	0x0073,	//	LATIN SMALL LETTER S
/* 74 */	0x0074,	//	LATIN SMALL LETTER T
/* 75 */	0x0075,	//	LATIN SMALL LETTER U
/* 76 */	0x0076,	//	LATIN SMALL LETTER V
/* 77 */	0x0077,	//	LATIN SMALL LETTER W
/* 78 */	0x0078,	//	LATIN SMALL LETTER X
/* 79 */	0x0079,	//	LATIN SMALL LETTER Y
/* 7A */	0x007A,	//	LATIN SMALL LETTER Z
/* 7B */	0x007B,	//	LEFT CURLY BRACKET
/* 7C */	0x007C,	//	VERTICAL LINE
/* 7D */	0x007D,	//	RIGHT CURLY BRACKET
/* 7E */	0x007E,	//	TILDE
/* 7F */	0x007F,	//	DELETE
/* 80 */	0x0080,	//	<control>
/* 81 */	0x0081,	//	<control>
/* 82 */	0x0082,	//	<control>
/* 83 */	0x0083,	//	<control>
/* 84 */	0x0084,	//	<control>
/* 85 */	0x0085,	//	<control>
/* 86 */	0x0086,	//	<control>
/* 87 */	0x0087,	//	<control>
/* 88 */	0x0088,	//	<control>
/* 89 */	0x0089,	//	<control>
/* 8A */	0x008A,	//	<control>
/* 8B */	0x008B,	//	<control>
/* 8C */	0x008C,	//	<control>
/* 8D */	0x008D,	//	<control>
/* 8E */	0x008E,	//	<control>
/* 8F */	0x008F,	//	<control>
/* 90 */	0x0090,	//	<control>
/* 91 */	0x0091,	//	<control>
/* 92 */	0x0092,	//	<control>
/* 93 */	0x0093,	//	<control>
/* 94 */	0x0094,	//	<control>
/* 95 */	0x0095,	//	<control>
/* 96 */	0x0096,	//	<control>
/* 97 */	0x0097,	//	<control>
/* 98 */	0x0098,	//	<control>
/* 99 */	0x0099,	//	<control>
/* 9A */	0x009A,	//	<control>
/* 9B */	0x009B,	//	<control>
/* 9C */	0x009C,	//	<control>
/* 9D */	0x009D,	//	<control>
/* 9E */	0x009E,	//	<control>
/* 9F */	0x009F,	//	<control>
/* A0 */	0x00A0,	//	NO-BREAK SPACE
/* A1 */	0x0104,	//	LATIN CAPITAL LETTER A WITH OGONEK
/* A2 */	0x0105,	//	LATIN SMALL LETTER A WITH OGONEK
/* A3 */	0x0141,	//	LATIN CAPITAL LETTER L WITH STROKE
/* A4 */	0x20AC,	//	EURO SIGN
/* A5 */	0x201E,	//	DOUBLE LOW-9 QUOTATION MARK
/* A6 */	0x0160,	//	LATIN CAPITAL LETTER S WITH CARON
/* A7 */	0x00A7,	//	SECTION SIGN
/* A8 */	0x0161,	//	LATIN SMALL LETTER S WITH CARON
/* A9 */	0x00A9,	//	COPYRIGHT SIGN
/* AA */	0x0218,	//	LATIN CAPITAL LETTER S WITH COMMA BELOW
/* AB */	0x00AB,	//	LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
/* AC */	0x0179,	//	LATIN CAPITAL LETTER Z WITH ACUTE
/* AD */	0x00AD,	//	SOFT HYPHEN
/* AE */	0x017A,	//	LATIN SMALL LETTER Z WITH ACUTE
/* AF */	0x017B,	//	LATIN CAPITAL LETTER Z WITH DOT ABOVE
/* B0 */	0x00B0,	//	DEGREE SIGN
/* B1 */	0x00B1,	//	PLUS-MINUS SIGN
/* B2 */	0x010C,	//	LATIN CAPITAL LETTER C WITH CARON
/* B3 */	0x0142,	//	LATIN SMALL LETTER L WITH STROKE
/* B4 */	0x017D,	//	LATIN CAPITAL LETTER Z WITH CARON
/* B5 */	0x201D,	//	RIGHT DOUBLE QUOTATION MARK
/* B6 */	0x00B6,	//	PILCROW SIGN
/* B7 */	0x00B7,	//	MIDDLE DOT
/* B8 */	0x017E,	//	LATIN SMALL LETTER Z WITH CARON
/* B9 */	0x010D,	//	LATIN SMALL LETTER C WITH CARON
/* BA */	0x0219,	//	LATIN SMALL LETTER S WITH COMMA BELOW
/* BB */	0x00BB,	//	RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
/* BC */	0x0152,	//	LATIN CAPITAL LIGATURE OE
/* BD */	0x0153,	//	LATIN SMALL LIGATURE OE
/* BE */	0x0178,	//	LATIN CAPITAL LETTER Y WITH DIAERESIS
/* BF */	0x017C,	//	LATIN SMALL LETTER Z WITH DOT ABOVE
/* C0 */	0x00C0,	//	LATIN CAPITAL LETTER A WITH GRAVE
/* C1 */	0x00C1,	//	LATIN CAPITAL LETTER A WITH ACUTE
/* C2 */	0x00C2,	//	LATIN CAPITAL LETTER A WITH CIRCUMFLEX
/* C3 */	0x0102,	//	LATIN CAPITAL LETTER A WITH BREVE
/* C4 */	0x00C4,	//	LATIN CAPITAL LETTER A WITH DIAERESIS
/* C5 */	0x0106,	//	LATIN CAPITAL LETTER C WITH ACUTE
/* C6 */	0x00C6,	//	LATIN CAPITAL LETTER AE
/* C7 */	0x00C7,	//	LATIN CAPITAL LETTER C WITH CEDILLA
/* C8 */	0x00C8,	//	LATIN CAPITAL LETTER E WITH GRAVE
/* C9 */	0x00C9,	//	LATIN CAPITAL LETTER E WITH ACUTE
/* CA */	0x00CA,	//	LATIN CAPITAL LETTER E WITH CIRCUMFLEX
/* CB */	0x00CB,	//	LATIN CAPITAL LETTER E WITH DIAERESIS
/* CC */	0x00CC,	//	LATIN CAPITAL LETTER I WITH GRAVE
/* CD */	0x00CD,	//	LATIN CAPITAL LETTER I WITH ACUTE
/* CE */	0x00CE,	//	LATIN CAPITAL LETTER I WITH CIRCUMFLEX
/* CF */	0x00CF,	//	LATIN CAPITAL LETTER I WITH DIAERESIS
/* D0 */	0x0110,	//	LATIN CAPITAL LETTER D WITH STROKE
/* D1 */	0x0143,	//	LATIN CAPITAL LETTER N WITH ACUTE
/* D2 */	0x00D2,	//	LATIN CAPITAL LETTER O WITH GRAVE
/* D3 */	0x00D3,	//	LATIN CAPITAL LETTER O WITH ACUTE
/* D4 */	0x00D4,	//	LATIN CAPITAL LETTER O WITH CIRCUMFLEX
/* D5 */	0x0150,	//	LATIN CAPITAL LETTER O WITH DOUBLE ACUTE
/* D6 */	0x00D6,	//	LATIN CAPITAL LETTER O WITH DIAERESIS
/* D7 */	0x015A,	//	LATIN CAPITAL LETTER S WITH ACUTE
/* D8 */	0x0170,	//	LATIN CAPITAL LETTER U WITH DOUBLE ACUTE
/* D9 */	0x00D9,	//	LATIN CAPITAL LETTER U WITH GRAVE
/* DA */	0x00DA,	//	LATIN CAPITAL LETTER U WITH ACUTE
/* DB */	0x00DB,	//	LATIN CAPITAL LETTER U WITH CIRCUMFLEX
/* DC */	0x00DC,	//	LATIN CAPITAL LETTER U WITH DIAERESIS
/* DD */	0x0118,	//	LATIN CAPITAL LETTER E WITH OGONEK
/* DE */	0x021A,	//	LATIN CAPITAL LETTER T WITH COMMA BELOW
/* DF */	0x00DF,	//	LATIN SMALL LETTER SHARP S
/* E0 */	0x00E0,	//	LATIN SMALL LETTER A WITH GRAVE
/* E1 */	0x00E1,	//	LATIN SMALL LETTER A WITH ACUTE
/* E2 */	0x00E2,	//	LATIN SMALL LETTER A WITH CIRCUMFLEX
/* E3 */	0x0103,	//	LATIN SMALL LETTER A WITH BREVE
/* E4 */	0x00E4,	//	LATIN SMALL LETTER A WITH DIAERESIS
/* E5 */	0x0107,	//	LATIN SMALL LETTER C WITH ACUTE
/* E6 */	0x00E6,	//	LATIN SMALL LETTER AE
/* E7 */	0x00E7,	//	LATIN SMALL LETTER C WITH CEDILLA
/* E8 */	0x00E8,	//	LATIN SMALL LETTER E WITH GRAVE
/* E9 */	0x00E9,	//	LATIN SMALL LETTER E WITH ACUTE
/* EA */	0x00EA,	//	LATIN SMALL LETTER E WITH CIRCUMFLEX
/* EB */	0x00EB,	//	LATIN SMALL LETTER E WITH DIAERESIS
/* EC */	0x00EC,	//	LATIN SMALL LETTER I WITH GRAVE
/* ED */	0x00ED,	//	LATIN SMALL LETTER I WITH ACUTE
/* EE */	0x00EE,	//	LATIN SMALL LETTER I WITH CIRCUMFLEX
/* EF */	0x00EF,	//	LATIN SMALL LETTER I WITH DIAERESIS
/* F0 */	0x0111,	//	LATIN SMALL LETTER D WITH STROKE
/* F1 */	0x0144,	//	LATIN SMALL LETTER N WITH ACUTE
/* F2 */	0x00F2,	//	LATIN SMALL LETTER O WITH GRAVE
/* F3 */	0x00F3,	//	LATIN SMALL LETTER O WITH ACUTE
/* F4 */	0x00F4,	//	LATIN SMALL LETTER O WITH CIRCUMFLEX
/* F5 */	0x0151,	//	LATIN SMALL LETTER O WITH DOUBLE ACUTE
/* F6 */	0x00F6,	//	LATIN SMALL LETTER O WITH DIAERESIS
/* F7 */	0x015B,	//	LATIN SMALL LETTER S WITH ACUTE
/* F8 */	0x0171,	//	LATIN SMALL LETTER U WITH DOUBLE ACUTE
/* F9 */	0x00F9,	//	LATIN SMALL LETTER U WITH GRAVE
/* FA */	0x00FA,	//	LATIN SMALL LETTER U WITH ACUTE
/* FB */	0x00FB,	//	LATIN SMALL LETTER U WITH CIRCUMFLEX
/* FC */	0x00FC,	//	LATIN SMALL LETTER U WITH DIAERESIS
/* FD */	0x0119,	//	LATIN SMALL LETTER E WITH OGONEK
/* FE */	0x021B,	//	LATIN SMALL LETTER T WITH COMMA BELOW
/* FF */	0x00FF	//	LATIN SMALL LETTER Y WITH DIAERESIS
};

/***************************************************************************
# File encoding:    UTF-8
# Name:             AtariST to Unicode
# Unicode version:  4.0
# Table version:    1.2
# Table format:     Format A
# Date:             2011 October 27
# Authors:          Philippe Verdy <verdyp AT gmail.com>
#                   Bruno Haible <bruno AT clisp.org>
#
# Copyright (c) 1998 - 2011 Unicode, Inc.  All Rights reserved.
#
# This file is provided as-is by Unicode, Inc. (The Unicode Consortium).
# No claims are made as to fitness for any particular purpose.  No
# warranties of any kind are expressed or implied.  The recipient
# agrees to determine applicability of information provided.  If this
# file has been provided on optical media by Unicode, Inc., the sole
# remedy for any claim will be exchange of defective media within 90
# days of receipt.
#
# Unicode, Inc. hereby grants the right to freely use the information
# supplied in this file in the creation of products supporting the
# Unicode Standard, and to make copies of this file in any form for
# internal or external distribution as long as this notice remains
# attached.
#
# General notes:
#
# This table contains the data the Unicode Consortium has on how
# Atari ST/TT (TOS) characters map into Unicode.
#
# Format:  Three tab-separated columns
#   Column 1 is the Atari ST code (in hex as 0xXX)
#   Column 2 is the Unicode (in hex as 0xXXXX)
#   Column 3 is a comment field containing:
#            - the Unicode name (follows a comment sign, '#'),
#            - the actual character or <symbol> (follows a comment sign, '#'),
#            - some backward compatible character set mappings.
#
# The entries are in Atari ST order.
#
# Version history
# 1.2 Thanks to Ulf Zibis for his report (no mapping changes).
#     - Fix the correct Unicode character names for
#       code points 0x00C6, 0x00E6, 0x00D6, 0x00DC, 0x00A8, 0x00B4.
#     - Fix editorial issues from from the previous HTML reporting form.
#     - Document that the file is UTF-8 encoded.
# 1.1 Thanks to Bruno Haible.
#     - swap the positions of GREEK SMALL LETTER BETA and LATIN SHARP S
#       on code positions 0x9E and 0xE1. So AtariST mapped the LATIN SHARP S
#       differently than PC codepages 437 and 850 (was it true on the TT in 1992?).
#     - Fix the correct code point 0x00B3 instead of 0x22B3 for SUPERSCRIPT THREE,
#       on code position 0xFE.
#     - Fix the spelling of a "GREEP" letter name to "GREEK" (no code changed)
#     - Change the mapping of code 0xEE from GREEK SMALL LETTER EPSILON to the
#       mathematical ELEMENT OF SIGN, on code position 0xEE.
# 1.0 version new, by Philippe Verdy.
#
# Updated versions of this file may be found in:
#   <ftp://ftp.unicode.org/Public/MAPPINGS/>
#
# Any comments or problems, contact http://www.unicode.org/reporting.html
#
***************************************************************************/
static const unicode_char lookup_atari_st[256] = {
/* 00 */	0x0000,	// NULL                                       <NUL>	ISO646
/* 01 */	0x0001,	// START OF HEADING                           <SOH>	ISO646
/* 02 */	0x0002,	// START OF TEXT                              <STX>	ISO646
/* 03 */	0x0003,	// END OF TEXT                                <ETX>	ISO646
/* 04 */	0x0004,	// END OF TRANSMISSION                        <EOT>	ISO646
/* 05 */	0x0005,	// ENQUIRY                                    <ENQ>	ISO646
/* 06 */	0x0006,	// ACKNOWLEDGE                                <ACK>	ISO646
/* 07 */	0x0007,	// BELL                                       <BEL>	ISO646
/* 08 */	0x0008,	// BACKSPACE                                  <BS>	ISO646
/* 09 */	0x0009,	// HORIZONTAL TABULATION                      <TAB>	ISO646
/* 0A */	0x000A,	// LINE FEED                                  <LF>	ISO646
/* 0B */	0x000B,	// VERTICAL TABULATION                        <VT>	ISO646
/* 0C */	0x000C,	// FORM FEED                                  <FF>	ISO646
/* 0D */	0x000D,	// CARRIAGE RETURN                            <CR>	ISO646
/* 0E */	0x000E,	// SHIFT OUT                                  <SO>	ISO646
/* 0F */	0x000F,	// SHIFT IN                                   <SI>	ISO646
/* 10 */	0x0010,	// DATA LINK ESCAPE                           <DLE>	ISO646
/* 11 */	0x0011,	// DEVICE CONTROL ONE                         <DC1>	ISO646
/* 12 */	0x0012,	// DEVICE CONTROL TWO                         <DC2>	ISO646
/* 13 */	0x0013,	// DEVICE CONTROL THREE                       <DC3>	ISO646
/* 14 */	0x0014,	// DEVICE CONTROL FOUR                        <DC4>	ISO646
/* 15 */	0x0015,	// NEGATIVE ACKNOWLEDGE                       <NAK>	ISO646
/* 16 */	0x0016,	// SYNCHRONOUS IDLE                           <SYN>	ISO646
/* 17 */	0x0017,	// END OF TRANSMISSION BLOCK                  <ETB>	ISO646
/* 18 */	0x0018,	// CANCEL                                     <CAN>	ISO646
/* 19 */	0x0019,	// END OF MEDIUM                              <EM>	ISO646
/* 1A */	0x001A,	// SUBSTITUTE                                 <SUB>	ISO646
/* 1B */	0x001B,	// ESCAPE                                     <ESC>	ISO646
/* 1C */	0x001C,	// FILE SEPARATOR                             <FS>	ISO646
/* 1D */	0x001D,	// GROUP SEPARATOR                            <GS>	ISO646
/* 1E */	0x001E,	// RECORD SEPARATOR                           <RS>	ISO646
/* 1F */	0x001F,	// UNIT SEPARATOR                             <US>	ISO646
/* 20 */	0x0020,	// SPACE                                      <SP>	ISO646
/* 21 */	0x0021,	// EXCLAMATION MARK                           !	ASCII
/* 22 */	0x0022,	// QUOTATION MARK                             "	ASCII
/* 23 */	0x0023,	// NUMBER SIGN                                #	ASCII
/* 24 */	0x0024,	// DOLLAR SIGN                                $	ASCII
/* 25 */	0x0025,	// PERCENT SIGN                               %	ASCII
/* 26 */	0x0026,	// AMPERSAND                                  &	ASCII
/* 27 */	0x0027,	// APOSTROPHE                                 '	ASCII
/* 28 */	0x0028,	// LEFT PARENTHESIS                           (	ASCII
/* 29 */	0x0029,	// RIGHT PARENTHESIS                          )	ASCII
/* 2A */	0x002A,	// ASTERISK                                   *	ASCII
/* 2B */	0x002B,	// PLUS SIGN                                  +	ASCII
/* 2C */	0x002C,	// COMMA                                      ,	ASCII
/* 2D */	0x002D,	// HYPHEN-MINUS                               -	ASCII
/* 2E */	0x002E,	// FULL STOP                                  .	ASCII
/* 2F */	0x002F,	// SOLIDUS                                    /	ASCII
/* 30 */	0x0030,	// DIGIT ZERO                                 0	ASCII
/* 31 */	0x0031,	// DIGIT ONE                                  1	ASCII
/* 32 */	0x0032,	// DIGIT TWO                                  2	ASCII
/* 33 */	0x0033,	// DIGIT THREE                                3	ASCII
/* 34 */	0x0034,	// DIGIT FOUR                                 4	ASCII
/* 35 */	0x0035,	// DIGIT FIVE                                 5	ASCII
/* 36 */	0x0036,	// DIGIT SIX                                  6	ASCII
/* 37 */	0x0037,	// DIGIT SEVEN                                7	ASCII
/* 38 */	0x0038,	// DIGIT EIGHT                                8	ASCII
/* 39 */	0x0039,	// DIGIT NINE                                 9	ASCII
/* 3A */	0x003A,	// COLON                                      :	ASCII
/* 3B */	0x003B,	// SEMICOLON                                  ;	ASCII
/* 3C */	0x003C,	// LESS-THAN SIGN                             <	ASCII
/* 3D */	0x003D,	// EQUALS SIGN                                =	ASCII
/* 3E */	0x003E,	// GREATER-THAN SIGN                          >	ASCII
/* 3F */	0x003F,	// QUESTION MARK                              ?	ASCII
/* 40 */	0x0040,	// COMMERCIAL AT                              @	ASCII
/* 41 */	0x0041,	// LATIN CAPITAL LETTER A                     A	ASCII
/* 42 */	0x0042,	// LATIN CAPITAL LETTER B                     B	ASCII
/* 43 */	0x0043,	// LATIN CAPITAL LETTER C                     C	ASCII
/* 44 */	0x0044,	// LATIN CAPITAL LETTER D                     D	ASCII
/* 45 */	0x0045,	// LATIN CAPITAL LETTER E                     E	ASCII
/* 46 */	0x0046,	// LATIN CAPITAL LETTER F                     F	ASCII
/* 47 */	0x0047,	// LATIN CAPITAL LETTER G                     G	ASCII
/* 48 */	0x0048,	// LATIN CAPITAL LETTER H                     H	ASCII
/* 49 */	0x0049,	// LATIN CAPITAL LETTER I                     I	ASCII
/* 4A */	0x004A,	// LATIN CAPITAL LETTER J                     J	ASCII
/* 4B */	0x004B,	// LATIN CAPITAL LETTER K                     K	ASCII
/* 4C */	0x004C,	// LATIN CAPITAL LETTER L                     L	ASCII
/* 4D */	0x004D,	// LATIN CAPITAL LETTER M                     M	ASCII
/* 4E */	0x004E,	// LATIN CAPITAL LETTER N                     N	ASCII
/* 4F */	0x004F,	// LATIN CAPITAL LETTER O                     O	ASCII
/* 50 */	0x0050,	// LATIN CAPITAL LETTER P                     P	ASCII
/* 51 */	0x0051,	// LATIN CAPITAL LETTER Q                     Q	ASCII
/* 52 */	0x0052,	// LATIN CAPITAL LETTER R                     R	ASCII
/* 53 */	0x0053,	// LATIN CAPITAL LETTER S                     S	ASCII
/* 54 */	0x0054,	// LATIN CAPITAL LETTER T                     T	ASCII
/* 55 */	0x0055,	// LATIN CAPITAL LETTER U                     U	ASCII
/* 56 */	0x0056,	// LATIN CAPITAL LETTER V                     V	ASCII
/* 57 */	0x0057,	// LATIN CAPITAL LETTER W                     W	ASCII
/* 58 */	0x0058,	// LATIN CAPITAL LETTER X                     X	ASCII
/* 59 */	0x0059,	// LATIN CAPITAL LETTER Y                     Y	ASCII
/* 5A */	0x005A,	// LATIN CAPITAL LETTER Z                     Z	ASCII
/* 5B */	0x005B,	// LEFT SQUARE BRACKET                        [	ASCII
/* 5C */	0x005C,	// REVERSE SOLIDUS                            \	ASCII
/* 5D */	0x005D,	// RIGHT SQUARE BRACKET                       ]	ASCII
/* 5E */	0x005E,	// CIRCUMFLEX ACCENT                          ^	ASCII
/* 5F */	0x005F,	// LOW LINE                                   _	ASCII
/* 60 */	0x0060,	// GRAVE ACCENT                               `	ASCII
/* 61 */	0x0061,	// LATIN SMALL LETTER A                       a	ASCII
/* 62 */	0x0062,	// LATIN SMALL LETTER B                       b	ASCII
/* 63 */	0x0063,	// LATIN SMALL LETTER C                       c	ASCII
/* 64 */	0x0064,	// LATIN SMALL LETTER D                       d	ASCII
/* 65 */	0x0065,	// LATIN SMALL LETTER E                       e	ASCII
/* 66 */	0x0066,	// LATIN SMALL LETTER F                       f	ASCII
/* 67 */	0x0067,	// LATIN SMALL LETTER G                       g	ASCII
/* 68 */	0x0068,	// LATIN SMALL LETTER H                       h	ASCII
/* 69 */	0x0069,	// LATIN SMALL LETTER I                       i	ASCII
/* 6A */	0x006A,	// LATIN SMALL LETTER J                       j	ASCII
/* 6B */	0x006B,	// LATIN SMALL LETTER K                       k	ASCII
/* 6C */	0x006C,	// LATIN SMALL LETTER L                       l	ASCII
/* 6D */	0x006D,	// LATIN SMALL LETTER M                       m	ASCII
/* 6E */	0x006E,	// LATIN SMALL LETTER N                       n	ASCII
/* 6F */	0x006F,	// LATIN SMALL LETTER O                       o	ASCII
/* 70 */	0x0070,	// LATIN SMALL LETTER P                       p	ASCII
/* 71 */	0x0071,	// LATIN SMALL LETTER Q                       q	ASCII
/* 72 */	0x0072,	// LATIN SMALL LETTER R                       r	ASCII
/* 73 */	0x0073,	// LATIN SMALL LETTER S                       s	ASCII
/* 74 */	0x0074,	// LATIN SMALL LETTER T                       t	ASCII
/* 75 */	0x0075,	// LATIN SMALL LETTER U                       u	ASCII
/* 76 */	0x0076,	// LATIN SMALL LETTER V                       v	ASCII
/* 77 */	0x0077,	// LATIN SMALL LETTER W                       w	ASCII
/* 78 */	0x0078,	// LATIN SMALL LETTER X                       x	ASCII
/* 79 */	0x0079,	// LATIN SMALL LETTER Y                       y	ASCII
/* 7A */	0x007A,	// LATIN SMALL LETTER Z                       z	ASCII
/* 7B */	0x007B,	// LEFT CURLY BRACKET                         {	ASCII
/* 7C */	0x007C,	// VERTICAL LINE                              |	ASCII
/* 7D */	0x007D,	// RIGHT CURLY BRACKET                        }	ASCII
/* 7E */	0x007E,	// TILDE                                      ~	ASCII
/* 7F */	0x007F,	// DELETE                                     <DEL>	ISO646
/* 80 */	0x00C7,	// LATIN CAPITAL LETTER C WITH CEDILLA        Ç	CP437,CP850
/* 81 */	0x00FC,	// LATIN SMALL LETTER U WITH DIAERESIS        ü	CP437,CP850
/* 82 */	0x00E9,	// LATIN SMALL LETTER E WITH ACUTE            é	CP437,CP850
/* 83 */	0x00E2,	// LATIN SMALL LETTER A WITH CIRCUMFLEX       â	CP437,CP850
/* 84 */	0x00E4,	// LATIN SMALL LETTER A WITH DIAERESIS        ä	CP437,CP850
/* 85 */	0x00E0,	// LATIN SMALL LETTER A WITH GRAVE            à	CP437,CP850
/* 86 */	0x00E5,	// LATIN SMALL LETTER A WITH RING ABOVE       å	CP437,CP850
/* 87 */	0x00E7,	// LATIN SMALL LETTER C WITH CEDILLA          ç	CP437,CP850
/* 88 */	0x00EA,	// LATIN SMALL LETTER E WITH CIRCUMFLEX       ê	CP437,CP850
/* 89 */	0x00EB,	// LATIN SMALL LETTER E WITH DIAERESIS        ë	CP437,CP850
/* 8A */	0x00E8,	// LATIN SMALL LETTER E WITH GRAVE            è	CP437,CP850
/* 8B */	0x00EF,	// LATIN SMALL LETTER I WITH DIAERESIS        ï	CP437,CP850
/* 8C */	0x00EE,	// LATIN SMALL LETTER I WITH CIRCUMFLEX       î	CP437,CP850
/* 8D */	0x00EC,	// LATIN SMALL LETTER I WITH GRAVE            ì	CP437,CP850
/* 8E */	0x00C4,	// LATIN CAPITAL LETTER A WITH DIAERESIS      Ä	CP437,CP850
/* 8F */	0x00C5,	// LATIN CAPITAL LETTER A WITH RING ABOVE     Å	CP437,CP850
/* 90 */	0x00C9,	// LATIN CAPITAL LETTER E WITH ACUTE          É	CP437,CP850
/* 91 */	0x00E6,	// LATIN SMALL LETTER AE                      æ	CP437,CP850
/* 92 */	0x00C6,	// LATIN CAPITAL LETTER AE                    Æ	CP437,CP850
/* 93 */	0x00F4,	// LATIN SMALL LETTER O WITH CIRCUMFLEX       ô	CP437,CP850
/* 94 */	0x00F6,	// LATIN SMALL LETTER O WITH DIAERESIS        ö	CP437,CP850
/* 95 */	0x00F2,	// LATIN SMALL LETTER O WITH GRAVE            ò	CP437,CP850
/* 96 */	0x00FB,	// LATIN SMALL LETTER U WITH CIRCUMFLEX       û	CP437,CP850
/* 97 */	0x00F9,	// LATIN SMALL LETTER U WITH GRAVE            ù	CP437,CP850
/* 98 */	0x00FF,	// LATIN SMALL LETTER Y WITH DIAERESIS        ÿ	CP437,CP850
/* 99 */	0x00D6,	// LATIN CAPITAL LETTER O WITH DIAERESIS      Ö	CP437,CP850
/* 9A */	0x00DC,	// LATIN CAPITAL LETTER U WITH DIAERESIS      Ü	CP437,CP850
/* 9B */	0x00A2,	// CENT SIGN                                  ¢	CP437
/* 9C */	0x00A3,	// POUND SIGN                                 £	CP437,CP850
/* 9D */	0x00A5,	// YEN SIGN                                   ¥	CP437
/* 9E */	0x00DF,	// LATIN SMALL LETTER SHARP S                 ß	AtariST
/* 9F */	0x0192,	// LATIN SMALL LETTER F WITH HOOK             ƒ	CP437,CP850
/* A0 */	0x00E1,	// LATIN SMALL LETTER A WITH ACUTE            á	CP437,CP850
/* A1 */	0x00ED,	// LATIN SMALL LETTER I WITH ACUTE            í	CP437,CP850
/* A2 */	0x00F3,	// LATIN SMALL LETTER O WITH ACUTE            ó	CP437,CP850
/* A3 */	0x00FA,	// LATIN SMALL LETTER U WITH ACUTE            ú	CP437,CP850
/* A4 */	0x00F1,	// LATIN SMALL LETTER N WITH TILDE            ñ	CP437,CP850
/* A5 */	0x00D1,	// LATIN CAPITAL LETTER N WITH TILDE          Ñ	CP437,CP850
/* A6 */	0x00AA,	// FEMININE ORDINAL INDICATOR                 ª	CP437,CP850
/* A7 */	0x00BA,	// MASCULINE ORDINAL INDICATOR                º	CP437,CP850
/* A8 */	0x00BF,	// INVERTED QUESTION MARK                     ¿	CP437,CP850
/* A9 */	0x2310,	// REVERSED NOT SIGN                          ⌐	CP437
/* AA */	0x00AC,	// NOT SIGN                                   ¬	CP437,CP850
/* AB */	0x00BD,	// VULGAR FRACTION ONE HALF                   ½	CP437,CP850
/* AC */	0x00BC,	// VULGAR FRACTION ONE QUARTER                ¼	CP437,CP850
/* AD */	0x00A1,	// INVERTED EXCLAMATION MARK                  ¡	CP437,CP850
/* AE */	0x00AB,	// LEFT-POINTING DOUBLE ANGLE QUOTATION MARK  «	CP437,CP850
/* AF */	0x00BB,	// RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK,	// »	CP437,CP850
/* B0 */	0x00E3,	// LATIN SMALL LETTER A WITH TILDE            ã	AtariST
/* B1 */	0x00F5,	// LATIN SMALL LETTER O WITH TILDE            õ	AtariST
/* B2 */	0x00D8,	// LATIN CAPITAL LETTER O WITH STROKE         Ø	AtariST
/* B3 */	0x00F8,	// LATIN SMALL LETTER O WITH STROKE           ø	AtariST
/* B4 */	0x0153,	// LATIN SMALL LIGATURE OE                    œ	AtariST
/* B5 */	0x0152,	// LATIN CAPITAL LIGATURE OE                  Œ	AtariST
/* B6 */	0x00C0,	// LATIN CAPITAL LETTER A WITH GRAVE          À	AtariST
/* B7 */	0x00C3,	// LATIN CAPITAL LETTER A WITH TILDE          Ã	AtariST
/* B8 */	0x00D5,	// LATIN CAPITAL LETTER O WITH TILDE          Õ	AtariST
/* B9 */	0x00A8,	// DIAERESIS                                  ¨	AtariST
/* BA */	0x00B4,	// ACUTE ACCENT                               ´	AtariST
/* BB */	0x2020,	// DAGGER                                     †	AtariST
/* BC */	0x00B6,	// PILCROW SIGN                               ¶	AtariST
/* BD */	0x00A9,	// COPYRIGHT SIGN                             ©	AtariST
/* BE */	0x00AE,	// REGISTERED SIGN                            ®	AtariST
/* BF */	0x2122,	// TRADE MARK SIGN                            ™	AtariST
/* C0 */	0x0133,	// LATIN SMALL LIGATURE IJ                    ĳ	AtariST
/* C1 */	0x0132,	// LATIN CAPITAL LIGATURE IJ                  Ĳ	AtariST
/* C2 */	0x05D0,	// HEBREW LETTER ALEF                         א	AtariST
/* C3 */	0x05D1,	// HEBREW LETTER BET                          ב	AtariST
/* C4 */	0x05D2,	// HEBREW LETTER GIMEL                        ג	AtariST
/* C5 */	0x05D3,	// HEBREW LETTER DALET                        ד	AtariST
/* C6 */	0x05D4,	// HEBREW LETTER HE                           ה	AtariST
/* C7 */	0x05D5,	// HEBREW LETTER VAV                          ו	AtariST
/* C8 */	0x05D6,	// HEBREW LETTER ZAYIN                        ז	AtariST
/* C9 */	0x05D7,	// HEBREW LETTER HET                          ח	AtariST
/* CA */	0x05D8,	// HEBREW LETTER TET                          ט	AtariST
/* CB */	0x05D9,	// HEBREW LETTER YOD                          י	AtariST
/* CC */	0x05DB,	// HEBREW LETTER KAF                          כ	AtariST
/* CD */	0x05DC,	// HEBREW LETTER LAMED                        ל	AtariST
/* CE */	0x05DE,	// HEBREW LETTER MEM                          מ	AtariST
/* CF */	0x05E0,	// HEBREW LETTER NUN                          נ	AtariST
/* D0 */	0x05E1,	// HEBREW LETTER SAMEKH                       ס	AtariST
/* D1 */	0x05E2,	// HEBREW LETTER AYIN                         ע	AtariST
/* D2 */	0x05E4,	// HEBREW LETTER PE                           פ	AtariST
/* D3 */	0x05E6,	// HEBREW LETTER TSADI                        צ	AtariST
/* D4 */	0x05E7,	// HEBREW LETTER QOF                          ק	AtariST
/* D5 */	0x05E8,	// HEBREW LETTER RESH                         ר	AtariST
/* D6 */	0x05E9,	// HEBREW LETTER SHIN                         ש	AtariST
/* D7 */	0x05EA,	// HEBREW LETTER TAV                          ת	AtariST
/* D8 */	0x05DF,	// HEBREW LETTER FINAL NUN                    ן	AtariST
/* D9 */	0x05DA,	// HEBREW LETTER FINAL KAF                    ך	AtariST
/* DA */	0x05DD,	// HEBREW LETTER FINAL MEM                    ם	AtariST
/* DB */	0x05E3,	// HEBREW LETTER FINAL PE                     ף	AtariST
/* DC */	0x05E5,	// HEBREW LETTER FINAL TSADI                  ץ	AtariST
/* DD */	0x00A7,	// SECTION SIGN                               §	AtariST
/* DE */	0x2227,	// LOGICAL AND                                ∧	AtariST
/* DF */	0x221E,	// INFINITY                                   ∞	AtariST
/* E0 */	0x03B1,	// GREEK SMALL LETTER ALPHA                   α	CP437,CP850
/* E1 */	0x03B2,	// GREEK SMALL LETTER BETA                    ß	AtariST
/* E2 */	0x0393,	// GREEK CAPITAL LETTER GAMMA                 Γ	CP437,CP850
/* E3 */	0x03C0,	// GREEK SMALL LETTER PI                      π	CP437,CP850
/* E4 */	0x03A3,	// GREEK CAPITAL LETTER SIGMA                 Σ	CP437,CP850
/* E5 */	0x03C3,	// GREEK SMALL LETTER SIGMA                   σ	CP437,CP850
/* E6 */	0x00B5,	// MICRO SIGN                                 µ	CP437,CP850
/* E7 */	0x03C4,	// GREEK SMALL LETTER TAU                     τ	CP437,CP850
/* E8 */	0x03A6,	// GREEK CAPITAL LETTER PHI                   Φ	CP437,CP850
/* E9 */	0x0398,	// GREEK CAPITAL LETTER THETA                 Θ	CP437,CP850
/* EA */	0x03A9,	// GREEK CAPITAL LETTER OMEGA                 Ω	CP437,CP850
/* EB */	0x03B4,	// GREEK SMALL LETTER DELTA                   δ	CP437,CP850
/* EC */	0x222E,	// CONTOUR INTEGRAL                           ∮	AtariST
/* ED */	0x03C6,	// GREEK SMALL LETTER PHI                     φ	CP437,CP850
/* EE */	0x2208,	// ELEMENT OF SIGN                            ∈	AtariST
/* EF */	0x2229,	// INTERSECTION                               ∩	CP437,CP850
/* F0 */	0x2261,	// IDENTICAL TO                               ≡	CP437,CP850
/* F1 */	0x00B1,	// PLUS-MINUS SIGN                            ±	CP437,CP850
/* F2 */	0x2265,	// GREATER-THAN OR EQUAL TO                   ≥	CP437,CP850
/* F3 */	0x2264,	// LESS-THAN OR EQUAL TO                      ≤	CP437,CP850
/* F4 */	0x2320,	// TOP HALF INTEGRAL                          ⌠	CP437,CP850
/* F5 */	0x2321,	// BOTTOM HALF INTEGRAL                       ⌡	CP437,CP850
/* F6 */	0x00F7,	// DIVISION SIGN                              ÷	CP437,CP850
/* F7 */	0x2248,	// ALMOST EQUAL TO                            ≈	CP437,CP850
/* F8 */	0x00B0,	// DEGREE SIGN                                °	CP437,CP850
/* F9 */	0x2219,	// BULLET OPERATOR                            ∙	CP437,CP850
/* FA */	0x00B7,	// MIDDLE DOT                                 ·	CP437,CP850
/* FB */	0x221A,	// SQUARE ROOT                                √	CP437,CP850
/* FC */	0x207F,	// SUPERSCRIPT LATIN SMALL LETTER N           ⁿ	CP437,CP850
/* FD */	0x00B2,	// SUPERSCRIPT TWO                            ²	CP437,CP850
/* FE */	0x00B3,	// SUPERSCRIPT THREE                          ³	AtariST
/* FF */	0x00AF	// MACRON                                     ¯	AtariST
};

/***************************************************************************
# File encoding:    UTF-8
# Name:             ZX-81 to Unicode
# Date:             2013 November 12
# Authors:          Jürgen Buchmüller <pullmoll AT t-online.de>
#
# Copyright (c) 2013 The MAME team
#
# General notes:
# This table contains the data the Unicode values for
# ZX-81 characters map into Unicode.
#
***************************************************************************/
static const unicode_char lookup_zx81[256] = {
/* 00 */	0x0020,	// SPACE
/* 01 */	0x2598,	// TOP, LEFT BLOCK
/* 02 */	0x259d,	// TOP, RIGHT BLOCK
/* 03 */	0x2580,	// TOP TWO BLOCKS
/* 04 */	0x2596,	// BOTTOM LEFT BLOCK
/* 05 */	0x258c,	// LEFT TWO BLOCKS
/* 06 */	0x259e,	// BOTTOM LEFT and TOP RIGHT BLOCK
/* 07 */	0x259b,	// ALL BLOCKS, EXCEPT BOTTOM RIGHT
/* 08 */	0xfffd,	// CHECKERED
/* 09 */	0xfffd,	// CHECKERED BOTTOM HALF
/* 0a */	0xfffd,	// CHECKERED TOP HALF
/* 0b */	0x0022,	// QUOTATION MARK
/* 0c */	0x00a3,	// POUND SIGN
/* 0d */	0x0024,	// DOLLAR SIGN
/* 0e */	0x002a,	// COLON
/* 0f */	0x003f,	// QUESTION MARK
/* 10 */	0x0028,	// LEFT PARENTHESIS
/* 11 */	0x0029,	// RIGHT PARENTHESIS
/* 12 */	0x003e,	// GREATER-THAN SIGN
/* 13 */	0x003c,	// LESS-THAN SIGN
/* 14 */	0x003d,	// EQUALS SIGN
/* 15 */	0x002b,	// PLUS SIGN
/* 16 */	0x002d,	// MINUS SIGN
/* 17 */	0x002a,	// ASTERISK
/* 18 */	0x002f,	// SLASH
/* 19 */	0x003b,	// SEMICOLON
/* 1a */	0x002c,	// COMMA
/* 1b */	0x002e,	// PERIOD
/* 1c */	0x0030,	// DIGIT ZERO
/* 1d */	0x0031,	// DIGIT ONE
/* 1e */	0x0032,	// DIGIT TWO
/* 1f */	0x0033,	// DIGIT THREE
/* 20 */	0x0034,	// DIGIT FOUR
/* 21 */	0x0035,	// DIGIT FIVE
/* 22 */	0x0036,	// DIGIT SIX
/* 23 */	0x0037,	// DIGIT SEVEN
/* 24 */	0x0038,	// DIGIT EIGHT
/* 25 */	0x0039,	// DIGIT NINE
/* 26 */	0x0041,	// LATIN CAPITAL LETTER A
/* 27 */	0x0042,	// LATIN CAPITAL LETTER B
/* 28 */	0x0043,	// LATIN CAPITAL LETTER C
/* 29 */	0x0044,	// LATIN CAPITAL LETTER D
/* 2a */	0x0045,	// LATIN CAPITAL LETTER E
/* 2b */	0x0046,	// LATIN CAPITAL LETTER F
/* 2c */	0x0047,	// LATIN CAPITAL LETTER G
/* 2d */	0x0048,	// LATIN CAPITAL LETTER H
/* 2e */	0x0049,	// LATIN CAPITAL LETTER I
/* 2f */	0x004a,	// LATIN CAPITAL LETTER J
/* 30 */	0x004b,	// LATIN CAPITAL LETTER K
/* 31 */	0x004c,	// LATIN CAPITAL LETTER L
/* 32 */	0x004d,	// LATIN CAPITAL LETTER M
/* 33 */	0x004e,	// LATIN CAPITAL LETTER N
/* 34 */	0x004f,	// LATIN CAPITAL LETTER O
/* 35 */	0x0050,	// LATIN CAPITAL LETTER P
/* 36 */	0x0051,	// LATIN CAPITAL LETTER Q
/* 37 */	0x0052,	// LATIN CAPITAL LETTER R
/* 38 */	0x0053,	// LATIN CAPITAL LETTER S
/* 39 */	0x0054,	// LATIN CAPITAL LETTER T
/* 3a */	0x0055,	// LATIN CAPITAL LETTER U
/* 3b */	0x0056,	// LATIN CAPITAL LETTER V
/* 3c */	0x0057,	// LATIN CAPITAL LETTER W
/* 3d */	0x0058,	// LATIN CAPITAL LETTER X
/* 3e */	0x0059,	// LATIN CAPITAL LETTER Y
/* 3f */	0x005a,	// LATIN CAPITAL LETTER Z
/* 40 */	0xfffd,	// INVALID
/* 41 */	0xfffd,	// INVALID
/* 42 */	0xfffd,	// INVALID
/* 43 */	0xfffd,	// INVALID
/* 44 */	0xfffd,	// INVALID
/* 45 */	0xfffd,	// INVALID
/* 46 */	0xfffd,	// INVALID
/* 47 */	0xfffd,	// INVALID
/* 48 */	0xfffd,	// INVALID
/* 49 */	0xfffd,	// INVALID
/* 4a */	0xfffd,	// INVALID
/* 4b */	0xfffd,	// INVALID
/* 4c */	0xfffd,	// INVALID
/* 4d */	0xfffd,	// INVALID
/* 4e */	0xfffd,	// INVALID
/* 4f */	0xfffd,	// INVALID
/* 50 */	0xfffd,	// INVALID
/* 51 */	0xfffd,	// INVALID
/* 52 */	0xfffd,	// INVALID
/* 53 */	0xfffd,	// INVALID
/* 54 */	0xfffd,	// INVALID
/* 55 */	0xfffd,	// INVALID
/* 56 */	0xfffd,	// INVALID
/* 57 */	0xfffd,	// INVALID
/* 58 */	0xfffd,	// INVALID
/* 59 */	0xfffd,	// INVALID
/* 5a */	0xfffd,	// INVALID
/* 5b */	0xfffd,	// INVALID
/* 5c */	0xfffd,	// INVALID
/* 5d */	0xfffd,	// INVALID
/* 5e */	0xfffd,	// INVALID
/* 5f */	0xfffd,	// INVALID
/* 60 */	0xfffd,	// INVALID
/* 61 */	0xfffd,	// INVALID
/* 62 */	0xfffd,	// INVALID
/* 63 */	0xfffd,	// INVALID
/* 64 */	0xfffd,	// INVALID
/* 65 */	0xfffd,	// INVALID
/* 66 */	0xfffd,	// INVALID
/* 67 */	0xfffd,	// INVALID
/* 68 */	0xfffd,	// INVALID
/* 69 */	0xfffd,	// INVALID
/* 6a */	0xfffd,	// INVALID
/* 6b */	0xfffd,	// INVALID
/* 6c */	0xfffd,	// INVALID
/* 6d */	0xfffd,	// INVALID
/* 6e */	0xfffd,	// INVALID
/* 6f */	0xfffd,	// INVALID
/* 70 */	0xfffd,	// INVALID
/* 71 */	0xfffd,	// INVALID
/* 72 */	0xfffd,	// INVALID
/* 73 */	0xfffd,	// INVALID
/* 74 */	0xfffd,	// INVALID
/* 75 */	0xfffd,	// INVALID
/* 76 */	0xfffd,	// INVALID
/* 77 */	0xfffd,	// INVALID
/* 78 */	0xfffd,	// INVALID
/* 79 */	0xfffd,	// INVALID
/* 7a */	0xfffd,	// INVALID
/* 7b */	0xfffd,	// INVALID
/* 7c */	0xfffd,	// INVALID
/* 7d */	0xfffd,	// INVALID
/* 7e */	0xfffd,	// INVALID
/* 7f */	0xfffd,	// INVALID
/* 80 */	0x2588,	// [INVERSE] SPACE
/* 81 */	0x259f,	// ALL, EXCEPT TOP LEFT BLOCK
/* 82 */	0x2599,	// ALL, EXCEPT TOP RIGHT BLOCK
/* 83 */	0x2584,	// BOTTOM TWO BLOCKS
/* 84 */	0x259c,	// ALL, EXCEPT BOTTOM LEFT BLOCK
/* 85 */	0x2590,	// RIGHT TWO BLOCKS
/* 86 */	0x259a,	// TOP LEFT and BOTTOM RIGHT BLOCK
/* 87 */	0x2597,	// BOTTOM RIGHT BLOCK
/* 88 */	0xfffd,	// [INVERSE] CHECKERED
/* 89 */	0xfffd,	// CHECKERED TOP HALF, BLOCK BOTTOM
/* 8a */	0xfffd,	// CHECKERED BOTTOM HALF, BLOCK TOP
/* 8b */	0x0022,	// [INVERSE] QUOTATION MARK
/* 8c */	0x00a3,	// [INVERSE] POUND SIGN
/* 8d */	0x0024,	// [INVERSE] DOLLAR SIGN
/* 8e */	0x002a,	// [INVERSE] COLON
/* 8f */	0x003f,	// [INVERSE] QUESTION MARK
/* 90 */	0x0028,	// [INVERSE] LEFT PARENTHESIS
/* 91 */	0x0029,	// [INVERSE] RIGHT PARENTHESIS
/* 92 */	0x003e,	// [INVERSE] GREATER THAN
/* 93 */	0x003c,	// [INVERSE] LESS THAN
/* 94 */	0x003d,	// [INVERSE] EQUALS SIGN
/* 95 */	0x002b,	// [INVERSE] PLUS SIGN
/* 96 */	0x002d,	// [INVERSE] MINUS SIGN
/* 97 */	0x002a,	// [INVERSE] ASTERISK
/* 98 */	0x002f,	// [INVERSE] SLASH
/* 99 */	0x003b,	// [INVERSE] SEMICOLON
/* 9a */	0x002c,	// [INVERSE] COMMA
/* 9b */	0x002e,	// [INVERSE] PERIOD
/* 9c */	0x0030,	// [INVERSE] DIGIT ZERO
/* 9d */	0x0031,	// [INVERSE] DIGIT ONE
/* 9e */	0x0032,	// [INVERSE] DIGIT TWO
/* 9f */	0x0033,	// [INVERSE] DIGIT THREE
/* a0 */	0x0034,	// [INVERSE] DIGIT FOUR
/* a1 */	0x0035,	// [INVERSE] DIGIT FIVE
/* a2 */	0x0036,	// [INVERSE] DIGIT SIX
/* a3 */	0x0037,	// [INVERSE] DIGIT SEVEN
/* a4 */	0x0038,	// [INVERSE] DIGIT EIGHT
/* a5 */	0x0039,	// [INVERSE] DIGIT NINE
/* a6 */	0x0041,	// [INVERSE] LATIN CAPITAL LETTER A
/* a7 */	0x0042,	// [INVERSE] LATIN CAPITAL LETTER B
/* a8 */	0x0043,	// [INVERSE] LATIN CAPITAL LETTER C
/* a9 */	0x0044,	// [INVERSE] LATIN CAPITAL LETTER D
/* aa */	0x0045,	// [INVERSE] LATIN CAPITAL LETTER E
/* ab */	0x0046,	// [INVERSE] LATIN CAPITAL LETTER F
/* ac */	0x0047,	// [INVERSE] LATIN CAPITAL LETTER G
/* ad */	0x0048,	// [INVERSE] LATIN CAPITAL LETTER H
/* ae */	0x0049,	// [INVERSE] LATIN CAPITAL LETTER I
/* af */	0x004a,	// [INVERSE] LATIN CAPITAL LETTER J
/* b0 */	0x004b,	// [INVERSE] LATIN CAPITAL LETTER K
/* b1 */	0x004c,	// [INVERSE] LATIN CAPITAL LETTER L
/* b2 */	0x004d,	// [INVERSE] LATIN CAPITAL LETTER M
/* b3 */	0x004e,	// [INVERSE] LATIN CAPITAL LETTER N
/* b4 */	0x004f,	// [INVERSE] LATIN CAPITAL LETTER O
/* b5 */	0x0050,	// [INVERSE] LATIN CAPITAL LETTER P
/* b6 */	0x0051,	// [INVERSE] LATIN CAPITAL LETTER Q
/* b7 */	0x0052,	// [INVERSE] LATIN CAPITAL LETTER R
/* b8 */	0x0053,	// [INVERSE] LATIN CAPITAL LETTER S
/* b9 */	0x0054,	// [INVERSE] LATIN CAPITAL LETTER T
/* ba */	0x0055,	// [INVERSE] LATIN CAPITAL LETTER U
/* bb */	0x0056,	// [INVERSE] LATIN CAPITAL LETTER V
/* bc */	0x0057,	// [INVERSE] LATIN CAPITAL LETTER W
/* bd */	0x0058,	// [INVERSE] LATIN CAPITAL LETTER X
/* be */	0x0059,	// [INVERSE] LATIN CAPITAL LETTER Y
/* bf */	0x005a,	// [INVERSE] LATIN CAPITAL LETTER Z
/* c0 */	0xfffd,	// INVALID
/* c1 */	0xfffd,	// INVALID
/* c2 */	0xfffd,	// INVALID
/* c3 */	0xfffd,	// INVALID
/* c4 */	0xfffd,	// INVALID
/* c5 */	0xfffd,	// INVALID
/* c6 */	0xfffd,	// INVALID
/* c7 */	0xfffd,	// INVALID
/* c8 */	0xfffd,	// INVALID
/* c9 */	0xfffd,	// INVALID
/* ca */	0xfffd,	// INVALID
/* cb */	0xfffd,	// INVALID
/* cc */	0xfffd,	// INVALID
/* cd */	0xfffd,	// INVALID
/* ce */	0xfffd,	// INVALID
/* cf */	0xfffd,	// INVALID
/* d0 */	0xfffd,	// INVALID
/* d1 */	0xfffd,	// INVALID
/* d2 */	0xfffd,	// INVALID
/* d3 */	0xfffd,	// INVALID
/* d4 */	0xfffd,	// INVALID
/* d5 */	0xfffd,	// INVALID
/* d6 */	0xfffd,	// INVALID
/* d7 */	0xfffd,	// INVALID
/* d8 */	0xfffd,	// INVALID
/* d9 */	0xfffd,	// INVALID
/* da */	0xfffd,	// INVALID
/* db */	0xfffd,	// INVALID
/* dc */	0xfffd,	// INVALID
/* dd */	0xfffd,	// INVALID
/* de */	0xfffd,	// INVALID
/* df */	0xfffd,	// INVALID
/* e0 */	0xfffd,	// INVALID
/* e1 */	0xfffd,	// INVALID
/* e2 */	0xfffd,	// INVALID
/* e3 */	0xfffd,	// INVALID
/* e4 */	0xfffd,	// INVALID
/* e5 */	0xfffd,	// INVALID
/* e6 */	0xfffd,	// INVALID
/* e7 */	0xfffd,	// INVALID
/* e8 */	0xfffd,	// INVALID
/* e9 */	0xfffd,	// INVALID
/* ea */	0xfffd,	// INVALID
/* eb */	0xfffd,	// INVALID
/* ec */	0xfffd,	// INVALID
/* ed */	0xfffd,	// INVALID
/* ee */	0xfffd,	// INVALID
/* ef */	0xfffd,	// INVALID
/* f0 */	0xfffd,	// INVALID
/* f1 */	0xfffd,	// INVALID
/* f2 */	0xfffd,	// INVALID
/* f3 */	0xfffd,	// INVALID
/* f4 */	0xfffd,	// INVALID
/* f5 */	0xfffd,	// INVALID
/* f6 */	0xfffd,	// INVALID
/* f7 */	0xfffd,	// INVALID
/* f8 */	0xfffd,	// INVALID
/* f9 */	0xfffd,	// INVALID
/* fa */	0xfffd,	// INVALID
/* fb */	0xfffd,	// INVALID
/* fc */	0xfffd,	// INVALID
/* fd */	0xfffd,	// INVALID
/* fe */	0xfffd,	// INVALID
/* ff */	0xfffd	// INVALID
};

/***************************************************************************
#
#	Name:             KOI8-R (RFC1489) to Unicode
#	Unicode version:  3.0
#	Table version:    1.0
#	Table format:     Format A
#	Date:             18 August 1999
#	Authors:          Helmut Richter <richter@lrz.de>
#
#	Copyright (c) 1991-1999 Unicode, Inc.  All Rights reserved.
#
#	This file is provided as-is by Unicode, Inc. (The Unicode Consortium).
#	No claims are made as to fitness for any particular purpose.  No
#	warranties of any kind are expressed or implied.  The recipient
#	agrees to determine applicability of information provided.  If this
#	file has been provided on optical media by Unicode, Inc., the sole
#	remedy for any claim will be exchange of defective media within 90
#	days of receipt.
#
#	Unicode, Inc. hereby grants the right to freely use the information
#	supplied in this file in the creation of products supporting the
#	Unicode Standard, and to make copies of this file in any form for
#	internal or external distribution as long as this notice remains
#	attached.
#
#	General notes:
#
#	This table contains the data the Unicode Consortium has on how
#       KOI8-R characters map into Unicode. The underlying document is the
#	mapping described in RFC 1489. No statements are made as to whether
#	this mapping is the same as the mapping defined as "Code Page 878"
#	with some vendors.
#
#	Format:  Three tab-separated columns
#		 Column #1 is the KOI8-R code (in hex as 0xXX)
#		 Column #2 is the Unicode (in hex as 0xXXXX)
#		 Column #3 the Unicode name (follows a comment sign, '#')
#
#	The entries are in KOI8-R order.
#
#	Version history
#	1.0 version: created.
#
#	Any comments or problems, contact <errata@unicode.org>
#	Please note that <errata@unicode.org> is an archival address;
#	notices will be checked, but do not expect an immediate response.
#
***************************************************************************/
static const unicode_char lookup_koi8_r[256] = {
/* 00 */	0x0000,	//	NULL
/* 01 */	0x0001,	//	START OF HEADING
/* 02 */	0x0002,	//	START OF TEXT
/* 03 */	0x0003,	//	END OF TEXT
/* 04 */	0x0004,	//	END OF TRANSMISSION
/* 05 */	0x0005,	//	ENQUIRY
/* 06 */	0x0006,	//	ACKNOWLEDGE
/* 07 */	0x0007,	//	BELL
/* 08 */	0x0008,	//	BACKSPACE
/* 09 */	0x0009,	//	HORIZONTAL TABULATION
/* 0A */	0x000A,	//	LINE FEED
/* 0B */	0x000B,	//	VERTICAL TABULATION
/* 0C */	0x000C,	//	FORM FEED
/* 0D */	0x000D,	//	CARRIAGE RETURN
/* 0E */	0x000E,	//	SHIFT OUT
/* 0F */	0x000F,	//	SHIFT IN
/* 10 */	0x0010,	//	DATA LINK ESCAPE
/* 11 */	0x0011,	//	DEVICE CONTROL ONE
/* 12 */	0x0012,	//	DEVICE CONTROL TWO
/* 13 */	0x0013,	//	DEVICE CONTROL THREE
/* 14 */	0x0014,	//	DEVICE CONTROL FOUR
/* 15 */	0x0015,	//	NEGATIVE ACKNOWLEDGE
/* 16 */	0x0016,	//	SYNCHRONOUS IDLE
/* 17 */	0x0017,	//	END OF TRANSMISSION BLOCK
/* 18 */	0x0018,	//	CANCEL
/* 19 */	0x0019,	//	END OF MEDIUM
/* 1A */	0x001A,	//	SUBSTITUTE
/* 1B */	0x001B,	//	ESCAPE
/* 1C */	0x001C,	//	FILE SEPARATOR
/* 1D */	0x001D,	//	GROUP SEPARATOR
/* 1E */	0x001E,	//	RECORD SEPARATOR
/* 1F */	0x001F,	//	UNIT SEPARATOR
/* 20 */	0x0020,	//	SPACE
/* 21 */	0x0021,	//	EXCLAMATION MARK
/* 22 */	0x0022,	//	QUOTATION MARK
/* 23 */	0x0023,	//	NUMBER SIGN
/* 24 */	0x0024,	//	DOLLAR SIGN
/* 25 */	0x0025,	//	PERCENT SIGN
/* 26 */	0x0026,	//	AMPERSAND
/* 27 */	0x0027,	//	APOSTROPHE
/* 28 */	0x0028,	//	LEFT PARENTHESIS
/* 29 */	0x0029,	//	RIGHT PARENTHESIS
/* 2A */	0x002A,	//	ASTERISK
/* 2B */	0x002B,	//	PLUS SIGN
/* 2C */	0x002C,	//	COMMA
/* 2D */	0x002D,	//	HYPHEN-MINUS
/* 2E */	0x002E,	//	FULL STOP
/* 2F */	0x002F,	//	SOLIDUS
/* 30 */	0x0030,	//	DIGIT ZERO
/* 31 */	0x0031,	//	DIGIT ONE
/* 32 */	0x0032,	//	DIGIT TWO
/* 33 */	0x0033,	//	DIGIT THREE
/* 34 */	0x0034,	//	DIGIT FOUR
/* 35 */	0x0035,	//	DIGIT FIVE
/* 36 */	0x0036,	//	DIGIT SIX
/* 37 */	0x0037,	//	DIGIT SEVEN
/* 38 */	0x0038,	//	DIGIT EIGHT
/* 39 */	0x0039,	//	DIGIT NINE
/* 3A */	0x003A,	//	COLON
/* 3B */	0x003B,	//	SEMICOLON
/* 3C */	0x003C,	//	LESS-THAN SIGN
/* 3D */	0x003D,	//	EQUALS SIGN
/* 3E */	0x003E,	//	GREATER-THAN SIGN
/* 3F */	0x003F,	//	QUESTION MARK
/* 40 */	0x0040,	//	COMMERCIAL AT
/* 41 */	0x0041,	//	LATIN CAPITAL LETTER A
/* 42 */	0x0042,	//	LATIN CAPITAL LETTER B
/* 43 */	0x0043,	//	LATIN CAPITAL LETTER C
/* 44 */	0x0044,	//	LATIN CAPITAL LETTER D
/* 45 */	0x0045,	//	LATIN CAPITAL LETTER E
/* 46 */	0x0046,	//	LATIN CAPITAL LETTER F
/* 47 */	0x0047,	//	LATIN CAPITAL LETTER G
/* 48 */	0x0048,	//	LATIN CAPITAL LETTER H
/* 49 */	0x0049,	//	LATIN CAPITAL LETTER I
/* 4A */	0x004A,	//	LATIN CAPITAL LETTER J
/* 4B */	0x004B,	//	LATIN CAPITAL LETTER K
/* 4C */	0x004C,	//	LATIN CAPITAL LETTER L
/* 4D */	0x004D,	//	LATIN CAPITAL LETTER M
/* 4E */	0x004E,	//	LATIN CAPITAL LETTER N
/* 4F */	0x004F,	//	LATIN CAPITAL LETTER O
/* 50 */	0x0050,	//	LATIN CAPITAL LETTER P
/* 51 */	0x0051,	//	LATIN CAPITAL LETTER Q
/* 52 */	0x0052,	//	LATIN CAPITAL LETTER R
/* 53 */	0x0053,	//	LATIN CAPITAL LETTER S
/* 54 */	0x0054,	//	LATIN CAPITAL LETTER T
/* 55 */	0x0055,	//	LATIN CAPITAL LETTER U
/* 56 */	0x0056,	//	LATIN CAPITAL LETTER V
/* 57 */	0x0057,	//	LATIN CAPITAL LETTER W
/* 58 */	0x0058,	//	LATIN CAPITAL LETTER X
/* 59 */	0x0059,	//	LATIN CAPITAL LETTER Y
/* 5A */	0x005A,	//	LATIN CAPITAL LETTER Z
/* 5B */	0x005B,	//	LEFT SQUARE BRACKET
/* 5C */	0x005C,	//	REVERSE SOLIDUS
/* 5D */	0x005D,	//	RIGHT SQUARE BRACKET
/* 5E */	0x005E,	//	CIRCUMFLEX ACCENT
/* 5F */	0x005F,	//	LOW LINE
/* 60 */	0x0060,	//	GRAVE ACCENT
/* 61 */	0x0061,	//	LATIN SMALL LETTER A
/* 62 */	0x0062,	//	LATIN SMALL LETTER B
/* 63 */	0x0063,	//	LATIN SMALL LETTER C
/* 64 */	0x0064,	//	LATIN SMALL LETTER D
/* 65 */	0x0065,	//	LATIN SMALL LETTER E
/* 66 */	0x0066,	//	LATIN SMALL LETTER F
/* 67 */	0x0067,	//	LATIN SMALL LETTER G
/* 68 */	0x0068,	//	LATIN SMALL LETTER H
/* 69 */	0x0069,	//	LATIN SMALL LETTER I
/* 6A */	0x006A,	//	LATIN SMALL LETTER J
/* 6B */	0x006B,	//	LATIN SMALL LETTER K
/* 6C */	0x006C,	//	LATIN SMALL LETTER L
/* 6D */	0x006D,	//	LATIN SMALL LETTER M
/* 6E */	0x006E,	//	LATIN SMALL LETTER N
/* 6F */	0x006F,	//	LATIN SMALL LETTER O
/* 70 */	0x0070,	//	LATIN SMALL LETTER P
/* 71 */	0x0071,	//	LATIN SMALL LETTER Q
/* 72 */	0x0072,	//	LATIN SMALL LETTER R
/* 73 */	0x0073,	//	LATIN SMALL LETTER S
/* 74 */	0x0074,	//	LATIN SMALL LETTER T
/* 75 */	0x0075,	//	LATIN SMALL LETTER U
/* 76 */	0x0076,	//	LATIN SMALL LETTER V
/* 77 */	0x0077,	//	LATIN SMALL LETTER W
/* 78 */	0x0078,	//	LATIN SMALL LETTER X
/* 79 */	0x0079,	//	LATIN SMALL LETTER Y
/* 7A */	0x007A,	//	LATIN SMALL LETTER Z
/* 7B */	0x007B,	//	LEFT CURLY BRACKET
/* 7C */	0x007C,	//	VERTICAL LINE
/* 7D */	0x007D,	//	RIGHT CURLY BRACKET
/* 7E */	0x007E,	//	TILDE
/* 7F */	0x007F,	//	DELETE
/* 80 */	0x2500,	//	BOX DRAWINGS LIGHT HORIZONTAL
/* 81 */	0x2502,	//	BOX DRAWINGS LIGHT VERTICAL
/* 82 */	0x250C,	//	BOX DRAWINGS LIGHT DOWN AND RIGHT
/* 83 */	0x2510,	//	BOX DRAWINGS LIGHT DOWN AND LEFT
/* 84 */	0x2514,	//	BOX DRAWINGS LIGHT UP AND RIGHT
/* 85 */	0x2518,	//	BOX DRAWINGS LIGHT UP AND LEFT
/* 86 */	0x251C,	//	BOX DRAWINGS LIGHT VERTICAL AND RIGHT
/* 87 */	0x2524,	//	BOX DRAWINGS LIGHT VERTICAL AND LEFT
/* 88 */	0x252C,	//	BOX DRAWINGS LIGHT DOWN AND HORIZONTAL
/* 89 */	0x2534,	//	BOX DRAWINGS LIGHT UP AND HORIZONTAL
/* 8A */	0x253C,	//	BOX DRAWINGS LIGHT VERTICAL AND HORIZONTAL
/* 8B */	0x2580,	//	UPPER HALF BLOCK
/* 8C */	0x2584,	//	LOWER HALF BLOCK
/* 8D */	0x2588,	//	FULL BLOCK
/* 8E */	0x258C,	//	LEFT HALF BLOCK
/* 8F */	0x2590,	//	RIGHT HALF BLOCK
/* 90 */	0x2591,	//	LIGHT SHADE
/* 91 */	0x2592,	//	MEDIUM SHADE
/* 92 */	0x2593,	//	DARK SHADE
/* 93 */	0x2320,	//	TOP HALF INTEGRAL
/* 94 */	0x25A0,	//	BLACK SQUARE
/* 95 */	0x2219,	//	BULLET OPERATOR
/* 96 */	0x221A,	//	SQUARE ROOT
/* 97 */	0x2248,	//	ALMOST EQUAL TO
/* 98 */	0x2264,	//	LESS-THAN OR EQUAL TO
/* 99 */	0x2265,	//	GREATER-THAN OR EQUAL TO
/* 9A */	0x00A0,	//	NO-BREAK SPACE
/* 9B */	0x2321,	//	BOTTOM HALF INTEGRAL
/* 9C */	0x00B0,	//	DEGREE SIGN
/* 9D */	0x00B2,	//	SUPERSCRIPT TWO
/* 9E */	0x00B7,	//	MIDDLE DOT
/* 9F */	0x00F7,	//	DIVISION SIGN
/* A0 */	0x2550,	//	BOX DRAWINGS DOUBLE HORIZONTAL
/* A1 */	0x2551,	//	BOX DRAWINGS DOUBLE VERTICAL
/* A2 */	0x2552,	//	BOX DRAWINGS DOWN SINGLE AND RIGHT DOUBLE
/* A3 */	0x0451,	//	CYRILLIC SMALL LETTER IO
/* A4 */	0x2553,	//	BOX DRAWINGS DOWN DOUBLE AND RIGHT SINGLE
/* A5 */	0x2554,	//	BOX DRAWINGS DOUBLE DOWN AND RIGHT
/* A6 */	0x2555,	//	BOX DRAWINGS DOWN SINGLE AND LEFT DOUBLE
/* A7 */	0x2556,	//	BOX DRAWINGS DOWN DOUBLE AND LEFT SINGLE
/* A8 */	0x2557,	//	BOX DRAWINGS DOUBLE DOWN AND LEFT
/* A9 */	0x2558,	//	BOX DRAWINGS UP SINGLE AND RIGHT DOUBLE
/* AA */	0x2559,	//	BOX DRAWINGS UP DOUBLE AND RIGHT SINGLE
/* AB */	0x255A,	//	BOX DRAWINGS DOUBLE UP AND RIGHT
/* AC */	0x255B,	//	BOX DRAWINGS UP SINGLE AND LEFT DOUBLE
/* AD */	0x255C,	//	BOX DRAWINGS UP DOUBLE AND LEFT SINGLE
/* AE */	0x255D,	//	BOX DRAWINGS DOUBLE UP AND LEFT
/* AF */	0x255E,	//	BOX DRAWINGS VERTICAL SINGLE AND RIGHT DOUBLE
/* B0 */	0x255F,	//	BOX DRAWINGS VERTICAL DOUBLE AND RIGHT SINGLE
/* B1 */	0x2560,	//	BOX DRAWINGS DOUBLE VERTICAL AND RIGHT
/* B2 */	0x2561,	//	BOX DRAWINGS VERTICAL SINGLE AND LEFT DOUBLE
/* B3 */	0x0401,	//	CYRILLIC CAPITAL LETTER IO
/* B4 */	0x2562,	//	BOX DRAWINGS VERTICAL DOUBLE AND LEFT SINGLE
/* B5 */	0x2563,	//	BOX DRAWINGS DOUBLE VERTICAL AND LEFT
/* B6 */	0x2564,	//	BOX DRAWINGS DOWN SINGLE AND HORIZONTAL DOUBLE
/* B7 */	0x2565,	//	BOX DRAWINGS DOWN DOUBLE AND HORIZONTAL SINGLE
/* B8 */	0x2566,	//	BOX DRAWINGS DOUBLE DOWN AND HORIZONTAL
/* B9 */	0x2567,	//	BOX DRAWINGS UP SINGLE AND HORIZONTAL DOUBLE
/* BA */	0x2568,	//	BOX DRAWINGS UP DOUBLE AND HORIZONTAL SINGLE
/* BB */	0x2569,	//	BOX DRAWINGS DOUBLE UP AND HORIZONTAL
/* BC */	0x256A,	//	BOX DRAWINGS VERTICAL SINGLE AND HORIZONTAL DOUBLE
/* BD */	0x256B,	//	BOX DRAWINGS VERTICAL DOUBLE AND HORIZONTAL SINGLE
/* BE */	0x256C,	//	BOX DRAWINGS DOUBLE VERTICAL AND HORIZONTAL
/* BF */	0x00A9,	//	COPYRIGHT SIGN
/* C0 */	0x044E,	//	CYRILLIC SMALL LETTER YU
/* C1 */	0x0430,	//	CYRILLIC SMALL LETTER A
/* C2 */	0x0431,	//	CYRILLIC SMALL LETTER BE
/* C3 */	0x0446,	//	CYRILLIC SMALL LETTER TSE
/* C4 */	0x0434,	//	CYRILLIC SMALL LETTER DE
/* C5 */	0x0435,	//	CYRILLIC SMALL LETTER IE
/* C6 */	0x0444,	//	CYRILLIC SMALL LETTER EF
/* C7 */	0x0433,	//	CYRILLIC SMALL LETTER GHE
/* C8 */	0x0445,	//	CYRILLIC SMALL LETTER HA
/* C9 */	0x0438,	//	CYRILLIC SMALL LETTER I
/* CA */	0x0439,	//	CYRILLIC SMALL LETTER SHORT I
/* CB */	0x043A,	//	CYRILLIC SMALL LETTER KA
/* CC */	0x043B,	//	CYRILLIC SMALL LETTER EL
/* CD */	0x043C,	//	CYRILLIC SMALL LETTER EM
/* CE */	0x043D,	//	CYRILLIC SMALL LETTER EN
/* CF */	0x043E,	//	CYRILLIC SMALL LETTER O
/* D0 */	0x043F,	//	CYRILLIC SMALL LETTER PE
/* D1 */	0x044F,	//	CYRILLIC SMALL LETTER YA
/* D2 */	0x0440,	//	CYRILLIC SMALL LETTER ER
/* D3 */	0x0441,	//	CYRILLIC SMALL LETTER ES
/* D4 */	0x0442,	//	CYRILLIC SMALL LETTER TE
/* D5 */	0x0443,	//	CYRILLIC SMALL LETTER U
/* D6 */	0x0436,	//	CYRILLIC SMALL LETTER ZHE
/* D7 */	0x0432,	//	CYRILLIC SMALL LETTER VE
/* D8 */	0x044C,	//	CYRILLIC SMALL LETTER SOFT SIGN
/* D9 */	0x044B,	//	CYRILLIC SMALL LETTER YERU
/* DA */	0x0437,	//	CYRILLIC SMALL LETTER ZE
/* DB */	0x0448,	//	CYRILLIC SMALL LETTER SHA
/* DC */	0x044D,	//	CYRILLIC SMALL LETTER E
/* DD */	0x0449,	//	CYRILLIC SMALL LETTER SHCHA
/* DE */	0x0447,	//	CYRILLIC SMALL LETTER CHE
/* DF */	0x044A,	//	CYRILLIC SMALL LETTER HARD SIGN
/* E0 */	0x042E,	//	CYRILLIC CAPITAL LETTER YU
/* E1 */	0x0410,	//	CYRILLIC CAPITAL LETTER A
/* E2 */	0x0411,	//	CYRILLIC CAPITAL LETTER BE
/* E3 */	0x0426,	//	CYRILLIC CAPITAL LETTER TSE
/* E4 */	0x0414,	//	CYRILLIC CAPITAL LETTER DE
/* E5 */	0x0415,	//	CYRILLIC CAPITAL LETTER IE
/* E6 */	0x0424,	//	CYRILLIC CAPITAL LETTER EF
/* E7 */	0x0413,	//	CYRILLIC CAPITAL LETTER GHE
/* E8 */	0x0425,	//	CYRILLIC CAPITAL LETTER HA
/* E9 */	0x0418,	//	CYRILLIC CAPITAL LETTER I
/* EA */	0x0419,	//	CYRILLIC CAPITAL LETTER SHORT I
/* EB */	0x041A,	//	CYRILLIC CAPITAL LETTER KA
/* EC */	0x041B,	//	CYRILLIC CAPITAL LETTER EL
/* ED */	0x041C,	//	CYRILLIC CAPITAL LETTER EM
/* EE */	0x041D,	//	CYRILLIC CAPITAL LETTER EN
/* EF */	0x041E,	//	CYRILLIC CAPITAL LETTER O
/* F0 */	0x041F,	//	CYRILLIC CAPITAL LETTER PE
/* F1 */	0x042F,	//	CYRILLIC CAPITAL LETTER YA
/* F2 */	0x0420,	//	CYRILLIC CAPITAL LETTER ER
/* F3 */	0x0421,	//	CYRILLIC CAPITAL LETTER ES
/* F4 */	0x0422,	//	CYRILLIC CAPITAL LETTER TE
/* F5 */	0x0423,	//	CYRILLIC CAPITAL LETTER U
/* F6 */	0x0416,	//	CYRILLIC CAPITAL LETTER ZHE
/* F7 */	0x0412,	//	CYRILLIC CAPITAL LETTER VE
/* F8 */	0x042C,	//	CYRILLIC CAPITAL LETTER SOFT SIGN
/* F9 */	0x042B,	//	CYRILLIC CAPITAL LETTER YERU
/* FA */	0x0417,	//	CYRILLIC CAPITAL LETTER ZE
/* FB */	0x0428,	//	CYRILLIC CAPITAL LETTER SHA
/* FC */	0x042D,	//	CYRILLIC CAPITAL LETTER E
/* FD */	0x0429,	//	CYRILLIC CAPITAL LETTER SHCHA
/* FE */	0x0427,	//	CYRILLIC CAPITAL LETTER CHE
/* FF */	0x042A	//	CYRILLIC CAPITAL LETTER HARD SIGN
};

/***************************************************************************
#
#	Name:             KOI8-U (RFC2319) to Unicode
#	Unicode version:  5.1
#	Table version:    1.0
#	Table format:     Format A
#	Date:             13 October 2008
#	Authors:          Philippe Verdy <verdy_p AT wanadoo.fr>,
#	                  Helmut Richter <richter@lrz.de>
#
#	Copyright (c) 1991-2008 Unicode, Inc.  All Rights reserved.
#
#	This file is provided as-is by Unicode, Inc. (The Unicode Consortium).
#	No claims are made as to fitness for any particular purpose.  No
#	warranties of any kind are expressed or implied.  The recipient
#	agrees to determine applicability of information provided.  If this
#	file has been provided on optical media by Unicode, Inc., the sole
#	remedy for any claim will be exchange of defective media within 90
#	days of receipt.
#
#	Unicode, Inc. hereby grants the right to freely use the information
#	supplied in this file in the creation of products supporting the
#	Unicode Standard, and to make copies of this file in any form for
#	internal or external distribution as long as this notice remains
#	attached.
#
#	General notes:
#
#	This table contains the data the Unicode Consortium has on how
#	KOI8-U characters map into Unicode. The underlying document is the
#	mapping described in RFC 2319. No statements are made as to whether
#	this mapping is the same as the mapping defined as "Code Page 878"
#	with some vendors.
#
#	The KOI8-U (RFC2319) encoding is a variant based on KOI8-R (RFC1489),
#	where the support for four additional Cyrillic letters was added (both small
#	and capital letters), replacing 8 box-drawing characters. It is still widely
#	used to encode texts in Ukrainian, Byelorussian and Bulgarian.
#
#	Format:  Three tab-separated columns
#		 Column #1 is the KOI8-U code (in hex as 0xXX)
#		 Column #2 is the Unicode (in hex as 0xXXXX)
#		 Column #3 the Unicode name (follows a comment sign, '#')
#
#	The entries are in KOI8-U order.
#
#	Version history
#	1.0 version: created.
#
#	Updated versions of this file may be found in:
#		<http://www.unicode.org/Public/MAPPINGS/>
#
#	Any comments or problems, contact the Unicode Consortium at:
#	        <http://www.unicode.org/reporting.html>
#
***************************************************************************/
static const unicode_char lookup_koi8_u[256] = {
/* 00 */	0x0000,	//	NULL
/* 01 */	0x0001,	//	START OF HEADING
/* 02 */	0x0002,	//	START OF TEXT
/* 03 */	0x0003,	//	END OF TEXT
/* 04 */	0x0004,	//	END OF TRANSMISSION
/* 05 */	0x0005,	//	ENQUIRY
/* 06 */	0x0006,	//	ACKNOWLEDGE
/* 07 */	0x0007,	//	BELL
/* 08 */	0x0008,	//	BACKSPACE
/* 09 */	0x0009,	//	HORIZONTAL TABULATION
/* 0A */	0x000A,	//	LINE FEED
/* 0B */	0x000B,	//	VERTICAL TABULATION
/* 0C */	0x000C,	//	FORM FEED
/* 0D */	0x000D,	//	CARRIAGE RETURN
/* 0E */	0x000E,	//	SHIFT OUT
/* 0F */	0x000F,	//	SHIFT IN
/* 10 */	0x0010,	//	DATA LINK ESCAPE
/* 11 */	0x0011,	//	DEVICE CONTROL ONE
/* 12 */	0x0012,	//	DEVICE CONTROL TWO
/* 13 */	0x0013,	//	DEVICE CONTROL THREE
/* 14 */	0x0014,	//	DEVICE CONTROL FOUR
/* 15 */	0x0015,	//	NEGATIVE ACKNOWLEDGE
/* 16 */	0x0016,	//	SYNCHRONOUS IDLE
/* 17 */	0x0017,	//	END OF TRANSMISSION BLOCK
/* 18 */	0x0018,	//	CANCEL
/* 19 */	0x0019,	//	END OF MEDIUM
/* 1A */	0x001A,	//	SUBSTITUTE
/* 1B */	0x001B,	//	ESCAPE
/* 1C */	0x001C,	//	FILE SEPARATOR
/* 1D */	0x001D,	//	GROUP SEPARATOR
/* 1E */	0x001E,	//	RECORD SEPARATOR
/* 1F */	0x001F,	//	UNIT SEPARATOR
/* 20 */	0x0020,	//	SPACE
/* 21 */	0x0021,	//	EXCLAMATION MARK
/* 22 */	0x0022,	//	QUOTATION MARK
/* 23 */	0x0023,	//	NUMBER SIGN
/* 24 */	0x0024,	//	DOLLAR SIGN
/* 25 */	0x0025,	//	PERCENT SIGN
/* 26 */	0x0026,	//	AMPERSAND
/* 27 */	0x0027,	//	APOSTROPHE
/* 28 */	0x0028,	//	LEFT PARENTHESIS
/* 29 */	0x0029,	//	RIGHT PARENTHESIS
/* 2A */	0x002A,	//	ASTERISK
/* 2B */	0x002B,	//	PLUS SIGN
/* 2C */	0x002C,	//	COMMA
/* 2D */	0x002D,	//	HYPHEN-MINUS
/* 2E */	0x002E,	//	FULL STOP
/* 2F */	0x002F,	//	SOLIDUS
/* 30 */	0x0030,	//	DIGIT ZERO
/* 31 */	0x0031,	//	DIGIT ONE
/* 32 */	0x0032,	//	DIGIT TWO
/* 33 */	0x0033,	//	DIGIT THREE
/* 34 */	0x0034,	//	DIGIT FOUR
/* 35 */	0x0035,	//	DIGIT FIVE
/* 36 */	0x0036,	//	DIGIT SIX
/* 37 */	0x0037,	//	DIGIT SEVEN
/* 38 */	0x0038,	//	DIGIT EIGHT
/* 39 */	0x0039,	//	DIGIT NINE
/* 3A */	0x003A,	//	COLON
/* 3B */	0x003B,	//	SEMICOLON
/* 3C */	0x003C,	//	LESS-THAN SIGN
/* 3D */	0x003D,	//	EQUALS SIGN
/* 3E */	0x003E,	//	GREATER-THAN SIGN
/* 3F */	0x003F,	//	QUESTION MARK
/* 40 */	0x0040,	//	COMMERCIAL AT
/* 41 */	0x0041,	//	LATIN CAPITAL LETTER A
/* 42 */	0x0042,	//	LATIN CAPITAL LETTER B
/* 43 */	0x0043,	//	LATIN CAPITAL LETTER C
/* 44 */	0x0044,	//	LATIN CAPITAL LETTER D
/* 45 */	0x0045,	//	LATIN CAPITAL LETTER E
/* 46 */	0x0046,	//	LATIN CAPITAL LETTER F
/* 47 */	0x0047,	//	LATIN CAPITAL LETTER G
/* 48 */	0x0048,	//	LATIN CAPITAL LETTER H
/* 49 */	0x0049,	//	LATIN CAPITAL LETTER I
/* 4A */	0x004A,	//	LATIN CAPITAL LETTER J
/* 4B */	0x004B,	//	LATIN CAPITAL LETTER K
/* 4C */	0x004C,	//	LATIN CAPITAL LETTER L
/* 4D */	0x004D,	//	LATIN CAPITAL LETTER M
/* 4E */	0x004E,	//	LATIN CAPITAL LETTER N
/* 4F */	0x004F,	//	LATIN CAPITAL LETTER O
/* 50 */	0x0050,	//	LATIN CAPITAL LETTER P
/* 51 */	0x0051,	//	LATIN CAPITAL LETTER Q
/* 52 */	0x0052,	//	LATIN CAPITAL LETTER R
/* 53 */	0x0053,	//	LATIN CAPITAL LETTER S
/* 54 */	0x0054,	//	LATIN CAPITAL LETTER T
/* 55 */	0x0055,	//	LATIN CAPITAL LETTER U
/* 56 */	0x0056,	//	LATIN CAPITAL LETTER V
/* 57 */	0x0057,	//	LATIN CAPITAL LETTER W
/* 58 */	0x0058,	//	LATIN CAPITAL LETTER X
/* 59 */	0x0059,	//	LATIN CAPITAL LETTER Y
/* 5A */	0x005A,	//	LATIN CAPITAL LETTER Z
/* 5B */	0x005B,	//	LEFT SQUARE BRACKET
/* 5C */	0x005C,	//	REVERSE SOLIDUS
/* 5D */	0x005D,	//	RIGHT SQUARE BRACKET
/* 5E */	0x005E,	//	CIRCUMFLEX ACCENT
/* 5F */	0x005F,	//	LOW LINE
/* 60 */	0x0060,	//	GRAVE ACCENT
/* 61 */	0x0061,	//	LATIN SMALL LETTER A
/* 62 */	0x0062,	//	LATIN SMALL LETTER B
/* 63 */	0x0063,	//	LATIN SMALL LETTER C
/* 64 */	0x0064,	//	LATIN SMALL LETTER D
/* 65 */	0x0065,	//	LATIN SMALL LETTER E
/* 66 */	0x0066,	//	LATIN SMALL LETTER F
/* 67 */	0x0067,	//	LATIN SMALL LETTER G
/* 68 */	0x0068,	//	LATIN SMALL LETTER H
/* 69 */	0x0069,	//	LATIN SMALL LETTER I
/* 6A */	0x006A,	//	LATIN SMALL LETTER J
/* 6B */	0x006B,	//	LATIN SMALL LETTER K
/* 6C */	0x006C,	//	LATIN SMALL LETTER L
/* 6D */	0x006D,	//	LATIN SMALL LETTER M
/* 6E */	0x006E,	//	LATIN SMALL LETTER N
/* 6F */	0x006F,	//	LATIN SMALL LETTER O
/* 70 */	0x0070,	//	LATIN SMALL LETTER P
/* 71 */	0x0071,	//	LATIN SMALL LETTER Q
/* 72 */	0x0072,	//	LATIN SMALL LETTER R
/* 73 */	0x0073,	//	LATIN SMALL LETTER S
/* 74 */	0x0074,	//	LATIN SMALL LETTER T
/* 75 */	0x0075,	//	LATIN SMALL LETTER U
/* 76 */	0x0076,	//	LATIN SMALL LETTER V
/* 77 */	0x0077,	//	LATIN SMALL LETTER W
/* 78 */	0x0078,	//	LATIN SMALL LETTER X
/* 79 */	0x0079,	//	LATIN SMALL LETTER Y
/* 7A */	0x007A,	//	LATIN SMALL LETTER Z
/* 7B */	0x007B,	//	LEFT CURLY BRACKET
/* 7C */	0x007C,	//	VERTICAL LINE
/* 7D */	0x007D,	//	RIGHT CURLY BRACKET
/* 7E */	0x007E,	//	TILDE
/* 7F */	0x007F,	//	DELETE
/* 80 */	0x2500,	//	BOX DRAWINGS LIGHT HORIZONTAL
/* 81 */	0x2502,	//	BOX DRAWINGS LIGHT VERTICAL
/* 82 */	0x250C,	//	BOX DRAWINGS LIGHT DOWN AND RIGHT
/* 83 */	0x2510,	//	BOX DRAWINGS LIGHT DOWN AND LEFT
/* 84 */	0x2514,	//	BOX DRAWINGS LIGHT UP AND RIGHT
/* 85 */	0x2518,	//	BOX DRAWINGS LIGHT UP AND LEFT
/* 86 */	0x251C,	//	BOX DRAWINGS LIGHT VERTICAL AND RIGHT
/* 87 */	0x2524,	//	BOX DRAWINGS LIGHT VERTICAL AND LEFT
/* 88 */	0x252C,	//	BOX DRAWINGS LIGHT DOWN AND HORIZONTAL
/* 89 */	0x2534,	//	BOX DRAWINGS LIGHT UP AND HORIZONTAL
/* 8A */	0x253C,	//	BOX DRAWINGS LIGHT VERTICAL AND HORIZONTAL
/* 8B */	0x2580,	//	UPPER HALF BLOCK
/* 8C */	0x2584,	//	LOWER HALF BLOCK
/* 8D */	0x2588,	//	FULL BLOCK
/* 8E */	0x258C,	//	LEFT HALF BLOCK
/* 8F */	0x2590,	//	RIGHT HALF BLOCK
/* 90 */	0x2591,	//	LIGHT SHADE
/* 91 */	0x2592,	//	MEDIUM SHADE
/* 92 */	0x2593,	//	DARK SHADE
/* 93 */	0x2320,	//	TOP HALF INTEGRAL
/* 94 */	0x25A0,	//	BLACK SQUARE
/* 95 */	0x2219,	//	BULLET OPERATOR
/* 96 */	0x221A,	//	SQUARE ROOT
/* 97 */	0x2248,	//	ALMOST EQUAL TO
/* 98 */	0x2264,	//	LESS-THAN OR EQUAL TO
/* 99 */	0x2265,	//	GREATER-THAN OR EQUAL TO
/* 9A */	0x00A0,	//	NO-BREAK SPACE
/* 9B */	0x2321,	//	BOTTOM HALF INTEGRAL
/* 9C */	0x00B0,	//	DEGREE SIGN
/* 9D */	0x00B2,	//	SUPERSCRIPT TWO
/* 9E */	0x00B7,	//	MIDDLE DOT
/* 9F */	0x00F7,	//	DIVISION SIGN
/* A0 */	0x2550,	//	BOX DRAWINGS DOUBLE HORIZONTAL
/* A1 */	0x2551,	//	BOX DRAWINGS DOUBLE VERTICAL
/* A2 */	0x2552,	//	BOX DRAWINGS DOWN SINGLE AND RIGHT DOUBLE
/* A3 */	0x0451,	//	CYRILLIC SMALL LETTER IO
/* A4 */	0x0454,	//	CYRILLIC SMALL LETTER UKRAINIAN IE
/* A5 */	0x2554,	//	BOX DRAWINGS DOUBLE DOWN AND RIGHT
/* A6 */	0x0456,	//	CYRILLIC SMALL LETTER BYELORUSSIAN-UKRAINIAN I
/* A7 */	0x0457,	//	CYRILLIC SMALL LETTER YI (UKRAINIAN)
/* A8 */	0x2557,	//	BOX DRAWINGS DOUBLE DOWN AND LEFT
/* A9 */	0x2558,	//	BOX DRAWINGS UP SINGLE AND RIGHT DOUBLE
/* AA */	0x2559,	//	BOX DRAWINGS UP DOUBLE AND RIGHT SINGLE
/* AB */	0x255A,	//	BOX DRAWINGS DOUBLE UP AND RIGHT
/* AC */	0x255B,	//	BOX DRAWINGS UP SINGLE AND LEFT DOUBLE
/* AD */	0x0491,	//	CYRILLIC SMALL LETTER GHE WITH UPTURN
/* AE */	0x255D,	//	BOX DRAWINGS DOUBLE UP AND LEFT
/* AF */	0x255E,	//	BOX DRAWINGS VERTICAL SINGLE AND RIGHT DOUBLE
/* B0 */	0x255F,	//	BOX DRAWINGS VERTICAL DOUBLE AND RIGHT SINGLE
/* B1 */	0x2560,	//	BOX DRAWINGS DOUBLE VERTICAL AND RIGHT
/* B2 */	0x2561,	//	BOX DRAWINGS VERTICAL SINGLE AND LEFT DOUBLE
/* B3 */	0x0401,	//	CYRILLIC CAPITAL LETTER IO
/* B4 */	0x0404,	//	CYRILLIC CAPITAL LETTER UKRAINIAN IE
/* B5 */	0x2563,	//	BOX DRAWINGS DOUBLE VERTICAL AND LEFT
/* B6 */	0x0406,	//	CYRILLIC CAPITAL LETTER BYELORUSSIAN-UKRAINIAN I
/* B7 */	0x0407,	//	CYRILLIC CAPITAL LETTER YI (UKRAINIAN)
/* B8 */	0x2566,	//	BOX DRAWINGS DOUBLE DOWN AND HORIZONTAL
/* B9 */	0x2567,	//	BOX DRAWINGS UP SINGLE AND HORIZONTAL DOUBLE
/* BA */	0x2568,	//	BOX DRAWINGS UP DOUBLE AND HORIZONTAL SINGLE
/* BB */	0x2569,	//	BOX DRAWINGS DOUBLE UP AND HORIZONTAL
/* BC */	0x256A,	//	BOX DRAWINGS VERTICAL SINGLE AND HORIZONTAL DOUBLE
/* BD */	0x0490,	//	CYRILLIC CAPITAL LETTER GHE WITH UPTURN
/* BE */	0x256C,	//	BOX DRAWINGS DOUBLE VERTICAL AND HORIZONTAL
/* BF */	0x00A9,	//	COPYRIGHT SIGN
/* C0 */	0x044E,	//	CYRILLIC SMALL LETTER YU
/* C1 */	0x0430,	//	CYRILLIC SMALL LETTER A
/* C2 */	0x0431,	//	CYRILLIC SMALL LETTER BE
/* C3 */	0x0446,	//	CYRILLIC SMALL LETTER TSE
/* C4 */	0x0434,	//	CYRILLIC SMALL LETTER DE
/* C5 */	0x0435,	//	CYRILLIC SMALL LETTER IE
/* C6 */	0x0444,	//	CYRILLIC SMALL LETTER EF
/* C7 */	0x0433,	//	CYRILLIC SMALL LETTER GHE
/* C8 */	0x0445,	//	CYRILLIC SMALL LETTER HA
/* C9 */	0x0438,	//	CYRILLIC SMALL LETTER I
/* CA */	0x0439,	//	CYRILLIC SMALL LETTER SHORT I
/* CB */	0x043A,	//	CYRILLIC SMALL LETTER KA
/* CC */	0x043B,	//	CYRILLIC SMALL LETTER EL
/* CD */	0x043C,	//	CYRILLIC SMALL LETTER EM
/* CE */	0x043D,	//	CYRILLIC SMALL LETTER EN
/* CF */	0x043E,	//	CYRILLIC SMALL LETTER O
/* D0 */	0x043F,	//	CYRILLIC SMALL LETTER PE
/* D1 */	0x044F,	//	CYRILLIC SMALL LETTER YA
/* D2 */	0x0440,	//	CYRILLIC SMALL LETTER ER
/* D3 */	0x0441,	//	CYRILLIC SMALL LETTER ES
/* D4 */	0x0442,	//	CYRILLIC SMALL LETTER TE
/* D5 */	0x0443,	//	CYRILLIC SMALL LETTER U
/* D6 */	0x0436,	//	CYRILLIC SMALL LETTER ZHE
/* D7 */	0x0432,	//	CYRILLIC SMALL LETTER VE
/* D8 */	0x044C,	//	CYRILLIC SMALL LETTER SOFT SIGN
/* D9 */	0x044B,	//	CYRILLIC SMALL LETTER YERU
/* DA */	0x0437,	//	CYRILLIC SMALL LETTER ZE
/* DB */	0x0448,	//	CYRILLIC SMALL LETTER SHA
/* DC */	0x044D,	//	CYRILLIC SMALL LETTER E
/* DD */	0x0449,	//	CYRILLIC SMALL LETTER SHCHA
/* DE */	0x0447,	//	CYRILLIC SMALL LETTER CHE
/* DF */	0x044A,	//	CYRILLIC SMALL LETTER HARD SIGN
/* E0 */	0x042E,	//	CYRILLIC CAPITAL LETTER YU
/* E1 */	0x0410,	//	CYRILLIC CAPITAL LETTER A
/* E2 */	0x0411,	//	CYRILLIC CAPITAL LETTER BE
/* E3 */	0x0426,	//	CYRILLIC CAPITAL LETTER TSE
/* E4 */	0x0414,	//	CYRILLIC CAPITAL LETTER DE
/* E5 */	0x0415,	//	CYRILLIC CAPITAL LETTER IE
/* E6 */	0x0424,	//	CYRILLIC CAPITAL LETTER EF
/* E7 */	0x0413,	//	CYRILLIC CAPITAL LETTER GHE
/* E8 */	0x0425,	//	CYRILLIC CAPITAL LETTER HA
/* E9 */	0x0418,	//	CYRILLIC CAPITAL LETTER I
/* EA */	0x0419,	//	CYRILLIC CAPITAL LETTER SHORT I
/* EB */	0x041A,	//	CYRILLIC CAPITAL LETTER KA
/* EC */	0x041B,	//	CYRILLIC CAPITAL LETTER EL
/* ED */	0x041C,	//	CYRILLIC CAPITAL LETTER EM
/* EE */	0x041D,	//	CYRILLIC CAPITAL LETTER EN
/* EF */	0x041E,	//	CYRILLIC CAPITAL LETTER O
/* F0 */	0x041F,	//	CYRILLIC CAPITAL LETTER PE
/* F1 */	0x042F,	//	CYRILLIC CAPITAL LETTER YA
/* F2 */	0x0420,	//	CYRILLIC CAPITAL LETTER ER
/* F3 */	0x0421,	//	CYRILLIC CAPITAL LETTER ES
/* F4 */	0x0422,	//	CYRILLIC CAPITAL LETTER TE
/* F5 */	0x0423,	//	CYRILLIC CAPITAL LETTER U
/* F6 */	0x0416,	//	CYRILLIC CAPITAL LETTER ZHE
/* F7 */	0x0412,	//	CYRILLIC CAPITAL LETTER VE
/* F8 */	0x042C,	//	CYRILLIC CAPITAL LETTER SOFT SIGN
/* F9 */	0x042B,	//	CYRILLIC CAPITAL LETTER YERU
/* FA */	0x0417,	//	CYRILLIC CAPITAL LETTER ZE
/* FB */	0x0428,	//	CYRILLIC CAPITAL LETTER SHA
/* FC */	0x042D,	//	CYRILLIC CAPITAL LETTER E
/* FD */	0x0429,	//	CYRILLIC CAPITAL LETTER SHCHA
/* FE */	0x0427,	//	CYRILLIC CAPITAL LETTER CHE
/* FF */	0x042A	//	CYRILLIC CAPITAL LETTER HARD SIGN
};

//**************************************************************************
//  Unicode lookup table accessors
//**************************************************************************

/*-------------------------------------------------
	unicode_iso8859 - return a pointer to a lookup
	table for the specified ISO-8859 code,
	where code is 1-11, 13-16
-------------------------------------------------*/

const unicode_char* unicode_iso8859(int code)
{
	switch (code) {
	case  1: return lookup_iso8859_1;
	case  2: return lookup_iso8859_2;
	case  3: return lookup_iso8859_3;
	case  4: return lookup_iso8859_4;
	case  5: return lookup_iso8859_5;
	case  6: return lookup_iso8859_6;
	case  7: return lookup_iso8859_7;
	case  8: return lookup_iso8859_8;
	case  9: return lookup_iso8859_9;
	case 10: return lookup_iso8859_10;
	case 11: return lookup_iso8859_11;
	case 13: return lookup_iso8859_13;
	case 14: return lookup_iso8859_14;
	case 15: return lookup_iso8859_15;
	case 16: return lookup_iso8859_16;
	}
	return NULL;
}

/*-------------------------------------------------
	unicode_atari_st - return a pointer to a lookup
	table for the Atari ST character codes
-------------------------------------------------*/
const unicode_char* unicode_atari_st()
{
	return lookup_atari_st;
}

/*-------------------------------------------------
	unicode_zx81 - return a pointer to a lookup
	table for the ZX-81 character codes
-------------------------------------------------*/
const unicode_char* unicode_zx81()
{
	return lookup_zx81;
}

/*-------------------------------------------------
	unicode_koi8_r - return a pointer to a lookup
	table for the KOI8-R (RFC1489) character codes
-------------------------------------------------*/
const unicode_char* unicode_koi8_r()
{
	return lookup_koi8_r;
}

/*-------------------------------------------------
	unicode_koi8_u - return a pointer to a lookup
	table for the KOI8-U (RFC2319) character codes
-------------------------------------------------*/
const unicode_char* unicode_koi8_u()
{
	return lookup_koi8_u;
}
