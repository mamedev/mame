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

template <typename T>
class pstring_const_iterator final
{
public:

	typedef typename T::ref_value_type value_type;

	typedef value_type const *pointer;
	typedef value_type const &reference;
	typedef std::ptrdiff_t difference_type;
	typedef std::forward_iterator_tag iterator_category;
	typedef typename T::string_type string_type;
	typedef typename T::traits_type traits_type;

	pstring_const_iterator() noexcept : p() { }
	explicit constexpr pstring_const_iterator(const typename string_type::const_iterator &x) noexcept : p(x) { }
#if !defined(_MSC_VER) || !defined(_ITERATOR_DEBUG_LEVEL) || (0 == _ITERATOR_DEBUG_LEVEL) // debug iterators are broken
	pstring_const_iterator(const pstring_const_iterator &rhs) noexcept = default;
	pstring_const_iterator(pstring_const_iterator &&rhs) noexcept = default;
	pstring_const_iterator &operator=(const pstring_const_iterator &rhs) noexcept = default;
	pstring_const_iterator &operator=(pstring_const_iterator &&rhs) noexcept = default;
#endif

	pstring_const_iterator& operator++() noexcept { p += static_cast<difference_type>(traits_type::codelen(&(*p))); return *this; }
	pstring_const_iterator operator++(int) noexcept { pstring_const_iterator tmp(*this); operator++(); return tmp; }

	bool operator==(const pstring_const_iterator& rhs) const noexcept { return p == rhs.p; }
	bool operator!=(const pstring_const_iterator& rhs) const noexcept { return p != rhs.p; }

	reference operator*() const noexcept { return *reinterpret_cast<pointer>(&(*p)); }
	pointer operator->() const noexcept { return reinterpret_cast<pointer>(&(*p)); }

private:
	template <typename G> friend struct pstring_t;
	typename string_type::const_iterator p;
};


template <typename F>
struct pstring_t
{
public:
	typedef F traits_type;

	typedef typename traits_type::mem_t mem_t;
	typedef typename traits_type::code_t code_t;
	typedef std::size_t     size_type;
	typedef std::ptrdiff_t difference_type;
	typedef typename traits_type::string_type string_type;

	class ref_value_type final
	{
	public:
		ref_value_type() = delete;
		ref_value_type(const ref_value_type &) = delete;
		ref_value_type(ref_value_type &&) = delete;
		ref_value_type &operator=(const ref_value_type &) = delete;
		ref_value_type &operator=(ref_value_type &&) = delete;
		operator code_t() const noexcept { return traits_type::code(&m); }
	private:
		const mem_t m;
	};
	typedef const ref_value_type& const_reference;
	typedef const_reference reference;

	enum enc_t
	{
		UTF8,
		UTF16
	};

	// simple construction/destruction
	pstring_t()
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

	template<typename C, std::size_t N,
		class = typename std::enable_if<std::is_same<C, const mem_t>::value>::type>
	pstring_t(C (&string)[N])
	{
		static_assert(N > 0,"pstring from array of length 0");
		if (string[N-1] != 0)
			throw std::exception();
		m_str.assign(string, N - 1);
	}

	pstring_t(const pstring_t &string)
	: m_str(string.m_str)
	{ }

	explicit pstring_t(const string_type &string, const enc_t enc)
		: m_str(string)
	{ }

	pstring_t(pstring_t &&string)
	: m_str(string.m_str)
	{  }

	explicit pstring_t(code_t code)
	{
		*this += code;
	}

	template <typename T,
		class = typename std::enable_if<!std::is_same<T, pstring_t::traits_type>::value>::type>
	explicit pstring_t(const pstring_t<T> &string)
	{
		m_str.clear();
		for (auto &c : string)
			*this += static_cast<code_t>(c); // FIXME: codepage conversion for u8
	}

	pstring_t &operator=(const pstring_t &string) { m_str = string.m_str; return *this; }

	template <typename T,
		class = typename std::enable_if<!std::is_same<T, pstring_t::traits_type>::value>::type>
	pstring_t &operator=(const pstring_t<T> &string)
	{
		m_str.clear();
		for (auto &c : string)
			*this += c;
		return *this;
	}

	// no non-const const_iterator for now
	typedef pstring_const_iterator<pstring_t> iterator;
	typedef pstring_const_iterator<pstring_t> const_iterator;

	iterator begin() { return iterator(m_str.begin()); }
	iterator end() { return iterator(m_str.end()); }
	const_iterator begin() const { return const_iterator(m_str.begin()); }
	const_iterator end() const { return const_iterator(m_str.end()); }
	const_iterator cbegin() const { return const_iterator(m_str.begin()); }
	const_iterator cend() const { return const_iterator(m_str.end()); }

	// C string conversion helpers
	const mem_t *c_str() const  {   return static_cast<const mem_t *>(m_str.c_str()); }
	const mem_t *data() const  {    return c_str(); }

	size_type length() const { return traits_type::len(m_str); }
	size_type size() const { return traits_type::len(m_str); }
	bool empty() const { return m_str.size() == 0; }

	pstring_t substr(size_type start, size_type nlen = npos) const;
	int compare(const pstring_t &right) const;

	size_type find(const pstring_t &search, size_type start = 0) const;
	size_type find(code_t search, size_type start = 0) const;

	size_type find_first_not_of(const pstring_t &no) const;
	size_type find_last_not_of(const pstring_t &no) const;

	// concatenation operators
	pstring_t& operator+=(const pstring_t &string) { m_str.append(string.m_str); return *this; }
	pstring_t& operator+=(const code_t c) { traits_type::encode(c, m_str); return *this; }
	friend pstring_t operator+(const pstring_t &lhs, const pstring_t &rhs) { return pstring_t(lhs) += rhs; }
	friend pstring_t operator+(const pstring_t &lhs, const code_t rhs) { return pstring_t(lhs) += rhs; }

	// comparison operators
	bool operator==(const pstring_t &string) const { return (compare(string) == 0); }
	bool operator!=(const pstring_t &string) const { return (compare(string) != 0); }

	bool operator<(const pstring_t &string) const { return (compare(string) < 0); }
	bool operator<=(const pstring_t &string) const { return (compare(string) <= 0); }
	bool operator>(const pstring_t &string) const { return (compare(string) > 0); }
	bool operator>=(const pstring_t &string) const { return (compare(string) >= 0); }

	const_reference at(const size_type pos) const { return *reinterpret_cast<const ref_value_type *>(F::nthcode(m_str.c_str(),pos)); }

	/* The following is not compatible to std::string */

	bool equals(const pstring_t &string) const { return (compare(string) == 0); }

	bool startsWith(const pstring_t &arg) const { return arg.mem_t_size() > mem_t_size() ? false : m_str.compare(0, arg.mem_t_size(), arg.m_str) == 0; }
	bool endsWith(const pstring_t &arg) const { return arg.mem_t_size() > mem_t_size() ? false : m_str.compare(mem_t_size()-arg.mem_t_size(), arg.mem_t_size(), arg.m_str) == 0; }

	pstring_t replace_all(const pstring_t &search, const pstring_t &replace) const;
	pstring_t cat(const pstring_t &s) const { return *this + s; }
	pstring_t cat(code_t c) const { return *this + c; }

	// conversions

	double as_double(bool *error = nullptr) const;
	long as_long(bool *error = nullptr) const;

	/* the following are extensions to <string> */

	size_type mem_t_size() const { return m_str.size(); }

	pstring_t left(size_type len) const { return substr(0, len); }
	pstring_t right(size_type nlen) const
	{
		return nlen >= length() ? *this : substr(length() - nlen, nlen);
	}

	pstring_t ltrim(const pstring_t &ws = pstring_t(" \t\n\r")) const
	{
		return substr(find_first_not_of(ws));
	}

	pstring_t rtrim(const pstring_t &ws = pstring_t(" \t\n\r")) const
	{
		auto f = find_last_not_of(ws);
		return f == npos ? pstring_t() : substr(0, f + 1);
	}

	pstring_t trim(const pstring_t &ws = pstring_t(" \t\n\r")) const { return this->ltrim(ws).rtrim(ws); }

	pstring_t rpad(const pstring_t &ws, const size_type cnt) const;

	pstring_t ucase() const;

	const string_type &cpp_string() const { return m_str; }

	static const size_type npos = static_cast<size_type>(-1);

protected:
	string_type m_str;

private:
};

struct pu8_traits
{
	typedef char mem_t;
	typedef char code_t;
	typedef std::string string_type;
	static std::size_t len(const string_type &p) { return p.size(); }
	static std::size_t codelen(const mem_t *p) { return 1; }
	static std::size_t codelen(const code_t c) { return 1; }
	static code_t code(const mem_t *p) { return *p; }
	static void encode(const code_t c, string_type &s) { s += static_cast<mem_t>(c); }
	static const mem_t *nthcode(const mem_t *p, const std::size_t n) { return &(p[n]); }
};

/* No checking, this may deliver invalid codes */
struct putf8_traits
{
	typedef char mem_t;
	typedef char32_t code_t;
	typedef std::string string_type;
	static std::size_t len(const string_type &p)
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
	static std::size_t codelen(const code_t c)
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
	static void encode(const code_t c, string_type &s)
	{
		if (c < 0x0080)
		{
			s += static_cast<mem_t>(c);
		}
		else if (c < 0x800)
		{
			s += static_cast<mem_t>(0xC0 | (c >> 6));
			s += static_cast<mem_t>(0x80 | (c & 0x3f));
		}
		else if (c < 0x10000)
		{
			s += static_cast<mem_t>(0xE0 | (c >> 12));
			s += static_cast<mem_t>(0x80 | ((c>>6) & 0x3f));
			s += static_cast<mem_t>(0x80 | (c & 0x3f));
		}
		else /* U+10000 U+1FFFFF */
		{
			s += static_cast<mem_t>(0xF0 | (c >> 18));
			s += static_cast<mem_t>(0x80 | ((c>>12) & 0x3f));
			s += static_cast<mem_t>(0x80 | ((c>>6) & 0x3f));
			s += static_cast<mem_t>(0x80 | (c & 0x3f));
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

struct putf16_traits
{
	typedef char16_t mem_t;
	typedef char32_t code_t;
	typedef std::u16string string_type;
	static std::size_t len(const string_type &p)
	{
		std::size_t ret = 0;
		auto i = p.begin();
		while (i != p.end())
		{
			// FIXME: check that size is equal
			uint16_t c = static_cast<uint16_t>(*i++);
			if (!((c & 0xd800) == 0xd800))
				ret++;
		}
		return ret;
	}
	static std::size_t codelen(const mem_t *p)
	{
		uint16_t c = static_cast<uint16_t>(*p);
		return ((c & 0xd800) == 0xd800) ? 2 : 1;
	}
	static std::size_t codelen(const code_t c)
	{
		if (c < 0x10000)
			return 1;
		else /* U+10000 U+1FFFFF */
			return 2;
	}
	static code_t code(const mem_t *p)
	{
		uint32_t c = static_cast<uint32_t>(*p++);
		if ((c & 0xd800) == 0xd800)
		{
			c = (c - 0xd800) << 10;
			c += static_cast<uint32_t>(*p) - 0xdc00 + 0x10000;
		}
		return static_cast<code_t>(c);
	}
	static void encode(code_t c, string_type &s)
	{
		uint32_t cu = static_cast<uint32_t>(c);
		if (c > 0xffff)
		{ //make a surrogate pair
			uint32_t t = ((cu - 0x10000) >> 10) + 0xd800;
			cu = (cu & 0x3ff) + 0xdc00;
			s += static_cast<mem_t>(t);
			s += static_cast<mem_t>(cu);
		}
		else
		{
			s += static_cast<mem_t>(cu);
		}
	}
	static const mem_t *nthcode(const mem_t *p, const std::size_t n)
	{
		std::size_t i = n;
		while (i-- > 0)
			p += codelen(p);
		return p;
	}
};

struct pwchar_traits
{
	typedef wchar_t mem_t;
	typedef char32_t code_t;
	typedef std::wstring string_type;
	static std::size_t len(const string_type &p)
	{
		if (sizeof(wchar_t) == 2)
		{
			std::size_t ret = 0;
			auto i = p.begin();
			while (i != p.end())
			{
				// FIXME: check that size is equal
				uint32_t c = static_cast<uint32_t>(*i++);
				if (!((c & 0xd800) == 0xd800))
					ret++;
			}
			return ret;
		}
		else
			return p.size();
	}

	static std::size_t codelen(const mem_t *p)
	{
		if (sizeof(wchar_t) == 2)
		{
			uint16_t c = static_cast<uint16_t>(*p);
			return ((c & 0xd800) == 0xd800) ? 2 : 1;
		}
		else
			return 1;
	}

	static std::size_t codelen(const code_t c)
	{
		if (sizeof(wchar_t) == 2)
			return ((c & 0xd800) == 0xd800) ? 2 : 1;
		else
			return 1;
	}

	static code_t code(const mem_t *p)
	{
		if (sizeof(wchar_t) == 2)
		{
			uint32_t c = static_cast<uint32_t>(*p++);
			if ((c & 0xd800) == 0xd800)
			{
				c = (c - 0xd800) << 10;
				c += static_cast<uint32_t>(*p) - 0xdc00 + 0x10000;
			}
			return static_cast<code_t>(c);
		}
		else
			return static_cast<code_t>(*p);
	}

	static void encode(code_t c, string_type &s)
	{
		if (sizeof(wchar_t) == 2)
		{
			uint32_t cu = static_cast<uint32_t>(c);
			if (c > 0xffff)
			{ //make a surrogate pair
				uint32_t t = ((cu - 0x10000) >> 10) + 0xd800;
				cu = (cu & 0x3ff) + 0xdc00;
				s += static_cast<mem_t>(t);
				s += static_cast<mem_t>(cu);
			}
			else
				s += static_cast<mem_t>(cu);
		}
		else
			s += static_cast<wchar_t>(c);
	}
	static const mem_t *nthcode(const mem_t *p, const std::size_t n)
	{
		if (sizeof(wchar_t) == 2)
		{
			std::size_t i = n;
			while (i-- > 0)
				p += codelen(p);
			return p;
		}
		else
			return p + n;
	}
};

extern template struct pstring_t<pu8_traits>;
extern template struct pstring_t<putf8_traits>;
extern template struct pstring_t<putf16_traits>;
extern template struct pstring_t<pwchar_traits>;

typedef pstring_t<putf8_traits> pstring;
typedef pstring_t<putf16_traits> pu16string;
typedef pstring_t<pwchar_traits> pwstring;

namespace plib
{
	template<typename T>
	pstring to_string(const T &v)
	{
		return pstring(std::to_string(v), pstring::UTF8);
	}

	template<typename T>
	pwstring to_wstring(const T &v)
	{
		return pwstring(std::to_wstring(v), pwstring::UTF16);
	}
}

// custom specialization of std::hash can be injected in namespace std
namespace std
{
	template<typename T> struct hash<pstring_t<T>>
	{
		typedef pstring_t<T> argument_type;
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
