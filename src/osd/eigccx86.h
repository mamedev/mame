// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    eigccx86.h

    x86 (32 and 64-bit) inline implementations for GCC compilers. This
    code is automatically included if appropriate by eminline.h.

***************************************************************************/

#ifndef __EIGCCX86__
#define __EIGCCX86__

/* Include MMX/SSE intrinsics headers */

#ifdef __SSE2__
#include <stdlib.h>
#include <mmintrin.h>   /* MMX */
#include <xmmintrin.h>  /* SSE */
#include <emmintrin.h>  /* SSE2 */
#endif


/***************************************************************************
    INLINE MATH FUNCTIONS
***************************************************************************/

union _x86_union
{
	UINT64 u64;
	struct {
		UINT32 l, h;
	} u32;
};

/*-------------------------------------------------
    mul_32x32 - perform a signed 32 bit x 32 bit
    multiply and return the full 64 bit result
-------------------------------------------------*/

#ifndef __x86_64__
#define mul_32x32 _mul_32x32
static inline INT64 ATTR_CONST ATTR_FORCE_INLINE
_mul_32x32(INT32 a, INT32 b)
{
	INT64 result;

	__asm__ (
		" imull  %[b] ;"
		: [result] "=A" (result)    /* result in edx:eax */
		: [a]      "%a"  (a)        /* 'a' should also be in eax on entry */
		, [b]      "rm"  (b)        /* 'b' can be memory or register */
		: "cc"                      /* Clobbers condition codes */
	);

	return result;
}
#endif


/*-------------------------------------------------
    mulu_32x32 - perform an unsigned 32 bit x
    32 bit multiply and return the full 64 bit
    result
-------------------------------------------------*/

#ifndef __x86_64__
#define mulu_32x32 _mulu_32x32
static inline UINT64 ATTR_CONST ATTR_FORCE_INLINE
_mulu_32x32(UINT32 a, UINT32 b)
{
	UINT64 result;

	__asm__ (
		" mull  %[b] ;"
		: [result] "=A" (result)    /* result in edx:eax */
		: [a]      "%a"  (a)        /* 'a' should also be in eax on entry */
		, [b]      "rm"  (b)        /* 'b' can be memory or register */
		: "cc"                      /* Clobbers condition codes */
	);

	return result;
}
#endif


/*-------------------------------------------------
    mul_32x32_hi - perform a signed 32 bit x 32 bit
    multiply and return the upper 32 bits of the
    result
-------------------------------------------------*/

#define mul_32x32_hi _mul_32x32_hi
static inline INT32 ATTR_CONST ATTR_FORCE_INLINE
_mul_32x32_hi(INT32 a, INT32 b)
{
	INT32 result, temp;

	__asm__ (
		" imull  %[b] ;"
		: [result] "=d"  (result)   /* result in edx */
		, [temp]   "=a"  (temp)     /* This is effectively a clobber */
		: [a]      "a"  (a)        /* 'a' should be in eax on entry */
		, [b]      "rm"  (b)        /* 'b' can be memory or register */
		: "cc"                      /* Clobbers condition codes */
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
_mulu_32x32_hi(UINT32 a, UINT32 b)
{
	UINT32 result, temp;

	__asm__ (
		" mull  %[b] ;"
		: [result] "=d"  (result)   /* result in edx */
		, [temp]   "=a"  (temp)     /* This is effectively a clobber */
		: [a]      "a"   (a)        /* 'a' should be in eax on entry */
		, [b]      "rm"  (b)        /* 'b' can be memory or register */
		: "cc"                      /* Clobbers condition codes */
	);

	return result;
}


/*-------------------------------------------------
    mul_32x32_shift - perform a signed 32 bit x
    32 bit multiply and shift the result by the
    given number of bits before truncating the
    result to 32 bits
-------------------------------------------------*/

#ifndef __x86_64__
#define mul_32x32_shift _mul_32x32_shift
static inline INT32 ATTR_CONST ATTR_FORCE_INLINE
_mul_32x32_shift(INT32 a, INT32 b, UINT8 shift)
{
	INT32 result;

	/* Valid for (0 <= shift <= 31) */
	__asm__ (
		" imull  %[b]                       ;"
		" shrdl  %[shift], %%edx, %[result] ;"
		: [result] "=a" (result)    /* result ends up in eax */
		: [a]      "%0" (a)         /* 'a' should also be in eax on entry */
		, [b]      "rm" (b)         /* 'b' can be memory or register */
		, [shift]  "Ic" (shift)     /* 'shift' must be constant in 0-31 range or in cl */
		: "%edx", "cc"              /* clobbers edx and condition codes */
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

#ifndef __x86_64__
#define mulu_32x32_shift _mulu_32x32_shift
static inline UINT32 ATTR_CONST ATTR_FORCE_INLINE
_mulu_32x32_shift(UINT32 a, UINT32 b, UINT8 shift)
{
	UINT32 result;

	/* Valid for (0 <= shift <= 31) */
	__asm__ (
		" mull   %[b]                       ;"
		" shrdl  %[shift], %%edx, %[result] ;"
		: [result] "=a" (result)    /* result ends up in eax */
		: [a]      "%0" (a)         /* 'a' should also be in eax on entry */
		, [b]      "rm" (b)         /* 'b' can be memory or register */
		, [shift]  "Ic" (shift)     /* 'shift' must be constant in 0-31 range or in cl */
		: "%edx", "cc"              /* clobbers edx and condition codes */
	);

	return result;
}
#endif


/*-------------------------------------------------
    div_64x32 - perform a signed 64 bit x 32 bit
    divide and return the 32 bit quotient
-------------------------------------------------*/

#ifndef __x86_64__
#define div_64x32 _div_64x32
static inline INT32 ATTR_CONST ATTR_FORCE_INLINE
_div_64x32(INT64 a, INT32 b)
{
	INT32 result, temp;

	/* Throws arithmetic exception if result doesn't fit in 32 bits */
	__asm__ (
		" idivl  %[b] ;"
		: [result] "=a" (result)    /* Result ends up in eax */
		, [temp]   "=d" (temp)      /* This is effectively a clobber */
		: [a]      "A"  (a)         /* 'a' in edx:eax */
		, [b]      "rm" (b)         /* 'b' in register or memory */
		: "cc"                      /* Clobbers condition codes */
	);

	return result;
}
#endif


/*-------------------------------------------------
    divu_64x32 - perform an unsigned 64 bit x 32 bit
    divide and return the 32 bit quotient
-------------------------------------------------*/

#ifndef __x86_64__
#define divu_64x32 _divu_64x32
static inline UINT32 ATTR_CONST ATTR_FORCE_INLINE
_divu_64x32(UINT64 a, UINT32 b)
{
	UINT32 result, temp;

	/* Throws arithmetic exception if result doesn't fit in 32 bits */
	__asm__ (
		" divl  %[b] ;"
		: [result] "=a" (result)    /* Result ends up in eax */
		, [temp]   "=d" (temp)      /* This is effectively a clobber */
		: [a]      "A"  (a)         /* 'a' in edx:eax */
		, [b]      "rm" (b)         /* 'b' in register or memory */
		: "cc"                      /* Clobbers condition codes */
	);

	return result;
}
#endif


/*-------------------------------------------------
    div_64x32_rem - perform a signed 64 bit x 32
    bit divide and return the 32 bit quotient and
    32 bit remainder
-------------------------------------------------*/

#ifndef __x86_64__
#define div_64x32_rem _div_64x32_rem
static inline INT32 ATTR_FORCE_INLINE
_div_64x32_rem(INT64 dividend, INT32 divisor, INT32 *remainder)
{
	INT32 quotient;

	/* Throws arithmetic exception if result doesn't fit in 32 bits */
	__asm__ (
		" idivl  %[divisor] ;"
		: [result]    "=a" (quotient)   /* Quotient ends up in eax */
		, [remainder] "=d" (*remainder) /* Remainder ends up in edx */
		: [dividend]  "A"  (dividend)   /* 'dividend' in edx:eax */
		, [divisor]   "rm" (divisor)    /* 'divisor' in register or memory */
		: "cc"                          /* Clobbers condition codes */
	);

	return quotient;
}
#endif


/*-------------------------------------------------
    divu_64x32_rem - perform an unsigned 64 bit x
    32 bit divide and return the 32 bit quotient
    and 32 bit remainder
-------------------------------------------------*/

#ifndef __x86_64__
#define divu_64x32_rem _divu_64x32_rem
static inline UINT32 ATTR_FORCE_INLINE
_divu_64x32_rem(UINT64 dividend, UINT32 divisor, UINT32 *remainder)
{
	UINT32 quotient;

	/* Throws arithmetic exception if result doesn't fit in 32 bits */
	__asm__ (
		" divl  %[divisor] ;"
		: [result]    "=a" (quotient)   /* Quotient ends up in eax */
		, [remainder] "=d" (*remainder) /* Remainder ends up in edx */
		: [dividend]  "A"  (dividend)   /* 'dividend' in edx:eax */
		, [divisor]   "rm" (divisor)    /* 'divisor' in register or memory */
		: "cc"                          /* Clobbers condition codes */
	);

	return quotient;
}
#else
#define divu_64x32_rem _divu_64x32_rem
static inline UINT32 ATTR_FORCE_INLINE
_divu_64x32_rem(UINT64 dividend, UINT32 divisor, UINT32 *remainder)
{
	UINT32 quotient;
	_x86_union r;

	r.u64 = dividend;

	/* Throws arithmetic exception if result doesn't fit in 32 bits */
	__asm__ (
		" divl  %[divisor] ;"
		: [result]    "=a" (quotient)   /* Quotient ends up in eax */
		, [remainder] "=d" (*remainder) /* Remainder ends up in edx */
		: [divl]  "a"  (r.u32.l)        /* 'dividend' in edx:eax */
		, [divh]  "d"  (r.u32.h)
		, [divisor]   "rm" (divisor)    /* 'divisor' in register or memory */
		: "cc"                          /* Clobbers condition codes */
	);

	return quotient;
}
#endif


/*-------------------------------------------------
    div_32x32_shift - perform a signed divide of
    two 32 bit values, shifting the first before
    division, and returning the 32 bit quotient
-------------------------------------------------*/

#ifndef __x86_64__
#define div_32x32_shift _div_32x32_shift
static inline INT32 ATTR_CONST ATTR_FORCE_INLINE
_div_32x32_shift(INT32 a, INT32 b, UINT8 shift)
{
	INT32 result;

	/* Valid for (0 <= shift <= 31) */
	/* Throws arithmetic exception if result doesn't fit in 32 bits */
	__asm__ (
		" cdq                          ;"
		" shldl  %[shift], %[a], %%edx ;"
		" shll   %[shift], %[a]        ;"
		" idivl  %[b]                  ;"
		: [result] "=&a" (result)   /* result ends up in eax */
		: [a]      "0"   (a)        /* 'a' should also be in eax on entry */
		, [b]      "rm"  (b)        /* 'b' can be memory or register */
		, [shift]  "Ic"  (shift)    /* 'shift' must be constant in 0-31 range or in cl */
		: "%edx", "cc"              /* clobbers edx and condition codes */
	);

	return result;
}
#endif


/*-------------------------------------------------
    divu_32x32_shift - perform an unsigned divide of
    two 32 bit values, shifting the first before
    division, and returning the 32 bit quotient
-------------------------------------------------*/

#ifndef __x86_64__
#define divu_32x32_shift _divu_32x32_shift
static inline UINT32 ATTR_CONST ATTR_FORCE_INLINE
_divu_32x32_shift(UINT32 a, UINT32 b, UINT8 shift)
{
	INT32 result;

	/* Valid for (0 <= shift <= 31) */
	/* Throws arithmetic exception if result doesn't fit in 32 bits */
	__asm__ (
		" clr    %%edx                 ;"
		" shldl  %[shift], %[a], %%edx ;"
		" shll   %[shift], %[a]        ;"
		" divl   %[b]                  ;"
		: [result] "=&a" (result)   /* result ends up in eax */
		: [a]      "0"   (a)        /* 'a' should also be in eax on entry */
		, [b]      "rm"  (b)        /* 'b' can be memory or register */
		, [shift]  "Ic"  (shift)    /* 'shift' must be constant in 0-31 range or in cl */
		: "%edx", "cc"              /* clobbers edx and condition codes */
	);

	return result;
}
#endif


/*-------------------------------------------------
    mod_64x32 - perform a signed 64 bit x 32 bit
    divide and return the 32 bit remainder
-------------------------------------------------*/

#ifndef __x86_64__
#define mod_64x32 _mod_64x32
static inline INT32 ATTR_CONST ATTR_FORCE_INLINE
_mod_64x32(INT64 a, INT32 b)
{
	INT32 result, temp;

	/* Throws arithmetic exception if quotient doesn't fit in 32 bits */
	__asm__ (
		" idivl  %[b] ;"
		: [result] "=d" (result)    /* Result ends up in edx */
		, [temp]   "=a" (temp)      /* This is effectively a clobber */
		: [a]      "A"  (a)         /* 'a' in edx:eax */
		, [b]      "rm" (b)         /* 'b' in register or memory */
		: "cc"                      /* Clobbers condition codes */
	);

	return result;
}
#endif


/*-------------------------------------------------
    modu_64x32 - perform an unsigned 64 bit x 32 bit
    divide and return the 32 bit remainder
-------------------------------------------------*/

#ifndef __x86_64__
#define modu_64x32 _modu_64x32
static inline UINT32 ATTR_CONST ATTR_FORCE_INLINE
_modu_64x32(UINT64 a, UINT32 b)
{
	UINT32 result, temp;

	/* Throws arithmetic exception if quotient doesn't fit in 32 bits */
	__asm__ (
		" divl  %[b] ;"
		: [result] "=d" (result)    /* Result ends up in edx */
		, [temp]   "=a" (temp)      /* This is effectively a clobber */
		: [a]      "A"  (a)         /* 'a' in edx:eax */
		, [b]      "rm" (b)         /* 'b' in register or memory */
		: "cc"                      /* Clobbers condition codes */
	);

	return result;
}
#endif


/*-------------------------------------------------
    recip_approx - compute an approximate floating
    point reciprocal
-------------------------------------------------*/

#ifdef __SSE2__
#define recip_approx _recip_approx
static inline float ATTR_CONST
_recip_approx(float value)
{
	__m128 value_xmm = _mm_set_ss(value);
	__m128 result_xmm = _mm_rcp_ss(value_xmm);
	float result;
	_mm_store_ss(&result, result_xmm);
	return result;
}
#endif



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
		"   bsrl  %[value], %[result] ;"
		"   jnz   1f                  ;"
		"   movl  $63, %[result]      ;"
		"1: xorl  $31, %[result]      ;"
		: [result] "=r" (result)    /* result can be in any register */
		: [value]  "rm" (value)     /* 'value' can be register or memory */
		: "cc"                      /* clobbers condition codes */
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
		"   movl  %[value], %[result]  ;"
		"   notl  %[result]            ;"
		"   bsrl  %[result], %[result] ;"
		"   jnz   1f                   ;"
		"   movl  $63, %[result]       ;"
		"1: xorl  $31, %[result]       ;"
		: [result] "=r"  (result)   /* result can be in any register */
		: [value]  "rmi" (value)    /* 'value' can be register, memory or immediate */
		: "cc"                      /* clobbers condition codes */
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
		" lock ; cmpxchgl  %[exchange], %[ptr] ;"
		: [ptr]      "+m" (*ptr)
		, [result]   "=a" (result)
		: [compare]  "1"  (compare)
		, [exchange] "q"  (exchange)
		: "cc"
	);

	return result;
}


/*-------------------------------------------------
    compare_exchange64 - compare the 'compare'
    value against the memory at 'ptr'; if equal,
    swap in the 'exchange' value. Regardless,
    return the previous value at 'ptr'.
-------------------------------------------------*/

#ifdef __x86_64__
#define compare_exchange64 _compare_exchange64
static inline INT64 ATTR_NONNULL(1) ATTR_FORCE_INLINE
_compare_exchange64(INT64 volatile *ptr, INT64 compare, INT64 exchange)
{
	INT64 result;

	__asm__ __volatile__ (
		" lock ; cmpxchgq  %[exchange], %[ptr] ;"
		: [ptr]      "+m" (*ptr)
		, [result]   "=a" (result)
		: [compare]  "1"  (compare)
		, [exchange] "q"  (exchange)
		: "cc"
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
		" lock ; xchgl  %[exchange], %[ptr] ;"
		: [ptr]      "+m" (*ptr)
		, [result]   "=r" (result)
		: [exchange] "1"  (exchange)
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
	INT32 result = delta;

	__asm__ __volatile__ (
		" lock ; xaddl  %[result], %[ptr] ;"
		: [ptr]    "+m" (*ptr)
		, [result] "+r" (result)
		:
		: "cc"
	);

	return result + delta;
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
	INT32 result = 1;

	__asm__ __volatile__ (
		" lock ; xaddl  %[result], %[ptr] ;"
		: [ptr]    "+m" (*ptr)
		, [result] "+r" (result)
		:
		: "cc"
	);

	return result + 1;
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
	INT32 result = -1;

	__asm__ __volatile__ (
		" lock ; xaddl  %[result], %[ptr] ;"
		: [ptr]    "+m" (*ptr)
		, [result] "+r" (result)
		:
		: "cc"
	);

	return result - 1;
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

#ifndef __x86_64__
static inline UINT64 ATTR_UNUSED ATTR_FORCE_INLINE _get_profile_ticks(void)
{
	UINT64 result;
	__asm__ __volatile__ (
			"rdtsc"
			: "=A" (result)
	);
	return result;
}
#else
static inline UINT64 ATTR_UNUSED ATTR_FORCE_INLINE _get_profile_ticks(void)
{
	_x86_union r;
	__asm__ __volatile__ (
			"rdtsc"
			: "=a" (r.u32.l), "=d" (r.u32.h)
	);

	return (UINT64) r.u64;
}
#endif

#endif /* __EIGCCX86__ */
