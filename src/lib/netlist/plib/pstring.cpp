// license:BSD-3-Clause
// copyright-holders:Couriersud

#include "pstring.h"
#include "palloc.h"

#include <algorithm>
#include <atomic>
#include <stack>

template<typename F>
int pstring_t<F>::compare(const pstring_t &right) const noexcept
{
#if 0
	return m_str.compare(right.m_str);
#else
	auto si = this->begin();
	auto ri = right.begin();
	const auto se = this->end();
	const auto re = right.end();

	while (si != se && ri != re && *si == *ri)
	{
		++ri;
		++si;
	}

	if (si != se && ri != re)
		return plib::narrow_cast<int>(*si) - plib::narrow_cast<int>(*ri);
	if (si != se)
		return 1;
	if (ri != re)
		return -1;
	return 0;
#endif
}

template<typename F>
pstring_t<F> pstring_t<F>::substr(size_type start, size_type nlen) const
{
	pstring_t ret;
	auto ps = begin();
	while (ps != end() && start > 0)
	{
		++ps;
		--start;
	}
	//FIXME: throw ?
	if (ps != end())
	{
		auto pe = ps;
		while (pe != end() && nlen > 0)
		{
			++pe;
			--nlen;
		}
		ret.m_str.assign(ps.p, pe.p);
	}
	return ret;
}

template<typename F>
pstring_t<F> pstring_t<F>::substr(size_type start) const
{
	pstring_t ret;
	auto ps = begin();
	while (ps != end() && start > 0)
	{
		++ps;
		--start;
	}
	//FIXME: throw ?
	if (ps != end())
	{
		ret.m_str.assign(ps.p, end().p);
	}
	return ret;
}

template<typename F>
typename pstring_t<F>::size_type pstring_t<F>::find(const pstring_t &search, size_type start) const noexcept
{
	auto istart = std::next(begin(), static_cast<difference_type>(start));
	for (; istart != end(); ++istart)
	{
		auto itc = istart;
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
typename pstring_t<F>::size_type pstring_t<F>::find(code_t search, size_type start) const noexcept
{
	auto i = std::next(begin(), static_cast<difference_type>(start));
	for (; i != end(); ++i)
	{
		if (*i == search)
			return start;
		++start;
	}
	return npos;
}

// ----------------------------------------------------------------------------------------
// template stuff ...
// ----------------------------------------------------------------------------------------

template struct pstring_t<pu8_traits>;
template struct pstring_t<putf8_traits>;
template struct pstring_t<putf16_traits>;
template struct pstring_t<putf32_traits>;
template struct pstring_t<pwchar_traits>;
