// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    eigccppc.h

    Inline implementations for GCC compilers. This code is automatically
    included if appropriate by eminline.h.

***************************************************************************/

#ifndef MAME_OSD_EIGCC_H
#define MAME_OSD_EIGCC_H

#include <cassert>


/***************************************************************************
    INLINE MATH FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    mul_64x64 - perform a signed 64 bit x 64 bit
    multiply and return the full 128 bit result
-------------------------------------------------*/

#if !defined(mulu_64x64) && defined(__SIZEOF_INT128__)
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

#if !defined(mulu_64x64) && defined(__SIZEOF_INT128__)
#define mulu_64x64 _mulu_64x64
inline uint64_t ATTR_FORCE_INLINE
_mulu_64x64(uint64_t a, uint64_t b, uint64_t &hi)
{
	unsigned __int128 const r((unsigned __int128)a * b);
	hi = uint64_t(r >> 64);
	return uint64_t(r);
}
#endif


/*-------------------------------------------------
    addu_32x32_co - perform an unsigned 32 bit + 32
    bit addition and return the result with carry
    out
-------------------------------------------------*/

#ifndef addu_32x32_co
#define addu_32x32_co _addu_32x32_co
inline bool _addu_32x32_co(uint32_t a, uint32_t b, uint32_t &sum)
{
	return __builtin_add_overflow(a, b, &sum);
}
#endif


/*-------------------------------------------------
    addu_64x64_co - perform an unsigned 64 bit + 64
    bit addition and return the result with carry
    out
-------------------------------------------------*/

#ifndef addu_64x64_co
#define addu_64x64_co _addu_64x64_co
inline bool _addu_64x64_co(uint64_t a, uint64_t b, uint64_t &sum)
{
	return __builtin_add_overflow(a, b, &sum);
}
#endif


/*-------------------------------------------------
    muldivu_64 - perform an unsigned 64 bits a*b/c,
    rounding toward zero.  Unpredictable (including
    crashes) when the result does not fit in 64
    bits.
-------------------------------------------------*/

#if !defined(muldivu_64) && defined(__SIZEOF_INT128__)
#define muldivu_64 _muldivu_64
inline uint64_t ATTR_FORCE_INLINE
_muldivu_64(uint64_t m1, uint64_t m2, uint64_t d)
{
	return (__uint128_t(m1) * m2) / d;
}
#endif


/*-------------------------------------------------
    muldivupu_64 - perform an unsigned 64 bits a*b/c,
    rounding away from zero.  Unpredictable
    (including crashes) when the result does not
    fit in 64 bits.
-------------------------------------------------*/

#if !defined(muldivupu_64) && defined(__SIZEOF_INT128__)
#define muldivupu_64 _muldivupu_64
inline uint64_t ATTR_FORCE_INLINE
_muldivupu_64(uint64_t m1, uint64_t m2, uint64_t d)
{
	return ((__uint128_t(m1) * m2) + d - 1) / d;
}
#endif

#endif // MAME_OSD_EIGCC_H
