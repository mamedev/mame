// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    corealloc.h

    Memory allocation helpers for the helper library.

***************************************************************************/

#ifndef MAME_LIB_UTIL_COREALLOC_H
#define MAME_LIB_UTIL_COREALLOC_H

#pragma once

#include "osdcore.h"

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>



// global allocation helpers

template<typename Tp> struct MakeUniqClearT { typedef std::unique_ptr<Tp> single_object; };

template<typename Tp> struct MakeUniqClearT<Tp[]> { typedef std::unique_ptr<Tp[]> array; };

template<typename Tp, size_t Bound> struct MakeUniqClearT<Tp[Bound]> { struct invalid_type { }; };

/// make_unique_clear for single objects
template<typename Tp, typename... Params>
inline typename MakeUniqClearT<Tp>::single_object make_unique_clear(Params&&... args)
{
	void *const ptr = ::operator new(sizeof(Tp)); // allocate memory
	std::memset(ptr, 0, sizeof(Tp));
	return std::unique_ptr<Tp>(new(ptr) Tp(std::forward<Params>(args)...));
}

/// make_unique_clear for arrays of unknown bound
template<typename Tp>
inline typename MakeUniqClearT<Tp>::array make_unique_clear(size_t num)
{
	auto size = sizeof(std::remove_extent_t<Tp>) * num;
	unsigned char* ptr = new unsigned char[size]; // allocate memory
	std::memset(ptr, 0, size);
	return std::unique_ptr<Tp>(new(ptr) std::remove_extent_t<Tp>[num]());
}

template<typename Tp, unsigned char F>
inline typename MakeUniqClearT<Tp>::array make_unique_clear(size_t num)
{
	auto size = sizeof(std::remove_extent_t<Tp>) * num;
	unsigned char* ptr = new unsigned char[size]; // allocate memory
	std::memset(ptr, F, size);
	return std::unique_ptr<Tp>(new(ptr) std::remove_extent_t<Tp>[num]());
}

/// Disable make_unique_clear for arrays of known bound
template<typename Tp, typename... Params>
inline typename MakeUniqClearT<Tp>::invalid_type make_unique_clear(Params&&...) = delete;

#endif  // MAME_LIB_UTIL_COREALLOC_H
