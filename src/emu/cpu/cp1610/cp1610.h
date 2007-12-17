/*****************************************************************************
 *
 *   cp1610.h
 *   Portable General Instruments CP1610 emulator interface
 *
 *   Copyright (c) 2004 Frank Palazzolo, all rights reserved.
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     palazzol@comcast.net
 *   - This entire notice must remain in the source code.
 *
 *****************************************************************************/

#ifndef _CP1610_H
#define _CP1610_H

#include "cpuintrf.h"

enum {
	CP1610_R0=1, CP1610_R1, CP1610_R2, CP1610_R3,
	CP1610_R4, CP1610_R5, CP1610_R6, CP1610_R7
};

#define CP1610_INT_NONE		0
#define CP1610_INT_INTRM	1				/* Maskable */
#define CP1610_RESET		2				/* Non-Maskable */
#define CP1610_INT_INTR		INPUT_LINE_NMI	/* Non-Maskable */

extern int cp1610_icount;				 /* cycle count */


void cp1610_get_info(UINT32 state, cpuinfo *info);

#ifdef MAME_DEBUG
unsigned cp1610_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);
#endif /* MAME_DEBUG */

// Temporary
#define cp1610_readop(A) program_read_word_16be((A)<<1)
#define cp1610_readmem16(A) program_read_word_16be((A)<<1)
#define cp1610_writemem16(A,B) program_write_word_16be((A)<<1,B)

#endif /* _CP1610_H */
