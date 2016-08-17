/*
 * Copyright 2010-2016 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_H_HEADER_GUARD
#define BX_H_HEADER_GUARD

#include <stdint.h> // uint32_t
#include <stdlib.h> // size_t
#include <string.h> // memcpy

#include "config.h"
#include "macros.h"

namespace bx
{
	// http://cnicholson.net/2011/01/stupid-c-tricks-a-better-sizeof_array/
	template<typename T, size_t N> char (&COUNTOF_REQUIRES_ARRAY_ARGUMENT(const T(&)[N]) )[N];
#define BX_COUNTOF(_x) sizeof(bx::COUNTOF_REQUIRES_ARRAY_ARGUMENT(_x) )

	// Template for avoiding MSVC: C4127: conditional expression is constant
	template<bool>
	inline bool isEnabled()
	{
		return true;
	}

	template<>
	inline bool isEnabled<false>()
	{
		return false;
	}
#define BX_ENABLED(_x) bx::isEnabled<!!(_x)>()

	inline bool ignoreC4127(bool _x)
	{
		return _x;
	}
#define BX_IGNORE_C4127(_x) bx::ignoreC4127(!!(_x) )

	template<typename Ty>
	inline void xchg(Ty& _a, Ty& _b)
	{
		Ty tmp = _a; _a = _b; _b = tmp;
	}

	/// Check if pointer is aligned. _align must be power of two.
	inline bool isPtrAligned(const void* _ptr, size_t _align)
	{
		union { const void* ptr; size_t addr; } un;
		un.ptr = _ptr;
		return 0 == (un.addr & (_align-1) );
	}

	/// Scatter/gather memcpy.
	inline void memCopy(void* _dst, const void* _src, uint32_t _size, uint32_t _num, uint32_t _srcPitch, uint32_t _dstPitch)
	{
		const uint8_t* src = (const uint8_t*)_src;
		uint8_t* dst = (uint8_t*)_dst;

		for (uint32_t ii = 0; ii < _num; ++ii)
		{
			memcpy(dst, src, _size);
			src += _srcPitch;
			dst += _dstPitch;
		}
	}

	///
	inline void gather(void* _dst, const void* _src, uint32_t _size, uint32_t _num, uint32_t _srcPitch)
	{
		memCopy(_dst, _src, _size, _num, _srcPitch, _size);
	}

	///
	inline void scatter(void* _dst, const void* _src, uint32_t _size, uint32_t _num, uint32_t _dstPitch)
	{
		memCopy(_dst, _src, _size, _num, _size, _dstPitch);
	}

} // namespace bx

// Annoying C++0x stuff..
namespace std { namespace tr1 {}; using namespace tr1; }

#endif // BX_H_HEADER_GUARD
