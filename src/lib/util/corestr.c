/***************************************************************************

    corestr.c

    Core string functions used throughout MAME.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

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
	int i, l1, l2;
	char *p;

	strncpy(s1, sp1, 16); s1[16] = 0; if (s1[0] == 0) strcpy(s1, "*");

	strncpy(s2, sp2, 16); s2[16] = 0; if (s2[0] == 0) strcpy(s2, "*");

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

	l1 = (int)strlen(s1);
	if (l1 < 16)
	{
		for (i = l1 + 1; i < 16; i++) s1[i] = ' ';
		s1[16] = 0;
	}

	l2 = (int)strlen(s2);
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
		cpy = (char *)osd_malloc(strlen(str) + 1);
		if (cpy != NULL)
			strcpy(cpy, str);
	}
	return cpy;
}


/*-------------------------------------------------
    core_i64_format - i64 format printf helper
-------------------------------------------------*/

char *core_i64_hex_format(UINT64 value, UINT8 mindigits)
{
	static char buffer[16][64];
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
