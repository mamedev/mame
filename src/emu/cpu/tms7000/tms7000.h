/*****************************************************************************
 *
 *   cpustate->h (c header file)
 *   Portable TMS7000 emulator (Texas Instruments 7000)
 *
 *   Copyright tim lindner, all rights reserved.
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     tlindner@macmess.org
 *   - This entire notice must remain in the source code.
 *
 *****************************************************************************/

#pragma once

#ifndef __TMS7000_H__
#define __TMS7000_H__


enum { TMS7000_PC=1, TMS7000_SP, TMS7000_ST, TMS7000_IDLE, TMS7000_T1_CL, TMS7000_T1_PS, TMS7000_T1_DEC };

enum { TMS7000_VCC, TMS7000_VSS };

enum { TMS7000_NMOS, TMS7000_CMOS };

enum
{
	TMS7000_IRQ1_LINE = 0,   /* INT1 */
	TMS7000_IRQ2_LINE,       /* INT2 */
	TMS7000_IRQ3_LINE,       /* INT3 */
	TMS7000_IRQNONE = 255
};

enum
{
	TMS7000_PORTA = 0,
	TMS7000_PORTB,
	TMS7000_PORTC,
	TMS7000_PORTD
};

/* PUBLIC FUNCTIONS */
extern void tms7000_A6EC1( const device_config *device ); /* External event counter */
extern CPU_GET_INFO( tms7000 );
extern CPU_GET_INFO( tms7000_exl );

#define CPU_TMS7000 CPU_GET_INFO_NAME( tms7000 )
#define CPU_TMS7000_EXL CPU_GET_INFO_NAME( tms7000_exl )

extern CPU_DISASSEMBLE( tms7000 );

#endif /* __TMS7000_H__ */
