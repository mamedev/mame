// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    corestr.c

    Core string functions used throughout MAME.

****************************************************************************/

#include "corestr.h"
#include "osdcore.h"
#include <ctype.h>
#include <stdlib.h>


/*-------------------------------------------------
    core_stricmp - case-insensitive string compare
-------------------------------------------------*/

int core_stricmp(const char *s1, const char *s2)
{
	for (;;)
	{
		int c1 = tolower((UINT8)*s1++);
		int c2 = tolower((UINT8)*s2++);
		if (c1 == 0 || c1 != c2)
			return c1 - c2;
	}
}


/*-------------------------------------------------
    core_strnicmp - case-insensitive string compare
-------------------------------------------------*/

int core_strnicmp(const char *s1, const char *s2, size_t n)
{
	size_t i;
	for (i = 0; i < n; i++)
	{
		int c1 = tolower((UINT8)*s1++);
		int c2 = tolower((UINT8)*s2++);
		if (c1 == 0 || c1 != c2)
			return c1 - c2;
	}

	return 0;
}


/*-------------------------------------------------
    core_strwildcmp - case-insensitive wildcard
    string compare (up to 16 characters at the
    moment)
-------------------------------------------------*/

int core_strwildcmp(const char *sp1, const char *sp2)
{
	char s1[17], s2[17];
	size_t i, l1, l2;
	char *p;

	//assert(strlen(sp1) < 16);
	//assert(strlen(sp2) < 16);

	if (sp1[0] == 0) strcpy(s1, "*");
	else { strncpy(s1, sp1, 16); s1[16] = 0; }

	if (sp2[0] == 0) strcpy(s2, "*");
	else { strncpy(s2, sp2, 16); s2[16] = 0; }

	p = strchr(s1, '*');
	if (p)
	{
		for (i = p - s1; i < 16; i++) s1[i] = '?';
		s1[16] = 0;
	}

	p = strchr(s2, '*');
	if (p)
	{
		for (i = p - s2; i < 16; i++) s2[i] = '?';
		s2[16] = 0;
	}

	l1 = strlen(s1);
	if (l1 < 16)
	{
		for (i = l1 + 1; i < 16; i++) s1[i] = ' ';
		s1[16] = 0;
	}

	l2 = strlen(s2);
	if (l2 < 16)
	{
		for (i = l2 + 1; i < 16; i++) s2[i] = ' ';
		s2[16] = 0;
	}

	for (i = 0; i < 16; i++)
	{
		if (s1[i] == '?' && s2[i] != '?') s1[i] = s2[i];
		if (s2[i] == '?' && s1[i] != '?') s2[i] = s1[i];
	}

	return core_stricmp(s1, s2);
}


/*-------------------------------------------------
    core_strdup - string duplication via osd_malloc
-------------------------------------------------*/

char *core_strdup(const char *str)
{
	char *cpy = NULL;
	if (str != NULL)
	{
		cpy = (char *)osd_malloc_array(strlen(str) + 1);
		if (cpy != NULL)
			strcpy(cpy, str);
	}
	return cpy;
}


/*-------------------------------------------------
    core_i64_hex_format - i64 format printf helper
-------------------------------------------------*/

char *core_i64_hex_format(UINT64 value, UINT8 mindigits)
{
	static char buffer[16][64];
	// TODO: this can overflow - e.g. when a lot of unmapped writes are logged
	static int index;
	char *bufbase = &buffer[index++ % 16][0];
	char *bufptr = bufbase;
	INT8 curdigit;

	for (curdigit = 15; curdigit >= 0; curdigit--)
	{
		int nibble = (value >> (curdigit * 4)) & 0xf;
		if (nibble != 0 || curdigit < mindigits)
		{
			mindigits = curdigit;
			*bufptr++ = "0123456789ABCDEF"[nibble];
		}
	}
	if (bufptr == bufbase)
		*bufptr++ = '0';
	*bufptr = 0;

	return bufbase;
}

/*-------------------------------------------------
    core_i64_oct_format - i64 format printf helper
-------------------------------------------------*/

char *core_i64_oct_format(UINT64 value, UINT8 mindigits)
{
	static char buffer[22][64];
	// TODO: this can overflow
	static int index;
	char *bufbase = &buffer[index++ % 22][0];
	char *bufptr = bufbase;
	INT8 curdigit;

	for (curdigit = 21; curdigit >= 0; curdigit--)
	{
		int octdigit = (value >> (curdigit * 3)) & 0x7;
		if (octdigit != 0 || curdigit < mindigits)
		{
			mindigits = curdigit;
			*bufptr++ = "01234567"[octdigit];
		}
	}
	if (bufptr == bufbase)
		*bufptr++ = '0';
	*bufptr = 0;

	return bufbase;
}

/*-------------------------------------------------
    core_i64_format - i64 format printf helper
-------------------------------------------------*/

char *core_i64_format(UINT64 value, UINT8 mindigits, bool is_octal)
{
	return is_octal ? core_i64_oct_format(value,mindigits) : core_i64_hex_format(value,mindigits);
}
