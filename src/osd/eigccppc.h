// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    eigccppc.h

    PowerPC (32 and 64-bit) inline implementations for GCC compilers. This
    code is automatically included if appropriate by eminline.h.

***************************************************************************/

#ifndef MAME_OSD_EIGCCPPC_H
#define MAME_OSD_EIGCCPPC_H


/***************************************************************************
    INLINE MATH FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    mul_32x32 - perform a signed 32 bit x 32 bit
    multiply and return the full 64 bit result
-------------------------------------------------*/

/* GCC can do a good job of this. */


/*-------------------------------------------------
    mulu_32x32 - perform an unsigned 32 bit x
    32 bit multiply and return the full 64 bit
    result
-------------------------------------------------*/

/* GCC can do a good job of this */


/*-------------------------------------------------
    mul_32x32_hi - perform a signed 32 bit x 32 bit
    multiply and return the upper 32 bits of the
    result
-------------------------------------------------*/

#define mul_32x32_hi _mul_32x32_hi
static inline int32_t ATTR_CONST ATTR_FORCE_INLINE
_mul_32x32_hi(int32_t val1, int32_t val2)
{
	int32_t result;

	__asm__ (
		" mulhw  %[result], %[val1], %[val2] \n"
		: [result] "=r" (result)
		: [val1]   "%r" (val1)
		, [val2]   "r"  (val2)
	);

	return result;
}


/*-------------------------------------------------
    mulu_32x32_hi - perform an unsigned 32 bit x
    32 bit multiply and return the upper 32 bits
    of the result
-------------------------------------------------*/

#define mulu_32x32_hi _mulu_32x32_hi
static inline uint32_t ATTR_CONST ATTR_FORCE_INLINE
_mulu_32x32_hi(uint32_t val1, uint32_t val2)
{
	uint32_t result;

	__asm__ (
		" mulhwu  %[result], %[val1], %[val2] \n"
		: [result] "=r" (result)
		: [val1]   "%r" (val1)
		, [val2]   "r"  (val2)
	);

	return result;
}


/*-------------------------------------------------
    mul_32x32_shift - perform a signed 32 bit x
    32 bit multiply and shift the result by the
    given number of bits before truncating the
    result to 32 bits
-------------------------------------------------*/

#if !defined(__ppc64__) && !defined(__PPC64__) && !defined(_ARCH_PPC64)
#define mul_32x32_shift _mul_32x32_shift
static inline int32_t ATTR_CONST ATTR_FORCE_INLINE
_mul_32x32_shift(int32_t val1, int32_t val2, uint8_t shift)
{
	int32_t result;

	/* Valid for (0 <= shift <= 32) */
	__asm__ (
		" mullw   %[result], %[val1], %[val2]    \n"
		" mulhw   %[val1], %[val1], %[val2]      \n"
		" srw     %[result], %[result], %[shift] \n"
		" subfic  %[shift], %[shift], 0x20       \n"
		" slw     %[val1], %[val1], %[shift]     \n"
		" or      %[result], %[result], %[val1]  \n"
		: [result] "=&r" (result)
		, [shift]  "+r"  (shift)
		, [val1]   "+r"  (val1)
		: [val2]   "r"   (val2)
		: "xer"
	);

	return result;
}
#endif


/*-------------------------------------------------
    mulu_32x32_shift - perform an unsigned 32 bit x
    32 bit multiply and shift the result by the
    given number of bits before truncating the
    result to 32 bits
-------------------------------------------------*/

#if !defined(__ppc64__) && !defined(__PPC64__) && !defined(_ARCH_PPC64)
#define mulu_32x32_shift _mulu_32x32_shift
static inline uint32_t ATTR_CONST ATTR_FORCE_INLINE
_mulu_32x32_shift(uint32_t val1, uint32_t val2, uint8_t shift)
{
	uint32_t result;

	/* Valid for (0 <= shift <= 32) */
	__asm__ (
		" mullw   %[result], %[val1], %[val2]    \n"
		" mulhwu  %[val1], %[val1], %[val2]      \n"
		" srw     %[result], %[result], %[shift] \n"
		" subfic  %[shift], %[shift], 0x20       \n"
		" slw     %[val1], %[val1], %[shift]     \n"
		" or      %[result], %[result], %[val1]  \n"
		: [result] "=&r" (result)
		, [shift]  "+r"  (shift)
		, [val1]   "+r"  (val1)
		: [val2]   "r"   (val2)
		: "xer"
	);

	return result;
}
#endif


/*-------------------------------------------------
    div_64x32 - perform a signed 64 bit x 32 bit
    divide and return the 32 bit quotient
-------------------------------------------------*/

/* TBD */


/*-------------------------------------------------
    divu_64x32 - perform an unsigned 64 bit x 32 bit
    divide and return the 32 bit quotient
-------------------------------------------------*/

/* TBD */


/*-------------------------------------------------
    div_64x32_rem - perform a signed 64 bit x 32
    bit divide and return the 32 bit quotient and
    32 bit remainder
-------------------------------------------------*/

/* TBD */


/*-------------------------------------------------
    divu_64x32_rem - perform an unsigned 64 bit x
    32 bit divide and return the 32 bit quotient
    and 32 bit remainder
-------------------------------------------------*/

/* TBD */


/*-------------------------------------------------
    div_32x32_shift - perform a signed divide of
    two 32 bit values, shifting the first before
    division, and returning the 32 bit quotient
-------------------------------------------------*/

/* TBD */


/*-------------------------------------------------
    divu_32x32_shift - perform an unsigned divide of
    two 32 bit values, shifting the first before
    division, and returning the 32 bit quotient
-------------------------------------------------*/

/* TBD */


/*-------------------------------------------------
    mod_64x32 - perform a signed 64 bit x 32 bit
    divide and return the 32 bit remainder
-------------------------------------------------*/

/* TBD */


/*-------------------------------------------------
    modu_64x32 - perform an unsigned 64 bit x 32 bit
    divide and return the 32 bit remainder
-------------------------------------------------*/

/* TBD */


/*-------------------------------------------------
    recip_approx - compute an approximate floating
    point reciprocal
-------------------------------------------------*/

#define recip_approx _recip_approx
static inline float ATTR_CONST ATTR_FORCE_INLINE
_recip_approx(float value)
{
	float result;

	__asm__ (
		" fres  %[result], %[value] \n"
		: [result] "=f" (result)
		: [value]  "f"  (value)
	);

	return result;
}



/***************************************************************************
    INLINE BIT MANIPULATION FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    count_leading_zeros - return the number of
    leading zero bits in a 32-bit value
-------------------------------------------------*/

#define count_leading_zeros _count_leading_zeros
static inline uint8_t ATTR_CONST ATTR_FORCE_INLINE
_count_leading_zeros(uint32_t value)
{
	uint32_t result;

	__asm__ (
		" cntlzw  %[result], %[value] \n"
		: [result] "=r" (result)    /* result can be in any register */
		: [value]  "r"  (value)     /* 'value' can be in any register */
	);

	return result;
}


/*-------------------------------------------------
    count_leading_ones - return the number of
    leading one bits in a 32-bit value
-------------------------------------------------*/

#define count_leading_ones _count_leading_ones
static inline uint8_t ATTR_CONST ATTR_FORCE_INLINE
_count_leading_ones(uint32_t value)
{
	uint32_t result;

	__asm__ (
		" cntlzw  %[result], %[result] \n"
		: [result] "=r" (result)    /* result can be in any register */
		: [value]  "r"  (~value)    /* 'value' can be in any register */
	);

	return result;
}

#endif // MAME_OSD_EIGCCPPC_H
