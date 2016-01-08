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
	UINT8* ptr = new UINT8[sizeof(_Tp)]; // allocate memory
	memset(ptr, 0, sizeof(_Tp));
	return new(ptr) _Tp(std::forward<_Args>(__args)...); 
}

template<typename _Tp>
inline _Tp* global_alloc_array_clear(size_t __num)
{ 
	auto size = sizeof(_Tp) * __num;
	UINT8* ptr = new UINT8[size]; // allocate memory
	memset(ptr, 0, size);
	return new(ptr) _Tp[__num]();
}


#endif  /* __COREALLOC_H__ */
