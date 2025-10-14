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
#elif defined(__arm__) || defined(__aarch64__)
#include "eigccarm.h"
#endif

#include "eigcc.h"

#elif defined(_MSC_VER)

#if defined(_M_IX86) || defined(_M_X64)
#include "eivcx86.h"
#elif defined(_M_ARM) || defined(_M_ARM64)
#include "eivcarm.h"
#endif

#include "eivc.h"

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
constexpr int64_t mul_32x32(int32_t a, int32_t b)
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
constexpr uint64_t mulu_32x32(uint32_t a, uint32_t b)
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
constexpr int32_t mul_32x32_hi(int32_t a, int32_t b)
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
constexpr uint32_t mulu_32x32_hi(uint32_t a, uint32_t b)
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
constexpr int32_t mul_32x32_shift(int32_t a, int32_t b, uint8_t shift)
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
constexpr uint32_t mulu_32x32_shift(uint32_t a, uint32_t b, uint8_t shift)
{
	return uint32_t((uint64_t(a) * uint64_t(b)) >> shift);
}
#endif


/*-------------------------------------------------
    div_64x32 - perform a signed 64 bit x 32 bit
    divide and return the 32 bit quotient
-------------------------------------------------*/

#ifndef div_64x32
constexpr int32_t div_64x32(int64_t a, int32_t b)
{
	return a / int64_t(b);
}
#endif


/*-------------------------------------------------
    divu_64x32 - perform an unsigned 64 bit x 32 bit
    divide and return the 32 bit quotient
-------------------------------------------------*/

#ifndef divu_64x32
constexpr uint32_t divu_64x32(uint64_t a, uint32_t b)
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
inline int32_t div_64x32_rem(int64_t a, int32_t b, int32_t &remainder)
{
	int32_t const res(div_64x32(a, b));
	remainder = a - (int64_t(b) * res);
	return res;
}
#endif


/*-------------------------------------------------
    divu_64x32_rem - perform an unsigned 64 bit x
    32 bit divide and return the 32 bit quotient
    and 32 bit remainder
-------------------------------------------------*/

#ifndef divu_64x32_rem
inline uint32_t divu_64x32_rem(uint64_t a, uint32_t b, uint32_t &remainder)
{
	uint32_t const res(divu_64x32(a, b));
	remainder = a - (uint64_t(b) * res);
	return res;
}
#endif


/*-------------------------------------------------
    div_32x32_shift - perform a signed divide of
    two 32 bit values, shifting the first before
    division, and returning the 32 bit quotient
-------------------------------------------------*/

#ifndef div_32x32_shift
constexpr int32_t div_32x32_shift(int32_t a, int32_t b, uint8_t shift)
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
constexpr uint32_t divu_32x32_shift(uint32_t a, uint32_t b, uint8_t shift)
{
	return (uint64_t(a) << shift) / uint64_t(b);
}
#endif


/*-------------------------------------------------
    mod_64x32 - perform a signed 64 bit x 32 bit
    divide and return the 32 bit remainder
-------------------------------------------------*/

#ifndef mod_64x32
constexpr int32_t mod_64x32(int64_t a, int32_t b)
{
	return a - (b * div_64x32(a, b));
}
#endif


/*-------------------------------------------------
    modu_64x32 - perform an unsigned 64 bit x 32 bit
    divide and return the 32 bit remainder
-------------------------------------------------*/

#ifndef modu_64x32
constexpr uint32_t modu_64x32(uint64_t a, uint32_t b)
{
	return a - (b * divu_64x32(a, b));
}
#endif


/*-------------------------------------------------
    recip_approx - compute an approximate floating
    point reciprocal
-------------------------------------------------*/

#ifndef recip_approx
constexpr float recip_approx(float value)
{
	return 1.0f / value;
}
#endif


/*-------------------------------------------------
    mul_64x64 - perform a signed 64 bit x 64 bit
    multiply and return the full 128 bit result
-------------------------------------------------*/

#ifndef mul_64x64
inline int64_t mul_64x64(int64_t a, int64_t b, int64_t &hi)
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

	hi = ab_hi + (ab_m1 >> 32) + (ab_m2 >> 32) + carry;

	// adjust for sign
	if (a < 0)
		hi -= b;
	if (b < 0)
		hi -= a;

	return ab_lo + (ab_m1 << 32) + (ab_m2 << 32);
}
#endif


/*-------------------------------------------------
    mulu_64x64 - perform an unsigned 64 bit x 64
    bit multiply and return the full 128 bit result
-------------------------------------------------*/

#ifndef mulu_64x64
inline uint64_t mulu_64x64(uint64_t a, uint64_t b, uint64_t &hi)
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

	hi = ab_hi + (ab_m1 >> 32) + (ab_m2 >> 32) + carry;

	return ab_lo + (ab_m1 << 32) + (ab_m2 << 32);
}
#endif


/*-------------------------------------------------
    addu_32x32_co - perform an unsigned 32 bit + 32
    bit addition and return the result with carry
    out
-------------------------------------------------*/

#ifndef addu_32x32_co
inline bool addu_32x32_co(uint32_t a, uint32_t b, uint32_t &sum)
{
	sum = a + b;
	return (a > sum) || (b > sum);
}
#endif


/*-------------------------------------------------
    addu_64x64_co - perform an unsigned 64 bit + 64
    bit addition and return the result with carry
    out
-------------------------------------------------*/

#ifndef addu_64x64_co
inline bool addu_64x64_co(uint64_t a, uint64_t b, uint64_t &sum)
{
	sum = a + b;
	return (a > sum) || (b > sum);
}
#endif


/*-------------------------------------------------
    muldivu_64 - perform an unsigned 64 bits a*b/c,
    rounding toward zero.  Unpredictable (including
    crashes) when the result does not fit in 64
    bits.
-------------------------------------------------*/

#ifndef muldivu_64
inline uint64_t muldivu_64(uint64_t a, uint64_t b, uint64_t c)
{
	// Borrowed from https://stackoverflow.com/questions/8733178/most-accurate-way-to-do-a-combined-multiply-and-divide-operation-in-64-bit

	constexpr uint64_t base = 1ULL<<32;
	constexpr uint64_t maxdiv = (base-1)*base + (base-1);

	// First get the easy thing
	uint64_t res = (a/c) * b + (a%c) * (b/c);
	a %= c;
	b %= c;
	// Are we done?
	if (a == 0 || b == 0)
		return res;
	// Is it easy to compute what remain to be added?
	if (c < base)
		return res + (a*b/c);
	// Now 0 < a < c, 0 < b < c, c >= 1ULL
	// Normalize
	uint64_t norm = maxdiv/c;
	c *= norm;
	a *= norm;
	// split into 2 digits
	uint64_t ah = a / base, al = a % base;
	uint64_t bh = b / base, bl = b % base;
	uint64_t ch = c / base, cl = c % base;
	// compute the product
	uint64_t p0 = al*bl;
	uint64_t p1 = p0 / base + al*bh;
	p0 %= base;
	uint64_t p2 = p1 / base + ah*bh;
	p1 = (p1 % base) + ah * bl;
	p2 += p1 / base;
	p1 %= base;
	// p2 holds 2 digits, p1 and p0 one

	// first digit is easy, not null only in case of overflow
	//  uint64_t q2 = p2 / c;
	p2 = p2 % c;

	// second digit, estimate
	uint64_t q1 = p2 / ch;
	// and now adjust
	uint64_t rhat = p2 % ch;
	// the loop can be unrolled, it will be executed at most twice for
	// even bases -- three times for odd one -- due to the normalisation above
	while (q1 >= base || (rhat < base && q1*cl > rhat*base+p1)) {
		q1--;
		rhat += ch;
	}
	// subtract
	p1 = ((p2 % base) * base + p1) - q1 * cl;
	p2 = (p2 / base * base + p1 / base) - q1 * ch;
	p1 = p1 % base + (p2 % base) * base;

	// now p1 hold 2 digits, p0 one and p2 is to be ignored
	uint64_t q0 = p1 / ch;
	rhat = p1 % ch;
	while (q0 >= base || (rhat < base && q0*cl > rhat*base+p0)) {
		q0--;
		rhat += ch;
	}
	// we don't need to do the subtraction (needed only to get the remainder,
	// in which case we have to divide it by norm)
	return res + q0 + q1 * base; // + q2 *base*base
}
#endif


/*-------------------------------------------------
    muldivupu_64 - perform an unsigned 64 bits
    a*b/c, rounding away from zero.  Unpredictable
    (including crashes) when the result does not
    fit in 64 bits.
-------------------------------------------------*/

#ifndef muldivupu_64
inline uint64_t muldivupu_64(uint64_t a, uint64_t b, uint64_t c)
{
	// The annoying case is c > 1e32, which is a case we do not use yet so
	// hard to test.

	constexpr uint64_t base = 1ULL<<32;

	// First get the easy thing
	uint64_t res = (a/c) * b + (a%c) * (b/c);
	a %= c;
	b %= c;
	// Are we done?
	if (a == 0 || b == 0)
		return res;
	// Is it easy to compute what remain to be added?
	if (c < base)
		return res + ((a*b+c-1)/c);
	fprintf(stderr, "muldiv64c: Annoying case not handled\n");
	abort();
}
#endif



/***************************************************************************
    INLINE BIT MANIPULATION FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    count_leading_zeros_32 - return the number of
    leading zero bits in a 32-bit value
-------------------------------------------------*/

#ifndef count_leading_zeros_32
inline uint8_t count_leading_zeros_32(uint32_t val)
{
	if (!val) return 32U;
	uint8_t count;
	for (count = 0; int32_t(val) >= 0; count++) val <<= 1;
	return count;
}
#endif


/*-------------------------------------------------
    count_leading_ones_32 - return the number of
    leading one bits in a 32-bit value
-------------------------------------------------*/

#ifndef count_leading_ones_32
inline uint8_t count_leading_ones_32(uint32_t val)
{
	uint8_t count;
	for (count = 0; int32_t(val) < 0; count++) val <<= 1;
	return count;
}
#endif


/*-------------------------------------------------
    count_leading_zeros_64 - return the number of
    leading zero bits in a 64-bit value
-------------------------------------------------*/

#ifndef count_leading_zeros_64
inline uint8_t count_leading_zeros_64(uint64_t val)
{
	if (!val) return 64U;
	uint8_t count;
	for (count = 0; int64_t(val) >= 0; count++) val <<= 1;
	return count;
}
#endif


/*-------------------------------------------------
    count_leading_ones_64 - return the number of
    leading one bits in a 64-bit value
-------------------------------------------------*/

#ifndef count_leading_ones_64
inline uint8_t count_leading_ones_64(uint64_t val)
{
	uint8_t count;
	for (count = 0; int64_t(val) < 0; count++) val <<= 1;
	return count;
}
#endif


/*-------------------------------------------------
    population_count_32 - return the number of
    one bits in a 32-bit value
-------------------------------------------------*/

#ifndef population_count_32
inline unsigned population_count_32(uint32_t val)
{
#if defined(__NetBSD__)
	return popcount32(val);
#else
	// optimal Hamming weight assuming fast 32*32->32
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


/*-------------------------------------------------
    population_count_64 - return the number of
    one bits in a 64-bit value
-------------------------------------------------*/

#ifndef population_count_64
inline unsigned population_count_64(uint64_t val)
{
#if defined(__NetBSD__)
	return popcount64(val);
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


/*-------------------------------------------------
    rotl_32 - circularly shift a 32-bit value left
    by the specified number of bits (modulo 32)
-------------------------------------------------*/

#ifndef rotl_32
constexpr uint32_t rotl_32(uint32_t val, int shift)
{
	shift &= 31;
	if (shift)
		return val << shift | val >> (32 - shift);
	else
		return val;
}
#endif


/*-------------------------------------------------
    rotr_32 - circularly shift a 32-bit value right
    by the specified number of bits (modulo 32)
-------------------------------------------------*/

#ifndef rotr_32
constexpr uint32_t rotr_32(uint32_t val, int shift)
{
	shift &= 31;
	if (shift)
		return val >> shift | val << (32 - shift);
	else
		return val;
}
#endif


/*-------------------------------------------------
    rotl_64 - circularly shift a 64-bit value left
    by the specified number of bits (modulo 64)
-------------------------------------------------*/

#ifndef rotl_64
constexpr uint64_t rotl_64(uint64_t val, int shift)
{
	shift &= 63;
	if (shift)
		return val << shift | val >> (64 - shift);
	else
		return val;
}
#endif


/*-------------------------------------------------
    rotr_64 - circularly shift a 64-bit value right
    by the specified number of bits (modulo 64)
-------------------------------------------------*/

#ifndef rotr_64
constexpr uint64_t rotr_64(uint64_t val, int shift)
{
	shift &= 63;
	if (shift)
		return val >> shift | val << (64 - shift);
	else
		return val;
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
inline int64_t get_profile_ticks() noexcept
{
	return osd_ticks();
}
#endif

#endif // MAME_OSD_EMINLINE_H
