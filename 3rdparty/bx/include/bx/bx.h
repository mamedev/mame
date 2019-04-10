/*
 * Copyright 2010-2018 Branimir Karadzic. All rights reserved.
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
#define BX_COUNTOF(_x) sizeof(bx::CountOfRequireArrayArgumentT(_x) )

///
#define BX_IGNORE_C4127(_x) bx::ignoreC4127(!!(_x) )

///
#define BX_ENABLED(_x) bx::isEnabled<!!(_x)>()

namespace bx
{
	constexpr int32_t kExitSuccess = 0;
	constexpr int32_t kExitFailure = 1;

	/// Template for avoiding MSVC: C4127: conditional expression is constant
	template<bool>
	constexpr bool isEnabled();

	///
	template<class Ty>
	constexpr bool isTriviallyCopyable();

	/// Swap two values.
	template<typename Ty>
	void swap(Ty& _a, Ty& _b);

	/// Swap memory.
	void swap(void* _a, void* _b, size_t _numBytes);

	/// Returns minimum of two values.
	template<typename Ty>
	constexpr Ty min(const Ty& _a, const Ty& _b);

	/// Returns maximum of two values.
	template<typename Ty>
	constexpr Ty max(const Ty& _a, const Ty& _b);

	/// Returns minimum of three values.
	template<typename Ty>
	constexpr Ty min(const Ty& _a, const Ty& _b, const Ty& _c);

	/// Returns maximum of three values.
	template<typename Ty>
	constexpr Ty max(const Ty& _a, const Ty& _b, const Ty& _c);

	/// Returns middle of three values.
	template<typename Ty>
	constexpr Ty mid(const Ty& _a, const Ty& _b, const Ty& _c);

	/// Returns clamped value between min/max.
	template<typename Ty>
	constexpr Ty clamp(const Ty& _a, const Ty& _min, const Ty& _max);

	/// Returns true if value is power of 2.
	template<typename Ty>
	constexpr bool isPowerOf2(Ty _a);

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
