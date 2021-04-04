// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  eivc.h
//
//  Inline implementations for MSVC compiler.
//
//============================================================

#ifndef MAME_OSD_EIVC_H
#define MAME_OSD_EIVC_H

#pragma once

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
__forceinline uint8_t _count_leading_zeros(uint32_t value)
{
	unsigned long index;
	return _BitScanReverse(&index, value) ? (31U - index) : 32U;
}
#endif


/*-------------------------------------------------
    count_leading_ones - return the number of
    leading one bits in a 32-bit value
-------------------------------------------------*/

#ifndef count_leading_ones
#define count_leading_ones _count_leading_ones
__forceinline uint8_t _count_leading_ones(uint32_t value)
{
	unsigned long index;
	return _BitScanReverse(&index, ~value) ? (31U - index) : 32U;
}
#endif


/*-------------------------------------------------
    count_leading_zeros_64 - return the number of
    leading zero bits in a 64-bit value
-------------------------------------------------*/

#if defined(_WIN64)
#ifndef count_leading_zeros_64
#define count_leading_zeros_64 _count_leading_zeros_64
__forceinline uint8_t _count_leading_zeros_64(uint64_t value)
{
	unsigned long index;
	return _BitScanReverse64(&index, value) ? (63U - index) : 64U;
}
#endif
#endif


/*-------------------------------------------------
    count_leading_ones_64 - return the number of
    leading one bits in a 64-bit value
-------------------------------------------------*/

#if defined(_WIN64)
#ifndef count_leading_ones_64
#define count_leading_ones_64 _count_leading_ones_64
__forceinline uint8_t _count_leading_ones_64(uint32_t value)
{
	unsigned long index;
	return _BitScanReverse64(&index, ~value) ? (63U - index) : 64U;
}
#endif
#endif


/***************************************************************************
    INLINE MATH FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    div_128x64 - perform a signed 128 bit x 64 bit
    divide and return the 64 bit quotient
-------------------------------------------------*/

#if defined(_WIN64)
#define div_128x64 _div_128x64
inline int64_t ATTR_CONST ATTR_FORCE_INLINE
_div_128x64(int64_t a_hi, uint64_t a_lo, int64_t b)
{
	int64_t remainder;
	return _div128(a_hi, a_lo, b, &remainder);
}
#endif


/*-------------------------------------------------
    divu_128x64 - perform an unsigned 128 bit x 64 bit
    divide and return the 64 bit quotient
-------------------------------------------------*/

#if defined(_WIN64)
#define divu_128x64 _divu_128x64
inline uint64_t ATTR_CONST ATTR_FORCE_INLINE
_divu_128x64(uint64_t a_hi, uint64_t a_lo, uint64_t b)
{
	uint64_t remainder;
	return _udiv128(a_hi, a_lo, b, &remainder);
}
#endif

#endif // MAME_OSD_EIVC_H
