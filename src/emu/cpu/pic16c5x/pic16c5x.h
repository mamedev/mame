 /**************************************************************************\
 *                      Microchip PIC16C5x Emulator                         *
 *                                                                          *
 *                    Copyright Tony La Porta                               *
 *                 Originally written for the MAME project.                 *
 *                                                                          *
 *                                                                          *
 *      Addressing architecture is based on the Harvard addressing scheme.  *
 *                                                                          *
 \**************************************************************************/

#pragma once

#ifndef __PIC16C5X_H__
#define __PIC16C5X_H__




/**************************************************************************
 *  Internal Clock divisor
 *
 *  External Clock is divided internally by 4 for the instruction cycle
 *  times. (Each instruction cycle passes through 4 machine states). This
 *  is handled by the cpu execution engine.
 */

enum
{
	PIC16C5x_PC=1, PIC16C5x_STK0, PIC16C5x_STK1, PIC16C5x_FSR,
	PIC16C5x_W,    PIC16C5x_ALU,  PIC16C5x_STR,  PIC16C5x_OPT,
	PIC16C5x_TMR0, PIC16C5x_PRTA, PIC16C5x_PRTB, PIC16C5x_PRTC,
	PIC16C5x_WDT,  PIC16C5x_TRSA, PIC16C5x_TRSB, PIC16C5x_TRSC,
	PIC16C5x_PSCL
};

#define PIC16C5x_T0		0x10


/****************************************************************************
 *  Function to configure the CONFIG register. This is actually hard-wired
 *  during ROM programming, so should be called in the driver INIT, with
 *  the value if known (available in HEX dumps of the ROM).
 */

void pic16c5x_set_config(running_device *cpu, int data);



CPU_GET_INFO( pic16c54 );
#define CPU_PIC16C54 CPU_GET_INFO_NAME( pic16c54 )


CPU_GET_INFO( pic16c55 );
#define CPU_PIC16C55 CPU_GET_INFO_NAME( pic16c55 )


CPU_GET_INFO( pic16c56 );
#define CPU_PIC16C56 CPU_GET_INFO_NAME( pic16c56 )


CPU_GET_INFO( pic16c57 );
#define CPU_PIC16C57 CPU_GET_INFO_NAME( pic16c57 )


CPU_GET_INFO( pic16c58 );
#define CPU_PIC16C58 CPU_GET_INFO_NAME( pic16c58 )


CPU_DISASSEMBLE( pic16c5x );


#endif	/* __PIC16C5X_H__ */
