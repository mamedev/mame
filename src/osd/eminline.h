// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    eminline.h

    Definitions for inline functions that can be overriden by OSD-
    specific code.

***************************************************************************/

#ifndef __EMINLINE__
#define __EMINLINE__

#if !defined(MAME_NOASM)
/* we come with implementations for GCC x86 and PPC */
#if defined(__GNUC__)

#if defined(__i386__) || defined(__x86_64__)
#include "eigccx86.h"
#elif defined(__ppc__) || defined (__PPC__) || defined(__ppc64__) || defined(__PPC64__)
#include "eigccppc.h"
#else
#error "no matching assembler implementations found - please compile with NOASM=1"
#endif

#else

#if defined(_MSC_VER)

#if (defined(_M_IX86) || defined(_M_X64))
#include "eivcx86.h"
#endif

#include "eivc.h"

#else

#error "no matching assembler implementations found - please compile with NOASM=1"

#endif

#endif
#endif


/***************************************************************************
    INLINE MATH FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    mul_32x32 - perform a signed 32 bit x 32 bit
    multiply and return the full 64 bit result
-------------------------------------------------*/

#ifndef mul_32x32
static inline INT64 mul_32x32(INT32 a, INT32 b)
{
	return (INT64)a * (INT64)b;
}
#endif


/*-------------------------------------------------
    mulu_32x32 - perform an unsigned 32 bit x
    32 bit multiply and return the full 64 bit
    result
-------------------------------------------------*/

#ifndef mulu_32x32
static inline UINT64 mulu_32x32(UINT32 a, UINT32 b)
{
	return (UINT64)a * (UINT64)b;
}
#endif


/*-------------------------------------------------
    mul_32x32_hi - perform a signed 32 bit x 32 bit
    multiply and return the upper 32 bits of the
    result
-------------------------------------------------*/

#ifndef mul_32x32_hi
static inline INT32 mul_32x32_hi(INT32 a, INT32 b)
{
	return (UINT32)(((INT64)a * (INT64)b) >> 32);
}
#endif


/*-------------------------------------------------
    mulu_32x32_hi - perform an unsigned 32 bit x
    32 bit multiply and return the upper 32 bits
    of the result
-------------------------------------------------*/

#ifndef mulu_32x32_hi
static inline UINT32 mulu_32x32_hi(UINT32 a, UINT32 b)
{
	return (UINT32)(((UINT64)a * (UINT64)b) >> 32);
}
#endif


/*-------------------------------------------------
    mul_32x32_shift - perform a signed 32 bit x
    32 bit multiply and shift the result by the
    given number of bits before truncating the
    result to 32 bits
-------------------------------------------------*/

#ifndef mul_32x32_shift
static inline INT32 mul_32x32_shift(INT32 a, INT32 b, UINT8 shift)
{
	return (INT32)(((INT64)a * (INT64)b) >> shift);
}
#endif


/*-------------------------------------------------
    mulu_32x32_shift - perform an unsigned 32 bit x
    32 bit multiply and shift the result by the
    given number of bits before truncating the
    result to 32 bits
-------------------------------------------------*/

#ifndef mulu_32x32_shift
static inline UINT32 mulu_32x32_shift(UINT32 a, UINT32 b, UINT8 shift)
{
	return (UINT32)(((UINT64)a * (UINT64)b) >> shift);
}
#endif


/*-------------------------------------------------
    div_64x32 - perform a signed 64 bit x 32 bit
    divide and return the 32 bit quotient
-------------------------------------------------*/

#ifndef div_64x32
static inline INT32 div_64x32(INT64 a, INT32 b)
{
	return a / (INT64)b;
}
#endif


/*-------------------------------------------------
    divu_64x32 - perform an unsigned 64 bit x 32 bit
    divide and return the 32 bit quotient
-------------------------------------------------*/

#ifndef divu_64x32
static inline UINT32 divu_64x32(UINT64 a, UINT32 b)
{
	return a / (UINT64)b;
}
#endif


/*-------------------------------------------------
    div_64x32_rem - perform a signed 64 bit x 32
    bit divide and return the 32 bit quotient and
    32 bit remainder
-------------------------------------------------*/

#ifndef div_64x32_rem
static inline INT32 div_64x32_rem(INT64 a, INT32 b, INT32 *remainder)
{
	INT32 res = div_64x32(a, b);
	*remainder = a - ((INT64)b * res);
	return res;
}
#endif


/*-------------------------------------------------
    divu_64x32_rem - perform an unsigned 64 bit x
    32 bit divide and return the 32 bit quotient
    and 32 bit remainder
-------------------------------------------------*/

#ifndef divu_64x32_rem
static inline UINT32 divu_64x32_rem(UINT64 a, UINT32 b, UINT32 *remainder)
{
	UINT32 res = divu_64x32(a, b);
	*remainder = a - ((UINT64)b * res);
	return res;
}
#endif


/*-------------------------------------------------
    div_32x32_shift - perform a signed divide of
    two 32 bit values, shifting the first before
    division, and returning the 32 bit quotient
-------------------------------------------------*/

#ifndef div_32x32_shift
static inline INT32 div_32x32_shift(INT32 a, INT32 b, UINT8 shift)
{
	return ((INT64)a << shift) / (INT64)b;
}
#endif


/*-------------------------------------------------
    divu_32x32_shift - perform an unsigned divide of
    two 32 bit values, shifting the first before
    division, and returning the 32 bit quotient
-------------------------------------------------*/

#ifndef divu_32x32_shift
static inline UINT32 divu_32x32_shift(UINT32 a, UINT32 b, UINT8 shift)
{
	return ((UINT64)a << shift) / (UINT64)b;
}
#endif


/*-------------------------------------------------
    mod_64x32 - perform a signed 64 bit x 32 bit
    divide and return the 32 bit remainder
-------------------------------------------------*/

#ifndef mod_64x32
static inline INT32 mod_64x32(INT64 a, INT32 b)
{
	return a - (b * div_64x32(a, b));
}
#endif


/*-------------------------------------------------
    modu_64x32 - perform an unsigned 64 bit x 32 bit
    divide and return the 32 bit remainder
-------------------------------------------------*/

#ifndef modu_64x32
static inline UINT32 modu_64x32(UINT64 a, UINT32 b)
{
	return a - (b * divu_64x32(a, b));
}
#endif


/*-------------------------------------------------
    recip_approx - compute an approximate floating
    point reciprocal
-------------------------------------------------*/

#ifndef recip_approx
static inline float recip_approx(float value)
{
	return 1.0f / value;
}
#endif



/***************************************************************************
    INLINE BIT MANIPULATION FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    count_leading_zeros - return the number of
    leading zero bits in a 32-bit value
-------------------------------------------------*/

#ifndef count_leading_zeros
static inline UINT8 count_leading_zeros(UINT32 val)
{
	UINT8 count;
	for (count = 0; (INT32)val >= 0; count++) val <<= 1;
	return count;
}
#endif


/*-------------------------------------------------
    count_leading_ones - return the number of
    leading one bits in a 32-bit value
-------------------------------------------------*/

#ifndef count_leading_ones
static inline UINT8 count_leading_ones(UINT32 val)
{
	UINT8 count;
	for (count = 0; (INT32)val < 0; count++) val <<= 1;
	return count;
}
#endif


/***************************************************************************
    INLINE TIMING FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    get_profile_ticks - return a tick counter
    from the processor that can be used for
    profiling. It does not need to run at any
    particular rate.
-------------------------------------------------*/

#ifndef get_profile_ticks
static inline INT64 get_profile_ticks(void)
{
	return osd_ticks();
}
#endif

#endif /* __EMINLINE__ */
