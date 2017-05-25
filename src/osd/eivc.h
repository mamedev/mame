// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  eivc.h
//
//  Inline implementations for MSVC compiler.
//
//============================================================

#ifndef __EIVC__
#define __EIVC__
#include <intrin.h>
#pragma intrinsic(_BitScanReverse)


/***************************************************************************
    INLINE BIT MANIPULATION FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    count_leading_zeros - return the number of
    leading zero bits in a 32-bit value
-------------------------------------------------*/

#ifndef count_leading_zeros
#define count_leading_zeros _count_leading_zeros
static inline uint8_t _count_leading_zeros(uint32_t value)
{
	uint32_t index;
	return _BitScanReverse((unsigned long *)&index, value) ? (index ^ 31) : 32;
}
#endif


/*-------------------------------------------------
    count_leading_ones - return the number of
    leading one bits in a 32-bit value
-------------------------------------------------*/

#ifndef count_leading_ones
#define count_leading_ones _count_leading_ones
static inline uint8_t _count_leading_ones(uint32_t value)
{
	uint32_t index;
	return _BitScanReverse((unsigned long *)&index, ~value) ? (index ^ 31) : 32;
}
#endif

#endif /* __EIVC__ */
