/***************************************************************************

    astring.c

    Allocated string manipulation functions.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************/

#include "astring.h"
#include "osdcore.h"

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct _astring
{
	char *			text;
	int				alloclen;
	char			smallbuf[256 - sizeof(int) - sizeof(char *)];
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static char dummy_text[2] = { 0 };
static astring dummy_astring = { dummy_text, 1, { 0 } };



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    ensure_room - ensure we have room for a
    given string, or else reallocate our buffer
-------------------------------------------------*/

INLINE int ensure_room(astring *str, int length)
{
	char *newbuf, *oldbuf;
	int alloclen;

	/* always fail to expand the dummy */
	if (str == &dummy_astring)
		return FALSE;

	/* if we have the room, do nothing */
	if (str->alloclen >= length + 1)
		return TRUE;

	/* allocate a new buffer with some slop */
	alloclen = length + 256;
	newbuf = malloc(alloclen);
	if (newbuf == NULL)
		return FALSE;

	/* swap in the new buffer and free the old one */
	oldbuf = (str->text == str->smallbuf) ? NULL : str->text;
	str->text = strcpy(newbuf, str->text);
	str->alloclen = alloclen;
	if (oldbuf != NULL)
		free(oldbuf);

	return TRUE;
}


/*-------------------------------------------------
    safe_string_base - return a "safe" string
    base for a given start index
-------------------------------------------------*/

INLINE char *safe_string_base(char *base, int start)
{
	int max = strlen(base);
	return (start >= 0 && start < max) ? base + start : base + max;
}


/*-------------------------------------------------
    normalize_substr - normalize substr parameters
-------------------------------------------------*/

INLINE void normalize_substr(int *start, int *count, int length)
{
	/* limit start */
	if (*start < 0)
		*start = 0;
	else if (*start > length)
		*start = length;

	/* update count */
	if (*count == -1 || *start + *count > length)
		*count = length - *start;
}



/***************************************************************************
    ASTRING ALLOCATION
***************************************************************************/

/*-------------------------------------------------
    astring_alloc - allocate a new astring
-------------------------------------------------*/

astring *astring_alloc(void)
{
	astring *str;

	/* allocate memory; if we fail, return the dummy */
	str = malloc(sizeof(*str));
	if (str == NULL)
		return &dummy_astring;
	memset(str, 0, sizeof(*str));

	/*  initialize the small buffer */
	str->text = str->smallbuf;
	str->alloclen = ARRAY_LENGTH(str->smallbuf);
	return str;
}


/*-------------------------------------------------
    astring_free - free an astring
-------------------------------------------------*/

void astring_free(astring *str)
{
	/* ignore attempts to free the dummy */
	if (str == &dummy_astring)
		return;

	/* if we allocated additional memory, free that */
	if (str->text != str->smallbuf)
		free(str->text);
	free(str);
}



/***************************************************************************
    INLINE ASTRING CHANGES
***************************************************************************/

/*-------------------------------------------------
    astring_cpy - copy one astring into another
-------------------------------------------------*/

astring *astring_cpy(astring *dst, const astring *src)
{
	return astring_cpyc(dst, src->text);
}


/*-------------------------------------------------
    astring_cpyc - copy a C string into an astring
-------------------------------------------------*/

astring *astring_cpyc(astring *dst, const char *src)
{
	return astring_cpych(dst, src, strlen(src));
}


/*-------------------------------------------------
    astring_cpych - copy a character array into
    an astring
-------------------------------------------------*/

astring *astring_cpych(astring *dst, const char *src, int count)
{
	/* make room; if we fail or if dst is the dummy, do nothing */
	if (!ensure_room(dst, count))
		return dst;

	/* copy the raw data and NULL-terminate */
	if (count > 0)
		memcpy(dst->text, src, count);
	dst->text[count] = 0;
	return dst;
}


/*-------------------------------------------------
    astring_cpysubstr - copy a substring of one
    string to another
-------------------------------------------------*/

astring *astring_cpysubstr(astring *dst, const astring *src, int start, int count)
{
	normalize_substr(&start, &count, strlen(src->text));
	return astring_cpych(dst, src->text + start, count);
}


/*-------------------------------------------------
    astring_ins - insert one astring into another
-------------------------------------------------*/

astring *astring_ins(astring *dst, int insbefore, const astring *src)
{
	return astring_insc(dst, insbefore, src->text);
}


/*-------------------------------------------------
    astring_insc - insert a C string into an
    astring
-------------------------------------------------*/

astring *astring_insc(astring *dst, int insbefore, const char *src)
{
	return astring_insch(dst, insbefore, src, strlen(src));
}


/*-------------------------------------------------
    astring_insch - insert a character array
    into an astring
-------------------------------------------------*/

astring *astring_insch(astring *dst, int insbefore, const char *src, int count)
{
	int dstlength = strlen(dst->text);

	/* make room; if we fail or if dst is the dummy, do nothing */
	if (!ensure_room(dst, dstlength + count))
		return dst;

	/* adjust insbefore to be logical */
	if (insbefore < 0 || insbefore > dstlength)
		insbefore = dstlength;

	/* copy the data an NULL-terminate */
	if (insbefore < dstlength)
		memmove(dst->text + insbefore + count, dst->text + insbefore, dstlength - insbefore);
	memcpy(dst->text + insbefore, src, count);
	dst->text[dstlength + count] = 0;
	return dst;
}


/*-------------------------------------------------
    astring_inssubstr - insert a substring of
    one string to another
-------------------------------------------------*/

astring *astring_inssubstr(astring *dst, int insbefore, const astring *src, int start, int count)
{
	normalize_substr(&start, &count, strlen(src->text));
	return astring_insch(dst, insbefore, src->text + start, count);
}


/*-------------------------------------------------
    astring_substr - extract a substring of
    ourself, removing everything else
-------------------------------------------------*/

astring *astring_substr(astring *str, int start, int count)
{
	/* ignore attempts to do this on the dummy */
	if (str == &dummy_astring)
		return str;

	/* normalize parameters */
	normalize_substr(&start, &count, strlen(str->text));

	/* move the data and NULL-terminate */
	if (count > 0 && start > 0)
		memmove(str->text, str->text + start, count);
	str->text[count] = 0;
	return str;
}


/*-------------------------------------------------
    astring_printf - printf text into an astring
-------------------------------------------------*/

int astring_printf(astring *dst, const char *format, ...)
{
	char tempbuf[4096];
	va_list args;
	int result;

	/* sprintf into the temporary buffer */
	va_start(args, format);
	result = vsprintf(tempbuf, format, args);
	va_end(args);

	/* set the result */
	astring_cpyc(dst, tempbuf);
	return result;
}



/***************************************************************************
    ASTRING QUERIES
***************************************************************************/

/*-------------------------------------------------
    astring_c - return a pointer to a C string
    from an astring
-------------------------------------------------*/

const char *astring_c(const astring *str)
{
	return str->text;
}


/*-------------------------------------------------
    astring_len - return the length of an astring
-------------------------------------------------*/

int astring_len(const astring *str)
{
	return strlen(str->text);
}


/*-------------------------------------------------
    astring_cmp - compare one astring to another
-------------------------------------------------*/

int astring_cmp(const astring *str1, const astring *str2)
{
	return astring_cmpc(str1, str2->text);
}


/*-------------------------------------------------
    astring_cmpc - compare a C string to an astring
-------------------------------------------------*/

int astring_cmpc(const astring *str1, const char *str2)
{
	const char *s1 = str1->text;

	/* loop while equal until we hit the end of strings */
	while (*s1 != 0 && *str2 != 0 && *s1 == *str2)
		s1++, str2++;
	return *s1 - *str2;
}


/*-------------------------------------------------
    astring_cmpch - compare a character array to
    an astring
-------------------------------------------------*/

int astring_cmpch(const astring *str1, const char *str2, int count)
{
	const char *s1 = str1->text;
	int result;

	/* loop while equal until we hit the end of strings */
	while (count-- > 0 && *s1 != 0 && *str2 != 0 && *s1 == *str2)
		s1++, str2++;
	result = (count == -1) ? 0 : *s1 - *str2;
	if (result == 0 && *s1 != 0)
		result = 1;
	return result;
}


/*-------------------------------------------------
    astring_cmpsubstr - compare a substring to
    an astring
-------------------------------------------------*/

int astring_cmpsubstr(const astring *str1, const astring *str2, int start, int count)
{
	normalize_substr(&start, &count, strlen(str2->text));
	return astring_cmpch(str1, str2->text + start, count);
}


/*-------------------------------------------------
    astring_icmp - case-insenstive compare one
    astring to another
-------------------------------------------------*/

int astring_icmp(const astring *str1, const astring *str2)
{
	return astring_icmpc(str1, str2->text);
}


/*-------------------------------------------------
    astring_icmpc - case-insenstive compare a C
    string to an astring
-------------------------------------------------*/

int astring_icmpc(const astring *str1, const char *str2)
{
	const char *s1 = str1->text;

	/* loop while equal until we hit the end of strings */
	while (*s1 != 0 && *str2 != 0 && tolower(*s1) == tolower(*str2))
		s1++, str2++;
	return tolower(*s1) - tolower(*str2);
}


/*-------------------------------------------------
    astring_icmpch - case-insenstive compare a
    character array to an astring
-------------------------------------------------*/

int astring_icmpch(const astring *str1, const char *str2, int count)
{
	const char *s1 = str1->text;
	int result;

	/* loop while equal until we hit the end of strings */
	while (count-- > 0 && *s1 != 0 && *str2 != 0 && tolower(*s1) == tolower(*str2))
		s1++, str2++;
	result = (count == -1) ? 0 : tolower(*s1) - tolower(*str2);
	if (result == 0 && *s1 != 0)
		result = 1;
	return result;
}


/*-------------------------------------------------
    astring_icmpsubstr - case-insenstive compare a
    substring to an astring
-------------------------------------------------*/

int astring_icmpsubstr(const astring *str1, const astring *str2, int start, int count)
{
	normalize_substr(&start, &count, strlen(str2->text));
	return astring_icmpch(str1, str2->text + start, count);
}


/*-------------------------------------------------
    astring_chr - return the index of a character
    in an astring
-------------------------------------------------*/

int astring_chr(const astring *str, int start, int ch)
{
	char *result = strchr(safe_string_base(str->text, start), ch);
	return (result != NULL) ? (result - str->text) : -1;
}


/*-------------------------------------------------
    astring_rchr - return the index of a character
    in an astring, searching from the end
-------------------------------------------------*/

int astring_rchr(const astring *str, int start, int ch)
{
	char *result = strrchr(safe_string_base(str->text, start), ch);
	return (result != NULL) ? (result - str->text) : -1;
}


/*-------------------------------------------------
    astring_find - find one astring in another
-------------------------------------------------*/

int astring_find(const astring *str, int start, const astring *search)
{
	return astring_findc(str, start, search->text);
}


/*-------------------------------------------------
    astring_findc - find a C string in an astring
-------------------------------------------------*/

int astring_findc(const astring *str, int start, const char *search)
{
	char *result = strstr(safe_string_base(str->text, start), search);
	return (result != NULL) ? (result - str->text) : -1;
}



/***************************************************************************
    ASTRING UTILITIES
***************************************************************************/

/*-------------------------------------------------
    astring_delchr - delete all instances of
    'ch'
-------------------------------------------------*/

astring *astring_delchr(astring *str, int ch)
{
	char *src, *dst;

	/* simple deletion */
	for (src = dst = str->text; *src != 0; src++)
		if (*src != ch)
			*dst++ = *src;
	*dst = 0;

	return str;
}


/*-------------------------------------------------
    astring_replacechr - replace all instances of
    'ch' with 'newch'
-------------------------------------------------*/

astring *astring_replacechr(astring *str, int ch, int newch)
{
	char *text;

	/* simple replacement */
	for (text = str->text; *text != 0; text++)
		if (*text == ch)
			*text = newch;

	return str;
}


/*-------------------------------------------------
    astring_toupper - convert string to all
    upper-case
-------------------------------------------------*/

astring *astring_toupper(astring *str)
{
	char *text;

	/* just toupper() on all characters */
	for (text = str->text; *text != 0; text++)
		*text = toupper(*text);

	return str;
}


/*-------------------------------------------------
    astring_tolower - convert string to all
    lower-case
-------------------------------------------------*/

astring *astring_tolower(astring *str)
{
	char *text;

	/* just toupper() on all characters */
	for (text = str->text; *text != 0; text++)
		*text = tolower(*text);

	return str;
}


/*-------------------------------------------------
    astring_trimspace - remove all space
    characters from beginning/end
-------------------------------------------------*/

astring *astring_trimspace(astring *str)
{
	char *ptr;

	/* first remove stuff from the end */
	for (ptr = str->text + strlen(str->text) - 1; ptr >= str->text && isspace(*ptr); ptr--)
		*ptr = 0;

	/* then count how much to remove from the beginning */
	for (ptr = str->text; *ptr != 0 && isspace(*ptr); ptr++) ;
	if (ptr > str->text)
		astring_substr(str, ptr - str->text, -1);

	return str;
}
