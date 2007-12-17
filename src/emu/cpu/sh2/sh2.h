/*****************************************************************************
 *
 *   sh2.h
 *   Portable Hitachi SH-2 (SH7600 family) emulator interface
 *
 *   Copyright (c) 2000 Juergen Buchmueller <pullmoll@t-online.de>,
 *   all rights reserved.
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
 *  This work is based on <tiraniddo@hotmail.com> C/C++ implementation of
 *  the SH-2 CPU core and was heavily changed to the MAME CPU requirements.
 *  Thanks also go to Chuck Mason <chukjr@sundail.net> and Olivier Galibert
 *  <galibert@pobox.com> for letting me peek into their SEMU code :-)
 *
 *****************************************************************************/

#ifndef _SH2_H
#define _SH2_H

#include "cpuintrf.h"

#define SH2_INT_NONE	-1
#define SH2_INT_VBLIN	0
#define SH2_INT_VBLOUT	1
#define SH2_INT_HBLIN	2
#define SH2_INT_TIMER0	3
#define SH2_INT_TIMER1	4
#define SH2_INT_DSP 	5
#define SH2_INT_SOUND	6
#define SH2_INT_SMPC	7
#define SH2_INT_PAD 	8
#define SH2_INT_DMA2	9
#define SH2_INT_DMA1	10
#define SH2_INT_DMA0	11
#define SH2_INT_DMAILL	12
#define SH2_INT_SPRITE	13
#define SH2_INT_14		14
#define SH2_INT_15		15
#define SH2_INT_ABUS	16

enum {
	SH2_PC=1, SH2_SR, SH2_PR, SH2_GBR, SH2_VBR, SH2_MACH, SH2_MACL,
	SH2_R0, SH2_R1, SH2_R2, SH2_R3, SH2_R4, SH2_R5, SH2_R6, SH2_R7,
	SH2_R8, SH2_R9, SH2_R10, SH2_R11, SH2_R12, SH2_R13, SH2_R14, SH2_R15, SH2_EA
};

enum
{
	CPUINFO_INT_SH2_FRT_INPUT = CPUINFO_INT_CPU_SPECIFIC
};

enum
{
	CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK = CPUINFO_PTR_CPU_SPECIFIC,
};

struct sh2_config
{
  int is_slave;
};

extern void sh2_get_info(UINT32 state, cpuinfo *info);

WRITE32_HANDLER( sh2_internal_w );
READ32_HANDLER( sh2_internal_r );

#ifdef MAME_DEBUG
extern unsigned DasmSH2( char *dst, unsigned pc, UINT16 opcode );
#endif

#endif /* _SH2_H */

