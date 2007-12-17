 /**************************************************************************\
 *                      Microchip PIC16C5x Emulator                         *
 *                                                                          *
 *                    Copyright (C) 2003+ Tony La Porta                     *
 *                 Originally written for the MAME project.                 *
 *                                                                          *
 *                                                                          *
 *      Addressing architecture is based on the Harvard addressing scheme.  *
 *                                                                          *
 \**************************************************************************/

#ifndef _PIC16C5X_H
#define _PIC16C5X_H


#include "cpuintrf.h"


/**************************************************************************
 *  Internal Clock divisor
 *
 *  External Clock is divided internally by 4 for the instruction cycle
 *  times. (Each instruction cycle passes through 4 machine states).
 */

#define PIC16C5x_CLOCK_DIVIDER		4


enum {
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

void pic16c5x_config(int data);


/****************************************************************************
 *  Read the state of the T0 Clock input signal
 */

#define PIC16C5x_T0		0x10
#define PIC16C5x_T0_In (io_read_byte_8(PIC16C5x_T0))


/****************************************************************************
 *  Input a word from given I/O port
 */

#define PIC16C5x_In(Port) ((UINT8)io_read_byte_8((Port)))


/****************************************************************************
 *  Output a word to given I/O port
 */

#define PIC16C5x_Out(Port,Value) (io_write_byte_8((Port),Value))



/****************************************************************************
 *  Read a word from given RAM memory location
 */

#define PIC16C5x_RAM_RDMEM(A) ((UINT8)data_read_byte_8(A))


/****************************************************************************
 *  Write a word to given RAM memory location
 */

#define PIC16C5x_RAM_WRMEM(A,V) (data_write_byte_8(A,V))



/****************************************************************************
 *  PIC16C5X_RDOP() is identical to PIC16C5X_RDMEM() except it is used for
 *  reading opcodes. In case of system with memory mapped I/O, this function
 *  can be used to greatly speed up emulation
 */

#define PIC16C5x_RDOP(A) (cpu_readop16((A)<<1))


/****************************************************************************
 *  PIC16C5X_RDOP_ARG() is identical to PIC16C5X_RDOP() except it is used
 *  for reading opcode arguments. This difference can be used to support systems
 *  that use different encoding mechanisms for opcodes and opcode arguments
 */

#define PIC16C5x_RDOP_ARG(A) (cpu_readop_arg16((A)<<1))




#if (HAS_PIC16C54)
void pic16C54_get_info(UINT32 state, cpuinfo *info);
#endif


#if (HAS_PIC16C55)
void pic16C55_get_info(UINT32 state, cpuinfo *info);
#endif


#if (HAS_PIC16C56)
void pic16C56_get_info(UINT32 state, cpuinfo *info);
#endif


#if (HAS_PIC16C57)
void pic16C57_get_info(UINT32 state, cpuinfo *info);
#endif


#if (HAS_PIC16C58)
void pic16C58_get_info(UINT32 state, cpuinfo *info);
#endif


#if (HAS_PIC16C54) || (HAS_PIC16C55) || (HAS_PIC16C56) || (HAS_PIC16C57) || (HAS_PIC16C58)
#ifdef MAME_DEBUG
offs_t pic16C5x_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);
#endif
#endif


#endif	/* _PIC16C5X_H */
