/*
 * Copyright 2010-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#include <bx/debug.h>
#include <bx/readerwriter.h>

#if !BX_CRT_NONE
#	include <string.h> // memcpy, memmove, memset
#endif // !BX_CRT_NONE

namespace bx
{
	Location Location::current(const char* _filePath, uint32_t _line)
	{
		return Location(_filePath, _line);
	}

	LocationFull LocationFull::current(const char* _function, const char* _filePath, uint32_t _line)
	{
		return LocationFull(_function, _filePath, _line);
	}

	static bool defaultAssertHandler(const Location& _location, const char* _format, va_list _argList)
	{
		char    temp[8192];
		int32_t total = 0;

		StaticMemoryBlockWriter smb(temp, BX_COUNTOF(temp) );

		ErrorIgnore err;

		total += write(&smb, &err, "\n--- ASSERT ---\n\n");

		total += write(&smb, &err, "%s(%d): "
			, _location.filePath
			, _location.line
			);
		total += write(&smb, _format, _argList, &err);
		total += write(&smb, "\n\n", &err);

		uintptr_t stack[32];
		const uint32_t num = getCallStack(2 /* skip self */, BX_COUNTOF(stack), stack);
		total += writeCallstack(&smb, stack, num, &err);

		total += write(&smb, &err,
			"\nBuild info:\n"
			"\tCompiler: " BX_COMPILER_NAME
			", CPU: " BX_CPU_NAME
			", Arch: " BX_ARCH_NAME
			", OS: " BX_PLATFORM_NAME
			", CRT: " BX_CRT_NAME
			", C++: " BX_CPP_NAME

			", Date: " __DATE__
			", Time: " __TIME__
			"\n"
			);

		total += write(&smb, &err, "\n--- END ---\n\n");

		write(getDebugOut(), temp, total, ErrorIgnore{});

		return true;
	}

	static AssertHandlerFn s_assertHandler = defaultAssertHandler;

	void setAssertHandler(AssertHandlerFn _assertHandlerFn)
	{
		BX_WARN(defaultAssertHandler == s_assertHandler, "Assert handler is already set.");

		if (defaultAssertHandler == s_assertHandler)
		{
			s_assertHandler = NULL == _assertHandlerFn
				? defaultAssertHandler
				: _assertHandlerFn
				;
		}
	}

	bool assertFunction(const Location& _location, const char* _format, ...)
	{
		va_list argList;
		va_start(argList, _format);
		const bool result = s_assertHandler(_location, _format, argList);
		va_end(argList);

		return result;
	}

	void swap(void* _a, void* _b, size_t _numBytes)
	{
		uint8_t* lhs = (uint8_t*)_a;
		uint8_t* rhs = (uint8_t*)_b;
		const uint8_t* end = rhs + _numBytes;
		while (rhs != end)
		{
			swap(*lhs++, *rhs++);
		}
	}

	void memCopyRef(void* _dst, const void* _src, size_t _numBytes)
	{
		uint8_t* dst = (uint8_t*)_dst;
		const uint8_t* end = dst + _numBytes;
		const uint8_t* src = (const uint8_t*)_src;
		while (dst != end)
		{
			*dst++ = *src++;
		}
	}

	void memCopy(void* _dst, const void* _src, size_t _numBytes)
	{
#if BX_CRT_NONE
		memCopyRef(_dst, _src, _numBytes);
#else
		::memcpy(_dst, _src, _numBytes);
#endif // BX_CRT_NONE
	}

	void memCopy(
		  void* _dst
		, uint32_t _dstStride
		, const void* _src
		, uint32_t _srcStride
		, uint32_t _stride
		, uint32_t _numStrides
		)
	{
		if (_stride == _srcStride
		&&  _stride == _dstStride)
		{
			memCopy(_dst, _src, _stride*_numStrides);
			return;
		}

		const uint8_t* src = (const uint8_t*)_src;
		      uint8_t* dst = (uint8_t*)_dst;

		for (uint32_t ii = 0; ii < _numStrides; ++ii, src += _srcStride, dst += _dstStride)
		{
			memCopy(dst, src, _stride);
		}
	}

	void memMoveRef(void* _dst, const void* _src, size_t _numBytes)
	{
		uint8_t* dst = (uint8_t*)_dst;
		const uint8_t* src = (const uint8_t*)_src;

		if (_numBytes == 0
		||  dst == src)
		{
			return;
		}

		//	if (src+_numBytes <= dst || end <= src)
		if (dst < src)
		{
			memCopy(_dst, _src, _numBytes);
			return;
		}

		for (intptr_t ii = _numBytes-1; ii >= 0; --ii)
		{
			dst[ii] = src[ii];
		}
	}

	void memMove(void* _dst, const void* _src, size_t _numBytes)
	{
#if BX_CRT_NONE
		memMoveRef(_dst, _src, _numBytes);
#else
		::memmove(_dst, _src, _numBytes);
#endif // BX_CRT_NONE
	}

	void memMove(
		  void* _dst
		, uint32_t _dstStride
		, const void* _src
		, uint32_t _srcStride
		, uint32_t _stride
		, uint32_t _numStrides
		)
	{
		if (_stride == _srcStride
		&&  _stride == _dstStride)
		{
			memMove(_dst, _src, _stride*_numStrides);
			return;
		}

		const uint8_t* src = (const uint8_t*)_src;
		      uint8_t* dst = (uint8_t*)_dst;

		for (uint32_t ii = 0; ii < _numStrides; ++ii, src += _srcStride, dst += _dstStride)
		{
			memMove(dst, src, _stride);
		}
	}

	void memSetRef(void* _dst, uint8_t _ch, size_t _numBytes)
	{
		uint8_t* dst = (uint8_t*)_dst;
		const uint8_t* end = dst + _numBytes;
		while (dst != end)
		{
			*dst++ = char(_ch);
		}
	}

	void memSet(void* _dst, uint8_t _ch, size_t _numBytes)
	{
#if BX_CRT_NONE
		memSetRef(_dst, _ch, _numBytes);
#else
		::memset(_dst, _ch, _numBytes);
#endif // BX_CRT_NONE
	}

	void memSet(void* _dst, uint32_t _dstStride, uint8_t _ch, uint32_t _stride, uint32_t _num)
	{
		if (_stride == _dstStride)
		{
			memSet(_dst, _ch, _stride*_num);
			return;
		}

		uint8_t* dst = (uint8_t*)_dst;

		for (uint32_t ii = 0; ii < _num; ++ii, dst += _dstStride)
		{
			memSet(dst, _ch, _stride);
		}
	}

	int32_t memCmpRef(const void* _lhs, const void* _rhs, size_t _numBytes)
	{
		const char* lhs = (const char*)_lhs;
		const char* rhs = (const char*)_rhs;
		for (
			; 0 < _numBytes && *lhs == *rhs
			; ++lhs, ++rhs, --_numBytes
			)
		{
		}

		return 0 == _numBytes ? 0 : *lhs - *rhs;
	}

	int32_t memCmp(const void* _lhs, const void* _rhs, size_t _numBytes)
	{
#if BX_CRT_NONE
		return memCmpRef(_lhs, _rhs, _numBytes);
#else
		return ::memcmp(_lhs, _rhs, _numBytes);
#endif // BX_CRT_NONE
	}

	///
	void gather(void* _dst, const void* _src, uint32_t _srcStride, uint32_t _stride, uint32_t _numStrides)
	{
		memMove(_dst, _stride, _src, _srcStride, _stride, _numStrides);
	}

	///
	void scatter(void* _dst, uint32_t _dstStride, const void* _src, uint32_t _stride, uint32_t _numStrides)
	{
		memMove(_dst, _dstStride, _src, _stride, _stride, _numStrides);
	}

} // namespace bx
