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

#if defined(_M_X64) || defined(_M_ARM64)
#pragma intrinsic(_BitScanReverse64)
#endif


/***************************************************************************
    INLINE BIT MANIPULATION FUNCTIONS
***************************************************************************/

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
