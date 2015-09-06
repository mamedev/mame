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

	const type_t vprintf(va_list args) const;
	static const type_t sprintf(const char *format, ...) ATTR_PRINTF(1,2);

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

template <typename T>
struct ptype_treats
{
};

template<>
struct ptype_treats<char>
{
	static short cast(char x) { return x; }
	static const bool is_signed = true;
	static const char *size_specifier() { return "h"; }
};

template<>
struct ptype_treats<short>
{
	static short cast(short x) { return x; }
	static const bool is_signed = true;
	static const char *size_specifier() { return "h"; }
};

template<>
struct ptype_treats<int>
{
	static int cast(int x) { return x; }
	static const bool is_signed = true;
	static const char *size_specifier() { return ""; }
};

template<>
struct ptype_treats<long>
{
	static long cast(long x) { return x; }
	static const bool is_signed = true;
	static const char *size_specifier() { return "l"; }
};

template<>
struct ptype_treats<long long>
{
	static long long cast(long long x) { return x; }
	static const bool is_signed = true;
	static const char *size_specifier() { return "ll"; }
};

template<>
struct ptype_treats<unsigned char>
{
	static unsigned short cast(unsigned char x) { return x; }
	static const bool is_signed = false;
	static const char *size_specifier() { return "h"; }
};

template<>
struct ptype_treats<unsigned short>
{
	static unsigned short cast(unsigned short x) { return x; }
	static const bool is_signed = false;
	static const char *size_specifier() { return "h"; }
};

template<>
struct ptype_treats<unsigned int>
{
	static unsigned int cast(unsigned int x) { return x; }
	static const bool is_signed = false;
	static const char *size_specifier() { return ""; }
};

template<>
struct ptype_treats<unsigned long>
{
	static unsigned long cast(unsigned long x) { return x; }
	static const bool is_signed = false;
	static const char *size_specifier() { return "l"; }
};

template<>
struct ptype_treats<unsigned long long>
{
	static unsigned long long cast(unsigned long long x) { return x; }
	static const bool is_signed = false;
	static const char *size_specifier() { return "ll"; }
};

template <typename P>
class pformat_base
{
public:

	virtual ~pformat_base() { }

	ATTR_COLD P &operator ()(const double x, const char *f = "") { format_element(f, "", "f", x); return static_cast<P &>(*this); }
	ATTR_COLD P &          e(const double x, const char *f = "") { format_element(f, "", "e", x); return static_cast<P &>(*this);  }
	ATTR_COLD P &          g(const double x, const char *f = "") { format_element(f, "", "g", x); return static_cast<P &>(*this);  }

	ATTR_COLD P &operator ()(const char *x, const char *f = "") { format_element(f, "", "s", x); return static_cast<P &>(*this);  }
	ATTR_COLD P &operator ()(char *x, const char *f = "") { format_element(f, "", "s", x); return static_cast<P &>(*this);  }
	ATTR_COLD P &operator ()(const void *x, const char *f = "") { format_element(f, "", "p", x); return static_cast<P &>(*this);  }
	ATTR_COLD P &operator ()(const pstring &x, const char *f = "") { format_element(f, "", "s", x.cstr() ); return static_cast<P &>(*this);  }

	template<typename T>
	ATTR_COLD P &operator ()(const T x, const char *f = "")
	{
		if (ptype_treats<T>::is_signed)
			format_element(f, ptype_treats<T>::size_specifier(), "d", ptype_treats<T>::cast(x));
		else
			format_element(f, ptype_treats<T>::size_specifier(), "u", ptype_treats<T>::cast(x));
		return static_cast<P &>(*this);
	}

	template<typename T>
	ATTR_COLD P &x(const T x, const char *f = "")
	{
		format_element(f, ptype_treats<T>::size_specifier(), "x", x);
		return static_cast<P &>(*this);
	}

	template<typename T>
	ATTR_COLD P &o(const T x, const char *f = "")
	{
		format_element(f, ptype_treats<T>::size_specifier(), "o", x);
		return static_cast<P &>(*this);
	}

protected:

	virtual void format_element(const char *f, const char *l, const char *fmt_spec, ...) = 0;

};

class pfmt : public pformat_base<pfmt>
{
public:
	ATTR_COLD pfmt(const pstring &fmt);
	ATTR_COLD pfmt(const char *fmt);

	operator pstring() const { return m_str; }

	const char *cstr() { return m_str; }


protected:
	void format_element(const char *f, const char *l, const char *fmt_spec, ...);

private:

	char m_str[2048];
	unsigned m_arg;
};

enum plog_level
{
	DEBUG,
	INFO,
	VERBOSE,
	WARNING,
	ERROR,
	FATAL
};

class plog_dispatch_intf;

template <bool build_enabled = true>
class pfmt_writer_t
{
public:
	pfmt_writer_t() : m_enabled(true)  { }
	virtual ~pfmt_writer_t() { }

	ATTR_COLD void operator ()(const char *fmt) const
	{
		if (build_enabled && m_enabled) vdowrite(fmt);
	}

	template<typename T1>
	ATTR_COLD void operator ()(const char *fmt, const T1 &v1) const
	{
		if (build_enabled && m_enabled) vdowrite(pfmt(fmt)(v1));
	}

	template<typename T1, typename T2>
	ATTR_COLD void operator ()(const char *fmt, const T1 &v1, const T2 &v2) const
	{
		if (build_enabled && m_enabled) vdowrite(pfmt(fmt)(v1)(v2));
	}

	template<typename T1, typename T2, typename T3>
	ATTR_COLD void operator ()(const char *fmt, const T1 &v1, const T2 &v2, const T3 &v3) const
	{
		if (build_enabled && m_enabled) vdowrite(pfmt(fmt)(v1)(v2)(v3));
	}

	template<typename T1, typename T2, typename T3, typename T4>
	ATTR_COLD void operator ()(const char *fmt, const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4) const
	{
		if (build_enabled && m_enabled) vdowrite(pfmt(fmt)(v1)(v2)(v3)(v4));
	}

	template<typename T1, typename T2, typename T3, typename T4, typename T5>
	ATTR_COLD void operator ()(const char *fmt, const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4, const T5 &v5) const
	{
		if (build_enabled && m_enabled) vdowrite(pfmt(fmt)(v1)(v2)(v3)(v4)(v5));
	}

	void set_enabled(const bool v)
	{
		m_enabled = v;
	}

	bool is_enabled() const { return m_enabled; }

protected:
	virtual void vdowrite(const pstring &ls) const {}

private:
	bool m_enabled;

};

template <plog_level L, bool build_enabled = true>
class plog_channel : public pfmt_writer_t<build_enabled>
{
public:
	plog_channel(plog_dispatch_intf *b) : pfmt_writer_t<build_enabled>(),  m_base(b) { }
	virtual ~plog_channel() { }

protected:
	virtual void vdowrite(const pstring &ls) const;

private:
	plog_dispatch_intf *m_base;
};

class plog_dispatch_intf
{
	template<plog_level, bool> friend class plog_channel;

public:
	virtual ~plog_dispatch_intf() { }
protected:
	virtual void vlog(const plog_level &l, const pstring &ls) const = 0;
};

template<bool debug_enabled>
class plog_base
{
public:

	plog_base(plog_dispatch_intf *proxy)
	: debug(proxy),
		info(proxy),
		verbose(proxy),
		warning(proxy),
		error(proxy),
		fatal(proxy)
	{}
	virtual ~plog_base() {};

	plog_channel<DEBUG, debug_enabled> debug;
	plog_channel<INFO> info;
	plog_channel<VERBOSE> verbose;
	plog_channel<WARNING> warning;
	plog_channel<ERROR> error;
	plog_channel<FATAL> fatal;
};


template <plog_level L, bool build_enabled>
void plog_channel<L, build_enabled>::vdowrite(const pstring &ls) const
{
	m_base->vlog(L, ls);
}

#endif /* _PSTRING_H_ */
