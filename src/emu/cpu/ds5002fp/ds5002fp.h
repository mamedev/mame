/*****************************************************************************
 *   DS5002FP emulator
 *
 *   The emulator is just a modified version of the MCS-51 Family Emulator by Steve Ellenoff
 *
 *****************************************************************************/

/*****************************************************************************
 *
 *   i8051.h
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
 *****************************************************************************/

#pragma once

#ifndef __DS5002FP_H__
#define __DS5002FP_H__

#include "cpuintrf.h"

enum
{
	DS5002FP_PC=1, DS5002FP_SP, DS5002FP_PSW, DS5002FP_ACC, DS5002FP_B, DS5002FP_DPH, DS5002FP_DPL, DS5002FP_IE,
	DS5002FP_PCON, DS5002FP_MCON, DS5002FP_RPCTL, DS5002FP_CRC,
	DS5002FP_R0, DS5002FP_R1, DS5002FP_R2, DS5002FP_R3, DS5002FP_R4, DS5002FP_R5, DS5002FP_R6, DS5002FP_R7, DS5002FP_RB
};

#define DS5002FP_INT0_LINE		0   /* External Interrupt 0 */
#define DS5002FP_INT1_LINE		1   /* External Interrupt 1 */
#define DS5002FP_T0_LINE		2   /* Timer 0 External Input */
#define DS5002FP_T1_LINE		3   /* Timer 1 External Input */
#define DS5002FP_RX_LINE		4   /* Serial Port Receive Line */
#define DS5002FP_PFI_LINE		5   /* Power Fail Interrupt */

/* definition of the special function registers. Note that the values are */
/* the same as the internal memory address in the DS5002FP */
#define		P0		0x80
#define		SP		0x81
#define		DPL		0x82
#define		DPH		0x83
#define		PCON	0x87
#define		TCON	0x88
#define		TMOD	0x89
#define		TL0		0x8a
#define		TL1		0x8b
#define		TH0		0x8c
#define		TH1		0x8d
#define		P1		0x90
#define		SCON	0x98
#define		SBUF	0x99
#define		P2		0xa0
#define		IE		0xa8
#define		P3		0xb0
#define		IP		0xb8
#define		CRCR	0xc1
#define		CRCL	0xc2
#define		CRCH	0xc3
#define		MCON	0xc6
#define		TA		0xc7
#define		RNR		0xcf
#define		PSW		0xd0
#define		RPCTL	0xd8
#define		RPS		0xda
#define		ACC		0xe0
#define		B		0xf0

/* commonly used bit address for the DS5002FP */
#define		C		0xd7
#define		P		0xd0
#define		AC		0xd6
#define		OV		0xd2
#define		PFW		0x8d
#define		TF1		0x8f
#define		IE0		0x89
#define		IE1		0x8b
#define		TI		0x99
#define		RI		0x98

#define TI_FLAG 1
#define RI_FLAG 2

/* configuration of the DS5002FP */
typedef struct _ds5002fp_config ds5002fp_config;
struct _ds5002fp_config
{
	UINT8	mcon;					/* bootstrap loader MCON register */
	UINT8	rpctl;					/* bootstrap loader RPCTL register */
	UINT8	crc;					/* bootstrap loader CRC register */
};

extern void ds5002fp_init (int index, int clock, const void *config, int (*irqcallback)(int));					/* Initialize save states */
extern void ds5002fp_reset (void);			/* Reset registers to the initial values */
extern void ds5002fp_exit	(void); 				/* Shut down CPU core */
extern int  ds5002fp_execute(int cycles);			/* Execute cycles - returns number of cycles actually run */
extern void ds5002fp_get_context (void *dst);	/* Get registers, return context size */
extern void ds5002fp_set_context (void *src);    	/* Set registers */
extern unsigned ds5002fp_get_intram (int offset);
extern unsigned ds5002fp_get_reg (int regnum);
extern void ds5002fp_set_reg (int regnum, unsigned val);
extern void ds5002fp_set_irq_line(int irqline, int state);
extern void ds5002fp_set_irq_callback(int (*callback)(int irqline));
extern void ds5002fp_state_save(void *file);
extern void ds5002fp_state_load(void *file);

WRITE8_HANDLER( ds5002fp_internal_w );
READ8_HANDLER( ds5002fp_internal_r );

extern void ds5002fp_set_serial_tx_callback(void (*callback)(int data));
extern void ds5002fp_set_serial_rx_callback(int (*callback)(void));
extern void ds5002fp_set_ebram_iaddr_callback(READ32_HANDLER((*callback)));

extern offs_t ds5002fp_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);

void ds5002fp_get_info(UINT32 state, cpuinfo *info);

#endif /* __DS5002FP_H__ */
