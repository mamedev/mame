// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nl_string.c
 *
 */

#include <cstring>
//FIXME:: pstring should be locale free
#include <cctype>
#include <cstdlib>
#include <cstdio>
#include <algorithm>
#include <stack>

#include "pstring.h"
#include "palloc.h"
#include "plists.h"

template<>
pstr_t pstring_t<putf8_traits>::m_zero = pstr_t(0);
template<>
pstr_t pstring_t<pu8_traits>::m_zero = pstr_t(0);

template<typename F>
pstring_t<F>::~pstring_t()
{
	if (m_ptr != nullptr)
		sfree(m_ptr);
}

template<typename F>
void pstring_t<F>::pcat(const mem_t *s)
{
	std::size_t slen = strlen(s);
	pstr_t *n = salloc(m_ptr->len() + slen);
	if (m_ptr->len() > 0)
		std::memcpy(n->str(), m_ptr->str(), m_ptr->len());
	if (slen > 0)
		std::memcpy(n->str() + m_ptr->len(), s, slen);
	*(n->str() + n->len()) = 0;
	sfree(m_ptr);
	m_ptr = n;
}

template<typename F>
void pstring_t<F>::pcat(const pstring_t &s)
{
	std::size_t slen = s.blen();
	pstr_t *n = salloc(m_ptr->len() + slen);
	if (m_ptr->len() > 0)
		std::memcpy(n->str(), m_ptr->str(), m_ptr->len());
	if (slen > 0)
		std::memcpy(n->str() + m_ptr->len(), s.cstr(), slen);
	*(n->str() + n->len()) = 0;
	sfree(m_ptr);
	m_ptr = n;
}

template<typename F>
int pstring_t<F>::pcmp(const pstring_t &right) const
{
	std::size_t l = std::min(blen(), right.blen());
	if (l == 0)
	{
		if (blen() == 0 && right.blen() == 0)
			return 0;
		else if (right.blen() == 0)
			return 1;
		else
			return -1;
	}
	int ret = memcmp(m_ptr->str(), right.cstr(), l);
	if (ret == 0)
	{
		if (this->blen() > right.blen())
			ret = 1;
		else if (this->blen() < right.blen())
			ret = -1;
	}
	return ret;
}


template<typename F>
void pstring_t<F>::pcopy(const mem_t *from, std::size_t size)
{
	pstr_t *n = salloc(size);
	if (size > 0)
		std::memcpy(n->str(), from, size);
	*(n->str() + size) = 0;
	sfree(m_ptr);
	m_ptr = n;
}

template<typename F>
const pstring_t<F> pstring_t<F>::substr(const iterator start, const iterator end) const
{
	pstring_t ret;
	//FIXME: throw ?
	ret.pcopy(start.p, static_cast<std::size_t>(end.p - start.p));
	return ret;
}


template<typename F>
const pstring_t<F> pstring_t<F>::ucase() const
{
	pstring_t ret = *this;
	ret.pcopy(cstr(), blen());
	for (std::size_t  i=0; i<ret.len(); i++)
		ret.m_ptr->str()[i] = static_cast<char>(toupper(static_cast<int>(ret.m_ptr->str()[i])));
	return ret;
}

template<typename F>
typename pstring_t<F>::iterator pstring_t<F>::find_first_not_of(const pstring_t &no) const
{
	for (auto it = begin(); it != end(); ++it)
	{
		bool f = true;
		for (auto const jt : no)
		{
			if (*it == jt)
			{
				f = false;
				break;
			}
		}
		if (f)
			return it;
	}
	return end();
}

template<typename F>
typename pstring_t<F>::iterator pstring_t<F>::find_last_not_of(const pstring_t &no) const
{
	/* FIXME: reverse iterator */
	iterator last_found = end();
	for (auto it = begin(); it != end(); ++it)
	{
		bool f = true;
		for (auto const jt : no)
		{
			if (*it == jt)
			{
				f = false;
				break;
			}
		}
		if (f)
			last_found = it;
	}
	return last_found;
}

template<typename F>
typename pstring_t<F>::iterator pstring_t<F>::find(const pstring_t &search, iterator start) const
{
	for (; start != end(); ++start)
	{
		iterator itc(start);
		auto cmp = search.begin();
		while (itc != end() && cmp != search.end() && *itc == *cmp)
		{
			++itc;
			++cmp;
		}
		if (cmp == search.end())
			return start;
	}
	return end();
}

template<typename F>
pstring_t<F> pstring_t<F>::replace(const pstring_t &search, const pstring_t &replace) const
{
	pstring_t ret("");
	const size_type slen = search.len();

	auto last_s = begin();
	auto s = find(search, last_s);
	while (s != end())
	{
		ret += substr(last_s, s);
		ret += replace;
		last_s = s + slen;
		s = find(search, last_s);
	}
	ret += substr(last_s, end());
	return ret;
}

template<typename F>
const pstring_t<F> pstring_t<F>::ltrim(const pstring_t &ws) const
{
	return substr(find_first_not_of(ws), end());
}

template<typename F>
const pstring_t<F> pstring_t<F>::rtrim(const pstring_t &ws) const
{
	auto f = find_last_not_of(ws);
	if (f==end())
		return pstring_t("");
	else
		return substr(begin(), f + 1);
}

template<typename F>
const pstring_t<F> pstring_t<F>::rpad(const pstring_t &ws, const size_type cnt) const
{
	// FIXME: pstringbuffer ret(*this);

	pstring_t ret(*this);
	size_type wsl = ws.len();
	for (auto i = ret.len(); i < cnt; i+=wsl)
		ret += ws;
	return ret;
}


template<typename F>
void pstring_t<F>::pcopy(const mem_t *from)
{
	pcopy(from, strlen(from));
}

template<typename F>
double pstring_t<F>::as_double(bool *error) const
{
	double ret;
	char *e = nullptr;

	if (error != nullptr)
		*error = false;
	ret = strtod(cstr(), &e);
	if (*e != 0)
		if (error != nullptr)
			*error = true;
	return ret;
}

template<typename F>
long pstring_t<F>::as_long(bool *error) const
{
	long ret;
	char *e = nullptr;

	if (error != nullptr)
		*error = false;
	if (startsWith("0x"))
		ret = strtol(substr(2).cstr(), &e, 16);
	else
		ret = strtol(cstr(), &e, 10);
	if (*e != 0)
		if (error != nullptr)
			*error = true;
	return ret;
}

template<typename F>
typename pstring_t<F>::iterator pstring_t<F>::find(const mem_t *search, iterator start) const
{
	for (; start != end(); ++start)
	{
		iterator itc(start);
		iterator cmp(search);
		while (itc != end() && *cmp != 0 && *itc == *cmp)
		{
			++itc;
			++cmp;
		}
		if (*cmp == 0)
			return start;
	}
	return end();
}

template<typename F>
bool pstring_t<F>::startsWith(const pstring_t &arg) const
{
	if (arg.blen() > blen())
		return false;
	else
		return (memcmp(arg.cstr(), cstr(), arg.blen()) == 0);
}

template<typename F>
bool pstring_t<F>::endsWith(const pstring_t &arg) const
{
	if (arg.blen() > blen())
		return false;
	else
		return (memcmp(cstr()+this->blen()-arg.blen(), arg.cstr(), arg.blen()) == 0);
}


template<typename F>
bool pstring_t<F>::startsWith(const mem_t *arg) const
{
	std::size_t alen = strlen(arg);
	if (alen > blen())
		return false;
	else
		return (memcmp(arg, cstr(), alen) == 0);
}

template<typename F>
int pstring_t<F>::pcmp(const mem_t *right) const
{
	return std::strcmp(m_ptr->str(), right);
}

// ----------------------------------------------------------------------------------------
// pstringbuffer
// ----------------------------------------------------------------------------------------

pstringbuffer::~pstringbuffer()
{
	if (m_ptr != nullptr)
		plib::pfree_array(m_ptr);
}

void pstringbuffer::resize(const std::size_t size)
{
	if (m_ptr == nullptr)
	{
		m_size = DEFAULT_SIZE;
		while (m_size <= size)
			m_size *= 2;
		m_ptr = plib::palloc_array<char>(m_size);
		*m_ptr = 0;
		m_len = 0;
	}
	else if (m_size < size)
	{
		while (m_size < size)
			m_size *= 2;
		char *new_buf = plib::palloc_array<char>(m_size);
		std::memcpy(new_buf, m_ptr, m_len + 1);
		plib::pfree_array(m_ptr);
		m_ptr = new_buf;
	}
}

void pstringbuffer::pcopy(const char *from)
{
	std::size_t nl = strlen(from) + 1;
	resize(nl);
	std::memcpy(m_ptr, from, nl);
}

void pstringbuffer::pcopy(const pstring &from)
{
	std::size_t nl = from.blen() + 1;
	resize(nl);
	std::memcpy(m_ptr, from.cstr(), nl);
}

void pstringbuffer::pcat(const char *s)
{
	const std::size_t slen = std::strlen(s);
	const std::size_t nl = m_len + slen + 1;
	resize(nl);
	std::memcpy(m_ptr + m_len, s, slen + 1);
	m_len += slen;
}

void pstringbuffer::pcat(const void *m, std::size_t l)
{
	const std::size_t nl = m_len + l + 1;
	resize(nl);
	std::memcpy(m_ptr + m_len, m, l);
	m_len += l;
	*(m_ptr + m_len) = 0;
}

void pstringbuffer::pcat(const pstring &s)
{
	const std::size_t slen = s.blen();
	const std::size_t nl = m_len + slen + 1;
	resize(nl);
	std::memcpy(m_ptr + m_len, s.cstr(), slen);
	m_len += slen;
	m_ptr[m_len] = 0;
}

// ----------------------------------------------------------------------------------------
// static stuff ...
// ----------------------------------------------------------------------------------------

/*
 * Cached allocation of string memory
 *
 * This improves startup performance by 30%.
 */

#if 1

static std::stack<pstr_t *> *stk = nullptr;

static inline std::size_t countleadbits(std::size_t x)
{
#ifndef count_leading_zeros
	std::size_t msk;
	std::size_t ret;
	if (x < 0x100)
	{
		msk = 0x80;
		ret = 24;
	}
	else if (x < 0x10000)
	{
		msk = 0x8000;
		ret = 16;
	}
	else if (x < 0x1000000)
	{
		msk = 0x800000;
		ret = 8;
	}
	else
	{
		msk = 0x80000000;
		ret = 0;
	}
	while ((msk & x) == 0 && ret < 31)
	{
		msk = msk >> 1;
		ret++;
	}
	return ret;
#else
	return count_leading_zeros(x);
#endif
}

template<typename F>
void pstring_t<F>::sfree(pstr_t *s)
{
	s->m_ref_count--;
	if (s->m_ref_count == 0 && s != &m_zero)
	{
		if (stk != nullptr)
		{
			size_type sn= ((32 - countleadbits(s->len())) + 1) / 2;
			stk[sn].push(s);
		}
		else
			plib::pfree_array(reinterpret_cast<char *>(s));
		//_mm_free(((char *)s));
	}
}

template<typename F>
pstr_t *pstring_t<F>::salloc(std::size_t n)
{
	if (stk == nullptr)
		stk = plib::palloc_array<std::stack<pstr_t *>>(17);
	pstr_t *p;
	std::size_t sn= ((32 - countleadbits(n)) + 1) / 2;
	std::size_t size = sizeof(pstr_t) + (static_cast<std::size_t>(1)<<(sn * 2)) + 1;
	if (stk[sn].empty())
		p = reinterpret_cast<pstr_t *>(plib::palloc_array<char>(size));
	else
	{
		p = stk[sn].top();
		stk[sn].pop();
	}

	//  str_t *p = (str_t *) mm_malloc(size, 8);
	p->init(n);
	return p;
}
template<typename F>
void pstring_t<F>::resetmem()
{
	if (stk != nullptr)
	{
		for (std::size_t  i=0; i<=16; i++)
		{
			for (; stk[i].size() > 0; )
			{
				plib::pfree_array(stk[i].top());
				stk[i].pop();
			}
		}
		plib::pfree_array(stk);
		stk = nullptr;
	}
}


#else
template<typename F>
void pstring_t<F>::sfree(pstr_t *s)
{
	s->m_ref_count--;
	if (s->m_ref_count == 0 && s != &m_zero)
	{
		plib::pfree_array(((char *)s));
		//_mm_free(((char *)s));
	}
}

template<typename F>
pstr_t *pstring_t<F>::salloc(int n)
{
	int size = sizeof(pstr_t) + n + 1;
	pstr_t *p = (pstr_t *) plib::palloc_array<char>(size);
	//  str_t *p = (str_t *) mm_malloc(size, 8);
	p->init(n);
	return p;
}

template<typename F>
void pstring_t<F>::resetmem()
{
	// Release the 0 string
}
#endif

// ----------------------------------------------------------------------------------------
// template stuff ...
// ----------------------------------------------------------------------------------------

template struct pstring_t<pu8_traits>;
template struct pstring_t<putf8_traits>;
