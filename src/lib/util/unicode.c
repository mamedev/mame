// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Jürgen Buchmüller
/*********************************************************************

	unicode.c

	Unicode related functions

***************************************************************************/

#include "unicode.h"


/**
 * @brief test for legitimate unicode values
 *
 * return true if a given character is a legitimate unicode character
 *
 * @param uchar value to inspect
 * @return non zero (true) if uchar is valid, 0 otherwise
 */
int uchar_isvalid(unicode_char uchar)
{
	return (uchar < 0x110000) && !((uchar >= 0xd800) && (uchar <= 0xdfff));
}


/**
 * @brief convert an UTF-8 sequence into an unicode character
 * @param uchar pointer to the resulting unicode_char
 * @param utf8char pointer to the source string (may be NULL)
 * @param count number of characters available in utf8char
 * @return the number of characters used
 */
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


/**
 * @brief convert a UTF-16 sequence	into an unicode character
 * @param uchar pointer to the resulting unicode_char
 * @param utf16char pointer to the source string (may be NULL)
 * @param count number of characters available in utf16char
 * @return the number of characters used
 */
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


/**
 * @brief convert a UTF-16 sequence	into an unicode character from a flipped byte order
 *
 * This flips endianness of the first two utf16_char in a local
 * copy and then calls uchar_from_utf16.
 *
 * @param uchar pointer to the resulting unicode_char
 * @param utf16char pointer to the source string (may be NULL)
 * @param count number of characters available in utf16char
 * @return the number of characters used
 */
int uchar_from_utf16f(unicode_char *uchar, const utf16_char *utf16char, size_t count)
{
	utf16_char buf[2] = {0};
	if (count > 0)
		buf[0] = FLIPENDIAN_INT16(utf16char[0]);
	if (count > 1)
		buf[1] = FLIPENDIAN_INT16(utf16char[1]);
	return uchar_from_utf16(uchar, buf, count);
}


/**
 * @brief convert an unicode character into a UTF-8 sequence
 * @param utf8string pointer to the result char array
 * @param count number of characters that can be written to utf8string
 * @param uchar unciode_char value to convert
 * @return -1 on error, or the number of chars written on success (1 to 6)
 */
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

/**
 * @brief convert an unicode character into a UTF-16 sequence
 * @param utf16string pointer to the result array of utf16_char
 * @param count number of characters that can be written to utf16string
 * @param uchar unciode_char value to convert
 * @return -1 on error, or the number of utf16_char written on success (1 or 2)
 */
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

/**
 * @brief convert an unicode character into a UTF-16 sequence with flipped endianness
 * @param utf16string pointer to the result array of utf16_char
 * @param count number of characters that can be written to utf16string
 * @param uchar unciode_char value to convert
 * @return -1 on error, or the number of utf16_char written on success (1 or 2)
 */
int utf16f_from_uchar(utf16_char *utf16string, size_t count, unicode_char uchar)
{
	int rc;
	utf16_char buf[2] = { 0, 0 };

	rc = utf16_from_uchar(buf, 2, uchar);

	if (rc >= 1 && count >= 1)
		utf16string[0] = FLIPENDIAN_INT16(buf[0]);
	if (rc >= 2 && count >= 2)
		utf16string[1] = FLIPENDIAN_INT16(buf[1]);
	return rc < count ? rc : count;
}


/**
 * @brief return a pointer to the previous character in a string
 * @param utf8string const pointer to the starting position in the string
 * @return pointer to the character which is not an UTF-8 auxiliary character
 */
const char *utf8_previous_char(const char *utf8string)
{
	while ((*--utf8string & 0xc0) == 0x80)
		;
	return utf8string;
}

/**
 * @brief return true if the given string is a properly formed sequence of UTF-8 characters
 * @param utf8string const pointer to the source string
 * @return TRUE if the string is valid, FALSE otherwise
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

/**
 * @brief return the number of decoded Unicode values in UTF-8 encoded string
 * @param src pointer to the array of UTF-8 encoded characters
 * @return number of unicode_char values decoded from the UTF-8 string
 */
size_t utf8_strlen(const char* src)
{
	int total = 0;
	while (*src) {
		unicode_char uchar;
		int len = uchar_from_utf8(&uchar, src, strlen(src));
		if (len < 0)
			break;	// invalid UTF-8
		total++;
		src += len;
	}
	return total;
}

/**
 * @brief load a lookup table 8 bit codes to Unicode values
 *
 * This opens and reads a file %name which has to be in the
 * unicode.org defined "Format A".
 * That is three columns
 *   column 1: hex encoded 8 bit value of the code
 *   column 2: hex encoded 32 bit (max) unicode value
 *   column 3: a hash (#) and optional comment until the end-of-line
 *
 * @param name name of the (text) file to parse
 * @return pointer to a newly allocated array of 256 unicode_char values
 */
unicode_char * uchar_table_load(const char* name)
{
	FILE* file = fopen(name, "r");
	if (NULL == file)
		return NULL;
	unicode_char* table = (unicode_char*) calloc(sizeof(unicode_char), 256);
	if (NULL == table)
	{
		fclose(file);
		return NULL;
	}
	char *line = (char *)calloc(sizeof(char), 1024);

	while (fgets(line, 1024, file))
	{
		char* src = line;
		while (*src && isspace(*src))
			src++;
		if (*src == '#')
			continue;			// skip comment lines
		// expect Unicode "Table format: Format A"
		UINT32 uchar;
		UINT32 ucode;
		int n = sscanf(line, "%u %u #", &uchar, &ucode);
		if (n != 2)
			continue;			// silently skip of non-conforming lines
		if (uchar > 255)
			continue;			// can't define codes greater than 255
		table[uchar] = ucode;
	}
	free(line);
	return table;
}

/**
 * @brief return the 8 bit code that is mapped to the specified unicode_char
 * @param table table of 256 unicode_char values to use for the reverse lookup
 * @param uchar unicode value to revers lookup
 * @return UINT8 with the 8 bit code, or 255 if uchar wasn't found
 */
UINT8 uchar_table_index(unicode_char* table, unicode_char uchar)
{
	UINT8 index;
	for (index = 0; index < 254; index++)
		if (uchar == table[index])
			return index;
	return index;
}

/**
 * @brief free an unicode lookup table
 * @param table
 */
void uchar_table_free(unicode_char* table)
{
	if (table)
		free(table);
}

/**
 * @brief return the unicode_char array length
 * @param src pointer to an array of unicode_char
 * @return length of the array until the first 0
 */
size_t uchar_strlen(const unicode_char* src)
{
	int len = 0;
	while (*src++)
		len++;
	return len;
}

/**
 * @brief compare two unicode_char arrays
 * @param dst pointer to the first array of unicode_char
 * @param src pointer to the second array of unicode_char
 * @return 0 if dst == src, -1 if dst < src or +1 otherwise
 */
int uchar_strcmp(const unicode_char* dst, const unicode_char* src)
{
	while (*src && *dst && *src == *dst)
	{
		src++;
		dst++;
	}
	if (*src != *dst)
		return *src < *dst ? -1 : +1;
	return 0;
}

/**
 * @brief compare two unicode_char arrays with length limiting
 * @param dst pointer to the first array of unicode_char
 * @param src pointer to the second array of unicode_char
 * @param len maximum number of unicode_char to compare
 * @return 0 if dst == src, -1 if dst < src or +1 otherwise
 */
int uchar_strncmp(const unicode_char* dst, const unicode_char* src, size_t len)
{
	while (*src && *dst && *src == *dst && len > 0)
	{
		src++;
		dst++;
		len--;
	}
	if (*src != *dst)
		return *src < *dst ? -1 : +1;
	return 0;
}

/**
 * @brief print a formatted string of ASCII characters to an unicode_char array
 * @param dst pointer to the array
 * @param format format string followed by optional parameters
 * @return number of unicode_char stored in dst
 */
int uchar_sprintf(unicode_char* dst, const char* format, ...)
{
	va_list ap;
	char buff[256];
	va_start(ap, format);
	int len = vsnprintf(buff, sizeof(buff), format, ap);
	va_end(ap);
	for (int i = 0; i < len; i++)
		*dst++ = buff[i];
	*dst = 0;
	return len;
}

/**
 * @brief copy an array of unicode_char from source to destination
 *
 * @param dst pointer to destination array
 * @param src const pointer to the source array
 * @return a pointer to the original destination
 */
unicode_char* uchar_strcpy(unicode_char* dst, const unicode_char* src)
{
	unicode_char* str = dst;
	while (*src)
		*dst++ = *src++;
	return str;
}

/**
 * @brief copy a length limited array of unicode_char from source to destination
 *
 * This function always terminates dst with a 0 unicode_char, unlike some
 * classic libc implementations of strncpy(). This means that actually at
 * most len-1 unicode_char are copied from src to leave room for the 0 code.
 *
 * @param dst pointer to destination array
 * @param src const pointer to the source array
 * @param len maximum number of unicode_char to copy
 * @return a pointer to the original destination
 */
unicode_char* uchar_strncpy(unicode_char* dst, const unicode_char* src, size_t len)
{
	unicode_char* str = dst;
	while (*src && len > 1)
	{
		*dst++ = *src++;
		len--;
	}
	if (len > 0)
		*dst = 0;
	return str;
}

/***************************************************************************
 *
 * Parsing and access to the UnicodeData table published at unicode.org
 *
 ***************************************************************************/

//! Information about an unicode_char
typedef struct {
#if	NEED_UNICODE_NAME
	char *name;									//!< name of the character
#endif
#if	NEED_UNICODE_NAME10
	char *name10;								//!< short name of the character
#endif
#if	NEED_UNICODE_GCAT
	unicode_general_category gen_cat;			//!< general category
#endif
#if	NEED_UNICODE_CCOM
	UINT8 canonical_comb;						//!< canonical combining is needed
#endif
#if	NEED_UNICODE_BIDI
	unicode_bidirectional_category bidi;		//!< bidirectional category
#endif
#if	NEED_UNICODE_DECO
	unicode_decomposition_mapping decomp_map;	//!< decomposition mapping
#endif
#if	NEED_UNICODE_DECIMAL
	UINT8 decimal_digit;						//!< decimal digit value
#endif
#if	NEED_UNICODE_DIGIT
	UINT8 digit;								//!< digit value
#endif
#if	NEED_UNICODE_NUMERIC
	UINT8 numeric;								//!< non zero, if this code is numeric
#endif
#if	NEED_UNICODE_MIRRORED
	bool mirrored;								//!< true, if the value is a mirrored form
#endif
#if	NEED_UNICODE_DECN
	UINT8 n_decomp;								//!< number of decomposition codes
	unicode_char *decomp_codes;					//!< array of decomposition codes
#endif
#if	NEED_UNICODE_UCASE
	unicode_char uppercase;						//!< upper case value of this code
#endif
#if	NEED_UNICODE_LCASE
	unicode_char lowercase;						//!< lower case value of this code
#endif
#if	NEED_UNICODE_TCASE
	unicode_char titlecase;						//!< title case value of this code
#endif
#if	NEED_UNICODE_WIDTH
	UINT8 width;								//!< width of the glyph in cells
#endif
}	unicode_data_t;

static unicode_data_t** unicode_data = NULL;

#if	NEED_UNICODE_RANGES
typedef struct {
	unicode_char first, last;
	const char *name;
}	unicode_range_t;

static const unicode_range_t unicode_ranges[] =
{
	{0x0000, 0x007f, "Basic Latin"},
	{0x0080, 0x00ff, "Latin-1 Supplement"},
	{0x0100, 0x017f, "Latin Extended-A"},
	{0x0180, 0x024f, "Latin Extended-B"},
	{0x0250, 0x02af, "IPA Extensions"},
	{0x02b0, 0x02ff, "Spacing Modifier Letters"},
	{0x0300, 0x036f, "Combining Diacritical Marks"},
	{0x0370, 0x03ff, "Greek"},
	{0x0400, 0x04ff, "Cyrillic"},
	{0x0530, 0x058f, "Armenian"},
	{0x0590, 0x05ff, "Hebrew"},
	{0x0600, 0x06ff, "Arabic"},
	{0x0700, 0x074f, "Syriac"},
	{0x0780, 0x07bf, "Thaana"},
	{0x0900, 0x097f, "Devanagari"},
	{0x0980, 0x09ff, "Bengali"},
	{0x0a00, 0x0a7f, "Gurmukhi"},
	{0x0a80, 0x0aff, "Gujarati"},
	{0x0b00, 0x0b7f, "Oriya"},
	{0x0b80, 0x0bff, "Tamil"},
	{0x0c00, 0x0c7f, "Telugu"},
	{0x0c80, 0x0cff, "Kannada"},
	{0x0d00, 0x0d7f, "Malayalam"},
	{0x0d80, 0x0dff, "Sinhala"},
	{0x0e00, 0x0e7f, "Thai"},
	{0x0e80, 0x0eff, "Lao"},
	{0x0f00, 0x0fff, "Tibetan"},
	{0x1000, 0x109f, "Myanmar"},
	{0x10a0, 0x10ff, "Georgian"},
	{0x1100, 0x11ff, "Hangul Jamo"},
	{0x1200, 0x137f, "Ethiopic"},
	{0x13a0, 0x13ff, "Cherokee"},
	{0x1400, 0x167f, "Unified Canadian Aboriginal Syllabic"},
	{0x1680, 0x169f, "Ogham"},
	{0x16a0, 0x16ff, "Runic"},
	{0x1780, 0x17ff, "Khmer"},
	{0x1800, 0x18af, "Mongolian"},
	{0x1e00, 0x1eff, "Latin Extended Additional"},
	{0x1f00, 0x1fff, "Greek Extended"},
	{0x2000, 0x206f, "General Punctuation"},
	{0x2070, 0x208f, "Superscripts and Subscripts"},
	{0x20a0, 0x20cf, "Currency Symbols"},
	{0x20d0, 0x20ff, "Combining Marks for Symbols"},
	{0x2100, 0x214f, "Letterlike Symbols"},
	{0x2150, 0x218f, "Number Forms"},
	{0x2190, 0x21ff, "Arrows"},
	{0x2200, 0x22ff, "Mathematical Operators"},
	{0x2300, 0x23ff, "Miscellaneous Technical"},
	{0x2400, 0x243f, "Control Pictures"},
	{0x2440, 0x245f, "Optical Character Recognition"},
	{0x2460, 0x24ff, "Enclosed Alphanumerics"},
	{0x2500, 0x257f, "Box Drawing"},
	{0x2580, 0x259f, "Block Elements"},
	{0x25a0, 0x25ff, "Geometric Shapes"},
	{0x2600, 0x26ff, "Miscellaneous Symbols"},
	{0x2700, 0x27bf, "Dingbats"},
	{0x2800, 0x28ff, "Braille Patterns"},
	{0x2e80, 0x2eff, "CJK Radicals Supplement"},
	{0x2f00, 0x2fdf, "Kangxi Radicals"},
	{0x2ff0, 0x2fff, "Ideographic Description Characters"},
	{0x3000, 0x303f, "CJK Symbols and Punctuation"},
	{0x3040, 0x309f, "Hiragana"},
	{0x30a0, 0x30ff, "Katakana"},
	{0x3100, 0x312f, "Bopomofo"},
	{0x3130, 0x318f, "Hangul Compatibility Jamo"},
	{0x3190, 0x319f, "Kanbun"},
	{0x31a0, 0x31bf, "Bopomofo Extended"},
	{0x3200, 0x32ff, "Enclosed CJK Letters and Months"},
	{0x3300, 0x33ff, "CJK Compatibility"},
	{0x3400, 0x4dbf, "CJK Unified Ideographs Extension A"},
	{0x4e00, 0x9faf, "CJK Unified Ideographs"},
	{0xa000, 0xa48f, "Yi Syllables"},
	{0xa490, 0xa4cf, "Yi Radicals"},
	{0xac00, 0xd7af, "Hangul Syllables"},
	{0xd800, 0xdb7f, "High Surrogates"},
	{0xdb80, 0xdbff, "High Private Use Surrogates"},
	{0xdc00, 0xdfff, "Low Surrogates"},
	{0xe000, 0xf8ff, "Private Use"},
	{0xf900, 0xfaff, "CJK Compatibility Ideographs"},
	{0xfb00, 0xfb4f, "Alphabetic Presentation Forms"},
	{0xfb50, 0xfdff, "Arabic Presentation Forms-A"},
	{0xfe20, 0xfe2f, "Combining Half Marks"},
	{0xfe30, 0xfe4f, "CJK Compatibility Forms"},
	{0xfe50, 0xfe6f, "Small Form Variants"},
	{0xfe70, 0xfeff, "Arabic Presentation Forms-B"},
	{0xff00, 0xffef, "Halfwidth and Fullwidth Forms"},
	{0xfff0, 0xffff, "Specials"}
	// FIXME: add ranges for the Unicode planes 1 to 16
};
#endif

#if	NEED_UNICODE_CCOM
static const char *canonical_combining_str(UINT8 val)
{
	switch (val)
	{
	case 0:		return "Spacing, split, enclosing, reordrant, and Tibetan subjoined";
	case 1:		return "Overlays and interior";
	case 7:		return "Nuktas";
	case 8:		return "Hiragana/Katakana voicing marks";
	case 9:		return "Viramas";
	case 10:	return "Start of fixed position classes";
	case 199:	return "End of fixed position classes";
	case 200:	return "Below left attached";
	case 202:	return "Below attached";
	case 204:	return "Below right attached";
	case 208:	return "Left attached (reordrant around single base character)";
	case 210:	return "Right attached";
	case 212:	return "Above left attached";
	case 214:	return "Above attached";
	case 216:	return "Above right attached";
	case 218:	return "Below left";
	case 220:	return "Below";
	case 222:	return "Below right";
	case 224:	return "Left (reordrant around single base character)";
	case 226:	return "Right";
	case 228:	return "Above left";
	case 230:	return "Above";
	case 232:	return "Above right";
	case 233:	return "Double below";
	case 234:	return "Double above";
	case 240:	return "Below (iota subscript)";
	}
	return "INVALID";
}
#endif

#if	NEED_UNICODE_NAME
const char * unicode_name(unicode_char uchar)
{
	if (!unicode_data || uchar >= UNICODE_PLANESIZE || !unicode_data[uchar])
		return NULL;
	return unicode_data[uchar]->name;
}
#endif

#if	NEED_UNICODE_NAME10
const char * unicode_name10(unicode_char uchar)
{
	if (!unicode_data || uchar >= UNICODE_PLANESIZE || !unicode_data[uchar])
		return NULL;
	return unicode_data[uchar]->name10;
}
#endif

#if	NEED_UNICODE_GCAT
unicode_general_category unicode_gcat(unicode_char uchar)
{
	if (!unicode_data || uchar >= UNICODE_PLANESIZE || !unicode_data[uchar])
		return gcat_0;
	return unicode_data[uchar]->gen_cat;
}

const char * unicode_gcat_name(unicode_char uchar)
{
	if (!unicode_data || uchar >= UNICODE_PLANESIZE || !unicode_data[uchar])
		return "";
	switch (unicode_data[uchar]->gen_cat)
	{
	case gcat_Lu:	return "Lu: Letter, Uppercase";
	case gcat_Ll:	return "Ll: Letter, Lowercase";
	case gcat_Lt:	return "Lt: Letter, Titlecase";
	case gcat_Mn:	return "Mn: Mark, Non-Spacing";
	case gcat_Mc:	return "Mc: Mark, Spacing Combining";
	case gcat_Me:	return "Me: Mark, Enclosing";
	case gcat_Nd:	return "Nd: Number, Decimal Digit";
	case gcat_Nl:	return "Nl: Number, Letter";
	case gcat_No:	return "No: Number, Other";
	case gcat_Zs:	return "Zs: Separator, Space";
	case gcat_Zl:	return "Zl: Separator, Line";
	case gcat_Zp:	return "Zp: Separator, Paragraph";
	case gcat_Cc:	return "Cc: Other, Control";
	case gcat_Cf:	return "Cf: Other, Format";
	case gcat_Cs:	return "Cs: Other, Surrogate";
	case gcat_Co:	return "Co: Other, Private Use";
	case gcat_Cn:	return "Cn: Other, Not Assigned (no characters have this property)";
	case gcat_Lm:	return "Lm: Letter, Modifier";
	case gcat_Lo:	return "Lo: Letter, Other";
	case gcat_Pc:	return "Pc: Punctuation, Connector";
	case gcat_Pd:	return "Pd: Punctuation, Dash";
	case gcat_Ps:	return "Ps: Punctuation, Open";
	case gcat_Pe:	return "Pe: Punctuation, Close";
	case gcat_Pi:	return "Pi: Punctuation, Initial quote (may behave like Ps or Pe depending on usage)";
	case gcat_Pf:	return "Pf: Punctuation, Final quote (may behave like Ps or Pe depending on usage)";
	case gcat_Po:	return "Po: Punctuation, Other";
	case gcat_Sm:	return "Sm: Symbol, Math";
	case gcat_Sc:	return "Sc: Symbol, Currency";
	case gcat_Sk:	return "Sk: Symbol, Modifier";
	case gcat_So:	return "So: Symbol, Other";
	default:
		return "";
	}
}
#endif

#if	NEED_UNICODE_CCOM
UINT8 unicode_ccom(unicode_char uchar)
{
	if (!unicode_data || uchar >= UNICODE_PLANESIZE || !unicode_data[uchar])
		return 255;
	return unicode_data[uchar]->canonical_comb;
}

const char * unicode_ccom_name(unicode_char uchar)
{
	if (!unicode_data || uchar >= UNICODE_PLANESIZE || !unicode_data[uchar])
		return "";
	return canonical_combining_str(unicode_data[uchar]->canonical_comb);
}
#endif

#if	NEED_UNICODE_BIDI
unicode_bidirectional_category unicode_bidi(unicode_char uchar)
{
	if (!unicode_data || uchar >= UNICODE_PLANESIZE || !unicode_data[uchar])
		return bidi_0;
	return unicode_data[uchar]->bidi;
}

const char * unicode_bidi_name(unicode_char uchar)
{
	if (!unicode_data || uchar >= UNICODE_PLANESIZE || !unicode_data[uchar])
		return "";
	switch (unicode_data[uchar]->bidi)
	{
	case bidi_L:	return "L: Left-to-Right";
	case bidi_LRE:	return "LRE: Left-to-Right Embedding";
	case bidi_LRO:	return "LRO: Left-to-Right Override";
	case bidi_R:	return "R: Right-to-Left";
	case bidi_AL:	return "AL: Right-to-Left Arabic";
	case bidi_RLE:	return "RLE: Right-to-Left Embedding";
	case bidi_RLO:	return "RLO: Right-to-Left Override";
	case bidi_PDF:	return "PDF: Pop Directional Format";
	case bidi_EN:	return "EN: European Number";
	case bidi_ES:	return "ES: European Number Separator";
	case bidi_ET:	return "ET: European Number Terminator";
	case bidi_AN:	return "AN: Arabic Number";
	case bidi_CS:	return "CS: Common Number Separator";
	case bidi_NSM:	return "NSM: Non-Spacing Mark";
	case bidi_BN:	return "BN: Boundary Neutral";
	case bidi_B:	return "B: Paragraph Separator";
	case bidi_S:	return "S: Segment Separator";
	case bidi_WS:	return "WS: Whitespace";
	case bidi_ON:	return "ON: Other Neutrals";
	default:
		return "";
	}
}
#endif

#if	NEED_UNICODE_DECO
unicode_decomposition_mapping unicode_deco(unicode_char uchar)
{
	if (!unicode_data || uchar >= UNICODE_PLANESIZE || !unicode_data[uchar])
		return deco_0;
	return unicode_data[uchar]->decomp_map;
}

const char * unicode_deco_name(unicode_char uchar)
{
	if (!unicode_data || uchar >= UNICODE_PLANESIZE || !unicode_data[uchar])
		return "";
	switch (unicode_data[uchar]->decomp_map)
	{
	case deco_canonical:	return "Canonical mapping";
	case deco_font:			return "A font variant (e.g. a blackletter form)";
	case deco_noBreak:		return "A no-break version of a space or hyphen";
	case deco_initial:		return "An initial presentation form (Arabic)";
	case deco_medial:		return "A medial presentation form (Arabic)";
	case deco_final:		return "A final presentation form (Arabic)";
	case deco_isolated:		return "An isolated presentation form (Arabic)";
	case deco_circle:		return "An encircled form";
	case deco_super:		return "A superscript form";
	case deco_sub:			return "A subscript form";
	case deco_vertical:		return "A vertical layout presentation form";
	case deco_wide:			return "A wide (or zenkaku) compatibility character";
	case deco_narrow:		return "A narrow (or hankaku) compatibility character";
	case deco_small:		return "A small variant form (CNS compatibility)";
	case deco_square:		return "A CJK squared font variant";
	case deco_fraction:		return "A vulgar fraction form";
	case deco_compat:		return "Otherwise unspecified compatibility character";
	default:
		return "";
	}
}

#if	NEED_UNICODE_DECN
unicode_char unicode_deco_n(unicode_char uchar, int n)
{
	if (!unicode_data || uchar >= UNICODE_PLANESIZE || !unicode_data[uchar])
		return 0;
	if (n == -1)
		return unicode_data[uchar]->n_decomp;
	if (n < unicode_data[uchar]->n_decomp)
		return unicode_data[uchar]->decomp_codes[n];
	return 0;
}
#endif
#endif

#if	NEED_UNICODE_DIGIT
UINT8 unicode_decimal(unicode_char uchar)
{
	if (!unicode_data || uchar >= UNICODE_PLANESIZE || !unicode_data[uchar])
		return UNICODE_NOT_DECIMAL;
	return unicode_data[uchar]->decimal_digit;
}

UINT8 unicode_digit(unicode_char uchar)
{
	if (!unicode_data || uchar >= UNICODE_PLANESIZE || !unicode_data[uchar])
		return UNICODE_NOT_DIGIT;
	return unicode_data[uchar]->digit;
}
#endif

#if	NEED_UNICODE_NUMERIC
UINT8 unicode_numeric(unicode_char uchar)
{
	if (!unicode_data || uchar >= UNICODE_PLANESIZE || !unicode_data[uchar])
		return UNICODE_NOT_NUMERIC;
	return unicode_data[uchar]->numeric;
}
#endif

#if	NEED_UNICODE_MIRRORED
bool unicode_mirrored(unicode_char uchar)
{
	if (!unicode_data || uchar >= UNICODE_PLANESIZE || !unicode_data[uchar])
		return 0;
	return unicode_data[uchar]->mirrored;
}
#endif

#if	NEED_UNICODE_UCASE
unicode_char unicode_ucase(unicode_char uchar)
{
	if (!unicode_data || uchar >= UNICODE_PLANESIZE || !unicode_data[uchar])
		return uchar;
	if (unicode_data[uchar]->uppercase)
		return unicode_data[uchar]->uppercase;
	return uchar;
}
#endif

#if	NEED_UNICODE_LCASE
unicode_char unicode_lcase(unicode_char uchar)
{
	if (!unicode_data || uchar >= UNICODE_PLANESIZE || !unicode_data[uchar])
		return uchar;
	if (unicode_data[uchar]->lowercase)
		return unicode_data[uchar]->lowercase;
	return uchar;
}
#endif

#if	NEED_UNICODE_TCASE
unicode_char unicode_tcase(unicode_char uchar)
{
	if (!unicode_data || uchar >= UNICODE_PLANESIZE || !unicode_data[uchar])
		return uchar;
	if (unicode_data[uchar]->titlecase)
		return unicode_data[uchar]->titlecase;
	return uchar;
}
#endif

#if	NEED_UNICODE_WIDTH
int unicode_width(unicode_char uchar)
{
	/* sorted list of non-overlapping intervals of non-spacing characters */
	static const struct interval {
		unicode_char first, last;
	} combining[] = {
		{ 0x0300, 0x034E }, { 0x0360, 0x0362 }, { 0x0483, 0x0486 },
		{ 0x0488, 0x0489 }, { 0x0591, 0x05A1 }, { 0x05A3, 0x05B9 },
		{ 0x05BB, 0x05BD }, { 0x05BF, 0x05BF }, { 0x05C1, 0x05C2 },
		{ 0x05C4, 0x05C4 }, { 0x064B, 0x0655 }, { 0x0670, 0x0670 },
		{ 0x06D6, 0x06E4 }, { 0x06E7, 0x06E8 }, { 0x06EA, 0x06ED },
		{ 0x0711, 0x0711 }, { 0x0730, 0x074A }, { 0x07A6, 0x07B0 },
		{ 0x0901, 0x0902 }, { 0x093C, 0x093C }, { 0x0941, 0x0948 },
		{ 0x094D, 0x094D }, { 0x0951, 0x0954 }, { 0x0962, 0x0963 },
		{ 0x0981, 0x0981 }, { 0x09BC, 0x09BC }, { 0x09C1, 0x09C4 },
		{ 0x09CD, 0x09CD }, { 0x09E2, 0x09E3 }, { 0x0A02, 0x0A02 },
		{ 0x0A3C, 0x0A3C }, { 0x0A41, 0x0A42 }, { 0x0A47, 0x0A48 },
		{ 0x0A4B, 0x0A4D }, { 0x0A70, 0x0A71 }, { 0x0A81, 0x0A82 },
		{ 0x0ABC, 0x0ABC }, { 0x0AC1, 0x0AC5 }, { 0x0AC7, 0x0AC8 },
		{ 0x0ACD, 0x0ACD }, { 0x0B01, 0x0B01 }, { 0x0B3C, 0x0B3C },
		{ 0x0B3F, 0x0B3F }, { 0x0B41, 0x0B43 }, { 0x0B4D, 0x0B4D },
		{ 0x0B56, 0x0B56 }, { 0x0B82, 0x0B82 }, { 0x0BC0, 0x0BC0 },
		{ 0x0BCD, 0x0BCD }, { 0x0C3E, 0x0C40 }, { 0x0C46, 0x0C48 },
		{ 0x0C4A, 0x0C4D }, { 0x0C55, 0x0C56 }, { 0x0CBF, 0x0CBF },
		{ 0x0CC6, 0x0CC6 }, { 0x0CCC, 0x0CCD }, { 0x0D41, 0x0D43 },
		{ 0x0D4D, 0x0D4D }, { 0x0DCA, 0x0DCA }, { 0x0DD2, 0x0DD4 },
		{ 0x0DD6, 0x0DD6 }, { 0x0E31, 0x0E31 }, { 0x0E34, 0x0E3A },
		{ 0x0E47, 0x0E4E }, { 0x0EB1, 0x0EB1 }, { 0x0EB4, 0x0EB9 },
		{ 0x0EBB, 0x0EBC }, { 0x0EC8, 0x0ECD }, { 0x0F18, 0x0F19 },
		{ 0x0F35, 0x0F35 }, { 0x0F37, 0x0F37 }, { 0x0F39, 0x0F39 },
		{ 0x0F71, 0x0F7E }, { 0x0F80, 0x0F84 }, { 0x0F86, 0x0F87 },
		{ 0x0F90, 0x0F97 }, { 0x0F99, 0x0FBC }, { 0x0FC6, 0x0FC6 },
		{ 0x102D, 0x1030 }, { 0x1032, 0x1032 }, { 0x1036, 0x1037 },
		{ 0x1039, 0x1039 }, { 0x1058, 0x1059 }, { 0x17B7, 0x17BD },
		{ 0x17C6, 0x17C6 }, { 0x17C9, 0x17D3 }, { 0x18A9, 0x18A9 },
		{ 0x20D0, 0x20E3 }, { 0x302A, 0x302F }, { 0x3099, 0x309A },
		{ 0xFB1E, 0xFB1E }, { 0xFE20, 0xFE23 }
	};
	int _min = 0;
	int _max = sizeof(combining) / sizeof(combining[0]) - 1;
	int _mid;

	/* test for 8-bit control characters */
	if (uchar == 0)
		return 0;
	if (uchar < 32 || (uchar >= 0x7f && uchar < 0xa0))
		return -1;

	/* first quick check for Latin-1 etc. characters */
	if (uchar < combining[0].first)
		return 1;

	/* binary search in table of non-spacing characters */
	while (_max >= _min)
	{
		_mid = (_min + _max) / 2;
		if (combining[_mid].last < uchar)
			_min = _mid + 1;
		else if (combining[_mid].first > uchar)
			_max = _mid - 1;
		else if (combining[_mid].first <= uchar && combining[_mid].last >= uchar)
			return 0;
	}

	/* if we arrive here, uchar is not a combining or C0/C1 control character */

	/* fast test for majority of non-wide scripts */
	if (uchar < 0x1100)
		return 1;

	return 1 +
		((uchar >= 0x1100 && uchar <= 0x115f) || /* Hangul Jamo */
		 (uchar >= 0x2e80 && uchar <= 0xa4cf && (uchar & ~0x0011) != 0x300a && uchar != 0x303f) || /* CJK ... Yi */
		 (uchar >= 0xac00 && uchar <= 0xd7a3) || /* Hangul Syllables */
		 (uchar >= 0xf900 && uchar <= 0xfaff) || /* CJK Compatibility Ideographs */
		 (uchar >= 0xfe30 && uchar <= 0xfe6f) || /* CJK Compatibility Forms */
		 (uchar >= 0xff00 && uchar <= 0xff5f) || /* Fullwidth Forms */
		 (uchar >= 0xffe0 && uchar <= 0xffe6));
}
#endif

#if	NEED_UNICODE_RANGES
const char * unicode_range_name(unicode_char uchar)
{
	static UINT32 hit = 0;
	UINT32 i;

	for (i = hit; i < sizeof(unicode_ranges)/sizeof(unicode_ranges[0]); i++)
	{
		if (unicode_ranges[i].first <= uchar && uchar <= unicode_ranges[i].last)
		{
			hit = i;
			return unicode_ranges[i].name;
		}
	}
	for (i = 0; i < hit; i++)
	{
		if (unicode_ranges[i].first <= uchar && uchar <= unicode_ranges[i].last)
		{
			hit = i;
			return unicode_ranges[i].name;
		}
	}

	return NULL;
}

unicode_char unicode_range_first(unicode_char uchar)
{
	static UINT32 hit = 0;
	UINT32 i;

	for (i = hit; i < sizeof(unicode_ranges)/sizeof(unicode_ranges[0]); i++)
	{
		if (unicode_ranges[i].first <= uchar && uchar <= unicode_ranges[i].last)
		{
			hit = i;
			return unicode_ranges[i].first;
		}
	}
	for (i = 0; i < hit; i++)
	{
		if (unicode_ranges[i].first <= uchar && uchar <= unicode_ranges[i].last)
		{
			hit = i;
			return unicode_ranges[i].first;
		}
	}

	return uchar;
}

unicode_char unicode_range_last(unicode_char uchar)
{
	static UINT32 hit = 0;
	UINT32 i;

	for (i = hit; i < sizeof(unicode_ranges)/sizeof(unicode_ranges[0]); i++)
	{
		if (unicode_ranges[i].first <= uchar && uchar <= unicode_ranges[i].last)
		{
			hit = i;
			return unicode_ranges[i].last;
		}
	}
	for (i = 0; i < hit; i++)
	{
		if (unicode_ranges[i].first <= uchar && uchar <= unicode_ranges[i].last)
		{
			hit = i;
			return unicode_ranges[i].last;
		}
	}

	return uchar;
}
#endif

//! parse a string just as strtok would do - local static tok
static char *parse_strtok(char *src, const char *delim)
{
	static char *token = NULL;
	char *start;

	if (src)
		token = src;
	start = token;
	while (token && *token)
	{
		const char *d = delim;
		while (*d)
		{
			if (*token == *d)
			{
				*token++ = '\0';
				return start;
			}
			d++;
		}
		token++;
	}
	return NULL;
}

//! load the UnicodeData-x.y.txt file and parse it
int unicode_data_load(const char* name)
{
	// already loaded?
	if (unicode_data)
		return -1;

	FILE* file = fopen(name, "r");
	if (NULL == file)
		return -1;

	unicode_data = (unicode_data_t **) calloc(sizeof(unicode_data_t*), UNICODE_PLANESIZE);
	char *line = (char*) calloc(sizeof(char), 1024);
	int linenum = 0;
	unicode_char first = 0;
	unicode_char last = 0;

	while (fgets(line, 1024, file))
	{
		unicode_data_t u;
		unicode_char code;
		int tokennum = 1;
		char* parse = strdup(line);
		char* token;
		memset(&u, 0, sizeof(u));

		++linenum;
		if (NULL == (token = parse_strtok(parse, ";\r\n")))
			fprintf(stderr, "%s: token #%d failed on line %d\n%s", __FUNCTION__, tokennum, linenum, line);
		code = UNICODE_PLANESIZE;
		if (NULL != token)
			code = strtoul(token, NULL, 16);

		tokennum++;
		if (NULL == (token = parse_strtok(NULL, ";\r\n")))
			fprintf(stderr, "%s: token #%d failed on line %d\n%s", __FUNCTION__, tokennum, linenum, line);
		if (NULL != token)
		{
			// check for a range description
			if (token[0] == '<')
			{
				// strip a trailing ", First>" string fragment
				if (0 == strcmp(token + strlen(token) - 8,", First>"))
				{
					strcpy(token, token + 1);
					token[strlen(token) - 8] = '\0';
					first = code;
				}
				// strip a trailing ", Last>" string fragment
				if (0 == strcmp(token + strlen(token) - 7,", Last>"))
				{
					strcpy(token, token + 1);
					token[strlen(token) - 7] = '\0';
					last = code;
				}
			}
#if	NEED_UNICODE_NAME
			// allocate a string for the name
			u.name = strdup(token);
#endif
		}

		tokennum++;
		if (NULL == (token = parse_strtok(NULL, ";\r\n")))
			fprintf(stderr, "%s: token #%d failed on line %d\n%s", __FUNCTION__, tokennum, linenum, line);
#if	NEED_UNICODE_GCAT
		// parse general category
		u.gen_cat = gcat_0;
		if (NULL != token)
		{
			if (0 == strcmp(token, "Lu"))
				u.gen_cat = gcat_Lu;
			if (0 == strcmp(token, "Ll"))
				u.gen_cat = gcat_Ll;
			if (0 == strcmp(token, "Lt"))
				u.gen_cat = gcat_Lt;
			if (0 == strcmp(token, "Mn"))
				u.gen_cat = gcat_Mn;
			if (0 == strcmp(token, "Mc"))
				u.gen_cat = gcat_Mc;
			if (0 == strcmp(token, "Me"))
				u.gen_cat = gcat_Me;
			if (0 == strcmp(token, "Nd"))
				u.gen_cat = gcat_Nd;
			if (0 == strcmp(token, "Nl"))
				u.gen_cat = gcat_Nl;
			if (0 == strcmp(token, "No"))
				u.gen_cat = gcat_No;
			if (0 == strcmp(token, "Zs"))
				u.gen_cat = gcat_Zs;
			if (0 == strcmp(token, "Zl"))
				u.gen_cat = gcat_Zl;
			if (0 == strcmp(token, "Zp"))
				u.gen_cat = gcat_Zp;
			if (0 == strcmp(token, "Cc"))
				u.gen_cat = gcat_Cc;
			if (0 == strcmp(token, "Cf"))
				u.gen_cat = gcat_Cf;
			if (0 == strcmp(token, "Cs"))
				u.gen_cat = gcat_Cs;
			if (0 == strcmp(token, "Co"))
				u.gen_cat = gcat_Co;
			if (0 == strcmp(token, "Cn"))
				u.gen_cat = gcat_Cn;
			if (0 == strcmp(token, "Lm"))
				u.gen_cat = gcat_Lm;
			if (0 == strcmp(token, "Lo"))
				u.gen_cat = gcat_Lo;
			if (0 == strcmp(token, "Pc"))
				u.gen_cat = gcat_Pc;
			if (0 == strcmp(token, "Pd"))
				u.gen_cat = gcat_Pd;
			if (0 == strcmp(token, "Ps"))
				u.gen_cat = gcat_Ps;
			if (0 == strcmp(token, "Pe"))
				u.gen_cat = gcat_Pe;
			if (0 == strcmp(token, "Pi"))
				u.gen_cat = gcat_Pi;
			if (0 == strcmp(token, "Pf"))
				u.gen_cat = gcat_Pf;
			if (0 == strcmp(token, "Po"))
				u.gen_cat = gcat_Po;
			if (0 == strcmp(token, "Sm"))
				u.gen_cat = gcat_Sm;
			if (0 == strcmp(token, "Sc"))
				u.gen_cat = gcat_Sc;
			if (0 == strcmp(token, "Sk"))
				u.gen_cat = gcat_Sk;
			if (0 == strcmp(token, "So"))
				u.gen_cat = gcat_So;
		}
#endif

		tokennum++;
		if (NULL == (token = parse_strtok(NULL, ";\r\n")))
			fprintf(stderr, "%s: token #%d failed on line %d\n%s", __FUNCTION__, tokennum, linenum, line);
#if	NEED_UNICODE_CCOM
		if (NULL != token)
			u.canonical_comb = (UINT8)strtoul(token, NULL, 10);
#endif

		tokennum++;
		if (NULL == (token = parse_strtok(NULL, ";\r\n")))
			fprintf(stderr, "%s: token #%d failed on line %d\n%s", __FUNCTION__, tokennum, linenum, line);
#if	NEED_UNICODE_BIDI
		// parse bidirectional category
		u.bidi = bidi_0;
		if (NULL != token)
		{
			if (0 == strcmp(token, "L"))
				u.bidi = bidi_L;	// Left-to-Right
			if (0 == strcmp(token, "LRE"))
				u.bidi = bidi_LRE;	// Left-to-Right Embedding
			if (0 == strcmp(token, "LRO"))
				u.bidi = bidi_LRO;	// Left-to-Right Override
			if (0 == strcmp(token, "R"))
				u.bidi = bidi_R;	// Right-to-Left
			if (0 == strcmp(token, "AL"))
				u.bidi = bidi_AL;	// Right-to-Left Arabic
			if (0 == strcmp(token, "RLE"))
				u.bidi = bidi_RLE;	// Right-to-Left Embedding
			if (0 == strcmp(token, "RLO"))
				u.bidi = bidi_RLO;	// Right-to-Left Override
			if (0 == strcmp(token, "PDF"))
				u.bidi = bidi_PDF;	// Pop Directional Format
			if (0 == strcmp(token, "EN"))
				u.bidi = bidi_EN;	// European Number
			if (0 == strcmp(token, "ES"))
				u.bidi = bidi_ES;	// European Number Separator
			if (0 == strcmp(token, "ET"))
				u.bidi = bidi_ET;	// European Number Terminator
			if (0 == strcmp(token, "AN"))
				u.bidi = bidi_AN;	// Arabic Number
			if (0 == strcmp(token, "CS"))
				u.bidi = bidi_CS;	// Common Number Separator
			if (0 == strcmp(token, "NSM"))
				u.bidi = bidi_NSM;	// Non-Spacing Mark
			if (0 == strcmp(token, "BN"))
				u.bidi = bidi_BN;	// Boundary Neutral
			if (0 == strcmp(token, "B"))
				u.bidi = bidi_B;	// Paragraph Separator
			if (0 == strcmp(token, "S"))
				u.bidi = bidi_S;	// Segment Separator
			if (0 == strcmp(token, "WS"))
				u.bidi = bidi_WS;	// Whitespace
			if (0 == strcmp(token, "ON"))
				u.bidi = bidi_ON;	// Other Neutrals
		}
#endif

		tokennum++;
		if (NULL == (token = parse_strtok(NULL, ";\r\n")))
			fprintf(stderr, "%s: token #%d failed on line %d\n%s", __FUNCTION__, tokennum, linenum, line);
#if	NEED_UNICODE_DECO
		// parse decomposition mapping
		if (NULL != token)
		{
			unicode_char decomposed[256];
			UINT8 n = 0;
			char *p = token;

			if (0 == strncmp(token, "<font>", 6))
				u.decomp_map = deco_font;
			if (0 == strncmp(token, "<noBreak>", 9))
				u.decomp_map = deco_noBreak;
			if (0 == strncmp(token, "<initial>", 9))
				u.decomp_map = deco_initial;
			if (0 == strncmp(token, "<medial>", 8))
				u.decomp_map = deco_medial;
			if (0 == strncmp(token, "<final>", 7))
				u.decomp_map = deco_final;
			if (0 == strncmp(token, "<isolated>", 10))
				u.decomp_map = deco_initial;
			if (0 == strncmp(token, "<circle>", 8))
				u.decomp_map = deco_circle;
			if (0 == strncmp(token, "<super>", 7))
				u.decomp_map = deco_super;
			if (0 == strncmp(token, "<sub>", 5))
				u.decomp_map = deco_sub;
			if (0 == strncmp(token, "<vertical>", 10))
				u.decomp_map = deco_vertical;
			if (0 == strncmp(token, "<wide>", 6))
				u.decomp_map = deco_wide;
			if (0 == strncmp(token, "<narrow>", 8))
				u.decomp_map = deco_narrow;
			if (0 == strncmp(token, "<small>", 7))
				u.decomp_map = deco_small;
			if (0 == strncmp(token, "<square>", 8))
				u.decomp_map = deco_square;
			if (0 == strncmp(token, "<fraction>", 10))
				u.decomp_map = deco_fraction;
			if (0 == strncmp(token, "<compat>", 8))
				u.decomp_map = deco_compat;
			// skip over whitespace, if a decomposition type was found
			if (u.decomp_map != deco_0)
				while (isspace(*p))
					p++;
			// parse decomposition codes
			while (*p)
			{
				// skip initial whitespace
				while (isspace(*p))
					p++;
				// convert next code
				decomposed[n++] = (unicode_char)strtoul(p, NULL, 16);
				// skip over hex digits
				while (isxdigit(*p))
					p++;
				// break if too many codes
				if (n >= 255)
					break;
			}
			if (n > 0)
			{
				u.n_decomp = n;
				u.decomp_codes = (unicode_char*)malloc(sizeof(unicode_char) * n);
				memcpy(u.decomp_codes, decomposed, sizeof(unicode_char) * n);
			}
		}
#endif

		tokennum++;
		if (NULL == (token = parse_strtok(NULL, ";\r\n")))
			fprintf(stderr, "%s: token #%d failed on line %d\n%s", __FUNCTION__, tokennum, linenum, line);
#if	NEED_UNICODE_DECIMAL
		u.decimal_digit = UNICODE_NOT_DECIMAL;
		if (NULL != token && *token != '\0')
			u.decimal_digit = (UINT8)strtoul(token, NULL, 10);
#endif

		tokennum++;
		if (NULL == (token = parse_strtok(NULL, ";\r\n")))
			fprintf(stderr, "%s: token #%d failed on line %d\n%s", __FUNCTION__, tokennum, linenum, line);
#if	NEED_UNICODE_DIGIT
		u.digit = UNICODE_NOT_DIGIT;
		if (NULL != token && *token != '\0')
			u.digit = (UINT8)strtoul(token, NULL, 10);
#endif

		tokennum++;
		if (NULL == (token = parse_strtok(NULL, ";\r\n")))
			fprintf(stderr, "%s: token #%d failed on line %d\n%s", __FUNCTION__, tokennum, linenum, line);
#if	NEED_UNICODE_NUMERIC
		u.numeric = UNICODE_NOT_NUMERIC;
		if (NULL != token && *token)
			u.numeric = (UINT8)strtoul(token, NULL, 10);
#endif

		tokennum++;
		if (NULL == (token = parse_strtok(NULL, ";\r\n")))
			fprintf(stderr, "%s: token #%d failed on line %d\n%s", __FUNCTION__, tokennum, linenum, line);
#if	NEED_UNICODE_MIRRORED
		if (NULL != token && *token)
			u.mirrored = token[0] == 'Y';
#endif

		tokennum++;
		if (NULL == (token = parse_strtok(NULL, ";\r\n")))
			fprintf(stderr, "%s: token #%d failed on line %d\n%s", __FUNCTION__, tokennum, linenum, line);
#if	NEED_UNICODE_NAME10
		if (NULL != token && *token)
			u.name10 = strdup(token);
#endif

		tokennum++;
		if (NULL == (token = parse_strtok(NULL, ";\r\n")))
			fprintf(stderr, "%s: token #%d failed on line %d\n%s", __FUNCTION__, tokennum, linenum, line);
		if (NULL != token && *token)
		{
			/* FIXME: hmm ... don't know what this token means */
		}

		tokennum++;
		if (NULL == (token = parse_strtok(NULL, ";\r\n")))
			fprintf(stderr, "%s: token #%d failed on line %d\n%s", __FUNCTION__, tokennum, linenum, line);
#if	NEED_UNICODE_UCASE
		if (NULL != token && *token)
			u.uppercase = strtoul(token, NULL, 16);
#endif

		tokennum++;
		if (NULL == (token = parse_strtok(NULL, ";\r\n")))
			fprintf(stderr, "%s: token #%d failed on line %d\n%s", __FUNCTION__, tokennum, linenum, line);
#if	NEED_UNICODE_LCASE
		if (NULL != token && *token)
			u.lowercase = strtoul(token, NULL, 16);
#endif

		tokennum++;
		if (NULL == (token = parse_strtok(NULL, ";\r\n")))
			fprintf(stderr, "%s: token #%d failed on line %d\n%s", __FUNCTION__, tokennum, linenum, line);
#if	NEED_UNICODE_TCASE
		if (NULL != token && *token)
			u.titlecase = strtoul(token, NULL, 16);
#endif

		if (first > 0 && last > 0)
		{
			if (first + 1 >= UNICODE_PLANESIZE)
			{
				fprintf(stderr, "%s: range %#07x-%#07x outside planes\n", __FUNCTION__, first + 1, last);
			}
			else
			{
				for (code = first + 1; code <= last && code < UNICODE_PLANESIZE; code++)
					unicode_data[code] = unicode_data[first];
			}
			first = 0;
			last = 0;
			code = UNICODE_PLANESIZE;
		}
		if (code < UNICODE_PLANESIZE)
		{
			unicode_data[code] = (unicode_data_t *)malloc(sizeof(unicode_data_t));
			memcpy(unicode_data[code], &u, sizeof(u));
		}
		free(parse);
	}
	fclose(file);
	return 0;
}

void unicode_data_free()
{
	if (NULL == unicode_data)
		return;
	for (unicode_char uchar = 0; uchar < UNICODE_PLANESIZE; uchar++)
	{
		if (NULL == unicode_data[uchar])
			continue;		// undefined code
		unicode_data_t * uc = unicode_data[uchar];
		// find and zero dupes
		for (unicode_char dupes = uchar + 1; dupes  < UNICODE_PLANESIZE; dupes++)
			if (unicode_data[dupes] == uc)
				unicode_data[dupes] = NULL;
			else
				break;
#if	NEED_UNICODE_NAME
		if (uc->name)
			free(uc->name);
#endif
#if	NEED_UNICODE_NAME10
		if (uc->name10)
			free(uc->name10);
#endif
#if	NEED_UNICODE_DECN
		if (uc->decomp_codes)
			free(uc->decomp_codes);
#endif
		free(uc);
		unicode_data[uchar] = NULL;
	}
	free(unicode_data);
	unicode_data = NULL;
}
