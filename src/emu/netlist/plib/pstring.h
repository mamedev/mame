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

struct pstring
{
public:
	// simple construction/destruction
	pstring()
	{
		init();
	}
	~pstring();

	// construction with copy
	pstring(const char *string) {init(); if (string != NULL && *string != 0) pcopy(string); }
	pstring(const pstring &string) {init(); pcopy(string); }

	// assignment operators
	pstring &operator=(const char *string) { pcopy(string); return *this; }
	pstring &operator=(const pstring &string) { pcopy(string); return *this; }

	// C string conversion operators and helpers
	operator const char *() const { return m_ptr->str(); }
	const char *cstr() const { return m_ptr->str(); }

	// concatenation operators
	pstring& operator+=(const char c) { char buf[2] = { c, 0 }; pcat(buf); return *this; }
	pstring& operator+=(const pstring &string) { pcat(string.cstr()); return *this; }
	pstring& operator+=(const char *string) { pcat(string); return *this; }
	friend pstring operator+(const pstring &lhs, const pstring &rhs) { return pstring(lhs) += rhs; }
	friend pstring operator+(const pstring &lhs, const char *rhs) { return pstring(lhs) += rhs; }
	friend pstring operator+(const pstring &lhs, const char rhs) { return pstring(lhs) += rhs; }
	friend pstring operator+(const char *lhs, const pstring &rhs) { return pstring(lhs) += rhs; }

	// comparison operators
	bool operator==(const char *string) const { return (pcmp(string) == 0); }
	bool operator==(const pstring &string) const { return (pcmp(string.cstr()) == 0); }
	bool operator!=(const char *string) const { return (pcmp(string) != 0); }
	bool operator!=(const pstring &string) const { return (pcmp(string.cstr()) != 0); }
	bool operator<(const char *string) const { return (pcmp(string) < 0); }
	bool operator<(const pstring &string) const { return (pcmp(string.cstr()) < 0); }
	bool operator<=(const char *string) const { return (pcmp(string) <= 0); }
	bool operator<=(const pstring &string) const { return (pcmp(string.cstr()) <= 0); }
	bool operator>(const char *string) const { return (pcmp(string) > 0); }
	bool operator>(const pstring &string) const { return (pcmp(string.cstr()) > 0); }
	bool operator>=(const char *string) const { return (pcmp(string) >= 0); }
	bool operator>=(const pstring &string) const { return (pcmp(string.cstr()) >= 0); }

	int len() const { return m_ptr->len(); }

	bool equals(const pstring &string) const { return (pcmp(string.cstr(), m_ptr->str()) == 0); }
	bool iequals(const pstring &string) const { return (pcmpi(string.cstr(), m_ptr->str()) == 0); }

	int cmp(const pstring &string) const { return pcmp(string.cstr()); }
	int cmpi(const pstring &string) const { return pcmpi(cstr(), string.cstr()); }

	int find(const char *search, int start = 0) const;

	int find(const char search, int start = 0) const;

	// various

	bool startsWith(const pstring &arg) const { return (pcmp(cstr(), arg.cstr(), arg.len()) == 0); }
	bool startsWith(const char *arg) const;

	bool endsWith(const pstring &arg) const { return (this->right(arg.len()) == arg); }
	bool endsWith(const char *arg) const { return endsWith(pstring(arg)); }

	pstring replace(const pstring &search, const pstring &replace) const;

	// these return nstring ...
	const pstring cat(const pstring &s) const { return *this + s; }
	const pstring cat(const char *s) const { return *this + s; }

	const pstring substr(unsigned int start, int count = -1) const ;

	const pstring left(unsigned int count) const { return substr(0, count); }
	const pstring right(unsigned int count) const  { return substr(len() - count, count); }

	int find_first_not_of(const pstring &no) const;
	int find_last_not_of(const pstring &no) const;

	const pstring ltrim(const pstring &ws = " \t\n\r") const;
	const pstring rtrim(const pstring &ws = " \t\n\r") const;
	const pstring trim(const pstring &ws = " \t\n\r") const { return this->ltrim(ws).rtrim(ws); }

	const pstring rpad(const pstring &ws, const int cnt) const
	{
		// FIXME: slow!
		pstring ret = *this;
		while (ret.len() < cnt)
			ret += ws;
		return ret.substr(0, cnt);
	}

	const pstring ucase() const;

	// conversions

	double as_double(bool *error = NULL) const;

	long as_long(bool *error = NULL) const;

	// printf using string as format ...

	const pstring vprintf(va_list args) const;

	// static
	static const pstring sprintf(const char *format, ...) ATTR_PRINTF(1,2);
	static void resetmem();

protected:

	struct str_t
	{
		//str_t() : m_ref_count(1), m_len(0) { m_str[0] = 0; }
		str_t(const int alen)
		{
			init(alen);
		}
		void init(const int alen)
		{
				m_ref_count = 1;
				m_len = alen;
				m_str[0] = 0;
		}
		char *str() { return &m_str[0]; }
		int len() { return m_len; }
		int m_ref_count;
	private:
		int m_len;
		char m_str[1];
	};

	str_t *m_ptr;

private:
	void init()
	{
		m_ptr = &m_zero;
		m_ptr->m_ref_count++;
	}

	int pcmp(const char *right) const
	{
		return pcmp(m_ptr->str(), right);
	}

	int pcmp(const char *left, const char *right, int count = -1) const;

	int pcmpi(const char *lhs, const char *rhs, int count = -1) const;

	void pcopy(const char *from, int size);

	void pcopy(const char *from);

	void pcopy(const pstring &from)
	{
		sfree(m_ptr);
		m_ptr = from.m_ptr;
		m_ptr->m_ref_count++;
	}

	void pcat(const char *s);

	static str_t *salloc(int n);
	static void sfree(str_t *s);

	static str_t m_zero;
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
	pstringbuffer &operator=(const pstringbuffer &string) { pcopy(string.cstr()); return *this; }

	// C string conversion operators and helpers
	operator const char *() const { return m_ptr; }
	const char *cstr() const { return m_ptr; }

	operator pstring() const { return pstring(m_ptr); }

	// concatenation operators
	pstringbuffer& operator+=(const char c) { char buf[2] = { c, 0 }; pcat(buf); return *this; }
	pstringbuffer& operator+=(const pstring &string) { pcat(string.cstr()); return *this; }

	std::size_t len() const { return m_len; }

	void cat(const pstring &s) { pcat(s); }
	void cat(const char *s) { pcat(s); }

	pstring substr(unsigned int start, int count = -1)
	{
		return pstring(m_ptr).substr(start, count);
	}

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

	char *m_ptr;
	std::size_t m_size;
	std::size_t m_len;

};


#endif /* _PSTRING_H_ */
