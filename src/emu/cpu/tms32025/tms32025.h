	/**************************************************************************\
	*                Texas Instruments TMS320x25 DSP Emulator                  *
	*                                                                          *
	*                 Copyright Tony La Porta                                  *
	*                      Written for the MAME project.                       *
	*                                                                          *
	*      Note :  This is a word based microcontroller, with addressing       *
	*              architecture based on the Harvard addressing scheme.        *
	*                                                                          *
	*  Three versions of the chip are available, and they are:                 *
	*  TMS320C25   Internal ROM one time programmed at TI                      *
	*  TMS320E25   Internal ROM programmable as a normal EPROM                 *
	*  TMS320P25   Internal ROM programmable once as a normal EPROM only       *
	*  These devices can also be used as a MicroController with external ROM   *
	*                                                                          *
	\***************************************************************************/

#pragma once

#ifndef __TMS32025_H__
#define __TMS32025_H__




#define TMS32025_BIO        0x10000     /* BIO input  */
#define TMS32025_HOLD       0x10001     /* HOLD input */
#define TMS32025_HOLDA      0x10001     /* HOLD Acknowledge output */
#define TMS32025_XF         0x10002     /* XF output  */
#define TMS32025_DR         0x10003     /* Serial Data  Receive  input  */
#define TMS32025_DX         0x10003     /* Serial Data  Transmit output */



/****************************************************************************
 *  Interrupt constants
 */

#define TMS32025_INT0             0         /* External INT0 */
#define TMS32025_INT1             1         /* External INT1 */
#define TMS32025_INT2             2         /* External INT2 */
#define TMS32025_TINT             3         /* Internal Timer interrupt */
#define TMS32025_RINT             4         /* Serial Port receive  interrupt */
#define TMS32025_XINT             5         /* Serial Port transmit interrupt */
#define TMS32025_TRAP             6         /* Trap instruction */
#define TMS32025_INT_NONE         -1

/* Non-irq line */
#define TMS32025_FSX              7         /* Frame synchronisation */

enum
{
	TMS32025_PC=1,
	TMS32025_PFC,  TMS32025_STR0, TMS32025_STR1, TMS32025_IFR,
	TMS32025_RPTC, TMS32025_ACC,  TMS32025_PREG, TMS32025_TREG,
	TMS32025_AR0,  TMS32025_AR1,  TMS32025_AR2,  TMS32025_AR3,
	TMS32025_AR4,  TMS32025_AR5,  TMS32025_AR6,  TMS32025_AR7,
	TMS32025_STK0, TMS32025_STK1, TMS32025_STK2, TMS32025_STK3,
	TMS32025_STK4, TMS32025_STK5, TMS32025_STK6, TMS32025_STK7,
	TMS32025_DRR,  TMS32025_DXR,  TMS32025_TIM,  TMS32025_PRD,
	TMS32025_IMR,  TMS32025_GREG
};


/****************************************************************************
 *  Public Functions
 */

DECLARE_LEGACY_CPU_DEVICE(TMS32025, tms32025);
DECLARE_LEGACY_CPU_DEVICE(TMS32026, tms32026);

CPU_DISASSEMBLE( tms32025 );

#endif  /* __TMS32025_H__ */
