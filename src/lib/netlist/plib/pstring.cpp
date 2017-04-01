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
int pstring_t<F>::pcmp(const pstring_t &right) const
{
	std::size_t l = std::min(size(), right.size());
	if (l == 0)
	{
		if (size() == 0 && right.size() == 0)
			return 0;
		else if (right.size() == 0)
			return 1;
		else
			return -1;
	}
	auto si = this->begin();
	auto ri = right.begin();
	while (si != this->end() && *si == *ri)
	{
		ri++;
		si++;
	}
	int ret = (si == this->end() ? 0 : static_cast<int>(*si) - static_cast<int>(*ri));
	if (ret == 0)
	{
		if (this->size() > right.size())
			ret = 1;
		else if (this->size() < right.size())
			ret = -1;
	}
	return ret;
}

template<typename F>
pstring_t<F> pstring_t<F>::substr(size_type start, size_type nlen) const
{
	pstring_t ret;
	//FIXME: throw ?
	const size_type l = len();
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
	pstring_t ret = "";
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
	mem_t buf[traits_type::MAXCODELEN+1] = { 0 };
	traits_type::encode(search, buf);
	return find(pstring_t(&buf[0], UTF8), start);
}


template<typename F>
pstring_t<F> pstring_t<F>::replace_all(const pstring_t &search, const pstring_t &replace) const
{
	pstring_t ret("");
	const size_type slen = search.len();

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
	size_type wsl = ws.len();
	for (auto i = ret.len(); i < cnt; i+=wsl)
		ret += ws;
	return ret;
}

template<typename F>
double pstring_t<F>::as_double(bool *error) const
{
	double ret;
	std::size_t e = 0;

	if (error != nullptr)
		*error = false;
	ret = std::stod(m_str, &e);
	if (e != size())
		if (error != nullptr)
			*error = true;
	return ret;
}

template<typename F>
long pstring_t<F>::as_long(bool *error) const
{
	long ret;
	std::size_t e = 0;

	if (error != nullptr)
		*error = false;
	if (startsWith("0x"))
		ret = std::stol(substr(2).m_str, &e, 16);
	else
		ret = std::stol(m_str, &e, 10);
	if (e != size())
		if (error != nullptr)
			*error = true;
	return ret;
}

// ----------------------------------------------------------------------------------------
// template stuff ...
// ----------------------------------------------------------------------------------------

template struct pstring_t<pu8_traits>;
template struct pstring_t<putf8_traits>;

const unsigned pu8_traits::MAXCODELEN;
const unsigned putf8_traits::MAXCODELEN;
