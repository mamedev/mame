// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  eivcx86.h
//
//  x86 inline implementations for MSVC compiler.
//
//============================================================

#ifndef __EIVCX86__
#define __EIVCX86__

#ifdef PTR64
#include <emmintrin.h>
#include <intrin.h>
#endif


/***************************************************************************
    INLINE MATH FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    mul_32x32 - perform a signed 32 bit x 32 bit
    multiply and return the full 64 bit result
-------------------------------------------------*/

#ifndef PTR64
#define mul_32x32 _mul_32x32
static inline INT64 _mul_32x32(INT32 a, INT32 b)
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
static inline UINT64 _mulu_32x32(UINT32 a, UINT32 b)
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
static inline INT32 _mul_32x32_hi(INT32 a, INT32 b)
{
	INT32 result;

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
static inline UINT32 _mulu_32x32_hi(UINT32 a, UINT32 b)
{
	INT32 result;

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
static inline INT32 _mul_32x32_shift(INT32 a, INT32 b, UINT8 shift)
{
	INT32 result;

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
static inline UINT32 _mulu_32x32_shift(UINT32 a, UINT32 b, UINT8 shift)
{
	INT32 result;

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
static inline INT32 _div_64x32(INT64 a, INT32 b)
{
	INT32 result;
	INT32 alow = a;
	INT32 ahigh = a >> 32;

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
static inline UINT32 _divu_64x32(UINT64 a, UINT32 b)
{
	UINT32 result;
	UINT32 alow = a;
	UINT32 ahigh = a >> 32;

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
static inline INT32 _div_64x32_rem(INT64 a, INT32 b, INT32 *remainder)
{
	INT32 result;
	INT32 alow = a;
	INT32 ahigh = a >> 32;
	INT32 rem;

	__asm
	{
		mov   eax,alow
		mov   edx,ahigh
		idiv  b
		mov   result,eax
		mov   rem,edx
	}

	*remainder = rem;
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
static inline UINT32 _divu_64x32_rem(UINT64 a, UINT32 b, UINT32 *remainder)
{
	UINT32 result;
	UINT32 alow = a;
	UINT32 ahigh = a >> 32;
	UINT32 rem;

	__asm
	{
		mov   eax,alow
		mov   edx,ahigh
		div   b
		mov   result,eax
		mov   rem,edx
	}

	*remainder = rem;
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
static inline INT32 _div_32x32_shift(INT32 a, INT32 b, UINT8 shift)
{
	INT32 result;

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
static inline UINT32 _divu_32x32_shift(UINT32 a, UINT32 b, UINT8 shift)
{
	UINT32 result;

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
static inline INT32 _mod_64x32(INT64 a, INT32 b)
{
	INT32 result;
	INT32 alow = a;
	INT32 ahigh = a >> 32;

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
static inline UINT32 _modu_64x32(UINT64 a, UINT32 b)
{
	UINT32 result;
	UINT32 alow = a;
	UINT32 ahigh = a >> 32;

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
static inline float _recip_approx(float z)
{
	__m128 mz = _mm_set_ss(z);
	__m128 mooz = _mm_rcp_ss(mz);
	float ooz;
	_mm_store_ss(&ooz, mooz);
	return ooz;
}
#endif



/***************************************************************************
    INLINE BIT MANIPULATION FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    count_leading_zeros - return the number of
    leading zero bits in a 32-bit value
-------------------------------------------------*/

#ifndef PTR64
#define count_leading_zeros _count_leading_zeros
static inline UINT8 _count_leading_zeros(UINT32 value)
{
	INT32 result;

	__asm
	{
		bsr   eax,value
		jnz   skip
		mov   eax,63
	skip:
		xor   eax,31
		mov   result,eax
	}

	return result;
}
#endif


/*-------------------------------------------------
    count_leading_ones - return the number of
    leading one bits in a 32-bit value
-------------------------------------------------*/

#ifndef PTR64
#define count_leading_ones _count_leading_ones
static inline UINT8 _count_leading_ones(UINT32 value)
{
	INT32 result;

	__asm
	{
		mov   eax,value
		not   eax
		bsr   eax,eax
		jnz   skip
		mov   eax,63
	skip:
		xor   eax,31
		mov   result,eax
	}

	return result;
}
#endif

#endif /* __EIVCX86__ */
