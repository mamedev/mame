// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef PTYPES_H_
#define PTYPES_H_

///
/// \file ptypes.h
///

#include "pconfig.h"

#include <limits>
#include <string>
#include <type_traits>

#if (PUSE_FLOAT128)
#include <quadmath.h>
#endif

// noexcept on move operator -> issue with macosx clang
#define PCOPYASSIGNMOVE(name, def)  \
		name(const name &) = def; \
		name(name &&) noexcept = def; \
		name &operator=(const name &) = def; \
		name &operator=(name &&) noexcept = def;

#define PCOPYASSIGN(name, def)  \
		name(const name &) = def; \
		name &operator=(const name &) = def; \

#define PMOVEASSIGN(name, def)  \
		name(name &&) noexcept = def; \
		name &operator=(name &&) noexcept = def;

namespace plib
{
	template<typename T> struct is_integral : public std::is_integral<T> { };
	template<typename T> struct is_signed : public std::is_signed<T> { };
	template<typename T> struct is_unsigned : public std::is_unsigned<T> { };
	template<typename T> struct numeric_limits : public std::numeric_limits<T> { };

	// 128 bit support at least on GCC is not fully supported
#if PHAS_INT128
	template<> struct is_integral<UINT128> { static constexpr bool value = true; };
	template<> struct is_integral<INT128> { static constexpr bool value = true; };

	template<> struct is_signed<UINT128> { static constexpr bool value = false; };
	template<> struct is_signed<INT128> { static constexpr bool value = true; };

	template<> struct is_unsigned<UINT128> { static constexpr bool value = true; };
	template<> struct is_unsigned<INT128> { static constexpr bool value = false; };

	template<> struct numeric_limits<UINT128>
	{
		static constexpr UINT128 max() noexcept
		{
			return ~(static_cast<UINT128>(0));
		}
	};
	template<> struct numeric_limits<INT128>
	{
		static constexpr INT128 max() noexcept
		{
			return (~static_cast<UINT128>(0)) >> 1;
		}
	};
#endif

	template<typename T> struct is_floating_point : public std::is_floating_point<T> { };

	template< class T >
	struct is_arithmetic : std::integral_constant<bool,
		plib::is_integral<T>::value || plib::is_floating_point<T>::value> {};

#if PUSE_FLOAT128
	template<> struct is_floating_point<FLOAT128> { static constexpr bool value = true; };
	template<> struct numeric_limits<FLOAT128>
	{
		static constexpr FLOAT128 max() noexcept
		{
			return FLT128_MAX;
		}
		static constexpr FLOAT128 lowest() noexcept
		{
			return -FLT128_MAX;
		}
	};
#endif

	//============================================================
	// Avoid unused variable warnings
	//============================================================
	template<typename... Ts>
	inline void unused_var(Ts&&...) noexcept {} // NOLINT(readability-named-parameter)

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

#endif // PTYPES_H_
