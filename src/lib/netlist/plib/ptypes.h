// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * ptypes.h
 *
 */

#ifndef PTYPES_H_
#define PTYPES_H_

#include <limits>
#include <string>
#include <type_traits>

#include "pconfig.h"

#define COPYASSIGNMOVE(name, def)  \
		name(const name &) = def; \
		name(name &&) noexcept = def; \
		name &operator=(const name &) = def; \
		name &operator=(name &&) noexcept = def;

#define COPYASSIGN(name, def)  \
		name(const name &) = def; \
		name &operator=(const name &) = def; \

namespace plib
{
	template<typename T> struct is_integral : public std::is_integral<T> { };
	template<typename T> struct numeric_limits : public std::numeric_limits<T> { };

	/* 128 bit support at least on GCC is not fully supported */
#if PHAS_INT128
	template<> struct is_integral<UINT128> { static constexpr bool value = true; };
	template<> struct is_integral<INT128> { static constexpr bool value = true; };
	template<> struct numeric_limits<UINT128>
	{
		static constexpr UINT128 max()
		{
			return ~((UINT128)0);
		}
	};
	template<> struct numeric_limits<INT128>
	{
		static constexpr INT128 max()
		{
			return (~((UINT128)0)) >> 1;
		}
	};
#endif

	//============================================================
	// prevent implicit copying
	//============================================================

	struct nocopyassignmove
	{
		nocopyassignmove(const nocopyassignmove &) = delete;
		nocopyassignmove(nocopyassignmove &&) = delete;
		nocopyassignmove &operator=(const nocopyassignmove &) = delete;
		nocopyassignmove &operator=(nocopyassignmove &&) = delete;
	protected:
		nocopyassignmove() = default;
		~nocopyassignmove() = default;
	};

	struct nocopyassign
	{
		nocopyassign(const nocopyassign &) = delete;
		nocopyassign &operator=(const nocopyassign &) = delete;
	protected:
		nocopyassign() = default;
		~nocopyassign() = default;
		nocopyassign(nocopyassign &&) = default;
		nocopyassign &operator=(nocopyassign &&) = default;
	};

	//============================================================
	// Avoid unused variable warnings
	//============================================================
	template<typename... Ts>
	inline void unused_var(Ts&&...) {}

	//============================================================
	// is_pow2
	//============================================================
	template <typename T>
	constexpr bool is_pow2(T v) noexcept
	{
		static_assert(is_integral<T>::value, "is_pow2 needs integer arguments");
		return !(v & (v-1));
	}


	//============================================================
	// abs, lcd, gcm
	//============================================================

	template<typename T>
	constexpr
	typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value, T>::type
	abs(T v)
	{
		return v < 0 ? -v : v;
	}

	template<typename T>
	constexpr
	typename std::enable_if<std::is_integral<T>::value && std::is_unsigned<T>::value, T>::type
	abs(T v)
	{
		return v;
	}

	template<typename M, typename N>
	constexpr typename std::common_type<M, N>::type
	gcd(M m, N n)
	{
		static_assert(std::is_integral<M>::value, "gcd: M must be an integer");
		static_assert(std::is_integral<N>::value, "gcd: N must be an integer");

		return m == 0 ? plib::abs(n)
		     : n == 0 ? plib::abs(m)
		     : gcd(n, m % n);
	}

	template<typename M, typename N>
	constexpr typename std::common_type<M, N>::type
	lcm(M m, N n)
	{
		static_assert(std::is_integral<M>::value, "lcm: M must be an integer");
		static_assert(std::is_integral<N>::value, "lcm: N must be an integer");

		return (m != 0 && n != 0) ? (plib::abs(m) / gcd(m, n)) * plib::abs(n) : 0;
	}

} // namespace plib

//============================================================
// Define a "has member" trait.
//============================================================

#define PDEFINE_HAS_MEMBER(name, member)                                        \
	template <typename T> class name                                            \
	{                                                                           \
		template <typename U> static long test(decltype(&U:: member));          \
		template <typename U> static char  test(...);                           \
	public:                                                                     \
		static constexpr const bool value = sizeof(test<T>(nullptr)) == sizeof(long);   \
	}

#endif /* PTYPES_H_ */
