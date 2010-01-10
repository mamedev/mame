 /**************************************************************************\
 *                  Microchip PIC16C62X Emulator                            *
 *                                                                          *
 *                          Based On                                        *
 *                  Microchip PIC16C5X Emulator                             *
 *                    Copyright Tony La Porta                               *
 *                 Originally written for the MAME project.                 *
 *                                                                          *
 *                                                                          *
 *      Addressing architecture is based on the Harvard addressing scheme.  *
 *                                                                          *
 \**************************************************************************/

#pragma once

#ifndef __PIC16C62X_H__
#define __PIC16C62X_H__




/**************************************************************************
 *  Internal Clock divisor
 *
 *  External Clock is divided internally by 4 for the instruction cycle
 *  times. (Each instruction cycle passes through 4 machine states). This
 *  is handled by the cpu execution engine.
 */

enum
{
	PIC16C62x_PC=1, PIC16C62x_STK0, PIC16C62x_STK1, PIC16C62x_STK2,
	PIC16C62x_STK3, PIC16C62x_STK4, PIC16C62x_STK5, PIC16C62x_STK6,
	PIC16C62x_STK7, PIC16C62x_FSR,	PIC16C62x_W,    PIC16C62x_ALU,
	PIC16C62x_STR,  PIC16C62x_OPT,	PIC16C62x_TMR0, PIC16C62x_PRTA,
	PIC16C62x_PRTB, PIC16C62x_WDT,  PIC16C62x_TRSA, PIC16C62x_TRSB,
	PIC16C62x_PSCL
};

#define PIC16C62x_T0		0


/****************************************************************************
 *  Function to configure the CONFIG register. This is actually hard-wired
 *  during ROM programming, so should be called in the driver INIT, with
 *  the value if known (available in HEX dumps of the ROM).
 */

void pic16c62x_set_config(const device_config *cpu, int data);



CPU_GET_INFO( pic16c620 );
#define CPU_PIC16C620 CPU_GET_INFO_NAME( pic16c620 )


CPU_GET_INFO( pic16c620a );
#define CPU_PIC16C620A CPU_GET_INFO_NAME( pic16c620a )


//CPU_GET_INFO( pic16cr620a );
//#define CPU_PIC16CR620A CPU_GET_INFO_NAME( pic16cr620a )


CPU_GET_INFO( pic16c621 );
#define CPU_PIC16C621 CPU_GET_INFO_NAME( pic16c621 )


CPU_GET_INFO( pic16c621a );
#define CPU_PIC16C621A CPU_GET_INFO_NAME( pic16c621a )

CPU_GET_INFO( pic16c622 );
#define CPU_PIC16C622 CPU_GET_INFO_NAME( pic16c622 )


CPU_GET_INFO( pic16c622a );
#define CPU_PIC16C622A CPU_GET_INFO_NAME( pic16c622a )


CPU_DISASSEMBLE( pic16c62x );


#endif	/* __PIC16C62X_H__ */
