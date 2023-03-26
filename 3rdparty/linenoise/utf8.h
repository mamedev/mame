#ifndef UTF8_UTIL_H
#define UTF8_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * UTF-8 utility functions
 *
 * (c) 2010-2019 Steve Bennett <steveb@workware.net.au>
 *
 * See utf8.c for licence details.
 */

#ifndef USE_UTF8
#include <ctype.h>

#define MAX_UTF8_LEN 1

/* No utf-8 support. 1 byte = 1 char */
#define utf8_strlen(S, B) ((B) < 0 ? (int)strlen(S) : (B))
#define utf8_strwidth(S, B) utf8_strlen((S), (B))
#define utf8_tounicode(S, CP) (*(CP) = (unsigned char)*(S), 1)
#define utf8_index(C, I) (I)
#define utf8_charlen(C) 1
 #define utf8_width(C) 1

#else

#define MAX_UTF8_LEN 4

/**
 * Converts the given unicode codepoint (0 - 0x1fffff) to utf-8
 * and stores the result at 'p'.
 *
 * Returns the number of utf-8 characters
 */
int utf8_fromunicode(char *p, unsigned uc);

/**
 * Returns the length of the utf-8 sequence starting with 'c'.
 *
 * Returns 1-4, or -1 if this is not a valid start byte.
 *
 * Note that charlen=4 is not supported by the rest of the API.
 */
int utf8_charlen(int c);

/**
 * Returns the number of characters in the utf-8
 * string of the given byte length.
 *
 * Any bytes which are not part of an valid utf-8
 * sequence are treated as individual characters.
 *
 * The string *must* be null terminated.
 *
 * Does not support unicode code points > \u1fffff
 */
int utf8_strlen(const char *str, int bytelen);

/**
 * Calculates the display width of the first 'charlen' characters in 'str'.
 * See utf8_width()
 */
int utf8_strwidth(const char *str, int charlen);

/**
 * Returns the byte index of the given character in the utf-8 string.
 *
 * The string *must* be null terminated.
 *
 * This will return the byte length of a utf-8 string
 * if given the char length.
 */
int utf8_index(const char *str, int charindex);

/**
 * Returns the unicode codepoint corresponding to the
 * utf-8 sequence 'str'.
 *
 * Stores the result in *uc and returns the number of bytes
 * consumed.
 *
 * If 'str' is null terminated, then an invalid utf-8 sequence
 * at the end of the string will be returned as individual bytes.
 *
 * If it is not null terminated, the length *must* be checked first.
 *
 * Does not support unicode code points > \u1fffff
 */
int utf8_tounicode(const char *str, int *uc);

/**
 * Returns the width (in characters) of the given unicode codepoint.
 * This is 1 for normal letters and 0 for combining characters and 2 for wide characters.
 */
int utf8_width(int ch);

#endif

#ifdef __cplusplus
}
#endif

#endif
