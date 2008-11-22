 /**************************************************************************\
 *                      Alpha8201 Emulator                                  *
 *                                                                          *
 *                    Copyright Tatsuyuki Satoh                             *
 *                 Originally written for the MAME project.                 *
 *                                                                          *
 *                                                                          *
 \**************************************************************************/

#pragma once

#ifndef __ALPH8201_H__
#define __ALPH8201_H__

#ifndef INLINE
#define INLINE static inline
#endif

#include "cpuintrf.h"

enum
{
	ALPHA8201_PC=1,
	ALPHA8201_SP,
	ALPHA8201_RB,
	ALPHA8201_MB,
//
	ALPHA8201_CF,
	ALPHA8201_ZF,
//
	ALPHA8201_IX0,
	ALPHA8201_IX1,
	ALPHA8201_IX2,
	ALPHA8201_LP0,
	ALPHA8201_LP1,
	ALPHA8201_LP2,
	ALPHA8201_A,
	ALPHA8201_B,
//
	ALPHA8201_R0,ALPHA8201_R1,ALPHA8201_R2,ALPHA8201_R3,
	ALPHA8201_R4,ALPHA8201_R5,ALPHA8201_R6,ALPHA8201_R7
};

extern CPU_GET_INFO( alpha8201 );
extern CPU_GET_INFO( alpha8301 );

/*
 *   Read a UINT8 from given memory location
 */
#define ALPHA8201_RDMEM(A) ((unsigned)memory_read_byte_8le(R.program, A))

/*
 *   Write a UINT8 to given memory location
 */
#define ALPHA8201_WRMEM(A,V) (memory_write_byte_8le(R.program, A,V))

/*
 *   ALPHA8201_RDOP() is identical to ALPHA8201_RDMEM() except it is used for reading
 *   opcodes. In case of system with memory mapped I/O, this function can be
 *   used to greatly speed up emulation
 */
#define ALPHA8201_RDOP(A) ((unsigned)memory_decrypted_read_byte(R.program, A))

/*
 *   ALPHA8201_RDOP_ARG() is identical to ALPHA8201_RDOP() except it is used for reading
 *   opcode arguments. This difference can be used to support systems that
 *   use different encoding mechanisms for opcodes and opcode arguments
 */
#define ALPHA8201_RDOP_ARG(A) ((unsigned)memory_raw_read_byte(R.program, A))

CPU_DISASSEMBLE( ALPHA8201 );

#endif  /* __ALPH8201_H__ */
