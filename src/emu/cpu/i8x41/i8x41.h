/*****************************************************************************
 *
 *   i8x41.h
 *   Portable UPI-41/8041/8741/8042/8742 emulator interface
 *
 *   Copyright (c) 1999 Juergen Buchmueller, all rights reserved.
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
 *   TLP (10-Jan-2003)
 *     Added output ports registers to the debug viewer
 *     Added the Clock Divider
 *
 *****************************************************************************/

#ifndef _I8X41_H
#define _I8X41_H

#include "cpuintrf.h"


/* The i8x41/i8x42 input clock is divided by 15. Use it with your CPU speed */
#define I8X41_CLOCK_DIVIDER		15

/* Note:
 * I8X41_DATA is A0 = 0 and R/W
 * I8X41_CMND is A0 = 1 and W only
 * I8X41_STAT is A0 = 1 and R only
 */


/****************************************************************************
 *  Interrupt constants
 */

#define I8X41_INT_IBF	0	/* input buffer full interrupt */
#define I8X41_INT_TEST1 1	/* test1 line (also counter interrupt; taken on cntr overflow) */


/****************************************************************************
 *  Use these in the I/O port fields of your driver for the test lines - i.e,
 *    { I8X41_t0,   I8X41_t0,   i8041_test0_r },
 *    { I8X41_t1,   I8X41_t1,   i8041_test1_r },
 *    { I8X41_ps,   I8X41_ps,   i8041_port_strobe_w },
 */

#define I8X41_t0		0x80	/* TEST0 input port handle */
#define I8X41_t1		0x81	/* TEST1 input port handle */
#define I8X41_ps		0x82	/* Prog pin strobe for expanded port sync */


/****************************************************************************
 *  The i8x41/i8x42 have 128/256 bytes of internal memory respectively
 */

#define I8X41_intRAM_MASK		0x7f
#define I8X42_intRAM_MASK		0xff


enum {
	I8X41_PC=1, I8X41_SP, I8X41_PSW, I8X41_T, I8X41_DATA, I8X41_DATA_DASM,
	I8X41_CMND, I8X41_CMND_DASM, I8X41_STAT, I8X41_P1, I8X41_P2,I8X41_A,
	I8X41_R0, I8X41_R1, I8X41_R2, I8X41_R3, I8X41_R4, I8X41_R5, I8X41_R6, I8X41_R7
};



/****************************************************************************
 *  Public Functions
 */

extern void i8x41_get_info(UINT32 state, cpuinfo *info);

#ifdef MAME_DEBUG
extern offs_t i8x41_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);
#endif

#endif /* _I8X41_H */
