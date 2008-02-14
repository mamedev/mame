/*** m6809: Portable 6809 emulator ******************************************/

#ifndef _M6809_H
#define _M6809_H

#include "cpuintrf.h"

enum {
	M6809_PC=1, M6809_S, M6809_CC ,M6809_A, M6809_B, M6809_U, M6809_X, M6809_Y,
	M6809_DP };

#define M6809_IRQ_LINE	0	/* IRQ line number */
#define M6809_FIRQ_LINE 1   /* FIRQ line number */

void m6809_get_info(UINT32 state, cpuinfo *info);
void m6809e_get_info(UINT32 state, cpuinfo *info);

/****************************************************************************/
/* Read a byte from given memory location                                   */
/****************************************************************************/
/* ASG 971005 -- changed to program_read_byte_8/cpu_writemem16 */
#define M6809_RDMEM(Addr) ((unsigned)program_read_byte_8(Addr))

/****************************************************************************/
/* Write a byte to given memory location                                    */
/****************************************************************************/
#define M6809_WRMEM(Addr,Value) (program_write_byte_8(Addr,Value))

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

#endif /* _M6809_H */
