// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    charconv.c

    Imgtool character set conversion routines.

***************************************************************************/

#include "corestr.h"
#include "charconv.h"


/*-------------------------------------------------
    utf8_from_latin1 - convert an ISO-8859-1
    character sequence to an UTF-8 string
-------------------------------------------------*/

static char *utf8_from_latin1(const char *src)
{
	char *buffer, *bufptr;

	/* validate input */
	if (!src)
	{
		return nullptr;
	}

	/* allocate space for result, twice the source len to be safe */
	buffer = (char *) osd_malloc(strlen(src) * 2 + 1);

	/* point to the start */
	bufptr = buffer;

	do
	{
		unsigned char c = *src;

		if (c < 0x80)
		{
			*bufptr++ = c;
		}
		else if (c < 0xc0)
		{
			*bufptr++ = '\xc2';
			*bufptr++ = c;
		}
		else
		{
			*bufptr++ = '\xc3';
			*bufptr++ = c - 0x40;
		}
	} while (*src++);

	return buffer;
}


/*-------------------------------------------------
    latin1_from_utf8 - convert an UTF-8
    character sequence to an ISO-8859-1 string
-------------------------------------------------*/

static char *latin1_from_utf8(const char *src)
{
	char *buffer, *bufptr;

	/* validate input */
	if (!src)
	{
		return nullptr;
	}

	/* allocate space for result */
	buffer = (char *) osd_malloc(strlen(src) + 1);

	/* point to the start */
	bufptr = buffer;

	do
	{
		unsigned char c = *src;

		if (c < 0x80)
		{
			*bufptr++ = c;
		}
		else if (c == 0xc2)
		{
			c = *++src;
			*bufptr++ = c;
		}
		else if (c == 0xc3)
		{
			c = *++src;
			*bufptr++ = c + 0x40;
		}
		else
		{
			/* conversion failed */
			*bufptr++ = '\0';
			break;
		}
	} while(*src++);

	return buffer;
}


/*-------------------------------------------------
    utf8_from_native - convert specified character
    sequence to an UTF-8 string
-------------------------------------------------*/

char *utf8_from_native(imgtool_charset charset, const char *src)
{
	char *result;

	switch (charset)
	{
		case IMGTOOL_CHARSET_UTF8:
			result = core_strdup(src);
			break;

		case IMGTOOL_CHARSET_ISO_8859_1:
			result = utf8_from_latin1(src);
			break;

		default:
			result = nullptr;
			break;
	}
	return result;
}


/*-------------------------------------------------
    native_from_utf8 - convert an UTF-8 string
    to specified character set
-------------------------------------------------*/

char *native_from_utf8(imgtool_charset charset, const char *src)
{
	char *result;

	switch (charset)
	{
		case IMGTOOL_CHARSET_UTF8:
			result = core_strdup(src);
			break;

		case IMGTOOL_CHARSET_ISO_8859_1:
			result = latin1_from_utf8(src);
			break;

		default:
			result = nullptr;
			break;
	}
	return result;
}
