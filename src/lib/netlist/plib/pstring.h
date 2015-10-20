// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * pstring.h
 */

#ifndef _PSTRING_H_
#define _PSTRING_H_

#include <cstdarg>
#include <cstddef>

#include "pconfig.h"

// ----------------------------------------------------------------------------------------
// pstring: immutable strings ...
//
// pstrings are just a pointer to a "pascal-style" string representation.
// It uses reference counts and only uses new memory when a string changes.
// ----------------------------------------------------------------------------------------

	struct pstr_t
	{
		//str_t() : m_ref_count(1), m_len(0) { m_str[0] = 0; }
		pstr_t(const unsigned alen)
		{
			init(alen);
		}
		void init(const unsigned alen)
		{
				m_ref_count = 1;
				m_len = alen;
				m_str[0] = 0;
		}
		char *str() { return &m_str[0]; }
		unsigned len() const  { return m_len; }
		int m_ref_count;
	private:
		unsigned m_len;
		char m_str[1];
	};


template <typename F>
struct pstring_t
{
public:
	typedef F traits;

	typedef typename traits::mem_t mem_t;
	typedef typename traits::code_t code_t;

	// simple construction/destruction
	pstring_t()
	{
		init();
	}
	~pstring_t();

	// construction with copy
	pstring_t(const mem_t *string) {init(); if (string != NULL && *string != 0) pcopy(string); }
	pstring_t(const pstring_t &string) {init(); pcopy(string); }

	// assignment operators
	pstring_t &operator=(const mem_t *string) { pcopy(string); return *this; }
	pstring_t &operator=(const pstring_t &string) { pcopy(string); return *this; }

	// C string conversion helpers
	const mem_t *cstr() const { return m_ptr->str(); }

	// concatenation operators
	pstring_t& operator+=(const pstring_t &string) { pcat(string); return *this; }
	pstring_t& operator+=(const mem_t *string) { pcat(string); return *this; }
	friend pstring_t operator+(const pstring_t &lhs, const pstring_t &rhs) { return pstring_t(lhs) += rhs; }
	friend pstring_t operator+(const pstring_t &lhs, const mem_t *rhs) { return pstring_t(lhs) += rhs; }
	friend pstring_t operator+(const mem_t *lhs, const pstring_t &rhs) { return pstring_t(lhs) += rhs; }

	// comparison operators
	bool operator==(const mem_t *string) const { return (pcmp(string) == 0); }
	bool operator==(const pstring_t &string) const { return (pcmp(string) == 0); }
	bool operator!=(const mem_t *string) const { return (pcmp(string) != 0); }
	bool operator!=(const pstring_t &string) const { return (pcmp(string) != 0); }

	bool operator<(const mem_t *string) const { return (pcmp(string) < 0); }
	bool operator<(const pstring_t &string) const { return (pcmp(string) < 0); }
	bool operator<=(const mem_t *string) const { return (pcmp(string) <= 0); }
	bool operator<=(const pstring_t &string) const { return (pcmp(string) <= 0); }
	bool operator>(const mem_t *string) const { return (pcmp(string) > 0); }
	bool operator>(const pstring_t &string) const { return (pcmp(string) > 0); }
	bool operator>=(const mem_t *string) const { return (pcmp(string) >= 0); }
	bool operator>=(const pstring_t &string) const { return (pcmp(string) >= 0); }

	bool equals(const pstring_t &string) const { return (pcmp(string) == 0); }

	int cmp(const pstring_t &string) const { return pcmp(string); }
	int cmp(const mem_t *string) const { return pcmp(string); }

	bool startsWith(const pstring_t &arg) const;
	bool startsWith(const mem_t *arg) const;

	bool endsWith(const pstring_t &arg) const;
	bool endsWith(const mem_t *arg) const { return endsWith(pstring_t(arg)); }

	pstring_t replace(const pstring_t &search, const pstring_t &replace) const;

	const pstring_t cat(const pstring_t &s) const { return *this + s; }
	const pstring_t cat(const mem_t *s) const { return *this + s; }

	unsigned blen() const { return m_ptr->len(); }

	// conversions

	double as_double(bool *error = NULL) const;
	long as_long(bool *error = NULL) const;

	/*
	 * everything below MAY not work for utf8.
	 * Example a=s.find(EUROSIGN); b=s.substr(a,1); will deliver invalid utf8
	 */

	unsigned len() const
	{
		return F::len(m_ptr);
	}

	pstring_t& operator+=(const code_t c) { mem_t buf[F::MAXCODELEN+1] = { 0 }; F::encode(c, buf); pcat(buf); return *this; }
	friend pstring_t operator+(const pstring_t &lhs, const mem_t rhs) { return pstring_t(lhs) += rhs; }

	int find(const pstring_t &search, unsigned start = 0) const;
	int find(const mem_t *search, unsigned start = 0) const;
	int find(const code_t search, unsigned start = 0) const { mem_t buf[F::MAXCODELEN+1] = { 0 }; F::encode(search, buf); return find(buf, start); };

	const pstring_t substr(int start, int count = -1) const ;

	const pstring_t left(unsigned count) const { return substr(0, count); }
	const pstring_t right(unsigned count) const  { return substr((int) len() - (int) count, count); }

	int find_first_not_of(const pstring_t &no) const;
	int find_last_not_of(const pstring_t &no) const;

	// FIXME:
	code_t code_at(const unsigned pos) const { return F::code(F::nthcode(m_ptr->str(),pos)); }

	const pstring_t ltrim(const pstring_t &ws = " \t\n\r") const;
	const pstring_t rtrim(const pstring_t &ws = " \t\n\r") const;
	const pstring_t trim(const pstring_t &ws = " \t\n\r") const { return this->ltrim(ws).rtrim(ws); }

	const pstring_t rpad(const pstring_t &ws, const unsigned cnt) const;

	const pstring_t ucase() const;

	static void resetmem();

protected:

	pstr_t *m_ptr;

private:
	void init()
	{
		m_ptr = &m_zero;
		m_ptr->m_ref_count++;
	}

	int pcmp(const pstring_t &right) const;

	int pcmp(const mem_t *right) const;

	void pcopy(const mem_t *from, int size);

	void pcopy(const mem_t *from);

	void pcopy(const pstring_t &from)
	{
		sfree(m_ptr);
		m_ptr = from.m_ptr;
		m_ptr->m_ref_count++;
	}

	void pcat(const mem_t *s);
	void pcat(const pstring_t &s);

	static pstr_t *salloc(int n);
	static void sfree(pstr_t *s);

	static pstr_t m_zero;
};

struct pu8_traits
{
	static const unsigned MAXCODELEN = 1; /* in memory units */
	typedef char mem_t;
	typedef char code_t;
	static unsigned len(const pstr_t *p) { return p->len(); }
	static unsigned codelen(const mem_t *p) { return 1; }
	static unsigned codelen(const code_t c) { return 1; }
	static code_t code(const mem_t *p) { return *p; }
	static void encode(const code_t c, mem_t *p) { *p = c; }
	static const mem_t *nthcode(const mem_t *p, const unsigned n) { return &(p[n]); }
};

/* No checking, this may deliver invalid codes */
struct putf8_traits
{
	static const unsigned MAXCODELEN = 4; /* in memory units,  RFC 3629 */
	typedef char mem_t;
	typedef unsigned code_t;
	static unsigned len(pstr_t *p)
	{
		unsigned ret = 0;
		unsigned char *c = (unsigned char *) p->str();
		while (*c)
		{
			if (!((*c & 0xC0) == 0x80))
				ret++;
			c++;
		}
		return ret;
	}
	static unsigned codelen(const mem_t *p)
	{
		unsigned char *p1 = (unsigned char *) p;
		if ((*p1 & 0x80) == 0x00)
			return 1;
		else if ((*p1 & 0xE0) == 0xC0)
			return 2;
		else if ((*p1 & 0xF0) == 0xE0)
			return 3;
		else if ((*p1 & 0xF8) == 0xF0)
			return 4;
		else
		{
			return 1; // not correct
		}
	}
	static unsigned codelen(const code_t c)
	{
		if (c < 0x0080)
			return 1;
		else if (c < 0x800)
			return 2;
		else if (c < 0x10000)
			return 3;
		else /* U+10000 U+1FFFFF */
			return 4; /* no checks */
	}
	static code_t code(const mem_t *p)
	{
		unsigned char *p1 = (unsigned char *)p;
		if ((*p1 & 0x80) == 0x00)
			return (code_t) *p1;
		else if ((*p1 & 0xE0) == 0xC0)
			return ((p1[0] & 0x3f) << 6) | ((p1[1] & 0x3f));
		else if ((*p1 & 0xF0) == 0xE0)
			return ((p1[0] & 0x1f) << 12) | ((p1[1] & 0x3f) << 6) | ((p1[2] & 0x3f) << 0);
		else if ((*p1 & 0xF8) == 0xF0)
			return ((p1[0] & 0x0f) << 18) | ((p1[1] & 0x3f) << 12) | ((p1[2] & 0x3f) << 6)  | ((p1[3] & 0x3f) << 0);
		else
			return *p1; // not correct
	}
	static void encode(const code_t c, mem_t *p)
	{
		unsigned char *m = (unsigned char*)p;
		if (c < 0x0080)
		{
			m[0] = c;
		}
		else if (c < 0x800)
		{
			m[0] = 0xC0 | (c >> 6);
			m[1] = 0x80 | (c & 0x3f);
		}
		else if (c < 0x10000)
		{
			m[0] = 0xE0 | (c >> 12);
			m[1] = 0x80 | ((c>>6) & 0x3f);
			m[2] = 0x80 | (c & 0x3f);
		}
		else /* U+10000 U+1FFFFF */
		{
			m[0] = 0xF0 | (c >> 18);
			m[1] = 0x80 | ((c>>12) & 0x3f);
			m[2] = 0x80 | ((c>>6) & 0x3f);
			m[3] = 0x80 | (c & 0x3f);
		}
	}
	static const mem_t *nthcode(const mem_t *p, const unsigned n)
	{
		const mem_t *p1 = p;
		int i = n;
		while (i-- > 0)
			p1 += codelen(p1);
		return p1;
	}
};

struct pstring : public pstring_t<putf8_traits>
{
public:

	typedef pstring_t<putf8_traits> type_t;

	// simple construction/destruction
	pstring() : type_t() { }

	// construction with copy
	pstring(const mem_t *string) : type_t(string) { }
	pstring(const type_t &string) : type_t(string) { }

};

// ----------------------------------------------------------------------------------------
// pstringbuffer: a string buffer implementation
//
// string buffer are optimized to handle concatenations. This implementation is designed
// to specifically interact with pstrings nicely.
// ----------------------------------------------------------------------------------------

struct pstringbuffer
{
public:

	static const int DEFAULT_SIZE = 2048;
	// simple construction/destruction
	pstringbuffer()
	{
		init();
		resize(DEFAULT_SIZE);
	}

	~pstringbuffer();

	// construction with copy
	pstringbuffer(const char *string) {init(); if (string != NULL) pcopy(string); }
	pstringbuffer(const pstring &string) {init(); pcopy(string); }

	// assignment operators
	pstringbuffer &operator=(const char *string) { pcopy(string); return *this; }
	pstringbuffer &operator=(const pstring &string) { pcopy(string); return *this; }
	pstringbuffer &operator=(const pstringbuffer &string) { pcopy(string); return *this; }

	// C string conversion helpers
	const char *cstr() const { return m_ptr; }

	operator pstring() const { return pstring(m_ptr); }

	// concatenation operators
	pstringbuffer& operator+=(const UINT8 c) { UINT8 buf[2] = { c, 0 }; pcat((char *) buf); return *this; }
	pstringbuffer& operator+=(const pstring &string) { pcat(string); return *this; }
	pstringbuffer& operator+=(const char *string) { pcat(string); return *this; }

	std::size_t len() const { return m_len; }

	void cat(const pstring &s) { pcat(s); }
	void cat(const char *s) { pcat(s); }
	void cat(const void *m, unsigned l) { pcat(m, l); }

private:

	void init()
	{
		m_ptr = NULL;
		m_size = 0;
		m_len = 0;
	}

	void resize(const std::size_t size);

	void pcopy(const char *from);
	void pcopy(const pstring &from);
	void pcat(const char *s);
	void pcat(const pstring &s);
	void pcat(const void *m, unsigned l);

	char *m_ptr;
	std::size_t m_size;
	std::size_t m_len;

};

#endif /* _PSTRING_H_ */
