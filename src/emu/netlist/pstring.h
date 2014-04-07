// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * pstring.h
 */

#ifndef _PSTRING_H_
#define _PSTRING_H_

#include "nl_config.h"
#include <cstdio>

// ----------------------------------------------------------------------------------------
// pblockbool: allocate small memory more efficiently at the expense of some overhead
// ----------------------------------------------------------------------------------------

struct pblockpool {
	NETLIST_PREVENT_COPYING(pblockpool)
public:
	static const int MINDATASIZE = 8;

	pblockpool();
	~pblockpool();

	void resetmem();

	void *alloc(std::size_t n);
	void dealloc(void *ptr);

	template<class T>
	void destroy(T* object)
	{
		object->~T();
		dealloc(object);
	}
	bool m_shutdown;

private:
	struct memblock
	{
		memblock *next;
		int size;
		int allocated;
		int remaining;
		char *cur;
		char data[MINDATASIZE];
	};

	memblock *m_first;
	int m_blocksize;
	int m_align;
};

/* objects must be destroyed using destroy above */

inline void *operator new(std::size_t size, pblockpool &pool, int extra = 0) throw (std::bad_alloc)
{
	void *result = pool.alloc(size + extra);
	//std::printf("allocating %ld + %d\n", size, extra);
	if (result == NULL)
		throw std::bad_alloc();
	return result;
}

inline void operator delete(void *pMem, pblockpool &pool, int extra)
{
	pool.dealloc(pMem);
}

// ----------------------------------------------------------------------------------------
// nstring: immutable strings ...
//
// nstrings are just a pointer to a "pascal-style" string representation.
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
	inline const char *cstr() const { return m_ptr->str(); }

	// concatenation operators
	pstring& operator+=(const char c) { char buf[2] = { c, 0 }; pcat(buf); return *this; }
	pstring& operator+=(const pstring &string) { pcat(string.cstr()); return *this; }
	friend pstring operator+(const pstring &lhs, const pstring &rhs) { return pstring(lhs) += rhs; }
	friend pstring operator+(const pstring &lhs, const char *rhs) { return pstring(lhs) += rhs; }
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

	//
	inline int len() const { return m_ptr->len(); }

	inline bool equals(const pstring &string) const { return (pcmp(string.cstr(), m_ptr->str()) == 0); }
	inline bool iequals(const pstring &string) const { return (pcmpi(string.cstr(), m_ptr->str()) == 0); }

	inline int cmp(const pstring &string) const { return pcmp(string.cstr()); }
	inline int cmpi(const pstring &string) const { return pcmpi(cstr(), string.cstr()); }

	inline int find(const char *search, int start = 0) const
	{
		int alen = len();
		const char *result = strstr(cstr() + MIN(start, alen), search);
		return (result != NULL) ? (result - cstr()) : -1;
	}

	inline int find(const char search, int start = 0) const
	{
		int alen = len();
		const char *result = strchr(cstr() + MIN(start, alen), search);
		return (result != NULL) ? (result - cstr()) : -1;
	}

	// various

	inline bool startsWith(const pstring &arg) const { return (pcmp(cstr(), arg.cstr(), arg.len()) == 0); }
	inline bool startsWith(const char *arg) const { return (pcmp(cstr(), arg, strlen(arg)) == 0); }

	// these return nstring ...
	inline pstring cat(const pstring &s) const { return *this + s; }
	inline pstring cat(const char *s) const { return *this + s; }

	pstring substr(unsigned int start, int count = -1) const ;

	inline pstring left(unsigned int count) const { return substr(0, count); }
	inline pstring right(unsigned int count) const  { return substr(len() - count, count); }

	pstring ucase() const;

	// conversions

	double as_double(bool *error = NULL) const;

	long as_long(bool *error = NULL) const;

	// printf using string as format ...

	pstring vprintf(va_list args) const;

	// static
	static pstring sprintf(const char *format, ...) ATTR_PRINTF(1,2);
	static void resetmem();

protected:

	struct str_t
	{
		str_t() : m_ref_count(1), m_len(0) { m_str[0] = 0; }
		str_t(int alen) : m_ref_count(1), m_len(alen) { m_str[0] = 0; }

		char *str() { return &m_str[0]; }
		int len() { return m_len; }
	//private:
		int m_ref_count;
		int m_len;
		char m_str[1];
	};

	str_t *m_ptr;

	static pblockpool m_pool;

private:
	inline void init()
	{
		m_ptr = &m_zero;
		m_ptr->m_ref_count++;
	}

	inline int pcmp(const char *right) const
	{
		return pcmp(m_ptr->str(), right);
	}

	inline int pcmp(const char *left, const char *right, int count = -1) const
	{
		if (count < 0)
			return strcmp(left, right);
		else
			return strncmp(left, right, count);
	}

	int pcmpi(const char *lhs, const char *rhs, int count = -1) const;

	void pcopy(const char *from, int size);

	inline void pcopy(const char *from)
	{
		pcopy(from, strlen(from));
	}

	inline void pcopy(const pstring &from)
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



#endif /* _PSTRING_H_ */
