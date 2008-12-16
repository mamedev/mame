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


#include "cpuintrf.h"


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


/****************************************************************************
 *  Function to configure the CONFIG register. This is actually hard-wired
 *  during ROM programming, so should be called in the driver INIT, with
 *  the value if known (available in HEX dumps of the ROM).
 */

void pic16c5x_set_config(const device_config *cpu, int data);


/****************************************************************************
 *  Read the state of the T0 Clock input signal
 */

#define PIC16C5x_T0		0x10
#define PIC16C5x_T0_In (memory_read_byte_8le(cpustate->io, PIC16C5x_T0))


/****************************************************************************
 *  Input a word from given I/O port
 */

#define PIC16C5x_In(Port) ((UINT8)memory_read_byte_8le(cpustate->io, (Port)))


/****************************************************************************
 *  Output a word to given I/O port
 */

#define PIC16C5x_Out(Port,Value) (memory_write_byte_8le(cpustate->io, (Port),Value))



/****************************************************************************
 *  Read a word from given RAM memory location
 */

#define PIC16C5x_RAM_RDMEM(A) ((UINT8)memory_read_byte_8le(cpustate->data, A))


/****************************************************************************
 *  Write a word to given RAM memory location
 */

#define PIC16C5x_RAM_WRMEM(A,V) (memory_write_byte_8le(cpustate->data, A,V))



/****************************************************************************
 *  PIC16C5X_RDOP() is identical to PIC16C5X_RDMEM() except it is used for
 *  reading opcodes. In case of system with memory mapped I/O, this function
 *  can be used to greatly speed up emulation
 */

#define PIC16C5x_RDOP(A) (memory_decrypted_read_word(cpustate->program, (A)<<1))


/****************************************************************************
 *  PIC16C5X_RDOP_ARG() is identical to PIC16C5X_RDOP() except it is used
 *  for reading opcode arguments. This difference can be used to support systems
 *  that use different encoding mechanisms for opcodes and opcode arguments
 */

#define PIC16C5x_RDOP_ARG(A) (memory_raw_read_word(cpustate->program, (A)<<1))




#if (HAS_PIC16C54)
CPU_GET_INFO( pic16C54 );
#endif


#if (HAS_PIC16C55)
CPU_GET_INFO( pic16C55 );
#endif


#if (HAS_PIC16C56)
CPU_GET_INFO( pic16C56 );
#endif


#if (HAS_PIC16C57)
CPU_GET_INFO( pic16C57 );
#endif


#if (HAS_PIC16C58)
CPU_GET_INFO( pic16C58 );
#endif


#if (HAS_PIC16C54) || (HAS_PIC16C55) || (HAS_PIC16C56) || (HAS_PIC16C57) || (HAS_PIC16C58)
CPU_DISASSEMBLE( pic16c5x );
#endif


#endif	/* __PIC16C5X_H__ */
