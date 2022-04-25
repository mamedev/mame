// license:BSD-3-Clause
// copyright-holders:Couriersud

#ifndef PSTRING_H_
#define PSTRING_H_

///
/// \file pstring.h
///

#include "ptypes.h"

#include <exception>
#include <iterator>
#include <limits>
#include <ostream>
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

	reference operator*() const noexcept { return *reinterpret_cast<pointer>(&(*p)); } // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
	pointer operator->() const noexcept { return reinterpret_cast<pointer>(&(*p)); }   // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)

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
	// no non-const const_iterator for now
	using iterator = pstring_const_iterator<pstring_t<F> >;
	using const_iterator = pstring_const_iterator<pstring_t<F> >;

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

	pstring_t(const mem_t *string, const size_type len)
	: m_str(string, len)
	{
	}

	// mingw treats string constants as char* instead of char[N]
	template<typename C,
		class = std::enable_if_t<std::is_same<C, const mem_t>::value>>
	pstring_t(const C *string)
	: m_str(string)
	{
	}

	template<typename C, std::size_t N,
		class = std::enable_if_t<std::is_same<C, const mem_t>::value>>
	pstring_t(C (&string)[N]) noexcept(false)  // NOLINT(cppcoreguidelines-avoid-c-arrays, modernize-avoid-c-arrays)
	{
		static_assert(N > 0,"pstring from array of length 0");
		// need std::exception since pexception depends on pstring
		if (string[N-1] != 0)
			throw std::exception();
		m_str.assign(string, N - 1);
	}

	// interpret other string as putf8strings
	template <typename C,
		class = std::enable_if_t<std::is_same<std::remove_const_t<C>, char>::value
		&& !std::is_same<std::remove_const_t<C>, mem_t>::value>>
	pstring_t(C *string);

	explicit pstring_t(const string_type &string)
		: m_str(string)
	{ }

	pstring_t(iterator first, iterator last)
	{
		m_str.assign(first.p, last.p);
	}

	pstring_t(const pstring_t &string) = default;
	pstring_t(pstring_t &&string) noexcept = default;
	pstring_t &operator=(const pstring_t &string) = default;
	pstring_t &operator=(pstring_t &&string) noexcept = default;

	explicit pstring_t(size_type n, code_t code)
	{
		while (n-- != 0)
			*this += code;
	}

	template <typename T,
		class = std::enable_if_t<!std::is_same<T, pstring_t::traits_type>::value>>
	explicit pstring_t(const pstring_t<T> &string)
	{
		m_str.clear();
		for (auto &c : string)
			*this += static_cast<code_t>(c); // FIXME: codepage conversion for u8
	}

	operator string_type () const { return m_str; }

	template <typename T,
		class = std::enable_if_t<!std::is_same<T, pstring_t::traits_type>::value>>
	pstring_t &operator=(const pstring_t<T> &string)
	{
		m_str.clear();
		for (auto &c : string)
			*this += c;
		return *this;
	}

	iterator begin() noexcept { return iterator(m_str.begin()); }
	iterator end() noexcept { return iterator(m_str.end()); }
	const_iterator begin() const noexcept { return const_iterator(m_str.begin()); }
	const_iterator end() const noexcept { return const_iterator(m_str.end()); }
	const_iterator cbegin() const noexcept { return const_iterator(m_str.begin()); }
	const_iterator cend() const noexcept { return const_iterator(m_str.end()); }

	// C string conversion helpers
	const mem_t *c_str() const noexcept  { return static_cast<const mem_t *>(m_str.c_str()); }
	const mem_t *data() const noexcept  { return c_str(); }

	/// \brief return number of codes in the string
	///
	/// This may report a number less than what \ref size reports. pstrings
	/// operate on character codes. In the case of utf pstrings thus the physical size
	/// may be bigger than the logical size.
	size_type length() const noexcept { return traits_type::len(m_str); }

	/// \brief return number of memory units in the string
	///
	/// This function returns the number of memory units used by a string.
	/// Depending on the string type the size may be reported as bytes, words
	/// or quad-words.
	size_type size() const noexcept { return m_str.size(); }

	bool empty() const noexcept { return m_str.empty(); }
	void clear() noexcept { m_str.clear(); }

	pstring_t substr(size_type start, size_type nlen) const;
	pstring_t substr(size_type start) const;
	int compare(const pstring_t &right) const noexcept;

	size_type find(const pstring_t &search, size_type start = 0) const noexcept;
	size_type find(code_t search, size_type start = 0) const noexcept;

	// concatenation operators
	pstring_t& operator+=(const pstring_t &string) { m_str.append(string.m_str); return *this; }
	pstring_t& operator+=(const code_t c) { traits_type::encode(c, m_str); return *this; }
	friend pstring_t operator+(const pstring_t &lhs, const pstring_t &rhs) { return pstring_t(lhs) += rhs; }
	friend pstring_t operator+(const pstring_t &lhs, code_t rhs) { return pstring_t(lhs) += rhs; }
	friend pstring_t operator+(code_t lhs, const pstring_t &rhs) { return pstring_t(1, lhs) += rhs; }

	// comparison operators
	bool operator==(const pstring_t &string) const noexcept { return m_str == string.m_str; }
	bool operator!=(const pstring_t &string) const noexcept { return m_str != string.m_str; }

	bool operator<(const pstring_t &string) const noexcept { return (compare(string) < 0); }
	bool operator<=(const pstring_t &string) const noexcept { return (compare(string) <= 0); }
	bool operator>(const pstring_t &string) const noexcept { return (compare(string) > 0); }
	bool operator>=(const pstring_t &string) const noexcept { return (compare(string) >= 0); }

	friend auto operator<<(std::basic_ostream<typename string_type::value_type> &ostrm, const pstring_t &str) -> std::basic_ostream<typename string_type::value_type> &
	{
		ostrm << str.m_str;
		return ostrm;
	}

	const_reference at(const size_type pos) const { return *reinterpret_cast<const ref_value_type *>(F::nthcode(m_str.c_str(),pos)); } // NOLINT(cppcoreguidelines-pro-type-reinterpret

	static constexpr const size_type npos = static_cast<size_type>(-1);

	// the following are extensions to <string>
	// FIXME: remove those
	size_type mem_t_size() const noexcept { return m_str.size(); }
private:

	string_type m_str;
};

struct pu8_traits
{
	using mem_t = char;
	using code_t = char;
	using string_type = std::string;
	static std::size_t len(const string_type &p) noexcept { return p.size(); }
	static std::size_t codelen(const mem_t *p) noexcept { plib::unused_var(p); return 1; }
	static std::size_t codelen(code_t c) noexcept { plib::unused_var(c); return 1; }
	static code_t code(const mem_t *p) noexcept { return *p; }
	static void encode(const code_t c, string_type &s) { s += static_cast<mem_t>(c); }
	static const mem_t *nthcode(const mem_t *p, std::size_t n) noexcept { return &(p[n]); }
};

// No checking, this may deliver invalid codes


template <std::size_t N, typename CT>
struct putf_traits
{
};

template<typename CT>
struct putf_traits<1, CT>
{
	using mem_t = CT;
	using code_t = char32_t;
	using string_type = std::basic_string<CT>;
	static std::size_t len(const string_type &p) noexcept
	{
		std::size_t ret = 0;
		for (const auto &c : p)
		{
			ret += (!((c & 0xC0) == 0x80)); // NOLINT
		}
		return ret;
	}

	static constexpr std::size_t codelen(const mem_t *p) noexcept
	{
		const auto *p1 = reinterpret_cast<const unsigned char *>(p); // NOLINT(cppcoreguidelines-pro-type-reinterpret
		return ((*p1 & 0x80) == 0x00) ? 1 : // NOLINT
			   ((*p1 & 0xE0) == 0xC0) ? 2 : // NOLINT
			   ((*p1 & 0xF0) == 0xE0) ? 3 : // NOLINT
			   ((*p1 & 0xF8) == 0xF0) ? 4 : // NOLINT
			   1; // Invalid UTF8 code - ignore
	}

	static constexpr std::size_t codelen(code_t c) noexcept
	{
		return (c < 0x00080) ? 1 : // NOLINT
			   (c < 0x00800) ? 2 : // NOLINT
			   (c < 0x10000) ? 3 : // NOLINT
			   4; // U+10000 U+1FFFFF
	}

	static constexpr code_t code(const mem_t *p) noexcept
	{
		const auto *p1 = reinterpret_cast<const unsigned char *>(p); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
		return ((*p1 & 0x80) == 0x00) ? *p1 : // NOLINT
			   ((*p1 & 0xE0) == 0xC0) ? static_cast<code_t>(((p1[0] & 0x3f) << 6) | (p1[1] & 0x3f)) : // NOLINT
			   ((*p1 & 0xF0) == 0xE0) ? static_cast<code_t>(((p1[0] & 0x1f) << 12) | ((p1[1] & 0x3f) << 6) | ((p1[2] & 0x3f) << 0)) : // NOLINT
			   ((*p1 & 0xF8) == 0xF0) ? static_cast<code_t>(((p1[0] & 0x0f) << 18) | ((p1[1] & 0x3f) << 12) | ((p1[2] & 0x3f) << 6)  | ((p1[3] & 0x3f) << 0)) : // NOLINT
			   0xFFFD; // NOLINT: unicode-replacement character
	}

	static void encode(const code_t c, string_type &s)
	{
		if (c < 0x0080) // NOLINT
		{
			s += static_cast<mem_t>(c);
		}
		else if (c < 0x800) // NOLINT
		{
			s += static_cast<mem_t>(0xC0 | (c >> 6)); // NOLINT
			s += static_cast<mem_t>(0x80 | (c & 0x3f)); // NOLINT
		}
		else if (c < 0x10000) // NOLINT
		{
			s += static_cast<mem_t>(0xE0 | (c >> 12)); // NOLINT
			s += static_cast<mem_t>(0x80 | ((c>>6) & 0x3f)); // NOLINT
			s += static_cast<mem_t>(0x80 | (c & 0x3f)); // NOLINT
		}
		else // U+10000 U+1FFFFF
		{
			s += static_cast<mem_t>(0xF0 | (c >> 18)); // NOLINT
			s += static_cast<mem_t>(0x80 | ((c>>12) & 0x3f)); // NOLINT
			s += static_cast<mem_t>(0x80 | ((c>>6) & 0x3f)); // NOLINT
			s += static_cast<mem_t>(0x80 | (c & 0x3f)); // NOLINT
		}
	}

	static const mem_t *nthcode(const mem_t *p, std::size_t n) noexcept
	{
		const mem_t *p1 = p;
		std::size_t i = n;
		while (i-- > 0)
		{
			p1 += codelen(p1);
		}
		return p1;
	}
};

template<typename CT>
struct putf_traits<2, CT>
{
	using mem_t = CT;
	using code_t = char32_t;
	using string_type = std::basic_string<CT>;
	static std::size_t len(const string_type &p) noexcept
	{
		std::size_t ret = 0;
		auto i = p.begin();
		while (i != p.end())
		{
			// FIXME: check that size is equal
			auto c = static_cast<uint16_t>(*i++);
			if (!((c & 0xd800) == 0xd800)) // NOLINT
			{
				ret++;
			}
		}
		return ret;
	}
	static std::size_t codelen(const mem_t *p) noexcept
	{
		auto c = static_cast<uint16_t>(*p);
		return ((c & 0xd800) == 0xd800) ? 2 : 1; // NOLINT
	}
	static std::size_t codelen(code_t c) noexcept
	{
		return (c < 0x10000) ? 1 : 2;  // NOLINT: U+10000 U+1FFFFF
	}
	static code_t code(const mem_t *p) noexcept
	{
		auto c = static_cast<uint32_t>(*p++);
		if ((c & 0xd800) == 0xd800) // NOLINT
		{
			c = (c - 0xd800) << 10; // NOLINT
			c += static_cast<uint32_t>(*p) - 0xdc00 + 0x10000; // NOLINT
		}
		return static_cast<code_t>(c);
	}
	static void encode(code_t c, string_type &s) noexcept
	{
		auto cu = static_cast<uint32_t>(c);
		if (c > 0xffff) // NOLINT
		{ //make a surrogate pair
			uint32_t t = ((cu - 0x10000) >> 10) + 0xd800; // NOLINT
			cu = (cu & 0x3ff) + 0xdc00; // NOLINT
			s += static_cast<mem_t>(t);
			s += static_cast<mem_t>(cu);
		}
		else
		{
			s += static_cast<mem_t>(cu);
		}
	}
	static const mem_t *nthcode(const mem_t *p, std::size_t n) noexcept
	{
		std::size_t i = n;
		while (i-- > 0)
		{
			p += codelen(p);
		}
		return p;
	}
};

template<typename CT>
struct putf_traits<4, CT>
{
	using mem_t = CT;
	using code_t = char32_t;
	using string_type = std::basic_string<CT>;
	static std::size_t len(const string_type &p) noexcept
	{
		return p.size();
	}

	static std::size_t codelen(const mem_t *p) noexcept
	{
		plib::unused_var(p);
		return 1;
	}

	static std::size_t codelen(code_t c) noexcept
	{
		plib::unused_var(c);
		return 1;
	}

	static code_t code(const mem_t *p)
	{
		return static_cast<code_t>(*p);
	}

	static void encode(code_t c, string_type &s)
	{
		s += static_cast<mem_t>(c);
	}
	static const mem_t *nthcode(const mem_t *p, std::size_t n) noexcept
	{
		return p + n;
	}
};

using putf8_traits  = putf_traits<sizeof(char), char>;
using putf16_traits = putf_traits<sizeof(char16_t), char16_t>;
using putf32_traits = putf_traits<sizeof(char32_t), char32_t>;
using pwchar_traits = putf_traits<sizeof(wchar_t), wchar_t>;

extern template struct pstring_t<pu8_traits>;
extern template struct pstring_t<putf8_traits>;
extern template struct pstring_t<putf16_traits>;
extern template struct pstring_t<putf32_traits>;
extern template struct pstring_t<pwchar_traits>;

#if (PSTRING_USE_STD_STRING)
using pstring = std::string;
#else
using pstring = pstring_t<putf8_traits>;
#endif
using pu8string = pstring_t<pu8_traits>;
using putf8string = pstring_t<putf8_traits>;
using putf16string = pstring_t<putf16_traits>;
using putf32string = pstring_t<putf32_traits>;
using pwstring = pstring_t<pwchar_traits>;

// interpret other string as putf8strings
template <typename F>
template <typename C, class>
pstring_t<F>::pstring_t(C *string)
{
	m_str.clear();
	putf8string utf8(string);
	for (const auto &c : utf8)
		*this += c;
}

namespace plib
{
	template<class T>
	struct string_info
	{
	};

	template<typename T>
	struct string_info<pstring_t<T>>
	{
		using mem_t = typename T::mem_t;
#if 0
		static std::size_t mem_size(const pstring_t<T> &s) { return s.mem_t_size(); }
#endif
	};

	template<typename T>
	struct string_info<std::basic_string<T>>
	{
		using mem_t = T;
#if 0
		static std::size_t mem_size(const std::basic_string<T> &s) { return s.size(); }
#endif
	};
} // namespace plib

// custom specialization of std::hash can be injected in namespace std
namespace std
{
	template<typename T>
	struct hash<pstring_t<T>>
	{
		using argument_type = pstring_t<T>;
		using result_type = std::size_t;
		result_type operator()(const argument_type & s) const
		{
			const typename argument_type::mem_t *string = s.c_str();
			result_type result = 5381; // NOLINT
			for (typename argument_type::mem_t c = *string; c != 0; c = *string++)
				result = ((result << 5) + result ) ^ (result >> (32 - 5)) ^ static_cast<result_type>(c); // NOLINT
			return result;
		}
	};
} // namespace std

#endif // PSTRING_H_
