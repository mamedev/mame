// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    charconv.h

    Imgtool character set conversion routines.

***************************************************************************/


#ifndef __CHARCONV_H__
#define __CHARCONV_H__


/* Supported character sets */
enum imgtool_charset
{
	IMGTOOL_CHARSET_UTF8,
	IMGTOOL_CHARSET_ISO_8859_1,
};


/* Convert specified charset to UTF-8 */
char *utf8_from_native(imgtool_charset charset, const char *src);

/* Convert UTF-8 string to specified charset */
char *native_from_utf8(imgtool_charset charset, const char *src);


#endif /* __CHARCONV_H__ */
