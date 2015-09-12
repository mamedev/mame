/*
 * Copyright 2010-2015 Branimir Karadzic. All rights reserved.
 * License: http://www.opensource.org/licenses/BSD-2-Clause
 */

#ifndef BX_H_HEADER_GUARD
#define BX_H_HEADER_GUARD

#include <stdint.h> // uint32_t
#include <stdlib.h> // size_t

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

} // namespace bx

// Annoying C++0x stuff..
namespace std { namespace tr1 {}; using namespace tr1; }

#endif // BX_H_HEADER_GUARD
