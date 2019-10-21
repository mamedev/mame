// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    eminline.h

    Definitions for inline functions that can be overridden by OSD-
    specific code.

***************************************************************************/

#ifndef MAME_OSD_EMINLINE_H
#define MAME_OSD_EMINLINE_H

#pragma once

#include "osdcomm.h"
#include "osdcore.h"

#if !defined(MAME_NOASM)

#if defined(__GNUC__)

#if defined(__i386__) || defined(__x86_64__)
#include "eigccx86.h"
#elif defined(__ppc__) || defined (__PPC__) || defined(__ppc64__) || defined(__PPC64__)
#include "eigccppc.h"
#else
#error "no matching assembler implementations found - please compile with NOASM=1"
#endif

#elif defined(_MSC_VER)

#if (defined(_M_IX86) || defined(_M_X64))
#include "eivcx86.h"
#endif

#include "eivc.h"

#else

#error "no matching assembler implementations found - please compile with NOASM=1"

#endif

#endif // !defined(MAME_NOASM)


/***************************************************************************
    INLINE MATH FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    mul_32x32 - perform a signed 32 bit x 32 bit
    multiply and return the full 64 bit result
-------------------------------------------------*/

#ifndef mul_32x32
inline int64_t mul_32x32(int32_t a, int32_t b)
{
	return int64_t(a) * int64_t(b);
}
#endif


/*-------------------------------------------------
    mulu_32x32 - perform an unsigned 32 bit x
    32 bit multiply and return the full 64 bit
    result
-------------------------------------------------*/

#ifndef mulu_32x32
inline uint64_t mulu_32x32(uint32_t a, uint32_t b)
{
	return uint64_t(a) * uint64_t(b);
}
#endif


/*-------------------------------------------------
    mul_32x32_hi - perform a signed 32 bit x 32 bit
    multiply and return the upper 32 bits of the
    result
-------------------------------------------------*/

#ifndef mul_32x32_hi
inline int32_t mul_32x32_hi(int32_t a, int32_t b)
{
	return uint32_t((int64_t(a) * int64_t(b)) >> 32);
}
#endif


/*-------------------------------------------------
    mulu_32x32_hi - perform an unsigned 32 bit x
    32 bit multiply and return the upper 32 bits
    of the result
-------------------------------------------------*/

#ifndef mulu_32x32_hi
inline uint32_t mulu_32x32_hi(uint32_t a, uint32_t b)
{
	return uint32_t((uint64_t(a) * uint64_t(b)) >> 32);
}
#endif


/*-------------------------------------------------
    mul_32x32_shift - perform a signed 32 bit x
    32 bit multiply and shift the result by the
    given number of bits before truncating the
    result to 32 bits
-------------------------------------------------*/

#ifndef mul_32x32_shift
inline int32_t mul_32x32_shift(int32_t a, int32_t b, uint8_t shift)
{
	return int32_t((int64_t(a) * int64_t(b)) >> shift);
}
#endif


/*-------------------------------------------------
    mulu_32x32_shift - perform an unsigned 32 bit x
    32 bit multiply and shift the result by the
    given number of bits before truncating the
    result to 32 bits
-------------------------------------------------*/

#ifndef mulu_32x32_shift
inline uint32_t mulu_32x32_shift(uint32_t a, uint32_t b, uint8_t shift)
{
	return uint32_t((uint64_t(a) * uint64_t(b)) >> shift);
}
#endif


/*-------------------------------------------------
    div_64x32 - perform a signed 64 bit x 32 bit
    divide and return the 32 bit quotient
-------------------------------------------------*/

#ifndef div_64x32
inline int32_t div_64x32(int64_t a, int32_t b)
{
	return a / int64_t(b);
}
#endif


/*-------------------------------------------------
    divu_64x32 - perform an unsigned 64 bit x 32 bit
    divide and return the 32 bit quotient
-------------------------------------------------*/

#ifndef divu_64x32
inline uint32_t divu_64x32(uint64_t a, uint32_t b)
{
	return a / uint64_t(b);
}
#endif


/*-------------------------------------------------
    div_64x32_rem - perform a signed 64 bit x 32
    bit divide and return the 32 bit quotient and
    32 bit remainder
-------------------------------------------------*/

#ifndef div_64x32_rem
inline int32_t div_64x32_rem(int64_t a, int32_t b, int32_t *remainder)
{
	int32_t const res = div_64x32(a, b);
	*remainder = a - (int64_t(b) * res);
	return res;
}
#endif


/*-------------------------------------------------
    divu_64x32_rem - perform an unsigned 64 bit x
    32 bit divide and return the 32 bit quotient
    and 32 bit remainder
-------------------------------------------------*/

#ifndef divu_64x32_rem
inline uint32_t divu_64x32_rem(uint64_t a, uint32_t b, uint32_t *remainder)
{
	uint32_t const res = divu_64x32(a, b);
	*remainder = a - (uint64_t(b) * res);
	return res;
}
#endif


/*-------------------------------------------------
    div_32x32_shift - perform a signed divide of
    two 32 bit values, shifting the first before
    division, and returning the 32 bit quotient
-------------------------------------------------*/

#ifndef div_32x32_shift
inline int32_t div_32x32_shift(int32_t a, int32_t b, uint8_t shift)
{
	return (int64_t(a) << shift) / int64_t(b);
}
#endif


/*-------------------------------------------------
    divu_32x32_shift - perform an unsigned divide of
    two 32 bit values, shifting the first before
    division, and returning the 32 bit quotient
-------------------------------------------------*/

#ifndef divu_32x32_shift
inline uint32_t divu_32x32_shift(uint32_t a, uint32_t b, uint8_t shift)
{
	return (uint64_t(a) << shift) / uint64_t(b);
}
#endif


/*-------------------------------------------------
    mod_64x32 - perform a signed 64 bit x 32 bit
    divide and return the 32 bit remainder
-------------------------------------------------*/

#ifndef mod_64x32
inline int32_t mod_64x32(int64_t a, int32_t b)
{
	return a - (b * div_64x32(a, b));
}
#endif


/*-------------------------------------------------
    modu_64x32 - perform an unsigned 64 bit x 32 bit
    divide and return the 32 bit remainder
-------------------------------------------------*/

#ifndef modu_64x32
inline uint32_t modu_64x32(uint64_t a, uint32_t b)
{
	return a - (b * divu_64x32(a, b));
}
#endif


/*-------------------------------------------------
    recip_approx - compute an approximate floating
    point reciprocal
-------------------------------------------------*/

#ifndef recip_approx
inline float recip_approx(float value)
{
	return 1.0f / value;
}
#endif


/*-------------------------------------------------
    mul_64x64 - perform a signed 64 bit x 64 bit
    multiply and return the full 128 bit result
-------------------------------------------------*/

#ifndef mul_64x64
inline int64_t mul_64x64(int64_t a, int64_t b, int64_t *hi)
{
	uint64_t const a_hi = uint64_t(a) >> 32;
	uint64_t const b_hi = uint64_t(b) >> 32;
	uint64_t const a_lo = uint32_t(uint64_t(a));
	uint64_t const b_lo = uint32_t(uint64_t(b));

	uint64_t const ab_lo = a_lo * b_lo;
	uint64_t const ab_m1 = a_hi * b_lo;
	uint64_t const ab_m2 = a_lo * b_hi;
	uint64_t const ab_hi = a_hi * b_hi;
	uint64_t const carry = ((ab_lo >> 32) + uint32_t(ab_m1) + uint32_t(ab_m2)) >> 32;

	*hi = ab_hi + (ab_m1 >> 32) + (ab_m2 >> 32) + carry;

	// adjust for sign
	if (a < 0)
		*hi -= b;
	if (b < 0)
		*hi -= a;

	return ab_lo + (ab_m1 << 32) + (ab_m2 << 32);
}
#endif


/*-------------------------------------------------
    mulu_64x64 - perform an unsigned 64 bit x 64
    bit multiply and return the full 128 bit result
-------------------------------------------------*/

#ifndef mulu_64x64
inline uint64_t mulu_64x64(uint64_t a, uint64_t b, uint64_t *hi)
{
	uint64_t const a_hi = uint32_t(a >> 32);
	uint64_t const b_hi = uint32_t(b >> 32);
	uint64_t const a_lo = uint32_t(a);
	uint64_t const b_lo = uint32_t(b);

	uint64_t const ab_lo = a_lo * b_lo;
	uint64_t const ab_m1 = a_hi * b_lo;
	uint64_t const ab_m2 = a_lo * b_hi;
	uint64_t const ab_hi = a_hi * b_hi;
	uint64_t const carry = ((ab_lo >> 32) + uint32_t(ab_m1) + uint32_t(ab_m2)) >> 32;

	*hi = ab_hi + (ab_m1 >> 32) + (ab_m2 >> 32) + carry;

	return ab_lo + (ab_m1 << 32) + (ab_m2 << 32);
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
inline uint8_t count_leading_zeros(uint32_t val)
{
	if (!val) return 32U;
	uint8_t count;
	for (count = 0; int32_t(val) >= 0; count++) val <<= 1;
	return count;
}
#endif


/*-------------------------------------------------
    count_leading_ones - return the number of
    leading one bits in a 32-bit value
-------------------------------------------------*/

#ifndef count_leading_ones
inline uint8_t count_leading_ones(uint32_t val)
{
	uint8_t count;
	for (count = 0; int32_t(val) < 0; count++) val <<= 1;
	return count;
}
#endif


/*-------------------------------------------------
    population_count_32 - return the number of
    one bits in a 32-bit value
-------------------------------------------------*/

#ifndef population_count_32
#if defined(__NetBSD__)
#define population_count_32 popcount32
#else
inline unsigned population_count_32(uint32_t val)
{
#if defined(__GNUC__)
	// uses CPU feature if available, otherwise falls back to implementation similar to what follows
	static_assert(sizeof(val) == sizeof(unsigned), "expected 32-bit unsigned int");
	return unsigned(__builtin_popcount(static_cast<unsigned>(val)));
#else
	// optimal Hamming weight assuing fast 32*32->32
	constexpr uint32_t m1(0x55555555);
	constexpr uint32_t m2(0x33333333);
	constexpr uint32_t m4(0x0f0f0f0f);
	constexpr uint32_t h01(0x01010101);
	val -= (val >> 1) & m1;
	val = (val & m2) + ((val >> 2) & m2);
	val = (val + (val >> 4)) & m4;
	return unsigned((val * h01) >> 24);
#endif
}
#endif
#endif


/*-------------------------------------------------
    population_count_64 - return the number of
    one bits in a 64-bit value
-------------------------------------------------*/

#ifndef population_count_64
#if defined(__NetBSD__)
#define population_count_64 popcount64
#else
inline unsigned population_count_64(uint64_t val)
{
#if defined(__GNUC__)
	// uses CPU feature if available, otherwise falls back to implementation similar to what follows
	static_assert(sizeof(val) == sizeof(unsigned long long), "expected 64-bit unsigned long long int");
	return unsigned(__builtin_popcountll(static_cast<unsigned long long>(val)));
#else
	// guess that architectures with 64-bit pointers have 64-bit multiplier
	if (sizeof(void *) >= sizeof(uint64_t))
	{
		// optimal Hamming weight assuming fast 64*64->64
		constexpr uint64_t m1(0x5555555555555555);
		constexpr uint64_t m2(0x3333333333333333);
		constexpr uint64_t m4(0x0f0f0f0f0f0f0f0f);
		constexpr uint64_t h01(0x0101010101010101);
		val -= (val >> 1) & m1;
		val = (val & m2) + ((val >> 2) & m2);
		val = (val + (val >> 4)) & m4;
		return unsigned((val * h01) >> 56);
	}
	else
	{
		// fall back to two 32-bit operations to avoid slow multiply
		return population_count_32(uint32_t(val)) + population_count_32(uint32_t(val >> 32));
	}
#endif
}
#endif
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
inline int64_t get_profile_ticks()
{
	return osd_ticks();
}
#endif

#endif // MAME_OSD_EMINLINE_H
