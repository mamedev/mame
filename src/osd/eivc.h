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

#if (_MSC_VER >= 1400)

// need to ignore 'nonstandard extension used' warning in setjmp.h
#pragma warning(push)
#pragma warning(disable: 4987)
#include <intrin.h>
#pragma warning(pop)

#else
extern "C" unsigned char _BitScanReverse(unsigned long *Index, unsigned long Mask);
#endif

#if (_MSC_VER >= 1310)
#pragma intrinsic(_BitScanReverse)
#endif


/***************************************************************************
    INLINE BIT MANIPULATION FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    count_leading_zeros - return the number of
    leading zero bits in a 32-bit value
-------------------------------------------------*/

#ifndef count_leading_zeros
#define count_leading_zeros _count_leading_zeros
static inline UINT8 _count_leading_zeros(UINT32 value)
{
	UINT32 index;
	return _BitScanReverse((unsigned long *)&index, value) ? (index ^ 31) : 32;
}
#endif


/*-------------------------------------------------
    count_leading_ones - return the number of
    leading one bits in a 32-bit value
-------------------------------------------------*/

#ifndef count_leading_ones
#define count_leading_ones _count_leading_ones
static inline UINT8 _count_leading_ones(UINT32 value)
{
	UINT32 index;
	return _BitScanReverse((unsigned long *)&index, ~value) ? (index ^ 31) : 32;
}
#endif

#endif /* __EIVC__ */
