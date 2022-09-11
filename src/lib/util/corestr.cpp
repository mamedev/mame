// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    corestr.c

    Core string functions used throughout MAME.

****************************************************************************/

#include "corestr.h"

#include <algorithm>
#include <iterator>
#include <memory>

#include <cctype>
#include <cstdlib>


/*-------------------------------------------------
    core_stricmp - case-insensitive string compare
-------------------------------------------------*/

int core_stricmp(const char *s1, const char *s2)
{
	for (;;)
	{
		int c1 = tolower((uint8_t)*s1++);
		int c2 = tolower((uint8_t)*s2++);
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
		int c1 = tolower((uint8_t)*s1++);
		int c2 = tolower((uint8_t)*s2++);
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

bool core_iswildstr(const char *sp)
{
	for ( ; sp && *sp; sp++)
	{
		if (('?' == *sp) || ('*' == *sp))
			return true;
	}
	return false;
}


/*-------------------------------------------------
    std::string helpers
-------------------------------------------------*/

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

std::string_view strtrimspace(std::string_view str)
{
	std::string_view str2 = strtrimleft(str, [] (char c) { return !isspace(uint8_t(c)); });
	return strtrimright(str2, [] (char c) { return !isspace(uint8_t(c)); });
}

std::string_view strtrimrightspace(std::string_view str)
{
	return strtrimright(str, [] (char c) { return !isspace(uint8_t(c)); });
}

std::string strmakeupper(std::string_view str)
{
	std::string result;
	std::transform(str.begin(), str.end(), std::back_inserter(result), ::toupper);
	return result;
}

/**
 * @fn  std::string strmakelower(std::string_view str)
 *
 * @brief   Returns a lower case version of the given string.
 *
 * @param [in]  str The string to make lower case
 *
 * @return  A new std::string having been changed to lower case
 */

std::string strmakelower(std::string_view str)
{
	std::string result;
	std::transform(str.begin(), str.end(), std::back_inserter(result), ::tolower);
	return result;
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

namespace util {

/**
 * @fn  bool streqlower(std::string_view str, std::string_view lcstr)
 *
 * @brief   Tests whether a mixed-case string matches a lowercase string.
 *
 * @param [in]  str   First string to compare (may be mixed-case).
 * @param [in]  lcstr Second string to compare (must be all lowercase).
 *
 * @return  True if the strings match regardless of case.
 */

bool streqlower(std::string_view str, std::string_view lcstr)
{
	return std::equal(str.begin(), str.end(), lcstr.begin(), lcstr.end(),
						[] (unsigned char c1, unsigned char c2) { return std::tolower(c1) == c2; });
}

/**
 * @fn  bool strequpper(std::string_view str, std::string_view ucstr)
 *
 * @brief   Tests whether a mixed-case string matches an uppercase string.
 *
 * @param [in]  str   First string to compare (may be mixed-case).
 * @param [in]  ucstr Second string to compare (must be all uppercase).
 *
 * @return  True if the strings match regardless of case.
 */

bool strequpper(std::string_view str, std::string_view ucstr)
{
	return std::equal(str.begin(), str.end(), ucstr.begin(), ucstr.end(),
						[] (unsigned char c1, unsigned char c2) { return std::toupper(c1) == c2; });
}

/**
 * @fn  double edit_distance(std::u32string_view lhs, std::u32string_view rhs)
 *
 * @brief   Compares strings and returns prefix-weighted similarity score (smaller is more similar).
 *
 * @param   lhs         First input.
 * @param   rhs         Second input.
 *
 * @return  Similarity score ranging from 0.0 (totally dissimilar) to 1.0 (identical).
 */

double edit_distance(std::u32string_view lhs, std::u32string_view rhs)
{
	// based on Jaro-Winkler distance
	// TODO: this breaks if the lengths don't fit in a long int, but that's not a big limitation
	constexpr long MAX_PREFIX(4);
	constexpr double PREFIX_WEIGHT(0.1);
	constexpr double PREFIX_THRESHOLD(0.7);

	std::u32string_view const &longer((lhs.length() >= rhs.length()) ? lhs : rhs);
	std::u32string_view const &shorter((lhs.length() < rhs.length()) ? lhs : rhs);

	// find matches
	long const range((std::max)(long(longer.length() / 2) - 1, 0L));
	std::unique_ptr<long []> match_idx(std::make_unique<long []>(shorter.length()));
	std::unique_ptr<bool []> match_flg(std::make_unique<bool []>(longer.length()));
	std::fill_n(match_idx.get(), shorter.length(), -1);
	std::fill_n(match_flg.get(), longer.length(), false);
	long match_cnt(0);
	for (long i = 0; shorter.length() > i; ++i)
	{
		char32_t const ch(shorter[i]);
		long const n((std::min)(i + range + 1L, long(longer.length())));
		for (long j = (std::max)(i - range, 0L); n > j; ++j)
		{
			if (!match_flg[j] && (ch == longer[j]))
			{
				match_idx[i] = j;
				match_flg[j] = true;
				++match_cnt;
				break;
			}
		}
	}

	// early exit if strings are very dissimilar
	if (!match_cnt)
		return 1.0;

	// now find transpositions
	std::unique_ptr<char32_t []> ms(std::make_unique<char32_t []>(2 * match_cnt));
	std::fill_n(ms.get(), 2 * match_cnt, char32_t(0));
	char32_t *const ms1(&ms[0]);
	char32_t *const ms2(&ms[match_cnt]);
	for (long i = 0, j = 0; shorter.length() > i; ++i)
	{
		if (0 <= match_idx[i])
			ms1[j++] = shorter[i];
	}
	match_idx.reset();
	for (long i = 0, j = 0; longer.length() > i; ++i)
	{
		if (match_flg[i])
			ms2[j++] = longer[i];
	}
	match_flg.reset();
	long halftrans_cnt(0);
	for (long i = 0; match_cnt > i; ++i)
	{
		if (ms1[i] != ms2[i])
			++halftrans_cnt;
	}
	ms.reset();

	// simple prefix detection
	long prefix_len(0);
	for (long i = 0; ((std::min)(long(shorter.length()), MAX_PREFIX) > i) && (lhs[i] == rhs[i]); ++i)
		++prefix_len;

	// do the weighting
	double const m(match_cnt);
	double const t(double(halftrans_cnt) / 2);
	double const jaro(((m / lhs.length()) + (m / rhs.length()) + ((m - t) / m)) / 3);
	double const jaro_winkler((PREFIX_THRESHOLD > jaro) ? jaro : (jaro + (PREFIX_WEIGHT * prefix_len * (1.0 - jaro))));
	return 1.0 - jaro_winkler;
}

} // namespace util
