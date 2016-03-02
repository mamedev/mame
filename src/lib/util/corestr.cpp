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
	char *cpy = nullptr;
	if (str != nullptr)
	{
		cpy = (char *)osd_malloc_array(strlen(str) + 1);
		if (cpy != nullptr)
			strcpy(cpy, str);
	}
	return cpy;
}


/*-------------------------------------------------
    std::string helpers
-------------------------------------------------*/

#include <algorithm>

int strcatvprintf(std::string &str, const char *format, va_list args)
{
	char tempbuf[4096];
	int result = vsprintf(tempbuf, format, args);

	// set the result
	str.append(tempbuf);
	return result;
}

void strdelchr(std::string& str, char chr)
{
	for (size_t i = 0; i < str.length(); i++)
	{
		if (str[i] == chr)
		{
			str.erase(i, 1);
			i--;
		}
	}
}

void strreplacechr(std::string& str, char ch, char newch)
{
	for (auto & elem : str)
	{
		if (elem == ch) elem = newch;
	}
}

std::string strtrimspace(std::string& str)
{
	int start = 0;
	for (auto & elem : str)
	{
		if (!isspace(UINT8(elem)))  break;
		start++;
	}
	int end = str.length();
	if (end > 0)
	{
		for (size_t i = str.length() - 1; i > 0; i--)
		{
			if (!isspace(UINT8(str[i]))) break;
			end--;
		}
	}
	str = str.substr(start, end-start);
	return str;
}

std::string strmakeupper(std::string& str)
{
	std::transform(str.begin(), str.end(), str.begin(), ::toupper);
	return str;
}

/**
 * @fn  std::string strmakelower(std::string& str)
 *
 * @brief   Strmakelowers the given string.
 *
 * @param [in,out]  str The string.
 *
 * @return  A std::string.
 */

std::string strmakelower(std::string& str)
{
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
	return str;
}

/**
 * @fn  int strreplace(std::string &str, const std::string& search, const std::string& replace)
 *
 * @brief   Strreplaces.
 *
 * @param [in,out]  str The string.
 * @param   search      The search.
 * @param   replace     The replace.
 *
 * @return  An int.
 */

int strreplace(std::string &str, const std::string& search, const std::string& replace)
{
	int searchlen = search.length();
	int replacelen = replace.length();
	int matches = 0;

	for (int curindex = str.find(search, 0); curindex != -1; curindex = str.find(search, curindex + replacelen))
	{
		matches++;
		str.erase(curindex, searchlen).insert(curindex, replace);
	}
	return matches;
}
