// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    corealloc.h

    Memory allocation helpers for the helper library.

***************************************************************************/

#pragma once

#ifndef MAME_LIB_UTIL_COREALLOC_H
#define MAME_LIB_UTIL_COREALLOC_H

#include "osdcore.h"

#include <stdlib.h>

#include <cstddef>
#include <cstring>
#include <new>
#include <memory>
#include <type_traits>
#include <utility>



//**************************************************************************
//  MACROS
//**************************************************************************

// global allocation helpers -- use these instead of new and delete
#define global_alloc(Type)                          new Type
#define global_alloc_array(Type, Num)               new Type[Num]
#define global_free(Ptr)                            do { delete Ptr; } while (0)
#define global_free_array(Ptr)                      do { delete[] Ptr; } while (0)



template<typename T, typename... Params>
inline T* global_alloc_clear(Params &&... args)
{
	void *const ptr = ::operator new(sizeof(T)); // allocate memory
	std::memset(ptr, 0, sizeof(T));
	return new(ptr) T(std::forward<Params>(args)...);
}

template<typename T>
inline T* global_alloc_array_clear(std::size_t num)
{
	auto const size = sizeof(T) * num;
	void *const ptr = new unsigned char[size]; // allocate memory
	std::memset(ptr, 0, size);
	return new(ptr) T[num]();
}



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
