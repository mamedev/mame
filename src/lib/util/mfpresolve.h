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
#include <utility>


namespace util {

namespace detail {

std::pair<std::uintptr_t, std::uintptr_t> resolve_member_function_itanium(std::uintptr_t function, std::ptrdiff_t delta, void const *object) noexcept;
std::pair<std::uintptr_t, std::uintptr_t> resolve_member_function_msvc(void const *funcptr, std::size_t size, void const *object) noexcept;
std::uintptr_t bypass_member_function_thunks(std::uintptr_t entrypoint, void const *object) noexcept;

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
		struct { std::uintptr_t ptr; std::ptrdiff_t adj; } equiv;
		assert(sizeof(function) == sizeof(equiv));
		*reinterpret_cast<decltype(function) *>(&equiv) = function;
		return detail::resolve_member_function_itanium(equiv.ptr, equiv.adj, &object);
	}
	else if (MAME_ABI_CXX_TYPE == MAME_ABI_CXX_MSVC)
	{
		return detail::resolve_member_function_msvc(&function, sizeof(function), &object);
	}
	else
	{
		return std::make_pair(std::uintptr_t(nullptr), std::uintptr_t(nullptr));
	}
}


template <typename T, typename Ret, typename... Params>
inline std::pair<std::uintptr_t, std::uintptr_t> resolve_member_function(Ret (T::*function)(Params...) const, T const &object) noexcept
{
	if (MAME_ABI_CXX_TYPE == MAME_ABI_CXX_ITANIUM)
	{
		struct { std::uintptr_t ptr; std::ptrdiff_t adj; } equiv;
		assert(sizeof(function) == sizeof(equiv));
		*reinterpret_cast<decltype(function) *>(&equiv) = function;
		return detail::resolve_member_function_itanium(equiv.ptr, equiv.adj, &object);
	}
	else if (MAME_ABI_CXX_TYPE == MAME_ABI_CXX_MSVC)
	{
		return detail::resolve_member_function_msvc(&function, sizeof(function), &object);
	}
	else
	{
		return std::make_pair(std::uintptr_t(nullptr), std::uintptr_t(nullptr));
	}
}

} // namespace util

#endif // MAME_LIB_UTIL_MFPRESOLVE_H
