/*
 * Copyright 2010-2021 Branimir Karadzic. All rights reserved.
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
#if BX_COMPILER_MSVC
#	define BX_IGNORE_C4127(_x) bx::ignoreC4127(!!(_x) )
#else
#	define BX_IGNORE_C4127(_x) (!!(_x) )
#endif // BX_COMPILER_MSVC

///
#define BX_ENABLED(_x) BX_IGNORE_C4127(bx::isEnabled<!!(_x)>::value)

namespace bx
{
	constexpr int32_t kExitSuccess = 0;
	constexpr int32_t kExitFailure = 1;

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

	/// Copy memory block.
	///
	/// @param _dst Destination pointer.
	/// @param _src Source pointer.
	/// @param _numBytes Number of bytes to copy.
	///
	/// @remark Source and destination memory blocks must not overlap.
	///
	void memCopy(void* _dst, const void* _src, size_t _numBytes);

	/// Copy strided memory block.
	///
	/// @param _dst Destination pointer.
	/// @param _dstStride Destination stride.
	/// @param _src Source pointer.
	/// @param _srcStride Source stride.
	/// @param _stride Number of bytes per stride to copy.
	/// @param _num Number of strides.
	///
	/// @remark Source and destination memory blocks must not overlap.
	///
	void memCopy(
		  void* _dst
		, uint32_t _dstStride
		, const void* _src
		, uint32_t _srcStride
		, uint32_t _stride
		, uint32_t _num
		);

	///
	void memMove(void* _dst, const void* _src, size_t _numBytes);

	///
	void memMove(
		  void* _dst
		, uint32_t _dstStride
		, const void* _src
		, uint32_t _srcStride
		, uint32_t _stride
		, uint32_t _num
		);

	///
	void memSet(void* _dst, uint8_t _ch, size_t _numBytes);

	///
	void memSet(void* _dst, uint32_t _dstStride, uint8_t _ch, uint32_t _stride, uint32_t _num);

	///
	int32_t memCmp(const void* _lhs, const void* _rhs, size_t _numBytes);

	///
	void gather(
		  void* _dst
		, const void* _src
		, uint32_t _srcStride
		, uint32_t _stride
		, uint32_t _num
		);

	///
	void scatter(
		  void* _dst
		, uint32_t _dstStride
		, const void* _src
		, uint32_t _stride
		, uint32_t _num
		);

} // namespace bx

#include "inline/bx.inl"

#endif // BX_H_HEADER_GUARD
