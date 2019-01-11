// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nl_string.c
 *
 */

#include "pstring.h"
#include "palloc.h"
#include "plists.h"

#include <algorithm>
#include <stack>
#include <atomic>

template <typename T>
std::size_t strlen_mem(const T *s)
{
	std::size_t len(0);
	while (*s++)
		++len;
	return len;
}

template<typename F>
int pstring_t<F>::compare(const pstring_t &right) const
{
	if (mem_t_size() == 0 && right.mem_t_size() == 0)
		return 0;
	else if (right.mem_t_size() == 0)
		return 1;
	else if (mem_t_size() == 0)
		return -1;

	auto si = this->begin();
	auto ri = right.begin();
	while (si != this->end() && ri != right.end() && *si == *ri)
	{
		ri++;
		si++;
	}

	if (si != this->end() && ri != right.end())
		return static_cast<int>(*si) - static_cast<int>(*ri);
	else if (this->mem_t_size() > right.mem_t_size())
		return 1;
	else if (this->mem_t_size() < right.mem_t_size())
		return -1;
	return 0;
}

template<typename F>
pstring_t<F> pstring_t<F>::substr(size_type start, size_type nlen) const
{
	pstring_t ret;
	//FIXME: throw ?
	const size_type l = length();
	if (start < l)
	{
		if (nlen == npos || start + nlen > l)
			nlen = l - start;
		auto ps = std::next(begin(), static_cast<difference_type>(start));
		auto pe = std::next(ps, static_cast<difference_type>(nlen));
		ret.m_str.assign(ps.p, pe.p);
	}
	return ret;
}

template<typename F>
pstring_t<F> pstring_t<F>::ucase() const
{
	pstring_t ret;
	for (const auto &c : *this)
		if (c >= 'a' && c <= 'z')
			ret += (c - 'a' + 'A');
		else
			ret += c;
	return ret;
}

template<typename F>
typename pstring_t<F>::size_type pstring_t<F>::find_first_not_of(const pstring_t &no) const
{
	size_type pos = 0;
	for (auto it = begin(); it != end(); ++it, ++pos)
	{
		bool f = true;
		for (code_t const jt : no)
		{
			if (*it == jt)
			{
				f = false;
				break;
			}
		}
		if (f)
			return pos;
	}
	return npos;
}

template<typename F>
typename pstring_t<F>::size_type pstring_t<F>::find_last_not_of(const pstring_t &no) const
{
	/* FIXME: reverse iterator */
	size_type last_found = npos;
	size_type pos = 0;
	for (auto it = begin(); it != end(); ++it, ++pos)
	{
		bool f = true;
		for (code_t const jt : no)
		{
			if (*it == jt)
			{
				f = false;
				break;
			}
		}
		if (f)
			last_found = pos;
	}
	return last_found;
}

template<typename F>
typename pstring_t<F>::size_type pstring_t<F>::find(const pstring_t &search, size_type start) const
{
	auto istart = std::next(begin(), static_cast<difference_type>(start));
	for (; istart != end(); ++istart)
	{
		auto itc(istart);
		auto cmp = search.begin();
		while (itc != end() && cmp != search.end() && *itc == *cmp)
		{
			++itc;
			++cmp;
		}
		if (cmp == search.end())
			return start;
		++start;
	}
	return npos;
}

template<typename F>
typename pstring_t<F>::size_type pstring_t<F>::find(code_t search, size_type start) const
{
	pstring_t ss;
	traits_type::encode(search, ss.m_str);
	return find(ss, start);
}


template<typename F>
pstring_t<F> pstring_t<F>::replace_all(const pstring_t &search, const pstring_t &replace) const
{
	pstring_t ret;
	const size_type slen = search.length();

	size_type last_s = 0;
	size_type s = find(search, last_s);
	while (s != npos)
	{
		ret += substr(last_s, s - last_s);
		ret += replace;
		last_s = s + slen;
		s = find(search, last_s);
	}
	ret += substr(last_s);
	return ret;
}

template<typename F>
pstring_t<F> pstring_t<F>::rpad(const pstring_t &ws, const size_type cnt) const
{
	// FIXME: pstringbuffer ret(*this);

	pstring_t ret(*this);
	size_type wsl = ws.length();
	for (auto i = ret.length(); i < cnt; i+=wsl)
		ret += ws;
	return ret;
}

static double pstod(const pstring_t<pu8_traits> &str, std::size_t *e)
{
	return std::stod(str.cpp_string(), e);
}

static double pstod(const pstring &str, std::size_t *e)
{
	return std::stod(str.cpp_string(), e);
}

static double pstod(const pwstring &str, std::size_t *e)
{
	return std::stod(str.cpp_string(), e);
}

static double pstod(const pu16string &str, std::size_t *e)
{
	pstring c;
	c = str;
	return std::stod(c.cpp_string(), e);
}

static long pstol(const pstring_t<pu8_traits> &str, std::size_t *e, int base = 10)
{
	return std::stol(str.cpp_string(), e, base);
}

static long pstol(const pstring &str, std::size_t *e, int base = 10)
{
	return std::stol(str.cpp_string(), e, base);
}

static long pstol(const pwstring &str, std::size_t *e, int base = 10)
{
	return std::stol(str.cpp_string(), e, base);
}

static long pstol(const pu16string &str, std::size_t *e, int base = 10)
{
	pstring c;
	c = str;
	return std::stol(c.cpp_string(), e, base);
}

template<typename F>
double pstring_t<F>::as_double(bool *error) const
{
	std::size_t e = 0;
	if (error != nullptr)
		*error = false;
	double ret = pstod(*this, &e);
	if (e != mem_t_size())
		if (error != nullptr)
			*error = true;
	return ret;
}

template<typename F>
long pstring_t<F>::as_long(bool *error) const
{
	static pstring_t prefix(pstring("0x"));
	long ret;
	std::size_t e = 0;

	if (error != nullptr)
		*error = false;
	if (startsWith(prefix))
		ret = pstol(substr(2), &e, 16);
	else
		ret = pstol(*this, &e, 10);
	if (e != mem_t_size())
		if (error != nullptr)
			*error = true;
	return ret;
}

// ----------------------------------------------------------------------------------------
// template stuff ...
// ----------------------------------------------------------------------------------------

template struct pstring_t<pu8_traits>;
template struct pstring_t<putf8_traits>;
template struct pstring_t<putf16_traits>;
template struct pstring_t<pwchar_traits>;
