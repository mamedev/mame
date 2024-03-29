/*
 * Copyright 2010-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#ifndef BX_H_HEADER_GUARD
#define BX_H_HEADER_GUARD

#include <alloca.h> // alloca
#include <stdarg.h> // va_list
#include <stddef.h> // ptrdiff_t
#include <stdint.h> // uint32_t
#include <stdlib.h> // size_t

#include "platform.h"
#include "config.h"
#include "constants.h"
#include "macros.h"
#include "debug.h"
#include "typetraits.h"

///
#define BX_COUNTOF(_x) sizeof(bx::CountOfRequireArrayArgumentT(_x) )

///
#define BX_OFFSETOF(_type, _member) \
	(reinterpret_cast<ptrdiff_t>(&(reinterpret_cast<_type*>(16)->_member) )-ptrdiff_t(16) )

///
#if BX_COMPILER_MSVC
#	define BX_IGNORE_C4127(_x) bx::ignoreC4127(!!(_x) )
#else
#	define BX_IGNORE_C4127(_x) (!!(_x) )
#endif // BX_COMPILER_MSVC

///
#define BX_ENABLED(_x) BX_IGNORE_C4127(bx::isEnabled<!!(_x)>::value)

///
#define BX_DECLARE_TAG(_name)  \
	struct    _name ## Tag {}; \
	constexpr _name ## Tag _name

namespace bx
{
	/// Placement new tag.
	BX_DECLARE_TAG(PlacementNew);

	/// Fields are left uninitialized.
	BX_DECLARE_TAG(InitNone);

	/// Fields are initialized to zero.
	BX_DECLARE_TAG(InitZero);

	/// Fields are initialized to identity value.
	BX_DECLARE_TAG(InitIdentity);

	/// Source location with file path, and file line.
	///
	struct Location
	{
		/// Default constructor.
		///
		constexpr Location()
			: filePath(""), line(0) {}

		/// Constructor with specific file name, and line number.
		///
		constexpr Location(const char* _filePath, uint32_t _line)
			: filePath(_filePath), line(_line) {}

		/// Current source location.
		///
		static Location current(
			  const char* _filePath = __builtin_FILE()
			, uint32_t _line = __builtin_LINE()
			);

		const char* filePath; //!< File path.
		uint32_t    line;     //!< File line.
	};

	/// Unknown source code location.
	static constexpr Location kUnknownLocation("Unknown?", 0);

	/// Source location with file path, file line, and function name.
	///
	struct LocationFull
	{
		/// Default constructor.
		///
		constexpr LocationFull()
			: function(""), filePath(""), line(0) {}

		/// Constructor with specific function name, file name, and line number.
		///
		constexpr LocationFull(const char* _function, const char* _filePath, uint32_t _line)
			: function(_function), filePath(_filePath), line(_line) {}

		/// Current source location.
		///
		static LocationFull current(
			  const char* _function = __builtin_FUNCTION()
			, const char* _filePath = __builtin_FILE()
			, uint32_t _line = __builtin_LINE()
			);

		const char* function; //!< Function name.
		const char* filePath; //!< File path.
		uint32_t    line;     //!< File line.
	};

	/// Unknown source code location.
	static constexpr LocationFull kUnknownLocationFull("Unknown?", "Unknown?", 0);

	/// Assert handler function.
	///
	/// @param[in] _location Source code location where function is called.
	/// @param[in] _format Printf style format.
	/// @param[in] ... Arguments for `_format` specification.
	///
	/// @returns True if assert should stop code execution, otherwise returns false.
	///
	typedef bool (*AssertHandlerFn)(const Location& _location, const char* _format, va_list _argList);

	/// Set assert handler function.
	///
	/// @param[in] _assertHandlerFn Pointer to AssertHandlerFn function.
	///
	void setAssertHandler(AssertHandlerFn _assertHandlerFn);

	/// Assert function calls AssertHandlerFn.
	///
	/// @param[in] _location Source code location where function is called.
	/// @param[in] _format Printf style format.
	/// @param[in] ... Arguments for `_format` specification.
	///
	/// @returns True if assert should stop code execution, otherwise returns false.
	///
	bool assertFunction(const Location& _location, const char* _format, ...);

	/// Arithmetic type `Ty` limits.
	template<typename Ty, bool SignT = isSigned<Ty>()>
	struct LimitsT;

	/// Find the address of an object of a class that has an overloaded unary ampersand (&) operator.
	template<typename Ty>
	Ty* addressOf(Ty& _a);

	/// Find the address of an object of a class that has an overloaded unary ampersand (&) operator.
	template<typename Ty>
	const Ty* addressOf(const Ty& _a);

	/// Returns typed pointer from typeless pointer offseted.
	///
	/// @param[in] _ptr Pointer to get offset from.
	/// @param[in] _offsetInBytes Offset from pointer in bytes.
	///
	/// @returns Typed pointer from typeless pointer offseted.
	///
	template<typename Ty>
	Ty* addressOf(void* _ptr, ptrdiff_t _offsetInBytes = 0);

	/// Returns typed pointer from typeless pointer offseted.
	///
	/// @param[in] _ptr Pointer to get offset from.
	/// @param[in] _offsetInBytes Offset from pointer in bytes.
	///
	/// @returns Typed pointer from typeless pointer offseted.
	///
	template<typename Ty>
	const Ty* addressOf(const void* _ptr, ptrdiff_t _offsetInBytes = 0);

	/// Swap two values.
	template<typename Ty>
	void swap(Ty& _a, Ty& _b);

	/// Swap memory.
	void swap(void* _a, void* _b, size_t _numBytes);

	/// Returns numeric minimum of type.
	template<typename Ty>
	constexpr Ty min();

	/// Returns numeric maximum of type.
	template<typename Ty>
	constexpr Ty max();

	/// Returns minimum of two values.
	template<typename Ty>
	constexpr Ty min(const Ty& _a, const TypeIdentityType<Ty>& _b);

	/// Returns maximum of two values.
	template<typename Ty>
	constexpr Ty max(const Ty& _a, const TypeIdentityType<Ty>& _b);

	/// Returns minimum of three or more values.
	template<typename Ty, typename... Args>
	constexpr Ty min(const Ty& _a, const TypeIdentityType<Ty>& _b, const Args&... _args);

	/// Returns maximum of three or more values.
	template<typename Ty, typename... Args>
	constexpr Ty max(const Ty& _a, const TypeIdentityType<Ty>& _b, const Args&... _args);

	/// Returns middle of three or more values.
	template<typename Ty, typename... Args>
	constexpr Ty mid(const Ty& _a, const TypeIdentityType<Ty>& _b, const Args&... _args);

	/// Returns clamped value between min/max.
	template<typename Ty>
	constexpr Ty clamp(const Ty& _a, const TypeIdentityType<Ty>& _min, const TypeIdentityType<Ty>& _max);

	/// Returns true if value `_a` is power of 2.
	template<typename Ty>
	constexpr bool isPowerOf2(Ty _a);

	/// Returns a value of type `Ty` by reinterpreting the object representation of `FromT`.
	template <typename Ty, typename FromT>
	constexpr Ty bitCast(const FromT& _from);

	/// Performs `static_cast` of value `_from`, and in debug build runtime verifies/asserts
	/// that the value didn't change.
	template<typename Ty, typename FromT>
	constexpr Ty narrowCast(const FromT& _from, Location _location = Location::current() );

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
	/// @param _numStrides Number of strides.
	///
	/// @remark Source and destination memory blocks must not overlap.
	///
	void memCopy(
		  void* _dst
		, uint32_t _dstStride
		, const void* _src
		, uint32_t _srcStride
		, uint32_t _stride
		, uint32_t _numStrides
		);

	/// Copy memory block.
	///
	/// @param _dst Destination pointer.
	/// @param _src Source pointer.
	/// @param _numBytes Number of bytes to copy.
	///
	/// @remark If source and destination memory blocks overlap memory will be copied in reverse
	///   order.
	///
	void memMove(void* _dst, const void* _src, size_t _numBytes);

	/// Copy strided memory block.
	///
	/// @param _dst Destination pointer.
	/// @param _dstStride Destination stride.
	/// @param _src Source pointer.
	/// @param _srcStride Source stride.
	/// @param _stride Number of bytes per stride to copy.
	/// @param _numStrides Number of strides.
	///
	/// @remark If source and destination memory blocks overlap memory will be copied in reverse
	///   order.
	///
	void memMove(
		  void* _dst
		, uint32_t _dstStride
		, const void* _src
		, uint32_t _srcStride
		, uint32_t _stride
		, uint32_t _numStrides
		);

	/// Fill memory block to specified value `_ch`.
	///
	/// @param _dst Destination pointer.
	/// @param _ch Fill value.
	/// @param _numBytes Number of bytes to copy.
	///
	void memSet(void* _dst, uint8_t _ch, size_t _numBytes);

	/// Fill strided memory block to specified value `_ch`.
	///
	/// @param _dst Destination pointer.
	/// @param _dstStride Destination stride.
	/// @param _ch Fill value.
	/// @param _stride Number of bytes per stride to copy.
	/// @param _numStrides Number of strides.
	///
	void memSet(
		  void* _dst
		, uint32_t _dstStride
		, uint8_t _ch
		, uint32_t _stride
		, uint32_t _numStrides
		);

	/// Compare two memory blocks.
	///
	/// @param _lhs Pointer to memory block.
	/// @param _rhs Pointer to memory block.
	/// @param _numBytes Number of bytes to compare.
	///
	/// @returns 0 if two blocks are identical, positive value if first different byte in `_lhs` is
	///   greater than corresponding byte in `_rhs`.
	///
	int32_t memCmp(const void* _lhs, const void* _rhs, size_t _numBytes);

	/// Gather data scattered through memory into linear memory block.
	///
	/// @param _dst Destination pointer.
	/// @param _src Source pointer.
	/// @param _stride Number of bytes per stride to copy.
	/// @param _numStrides Number of strides.
	///
	void gather(
		  void* _dst
		, const void* _src
		, uint32_t _srcStride
		, uint32_t _stride
		, uint32_t _numStrides
		);

	/// Scatter data from linear memory block through memory.
	///
	/// @param _dst Destination pointer.
	/// @param _dstStride Destination stride.
	/// @param _src Source pointer.
	/// @param _stride Number of bytes per stride to copy.
	/// @param _numStrides Number of strides.
	///
	void scatter(
		  void* _dst
		, uint32_t _dstStride
		, const void* _src
		, uint32_t _stride
		, uint32_t _numStrides
		);

} // namespace bx

#include "inline/bx.inl"

#endif // BX_H_HEADER_GUARD
