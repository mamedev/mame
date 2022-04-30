// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    eigccarm.h

    ARM/AArch64 inline implementations for GCC compilers. This code is
    automatically included if appropriate by eminline.h.

***************************************************************************/

#ifndef MAME_OSD_EIGCCARM_H
#define MAME_OSD_EIGCCARM_H


/***************************************************************************
    INLINE MATH FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    mul_32x32 - perform a signed 32 bit x 32 bit
    multiply and return the full 64 bit result
-------------------------------------------------*/

// GCC can do a good job of this.


/*-------------------------------------------------
    mulu_32x32 - perform an unsigned 32 bit x
    32 bit multiply and return the full 64 bit
    result
-------------------------------------------------*/

// GCC can do a good job of this


/*-------------------------------------------------
    mul_32x32_hi - perform a signed 32 bit x 32 bit
    multiply and return the upper 32 bits of the
    result
-------------------------------------------------*/

// GCC can do a good job of this


/*-------------------------------------------------
    mulu_32x32_hi - perform an unsigned 32 bit x
    32 bit multiply and return the upper 32 bits
    of the result
-------------------------------------------------*/

// GCC can do a good job of this


/*-------------------------------------------------
    mul_32x32_shift - perform a signed 32 bit x
    32 bit multiply and shift the result by the
    given number of bits before truncating the
    result to 32 bits
-------------------------------------------------*/

#if !defined(__aarch64__)
#define mul_32x32_shift _mul_32x32_shift
inline int32_t ATTR_CONST ATTR_FORCE_INLINE
_mul_32x32_shift(int32_t val1, int32_t val2, uint8_t shift)
{
	uint32_t l, h;

	__asm__ (
		" smull  %[l], %[h], %[val1], %[val2] \n"
		: [l]      "=r" (l)
		, [h]      "=r" (h)
		: [val1]   "%r" (val1)
		, [val2]   "r"  (val2)
	);

	// Valid for (0 <= shift <= 31)
	return int32_t((l >> shift) | (h << (32 - shift)));
}
#endif


/*-------------------------------------------------
    mulu_32x32_shift - perform an unsigned 32 bit x
    32 bit multiply and shift the result by the
    given number of bits before truncating the
    result to 32 bits
-------------------------------------------------*/

#if !defined(__aarch64__)
#define mulu_32x32_shift _mulu_32x32_shift
inline uint32_t ATTR_CONST ATTR_FORCE_INLINE
_mulu_32x32_shift(uint32_t val1, uint32_t val2, uint8_t shift)
{
	uint32_t l, h;

	__asm__ (
		" umull  %[l], %[h], %[val1], %[val2] \n"
		: [l]      "=r" (l)
		, [h]      "=r" (h)
		: [val1]   "%r" (val1)
		, [val2]   "r"  (val2)
	);

	// Valid for (0 <= shift <= 31)
	return (l >> shift) | (h << (32 - shift));
}
#endif


/*-------------------------------------------------
    div_64x32 - perform a signed 64 bit x 32 bit
    divide and return the 32 bit quotient
-------------------------------------------------*/

// TBD


/*-------------------------------------------------
    divu_64x32 - perform an unsigned 64 bit x 32 bit
    divide and return the 32 bit quotient
-------------------------------------------------*/

// TBD


/*-------------------------------------------------
    div_64x32_rem - perform a signed 64 bit x 32
    bit divide and return the 32 bit quotient and
    32 bit remainder
-------------------------------------------------*/

// TBD


/*-------------------------------------------------
    divu_64x32_rem - perform an unsigned 64 bit x
    32 bit divide and return the 32 bit quotient
    and 32 bit remainder
-------------------------------------------------*/

// TBD


/*-------------------------------------------------
    div_32x32_shift - perform a signed divide of
    two 32 bit values, shifting the first before
    division, and returning the 32 bit quotient
-------------------------------------------------*/

// TBD


/*-------------------------------------------------
    divu_32x32_shift - perform an unsigned divide of
    two 32 bit values, shifting the first before
    division, and returning the 32 bit quotient
-------------------------------------------------*/

// TBD


/*-------------------------------------------------
    mod_64x32 - perform a signed 64 bit x 32 bit
    divide and return the 32 bit remainder
-------------------------------------------------*/

// TBD


/*-------------------------------------------------
    modu_64x32 - perform an unsigned 64 bit x 32 bit
    divide and return the 32 bit remainder
-------------------------------------------------*/

// TBD


/*-------------------------------------------------
    recip_approx - compute an approximate floating
    point reciprocal
-------------------------------------------------*/

#if defined(__aarch64__)
#define recip_approx _recip_approx
inline float ATTR_CONST ATTR_FORCE_INLINE
_recip_approx(float value)
{
	float result;

	__asm__ (
		" frecpe  %s[result], %s[value] \n"
		: [result] "=w" (result)
		: [value]  "w"  (value)
	);

	return result;
}
#endif


/*-------------------------------------------------
    mul_64x64 - perform a signed 64 bit x 64 bit
    multiply and return the full 128 bit result
-------------------------------------------------*/

#if defined(__aarch64__)
#define mul_64x64 _mul_64x64
inline int64_t ATTR_FORCE_INLINE
_mul_64x64(int64_t a, int64_t b, int64_t &hi)
{
	__int128 const r(__int128(a) * b);
	hi = int64_t(uint64_t((unsigned __int128)r >> 64));
	return int64_t(uint64_t((unsigned __int128)r));
}
#endif


/*-------------------------------------------------
    mulu_64x64 - perform an unsigned 64 bit x 64
    bit multiply and return the full 128 bit result
-------------------------------------------------*/

#if defined(__aarch64__)
#define mulu_64x64 _mulu_64x64
inline uint64_t ATTR_FORCE_INLINE
_mulu_64x64(uint64_t a, uint64_t b, uint64_t &hi)
{
	unsigned __int128 const r((unsigned __int128)a * b);
	hi = uint64_t(r >> 64);
	return uint64_t(r);
}
#endif



/***************************************************************************
    INLINE BIT MANIPULATION FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    count_leading_zeros_32 - return the number of
    leading zero bits in a 32-bit value
-------------------------------------------------*/

#if defined(__aarch64__)
#define count_leading_zeros_32 _count_leading_zeros_32
inline uint8_t ATTR_CONST ATTR_FORCE_INLINE
_count_leading_zeros_32(uint32_t value)
{
	uint32_t result;

	__asm__ (
		" clz  %w[result], %w[value] \n"
		: [result] "=r" (result)
		: [value]  "r"  (value)
	);

	return uint8_t(result);
}
#endif


/*-------------------------------------------------
    count_leading_ones_32 - return the number of
    leading one bits in a 32-bit value
-------------------------------------------------*/

#if defined(__aarch64__)
#define count_leading_ones_32 _count_leading_ones_32
inline uint8_t ATTR_CONST ATTR_FORCE_INLINE
_count_leading_ones_32(uint32_t value)
{
	uint32_t result;

	__asm__ (
		" clz  %w[result], %w[value] \n"
		: [result] "=r" (result)
		: [value]  "r"  (~value)
	);

	return uint8_t(result);
}
#endif


/*-------------------------------------------------
    count_leading_zeros_64 - return the number of
    leading zero bits in a 64-bit value
-------------------------------------------------*/

#if defined(__aarch64__)
#define count_leading_zeros_64 _count_leading_zeros_64
inline uint8_t ATTR_CONST ATTR_FORCE_INLINE
_count_leading_zeros_64(uint64_t value)
{
	uint64_t result;

	__asm__ (
		" clz  %[result], %[value] \n"
		: [result] "=r" (result)
		: [value]  "r"  (value)
	);

	return uint8_t(result);
}
#endif


/*-------------------------------------------------
    count_leading_ones_64 - return the number of
    leading one bits in a 64-bit value
-------------------------------------------------*/

#if defined(__aarch64__)
#define count_leading_ones_64 _count_leading_ones_64
inline uint8_t ATTR_CONST ATTR_FORCE_INLINE
_count_leading_ones_64(uint64_t value)
{
	uint64_t result;

	__asm__ (
		" clz  %[result], %[value] \n"
		: [result] "=r" (result)
		: [value]  "r"  (~value)
	);

	return uint8_t(result);
}
#endif

#endif // MAME_OSD_EIGCCARM_H
