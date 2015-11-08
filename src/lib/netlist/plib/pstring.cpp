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

#include "pstring.h"
#include "palloc.h"
#include "plists.h"

template<>
pstr_t pstring_t<putf8_traits>::m_zero = pstr_t(0);
template<>
pstr_t pstring_t<pu8_traits>::m_zero = pstr_t(0);


/*
 * Uncomment the following to override defaults
 */

#define IMMEDIATE_MODE  (1)
//#define DEBUG_MODE      (0)

#ifdef MAME_DEBUG
	#ifndef IMMEDIATE_MODE
		#define IMMEDIATE_MODE  (1)
	#endif
	#ifndef DEBUG_MODE
		#define DEBUG_MODE      (0)
	#endif
#else
	#ifndef IMMEDIATE_MODE
		#define IMMEDIATE_MODE  (1)
	#endif
	#ifndef DEBUG_MODE
		#define DEBUG_MODE      (0)
	#endif
#endif

template<typename F>
pstring_t<F>::~pstring_t()
{
	sfree(m_ptr);
}

template<typename F>
void pstring_t<F>::pcat(const mem_t *s)
{
	int slen = strlen(s);
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
	int slen = s.blen();
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
	long l = std::min(blen(), right.blen());
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
		ret = this->blen() - right.blen();
	if (ret < 0)
		return -1;
	else if (ret > 0)
		return 1;
	else
		return 0;
}


template<typename F>
void pstring_t<F>::pcopy(const mem_t *from, int size)
{
	pstr_t *n = salloc(size);
	if (size > 0)
		std::memcpy(n->str(), from, size);
	*(n->str() + size) = 0;
	sfree(m_ptr);
	m_ptr = n;
}

template<typename F>
const pstring_t<F> pstring_t<F>::substr(int start, int count) const
{
	pstring_t ret;
	int alen = (int) len();
	if (start < 0)
		start = 0;
	if (start >= alen)
		return ret;
	if (count <0 || start + count > alen)
		count = alen - start;
	const char *p = cstr();
	if (count <= 0)
		ret.pcopy(p, 0);
	else
	{
		//FIXME: Trait to tell which one
		//ret.pcopy(cstr() + start, count);
		// find start
		for (int i=0; i<start; i++)
			p += F::codelen(p);
		const char *e = p;
		for (int i=0; i<count; i++)
			e += F::codelen(e);
		ret.pcopy(p, e-p);
	}
	return ret;
}

template<typename F>
const pstring_t<F> pstring_t<F>::ucase() const
{
	pstring_t ret = *this;
	ret.pcopy(cstr(), blen());
	for (unsigned i=0; i<ret.len(); i++)
		ret.m_ptr->str()[i] = toupper((unsigned) ret.m_ptr->str()[i]);
	return ret;
}

template<typename F>
int pstring_t<F>::find_first_not_of(const pstring_t &no) const
{
	char *t = m_ptr->str();
	unsigned nolen = no.len();
	unsigned tlen = len();
	for (unsigned i=0; i < tlen; i++)
	{
		char *n = no.m_ptr->str();
		bool f = true;
		for (unsigned j=0; j < nolen; j++)
		{
			if (F::code(t) == F::code(n))
				f = false;
			n += F::codelen(t);
		}
		if (f)
			return i;
		t += F::codelen(t);
	}
	return -1;
}

template<typename F>
int pstring_t<F>::find_last_not_of(const pstring_t &no) const
{
	char *t = m_ptr->str();
	unsigned nolen = no.len();
	unsigned tlen = len();
	int last_found = -1;
	for (unsigned i=0; i < tlen; i++)
	{
		char *n = no.m_ptr->str();
		bool f = true;
		for (unsigned j=0; j < nolen; j++)
		{
			if (F::code(t) == F::code(n))
				f = false;
			n += F::codelen(t);
		}
		if (f)
			last_found = i;
		t += F::codelen(t);
	}
	return last_found;
}

template<typename F>
pstring_t<F> pstring_t<F>::replace(const pstring_t &search, const pstring_t &replace) const
{
	// FIXME: use this pstringbuffer ret = "";
	pstring_t ret = "";
	const int slen = search.blen();
	const int tlen = blen();

	if (slen == 0 || tlen < slen )
		return *this;
	int i = 0;
	while (i < tlen - slen + 1)
	{
		if (memcmp(cstr()+i,search.cstr(),slen) == 0)
		{
			ret += replace;
			i += slen;
		}
		else
		{
			/* avoid adding a code, cat a string ... */
			mem_t buf[2] = { *(cstr() + i), 0 };
			ret = ret.cat(buf);
			i++;
		}
	}
	ret = ret.cat(cstr() + i);
	return ret;
}

template<typename F>
const pstring_t<F> pstring_t<F>::ltrim(const pstring_t &ws) const
{
	int f = find_first_not_of(ws);
	if (f>=0)
		return substr(f);
	else
		return "";
}

template<typename F>
const pstring_t<F> pstring_t<F>::rtrim(const pstring_t &ws) const
{
	int f = find_last_not_of(ws);
	if (f>=0)
		return left(f+1);
	else
		return "";
}

template<typename F>
const pstring_t<F> pstring_t<F>::rpad(const pstring_t &ws, const unsigned cnt) const
{
	// FIXME: pstringbuffer ret(*this);

	pstring_t ret(*this);
	while (ret.len() < cnt)
		ret += ws;
	return pstring_t(ret).substr(0, cnt);
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
	char *e = NULL;

	if (error != NULL)
		*error = false;
	ret = strtod(cstr(), &e);
	if (*e != 0)
		if (error != NULL)
			*error = true;
	return ret;
}

template<typename F>
long pstring_t<F>::as_long(bool *error) const
{
	long ret;
	char *e = NULL;

	if (error != NULL)
		*error = false;
	if (startsWith("0x"))
		ret = strtol(substr(2).cstr(), &e, 16);
	else
		ret = strtol(cstr(), &e, 10);
	if (*e != 0)
		if (error != NULL)
			*error = true;
	return ret;
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

static pstack_t<pstr_t *> *stk = NULL;

static inline unsigned countleadbits(unsigned x)
{
#ifndef count_leading_zeros
	unsigned msk;
	unsigned ret;
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
		if (stk != NULL)
		{
			unsigned sn= ((32 - countleadbits(s->len())) + 1) / 2;
			stk[sn].push(s);
		}
		else
			pfree_array(((char *)s));
		//_mm_free(((char *)s));
	}
}

template<typename F>
pstr_t *pstring_t<F>::salloc(int n)
{
	if (stk == NULL)
		stk = palloc_array(pstack_t<pstr_t *>, 17);
	pstr_t *p;
	unsigned sn= ((32 - countleadbits(n)) + 1) / 2;
	unsigned size = sizeof(pstr_t) + ((UINT64) 1<<(sn * 2)) + 1;
	if (stk[sn].empty())
		p = (pstr_t *) palloc_array(char, size);
	else
	{
		p = stk[sn].pop();
	}

	//  str_t *p = (str_t *) _mm_malloc(size, 8);
	p->init(n);
	return p;
}
template<typename F>
void pstring_t<F>::resetmem()
{
	if (stk != NULL)
	{
		for (unsigned i=0; i<=16; i++)
		{
			for (; stk[i].count() > 0; )
				pfree_array(stk[i].pop());
		}
		pfree_array(stk);
		stk = NULL;
	}
}


#else
template<typename F>
void pstring_t<F>::sfree(pstr_t *s)
{
	s->m_ref_count--;
	if (s->m_ref_count == 0 && s != &m_zero)
	{
		pfree_array(((char *)s));
		//_mm_free(((char *)s));
	}
}

template<typename F>
pstr_t *pstring_t<F>::salloc(int n)
{
	int size = sizeof(pstr_t) + n + 1;
	pstr_t *p = (pstr_t *) palloc_array(char, size);
	//  str_t *p = (str_t *) _mm_malloc(size, 8);
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
// pstring ...
// ----------------------------------------------------------------------------------------

template<typename F>
int pstring_t<F>::find(const pstring_t &search, unsigned start) const
{
	const unsigned tlen = len();
	const unsigned slen = search.len();
	const char *s = search.cstr();
	const unsigned startt = std::min(start, tlen);
	const char *t = cstr();
	for (unsigned i=0; i<startt; i++)
		t += F::codelen(t);
	for (int i=0; i <= (int) tlen - (int) startt - (int) slen; i++)
	{
		if (F::code(t) == F::code(s))
			if (std::memcmp(t,s,search.blen())==0)
				return i+startt;
		t += F::codelen(t);
	}
	return -1;
}

template<typename F>
int pstring_t<F>::find(const mem_t *search, unsigned start) const
{
	const unsigned tlen = len();
	unsigned slen = 0;
	unsigned sblen = 0;
	const mem_t *x = search;
	while (*x != 0)
	{
		slen++;
		const unsigned sl = F::codelen(x);
		x += sl;
		sblen += sl;
	}
	const char *s = search;
	const unsigned startt = std::min(start, tlen);
	const char *t = cstr();
	for (unsigned i=0; i<startt; i++)
		t += F::codelen(t);
	for (int i=0; i <= (int) tlen - (int) startt - (int) slen; i++)
	{
		if (F::code(t) == F::code(s))
			if (std::memcmp(t,s,sblen)==0)
				return i+startt;
		t += F::codelen(t);
	}
	return -1;
}

template<typename F>
bool pstring_t<F>::startsWith(const pstring_t &arg) const
{
	if (arg.blen() > blen())
		return false;
	else
		return (memcmp(arg.cstr(), cstr(), arg.len()) == 0);
}

template<typename F>
bool pstring_t<F>::endsWith(const pstring_t &arg) const
{
	if (arg.blen() > blen())
		return false;
	else
		return (memcmp(cstr()+this->len()-arg.len(), arg.cstr(), arg.len()) == 0);
}


template<typename F>
bool pstring_t<F>::startsWith(const mem_t *arg) const
{
	unsigned alen = strlen(arg);
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
	if (m_ptr != NULL)
		pfree_array(m_ptr);
}

void pstringbuffer::resize(const std::size_t size)
{
	if (m_ptr == NULL)
	{
		m_size = DEFAULT_SIZE;
		while (m_size <= size)
			m_size *= 2;
		m_ptr = palloc_array(char, m_size);
		*m_ptr = 0;
		m_len = 0;
	}
	else if (m_size < size)
	{
		while (m_size < size)
			m_size *= 2;
		char *new_buf = palloc_array(char, m_size);
		std::memcpy(new_buf, m_ptr, m_len + 1);
		pfree_array(m_ptr);
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

void pstringbuffer::pcat(const void *m, unsigned l)
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

template struct pstring_t<pu8_traits>;
template struct pstring_t<putf8_traits>;
