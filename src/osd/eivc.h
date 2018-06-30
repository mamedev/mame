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
inline uint8_t _count_leading_zeros(uint32_t value)
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
inline uint8_t _count_leading_ones(uint32_t value)
{
	unsigned long index;
	return _BitScanReverse(&index, ~value) ? (31U - index) : 32U;
}
#endif

#endif // MAME_OSD_EIVC_H
