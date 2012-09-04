/*****************************************************************************
 *
 *   ops4510.h
 *   Addressing mode and opcode macros for 4510 CPU
 *
 *   Copyright Peter Trauner, all rights reserved.
 *   documentation preliminary databook
 *   documentation by michael steil mist@c64.org
 *   available at ftp://ftp.funet.fi/pub/cbm/c65
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     pullmoll@t-online.de
 *   - The author of this copywritten work reserves the right to change the
 *     terms of its usage and license at any time, including retroactively
 *   - This entire notice must remain in the source code.
 *
 *****************************************************************************/


/* 65ce02 ********************************************************
 * TXS  Transfer index X to stack LSB
 * no flags changed (sic!)
 * txs tys not interruptable
 ***************************************************************/
#undef TXS
#define TXS								\
	SPL = X;							\
	if (PEEK_OP() == 0x2b /*TYS*/ ) {	            \
		UINT8 op = RDOP();				\
		(*cpustate->insn[op])(cpustate);			\
	}

#undef NOP
#define NOP       \
  cpustate->interrupt_inhibit = 0;

/* c65 docu says transfer of axyz to the mapper register
   so no readback!? */
#define MAP 						\
  cpustate->interrupt_inhibit = 1;			\
  cpustate->low=cpustate->a|(cpustate->x<<8); \
  cpustate->high=cpustate->y|(cpustate->z<<8); \
  cpustate->mem[0]=(cpustate->low&0x1000) ?  (cpustate->low&0xfff)<<8:0;  \
  cpustate->mem[1]=(cpustate->low&0x2000) ?  (cpustate->low&0xfff)<<8:0;  \
  cpustate->mem[2]=(cpustate->low&0x4000) ?  (cpustate->low&0xfff)<<8:0;  \
  cpustate->mem[3]=(cpustate->low&0x8000) ?  (cpustate->low&0xfff)<<8:0;  \
  cpustate->mem[4]=(cpustate->high&0x1000) ? (cpustate->high&0xfff)<<8:0; \
  cpustate->mem[5]=(cpustate->high&0x2000) ? (cpustate->high&0xfff)<<8:0; \
  cpustate->mem[6]=(cpustate->high&0x4000) ? (cpustate->high&0xfff)<<8:0; \
  cpustate->mem[7]=(cpustate->high&0x8000) ? (cpustate->high&0xfff)<<8:0; \
  cpustate->icount -= 3; \
  { \
				UINT8 op = RDOP();								\
				(*cpustate->insn[op])(cpustate);							\
  }

#undef RDMEM_ID
#undef WRMEM_ID
#define RDMEM_ID(a)   (cpustate->rdmem_id.isnull() ? cpustate->space->read_byte(M4510_MEM(a)) : cpustate->rdmem_id(M4510_MEM(a)))
#define WRMEM_ID(a,d) (cpustate->wrmem_id.isnull() ? cpustate->space->write_byte(M4510_MEM(a),d) : cpustate->wrmem_id(M4510_MEM(a),d))
