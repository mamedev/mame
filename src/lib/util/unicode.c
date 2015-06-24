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


/*-------------------------------------------------
    utf8_is_valid_string - return true if the
    given string is a properly formed sequence of
    UTF-8 characters
-------------------------------------------------*/

/**
 * @fn  int utf8_is_valid_string(const char *utf8string)
 *
 * @brief   UTF 8 is valid string.
 *
 * @param   utf8string  The UTF 8string.
 *
 * @return  An int.
 */

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
