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



/***************************************************************************
    INLINE SYNCHRONIZATION FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    compare_exchange32 - compare the 'compare'
    value against the memory at 'ptr'; if equal,
    swap in the 'exchange' value. Regardless,
    return the previous value at 'ptr'.
-------------------------------------------------*/

#define compare_exchange32 _compare_exchange32
static inline INT32 ATTR_NONNULL(1) ATTR_FORCE_INLINE
_compare_exchange32(INT32 volatile *ptr, INT32 compare, INT32 exchange)
{
	INT32 result;

	__asm__ __volatile__ (
		"1: lwarx  %[result], 0, %[ptr]   \n"
		"   cmpw   %[compare], %[result]  \n"
		"   bne    2f                     \n"
		"   stwcx. %[exchange], 0, %[ptr] \n"
		"   bne-   1b                     \n"
		"   lwsync                        \n"
		"2:                                 "
		: [dummy]    "+m"  (*ptr)   /* Lets GCC know that *ptr will be read/written in case it's not marked volatile */
		, [result]   "=&r" (result)
		: [ptr]      "r"   (ptr)
		, [exchange] "r"   (exchange)
		, [compare]  "r"   (compare)
		: "cr0"
	);

	return result;
}


/*-------------------------------------------------
    compare_exchange64 - compare the 'compare'
    value against the memory at 'ptr'; if equal,
    swap in the 'exchange' value. Regardless,
    return the previous value at 'ptr'.
-------------------------------------------------*/

#if defined(__ppc64__) || defined(__PPC64__)
#define compare_exchange64 _compare_exchange64
static inline INT64 ATTR_NONNULL(1) ATTR_FORCE_INLINE
_compare_exchange64(INT64 volatile *ptr, INT64 compare, INT64 exchange)
{
	INT64 result;

	__asm__ __volatile__ (
		"1: ldarx  %[result], 0, %[ptr]   \n"
		"   cmpd   %[compare], %[result]  \n"
		"   bne    2f                     \n"
		"   stdcx. %[exchange], 0, %[ptr] \n"
		"   bne-   1b                     \n"
		"   lwsync                        \n"
		"2:                                 "
		: [dummy]    "+m"  (*ptr)   /* Lets GCC know that *ptr will be read/written in case it's not marked volatile */
		, [result]   "=&r" (result)
		: [ptr]      "r"   (ptr)
		, [exchange] "r"   (exchange)
		, [compare]  "r"   (compare)
		: "cr0"
	);

	return result;
}
#endif


/*-------------------------------------------------
    atomic_exchange32 - atomically exchange the
    exchange value with the memory at 'ptr',
    returning the original value.
-------------------------------------------------*/

#define atomic_exchange32 _atomic_exchange32
static inline INT32 ATTR_NONNULL(1) ATTR_FORCE_INLINE
_atomic_exchange32(INT32 volatile *ptr, INT32 exchange)
{
	INT32 result;

	__asm__ __volatile__ (
		"1: lwarx  %[result], 0, %[ptr]   \n"
		"   stwcx. %[exchange], 0, %[ptr] \n"
		"   bne-   1b                     \n"
		"   lwsync                        \n"
		: [dummy]    "+m"  (*ptr)   /* Lets GCC know that *ptr will be read/written in case it's not marked volatile */
		, [result]   "=&r" (result)
		: [ptr]      "r"   (ptr)
		, [exchange] "r"   (exchange)
		: "cr0"
	);

	return result;
}


/*-------------------------------------------------
    atomic_add32 - atomically add the delta value
    to the memory at 'ptr', returning the final
    result.
-------------------------------------------------*/

#define atomic_add32 _atomic_add32
static inline INT32 ATTR_NONNULL(1) ATTR_FORCE_INLINE
_atomic_add32(INT32 volatile *ptr, INT32 delta)
{
	INT32 result;

	__asm__ __volatile__ (
		"1: lwarx  %[result], 0, %[ptr]           \n"
		"   add    %[result], %[result], %[delta] \n"
		"   stwcx. %[result], 0, %[ptr]           \n"
		"   bne-   1b                             \n"
		"   lwsync                                \n"
		: [dummy]  "+m"  (*ptr) /* Lets GCC know that *ptr will be read/written in case it's not marked volatile */
		, [result] "=&b" (result)
		: [ptr]    "r"   (ptr)
		, [delta]  "r"   (delta)
		: "cr0"
	);

	return result;
}


/*-------------------------------------------------
    atomic_increment32 - atomically increment the
    32-bit value in memory at 'ptr', returning the
    final result.
-------------------------------------------------*/

#define atomic_increment32 _atomic_increment32
static inline INT32 ATTR_NONNULL(1) ATTR_FORCE_INLINE
_atomic_increment32(INT32 volatile *ptr)
{
	INT32 result;

	__asm__ __volatile__ (
		"1: lwarx   %[result], 0, %[ptr]    \n"
		"   addi    %[result], %[result], 1 \n"
		"   stwcx.  %[result], 0, %[ptr]    \n"
		"   bne-    1b                      \n"
		"   lwsync                          \n"
		: [dummy]  "+m"  (*ptr) /* Lets GCC know that *ptr will be read/written in case it's not marked volatile */
		, [result] "=&b" (result)
		: [ptr]    "r"   (ptr)
		: "cr0"
	);

	return result;
}


/*-------------------------------------------------
    atomic_decrement32 - atomically decrement the
    32-bit value in memory at 'ptr', returning the
    final result.
-------------------------------------------------*/

#define atomic_decrement32 _atomic_decrement32
static inline INT32 ATTR_NONNULL(1) ATTR_FORCE_INLINE
_atomic_decrement32(INT32 volatile *ptr)
{
	INT32 result;

	__asm__ __volatile__ (
		"1: lwarx   %[result], 0, %[ptr]     \n"
		"   addi    %[result], %[result], -1 \n"
		"   stwcx.  %[result], 0, %[ptr]     \n"
		"   bne-    1b                       \n"
		"   lwsync                           \n"
		: [dummy]  "+m"  (*ptr) /* Lets GCC know that *ptr will be read/written in case it's not marked volatile */
		, [result] "=&b" (result)
		: [ptr]    "r"   (ptr)
		: "cr0"
	);

	return result;
}



/***************************************************************************
    INLINE TIMING FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    get_profile_ticks - return a tick counter
    from the processor that can be used for
    profiling. It does not need to run at any
    particular rate.
-------------------------------------------------*/

#define get_profile_ticks _get_profile_ticks
static inline INT64 ATTR_UNUSED ATTR_FORCE_INLINE _get_profile_ticks(void)
{
	// fix me - should use the time base
	return osd_ticks();
}

#endif /* __EIGCCPPC__ */
