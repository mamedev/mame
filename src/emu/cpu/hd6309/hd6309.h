/*** hd6309: Portable 6309 emulator ******************************************/

#ifndef _HD6309_H
#define _HD6309_H

#include "cpuintrf.h"

enum {
	HD6309_PC=1, HD6309_S, HD6309_CC ,HD6309_A, HD6309_B, HD6309_U, HD6309_X, HD6309_Y, HD6309_DP,
	HD6309_E, HD6309_F, HD6309_V, HD6309_MD };

#define HD6309_IRQ_LINE 0	/* IRQ line number */
#define HD6309_FIRQ_LINE 1	 /* FIRQ line number */


/* PUBLIC FUNCTIONS */
void hd6309_get_info(UINT32 state, cpuinfo *info);

/****************************************************************************/
/* Read a byte from given memory location                                   */
/****************************************************************************/
/* ASG 971005 -- changed to program_read_byte_8/cpu_writemem16 */
#define HD6309_RDMEM(Addr) ((unsigned)program_read_byte_8(Addr))

/****************************************************************************/
/* Write a byte to given memory location                                    */
/****************************************************************************/
#define HD6309_WRMEM(Addr,Value) (program_write_byte_8(Addr,Value))

/****************************************************************************/
/* Z80_RDOP() is identical to Z80_RDMEM() except it is used for reading     */
/* opcodes. In case of system with memory mapped I/O, this function can be  */
/* used to greatly speed up emulation                                       */
/****************************************************************************/
#define HD6309_RDOP(Addr) ((unsigned)cpu_readop(Addr))

/****************************************************************************/
/* Z80_RDOP_ARG() is identical to Z80_RDOP() except it is used for reading  */
/* opcode arguments. This difference can be used to support systems that    */
/* use different encoding mechanisms for opcodes and opcode arguments       */
/****************************************************************************/
#define HD6309_RDOP_ARG(Addr) ((unsigned)cpu_readop_arg(Addr))

#ifndef FALSE
#	 define FALSE 0
#endif
#ifndef TRUE
#	 define TRUE (!FALSE)
#endif

#endif /* _HD6309_H */

