// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  eivcx86.h
//
//  x86 inline implementations for MSVC compiler.
//
//============================================================

#ifndef MAME_OSD_EIVCX86_H
#define MAME_OSD_EIVCX86_H

#pragma once

#ifdef PTR64
#include <emmintrin.h>
#endif

#include <intrin.h>


/***************************************************************************
    INLINE MATH FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    mul_32x32 - perform a signed 32 bit x 32 bit
    multiply and return the full 64 bit result
-------------------------------------------------*/

#ifndef PTR64
#define mul_32x32 _mul_32x32
inline int64_t _mul_32x32(int32_t a, int32_t b)
{
	// in theory this should work, but it is untested
	__asm
	{
		mov   eax,a
		imul  b
		// leave results in edx:eax
	}
}
#endif


/*-------------------------------------------------
    mulu_32x32 - perform an unsigned 32 bit x
    32 bit multiply and return the full 64 bit
    result
-------------------------------------------------*/

#ifndef PTR64
#define mulu_32x32 _mulu_32x32
inline uint64_t _mulu_32x32(uint32_t a, uint32_t b)
{
	// in theory this should work, but it is untested
	__asm
	{
		mov   eax,a
		mul   b
		// leave results in edx:eax
	}
}
#endif


/*-------------------------------------------------
    mul_32x32_hi - perform a signed 32 bit x 32 bit
    multiply and return the upper 32 bits of the
    result
-------------------------------------------------*/

#ifndef PTR64
#define mul_32x32_hi _mul_32x32_hi
inline int32_t _mul_32x32_hi(int32_t a, int32_t b)
{
	int32_t result;

	__asm
	{
		mov   eax,a
		imul  b
		mov   result,edx
	}

	return result;
}
#endif


/*-------------------------------------------------
    mulu_32x32_hi - perform an unsigned 32 bit x
    32 bit multiply and return the upper 32 bits
    of the result
-------------------------------------------------*/

#ifndef PTR64
#define mulu_32x32_hi _mulu_32x32_hi
inline uint32_t _mulu_32x32_hi(uint32_t a, uint32_t b)
{
	int32_t result;

	__asm
	{
		mov   eax,a
		mul   b
		mov   result,edx
	}

	return result;
}
#endif


/*-------------------------------------------------
    mul_32x32_shift - perform a signed 32 bit x
    32 bit multiply and shift the result by the
    given number of bits before truncating the
    result to 32 bits
-------------------------------------------------*/

#ifndef PTR64
#define mul_32x32_shift _mul_32x32_shift
static inline int32_t _mul_32x32_shift(int32_t a, int32_t b, uint8_t shift)
{
	int32_t result;

	__asm
	{
		mov   eax,a
		imul  b
		mov   cl,shift
		shrd  eax,edx,cl
		mov   result,eax
	}

	return result;
}
#endif


/*-------------------------------------------------
    mulu_32x32_shift - perform an unsigned 32 bit x
    32 bit multiply and shift the result by the
    given number of bits before truncating the
    result to 32 bits
-------------------------------------------------*/

#ifndef PTR64
#define mulu_32x32_shift _mulu_32x32_shift
inline uint32_t _mulu_32x32_shift(uint32_t a, uint32_t b, uint8_t shift)
{
	int32_t result;

	__asm
	{
		mov   eax,a
		mul   b
		mov   cl,shift
		shrd  eax,edx,cl
		mov   result,eax
	}

	return result;
}
#endif


/*-------------------------------------------------
    div_64x32 - perform a signed 64 bit x 32 bit
    divide and return the 32 bit quotient
-------------------------------------------------*/

#ifndef PTR64
#define div_64x32 _div_64x32
inline int32_t _div_64x32(int64_t a, int32_t b)
{
	int32_t result;
	int32_t alow = a;
	int32_t ahigh = a >> 32;

	__asm
	{
		mov   eax,alow
		mov   edx,ahigh
		idiv  b
		mov   result,eax
	}

	return result;
}
#endif


/*-------------------------------------------------
    divu_64x32 - perform an unsigned 64 bit x 32 bit
    divide and return the 32 bit quotient
-------------------------------------------------*/

#ifndef PTR64
#define divu_64x32 _divu_64x32
inline uint32_t _divu_64x32(uint64_t a, uint32_t b)
{
	uint32_t result;
	uint32_t alow = a;
	uint32_t ahigh = a >> 32;

	__asm
	{
		mov   eax,alow
		mov   edx,ahigh
		div   b
		mov   result,eax
	}

	return result;
}
#endif


/*-------------------------------------------------
    div_64x32_rem - perform a signed 64 bit x 32
    bit divide and return the 32 bit quotient and
    32 bit remainder
-------------------------------------------------*/

#ifndef PTR64
#define div_64x32_rem _div_64x32_rem
inline int32_t _div_64x32_rem(int64_t a, int32_t b, int32_t &remainder)
{
	int32_t result;
	int32_t alow = a;
	int32_t ahigh = a >> 32;
	int32_t rem;

	__asm
	{
		mov   eax,alow
		mov   edx,ahigh
		idiv  b
		mov   result,eax
		mov   rem,edx
	}

	remainder = rem;
	return result;
}
#endif


/*-------------------------------------------------
    divu_64x32_rem - perform an unsigned 64 bit x
    32 bit divide and return the 32 bit quotient
    and 32 bit remainder
-------------------------------------------------*/

#ifndef PTR64
#define divu_64x32_rem _divu_64x32_rem
inline uint32_t _divu_64x32_rem(uint64_t a, uint32_t b, uint32_t &remainder)
{
	uint32_t result;
	uint32_t alow = a;
	uint32_t ahigh = a >> 32;
	uint32_t rem;

	__asm
	{
		mov   eax,alow
		mov   edx,ahigh
		div   b
		mov   result,eax
		mov   rem,edx
	}

	remainder = rem;
	return result;
}
#endif


/*-------------------------------------------------
    div_32x32_shift - perform a signed divide of
    two 32 bit values, shifting the first before
    division, and returning the 32 bit quotient
-------------------------------------------------*/

#ifndef PTR64
#define div_32x32_shift _div_32x32_shift
inline int32_t _div_32x32_shift(int32_t a, int32_t b, uint8_t shift)
{
	int32_t result;

	__asm
	{
		mov   eax,a
		cdq
		mov   cl,shift
		shld  edx,eax,cl
		shl   eax,cl
		idiv  b
		mov   result,eax
	}

	return result;
}
#endif


/*-------------------------------------------------
    divu_32x32_shift - perform an unsigned divide of
    two 32 bit values, shifting the first before
    division, and returning the 32 bit quotient
-------------------------------------------------*/

#ifndef PTR64
#define divu_32x32_shift _divu_32x32_shift
inline uint32_t _divu_32x32_shift(uint32_t a, uint32_t b, uint8_t shift)
{
	uint32_t result;

	__asm
	{
		mov   eax,a
		xor   edx,edx
		mov   cl,shift
		shld  edx,eax,cl
		shl   eax,cl
		div   b
		mov   result,eax
	}

	return result;
}
#endif


/*-------------------------------------------------
    mod_64x32 - perform a signed 64 bit x 32 bit
    divide and return the 32 bit remainder
-------------------------------------------------*/

#ifndef PTR64
#define mod_64x32 _mod_64x32
static inline int32_t _mod_64x32(int64_t a, int32_t b)
{
	int32_t result;
	int32_t alow = a;
	int32_t ahigh = a >> 32;

	__asm
	{
		mov   eax,alow
		mov   edx,ahigh
		idiv  b
		mov   result,edx
	}

	return result;
}
#endif


/*-------------------------------------------------
    modu_64x32 - perform an unsigned 64 bit x 32 bit
    divide and return the 32 bit remainder
-------------------------------------------------*/

#ifndef PTR64
#define modu_64x32 _modu_64x32
inline uint32_t _modu_64x32(uint64_t a, uint32_t b)
{
	uint32_t result;
	uint32_t alow = a;
	uint32_t ahigh = a >> 32;

	__asm
	{
		mov   eax,alow
		mov   edx,ahigh
		div   b
		mov   result,edx
	}

	return result;
}
#endif


/*-------------------------------------------------
    recip_approx - compute an approximate floating
    point reciprocal
-------------------------------------------------*/

#ifdef PTR64
#define recip_approx _recip_approx
inline float _recip_approx(float z)
{
	__m128 const mz = _mm_set_ss(z);
	__m128 const mooz = _mm_rcp_ss(mz);
	float ooz;
	_mm_store_ss(&ooz, mooz);
	return ooz;
}
#endif


/*-------------------------------------------------
    mul_64x64 - perform a signed 64 bit x 64 bit
    multiply and return the full 128 bit result
-------------------------------------------------*/

#ifdef PTR64
#define mul_64x64 _mul_64x64
__forceinline int64_t _mul_64x64(int64_t a, int64_t b, int64_t &hi)
{
	return _mul128(a, b, &hi);
}
#endif


/*-------------------------------------------------
    mulu_64x64 - perform an unsigned 64 bit x 64
    bit multiply and return the full 128 bit result
-------------------------------------------------*/

#ifdef PTR64
#define mulu_64x64 _mulu_64x64
__forceinline int64_t _mulu_64x64(uint64_t a, uint64_t b, uint64_t &hi)
{
	return _umul128(a, b, &hi);
}
#endif


/*-------------------------------------------------
    addu_32x32_co - perform an unsigned 32 bit + 32
    bit addition and return the result with carry
    out
-------------------------------------------------*/

#define addu_32x32_co _addu_32x32_co
__forceinline bool _addu_32x32_co(uint32_t a, uint32_t b, uint32_t &sum)
{
	return _addcarry_u32(0, a, b, &sum);
}


/*-------------------------------------------------
    addu_64x64_co - perform an unsigned 64 bit + 64
    bit addition and return the result with carry
    out
-------------------------------------------------*/

#define addu_64x64_co _addu_64x64_co
__forceinline bool _addu_64x64_co(uint64_t a, uint64_t b, uint64_t &sum)
{
#ifdef PTR64
	return _addcarry_u64(0, a, b, &sum);
#else
	uint32_t l, h;
	bool const result = _addcarry_u32(_addcarry_u32(0, uint32_t(a), uint32_t(b), &l), uint32_t(a >> 32), uint32_t(b >> 32), &h);
	sum = (uint64_t(h) << 32) | l;
	return result;
#endif
}

#endif // MAME_OSD_EIVCX86_H
