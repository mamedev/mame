// license:BSD-3-Clause
// copyright-holders:Couriersud

///
/// \file test_penum.cpp
///
/// tests for penum
///

#include "plib/putil.h"

#include "plib/pexception.h"
#include "plib/pstring.h"

#include <string>
#include <type_traits>
#include <string_view>

#if 0
namespace plib
{
	template <typename E>
	struct functor { using is_defined = std::false_type; };
	template <std::size_t N, typename E>
	struct functor_base
	{
	public:
		using is_defined = std::true_type;
		using size = std::integral_constant<std::size_t, N>;
		constexpr operator E () const noexcept { return m_v; }
		constexpr functor_base(const E v) noexcept : m_v(v) { };
	protected:
		constexpr functor_base(const char *s, const char * const vals[size::value]) noexcept : m_v(E::PENUM_UNKNOWN)
		{ for (std::size_t i = 0; i < size::value; i++) if (vals[i] == s) { m_v = static_cast<E>(i); return; }  }
		E m_v;
	private:
		//static constexpr const char *m_str[PNARGS(__VA_ARGS__)] = { PSTRINGIFY_VA(__VA_ARGS__) };
	};

} // namespace plib

#define PENUM(ename, ...) \
	enum class ename { __VA_ARGS__ , PENUM_UNKNOWN }; \
	PENUM_FUNCTOR(ename, __VA_ARGS__)

#define PENUM_NS(ns, ename, ...) \
	namespace ns { enum class ename { __VA_ARGS__ , PENUM_UNKNOWN }; } \
	PENUM_FUNCTOR(ns :: ename, __VA_ARGS__)

#define PENUM_FUNCTOR(ename, ...) \
	template <> struct plib::functor<ename> : plib::functor_base<PNARGS(__VA_ARGS__), ename> \
	{ \
	public: \
		using plib::functor_base<size::value, ename>::functor_base; \
		constexpr functor(const char  *s) noexcept : functor_base(s, m_str) { } \
		constexpr operator const char * () const noexcept { return m_str[static_cast<std::size_t>(m_v)]; } \
	private: \
		static constexpr const char *m_str[3] = { PSTRINGIFY_VA(__VA_ARGS__) }; \
	};

#else
#if 1
namespace plib::enum_static
{
	template <typename E>
	struct functor_static
	{
		using is_defined = std::false_type;
		using size = std::integral_constant<std::size_t, 0>;
		static constexpr const char *m_str[1] = { "" };
	};
} // namespace plib::enum_static

#endif
namespace plib
{
	template <typename E>
	struct functor
	{
	public:
		using staticf = enum_static::functor_static<E>;
		using is_defined = typename staticf::is_defined;
		using size = typename staticf::size;
		constexpr functor(const E v) noexcept : m_v(v) { }
		constexpr functor(const char *s) noexcept : m_v(E::PENUM_UNKNOWN)
		{
			for (std::size_t i = 0; i < size::value; i++)
				if (staticf::m_str[i] == s)
				{
					m_v = static_cast<E>(i);
					return;
				}
		}
		constexpr operator const char * () const noexcept { return staticf::m_str[static_cast<std::size_t>(m_v)]; } \
		constexpr operator E () const noexcept { return m_v; }
	protected:
		E m_v;
	private:
		//static constexpr const char *m_str[PNARGS(__VA_ARGS__)] = { PSTRINGIFY_VA(__VA_ARGS__) };
	};

	// template deduction guide
	template<typename E> functor(E) -> functor<E>;

	template<typename E>
	static inline typename std::enable_if<plib::enum_static::functor_static<E>::is_defined::value, const char *>::type
	penum_to_string(E e)
	{
		using staticf = enum_static::functor_static<E>;
		return staticf::m_str[static_cast<std::size_t>(e)];
	}

	template<typename E>
	static inline typename std::enable_if<plib::enum_static::functor_static<E>::is_defined::value, E>::type
	string_to_penum(const char *s)
	{
		using staticf = enum_static::functor_static<E>;
		for (std::size_t i = 0; i < staticf::size::value; i++)
			if (staticf::m_str[i] == s)
				return static_cast<E>(i);
		return E::PENUM_UNKNOWN;
	}

} // namespace plib

#define PENUM(ename, ...) \
	enum class ename { __VA_ARGS__ , PENUM_UNKNOWN }; \
	PENUM_FUNCTOR(ename, __VA_ARGS__)

#define PENUM_NS(ns, ename, ...) \
	namespace ns { enum class ename { __VA_ARGS__ , PENUM_UNKNOWN }; } \
	PENUM_FUNCTOR(ns :: ename, __VA_ARGS__)

#define PENUM_FUNCTOR(ename, ...) \
	namespace plib::enum_static { \
	template <> struct functor_static<ename> \
	{ \
	public: \
		using is_defined = std::true_type; \
		using size = std::integral_constant<std::size_t, PNARGS(__VA_ARGS__) >; \
		static constexpr const char *m_str[size::value] = { PSTRINGIFY_VA(__VA_ARGS__) }; \
	}; \
	} // namespace plib::enum_static

#endif

template <class O, class E>
typename std::enable_if<!plib::has_ostream_operator<O, E>::value && plib::enum_static::functor_static<E>::is_defined::value, O&>::type
operator << (O& os, const E &p)
{
	os << static_cast<const char *>(plib::functor<E>(p));
	return os;
}

#if 1
template <typename E>
inline typename std::enable_if<plib::enum_static::functor_static<E>::is_defined::value, bool>::type
operator == (const char * lhs, const E &rhs)
{
	return lhs == plib::functor<E>(rhs);
}
template <typename E>
inline typename std::enable_if<plib::enum_static::functor_static<E>::is_defined::value, bool>::type
operator == (const E &lhs, const char * rhs)
{
	return lhs == plib::functor<E>(rhs);
}
#endif

// ---------------------------------------

template <typename T>
struct ret_t
{
	bool success;
	T value;
};

template <typename T, T BASE>
constexpr ret_t<T> stl_h(std::string_view sv)
{
	T r(0);
	for (const auto &c : sv)
	{
		if (c >= '0' && c < '0' + std::min(10,BASE))
		{
			r = r * BASE + (c - '0');
		}
		else if (c >= 'A' && c < ('A' + BASE - 10))
		{
			r = r * BASE + (10 + c - 'A');
		}
		else if (c >= 'a' && c < ('a' + BASE - 10))
		{
			r = r * BASE + (10 + c - 'a');
		}
		else
			return { false, T(0) };
	}
	return { true, r };
}

template <typename T, T BASE = 0>
constexpr ret_t<T> stl(std::string_view sv)
{
	if constexpr (BASE == 0)
	{
		if (sv.size() > 2)
		{
			if (sv.substr(0,2) == "0x" || sv.substr(0,2) == "0X")
					return stl_h<T,16>(sv.substr(2));
			if (sv.substr(0,2) == "0b")
					return stl_h<T,2>(sv.substr(2));
		}
		if (sv.size() > 1 && sv.substr(0,1) == "0" )
			return stl_h<T,8>(sv.substr(1));
		return stl_h<T, 10>(sv);
	}
	return stl_h<T, BASE>(sv);
}

#include "plib/ptests.h"

//PENUM_NS(plibx, teste, A, B, C)

namespace plibx { enum class teste; }
PENUM(plibx::teste, A, B, C)

enum class pure { X, Y };

PENUM(testf, A, B, C)

PTEST(penum, conversion)
{
	static_assert(stl<int,0>("0xa0bc").success,"no");
	static_assert(stl<int,0>("0xa0bc").value==0xa0bc,"no");
	static_assert(stl<int,0>("0b1010").value==0b1010,"no");
	static_assert(stl<int,0>("0123").value==0123,"no");
	//const auto x(stl<int,0>("123"));
	//static_assert(x.value==123,"no");

	plibx::teste x = plib::functor<plibx::teste>("A");
	PEXPECT_NE(x, plibx::teste::B);
	PEXPECT_NE(plib::functor<plibx::teste>("A"), plibx::teste::B);
	PEXPECT_NE(plib::functor<testf>("A"), testf::B);
	//PEXPECT_EQ(plib::functor("A"), testf::B);
	PEXPECT_EQ(testf::B, "B");
	PEXPECT_EQ("A", testf::A);
	PEXPECT_NE(plib::functor(testf::A), "B");
	PEXPECT_FALSE(plib::functor<pure>::is_defined::value);
	PEXPECT_EQ(plib::penum_to_string(testf::B), "B");
	PEXPECT_NE(plib::string_to_penum<testf>("B"), testf::A);
	//PEXPECT_EQ(pure::X, pure::Y);
}
