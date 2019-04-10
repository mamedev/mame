/*
 * Copyright 2010-2018 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include "bx_p.h"
#include <bx/debug.h>
#include <bx/readerwriter.h>

#if !BX_CRT_NONE
#	include <string.h> // memcpy, memmove, memset
#endif // !BX_CRT_NONE

namespace bx
{
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

	void memCopy(void* _dst, const void* _src, uint32_t _size, uint32_t _num, uint32_t _srcPitch, uint32_t _dstPitch)
	{
		const uint8_t* src = (const uint8_t*)_src;
		uint8_t* dst = (uint8_t*)_dst;

		for (uint32_t ii = 0; ii < _num; ++ii)
		{
			memCopy(dst, src, _size);
			src += _srcPitch;
			dst += _dstPitch;
		}
	}

	///
	void gather(void* _dst, const void* _src, uint32_t _size, uint32_t _num, uint32_t _srcPitch)
	{
		memCopy(_dst, _src, _size, _num, _srcPitch, _size);
	}

	///
	void scatter(void* _dst, const void* _src, uint32_t _size, uint32_t _num, uint32_t _dstPitch)
	{
		memCopy(_dst, _src, _size, _num, _size, _dstPitch);
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

} // namespace bx
