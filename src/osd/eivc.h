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
#include <stdlib.h>
#pragma intrinsic(_BitScanReverse)
#ifdef PTR64
#pragma intrinsic(_BitScanReverse64)
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
__forceinline uint8_t _count_leading_zeros_32(uint32_t value)
{
	unsigned long index;
	return _BitScanReverse(&index, value) ? (31U - index) : 32U;
}
#endif


/*-------------------------------------------------
    count_leading_ones_32 - return the number of
    leading one bits in a 32-bit value
-------------------------------------------------*/

#ifndef count_leading_ones_32
#define count_leading_ones_32 _count_leading_ones_32
__forceinline uint8_t _count_leading_ones_32(uint32_t value)
{
	unsigned long index;
	return _BitScanReverse(&index, ~value) ? (31U - index) : 32U;
}
#endif


/*-------------------------------------------------
    count_leading_zeros_64 - return the number of
    leading zero bits in a 64-bit value
-------------------------------------------------*/

#ifndef count_leading_zeros_64
#define count_leading_zeros_64 _count_leading_zeros_64
__forceinline uint8_t _count_leading_zeros_64(uint64_t value)
{
	unsigned long index;
#ifdef PTR64
	return _BitScanReverse64(&index, value) ? (63U - index) : 64U;
#else
	return _BitScanReverse(&index, uint32_t(value >> 32)) ? (31U - index) : _BitScanReverse(&index, uint32_t(value)) ? (63U - index) : 64U;
#endif
}
#endif


/*-------------------------------------------------
    count_leading_ones_64 - return the number of
    leading one bits in a 64-bit value
-------------------------------------------------*/

#ifndef count_leading_ones_64
#define count_leading_ones_64 _count_leading_ones_64
__forceinline uint8_t _count_leading_ones_64(uint64_t value)
{
	unsigned long index;
#ifdef PTR64
	return _BitScanReverse64(&index, ~value) ? (63U - index) : 64U;
#else
	return _BitScanReverse(&index, ~uint32_t(value >> 32)) ? (31U - index) : _BitScanReverse(&index, ~uint32_t(value)) ? (63U - index) : 64U;
#endif
}
#endif


/*-------------------------------------------------
    rotl_32 - circularly shift a 32-bit value left
    by the specified number of bits (modulo 32)
-------------------------------------------------*/

#ifndef rotl_32
#define rotl_32 _rotl
#endif


/*-------------------------------------------------
    rotr_32 - circularly shift a 32-bit value right
    by the specified number of bits (modulo 32)
-------------------------------------------------*/

#ifndef rotr_32
#define rotr_32 _rotr
#endif


/*-------------------------------------------------
    rotl_64 - circularly shift a 64-bit value left
    by the specified number of bits (modulo 64)
-------------------------------------------------*/

#ifndef rotl_64
#define rotl_64 _rotl64
#endif


/*-------------------------------------------------
    rotr_64 - circularly shift a 64-bit value right
    by the specified number of bits (modulo 64)
-------------------------------------------------*/

#ifndef rotr_64
#define rotr_64 _rotr64
#endif

#endif // MAME_OSD_EIVC_H
