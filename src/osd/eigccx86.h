// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    eigccx86.h

    x86 (32 and 64-bit) inline implementations for GCC compilers. This
    code is automatically included if appropriate by eminline.h.

***************************************************************************/

#ifndef MAME_OSD_EIGCCX86_H
#define MAME_OSD_EIGCCX86_H

// Include MMX/SSE intrinsics headers

#ifdef __SSE2__
#include <cstdlib>
#include <mmintrin.h>   // MMX
#include <xmmintrin.h>  // SSE
#include <emmintrin.h>  // SSE2
#endif


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

// GCC can do a good job of this.


/*-------------------------------------------------
    mul_32x32_hi - perform a signed 32 bit x 32 bit
    multiply and return the upper 32 bits of the
    result
-------------------------------------------------*/

// GCC can do a good job of this.


/*-------------------------------------------------
    mulu_32x32_hi - perform an unsigned 32 bit x
    32 bit multiply and return the upper 32 bits
    of the result
-------------------------------------------------*/

// GCC can do a good job of this.


/*-------------------------------------------------
    mul_32x32_shift - perform a signed 32 bit x
    32 bit multiply and shift the result by the
    given number of bits before truncating the
    result to 32 bits
-------------------------------------------------*/

#ifndef __x86_64__
#define mul_32x32_shift _mul_32x32_shift
inline int32_t ATTR_CONST ATTR_FORCE_INLINE
_mul_32x32_shift(int32_t a, int32_t b, uint8_t shift)
{
	int32_t result;

	// Valid for (0 <= shift <= 31)
	__asm__ (
		" imull  %[b]                       ;"
		" shrdl  %[shift], %%edx, %[result] ;"
		: [result] "=a" (result)    // result ends up in eax
		: [a]      "%0" (a)         // 'a' should also be in eax on entry
		, [b]      "rm" (b)         // 'b' can be memory or register
		, [shift]  "Ic" (shift)     // 'shift' must be constant in 0-31 range or in cl
		: "%edx", "cc"              // clobbers edx and condition codes
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
inline uint32_t ATTR_CONST ATTR_FORCE_INLINE
_mulu_32x32_shift(uint32_t a, uint32_t b, uint8_t shift)
{
	uint32_t result;

	// Valid for (0 <= shift <= 31)
	__asm__ (
		" mull   %[b]                       ;"
		" shrdl  %[shift], %%edx, %[result] ;"
		: [result] "=a" (result)    // result ends up in eax
		: [a]      "%0" (a)         // 'a' should also be in eax on entry
		, [b]      "rm" (b)         // 'b' can be memory or register
		, [shift]  "Ic" (shift)     // 'shift' must be constant in 0-31 range or in cl
		: "%edx", "cc"              // clobbers edx and condition codes
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
inline int32_t ATTR_CONST ATTR_FORCE_INLINE
_div_64x32(int64_t a, int32_t b)
{
	int32_t result, temp;

	// Throws arithmetic exception if result doesn't fit in 32 bits
	__asm__ (
		" idivl  %[b] ;"
		: [result] "=a" (result)    // result ends up in eax
		, [temp]   "=d" (temp)      // this is effectively a clobber
		: [a]      "A"  (a)         // 'a' in edx:eax
		, [b]      "rm" (b)         // 'b' in register or memory
		: "cc"                      // clobbers condition codes
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
inline uint32_t ATTR_CONST ATTR_FORCE_INLINE
_divu_64x32(uint64_t a, uint32_t b)
{
	uint32_t result, temp;

	// Throws arithmetic exception if result doesn't fit in 32 bits
	__asm__ (
		" divl  %[b] ;"
		: [result] "=a" (result)    // result ends up in eax
		, [temp]   "=d" (temp)      // this is effectively a clobber
		: [a]      "A"  (a)         // 'a' in edx:eax
		, [b]      "rm" (b)         // 'b' in register or memory
		: "cc"                      // clobbers condition codes
	);

	return result;
}
#endif


/*-------------------------------------------------
    div_64x32_rem - perform a signed 64 bit x 32
    bit divide and return the 32 bit quotient and
    32 bit remainder
-------------------------------------------------*/

#define div_64x32_rem _div_64x32_rem
inline int32_t ATTR_FORCE_INLINE
_div_64x32_rem(int64_t dividend, int32_t divisor, int32_t &remainder)
{
	int32_t quotient;
#ifndef __x86_64__
	// Throws arithmetic exception if result doesn't fit in 32 bits
	__asm__ (
		" idivl  %[divisor] ;"
		: [result]    "=a" (quotient)   // quotient ends up in eax
		, [remainder] "=d" (remainder)  // remainder ends up in edx
		: [dividend]  "A"  (dividend)   // 'dividend' in edx:eax
		, [divisor]   "rm" (divisor)    // 'divisor' in register or memory
		: "cc"                          // clobbers condition codes
	);
#else
	int32_t const divh{ int32_t(uint32_t(uint64_t(dividend) >> 32)) };
	int32_t const divl{ int32_t(uint32_t(uint64_t(dividend))) };

	// Throws arithmetic exception if result doesn't fit in 32 bits
	__asm__ (
		" idivl  %[divisor] ;"
		: [result]    "=a" (quotient)   // quotient ends up in eax
		, [remainder] "=d" (remainder)  // remainder ends up in edx
		: [divl]      "a"  (divl)       // 'dividend' in edx:eax
		, [divh]      "d"  (divh)
		, [divisor]   "rm" (divisor)    // 'divisor' in register or memory
		: "cc"                          // clobbers condition codes
	);
#endif
	return quotient;
}


/*-------------------------------------------------
    divu_64x32_rem - perform an unsigned 64 bit x
    32 bit divide and return the 32 bit quotient
    and 32 bit remainder
-------------------------------------------------*/

#define divu_64x32_rem _divu_64x32_rem
inline uint32_t ATTR_FORCE_INLINE
_divu_64x32_rem(uint64_t dividend, uint32_t divisor, uint32_t &remainder)
{
	uint32_t quotient;
#ifndef __x86_64__
	// Throws arithmetic exception if result doesn't fit in 32 bits
	__asm__ (
		" divl  %[divisor] ;"
		: [result]    "=a" (quotient)   // quotient ends up in eax
		, [remainder] "=d" (remainder)  // remainder ends up in edx
		: [dividend]  "A"  (dividend)   // 'dividend' in edx:eax
		, [divisor]   "rm" (divisor)    // 'divisor' in register or memory
		: "cc"                          // clobbers condition codes
	);
#else
	uint32_t const divh{ uint32_t(dividend >> 32) };
	uint32_t const divl{ uint32_t(dividend) };

	// Throws arithmetic exception if result doesn't fit in 32 bits
	__asm__ (
		" divl  %[divisor] ;"
		: [result]    "=a" (quotient)   // quotient ends up in eax
		, [remainder] "=d" (remainder)  // remainder ends up in edx
		: [divl]      "a"  (divl)       // 'dividend' in edx:eax
		, [divh]      "d"  (divh)
		, [divisor]   "rm" (divisor)    // 'divisor' in register or memory
		: "cc"                          // clobbers condition codes
	);

#endif
	return quotient;
}


/*-------------------------------------------------
    div_32x32_shift - perform a signed divide of
    two 32 bit values, shifting the first before
    division, and returning the 32 bit quotient
-------------------------------------------------*/

#ifndef __x86_64__
#define div_32x32_shift _div_32x32_shift
inline int32_t ATTR_CONST ATTR_FORCE_INLINE
_div_32x32_shift(int32_t a, int32_t b, uint8_t shift)
{
	int32_t result;

	// Valid for (0 <= shift <= 31)
	// Throws arithmetic exception if result doesn't fit in 32 bits
	__asm__ (
		" cdq                          ;"
		" shldl  %[shift], %[a], %%edx ;"
		" shll   %[shift], %[a]        ;"
		" idivl  %[b]                  ;"
		: [result] "=&a" (result)   // result ends up in eax
		: [a]      "0"   (a)        // 'a' should also be in eax on entry
		, [b]      "rm"  (b)        // 'b' can be memory or register
		, [shift]  "Ic"  (shift)    // 'shift' must be constant in 0-31 range or in cl
		: "%edx", "cc"              // clobbers edx and condition codes
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
inline uint32_t ATTR_CONST ATTR_FORCE_INLINE
_divu_32x32_shift(uint32_t a, uint32_t b, uint8_t shift)
{
	int32_t result;

	// Valid for (0 <= shift <= 31)
	// Throws arithmetic exception if result doesn't fit in 32 bits
	__asm__ (
		" clr    %%edx                 ;"
		" shldl  %[shift], %[a], %%edx ;"
		" shll   %[shift], %[a]        ;"
		" divl   %[b]                  ;"
		: [result] "=&a" (result)   // result ends up in eax
		: [a]      "0"   (a)        // 'a' should also be in eax on entry
		, [b]      "rm"  (b)        // 'b' can be memory or register
		, [shift]  "Ic"  (shift)    // 'shift' must be constant in 0-31 range or in cl
		: "%edx", "cc"              // clobbers edx and condition codes
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
inline int32_t ATTR_CONST ATTR_FORCE_INLINE
_mod_64x32(int64_t a, int32_t b)
{
	int32_t result, temp;

	// Throws arithmetic exception if quotient doesn't fit in 32 bits
	__asm__ (
		" idivl  %[b] ;"
		: [result] "=d" (result)    // Result ends up in edx
		, [temp]   "=a" (temp)      // This is effectively a clobber
		: [a]      "A"  (a)         // 'a' in edx:eax
		, [b]      "rm" (b)         // 'b' in register or memory
		: "cc"                      // Clobbers condition codes
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
inline uint32_t ATTR_CONST ATTR_FORCE_INLINE
_modu_64x32(uint64_t a, uint32_t b)
{
	uint32_t result, temp;

	// Throws arithmetic exception if quotient doesn't fit in 32 bits
	__asm__ (
		" divl  %[b] ;"
		: [result] "=d" (result)    // Result ends up in edx
		, [temp]   "=a" (temp)      // This is effectively a clobber
		: [a]      "A"  (a)         // 'a' in edx:eax
		, [b]      "rm" (b)         // 'b' in register or memory
		: "cc"                      // Clobbers condition codes
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
inline float ATTR_CONST ATTR_FORCE_INLINE
_recip_approx(float value)
{
	__m128 const value_xmm(_mm_set_ss(value));
	__m128 const result_xmm(_mm_rcp_ss(value_xmm));
	float result;
	_mm_store_ss(&result, result_xmm);
	return result;
}
#endif


/*-------------------------------------------------
    muldivu_64 - perform an unsigned 64 bits a*b/c,
    rounding toward zero.  Unpredictable (including
    crashes) when the result does not fit in 64
    bits.
-------------------------------------------------*/

#ifdef __x86_64__
#define muldivu_64 _muldivu_64
inline uint64_t ATTR_FORCE_INLINE
_muldivu_64(uint64_t m1, uint64_t m2, uint64_t d)
{
	uint64_t lower, upper;
	__asm__ (
		" mulq %[m2]"
		: [result]    "=a"  (lower)
		, [upper]     "=d"  (upper)
		: [m1]        "%0"  (m1)
		, [m2]        "rm"  (m2)
		: "cc"
	);
	uint64_t quotient, remainder;
	__asm__ (
		" divq %[d]"
		: [result]    "=a"  (quotient)
		, [remainder] "=d"  (remainder)
		: [lower]     "0"   (lower)
		, [upper]     "1"   (upper)
		, [d]         "rm"  (d)
		: "cc"
	);
	return quotient;
}
#endif


/*-------------------------------------------------
    muldivupu_64 - perform an unsigned 64 bits
    a*b/c, rounding away from zero.  Unpredictable
    (including crashes) when the result does not
    fit in 64 bits.
-------------------------------------------------*/

#ifdef __x86_64__
#define muldivupu_64 _muldivupu_64
inline uint64_t _muldivupu_64(uint64_t m1, uint64_t m2, uint64_t d)
{
	uint64_t lower, upper;
	__asm__ (
		" mulq %[m2]"
		: [result]    "=a"  (lower)
		, [upper]     "=d"  (upper)
		: [m1]        "%0"  (m1)
		, [m2]        "rm"  (m2)
		: "cc"
	);
	uint64_t quotient, remainder;
	__asm__ (
		" divq %[d]"
		: [result]    "=a"  (quotient)
		, [remainder] "=d"  (remainder)
		: [lower]     "0"   (lower)
		, [upper]     "1"   (upper)
		, [d]         "rm"  (d)
		: "cc"
	);
	return quotient + (remainder ? 1 : 0);
}
#endif



/***************************************************************************
    INLINE BIT MANIPULATION FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    count_leading_zeros_32 - return the number of
    leading zero bits in a 32-bit value
-------------------------------------------------*/

#define count_leading_zeros_32 _count_leading_zeros_32
inline uint8_t ATTR_CONST ATTR_FORCE_INLINE
_count_leading_zeros_32(uint32_t value)
{
	uint32_t result;
	__asm__ (
		" bsrl    %[value], %[result] ;"
		" cmovzl  %[bias], %[result]  ;"
		: [result] "=&r" (result)       // result can be in any register
		: [value]  "rm"  (value)        // 'value' can be register or memory
		, [bias]   "rm"  (~uint32_t(0)) // 'bias' can be register or memory
		: "cc"                          // clobbers condition codes
	);
	return uint8_t(31U - result);
}


/*-------------------------------------------------
    count_leading_ones_32 - return the number of
    leading one bits in a 32-bit value
-------------------------------------------------*/

#define count_leading_ones_32 _count_leading_ones_32
inline uint8_t ATTR_CONST ATTR_FORCE_INLINE
_count_leading_ones_32(uint32_t value)
{
	uint32_t result;
	__asm__ (
		" bsrl    %[value], %[result] ;"
		" cmovzl  %[bias], %[result]  ;"
		: [result] "=&r" (result)       // result can be in any register
		: [value]  "rm"  (~value)       // 'value' can be register or memory
		, [bias]   "rm"  (~uint32_t(0)) // 'bias' can be register or memory
		: "cc"                          // clobbers condition codes
	);
	return uint8_t(31U - result);
}


/*-------------------------------------------------
    count_leading_zeros_64 - return the number of
    leading zero bits in a 64-bit value
-------------------------------------------------*/

#ifdef __x86_64__
#define count_leading_zeros_64 _count_leading_zeros_64
inline uint8_t ATTR_CONST ATTR_FORCE_INLINE
_count_leading_zeros_64(uint64_t value)
{
	uint64_t result;
	__asm__ (
		" bsrq    %[value], %[result] ;"
		" cmovzq  %[bias], %[result]  ;"
		: [result] "=&r" (result)       // result can be in any register
		: [value]  "rm"  (value)        // 'value' can be register or memory
		, [bias]   "rm"  (~uint64_t(0)) // 'bias' can be register or memory
		: "cc"                          // clobbers condition codes
	);
	return uint8_t(63U - result);
}
#endif


/*-------------------------------------------------
    count_leading_ones_64 - return the number of
    leading one bits in a 64-bit value
-------------------------------------------------*/

#ifdef __x86_64__
#define count_leading_ones_64 _count_leading_ones_64
inline uint8_t ATTR_CONST ATTR_FORCE_INLINE
_count_leading_ones_64(uint64_t value)
{
	uint64_t result;
	__asm__ (
		" bsrq    %[value], %[result] ;"
		" cmovzq  %[bias], %[result]  ;"
		: [result] "=&r" (result)       // result can be in any register
		: [value]  "rm"  (~value)       // 'value' can be register or memory
		, [bias]   "rm"  (~uint64_t(0)) // 'bias' can be register or memory
		: "cc"                          // clobbers condition codes
	);
	return uint8_t(63U - result);
}
#endif


/*-------------------------------------------------
    rotl_32 - circularly shift a 32-bit value left
    by the specified number of bits (modulo 32)
-------------------------------------------------*/

#define rotl_32 _rotl_32
inline uint32_t ATTR_CONST ATTR_FORCE_INLINE
_rotl_32(uint32_t val, int shift)
{
	uint32_t result;
	__asm__ (
		" roll %[shift], %[value] ;"
		: [result] "=rm" (result)                   // result can be in register or memory
		: [value]  "%0" (val)                       // 'value' is updated with result
		, [shift]  "Ic" (uint8_t(unsigned(shift)))  // 'shift' must be constant in 0-31 range or in cl
		: "cc"                                      // clobbers condition codes
	);
	return result;
}


/*-------------------------------------------------
    rotr_32 - circularly shift a 32-bit value right
    by the specified number of bits (modulo 32)
-------------------------------------------------*/

#define rotr_32 _rotr_32
inline uint32_t ATTR_CONST ATTR_FORCE_INLINE
rotr_32(uint32_t val, int shift)
{
	uint32_t result;
	__asm__ (
		" rorl %[shift], %[value] ;"
		: [result] "=rm" (result)                   // result can be in register or memory
		: [value]  "%0" (val)                       // 'value' is updated with result
		, [shift]  "Ic" (uint8_t(unsigned(shift)))  // 'shift' must be constant in 0-31 range or in cl
		: "cc"                                      // clobbers condition codes
	);
	return result;
}


/*-------------------------------------------------
    rotl_64 - circularly shift a 64-bit value left
    by the specified number of bits (modulo 64)
-------------------------------------------------*/

#ifdef __x86_64__
#define rotl_64 _rotl_64
inline uint64_t ATTR_CONST ATTR_FORCE_INLINE
_rotl_64(uint64_t val, int shift)
{
	uint64_t result;
	__asm__ (
		" rolq %[shift], %[value] ;"
		: [result] "=rm" (result)                   // result can be in register or memory
		: [value]  "%0" (val)                       // 'value' is updated with result
		, [shift]  "Jc" (uint8_t(unsigned(shift)))  // 'shift' must be constant in 0-63 range or in cl
		: "cc"                                      // clobbers condition codes
	);
	return result;
}
#endif


/*-------------------------------------------------
    rotr_64 - circularly shift a 64-bit value right
    by the specified number of bits (modulo 64)
-------------------------------------------------*/

#ifdef __x86_64__
#define rotr_64 _rotr_64
inline uint64_t ATTR_CONST ATTR_FORCE_INLINE
rotr_64(uint64_t val, int shift)
{
	uint64_t result;
	__asm__ (
		" rorq %[shift], %[value] ;"
		: [result] "=rm" (result)                   // result can be in register or memory
		: [value]  "%0" (val)                       // 'value' is updated with result
		, [shift]  "Jc" (uint8_t(unsigned(shift)))  // 'shift' must be constant in 0-63 range or in cl
		: "cc"                                      // clobbers condition codes
	);
	return result;
}
#endif

#endif // MAME_OSD_EIGCCX86_H
