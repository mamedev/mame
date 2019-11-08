// license:GPL-2.0+
// copyright-holders:Couriersud

#include "pstring.h"
#include "palloc.h"
#include "plists.h"

#include <algorithm>
#include <atomic>
#include <stack>

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
		++ri;
		++si;
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

// ----------------------------------------------------------------------------------------
// template stuff ...
// ----------------------------------------------------------------------------------------

template struct pstring_t<pu8_traits>;
template struct pstring_t<putf8_traits>;
template struct pstring_t<putf16_traits>;
template struct pstring_t<pwchar_traits>;
