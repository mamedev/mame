/*** konami: Portable Konami cpu emulator ******************************************/

#ifndef _KONAMI_H
#define _KONAMI_H

#include "cpuintrf.h"

enum {
	KONAMI_PC=1, KONAMI_S, KONAMI_CC ,KONAMI_A, KONAMI_B, KONAMI_U, KONAMI_X, KONAMI_Y,
	KONAMI_DP };

enum
{
	CPUINFO_PTR_KONAMI_SETLINES_CALLBACK = CPUINFO_PTR_CPU_SPECIFIC
};

#define KONAMI_IRQ_LINE	0	/* IRQ line number */
#define KONAMI_FIRQ_LINE 1   /* FIRQ line number */

/* PUBLIC FUNCTIONS */
void konami_get_info(UINT32 state, cpuinfo *info);

/****************************************************************************/
/* Read a byte from given memory location                                   */
/****************************************************************************/
#define KONAMI_RDMEM(Addr) ((unsigned)program_read_byte_8(Addr))

/****************************************************************************/
/* Write a byte to given memory location                                    */
/****************************************************************************/
#define KONAMI_WRMEM(Addr,Value) (program_write_byte_8(Addr,Value))

/****************************************************************************/
/* Z80_RDOP() is identical to Z80_RDMEM() except it is used for reading     */
/* opcodes. In case of system with memory mapped I/O, this function can be  */
/* used to greatly speed up emulation                                       */
/****************************************************************************/
#define KONAMI_RDOP(Addr) ((unsigned)cpu_readop(Addr))

/****************************************************************************/
/* Z80_RDOP_ARG() is identical to Z80_RDOP() except it is used for reading  */
/* opcode arguments. This difference can be used to support systems that    */
/* use different encoding mechanisms for opcodes and opcode arguments       */
/****************************************************************************/
#define KONAMI_RDOP_ARG(Addr) ((unsigned)cpu_readop_arg(Addr))

#ifndef FALSE
#    define FALSE 0
#endif
#ifndef TRUE
#    define TRUE (!FALSE)
#endif

#ifdef MAME_DEBUG
offs_t konami_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);
#endif

#endif /* _KONAMI_H */
