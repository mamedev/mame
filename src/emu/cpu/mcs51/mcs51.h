/*****************************************************************************
 *
 *   mcs51.h
 *   Portable MCS-51 Family Emulator
 *
 *   Chips in the family:
 *   8051 Product Line (8031,8051,8751)
 *   8052 Product Line (8032,8052,8752)
 *   8054 Product Line (8054)
 *   8058 Product Line (8058)
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
 *  #1) 'Intel(tm) MC51 Microcontroller Family Users Manual' and
 *  #2) 8051 simulator by Travis Marlatte
 *  #3) Portable UPI-41/8041/8741/8042/8742 emulator V0.1 by Juergen Buchmueller (MAME CORE)
 *
 * 2008, October, Couriersud
 * - Rewrite of timer, interrupt and serial code
 * - addition of CMOS features
 * - internal memory maps
 * - addition of new processor types
 * - full emulation of 8xCx2 processors
 *****************************************************************************/

#pragma once

#ifndef __MCS51_H__
#define __MCS51_H__

#include "cpuintrf.h"

enum
{
	MCS51_PC=1, MCS51_SP, MCS51_PSW, MCS51_ACC, MCS51_B, MCS51_DPH, MCS51_DPL, MCS51_IE,
	MCS51_R0, MCS51_R1, MCS51_R2, MCS51_R3, MCS51_R4, MCS51_R5, MCS51_R6, MCS51_R7, MCS51_RB
};

enum
{
	MCS51_INT0_LINE = 0, 	/* P3.2: External Interrupt 0 */
	MCS51_INT1_LINE,		/* P3.3: External Interrupt 1 */
	MCS51_RX_LINE,			/* P3.0: Serial Port Receive Line */
	MCS51_T0_LINE,			/* P3,4: Timer 0 External Input */
	MCS51_T1_LINE,			/* P3.5: Timer 1 External Input */
	MCS51_T2_LINE,			/* P1.0: Timer 2 External Input */
	MCS51_T2EX_LINE,		/* P1.1: Timer 2 Capture Reload Trigger */
};

/* special I/O space ports */

enum
{
	MCS51_PORT_P0	= 0x10000,
	MCS51_PORT_P1	= 0x10001,
	MCS51_PORT_P2	= 0x10002,
	MCS51_PORT_P3	= 0x10003,
	MCS51_PORT_TX	= 0x10004,	/* P3.1 */
};


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

extern void i8051_set_serial_tx_callback(void (*callback)(int data));
extern void i8051_set_serial_rx_callback(int (*callback)(void));

/* variants with no internal rom and 128 byte internal memory */
void i8031_get_info(UINT32 state, cpuinfo *info);

/* variants with no internal rom and 256 byte internal memory */
void i8032_get_info(UINT32 state, cpuinfo *info);

/* variants 4k internal rom and 128 byte internal memory */
void i8051_get_info(UINT32 state, cpuinfo *info);
void i8751_get_info(UINT32 state, cpuinfo *info);

/* variants 8k internal rom and 256 byte internal memory and more registers */
void i8052_get_info(UINT32 state, cpuinfo *info);
void i8752_get_info(UINT32 state, cpuinfo *info);

/* cmos variants */
void i80c31_get_info(UINT32 state, cpuinfo *info);
void i80c51_get_info(UINT32 state, cpuinfo *info);
void i87c51_get_info(UINT32 state, cpuinfo *info);

void i80c32_get_info(UINT32 state, cpuinfo *info);
void i80c52_get_info(UINT32 state, cpuinfo *info);
void i87c52_get_info(UINT32 state, cpuinfo *info);

/* 4k internal perom and 128 internal ram and 2 analog comparators */
void at89c4051_get_info(UINT32 state, cpuinfo *info);

/****************************************************************************
 * Disassembler
 ****************************************************************************/

offs_t i8051_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);

#endif /* __MCS51_H__ */
