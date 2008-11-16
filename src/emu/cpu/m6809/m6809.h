/*** m6809: Portable 6809 emulator ******************************************/

#pragma once

#ifndef __M6809_H__
#define __M6809_H__

#include "cpuintrf.h"

enum
{
	M6809_PC=1, M6809_S, M6809_CC ,M6809_A, M6809_B, M6809_U, M6809_X, M6809_Y,
	M6809_DP
};

#define M6809_IRQ_LINE	0	/* IRQ line number */
#define M6809_FIRQ_LINE 1   /* FIRQ line number */

CPU_GET_INFO( m6809 );

/* M6809e has LIC line to indicate opcode/data fetch */
CPU_GET_INFO( m6809e );

/****************************************************************************/
/* Read a byte from given memory location                                   */
/****************************************************************************/
/* ASG 971005 -- changed to program_read_byte_8/cpu_writemem16 */
#define M6809_RDMEM(Addr) ((unsigned)program_read_byte_8be(Addr))

/****************************************************************************/
/* Write a byte to given memory location                                    */
/****************************************************************************/
#define M6809_WRMEM(Addr,Value) (program_write_byte_8be(Addr,Value))

/****************************************************************************/
/* Z80_RDOP() is identical to Z80_RDMEM() except it is used for reading     */
/* opcodes. In case of system with memory mapped I/O, this function can be  */
/* used to greatly speed up emulation                                       */
/****************************************************************************/
#define M6809_RDOP(Addr) ((unsigned)cpu_readop(Addr))

/****************************************************************************/
/* Z80_RDOP_ARG() is identical to Z80_RDOP() except it is used for reading  */
/* opcode arguments. This difference can be used to support systems that    */
/* use different encoding mechanisms for opcodes and opcode arguments       */
/****************************************************************************/
#define M6809_RDOP_ARG(Addr) ((unsigned)cpu_readop_arg(Addr))

#ifndef FALSE
#    define FALSE 0
#endif
#ifndef TRUE
#    define TRUE (!FALSE)
#endif

CPU_DISASSEMBLE( m6809 );


typedef struct _m6809_config m6809_config;
struct _m6809_config
{
	UINT8	encrypt_only_first_byte;		/* encrypt only the first byte in 10 xx and 11 xx opcodes */
};

#endif /* __M6809_H__ */
