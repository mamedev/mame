/*****************************************************************************
 *
 *   m65ce02.c
 *   Portable 65ce02 emulator V1.0beta
 *
 *   Copyright Peter Trauner, all rights reserved.
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     pullmoll@t-online.de
 *   - The author of this copywritten work reserves the right to change the
 *     terms of its usage and license at any time, including retroactively
 *   - This entire notice must remain in the source code.
 *
 *****************************************************************************/

#pragma once

#ifndef __M65CE02_H__
#define __M65CE02_H__

#include "m6502.h"

enum
{
	M65CE02_PC=1, M65CE02_S, M65CE02_P, M65CE02_A, M65CE02_X, M65CE02_Y,
	M65CE02_Z, M65CE02_B, M65CE02_EA, M65CE02_ZP,
	M65CE02_NMI_STATE, M65CE02_IRQ_STATE
};

#define M65CE02_IRQ_LINE				M6502_IRQ_LINE

DECLARE_LEGACY_CPU_DEVICE(M65CE02, m65ce02);

extern CPU_DISASSEMBLE( m65ce02 );

#endif /* __M65CE02_H__ */
