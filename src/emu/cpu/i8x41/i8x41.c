/*****************************************************************************
 *
 *   i8x41.c
 *   Portable UPI-41/8041/8741/8042/8742 emulator V0.2
 *
 *   Copyright Juergen Buchmueller, all rights reserved.
 *   You can contact me at juergen@mame.net or pullmoll@stop1984.com
 *
 *   - This source code is released as freeware for non-commercial purposes
 *     as part of the M.A.M.E. (Multiple Arcade Machine Emulator) project.
 *     The licensing terms of MAME apply to this piece of code for the MAME
 *     project and derviative works, as defined by the MAME license. You
 *     may opt to make modifications, improvements or derivative works under
 *     that same conditions, and the MAME project may opt to keep
 *     modifications, improvements or derivatives under their terms exclusively.
 *
 *   - Alternatively you can choose to apply the terms of the "GPL" (see
 *     below) to this - and only this - piece of code or your derivative works.
 *     Note that in no case your choice can have any impact on any other
 *     source code of the MAME project, or binary, or executable, be it closely
 *     or losely related to this piece of code.
 *
 *  -  At your choice you are also free to remove either licensing terms from
 *     this file and continue to use it under only one of the two licenses. Do this
 *     if you think that licenses are not compatible (enough) for you, or if you
 *     consider either license 'too restrictive' or 'too free'.
 *
 *  -  GPL (GNU General Public License)
 *     This program is free software; you can redistribute it and/or
 *     modify it under the terms of the GNU General Public License
 *     as published by the Free Software Foundation; either version 2
 *     of the License, or (at your option) any later version.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with this program; if not, write to the Free Software
 *     Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *
 *  This work is solely based on the
 *  'Intel(tm) UPI(tm)-41AH/42AH Users Manual'
 *
 *
 *  **** Change Log ****
 *  Wilbert Pol (24-Jun-2008) changed version to 0.6
 *   - Updated the ram sizes. 8041 uses 128 bytes, 8042
 *     uses 256 bytes.
 *   - Added support for re-enabling interrupts inside
 *     an interrupt handler.
 *   - Fixed cycle count for DJNZ instruction.
 *
 *  Wilbert Pol (23-Jun-2008) changed version to 0.5
 *   - Added configurable i8x41/i8x42 subtype support.
 *   - Fixed disassembly for opcode 0x67.
 *   - Fixed carry flag handling in ADDC A,#N instruction.
 *   - Fixed carry flag handling in RLC A instruction.
 *
 *  Wilbert Pol (22-Jun-2008) changed version to 0.4
 *   - Removed i8x41.ram hack.
 *
 *  HJB (19-Dec-2004) changed version to 0.3
 *   - Tried to handle accesses to registers in get_info/set_info
 *     before i8x41.ram is is initialized.
 *   - cosmetics: readability in get_info/set_info, replaced non-ASCII
 *     codes in comments, add 'ex' tabstop definition
 *
 *  TLP (10-Jan-2003) Changed ver from 0.1 to 0.2
 *   - Changed the internal RAM mask from 3Fh to FFh . The i8x41/i8x42 have
 *     128/256 bytes of internal RAM respectively.
 *   - Added output port data to the debug register view window.
 *   - Added some missing break commands to the set_reg switch function.
 *   - Changed Ports 1 and 2 to latched types (Quasi-bidirectional).
 *   - Stopped illegal access to Port 0 and 3 (they don't exist).
 *   - Changed ANLD, ORLD and MOVD instructions to act through Port 2 in
 *     nibble mode.
 *   - Copied F0 and moved F1 flags to the STATE flag bits where they belong.
 *   - Corrected the 'addr' field by changing it from UINT8 to UINT16 for:
 *     'INC @Rr' 'MOV @Rr,A' 'MOV @Rr,#N' 'XCH A,@Rr' 'XCHD A,@Rr'
 *   - Added mask to TIMER when the TEST1 Counter overflows.
 *   - Seperated the prescaler out of the timer/counter, in order to correct
 *     the TEST1 input counter step.
 *   - Moved TEST0 and TEST1 status flags out of the STATE register.
 *     STATE register uses these upper bits for user definable purposes.
 *   - TEST0 and TEST1 input lines are now sampled during the JTx/JNTx
 *     instructions.
 *   - Two methods for updating TEST1 input during counter mode are now
 *     supported depending on the mode of use required.
 *     You can use the Interrupt method, or input port read method.
 *   - TIMER is now only controlled by the timer or counter (not both)
 *     ie, When Starting the Counter, Stop the Timer and viceversa.
 *   - Nested IRQs of any sort are no longer allowed, however IRQs can
 *     become pending while a current interrupt is being serviced.
 *   - IBF Interrupt now has priority over the Timer Interrupt, when they
 *     occur simultaneously.
 *   - Add the external Interrupt FLAGS (Port 24, Port 25).
 *  To Do:
 *   - Add the external DMA FLAGS (Port 26, Port 27).  Page 4 and 37
 *
 *****************************************************************************/

/*
    Chip   RAM  ROM
    ----   ---  ---
    8041   128   1k  (ROM)
    8741   128   1k  (EPROM)
    8042   256   2k  (ROM)
    8742   256   2k  (EPROM)

    http://cpucharts.wallsoferyx.net/icmatrix0.html

*/

#include "debugger.h"
#include "i8x41.h"

typedef struct _upi41_state_t upi41_state_t;
struct _upi41_state_t {
	UINT16	ppc;
	UINT16	pc;
	UINT8	timer;
	UINT8	prescaler;
	UINT16	subtype;
	UINT8	a;
	UINT8	psw;
	UINT8	state;
	UINT8	enable;
	UINT8	control;
	UINT8	dbbi;
	UINT8	dbbo;
	UINT8	p1;
	UINT8	p2;
	UINT8	p2_hs;
	UINT8	ram_mask;
	cpu_irq_callback irq_callback;
	const device_config *device;
	const address_space *program;
	const address_space *data;
	const address_space *io;
	int		icount;
};

#define RM(s,a)	memory_read_byte_8le((s)->program, a)

#define IRAM_R(s,a)	memory_read_byte_8le((s)->data, (a) & upi41_state->ram_mask)
#define IRAM_W(s,a,v) memory_write_byte_8le((s)->data, (a)  & upi41_state->ram_mask, v)

#define RP(s,a)	memory_read_byte_8le((s)->io, a)
#define WP(s,a,v) memory_write_byte_8le((s)->io, a,v)

#define ROP(s,pc) memory_decrypted_read_byte((s)->program, pc)
#define ROP_ARG(s,pc) memory_raw_read_byte((s)->program, pc)

/* PC vectors */
#define V_RESET 0x000	/* power on address */
#define V_IBF	0x003	/* input buffer full interrupt vector */
#define V_TIMER 0x007	/* timer/counter interrupt vector */

/*
 * Memory locations
 * Note:
 * 000-3ff      internal ROM for 8x41 (1K)
 * 400-7ff      (more) internal for 8x42 type (2K)
 * 800-8ff      internal RAM
 */
#define M_BANK0 0x000	/* register bank 0 (8 times 8 bits) */
#define M_STACK 0x008	/* stack (8 times 16 bits) */
#define M_BANK1 0x018	/* register bank 1 (8 times 8 bits) */
#define M_USER	0x020	/* user memory (224 times 8 bits) */

/* PSW flag bits */
#define FC		0x80	/* carry flag */
#define FA		0x40	/* auxiliary carry flag */
#define Ff0		0x20	/* flag 0 - same flag as F0 below */
#define BS		0x10	/* bank select */
#define FU		0x08	/* unused */
#define SP		0x07	/* lower three bits are used as stack pointer */

/* STATE flag bits */
#define OBF 	0x01	/* output buffer full */
#define IBF 	0x02	/* input buffer full */
#define F0		0x04	/* flag 0 - same flag as Ff0 above */
#define F1		0x08	/* flag 1 */

/* ENABLE flag bits */
#define IBFI	0x01	/* input buffer full interrupt */
#define TCNTI	0x02	/* timer/counter interrupt */
#define DMA 	0x04	/* DMA mode */
#define FLAGS	0x08	/* FLAGS mode */
#define T		0x10	/* timer */
#define CNT 	0x20	/* counter */

/* CONTROL flag bits */
#define IBFI_IGNR	0x01	/* IBFI interrupt should be ignored */
#define IBFI_PEND	0x02	/* IBFI is pending */
#define TIRQ_IGNR	0x04	/* Timer interrupt should be ignored */
#define TIRQ_PEND	0x08	/* Timer interrupt is pending */
#define TEST1		0x10	/* Test1 line mode */
#define TOVF		0x20	/* Timer Overflow Flag */

#define IRQ_IGNR	0x05	/* Mask for IRQs being serviced */
#define IRQ_PEND	0x0a	/* Mask for IRQs pending */


/* shorter names for the I8x41 structure elements */
#define PPC 		upi41_state->ppc
#define PC			upi41_state->pc
#define A			upi41_state->a
#define PSW 		upi41_state->psw
#define DBBI		upi41_state->dbbi
#define DBBO		upi41_state->dbbo
#define STATE		upi41_state->state
#define ENABLE		upi41_state->enable
#define PRESCALER	upi41_state->prescaler
#define P1			upi41_state->p1
#define P2			upi41_state->p2
#define P2_HS		upi41_state->p2_hs		/* Port 2 Hand Shaking */
#define CONTROL		upi41_state->control


#define GETR(s,n) (IRAM_R((s), ((PSW & BS) ? M_BANK1:M_BANK0)+(n)))
#define SETR(s,n,v) (IRAM_W((s), ((PSW & BS) ? M_BANK1:M_BANK0)+(n), (v)))

static void set_irq_line(upi41_state_t *upi41_state, int irqline, int state);

/************************************************************************
 *  Shortcuts
 ************************************************************************/

INLINE void push_pc_to_stack(upi41_state_t *upi41_state)
{
	IRAM_W( upi41_state, M_STACK + (PSW&SP) * 2 + 0, PC & 0xff);
	IRAM_W( upi41_state, M_STACK + (PSW&SP) * 2 + 1, ((PC >> 8) & 0x0f) | (PSW & 0xf0) );
	PSW = (PSW & ~SP) | ((PSW + 1) & SP);
}

#define PUSH_PC_TO_STACK() push_pc_to_stack(upi41_state)

/***********************************************************************
 *  Opcodes
 ***********************************************************************/

#include "i8x41ops.c"

/***********************************************************************
 *  Execute a single opcode
 ***********************************************************************/

INLINE void execute_op(upi41_state_t *upi41_state, UINT8 op)
{
	switch( op )
	{
		/* opcode cycles bitmask */
		case 0x00: /* 1: 0000 0000 */
			nop(upi41_state, op);
			break;
		case 0x01: /* 1: 0000 0001 */
			illegal(upi41_state, op);
			break;
		case 0x02: /* 1: 0000 0010 */
			out_dbb_a(upi41_state, op);
			break;
		case 0x03: /* 2: 0000 0011 */
			add_i(upi41_state, op);
			break;
		case 0x04: /* 2: aaa0 0100 */
			jmp_i(upi41_state, op);
			break;
		case 0x05: /* 1: 0000 0101 */
			en_i(upi41_state, op);
			break;
		case 0x06: /* 1: 0000 0110 */
			illegal(upi41_state, op);
			break;
		case 0x07: /* 1: 0000 0111 */
			dec_a(upi41_state, op);
			break;
		case 0x08: /* 2: 0000 10pp */
		case 0x09: /* 2: 0000 10pp */
		case 0x0a: /* 2: 0000 10pp */
		case 0x0b: /* 2: 0000 10pp */
			in_a_p(upi41_state, op & 3);
			break;
		case 0x0c: /* 2: 0000 11pp */
		case 0x0d: /* 2: 0000 11pp */
		case 0x0e: /* 2: 0000 11pp */
		case 0x0f: /* 2: 0000 11pp */
			movd_a_p(upi41_state, op & 3);
			break;
		case 0x10: /* 1: 0001 000r */
			inc_rm(upi41_state, 0);
			break;
		case 0x11: /* 1: 0001 000r */
			inc_rm(upi41_state, 1);
			break;
		case 0x12: /* 2: bbb1 0010 */
			jbb_i(upi41_state, 0);
			break;
		case 0x13: /* 2: 0001 0011 */
			addc_i(upi41_state, op);
			break;
		case 0x14: /* 2: aaa1 0100 */
			call_i(upi41_state, op);
			break;
		case 0x15: /* 1: 0001 0101 */
			dis_i(upi41_state, op);
			break;
		case 0x16: /* 2: 0001 0110 */
			jtf_i(upi41_state, op);
			break;
		case 0x17: /* 1: 0001 0111 */
			inc_a(upi41_state, op);
			break;
		case 0x18: /* 1: 0001 1rrr */
		case 0x19: /* 1: 0001 1rrr */
		case 0x1a: /* 1: 0001 1rrr */
		case 0x1b: /* 1: 0001 1rrr */
		case 0x1c: /* 1: 0001 1rrr */
		case 0x1d: /* 1: 0001 1rrr */
		case 0x1e: /* 1: 0001 1rrr */
		case 0x1f: /* 1: 0001 1rrr */
			inc_r(upi41_state, op & 7);
			break;
		case 0x20: /* 1: 0010 000r */
			xch_a_rm(upi41_state, 0);
			break;
		case 0x21: /* 1: 0010 000r */
			xch_a_rm(upi41_state, 1);
			break;
		case 0x22: /* 1: 0010 0010 */
			in_a_dbb(upi41_state, op);
			break;
		case 0x23: /* 2: 0010 0011 */
			mov_a_i(upi41_state, op);
			break;
		case 0x24: /* 2: aaa0 0100 */
			jmp_i(upi41_state, op);
			break;
		case 0x25: /* 1: 0010 0101 */
			en_tcnti(upi41_state, op);
			break;
		case 0x26: /* 2: 0010 0110 */
			jnt0_i(upi41_state, op);
			break;
		case 0x27: /* 1: 0010 0111 */
			clr_a(upi41_state, op);
			break;
		case 0x28: /* 1: 0010 1rrr */
		case 0x29: /* 1: 0010 1rrr */
		case 0x2a: /* 1: 0010 1rrr */
		case 0x2b: /* 1: 0010 1rrr */
		case 0x2c: /* 1: 0010 1rrr */
		case 0x2d: /* 1: 0010 1rrr */
		case 0x2e: /* 1: 0010 1rrr */
		case 0x2f: /* 1: 0010 1rrr */
			xch_a_r(upi41_state, op & 7);
			break;
		case 0x30: /* 1: 0011 000r */
			xchd_a_rm(upi41_state, 0);
			break;
		case 0x31: /* 1: 0011 000r */
			xchd_a_rm(upi41_state, 1);
			break;
		case 0x32: /* 2: bbb1 0010 */
			jbb_i(upi41_state, 1);
			break;
		case 0x33: /* 1: 0011 0101 */
			illegal(upi41_state, op);
			break;
		case 0x34: /* 2: aaa1 0100 */
			call_i(upi41_state, op);
			break;
		case 0x35: /* 1: 0000 0101 */
			dis_tcnti(upi41_state, op);
			break;
		case 0x36: /* 2: 0011 0110 */
			jt0_i(upi41_state, op);
			break;
		case 0x37: /* 1: 0011 0111 */
			cpl_a(upi41_state, op);
			break;
		case 0x38: /* 2: 0011 10pp */
		case 0x39: /* 2: 0011 10pp */
		case 0x3a: /* 2: 0011 10pp */
		case 0x3b: /* 2: 0011 10pp */
			out_p_a(upi41_state, op & 3);
			break;
		case 0x3c: /* 2: 0011 11pp */
		case 0x3d: /* 2: 0011 11pp */
		case 0x3e: /* 2: 0011 11pp */
		case 0x3f: /* 2: 0011 11pp */
			movd_p_a(upi41_state, op & 3);
			break;
		case 0x40: /* 1: 0100 000r */
			orl_rm(upi41_state, 0);
			break;
		case 0x41: /* 1: 0100 000r */
			orl_rm(upi41_state, 1);
			break;
		case 0x42: /* 1: 0100 0010 */
			mov_a_t(upi41_state, op);
			break;
		case 0x43: /* 2: 0100 0011 */
			orl_i(upi41_state, op);
			break;
		case 0x44: /* 2: aaa0 0100 */
			jmp_i(upi41_state, op);
			break;
		case 0x45: /* 1: 0100 0101 */
			strt_cnt(upi41_state, op);
			break;
		case 0x46: /* 2: 0100 0110 */
			jnt1_i(upi41_state, op);
			break;
		case 0x47: /* 1: 0100 0111 */
			swap_a(upi41_state, op);
			break;
		case 0x48: /* 1: 0100 1rrr */
		case 0x49: /* 1: 0100 1rrr */
		case 0x4a: /* 1: 0100 1rrr */
		case 0x4b: /* 1: 0100 1rrr */
		case 0x4c: /* 1: 0100 1rrr */
		case 0x4d: /* 1: 0100 1rrr */
		case 0x4e: /* 1: 0100 1rrr */
		case 0x4f: /* 1: 0100 1rrr */
			orl_r(upi41_state, op & 7);
			break;
		case 0x50: /* 1: 0101 000r */
			anl_rm(upi41_state, 0);
			break;
		case 0x51: /* 1: 0101 000r */
			anl_rm(upi41_state, 1);
			break;
		case 0x52: /* 2: bbb1 0010 */
			jbb_i(upi41_state, 2);
			break;
		case 0x53: /* 2: 0101 0011 */
			anl_i(upi41_state, op);
			break;
		case 0x54: /* 2: aaa1 0100 */
			call_i(upi41_state, op);
			break;
		case 0x55: /* 1: 0101 0101 */
			strt_t(upi41_state, op);
			break;
		case 0x56: /* 2: 0101 0110 */
			jt1_i(upi41_state, op);
			break;
		case 0x57: /* 1: 0101 0111 */
			da_a(upi41_state, op);
			break;
		case 0x58: /* 1: 0101 1rrr */
		case 0x59: /* 1: 0101 1rrr */
		case 0x5a: /* 1: 0101 1rrr */
		case 0x5b: /* 1: 0101 1rrr */
		case 0x5c: /* 1: 0101 1rrr */
		case 0x5d: /* 1: 0101 1rrr */
		case 0x5e: /* 1: 0101 1rrr */
		case 0x5f: /* 1: 0101 1rrr */
			anl_r(upi41_state, op & 7);
			break;
		case 0x60: /* 1: 0110 000r */
			add_rm(upi41_state, 0);
			break;
		case 0x61: /* 1: 0110 000r */
			add_rm(upi41_state, 1);
			break;
		case 0x62: /* 1: 0110 0010 */
			mov_t_a(upi41_state, op);
			break;
		case 0x63: /* 1: 0110 0011 */
			illegal(upi41_state, op);
			break;
		case 0x64: /* 2: aaa0 0100 */
			jmp_i(upi41_state, op);
			break;
		case 0x65: /* 1: 0110 0101 */
			stop_tcnt(upi41_state, op);
			break;
		case 0x66: /* 1: 0110 0110 */
			illegal(upi41_state, op);
			break;
		case 0x67: /* 1: 0110 0111 */
			rrc_a(upi41_state, op);
			break;
		case 0x68: /* 1: 0110 1rrr */
		case 0x69: /* 1: 0110 1rrr */
		case 0x6a: /* 1: 0110 1rrr */
		case 0x6b: /* 1: 0110 1rrr */
		case 0x6c: /* 1: 0110 1rrr */
		case 0x6d: /* 1: 0110 1rrr */
		case 0x6e: /* 1: 0110 1rrr */
		case 0x6f: /* 1: 0110 1rrr */
			add_r(upi41_state, op & 7);
			break;
		case 0x70: /* 1: 0111 000r */
			addc_rm(upi41_state, 0);
			break;
		case 0x71: /* 1: 0111 000r */
			addc_rm(upi41_state, 1);
			break;
		case 0x72: /* 2: bbb1 0010 */
			jbb_i(upi41_state, 3);
			break;
		case 0x73: /* 1: 0111 0011 */
			illegal(upi41_state, op);
			break;
		case 0x74: /* 2: aaa1 0100 */
			call_i(upi41_state, op);
			break;
		case 0x75: /* 1: 0111 0101 */
			illegal(upi41_state, op);
			break;
		case 0x76: /* 2: 0111 0110 */
			jf1_i(upi41_state, op);
			break;
		case 0x77: /* 1: 0111 0111 */
			rr_a(upi41_state, op);
			break;
		case 0x78: /* 1: 0111 1rrr */
		case 0x79: /* 1: 0111 1rrr */
		case 0x7a: /* 1: 0111 1rrr */
		case 0x7b: /* 1: 0111 1rrr */
		case 0x7c: /* 1: 0111 1rrr */
		case 0x7d: /* 1: 0111 1rrr */
		case 0x7e: /* 1: 0111 1rrr */
		case 0x7f: /* 1: 0111 1rrr */
			addc_r(upi41_state, op & 7);
			break;
		case 0x80: /* 1: 1000 0000 */
			illegal(upi41_state, op);
			break;
		case 0x81: /* 1: 1000 0001 */
			illegal(upi41_state, op);
			break;
		case 0x82: /* 1: 1000 0010 */
			illegal(upi41_state, op);
			break;
		case 0x83: /* 2: 1000 0011 */
			ret(upi41_state, op);
			break;
		case 0x84: /* 2: aaa0 0100 */
			jmp_i(upi41_state, op);
			break;
		case 0x85: /* 1: 1000 0101 */
			clr_f0(upi41_state, op);
			break;
		case 0x86: /* 2: 1000 0110 */
			jobf_i(upi41_state, op);
			break;
		case 0x87: /* 1: 1000 0111 */
			illegal(upi41_state, op);
			break;
		case 0x88: /* 2: 1000 10pp */
		case 0x89: /* 2: 1000 10pp */
		case 0x8a: /* 2: 1000 10pp */
		case 0x8b: /* 2: 1000 10pp */
			orl_p_i(upi41_state, op & 3);
			break;
		case 0x8c: /* 2: 1000 11pp */
		case 0x8d: /* 2: 1000 11pp */
		case 0x8e: /* 2: 1000 11pp */
		case 0x8f: /* 2: 1000 11pp */
			orld_p_a(upi41_state, op & 7);
			break;
		case 0x90: /* 1: 1001 0000 */
			mov_sts_a(upi41_state, op);
			break;
		case 0x91: /* 1: 1001 0001 */
			illegal(upi41_state, op);
			break;
		case 0x92: /* 2: bbb1 0010 */
			jbb_i(upi41_state, 4);
			break;
		case 0x93: /* 2: 1001 0011 */
			retr(upi41_state, op);
			break;
		case 0x94: /* 1: aaa1 0100 */
			call_i(upi41_state, op);
			break;
		case 0x95: /* 1: 1001 0101 */
			cpl_f0(upi41_state, op);
			break;
		case 0x96: /* 2: 1001 0110 */
			jnz_i(upi41_state, op);
			break;
		case 0x97: /* 1: 1001 0111 */
			clr_c(upi41_state, op);
			break;
		case 0x98: /* 2: 1001 10pp , illegal port */
		case 0x99: /* 2: 1001 10pp */
		case 0x9a: /* 2: 1001 10pp */
		case 0x9b: /* 2: 1001 10pp , illegal port */
			anl_p_i(upi41_state, op & 3);
			break;
		case 0x9c: /* 2: 1001 11pp */
		case 0x9d: /* 2: 1001 11pp */
		case 0x9e: /* 2: 1001 11pp */
		case 0x9f: /* 2: 1001 11pp */
			anld_p_a(upi41_state, op & 7);
			break;
		case 0xa0: /* 1: 1010 000r */
			mov_rm_a(upi41_state, 0);
			break;
		case 0xa1: /* 1: 1010 000r */
			mov_rm_a(upi41_state, 1);
			break;
		case 0xa2: /* 1: 1010 0010 */
			illegal(upi41_state, op);
			break;
		case 0xa3: /* 2: 1010 0011 */
			movp_a_am(upi41_state, op);
			break;
		case 0xa4: /* 2: aaa0 0100 */
			jmp_i(upi41_state, op);
			break;
		case 0xa5: /* 1: 1010 0101 */
			clr_f1(upi41_state, op);
			break;
		case 0xa6: /* 1: 1010 0110 */
			illegal(upi41_state, op);
			break;
		case 0xa7: /* 1: 1010 0111 */
			cpl_c(upi41_state, op);
			break;
		case 0xa8: /* 1: 1010 1rrr */
		case 0xa9: /* 1: 1010 1rrr */
		case 0xaa: /* 1: 1010 1rrr */
		case 0xab: /* 1: 1010 1rrr */
		case 0xac: /* 1: 1010 1rrr */
		case 0xad: /* 1: 1010 1rrr */
		case 0xae: /* 1: 1010 1rrr */
		case 0xaf: /* 1: 1010 1rrr */
			mov_r_a(upi41_state, op & 7);
			break;
		case 0xb0: /* 2: 1011 000r */
			mov_rm_i(upi41_state, 0);
			break;
		case 0xb1: /* 2: 1011 000r */
			mov_rm_i(upi41_state, 1);
			break;
		case 0xb2: /* 2: bbb1 0010 */
			jbb_i(upi41_state, 5);
			break;
		case 0xb3: /* 2: 1011 0011 */
			jmpp_a(upi41_state, op);
			break;
		case 0xb4: /* 2: aaa1 0100 */
			call_i(upi41_state, op);
			break;
		case 0xb5: /* 1: 1011 0101 */
			cpl_f1(upi41_state, op);
			break;
		case 0xb6: /* 2: 1011 0110 */
			jf0_i(upi41_state, op);
			break;
		case 0xb7: /* 1: 1011 0111 */
			illegal(upi41_state, op);
			break;
		case 0xb8: /* 2: 1011 1rrr */
		case 0xb9: /* 2: 1011 1rrr */
		case 0xba: /* 2: 1011 1rrr */
		case 0xbb: /* 2: 1011 1rrr */
		case 0xbc: /* 2: 1011 1rrr */
		case 0xbd: /* 2: 1011 1rrr */
		case 0xbe: /* 2: 1011 1rrr */
		case 0xbf: /* 2: 1011 1rrr */
			mov_r_i(upi41_state, op & 7);
			break;
		case 0xc0: /* 1: 1100 0000 */
			illegal(upi41_state, op);
			break;
		case 0xc1: /* 1: 1100 0001 */
			illegal(upi41_state, op);
			break;
		case 0xc2: /* 1: 1100 0010 */
			illegal(upi41_state, op);
			break;
		case 0xc3: /* 1: 1100 0011 */
			illegal(upi41_state, op);
			break;
		case 0xc4: /* 2: aaa0 0100 */
			jmp_i(upi41_state, op);
			break;
		case 0xc5: /* 1: 1100 0101 */
			sel_rb0(upi41_state, op);
			break;
		case 0xc6: /* 2: 1100 0110 */
			jz_i(upi41_state, op);
			break;
		case 0xc7: /* 1: 1100 0111 */
			mov_a_psw(upi41_state, op);
			break;
		case 0xc8: /* 1: 1100 1rrr */
		case 0xc9: /* 1: 1100 1rrr */
		case 0xca: /* 1: 1100 1rrr */
		case 0xcb: /* 1: 1100 1rrr */
		case 0xcc: /* 1: 1100 1rrr */
		case 0xcd: /* 1: 1100 1rrr */
		case 0xcf: /* 1: 1100 1rrr */
			dec_r(upi41_state, op & 7);
			break;
		case 0xd0: /* 1: 1101 000r */
			xrl_rm(upi41_state, 0);
			break;
		case 0xd1: /* 1: 1101 000r */
			xrl_rm(upi41_state, 1);
			break;
		case 0xd2: /* 2: bbb1 0010 */
			jbb_i(upi41_state, 6);
			break;
		case 0xd3: /* 1: 1101 0011 */
			xrl_i(upi41_state, op);
			break;
		case 0xd4: /* 2: aaa1 0100 */
			call_i(upi41_state, op);
			break;
		case 0xd5: /* 1: 1101 0101 */
			sel_rb1(upi41_state, op);
			break;
		case 0xd6: /* 2: 1101 0110 */
			jnibf_i(upi41_state, op);
			break;
		case 0xd7: /* 1: 1101 0111 */
			mov_psw_a(upi41_state, op);
			break;
		case 0xd8: /* 1: 1101 1rrr */
		case 0xd9: /* 1: 1101 1rrr */
		case 0xda: /* 1: 1101 1rrr */
		case 0xdb: /* 1: 1101 1rrr */
		case 0xdc: /* 1: 1101 1rrr */
		case 0xdd: /* 1: 1101 1rrr */
		case 0xde: /* 1: 1101 1rrr */
		case 0xdf: /* 1: 1101 1rrr */
			xrl_r(upi41_state, op & 7);
			break;
		case 0xe0: /* 1: 1110 0000 */
			illegal(upi41_state, op);
			break;
		case 0xe1: /* 1: 1110 0001 */
			illegal(upi41_state, op);
			break;
		case 0xe2: /* 1: 1110 0010 */
			illegal(upi41_state, op);
			break;
		case 0xe3: /* 2: 1110 0011 */
			movp3_a_am(upi41_state, op);
			break;
		case 0xe4: /* 2: aaa0 0100 */
			jmp_i(upi41_state, op);
			break;
		case 0xe5: /* 1: 1110 0101 */
			en_dma(upi41_state, op);
			break;
		case 0xe6: /* 2: 1110 0110 */
			jnc_i(upi41_state, op);
			break;
		case 0xe7: /* 1: 1110 0111 */
			rl_a(upi41_state, op);
			break;
		case 0xe8: /* 2: 1110 1rrr */
		case 0xe9: /* 2: 1110 1rrr */
		case 0xea: /* 2: 1110 1rrr */
		case 0xeb: /* 2: 1110 1rrr */
		case 0xec: /* 2: 1110 1rrr */
		case 0xed: /* 2: 1110 1rrr */
		case 0xee: /* 2: 1110 1rrr */
		case 0xef: /* 2: 1110 1rrr */
			djnz_r_i(upi41_state, op & 7);
			break;
		case 0xf0: /* 1: 1111 000r */
			mov_a_rm(upi41_state, 0);
			break;
		case 0xf1: /* 1: 1111 000r */
			mov_a_rm(upi41_state, 1);
			break;
		case 0xf2: /* 2: bbb1 0010 */
			jbb_i(upi41_state, 7);
			break;
		case 0xf3: /* 1: 1111 0011 */
			illegal(upi41_state, op);
			break;
		case 0xf4: /* 2: aaa1 0100 */
			call_i(upi41_state, op);
			break;
		case 0xf5: /* 1: 1111 0101 */
			en_flags(upi41_state, op);
			break;
		case 0xf6: /* 2: 1111 0110 */
			jc_i(upi41_state, op);
			break;
		case 0xf7: /* 1: 1111 0111 */
			rlc_a(upi41_state, op);
			break;
		case 0xf8: /* 1: 1111 1rrr */
		case 0xf9: /* 1: 1111 1rrr */
		case 0xfa: /* 1: 1111 1rrr */
		case 0xfb: /* 1: 1111 1rrr */
		case 0xfc: /* 1: 1111 1rrr */
		case 0xfd: /* 1: 1111 1rrr */
		case 0xfe: /* 1: 1111 1rrr */
		case 0xff: /* 1: 1111 1rrr */
			mov_a_r(upi41_state, op & 7);
			break;
	}
}

/***********************************************************************
 *  Cycle Timings
 ***********************************************************************/

static const UINT8 i8x41_cycles[] = {
	1,1,1,2,2,1,1,1,2,2,2,2,2,2,2,2,
	1,1,2,2,2,1,2,1,1,1,1,1,1,1,1,1,
	1,1,1,2,2,1,2,1,1,1,1,1,1,1,1,1,
	1,1,2,1,2,1,2,1,2,2,2,2,2,2,2,2,
	1,1,1,2,2,1,2,1,1,1,1,1,1,1,1,1,
	1,1,2,2,2,1,2,1,1,1,1,1,1,1,1,1,
	1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,
	1,1,2,1,2,1,2,1,1,1,1,1,1,1,1,1,
	1,1,1,2,2,1,2,1,2,2,2,2,2,2,2,2,
	1,1,2,2,1,1,2,1,2,2,2,2,2,2,2,2,
	1,1,1,2,2,1,1,1,1,1,1,1,1,1,1,1,
	2,2,2,2,2,1,2,1,2,2,2,2,2,2,2,2,
	1,1,1,1,2,1,2,1,1,1,1,1,1,1,1,1,
	1,1,2,1,2,1,2,1,1,1,1,1,1,1,1,1,
	1,1,1,2,2,1,2,1,2,2,2,2,2,2,2,2,
	1,1,2,1,2,1,2,1,2,2,2,2,2,2,2,2
};


/****************************************************************************
 *  Inits CPU emulation
 ****************************************************************************/

static CPU_INIT( i8x41 )
{
	upi41_state_t *upi41_state = device->token;

	upi41_state->irq_callback = irqcallback;
	upi41_state->device = device;
	upi41_state->program = memory_find_address_space(device, ADDRESS_SPACE_PROGRAM);
	upi41_state->data = memory_find_address_space(device, ADDRESS_SPACE_DATA);
	upi41_state->io = memory_find_address_space(device, ADDRESS_SPACE_IO);
	upi41_state->subtype = 8041;
	upi41_state->ram_mask = I8X41_intRAM_MASK;

	state_save_register_item("i8x41", device->tag, 0, upi41_state->ppc);
	state_save_register_item("i8x41", device->tag, 0, upi41_state->pc);
	state_save_register_item("i8x41", device->tag, 0, upi41_state->timer);
	state_save_register_item("i8x41", device->tag, 0, upi41_state->prescaler);
	state_save_register_item("i8x41", device->tag, 0, upi41_state->subtype);
	state_save_register_item("i8x41", device->tag, 0, upi41_state->a);
	state_save_register_item("i8x41", device->tag, 0, upi41_state->psw);
	state_save_register_item("i8x41", device->tag, 0, upi41_state->state);
	state_save_register_item("i8x41", device->tag, 0, upi41_state->enable);
	state_save_register_item("i8x41", device->tag, 0, upi41_state->control);
	state_save_register_item("i8x41", device->tag, 0, upi41_state->dbbi);
	state_save_register_item("i8x41", device->tag, 0, upi41_state->dbbo);
	state_save_register_item("i8x41", device->tag, 0, upi41_state->p1);
	state_save_register_item("i8x41", device->tag, 0, upi41_state->p2);
	state_save_register_item("i8x41", device->tag, 0, upi41_state->p2_hs);
}

static CPU_INIT( i8042 )
{
	upi41_state_t *upi41_state = device->token;
	CPU_INIT_CALL(i8x41);
	upi41_state->subtype = 8042;
	upi41_state->ram_mask = I8X42_intRAM_MASK;
}



/****************************************************************************
 *  Reset registers to their initial values
 ****************************************************************************/

static CPU_RESET( i8x41 )
{
	upi41_state_t *upi41_state = device->token;

	upi41_state->ppc = 0;
	upi41_state->pc = 0;
	upi41_state->timer = 0;
	upi41_state->prescaler = 0;
	upi41_state->a = 0;
	upi41_state->psw = 0;
	upi41_state->state = 0;
	upi41_state->enable = 0;

	ENABLE = IBFI | TCNTI;
	DBBI = 0xff;
	DBBO = 0xff;
	/* Set Ports 1 and 2 to input mode */
	P1   = 0xff;
	P2   = 0xff;
	P2_HS= 0xff;
}


/****************************************************************************
 *  Shut down CPU emulation
 ****************************************************************************/

static CPU_EXIT( i8x41 )
{
	/* nothing to do */
}


/****************************************************************************
 *  Execute cycles - returns number of cycles actually run
 ****************************************************************************/

static CPU_EXECUTE( i8x41 )
{
	upi41_state_t *upi41_state = device->token;
	int inst_cycles, T1_level;

	upi41_state->icount  = cycles;

	do
	{
		UINT8 op = memory_decrypted_read_byte(upi41_state->program, PC);

		PPC = PC;

		debugger_instruction_hook(device, PC);

		PC += 1;
		upi41_state->icount  -= i8x41_cycles[op];

		execute_op(upi41_state, op);

		if( ENABLE & CNT )
		{
			inst_cycles = i8x41_cycles[op];
			for ( ; inst_cycles > 0; inst_cycles-- )
			{
				T1_level = RP(upi41_state, I8X41_t1);
				if( (CONTROL & TEST1) && (T1_level == 0) )	/* Negative Edge */
				{
					upi41_state->timer++;
					if (upi41_state->timer == 0)
					{
						CONTROL |= TOVF;
						if( ENABLE & TCNTI )
							CONTROL |= TIRQ_PEND;
					}
				}
				if( T1_level ) CONTROL |= TEST1;
				else CONTROL &= ~TEST1;
			}
		}

		if( ENABLE & T )
		{
			PRESCALER += i8x41_cycles[op];
			/**** timer is prescaled by 32 ****/
			if( PRESCALER >= 32 )
			{
				PRESCALER -= 32;
				upi41_state->timer++;
				if( upi41_state->timer == 0 )
				{
					CONTROL |= TOVF;
					if( ENABLE & TCNTI )
						CONTROL |= TIRQ_PEND;
				}
			}
		}

		if( CONTROL & IRQ_PEND )	/* Are any Interrupts Pending ? */
		{
			if( 0 == (CONTROL & IBFI_IGNR) )	/* Should we ignore IBFI interrupts ? */
			{
				if( (ENABLE & IBFI) && (CONTROL & IBFI_PEND) )
				{
					PUSH_PC_TO_STACK();
					PC = V_IBF;
					CONTROL &= ~IBFI_PEND;
					CONTROL |= IBFI_IGNR;
					upi41_state->icount  -= 2;
				}
			}
			if( 0 == (CONTROL & TIRQ_IGNR) )	/* Should we ignore Timer interrupts ? */
			{
				if( (ENABLE & TCNTI) && (CONTROL & TIRQ_PEND) )
				{
					PUSH_PC_TO_STACK();
					PC = V_TIMER;
					CONTROL &= ~TIRQ_PEND;
					CONTROL |= IRQ_IGNR;
					if( ENABLE & T ) PRESCALER += 2;	/* 2 states */
					upi41_state->icount  -= 2;		/* 2 states to take interrupt */
				}
			}
		}


	} while( upi41_state->icount  > 0 );

	return cycles - upi41_state->icount ;
}


/****************************************************************************
 *  Get all registers in given buffer
 ****************************************************************************/

static CPU_GET_CONTEXT( i8x41 )
{
}


/****************************************************************************
 *  Set all registers to given values
 ****************************************************************************/

static CPU_SET_CONTEXT( i8x41 )
{
}

/****************************************************************************
 *  Set IRQ line state
 ****************************************************************************/

static void set_irq_line(upi41_state_t *upi41_state, int irqline, int state)
{
	switch( irqline )
	{
	case I8X41_INT_IBF:
		if (state != CLEAR_LINE)
		{
			STATE |= IBF;
			if (ENABLE & IBFI)
			{
				CONTROL |= IBFI_PEND;
			}
		}
		else
		{
			STATE &= ~IBF;
		}
		break;

	case I8X41_INT_TEST1:
		if( state != CLEAR_LINE )
		{
			CONTROL |= TEST1;
		}
		else
		{
			/* high to low transition? */
			if( CONTROL & TEST1 )
			{
				/* counting enabled? */
				if( ENABLE & CNT )
				{
					upi41_state->timer++;
					if( upi41_state->timer == 0 )
					{
						CONTROL |= TOVF;
						CONTROL |= TIRQ_PEND;
					}
				}
			}
			CONTROL &= ~TEST1;
		}
		break;
	}
}


/**************************************************************************
 * Generic set_info
 **************************************************************************/

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

static ADDRESS_MAP_START(program_10bit, ADDRESS_SPACE_PROGRAM, 8)
	AM_RANGE(0x00, 0x3ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(program_11bit, ADDRESS_SPACE_PROGRAM, 8)
	AM_RANGE(0x00, 0x7ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(data_7bit, ADDRESS_SPACE_DATA, 8)
	AM_RANGE(0x00, 0x7f) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(data_8bit, ADDRESS_SPACE_DATA, 8)
	AM_RANGE(0x00, 0xff) AM_RAM
ADDRESS_MAP_END

static CPU_SET_INFO( i8x41 )
{
	upi41_state_t *upi41_state = device->token;

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + I8X41_INT_IBF:	set_irq_line(upi41_state, I8X41_INT_IBF, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + I8X41_INT_TEST1:	set_irq_line(upi41_state, I8X41_INT_TEST1, info->i);	break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + I8X41_PC:			PC = info->i & 0x7ff;					break;

		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + I8X41_SP:			PSW = (PSW & ~SP) | (info->i & SP);		break;

		case CPUINFO_INT_REGISTER + I8X41_PSW:			PSW = info->i;							break;
		case CPUINFO_INT_REGISTER + I8X41_A:			A = info->i;							break;
		case CPUINFO_INT_REGISTER + I8X41_T:			upi41_state->timer = info->i & 0x1fff;	break;
		case CPUINFO_INT_REGISTER + I8X41_R0:			SETR(upi41_state, 0, info->i);			break;
		case CPUINFO_INT_REGISTER + I8X41_R1:			SETR(upi41_state, 1, info->i);			break;
		case CPUINFO_INT_REGISTER + I8X41_R2:			SETR(upi41_state, 2, info->i);			break;
		case CPUINFO_INT_REGISTER + I8X41_R3:			SETR(upi41_state, 3, info->i);			break;
		case CPUINFO_INT_REGISTER + I8X41_R4:			SETR(upi41_state, 4, info->i);			break;
		case CPUINFO_INT_REGISTER + I8X41_R5:			SETR(upi41_state, 5, info->i);			break;
		case CPUINFO_INT_REGISTER + I8X41_R6:			SETR(upi41_state, 6, info->i);			break;
		case CPUINFO_INT_REGISTER + I8X41_R7:			SETR(upi41_state, 7, info->i);			break;

		case CPUINFO_INT_REGISTER + I8X41_DATA:
			DBBI = info->i;
			if( upi41_state->subtype == 8041 ) /* plain 8041 had no split input/output DBB buffers */
				DBBO = info->i;
			STATE &= ~F1;
			STATE |= IBF;
			if( ENABLE & IBFI )
				CONTROL |= IBFI_PEND;
			if( ENABLE & FLAGS)
			{
				P2_HS |= 0x20;
				if( 0 == (STATE & OBF) ) P2_HS |= 0x10;
				else P2_HS &= 0xef;
				WP(upi41_state, 0x02, (P2 & P2_HS) );	/* Assert the DBBI IRQ out on P25 */
			}
			break;

		case CPUINFO_INT_REGISTER + I8X41_DATA_DASM:
			/* Same as I8X41_DATA, except this is used by the */
			/* debugger and does not upset the flag states */
			DBBI = info->i;
			if( upi41_state->subtype == 8041 ) /* plain 8041 had no split input/output DBB buffers */
				DBBO = info->i;
			break;

		case CPUINFO_INT_REGISTER + I8X41_CMND:
			DBBI = info->i;
			if( upi41_state->subtype == 8041 ) /* plain 8041 had no split input/output DBB buffers */
				DBBO = info->i;
			STATE |= F1;
			STATE |= IBF;
			if( ENABLE & IBFI )
				CONTROL |= IBFI_PEND;
			if( ENABLE & FLAGS)
			{
				P2_HS |= 0x20;
				if( 0 == (STATE & OBF) ) P2_HS |= 0x10;
				else P2_HS &= 0xef;
				WP(upi41_state, 0x02, (P2 & P2_HS) );	/* Assert the DBBI IRQ out on P25 */
			}
			break;

		case CPUINFO_INT_REGISTER + I8X41_CMND_DASM:
			/* Same as I8X41_CMND, except this is used by the */
			/* debugger and does not upset the flag states */
			DBBI = info->i;
			if( upi41_state->subtype == 8041 ) /* plain 8041 had no split input/output DBB buffers */
				DBBO = info->i;
			break;

		case CPUINFO_INT_REGISTER + I8X41_STAT:
			logerror("i8x41 #%d:%03x  Setting STAT DBBI to %02x\n", cpunum_get_active(), PC, (UINT8)info->i);
			/* writing status.. hmm, should we issue interrupts here too? */
			STATE = info->i;
			break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

CPU_GET_INFO( i8041 )
{
	upi41_state_t *upi41_state = (device != NULL) ? device->token : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(upi41_state_t);		break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 2;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_LE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = I8X41_CLOCK_DIVIDER;			break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 2;							break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM:	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO:		info->i = 0;					break;

		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_PROGRAM:	info->internal_map8 = ADDRESS_MAP_NAME(program_10bit);	break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_DATA: 		info->internal_map8 = ADDRESS_MAP_NAME(data_7bit);		break;

		case CPUINFO_INT_INPUT_STATE + I8X41_INT_IBF:	info->i = (STATE & IBF) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + I8X41_INT_TEST1:	info->i = (STATE & TEST1) ? ASSERT_LINE : CLEAR_LINE; break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = PPC;							break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + I8X41_PC:			info->i = PC;							break;

		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + I8X41_SP:			info->i = PSW & SP;						break;

		case CPUINFO_INT_REGISTER + I8X41_PSW:			info->i = PSW;							break;
		case CPUINFO_INT_REGISTER + I8X41_A:			info->i = A;							break;
		case CPUINFO_INT_REGISTER + I8X41_T:			info->i = upi41_state->timer;			break;
		case CPUINFO_INT_REGISTER + I8X41_R0:			info->i = GETR(upi41_state, 0);			break;
		case CPUINFO_INT_REGISTER + I8X41_R1:			info->i = GETR(upi41_state, 1);			break;
		case CPUINFO_INT_REGISTER + I8X41_R2:			info->i = GETR(upi41_state, 2);			break;
		case CPUINFO_INT_REGISTER + I8X41_R3:			info->i = GETR(upi41_state, 3);			break;
		case CPUINFO_INT_REGISTER + I8X41_R4:			info->i = GETR(upi41_state, 4);			break;
		case CPUINFO_INT_REGISTER + I8X41_R5:			info->i = GETR(upi41_state, 5);			break;
		case CPUINFO_INT_REGISTER + I8X41_R6:			info->i = GETR(upi41_state, 6);			break;
		case CPUINFO_INT_REGISTER + I8X41_R7:			info->i = GETR(upi41_state, 7);			break;

		case CPUINFO_INT_REGISTER + I8X41_DATA:
			STATE &= ~OBF;	/* reset the output buffer full flag */
			if( ENABLE & FLAGS)
			{
				P2_HS &= 0xef;
				if( STATE & IBF ) P2_HS |= 0x20;
				else P2_HS &= 0xdf;
				WP(upi41_state, 0x02, (P2 & P2_HS) );	/* Clear the DBBO IRQ out on P24 */
			}
			info->i = DBBO;
			break;

		case CPUINFO_INT_REGISTER + I8X41_DATA_DASM:
			/* Same as I8X41_DATA, except this is used by the */
			/* debugger and does not upset the flag states */
			info->i = DBBO;
			break;

		case CPUINFO_INT_REGISTER + I8X41_STAT:
			logerror("i8x41 #%d:%03x  Reading STAT %02x\n", cpunum_get_active(), PC, STATE);
			info->i = STATE;
			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(i8x41);		break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = CPU_GET_CONTEXT_NAME(i8x41);	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = CPU_SET_CONTEXT_NAME(i8x41);	break;
		case CPUINFO_PTR_INIT:							info->init = CPU_INIT_NAME(i8x41);				break;
		case CPUINFO_PTR_RESET:							info->reset = CPU_RESET_NAME(i8x41);			break;
		case CPUINFO_PTR_EXIT:							info->exit = CPU_EXIT_NAME(i8x41);				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = CPU_EXECUTE_NAME(i8x41);		break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;								break;
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(i8x41);			break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &upi41_state->icount ;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "I8041");						break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "UPI-41/42");					break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "0.6");							break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);						break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright Juergen Buchmueller, all rights reserved."); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c%c%c%c%c",
				upi41_state->psw & 0x80 ? 'C':'.',
				upi41_state->psw & 0x40 ? 'A':'.',
				upi41_state->psw & 0x20 ? '0':'.',
				upi41_state->psw & 0x10 ? 'B':'.',
				upi41_state->psw & 0x08 ? '?':'.',
				upi41_state->psw & 0x04 ? 's':'.',
				upi41_state->psw & 0x02 ? 's':'.',
				upi41_state->psw & 0x01 ? 's':'.');
			break;

		case CPUINFO_STR_REGISTER + I8X41_PC:			sprintf(info->s, "PC:%04X", upi41_state->pc);	break;
		case CPUINFO_STR_REGISTER + I8X41_SP:			sprintf(info->s, "S:%X", upi41_state->psw & SP); break;
		case CPUINFO_STR_REGISTER + I8X41_PSW:			sprintf(info->s, "PSW:%02X", upi41_state->psw); break;
		case CPUINFO_STR_REGISTER + I8X41_A:			sprintf(info->s, "A:%02X", upi41_state->a);		break;
		case CPUINFO_STR_REGISTER + I8X41_T:			sprintf(info->s, "T:%02X.%02X", upi41_state->timer, (upi41_state->prescaler & 0x1f) ); break;
		case CPUINFO_STR_REGISTER + I8X41_R0:			sprintf(info->s, "R0:%02X", GETR(upi41_state, 0));break;
		case CPUINFO_STR_REGISTER + I8X41_R1:			sprintf(info->s, "R1:%02X", GETR(upi41_state, 1));break;
		case CPUINFO_STR_REGISTER + I8X41_R2:			sprintf(info->s, "R2:%02X", GETR(upi41_state, 2));break;
		case CPUINFO_STR_REGISTER + I8X41_R3:			sprintf(info->s, "R3:%02X", GETR(upi41_state, 3));break;
		case CPUINFO_STR_REGISTER + I8X41_R4:			sprintf(info->s, "R4:%02X", GETR(upi41_state, 4));break;
		case CPUINFO_STR_REGISTER + I8X41_R5:			sprintf(info->s, "R5:%02X", GETR(upi41_state, 5));break;
		case CPUINFO_STR_REGISTER + I8X41_R6:			sprintf(info->s, "R6:%02X", GETR(upi41_state, 6));break;
		case CPUINFO_STR_REGISTER + I8X41_R7:			sprintf(info->s, "R7:%02X", GETR(upi41_state, 7));break;
		case CPUINFO_STR_REGISTER + I8X41_P1:			sprintf(info->s, "P1:%02X", upi41_state->p1);	break;
		case CPUINFO_STR_REGISTER + I8X41_P2:			sprintf(info->s, "P2:%02X", upi41_state->p2);	break;
		case CPUINFO_STR_REGISTER + I8X41_DATA_DASM:	sprintf(info->s, "DBBI:%02X", upi41_state->dbbi); break;
		case CPUINFO_STR_REGISTER + I8X41_CMND_DASM:	sprintf(info->s, "DBBO:%02X", upi41_state->dbbo); break;
		case CPUINFO_STR_REGISTER + I8X41_STAT:			sprintf(info->s, "STAT:%02X", upi41_state->state);	break;
	}
}


#if (HAS_I8741)
CPU_GET_INFO( i8741 )
{
	switch (state)
	{
		case CPUINFO_STR_NAME:											strcpy(info->s, "I8741");							break;
		default:														CPU_GET_INFO_CALL(i8041);							break;
	}
}
#endif

#if (HAS_I8042)
CPU_GET_INFO( i8042 )
{
	switch (state)
	{
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_PROGRAM:	info->internal_map8 = ADDRESS_MAP_NAME(program_11bit);	break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_DATA: 		info->internal_map8 = ADDRESS_MAP_NAME(data_8bit);		break;
		case CPUINFO_PTR_INIT:											info->init = CPU_INIT_NAME(i8042);					break;
		case CPUINFO_STR_NAME:											strcpy(info->s, "I8042");							break;
		default:														CPU_GET_INFO_CALL(i8041);							break;
	}
}

#endif

#if (HAS_I8242)
CPU_GET_INFO( i8242 )
{
	switch (state)
	{
		case CPUINFO_STR_NAME:											strcpy(info->s, "I8242");							break;
		default:														CPU_GET_INFO_CALL(i8042);							break;
	}
}

#endif

#if (HAS_I8742)
CPU_GET_INFO( i8742 )
{
	switch (state)
	{
		case CPUINFO_STR_NAME:											strcpy(info->s, "I8742");							break;
		default:														CPU_GET_INFO_CALL(i8042);							break;
	}
}

#endif
