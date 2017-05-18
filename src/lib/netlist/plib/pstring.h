// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * pstring.h
 */

#ifndef PSTRING_H_
#define PSTRING_H_

#include <iterator>
#include <exception>

// ----------------------------------------------------------------------------------------
// pstring: immutable strings ...
//
// pstrings are just a pointer to a "pascal-style" string representation.
// It uses reference counts and only uses new memory when a string changes.
// ----------------------------------------------------------------------------------------

struct pstr_t
{
	explicit pstr_t(const std::size_t alen) { init(alen); }
	void init(const std::size_t alen)
	{
		m_ref_count = 1;
		m_len = alen;
		m_str[0] = 0;
	}
	char *str() { return &m_str[0]; }
	unsigned char *ustr() { return reinterpret_cast<unsigned char *>(&m_str[0]); }
	std::size_t len() const  { return m_len; }
	void inc() { m_ref_count++; }
	bool dec_and_check() { --m_ref_count; return m_ref_count == 0; }
	void copy_from(const char *p, std::size_t n) { std::copy(p, p + n, str()); }
private:
	int m_ref_count;
	std::size_t m_len;
	char m_str[1];
};

template <typename F>
struct pstring_t
{
public:
	typedef F traits;

	typedef typename F::mem_t mem_t;
	typedef typename F::code_t code_t;
	typedef std::size_t size_type;

	enum enc_t
	{
		UTF8
	};

	// simple construction/destruction
	pstring_t() : m_ptr(&m_zero)
	{
		init(nullptr);
	}
	~pstring_t();

	// FIXME: Do something with encoding
	pstring_t(const mem_t *string, const enc_t enc) : m_ptr(&m_zero)
	{
		init(string);
	}

	template<typename C, std::size_t N>
	pstring_t(C (&string)[N]) : m_ptr(&m_zero) {
		static_assert(std::is_same<C, const mem_t>::value, "pstring constructor only accepts const mem_t");
		static_assert(N>0,"pstring from array of length 0");
		if (string[N-1] != 0)
			throw std::exception();
		init(string);
	}

	pstring_t(const pstring_t &string) : m_ptr(&m_zero) { init(string); }
	pstring_t(pstring_t &&string) : m_ptr(string.m_ptr) { string.m_ptr = nullptr; }
	explicit pstring_t(code_t code) : m_ptr(&m_zero) { pstring_t t; t+= code;  init(t); }

	// assignment operators
	pstring_t &operator=(const pstring_t &string) { pcopy(string); return *this; }

	struct iterator final : public std::iterator<std::forward_iterator_tag, mem_t>
	{
		const mem_t * p;
	public:
		explicit constexpr iterator(const mem_t *x) noexcept : p(x) {}
		iterator(const iterator &rhs) noexcept = default;
		iterator(iterator &&rhs) noexcept { p = rhs.p; }
		iterator &operator=(const iterator &it) { p = it.p; return *this; }
		iterator& operator++() noexcept { p += traits::codelen(p); return *this; }
		iterator operator++(int) noexcept { iterator tmp(*this); operator++(); return tmp; }
		bool operator==(const iterator& rhs) noexcept { return p==rhs.p; }
		bool operator!=(const iterator& rhs) noexcept { return p!=rhs.p; }
		const code_t operator*() noexcept { return traits::code(p); }
		iterator& operator+=(size_type count) { while (count>0) { --count; ++(*this); } return *this; }
		friend iterator operator+(iterator lhs, const size_type &rhs) { return (lhs += rhs); }
	};

	iterator begin() const { return iterator(m_ptr->str()); }
	iterator end() const { return iterator(m_ptr->str() + blen()); }

	// C string conversion helpers
	const mem_t *c_str() const { return m_ptr->str(); }

	// concatenation operators
	pstring_t& operator+=(const pstring_t &string) { pcat(string); return *this; }
	friend pstring_t operator+(const pstring_t &lhs, const pstring_t &rhs) { return pstring_t(lhs) += rhs; }

	// comparison operators
	bool operator==(const pstring_t &string) const { return (pcmp(string) == 0); }
	bool operator!=(const pstring_t &string) const { return (pcmp(string) != 0); }

	bool operator<(const pstring_t &string) const { return (pcmp(string) < 0); }
	bool operator<=(const pstring_t &string) const { return (pcmp(string) <= 0); }
	bool operator>(const pstring_t &string) const { return (pcmp(string) > 0); }
	bool operator>=(const pstring_t &string) const { return (pcmp(string) >= 0); }

	bool equals(const pstring_t &string) const { return (pcmp(string) == 0); }
	bool startsWith(const pstring_t &arg) const;
	bool endsWith(const pstring_t &arg) const;

	pstring_t replace(const pstring_t &search, const pstring_t &replace) const;
	const pstring_t cat(const pstring_t &s) const { return *this + s; }
	const pstring_t cat(const code_t c) const { return *this + c; }

	size_type blen() const { return m_ptr->len(); }

	// conversions

	double as_double(bool *error = nullptr) const;
	long as_long(bool *error = nullptr) const;

	size_type len() const
	{
		return traits::len(m_ptr);
	}

	pstring_t& operator+=(const code_t c) { mem_t buf[traits::MAXCODELEN+1] = { 0 }; traits::encode(c, buf); pcat(buf); return *this; }
	friend pstring_t operator+(const pstring_t &lhs, const code_t rhs) { return pstring_t(lhs) += rhs; }

	iterator find(const pstring_t &search, iterator start) const;
	iterator find(const pstring_t &search) const { return find(search, begin()); }
	iterator find(const code_t search, iterator start) const;
	iterator find(const code_t search) const { return find(search, begin()); }

	const pstring_t substr(const iterator &start, const iterator &end) const ;
	const pstring_t substr(const iterator &start) const { return substr(start, end()); }
	const pstring_t substr(size_type start) const { return (start >= len()) ? pstring_t("") : substr(begin() + start, end()); }

	const pstring_t left(iterator leftof) const { return substr(begin(), leftof); }
	const pstring_t right(iterator pos) const  { return substr(pos, end()); }

	iterator find_first_not_of(const pstring_t &no) const;
	iterator find_last_not_of(const pstring_t &no) const;

	const pstring_t ltrim(const pstring_t &ws = pstring_t(" \t\n\r")) const;
	const pstring_t rtrim(const pstring_t &ws = pstring_t(" \t\n\r")) const;
	const pstring_t trim(const pstring_t &ws = pstring_t(" \t\n\r")) const { return this->ltrim(ws).rtrim(ws); }

	const pstring_t rpad(const pstring_t &ws, const size_type cnt) const;

	code_t code_at(const size_type pos) const { return F::code(F::nthcode(m_ptr->str(),pos)); }

	const pstring_t ucase() const;

	static void resetmem();

protected:
	pstr_t *m_ptr;

private:
	void init(const mem_t *string)
	{
		m_ptr->inc();
		if (string != nullptr && *string != 0)
			pcopy(string);
	}
	void init(const pstring_t &string)
	{
		m_ptr->inc();
		pcopy(string);
	}

	int pcmp(const pstring_t &right) const;

	void pcopy(const mem_t *from, std::size_t size);

	void pcopy(const mem_t *from);
	void pcopy(const pstring_t &from)
	{
		sfree(m_ptr);
		m_ptr = from.m_ptr;
		m_ptr->inc();
	}
	void pcat(const mem_t *s);
	void pcat(const pstring_t &s);

	static pstr_t *salloc(std::size_t n);
	static void sfree(pstr_t *s);

	static pstr_t m_zero;
};

struct pu8_traits
{
	static const unsigned MAXCODELEN = 1; /* in memory units */
	typedef char mem_t;
	typedef char code_t;
	static std::size_t len(const pstr_t *p) { return p->len(); }
	static unsigned codelen(const mem_t *p) { return 1; }
	static unsigned codelen(const code_t c) { return 1; }
	static code_t code(const mem_t *p) { return *p; }
	static void encode(const code_t c, mem_t *p) { *p = c; }
	static const mem_t *nthcode(const mem_t *p, const std::size_t n) { return &(p[n]); }
};

/* No checking, this may deliver invalid codes */
struct putf8_traits
{
	static const unsigned MAXCODELEN = 4; /* in memory units,  RFC 3629 */
	typedef char mem_t;
	typedef unsigned code_t;
	static std::size_t len(pstr_t *p)
	{
		std::size_t ret = 0;
		unsigned char *c = p->ustr();
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
		const unsigned char *p1 = reinterpret_cast<const unsigned char *>(p);
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
		const unsigned char *p1 = reinterpret_cast<const unsigned char *>(p);
		if ((*p1 & 0x80) == 0x00)
			return *p1;
		else if ((*p1 & 0xE0) == 0xC0)
			return static_cast<code_t>(((p1[0] & 0x3f) << 6) | (p1[1] & 0x3f));
		else if ((*p1 & 0xF0) == 0xE0)
			return static_cast<code_t>(((p1[0] & 0x1f) << 12) | ((p1[1] & 0x3f) << 6) | ((p1[2] & 0x3f) << 0));
		else if ((*p1 & 0xF8) == 0xF0)
			return static_cast<code_t>(((p1[0] & 0x0f) << 18) | ((p1[1] & 0x3f) << 12) | ((p1[2] & 0x3f) << 6)  | ((p1[3] & 0x3f) << 0));
		else
			return *p1; // not correct
	}
	static void encode(const code_t c, mem_t *p)
	{
		unsigned char *m = reinterpret_cast<unsigned char *>(p);
		if (c < 0x0080)
		{
			m[0] = static_cast<unsigned char>(c);
		}
		else if (c < 0x800)
		{
			m[0] = static_cast<unsigned char>(0xC0 | (c >> 6));
			m[1] = static_cast<unsigned char>(0x80 | (c & 0x3f));
		}
		else if (c < 0x10000)
		{
			m[0] = static_cast<unsigned char>(0xE0 | (c >> 12));
			m[1] = static_cast<unsigned char>(0x80 | ((c>>6) & 0x3f));
			m[2] = static_cast<unsigned char>(0x80 | (c & 0x3f));
		}
		else /* U+10000 U+1FFFFF */
		{
			m[0] = static_cast<unsigned char>(0xF0 | (c >> 18));
			m[1] = static_cast<unsigned char>(0x80 | ((c>>12) & 0x3f));
			m[2] = static_cast<unsigned char>(0x80 | ((c>>6) & 0x3f));
			m[3] = static_cast<unsigned char>(0x80 | (c & 0x3f));
		}
	}
	static const mem_t *nthcode(const mem_t *p, const std::size_t n)
	{
		const mem_t *p1 = p;
		std::size_t i = n;
		while (i-- > 0)
			p1 += codelen(p1);
		return p1;
	}
};

extern template struct pstring_t<pu8_traits>;
extern template struct pstring_t<putf8_traits>;

typedef pstring_t<putf8_traits> pstring;

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
	explicit pstringbuffer(const char *string) {init(); if (string != nullptr) pcopy(string); }
	explicit pstringbuffer(const pstring &string) {init(); pcopy(string); }
	pstringbuffer(const pstringbuffer &stringb) {init(); pcopy(stringb); }
	pstringbuffer(pstringbuffer &&b) : m_ptr(b.m_ptr), m_size(b.m_size), m_len(b.m_len) { b.m_ptr = nullptr; b.m_size = 0; b.m_len = 0; }

	// assignment operators
	pstringbuffer &operator=(const char *string) { pcopy(string); return *this; }
	pstringbuffer &operator=(const pstring &string) { pcopy(string); return *this; }
	pstringbuffer &operator=(const pstringbuffer &string) { pcopy(string); return *this; }

	// C string conversion helpers
	const char *c_str() const { return m_ptr; }

	// FIXME: encoding should be parameter
	operator pstring() const { return pstring(m_ptr, pstring::UTF8); }

	// concatenation operators
	pstringbuffer& operator+=(const char c) { char buf[2] = { c, 0 }; pcat(buf); return *this; }
	pstringbuffer& operator+=(const pstring &string) { pcat(string); return *this; }
	pstringbuffer& operator+=(const char *string) { pcat(string); return *this; }

	std::size_t len() const { return m_len; }

	void cat(const pstring &s) { pcat(s); }
	void cat(const char *s) { pcat(s); }
	void cat(const void *m, std::size_t l) { pcat(m, l); }

	void clear() { m_len = 0; *m_ptr = 0; }

private:

	void init()
	{
		m_ptr = nullptr;
		m_size = 0;
		m_len = 0;
	}

	void resize(const std::size_t size);

	void pcopy(const char *from);
	void pcopy(const pstring &from);
	void pcat(const char *s);
	void pcat(const pstring &s);
	void pcat(const void *m, std::size_t l);

	char *m_ptr;
	std::size_t m_size;
	std::size_t m_len;

};

// custom specialization of std::hash can be injected in namespace std
namespace std
{
	template<> struct hash<pstring>
	{
		typedef pstring argument_type;
		typedef std::size_t result_type;
		result_type operator()(argument_type const& s) const
		{
			const pstring::mem_t *string = s.c_str();
			result_type result = 5381;
			for (pstring::mem_t c = *string; c != 0; c = *string++)
				result = ((result << 5) + result ) ^ (result >> (32 - 5)) ^ static_cast<result_type>(c);
			return result;
		}
	};
}

#endif /* PSTRING_H_ */
