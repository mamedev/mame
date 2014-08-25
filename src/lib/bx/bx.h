/*
 * Copyright 2010-2013 Branimir Karadzic. All rights reserved.
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

} // namespace bx

// Annoying C++0x stuff..
namespace std { namespace tr1 {}; using namespace tr1; }

#endif // BX_H_HEADER_GUARD
