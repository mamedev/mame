// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    corealloc.h

    Memory allocation helpers for the helper library.

***************************************************************************/

#pragma once

#ifndef __COREALLOC_H__
#define __COREALLOC_H__

#include <stdlib.h>
#include <new>
#include <type_traits>
#include <utility>
#include <memory>
#include "osdcore.h"


//**************************************************************************
//  MACROS
//**************************************************************************

// global allocation helpers -- use these instead of new and delete
#define global_alloc(_type)                         new _type
#define global_alloc_array(_type, _num)             new _type[_num]
#define global_free(_ptr)                           do { delete _ptr; } while (0)
#define global_free_array(_ptr)                     do { delete[] _ptr; } while (0)



template<typename _Tp, typename... _Args>
inline _Tp* global_alloc_clear(_Args&&... __args)
{ 
	unsigned char * ptr = new unsigned char[sizeof(_Tp)]; // allocate memory
	memset(ptr, 0, sizeof(_Tp));
	return new(ptr) _Tp(std::forward<_Args>(__args)...); 
}

template<typename _Tp>
inline _Tp* global_alloc_array_clear(size_t __num)
{ 
	auto size = sizeof(_Tp) * __num;
	unsigned char* ptr = new unsigned char[size]; // allocate memory
	memset(ptr, 0, size);
	return new(ptr) _Tp[__num]();
}



template<typename _Tp>
struct _MakeUniqClear
{
	typedef std::unique_ptr<_Tp> __single_object;
};

template<typename _Tp>
struct _MakeUniqClear<_Tp[]>
{
	typedef std::unique_ptr<_Tp[]> __array;
};

template<typename _Tp, size_t _Bound>
struct _MakeUniqClear<_Tp[_Bound]>
{
	struct __invalid_type { };
};

/// make_unique_clear for single objects
template<typename _Tp, typename... _Args>
inline typename _MakeUniqClear<_Tp>::__single_object make_unique_clear(_Args&&... __args)
{
	unsigned char* ptr = new unsigned char[sizeof(_Tp)]; // allocate memory
	memset(ptr, 0, sizeof(_Tp));
	return std::unique_ptr<_Tp>(new(ptr) _Tp(std::forward<_Args>(__args)...));
}

/// make_unique_clear for arrays of unknown bound
template<typename _Tp>
inline typename _MakeUniqClear<_Tp>::__array make_unique_clear(size_t __num)
{
	auto size = sizeof(std::remove_extent_t<_Tp>) * __num;
	unsigned char* ptr = new unsigned char[size]; // allocate memory
	memset(ptr, 0, size);
	return std::unique_ptr<_Tp>(new(ptr) std::remove_extent_t<_Tp>[__num]());
}

template<typename _Tp, unsigned char _F>
inline typename _MakeUniqClear<_Tp>::__array make_unique_clear(size_t __num)
{
	auto size = sizeof(std::remove_extent_t<_Tp>) * __num;
	unsigned char* ptr = new unsigned char[size]; // allocate memory
	memset(ptr, _F, size);
	return std::unique_ptr<_Tp>(new(ptr) std::remove_extent_t<_Tp>[__num]());
}

/// Disable make_unique_clear for arrays of known bound
template<typename _Tp, typename... _Args>
inline typename _MakeUniqClear<_Tp>::__invalid_type make_unique_clear(_Args&&...) = delete;

#endif  /* __COREALLOC_H__ */
