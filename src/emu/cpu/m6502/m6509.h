/*****************************************************************************
 *
 *   m6509.h
 *   Portable 6509 emulator V1.0beta
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
 *     pullmoll@t-online.de
 *   - The author of this copywritten work reserves the right to change the
 *     terms of its usage and license at any time, including retroactively
 *   - This entire notice must remain in the source code.
 *
 *****************************************************************************/

#ifndef _M6509_H
#define _M6509_H

#include "m6502.h"

enum {
	M6509_PC=1, M6509_S, M6509_P, M6509_A, M6509_X, M6509_Y,
	M6509_EA, M6509_ZP, M6509_NMI_STATE, M6509_IRQ_STATE, M6509_SO_STATE,
	M6509_PC_BANK, M6509_IND_BANK
};

#define M6509_IRQ_LINE					M6502_IRQ_LINE
/* use cpunum_set_input_line(cpu, M6509_SET_OVERFLOW, level)
   to change level of the so input line
   positiv edge sets overflow flag */
#define M6509_SET_OVERFLOW 3

void m6509_get_info(UINT32 state, cpuinfo *info);

#endif /* _M6509_H */


