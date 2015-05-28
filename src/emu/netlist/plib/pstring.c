// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nl_string.c
 *
 */

#include <new>
#include <cstdio>
#include <cstring>
//FIXME:: pstring should be locale free
#include <cctype>
#include <cstdlib>

#include "pstring.h"
#include "palloc.h"

// The following will work on linux, however not on Windows ....

//pblockpool *pstring::m_pool = new pblockpool;
//pstring::str_t *pstring::m_zero = new(pstring::m_pool, 0) pstring::str_t(0);

pstring::str_t pstring::m_zero = str_t(0);

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

pstring::~pstring()
{
	sfree(m_ptr);
}

void pstring::pcat(const char *s)
{
	int slen = strlen(s);
	str_t *n = salloc(m_ptr->len() + slen);
	if (m_ptr->len() > 0)
		std::memcpy(n->str(), m_ptr->str(), m_ptr->len());
	if (slen > 0)
		std::memcpy(n->str() + m_ptr->len(), s, slen);
	*(n->str() + n->len()) = 0;
	sfree(m_ptr);
	m_ptr = n;
}

void pstring::pcopy(const char *from, int size)
{
	str_t *n = salloc(size);
	if (size > 0)
		std::memcpy(n->str(), from, size);
	*(n->str() + size) = 0;
	sfree(m_ptr);
	m_ptr = n;
}

pstring pstring::substr(unsigned int start, int count) const
{
	pstring ret;
	unsigned alen = len();
	if (start >= alen)
		return ret;
	if (count <0 || start + count > alen)
		count = alen - start;
	ret.pcopy(cstr() + start, count);
	return ret;
}

pstring pstring::ucase() const
{
	pstring ret = *this;
	ret.pcopy(cstr(), len());
	for (int i=0; i<ret.len(); i++)
		ret.m_ptr->str()[i] = toupper((unsigned) ret.m_ptr->str()[i]);
	return ret;
}

int pstring::find_first_not_of(const pstring no) const
{
	for (int i=0; i < len(); i++)
	{
		bool f = true;
		for (int j=0; j < no.len(); j++)
			if (m_ptr->str()[i] == no.m_ptr->str()[j])
				f = false;
		if (f)
			return i;
	}
	return -1;
}

int pstring::find_last_not_of(const pstring no) const
{
	for (int i=len() - 1; i >= 0; i--)
	{
		bool f = true;
		for (int j=0; j < no.len(); j++)
			if (m_ptr->str()[i] == no.m_ptr->str()[j])
				f = false;
		if (f)
			return i;
	}
	return -1;
}

pstring pstring::replace(const pstring &search, const pstring &replace) const
{
	pstring ret = "";

	if (search.len() == 0)
		return *this;
	int i = 0;
	while (i<len())
	{
		if (strncmp(cstr()+i,search.cstr(),search.len()) == 0)
		{
			ret += replace;
			i += search.len();
		}
		else
		{
			ret += *(cstr() + i);
			i++;
		}
	}
	return ret;
}
pstring pstring::ltrim(const pstring ws) const
{
	int f = find_first_not_of(ws);
	if (f>=0)
		return substr(f);
	else
		return "";
}

pstring pstring::rtrim(const pstring ws) const
{
	int f = find_last_not_of(ws);
	if (f>=0)
		return left(f+1);
	else
		return "";
}

void pstring::pcopy(const char *from)
{
	pcopy(from, strlen(from));
}

//-------------------------------------------------
//  pcmpi - compare a character array to an nstring
//-------------------------------------------------

int pstring::pcmpi(const char *lhs, const char *rhs, int count) const
{
	// loop while equal until we hit the end of strings
	int index;
	for (index = 0; index < count; index++)
		if (lhs[index] == 0 || std::tolower(lhs[index]) != std::tolower(rhs[index]))
			break;

	// determine the final result
	if (index < count)
		return std::tolower(lhs[index]) - std::tolower(rhs[index]);
	if (lhs[index] == 0)
		return 0;
	return 1;
}

double pstring::as_double(bool *error) const
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

long pstring::as_long(bool *error) const
{
	long ret;
	char *e = NULL;

	if (error != NULL)
		*error = false;
	if (startsWith("0x"))
		ret = strtol(&(cstr()[2]), &e, 16);
	else
		ret = strtol(cstr(), &e, 10);
	if (*e != 0)
		if (error != NULL)
			*error = true;
	return ret;
}

pstring pstring::vprintf(va_list args) const
{
	// sprintf into the temporary buffer
	char tempbuf[4096];
	std::vsprintf(tempbuf, cstr(), args);

	return pstring(tempbuf);
}

// ----------------------------------------------------------------------------------------
// static stuff ...
// ----------------------------------------------------------------------------------------

void pstring::sfree(str_t *s)
{
	s->m_ref_count--;
	if (s->m_ref_count == 0 && s != &m_zero)
	{
		pfree_array(((char *)s));
		//_mm_free(((char *)s));
	}
}

pstring::str_t *pstring::salloc(int n)
{
	int size = sizeof(str_t) + n + 1;
	str_t *p = (str_t *) palloc_array(char, size);
	//  str_t *p = (str_t *) _mm_malloc(size, 8);
	p->init(n);
	return p;
}

void pstring::resetmem()
{
	// Release the 0 string
}

// ----------------------------------------------------------------------------------------
// pstring ...
// ----------------------------------------------------------------------------------------

pstring pstring::sprintf(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	pstring ret = pstring(format).vprintf(ap);
	va_end(ap);
	return ret;
}


int pstring::find(const char *search, int start) const
{
	int alen = len();
	const char *result = std::strstr(cstr() + std::min(start, alen), search);
	return (result != NULL) ? (result - cstr()) : -1;
}

int pstring::find(const char search, int start) const
{
	int alen = len();
	const char *result = std::strchr(cstr() + std::min(start, alen), search);
	return (result != NULL) ? (result - cstr()) : -1;
}

bool pstring::startsWith(const char *arg) const
{
	return (pcmp(cstr(), arg, std::strlen(arg)) == 0);
}

int pstring::pcmp(const char *left, const char *right, int count) const
{
	if (count < 0)
		return std::strcmp(left, right);
	else
		return std::strncmp(left, right, count);
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
		m_len = 0;
	}
	else if (m_size < size)
	{
		while (m_size < size)
			m_size *= 2;
		char *new_buf = palloc_array(char, m_size);
		std::strncpy(new_buf, m_ptr, m_len + 1);
		pfree_array(m_ptr);
		m_ptr = new_buf;
	}
}

void pstringbuffer::pcopy(const char *from)
{
	std::size_t nl = strlen(from) + 1;
	resize(nl);
	std::strncpy(m_ptr, from, nl);
}

void pstringbuffer::pcopy(const pstring &from)
{
	std::size_t nl = from.len() + 1;
	resize(nl);
	std::strncpy(m_ptr, from.cstr(), nl);
}

void pstringbuffer::pcat(const char *s)
{
	std::size_t slen = std::strlen(s);
	std::size_t nl = m_len + slen + 1;
	resize(nl);
	std::strncpy(m_ptr + m_len, s, slen + 1);
	m_len += slen;
}

void pstringbuffer::pcat(const pstring &s)
{
	std::size_t slen = s.len();
	std::size_t nl = m_len + slen + 1;
	resize(nl);
	std::strncpy(m_ptr + m_len, s.cstr(), slen + 1);
	m_len += slen;
}
