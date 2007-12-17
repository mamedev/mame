 /**************************************************************************\
 *                      Alpha8201 Emulator                                  *
 *                                                                          *
 *                    Copyright (C) 2006 Tatsuyuki Satoh                    *
 *                 Originally written for the MAME project.                 *
 *                                                                          *
 *                                                                          *
 \**************************************************************************/

#ifndef _ALPH8202_H
#define _ALPH8202_H

#ifndef INLINE
#define INLINE static inline
#endif

#include "cpuintrf.h"

enum {
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

extern void ALPHA8201_get_info(UINT32 state, cpuinfo *info);
extern void ALPHA8301_get_info(UINT32 state, cpuinfo *info);

/*
 *   Read a UINT8 from given memory location
 */
#define ALPHA8201_RDMEM(A) ((unsigned)program_read_byte_8(A))

/*
 *   Write a UINT8 to given memory location
 */
#define ALPHA8201_WRMEM(A,V) (program_write_byte_8(A,V))

/*
 *   ALPHA8201_RDOP() is identical to ALPHA8201_RDMEM() except it is used for reading
 *   opcodes. In case of system with memory mapped I/O, this function can be
 *   used to greatly speed up emulation
 */
#define ALPHA8201_RDOP(A) ((unsigned)cpu_readop(A))

/*
 *   ALPHA8201_RDOP_ARG() is identical to ALPHA8201_RDOP() except it is used for reading
 *   opcode arguments. This difference can be used to support systems that
 *   use different encoding mechanisms for opcodes and opcode arguments
 */
#define ALPHA8201_RDOP_ARG(A) ((unsigned)cpu_readop_arg(A))

#ifdef  MAME_DEBUG
offs_t ALPHA8201_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);
#endif

#endif  /* _ALPHA8201_H */
