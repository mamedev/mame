// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    mfpresolve.h

    Helpers for resolving member function pointers to entry points.

***************************************************************************/
#ifndef MAME_LIB_UTIL_MFPRESOLVE_H
#define MAME_LIB_UTIL_MFPRESOLVE_H

#pragma once

#include "abi.h"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <tuple>
#include <utility>


namespace util {

namespace detail {

struct mfp_itanium_equiv
{
	std::uintptr_t function;
	std::ptrdiff_t delta;

	constexpr std::ptrdiff_t this_delta() const noexcept { return delta >> ((MAME_ABI_CXX_ITANIUM_MFP_TYPE == MAME_ABI_CXX_ITANIUM_MFP_ARM) ? 1 : 0); }
	constexpr bool is_virtual() const noexcept { return ((MAME_ABI_CXX_ITANIUM_MFP_TYPE == MAME_ABI_CXX_ITANIUM_MFP_ARM) ? delta : function) & 1; }
};

struct mfp_msvc_single_equiv { std::uintptr_t entrypoint; };
struct mfp_msvc_multi_equiv { std::uintptr_t entrypoint; int delta; };
struct mfp_msvc_virtual_equiv { std::uintptr_t entrypoint; int delta; int vindex; };
struct mfp_msvc_unknown_equiv { std::uintptr_t entrypoint; int delta; int voffset; int vindex; };

std::pair<std::uintptr_t, std::uintptr_t> resolve_member_function_itanium(std::uintptr_t function, std::ptrdiff_t delta, void const *object) noexcept;
std::tuple<std::uintptr_t, std::ptrdiff_t, bool> resolve_member_function_itanium(std::uintptr_t function, std::ptrdiff_t delta) noexcept;
std::pair<std::uintptr_t, std::uintptr_t> resolve_member_function_msvc(void const *funcptr, std::size_t size, void const *object) noexcept;
std::tuple<std::uintptr_t, std::ptrdiff_t, bool> resolve_member_function_msvc(void const *funcptr, std::size_t size) noexcept;
std::uintptr_t bypass_member_function_thunks(std::uintptr_t entrypoint, void const *object) noexcept;
std::pair<std::uintptr_t, bool> bypass_member_function_thunks(std::uintptr_t entrypoint) noexcept;

} // namespace detail


template <typename T, typename U>
inline T bypass_member_function_thunks(T entrypoint, U const *object) noexcept
{
	return reinterpret_cast<T>(
			detail::bypass_member_function_thunks(
				reinterpret_cast<std::uintptr_t>(entrypoint),
				reinterpret_cast<void const *>(object)));
}


template <typename T, typename Ret, typename... Params>
inline std::pair<std::uintptr_t, std::uintptr_t> resolve_member_function(Ret (T::*function)(Params...), T &object) noexcept
{
	if (MAME_ABI_CXX_TYPE == MAME_ABI_CXX_ITANIUM)
	{
		detail::mfp_itanium_equiv equiv;
		assert(sizeof(function) == sizeof(equiv));
		*reinterpret_cast<decltype(function) *>(&equiv) = function;
		return detail::resolve_member_function_itanium(equiv.function, equiv.delta, &object);
	}
	else if (MAME_ABI_CXX_TYPE == MAME_ABI_CXX_MSVC)
	{
		return detail::resolve_member_function_msvc(&function, sizeof(function), &object);
	}
	else
	{
		return std::make_pair(std::uintptr_t(static_cast<void (*)()>(nullptr)), std::uintptr_t(nullptr));
	}
}


template <typename T, typename Ret, typename... Params>
inline std::pair<std::uintptr_t, std::uintptr_t> resolve_member_function(Ret (T::*function)(Params...) const, T const &object) noexcept
{
	if (MAME_ABI_CXX_TYPE == MAME_ABI_CXX_ITANIUM)
	{
		detail::mfp_itanium_equiv equiv;
		assert(sizeof(function) == sizeof(equiv));
		*reinterpret_cast<decltype(function) *>(&equiv) = function;
		return detail::resolve_member_function_itanium(equiv.function, equiv.delta, &object);
	}
	else if (MAME_ABI_CXX_TYPE == MAME_ABI_CXX_MSVC)
	{
		return detail::resolve_member_function_msvc(&function, sizeof(function), &object);
	}
	else
	{
		return std::make_pair(std::uintptr_t(static_cast<void (*)()>(nullptr)), std::uintptr_t(nullptr));
	}
}


template <typename T, typename Ret, typename... Params>
inline std::tuple<std::uintptr_t, std::ptrdiff_t, bool> resolve_member_function(Ret (T::*function)(Params...)) noexcept
{
	if (MAME_ABI_CXX_TYPE == MAME_ABI_CXX_ITANIUM)
	{
		detail::mfp_itanium_equiv equiv;
		assert(sizeof(function) == sizeof(equiv));
		*reinterpret_cast<decltype(function) *>(&equiv) = function;
		return detail::resolve_member_function_itanium(equiv.function, equiv.delta);
	}
	else if (MAME_ABI_CXX_TYPE == MAME_ABI_CXX_MSVC)
	{
		return detail::resolve_member_function_msvc(&function, sizeof(function));
	}
	else
	{
		return std::make_tuple(std::uintptr_t(static_cast<void (*)()>(nullptr)), std::ptrdiff_t(0), false);
	}
}


template <typename T, typename Ret, typename... Params>
inline std::tuple<std::uintptr_t, std::ptrdiff_t, bool> resolve_member_function(Ret (T::*function)(Params...) const) noexcept
{
	if (MAME_ABI_CXX_TYPE == MAME_ABI_CXX_ITANIUM)
	{
		detail::mfp_itanium_equiv equiv;
		assert(sizeof(function) == sizeof(equiv));
		*reinterpret_cast<decltype(function) *>(&equiv) = function;
		return detail::resolve_member_function_itanium(equiv.function, equiv.delta);
	}
	else if (MAME_ABI_CXX_TYPE == MAME_ABI_CXX_MSVC)
	{
		return detail::resolve_member_function_msvc(&function, sizeof(function));
	}
	else
	{
		return std::make_tuple(std::uintptr_t(static_cast<void (*)()>(nullptr)), std::ptrdiff_t(0), false);
	}
}

} // namespace util

#endif // MAME_LIB_UTIL_MFPRESOLVE_H
