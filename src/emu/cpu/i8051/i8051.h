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

#ifndef _I8051_H
#define _I8051_H

#include "cpuintrf.h"

enum {
	I8051_PC=1, I8051_SP, I8051_PSW, I8051_ACC, I8051_B, I8051_DPH, I8051_DPL, I8051_IE,
	I8051_R0, I8051_R1, I8051_R2, I8051_R3, I8051_R4, I8051_R5, I8051_R6, I8051_R7, I8051_RB
};

#define I8051_INT0_LINE		0   /* External Interrupt 0 */
#define I8051_INT1_LINE		1   /* External Interrupt 1 */
#define I8051_T0_LINE		2   /* Timer 0 External Input */
#define I8051_T1_LINE		3   /* Timer 1 External Input */
#define I8051_RX_LINE		4   /* Serial Port Receive Line */

/* definition of the special function registers. Note that the values are */
/* the same as the internal memory address in the 8051 */
#define		P0		0x80
#define		SP		0x81
#define		DPL		0x82
#define		DPH		0x83
#define		PCON		0x87
#define		TCON		0x88
#define		TMOD		0x89
#define		TL0		0x8a
#define		TL1		0x8b
#define		TH0		0x8c
#define		TH1		0x8d
#define		P1		0x90
#define		SCON		0x98
#define		SBUF		0x99
#define		P2		0xa0
#define		IE		0xa8
#define		P3		0xb0
#define		IP		0xb8
//8052 Only registers
#if (HAS_I8052 || HAS_I8752)
 #define		T2CON	0xc8
 #define		RCAP2L	0xca
 #define		RCAP2H	0xcb
 #define		TL2	0xcc
 #define		TH2	0xcd
#endif
#define		PSW		0xd0
#define		ACC		0xe0
#define		B		0xf0

/* commonly used bit address for the 8051 */
#define		C		0xd7
#define		P		0xd0
#define		AC		0xd6
#define		OV		0xd2
#define		TF0		0x8d
#define		TF1		0x8f
#define		IE0		0x89
#define		IE1		0x8b
#define		TI		0x99
#define		RI		0x98

#define TI_FLAG 1
#define RI_FLAG 2

extern void i8051_init (int index, int clock, const void *config, int (*irqcallback)(int));					/* Initialize save states */
extern void i8051_reset (void);			/* Reset registers to the initial values */
extern void i8051_exit	(void); 				/* Shut down CPU core */
extern int  i8051_execute(int cycles);			/* Execute cycles - returns number of cycles actually run */
extern void i8051_get_context (void *dst);	/* Get registers, return context size */
extern void i8051_set_context (void *src);    	/* Set registers */
extern unsigned i8051_get_intram (int offset);
extern unsigned i8051_get_reg (int regnum);
extern void i8051_set_reg (int regnum, unsigned val);
extern void i8051_set_irq_line(int irqline, int state);
extern void i8051_set_irq_callback(int (*callback)(int irqline));
extern void i8051_state_save(void *file);
extern void i8051_state_load(void *file);

WRITE8_HANDLER( i8051_internal_w );
READ8_HANDLER( i8051_internal_r );

extern void i8051_set_serial_tx_callback(void (*callback)(int data));
extern void i8051_set_serial_rx_callback(int (*callback)(void));
extern void i8051_set_eram_iaddr_callback(READ32_HANDLER((*callback)));

extern offs_t i8051_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);

/****************************************************************************
 * 8752 Section
 ****************************************************************************/
#if (HAS_I8752)
extern void i8752_init (int index, int clock, const void *config, int (*irqcallback)(int));					/* Initialize save states */
extern void i8752_reset (void);			/* Reset registers to the initial values */
extern void i8752_exit	(void); 				/* Shut down CPU core */
extern int	i8752_execute(int cycles);			/* Execute cycles - returns number of cycles actually run */
extern void i8752_get_context (void *dst);	/* Get registers, return context size */
extern void i8752_set_context (void *src);		/* Set registers */
extern unsigned i8752_get_reg (int regnum);
extern void i8752_set_reg (int regnum, unsigned val);
extern void i8752_set_irq_line(int irqline, int state);
extern void i8752_set_irq_callback(int (*callback)(int irqline));
extern void i8752_state_save(void *file);
extern void i8752_state_load(void *file);
extern void i8752_set_serial_tx_callback(void (*callback)(int data));
extern void i8752_set_serial_rx_callback(int (*callback)(void));
WRITE8_HANDLER( i8752_internal_w );
READ8_HANDLER( i8752_internal_r );
#endif	//(HAS_8752)


void i8051_get_info(UINT32 state, cpuinfo *info);
void i8052_get_info(UINT32 state, cpuinfo *info);
void i8751_get_info(UINT32 state, cpuinfo *info);
void i8752_get_info(UINT32 state, cpuinfo *info);

#endif /* _I8051_H */


