/*****************************************************************************
 *
 *   arm7.h
 *   Portable ARM7TDMI CPU Emulator
 *
 *   Copyright Steve Ellenoff, all rights reserved.
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     sellenoff@hotmail.com
 *   - The author of this copywritten work reserves the right to change the
 *     terms of its usage and license at any time, including retroactively
 *   - This entire notice must remain in the source code.
 *
 *  This work is based on:
 *  #1) 'Atmel Corporation ARM7TDMI (Thumb) Datasheet - January 1999'
 *  #2) Arm 2/3/6 emulator By Bryan McPhail (bmcphail@tendril.co.uk) and Phil Stroffolino (MAME CORE 0.76)
 *
 *****************************************************************************

 This file contains everything related to the arm7 cpu specific implementation.
 Anything related to the arm7 core itself is defined in arm7core.h instead.

 ******************************************************************************/

#pragma once

#ifndef __ARM7_H__
#define __ARM7_H__


/****************************************************************************************************
 *  PUBLIC FUNCTIONS
 ***************************************************************************************************/

DECLARE_LEGACY_CPU_DEVICE(ARM7, arm7);
DECLARE_LEGACY_CPU_DEVICE(ARM7_BE, arm7_be);
DECLARE_LEGACY_CPU_DEVICE(ARM9, arm9);
DECLARE_LEGACY_CPU_DEVICE(ARM920T, arm920t);
DECLARE_LEGACY_CPU_DEVICE(PXA255, pxa255);
DECLARE_LEGACY_CPU_DEVICE(SA1110, sa1110);

#endif /* __ARM7_H__ */
