// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * pstring.h
 */

#ifndef PSTRING_H_
#define PSTRING_H_

#include "ptypes.h"

#include <exception>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>

// ----------------------------------------------------------------------------------------
// pstring: semi-immutable strings ...
//
// The only reason this class exists is the absence of support for multi-byte
// strings in std:: which I would consider sub-optimal for the use-cases I encounter.
// ----------------------------------------------------------------------------------------

// enable this to use std::string instead of pstring globally.

#define PSTRING_USE_STD_STRING  (0)

template <typename T>
class pstring_const_iterator final
{
public:

	using value_type = typename T::ref_value_type;

	using pointer = value_type const *;
	using reference = value_type const &;
	using difference_type = std::ptrdiff_t;
	using iterator_category = std::forward_iterator_tag;
	using string_type = typename T::string_type;
	using traits_type = typename T::traits_type;

	constexpr pstring_const_iterator() noexcept : p() { }
	explicit constexpr pstring_const_iterator(const typename string_type::const_iterator &x) noexcept : p(x) { }

	pstring_const_iterator& operator++() noexcept { p += static_cast<difference_type>(traits_type::codelen(&(*p))); return *this; }
	// NOLINTNEXTLINE(cert-dcl21-cpp)
	pstring_const_iterator operator++(int) & noexcept { pstring_const_iterator tmp(*this); operator++(); return tmp; }

	constexpr bool operator==(const pstring_const_iterator& rhs) const noexcept { return p == rhs.p; }
	constexpr bool operator!=(const pstring_const_iterator& rhs) const noexcept { return p != rhs.p; }

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
	using traits_type = F;

	using mem_t = typename traits_type::mem_t;
	using code_t = typename traits_type::code_t;
	using value_type = typename traits_type::code_t;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using string_type = typename traits_type::string_type;

	// FIXME: this is ugly
	struct ref_value_type final
	{
	public:
		ref_value_type() = delete;
		~ref_value_type() = delete;
		ref_value_type(const ref_value_type &) = delete;
		ref_value_type(ref_value_type &&) = delete;
		ref_value_type &operator=(const ref_value_type &) = delete;
		ref_value_type &operator=(ref_value_type &&) = delete;
		operator code_t() const noexcept { return traits_type::code(&m); }
	private:
		const mem_t m;
	};
	using const_reference = const ref_value_type &;
	using reference = const_reference;

	// simple construction/destruction
	pstring_t() = default;
	~pstring_t() noexcept = default;

	// FIXME: Do something with encoding
	pstring_t(const mem_t *string)
	: m_str(string)
	{
	}

	pstring_t(const mem_t *string, const size_type len)
	: m_str(string, len)
	{
	}

	template<typename C, std::size_t N,
		class = typename std::enable_if<std::is_same<C, const mem_t>::value>::type>
	pstring_t(C (&string)[N]) // NOLINT(cppcoreguidelines-avoid-c-arrays, modernize-avoid-c-arrays)
	{
		static_assert(N > 0,"pstring from array of length 0");
		if (string[N-1] != 0)
			throw std::exception();
		m_str.assign(string, N - 1);
	}


	explicit pstring_t(const string_type &string)
		: m_str(string)
	{ }

	pstring_t(const pstring_t &string) = default;
	pstring_t(pstring_t &&string) noexcept = default;
	pstring_t &operator=(const pstring_t &string) = default;
	pstring_t &operator=(pstring_t &&string) noexcept = default;

	explicit pstring_t(size_type n, code_t code)
	{
		while (n--)
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

	operator string_type () const { return m_str; }


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
	using iterator = pstring_const_iterator<pstring_t<F> >;
	using const_iterator = pstring_const_iterator<pstring_t<F> >;

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

	static constexpr const size_type npos = static_cast<size_type>(-1);

	/* the following are extensions to <string> */

	// FIXME: remove those
	size_type mem_t_size() const { return m_str.size(); }

private:
	string_type m_str;
};

struct pu8_traits
{
	using mem_t = char;
	using code_t = char;
	using string_type = std::string;
	static std::size_t len(const string_type &p) { return p.size(); }
	static std::size_t codelen(const mem_t *p) { plib::unused_var(p); return 1; }
	static std::size_t codelen(const code_t c) { plib::unused_var(c); return 1; }
	static code_t code(const mem_t *p) { return *p; }
	static void encode(const code_t c, string_type &s) { s += static_cast<mem_t>(c); }
	static const mem_t *nthcode(const mem_t *p, const std::size_t n) { return &(p[n]); }
};

/* No checking, this may deliver invalid codes */
struct putf8_traits
{
	using mem_t = char;
	using code_t = char32_t;
	using string_type = std::string;
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
		const auto p1 = reinterpret_cast<const unsigned char *>(p);
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
		const auto p1 = reinterpret_cast<const unsigned char *>(p);
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
	using mem_t = char16_t;
	using code_t = char32_t;
	using string_type = std::u16string;
	static std::size_t len(const string_type &p)
	{
		std::size_t ret = 0;
		auto i = p.begin();
		while (i != p.end())
		{
			// FIXME: check that size is equal
			auto c = static_cast<uint16_t>(*i++);
			if (!((c & 0xd800) == 0xd800))
				ret++;
		}
		return ret;
	}
	static std::size_t codelen(const mem_t *p)
	{
		auto c = static_cast<uint16_t>(*p);
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
		auto c = static_cast<uint32_t>(*p++);
		if ((c & 0xd800) == 0xd800)
		{
			c = (c - 0xd800) << 10;
			c += static_cast<uint32_t>(*p) - 0xdc00 + 0x10000;
		}
		return static_cast<code_t>(c);
	}
	static void encode(code_t c, string_type &s)
	{
		auto cu = static_cast<uint32_t>(c);
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
	using mem_t = wchar_t;
	using code_t = char32_t;
	using string_type = std::wstring;
	static std::size_t len(const string_type &p)
	{
		if (sizeof(wchar_t) == 2)
		{
			std::size_t ret = 0;
			auto i = p.begin();
			while (i != p.end())
			{
				// FIXME: check that size is equal
				auto c = static_cast<uint32_t>(*i++);
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
			auto c = static_cast<uint16_t>(*p);
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
			auto c = static_cast<uint32_t>(*p++);
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
			auto cu = static_cast<uint32_t>(c);
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

#if (PSTRING_USE_STD_STRING)
typedef std::string pstring;
static inline pstring::size_type pstring_mem_t_size(const pstring &s) { return s.size(); }
#else
using pstring = pstring_t<putf8_traits>;
static inline pstring::size_type pstring_mem_t_size(const pstring &s) { return s.mem_t_size(); }
#endif
using putf8string = pstring_t<putf8_traits>;
using pu16string = pstring_t<putf16_traits>;
using pwstring = pstring_t<pwchar_traits>;

namespace plib
{
	template<class T>
	struct string_info
	{
		using mem_t = typename T::mem_t;
	};

	template<>
	struct string_info<std::string>
	{
		using mem_t = char;
	};

	template<typename T>
	pstring to_string(const T &v)
	{
		return pstring(std::to_string(v));
	}

	template<typename T>
	pwstring to_wstring(const T &v)
	{
		return pwstring(std::to_wstring(v));
	}


	template<typename T>
	typename T::size_type find_first_not_of(const T &str, const T &no)
	{
		typename T::size_type pos = 0;
		for (auto it = str.begin(); it != str.end(); ++it, ++pos)
		{
			bool f = true;
			for (typename T::value_type const jt : no)
			{
				if (*it == jt)
				{
					f = false;
					break;
				}
			}
			if (f)
				return pos;
		}
		return T::npos;
	}

	template<typename T>
	typename T::size_type find_last_not_of(const T &str, const T &no)
	{
		/* FIXME: reverse iterator */
		typename T::size_type last_found = T::npos;
		typename T::size_type pos = 0;
		for (auto it = str.begin(); it != str.end(); ++it, ++pos)
		{
			bool f = true;
			for (typename T::value_type const jt : no)
			{
				if (*it == jt)
				{
					f = false;
					break;
				}
			}
			if (f)
				last_found = pos;
		}
		return last_found;
	}

	template<typename T>
	T ltrim(const T &str, const T &ws = T(" \t\n\r"))
	{
		auto f = find_first_not_of(str, ws);
		return (f == T::npos) ? T() : str.substr(f);
	}

	template<typename T>
	T rtrim(const T &str, const T &ws = T(" \t\n\r"))
	{
		auto f = find_last_not_of(str, ws);
		return (f == T::npos) ? T() : str.substr(0, f + 1);
	}

	template<typename T>
	T trim(const T &str, const T &ws = T(" \t\n\r"))
	{
		return rtrim(ltrim(str, ws), ws);
	}

	template<typename T>
	T left(const T &str, typename T::size_type len)
	{
		return str.substr(0, len);
	}

	template<typename T>
	T right(const T &str, typename T::size_type nlen)
	{
		return nlen >= str.length() ? str : str.substr(str.length() - nlen, nlen);
	}

	template<typename T>
	bool startsWith(const T &str, const T &arg)
	{
		return (arg == left(str, arg.length()));
	}

	template<typename T>
	bool endsWith(const T &str, const T &arg)
	{
		return (right(str, arg.length()) == arg);
	}

	template<typename T>
	bool startsWith(const T &str, const char *arg)
	{
		return startsWith(str, static_cast<pstring>(arg));
	}

	template<typename T>
	bool endsWith(const T &str, const char *arg)
	{
		return endsWith(str, static_cast<pstring>(arg));
	}

	template<typename T>
	std::size_t strlen(const T *str)
	{
		const T *p = str;
		while (*p)
			p++;
		return p - str;
	}

	template<typename T>
	T ucase(const T &str)
	{
		T ret;
		for (const auto &c : str)
			if (c >= 'a' && c <= 'z')
				ret += (c - 'a' + 'A');
			else
				ret += c;
		return ret;
	}

	template<typename T>
	T rpad(const T &str, const T &ws, const typename T::size_type cnt)
	{
		// FIXME: pstringbuffer ret(*this);

		T ret(str);
		typename T::size_type wsl = ws.length();
		for (auto i = ret.length(); i < cnt; i+=wsl)
			ret += ws;
		return ret;
	}

	template<typename T>
	T replace_all(const T &str, const T &search, const T &replace)
	{
		T ret;
		const typename T::size_type slen = search.length();

		typename T::size_type last_s = 0;
		typename T::size_type s = str.find(search, last_s);
		while (s != T::npos)
		{
			ret += str.substr(last_s, s - last_s);
			ret += replace;
			last_s = s + slen;
			s = str.find(search, last_s);
		}
		ret += str.substr(last_s);
		return ret;
	}

	template<typename T, typename T1, typename T2>
	T replace_all(const T &str, const T1 &search, const T2 &replace)
	{
		return replace_all(str, static_cast<T>(search), static_cast<T>(replace));
	}

} // namespace plib

// custom specialization of std::hash can be injected in namespace std
namespace std
{

	template<typename T> struct hash<pstring_t<T>>
	{
		using argument_type = pstring_t<T>;
		using result_type = std::size_t;
		result_type operator()(const argument_type & s) const
		{
			const typename argument_type::mem_t *string = s.c_str();
			result_type result = 5381;
			for (typename argument_type::mem_t c = *string; c != 0; c = *string++)
				result = ((result << 5) + result ) ^ (result >> (32 - 5)) ^ static_cast<result_type>(c);
			return result;
		}
	};
} // namespace std

#endif /* PSTRING_H_ */
