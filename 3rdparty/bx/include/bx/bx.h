/*
 * Copyright 2010-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_H_HEADER_GUARD
#define BX_H_HEADER_GUARD

#include <alloca.h> // alloca
#include <stdarg.h> // va_list
#include <stdint.h> // uint32_t
#include <stdlib.h> // size_t
#include <stddef.h> // ptrdiff_t

#include "platform.h"
#include "config.h"
#include "macros.h"

///
#define BX_COUNTOF(_x) sizeof(bx::COUNTOF_REQUIRES_ARRAY_ARGUMENT(_x) )

///
#define BX_IGNORE_C4127(_x) bx::ignoreC4127(!!(_x) )

///
#define BX_ENABLED(_x) bx::isEnabled<!!(_x)>()

namespace bx
{
	/// Template for avoiding MSVC: C4127: conditional expression is constant
	template<bool>
	bool isEnabled();

	///
	bool ignoreC4127(bool _x);

	///
	template<typename Ty>
	void xchg(Ty& _a, Ty& _b);

	///
	void xchg(void* _a, void* _b, size_t _numBytes);

	// http://cnicholson.net/2011/01/stupid-c-tricks-a-better-sizeof_array/
	template<typename T, size_t N>
	char (&COUNTOF_REQUIRES_ARRAY_ARGUMENT(const T(&)[N]) )[N];

	///
	void memCopy(void* _dst, const void* _src, size_t _numBytes);

	///
	void memCopy(void* _dst, const void* _src, uint32_t _size, uint32_t _num, uint32_t _srcPitch, uint32_t _dstPitch);

	///
	void gather(void* _dst, const void* _src, uint32_t _size, uint32_t _num, uint32_t _srcPitch);

	///
	void scatter(void* _dst, const void* _src, uint32_t _size, uint32_t _num, uint32_t _dstPitch);

	///
	void memMove(void* _dst, const void* _src, size_t _numBytes);

	///
	void memSet(void* _dst, uint8_t _ch, size_t _numBytes);

	///
	int32_t memCmp(const void* _lhs, const void* _rhs, size_t _numBytes);

} // namespace bx

#include "inline/bx.inl"

#endif // BX_H_HEADER_GUARD
