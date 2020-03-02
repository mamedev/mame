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

// noexcept on move operator -> issue with macosx clang
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

	// 128 bit support at least on GCC is not fully supported
#if PHAS_INT128
	template<> struct is_integral<UINT128> { static constexpr bool value = true; };
	template<> struct is_integral<INT128> { static constexpr bool value = true; };
	template<> struct numeric_limits<UINT128>
	{
		static constexpr UINT128 max() noexcept
		{
			return ~((UINT128)0);
		}
	};
	template<> struct numeric_limits<INT128>
	{
		static constexpr INT128 max() noexcept
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
		nocopyassignmove(nocopyassignmove &&) noexcept = delete;
		nocopyassignmove &operator=(const nocopyassignmove &) = delete;
		nocopyassignmove &operator=(nocopyassignmove &&) noexcept = delete;
	protected:
		nocopyassignmove() = default;
		~nocopyassignmove() noexcept = default;
	};

	struct nocopyassign
	{
		nocopyassign(const nocopyassign &) = delete;
		nocopyassign &operator=(const nocopyassign &) = delete;
	protected:
		nocopyassign() = default;
		~nocopyassign() noexcept = default;
		nocopyassign(nocopyassign &&) noexcept = default;
		nocopyassign &operator=(nocopyassign &&) noexcept = default;
	};

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
