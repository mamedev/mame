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



/***************************************************************************
    INLINE BIT MANIPULATION FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    count_leading_zeros_32 - return the number of
    leading zero bits in a 32-bit value
-------------------------------------------------*/

#ifndef count_leading_zeros_32
#define count_leading_zeros_32 _count_leading_zeros_32
inline uint8_t _count_leading_zeros_32(uint32_t val)
{
	// uses CPU feature if available, otherwise falls back to runtime library call
	static_assert(sizeof(val) == sizeof(unsigned), "expected 32-bit unsigned int");
	return uint8_t(unsigned(val ? __builtin_clz(val) : 32));
}
#endif


/*-------------------------------------------------
    count_leading_ones_32 - return the number of
    leading one bits in a 32-bit value
-------------------------------------------------*/

#ifndef count_leading_ones_32
#define count_leading_ones_32 _count_leading_ones_32
inline uint8_t _count_leading_ones_32(uint32_t val)
{
	return count_leading_zeros_32(~val);
}
#endif


/*-------------------------------------------------
    count_leading_zeros_64 - return the number of
    leading zero bits in a 64-bit value
-------------------------------------------------*/

#ifndef count_leading_zeros_64
#define count_leading_zeros_64 _count_leading_zeros_64
inline uint8_t _count_leading_zeros_64(uint64_t val)
{
	// uses CPU feature if available, otherwise falls back to runtime library call
	static_assert(sizeof(val) == sizeof(unsigned long long), "expected 64-bit unsigned long long int");
	return uint8_t(unsigned(val ? __builtin_clzll(val) : 64));
}
#endif


/*-------------------------------------------------
    count_leading_ones_64 - return the number of
    leading one bits in a 64-bit value
-------------------------------------------------*/

#ifndef count_leading_ones_64
#define count_leading_ones_64 _count_leading_ones_64
inline uint8_t _count_leading_ones_64(uint64_t val)
{
	return count_leading_zeros_64(~val);
}
#endif


/*-------------------------------------------------
    population_count_32 - return the number of
    one bits in a 32-bit value
-------------------------------------------------*/

#ifndef population_count_32
#define population_count_32 _population_count_32
inline unsigned _population_count_32(uint32_t val)
{
	// uses CPU feature if available, otherwise falls back to implementation similar to eminline.h
	static_assert(sizeof(val) == sizeof(unsigned), "expected 32-bit unsigned int");
	return unsigned(__builtin_popcount(static_cast<unsigned>(val)));
}
#endif


/*-------------------------------------------------
    population_count_64 - return the number of
    one bits in a 64-bit value
-------------------------------------------------*/

#ifndef population_count_64
#define population_count_64 _population_count_64
inline unsigned _population_count_64(uint64_t val)
{
	// uses CPU feature if available, otherwise falls back to implementation similar to eminline.h
	static_assert(sizeof(val) == sizeof(unsigned long long), "expected 64-bit unsigned long long int");
	return unsigned(__builtin_popcountll(static_cast<unsigned long long>(val)));
}
#endif

#endif // MAME_OSD_EIGCC_H
