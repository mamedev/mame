/*** m6805: Portable 6805 emulator ******************************************/

#ifndef _M6805_H
#define _M6805_H

#include "cpuintrf.h"

enum { M6805_PC=1, M6805_S, M6805_CC, M6805_A, M6805_X, M6805_IRQ_STATE };

#define M6805_IRQ_LINE		0

#define M6805_CLOCK_DIVIDER	4

extern void m6805_get_info(UINT32 state, cpuinfo *info);

/****************************************************************************
 * 68705 section
 ****************************************************************************/
#if (HAS_M68705)
#define M68705_A					M6805_A
#define M68705_PC					M6805_PC
#define M68705_S					M6805_S
#define M68705_X					M6805_X
#define M68705_CC					M6805_CC
#define M68705_IRQ_STATE			M6805_IRQ_STATE

#define M68705_INT_MASK				0x03
#define M68705_IRQ_LINE				M6805_IRQ_LINE
#define M68705_INT_TIMER			0x01

#define M68705_CLOCK_DIVIDER		M6805_CLOCK_DIVIDER

extern void m68705_get_info(UINT32 state, cpuinfo *info);
#endif

/****************************************************************************
 * HD63705 section
 ****************************************************************************/
#if (HAS_HD63705)
#define HD63705_A					M6805_A
#define HD63705_PC					M6805_PC
#define HD63705_S					M6805_S
#define HD63705_X					M6805_X
#define HD63705_CC					M6805_CC
#define HD63705_NMI_STATE			M6805_IRQ_STATE
#define HD63705_IRQ1_STATE			M6805_IRQ_STATE+1
#define HD63705_IRQ2_STATE			M6805_IRQ_STATE+2
#define HD63705_ADCONV_STATE		M6805_IRQ_STATE+3

#define HD63705_INT_MASK			0x1ff

#define HD63705_INT_IRQ1			0x00
#define HD63705_INT_IRQ2			0x01
#define	HD63705_INT_TIMER1			0x02
#define	HD63705_INT_TIMER2			0x03
#define	HD63705_INT_TIMER3			0x04
#define	HD63705_INT_PCI				0x05
#define	HD63705_INT_SCI				0x06
#define	HD63705_INT_ADCONV			0x07
#define HD63705_INT_NMI				0x08

#define HD3705_CLOCK_DIVIDER		M6805_CLOCK_DIVIDER

extern void hd63705_get_info(UINT32 state, cpuinfo *info);
#endif

/****************************************************************************/
/* Read a byte from given memory location                                   */
/****************************************************************************/
#define M6805_RDMEM(Addr) ((unsigned)program_read_byte_8(Addr))

/****************************************************************************/
/* Write a byte to given memory location                                    */
/****************************************************************************/
#define M6805_WRMEM(Addr,Value) (program_write_byte_8(Addr,Value))

/****************************************************************************/
/* M6805_RDOP() is identical to M6805_RDMEM() except it is used for reading */
/* opcodes. In case of system with memory mapped I/O, this function can be  */
/* used to greatly speed up emulation                                       */
/****************************************************************************/
#define M6805_RDOP(Addr) ((unsigned)cpu_readop(Addr))

/****************************************************************************/
/* M6805_RDOP_ARG() is identical to M6805_RDOP() but it's used for reading  */
/* opcode arguments. This difference can be used to support systems that    */
/* use different encoding mechanisms for opcodes and opcode arguments       */
/****************************************************************************/
#define M6805_RDOP_ARG(Addr) ((unsigned)cpu_readop_arg(Addr))

#ifdef MAME_DEBUG
offs_t m6805_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);
#endif

#endif /* _M6805_H */
