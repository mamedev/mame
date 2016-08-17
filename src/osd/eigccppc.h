// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    eigccppc.h

    PowerPC (32 and 64-bit) inline implementations for GCC compilers. This
    code is automatically included if appropriate by eminline.h.

***************************************************************************/

#ifndef __EIGCCPPC__
#define __EIGCCPPC__


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
static inline INT32 ATTR_CONST ATTR_FORCE_INLINE
_mul_32x32_hi(INT32 val1, INT32 val2)
{
	INT32 result;

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
static inline UINT32 ATTR_CONST ATTR_FORCE_INLINE
_mulu_32x32_hi(UINT32 val1, UINT32 val2)
{
	UINT32 result;

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
static inline INT32 ATTR_CONST ATTR_FORCE_INLINE
_mul_32x32_shift(INT32 val1, INT32 val2, UINT8 shift)
{
	INT32 result;

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
static inline UINT32 ATTR_CONST ATTR_FORCE_INLINE
_mulu_32x32_shift(UINT32 val1, UINT32 val2, UINT8 shift)
{
	UINT32 result;

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
static inline UINT8 ATTR_CONST ATTR_FORCE_INLINE
_count_leading_zeros(UINT32 value)
{
	UINT32 result;

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
static inline UINT8 ATTR_CONST ATTR_FORCE_INLINE
_count_leading_ones(UINT32 value)
{
	UINT32 result;

	__asm__ (
		" not     %[result], %[value]  \n"
		" cntlzw  %[result], %[result] \n"
		: [result] "=r" (result)    /* result can be in any register */
		: [value]  "r"  (value)     /* 'value' can be in any register */
	);

	return result;
}

#endif /* __EIGCCPPC__ */
