// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * pstring.h
 */

#ifndef PSTRING_H_
#define PSTRING_H_

#include <iterator>
#include <exception>
#include <string>

// ----------------------------------------------------------------------------------------
// pstring: semi-immutable strings ...
//
// The only reason this class exists is the absence of support for multi-byte
// strings in std:: which I would consider usuable for the use-cases I encounter.
// ----------------------------------------------------------------------------------------

template <typename F>
struct pstring_t
{
public:
	typedef F traits_type;

	typedef typename traits_type::mem_t mem_t;
	typedef typename traits_type::code_t code_t;
	typedef std::size_t 	size_type;
    typedef std::ptrdiff_t difference_type;

	enum enc_t
	{
		UTF8
	};

	// simple construction/destruction
	pstring_t()
	: m_str("")
	{
	}
	~pstring_t()
	{
	}

	// FIXME: Do something with encoding
	pstring_t(const mem_t *string, const enc_t enc)
	: m_str(string)
	{
	}

	pstring_t(const mem_t *string, const size_type len, const enc_t enc)
	: m_str(string, len)
	{
	}

	template<typename C, std::size_t N>
	pstring_t(C (&string)[N])
	{
		static_assert(std::is_same<C, const mem_t>::value, "pstring constructor only accepts const mem_t");
		static_assert(N > 0,"pstring from array of length 0");
		if (string[N-1] != 0)
			throw std::exception();
		m_str.assign(string, N - 1);
	}

	pstring_t(const pstring_t &string)
	: m_str(string.m_str)
	{ }

	pstring_t(pstring_t &&string)
	: m_str(string.m_str)
	{  }

	explicit pstring_t(code_t code)
	{
		pstring_t t;
		t+= code;
		m_str.assign(t.m_str);
	}

	// assignment operators
	pstring_t &operator=(const pstring_t &string) { m_str = string.m_str; return *this; }

	class const_iterator final
	{
	public:
		class value_type final
		{
		public:
			value_type() = delete;
			value_type(const value_type &) = delete;
			value_type(value_type &&) = delete;
			value_type &operator=(const value_type &) = delete;
			value_type &operator=(value_type &&) = delete;
			operator code_t() const noexcept { return traits_type::code(&m); }
		private:
			const mem_t m;
		};

		typedef value_type const *pointer;
		typedef value_type const &reference;
		typedef std::ptrdiff_t difference_type;
		typedef std::forward_iterator_tag iterator_category;

		const_iterator() noexcept : p() { }
		explicit constexpr const_iterator(const std::string::const_iterator &x) noexcept : p(x) { }
		const_iterator(const const_iterator &rhs) noexcept = default;
		const_iterator(const_iterator &&rhs) noexcept = default;
		const_iterator &operator=(const const_iterator &rhs) noexcept = default;
		const_iterator &operator=(const_iterator &&rhs) noexcept = default;

		const_iterator& operator++() noexcept { p += static_cast<difference_type>(traits_type::codelen(&(*p))); return *this; }
		const_iterator operator++(int) noexcept { const_iterator tmp(*this); operator++(); return tmp; }

		bool operator==(const const_iterator& rhs) const noexcept { return p == rhs.p; }
		bool operator!=(const const_iterator& rhs) const noexcept { return p != rhs.p; }

		reference operator*() const noexcept { return *reinterpret_cast<pointer>(&(*p)); }
		pointer operator->() const noexcept { return reinterpret_cast<pointer>(&(*p)); }

	private:
		template <typename G> friend struct pstring_t;
		std::string::const_iterator p;
	};

	// no non-const const_iterator for now
	typedef const_iterator iterator;

	iterator begin() { return iterator(m_str.begin()); }
	iterator end() { return iterator(m_str.end()); }
	const_iterator begin() const { return const_iterator(m_str.begin()); }
	const_iterator end() const { return const_iterator(m_str.end()); }
	const_iterator cbegin() const { return const_iterator(m_str.begin()); }
	const_iterator cend() const { return const_iterator(m_str.end()); }

	// C string conversion helpers
	const mem_t *c_str() const  { 	return static_cast<const mem_t *>(m_str.c_str()); }

	size_type size() const { return m_str.size(); }

	pstring_t substr(size_type start, size_type nlen = npos) const;

	size_type find(const pstring_t &search, size_type start = 0) const;
	size_type find(code_t search, size_type start = 0) const;

	size_type find_first_not_of(const pstring_t &no) const;
	size_type find_last_not_of(const pstring_t &no) const;

	// concatenation operators
	pstring_t& operator+=(const pstring_t &string) { m_str.append(string.m_str); return *this; }
	pstring_t& operator+=(const code_t c) { mem_t buf[traits_type::MAXCODELEN+1] = { 0 }; traits_type::encode(c, buf); m_str.append(buf); return *this; }
	friend pstring_t operator+(const pstring_t &lhs, const pstring_t &rhs) { return pstring_t(lhs) += rhs; }
	friend pstring_t operator+(const pstring_t &lhs, const code_t rhs) { return pstring_t(lhs) += rhs; }

	// comparison operators
	bool operator==(const pstring_t &string) const { return (pcmp(string) == 0); }
	bool operator!=(const pstring_t &string) const { return (pcmp(string) != 0); }

	bool operator<(const pstring_t &string) const { return (pcmp(string) < 0); }
	bool operator<=(const pstring_t &string) const { return (pcmp(string) <= 0); }
	bool operator>(const pstring_t &string) const { return (pcmp(string) > 0); }
	bool operator>=(const pstring_t &string) const { return (pcmp(string) >= 0); }

	/* The following is not compatible to std::string */

	bool equals(const pstring_t &string) const { return (pcmp(string) == 0); }

	bool startsWith(const pstring_t &arg) const { return arg.size() > size() ? false : m_str.compare(0, arg.size(), arg.m_str) == 0; }
	bool endsWith(const pstring_t &arg) const { return arg.size() > size() ? false : m_str.compare(size()-arg.size(), arg.size(), arg.m_str) == 0; }

	pstring_t replace_all(const pstring_t &search, const pstring_t &replace) const;
	pstring_t cat(const pstring_t &s) const { return *this + s; }
	pstring_t cat(code_t c) const { return *this + c; }

	// conversions

	double as_double(bool *error = nullptr) const;
	long as_long(bool *error = nullptr) const;

	size_type len() const { return traits_type::len(m_str); }

	/* the following are extensions to <string> */

	pstring_t left(size_type len) const { return substr(0, len); }
	pstring_t right(size_type nlen) const
	{
		return nlen >= len() ? pstring_t(*this) : substr(len() - nlen, nlen);
	}

	pstring_t ltrim(const pstring_t &ws = pstring_t(" \t\n\r")) const
	{
		return substr(find_first_not_of(ws));
	}

	pstring_t rtrim(const pstring_t &ws = pstring_t(" \t\n\r")) const
	{
		auto f = find_last_not_of(ws);
		if (f==npos)
			return pstring_t("");
		else
			return substr(0, f + 1);
	}

	pstring_t trim(const pstring_t &ws = pstring_t(" \t\n\r")) const { return this->ltrim(ws).rtrim(ws); }

	pstring_t rpad(const pstring_t &ws, const size_type cnt) const;

	code_t code_at(const size_type pos) const {return F::code(F::nthcode(m_str.c_str(),pos)); }

	pstring_t ucase() const;

	static const size_type npos = static_cast<size_type>(-1);

protected:
	std::string m_str;

private:

	int pcmp(const pstring_t &right) const;
};

struct pu8_traits
{
	static const unsigned MAXCODELEN = 1; /* in memory units */
	typedef char mem_t;
	typedef char code_t;
	static std::size_t len(const std::string &p) { return p.size(); }
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
	typedef char32_t code_t;
	static std::size_t len(const std::string &p)
	{
		std::size_t ret = 0;
		for (const auto &c : p)
		{
			if (!((c & 0xC0) == 0x80))
				ret++;
		}
		return ret;
	}
	static std::size_t codelen(const mem_t *p)
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
	static char32_t codelen(const code_t c)
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
