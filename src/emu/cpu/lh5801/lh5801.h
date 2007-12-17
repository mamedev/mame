/*****************************************************************************
 *
 *   lh5801.h
 *   portable lh5801 emulator interface
 *
 *   Copyright (c) 2000 Peter Trauner, all rights reserved.
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     peter.trauner@jk.uni-linz.ac.at
 *   - The author of this copywritten work reserves the right to change the
 *     terms of its usage and license at any time, including retroactively
 *   - This entire notice must remain in the source code.
 *
 *****************************************************************************/
#ifndef _LH5801_H
#define _LH5801_H

/*
lh5801

little endian

ph, pl p
sh, sl s

xh, xl x
yh, yl y
uh, ul u

a A

0 0 0 H V Z IE C

TM 9bit polynominal?

pu pv disp flipflops

bf flipflop (break key connected)

    me0, me1 chip select for 2 64kb memory blocks

in0-in7 input pins

    mi maskable interrupt input (fff8/9)
    timer fffa/b
    nmi non .. (fffc/d)
    reset fffe/f
e ?



lh5811 chip
pa 8bit io
pb 8bit io
pc 8bit
*/

#include "cpuintrf.h"


typedef struct {
	UINT8 (*in)(void);
} LH5801_CONFIG;

#define LH5801_INT_NONE 0
#define LH5801_IRQ 1

extern offs_t lh5801_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);

#endif
