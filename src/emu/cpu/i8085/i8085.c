/*****************************************************************************
 *
 *   i8085.c
 *   Portable I8085A emulator V1.2
 *
 *   Copyright Juergen Buchmueller, all rights reserved.
 *   Partially based on information out of Z80Em by Marcel De Kogel
 *
 * changes in V1.3
 *   - Added undocumented opcodes for the 8085A, based on a german
 *     book about microcomputers: "Mikrocomputertechnik mit dem
 *     Prozessor 8085A".
 *   - This book also suggest that INX/DCX should modify the X flag bit
 *     for a LSB to MSB carry and
 *   - that jumps take 10 T-states only when they're executed, 7 when
 *     they're skipped.
 *     Thanks for the info and a copy of the tables go to Timo Sachsenberg
 *     <timo.sachsenberg@student.uni-tuebingen.de>
 * changes in V1.2
 *   - corrected cycle counts for these classes of opcodes
 *     Thanks go to Jim Battle <frustum@pacbell.bet>
 *
 *                  808x     Z80
 *     DEC A           5       4    \
 *     INC A           5       4     \
 *     LD A,B          5       4      >-- Z80 is faster
 *     JP (HL)         5       4     /
 *     CALL cc,nnnn: 11/17   10/17  /
 *
 *     INC HL          5       6    \
 *     DEC HL          5       6     \
 *     LD SP,HL        5       6      \
 *     ADD HL,BC      10      11       \
 *     INC (HL)       10      11        >-- 8080 is faster
 *     DEC (HL)       10      11       /
 *     IN A,(#)       10      11      /
 *     OUT (#),A      10      11     /
 *     EX (SP),HL     18      19    /
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
 * Revisions:
 *
 * xx-xx-2002 Acho A. Tang
 *
 * - 8085 emulation was in fact never used. It's been treated as a plain 8080.
 * - protected IRQ0 vector from being overwritten
 * - modified interrupt handler to properly process 8085-specific IRQ's
 * - corrected interrupt masking, RIM and SIM behaviors according to Intel's documentation
 *
 * 20-Jul-2002 Krzysztof Strzecha
 *
 * - SBB r instructions should affect parity flag.
 *   Fixed only for non x86 asm version (#define i8080_EXACT 1).
 *   There are probably more opcodes which should affect this flag, but don't.
 * - JPO nnnn and JPE nnnn opcodes in disassembler were misplaced. Fixed.
 * - Undocumented i8080 opcodes added:
 *   08h, 10h, 18h, 20h, 28h, 30h, 38h  -  NOP
 *   0CBh                               -  JMP
 *   0D9h                               -  RET
 *   0DDh, 0EDh, 0FDh                   -  CALL
 *   Thanks for the info go to Anton V. Ignatichev.
 *
 * 08-Dec-2002 Krzysztof Strzecha
 *
 * - ADC r instructions should affect parity flag.
 *   Fixed only for non x86 asm version (#define i8080_EXACT 1).
 *   There are probably more opcodes which should affect this flag, but don't.
 *
 * 05-Sep-2003 Krzysztof Strzecha
 *
 * - INR r, DCR r, ADD r, SUB r, CMP r instructions should affect parity flag.
 *   Fixed only for non x86 asm version (#define i8080_EXACT 1).
 *
 * 23-Dec-2006 Tomasz Slanina
 *
 * - SIM fixed
 *
 * 28-Jan-2007 Zsolt Vasvari
 *
 * - Removed archaic i8080_EXACT flag.
 *
 * 08-June-2008 Miodrag Milanovic
 *
 * - Flag setting fix for some instructions and cycle count update
 *
 *****************************************************************************/

/*int survival_prot = 0; */

#include "debugger.h"
#include "i8085.h"
#include "i8085cpu.h"
#include "i8085daa.h"

#define VERBOSE 0

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

#define I8085_INTR      0xff

typedef struct _i8085_state i8085_state;
struct _i8085_state
{
	int 	cputype;	/* 0 8080, 1 8085A */
	PAIR	PC,SP,AF,BC,DE,HL,XX;
	UINT8	HALT;
	UINT8	IM; 		/* interrupt mask */
	UINT8	IREQ;		/* requested interrupts */
	UINT8	ISRV;		/* serviced interrupt */
	UINT32	INTR;		/* vector for INTR */
	UINT32	IRQ2;		/* scheduled interrupt address */
	UINT32	IRQ1;		/* executed interrupt address */
	UINT8   STATUS;		/* status word */
	UINT8	after_ei;	/* are we in the EI shadow? */
	INT8	irq_state[4];
	cpu_irq_callback irq_callback;
	const device_config *device;
	const address_space *program;
	const address_space *io;
	i8085_sod_func sod_callback;
	i8085_sid_func sid_callback;
	i8085_inte_func inte_callback;
	i8085_status_func status_callback;
	int		icount;
	UINT8 	rim_ien;
};

static UINT8 ZS[256];
static UINT8 ZSP[256];

static UINT8 ROP(i8085_state *cpustate)
{
	cpustate->STATUS = 0xa2; // instruction fetch
	return memory_decrypted_read_byte(cpustate->program, cpustate->PC.w.l++);
}

static UINT8 ARG(i8085_state *cpustate)
{
	return memory_raw_read_byte(cpustate->program, cpustate->PC.w.l++);
}

static UINT16 ARG16(i8085_state *cpustate)
{
	UINT16 w;
	w  = memory_raw_read_byte(cpustate->program, cpustate->PC.d);
	cpustate->PC.w.l++;
	w += memory_raw_read_byte(cpustate->program, cpustate->PC.d) << 8;
	cpustate->PC.w.l++;
	return w;
}

static UINT8 RM(i8085_state *cpustate, UINT32 a)
{
	cpustate->STATUS = 0x82; // memory read
	return memory_read_byte_8le(cpustate->program, a);
}

static void WM(i8085_state *cpustate, UINT32 a, UINT8 v)
{
	cpustate->STATUS = 0x00; // memory write
	memory_write_byte_8le(cpustate->program, a, v);
}

INLINE void execute_one(i8085_state *cpustate, int opcode)
{
	/* output state word */
	if (cpustate->status_callback) (*cpustate->status_callback)(cpustate->device, cpustate->STATUS);

	switch (opcode)
	{
		case 0x00: cpustate->icount -= 4;	/* NOP  */
			/* no op */
			break;
		case 0x01: cpustate->icount -= 10;	/* LXI  B,nnnn */
			cpustate->BC.w.l = ARG16(cpustate);
			break;
		case 0x02: cpustate->icount -= 7;	/* STAX B */
			WM(cpustate, cpustate->BC.d, cpustate->AF.b.h);
			break;
		case 0x03: cpustate->icount -= (cpustate->cputype) ? 6 : 5;	/* INX  B */
			cpustate->BC.w.l++;
			if( cpustate->cputype )
			{
				if (cpustate->BC.b.l == 0x00) cpustate->AF.b.l |= XF; else cpustate->AF.b.l &= ~XF;
			}
			break;
		case 0x04: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* INR  B */
			M_INR(cpustate->BC.b.h);
			break;
		case 0x05: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* DCR  B */
			M_DCR(cpustate->BC.b.h);
			break;
		case 0x06: cpustate->icount -= 7;	/* MVI  B,nn */
			M_MVI(cpustate->BC.b.h);
			break;
		case 0x07: cpustate->icount -= 4;	/* RLC  */
			M_RLC;
			break;

		case 0x08:
			if( cpustate->cputype ) {
				cpustate->icount -= 10;		/* DSUB */
				M_DSUB(cpustate);
			} else {
				cpustate->icount -= 4;		/* NOP undocumented */
			}
			break;
		case 0x09: cpustate->icount -= 10;	/* DAD  B */
			M_DAD(BC);
			break;
		case 0x0a: cpustate->icount -= 7;	/* LDAX B */
			cpustate->AF.b.h = RM(cpustate, cpustate->BC.d);
			break;
		case 0x0b: cpustate->icount -= (cpustate->cputype) ? 6 : 5;	/* DCX  B */
			cpustate->BC.w.l--;
			if( cpustate->cputype )
			{
				if (cpustate->BC.b.l == 0xff) cpustate->AF.b.l |= XF; else cpustate->AF.b.l &= ~XF;
			}
			break;
		case 0x0c: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* INR  C */
			M_INR(cpustate->BC.b.l);
			break;
		case 0x0d: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* DCR  C */
			M_DCR(cpustate->BC.b.l);
			break;
		case 0x0e: cpustate->icount -= 7;	/* MVI  C,nn */
			M_MVI(cpustate->BC.b.l);
			break;
		case 0x0f: cpustate->icount -= 4;	/* RRC  */
			M_RRC;
			break;

		case 0x10:
			if( cpustate->cputype ) {
				cpustate->icount -= 7;		/* ASRH */
				cpustate->AF.b.l = (cpustate->AF.b.l & ~CF) | (cpustate->HL.b.l & CF);
				cpustate->HL.w.l = (cpustate->HL.w.l >> 1);
			} else {
				cpustate->icount -= 4;		/* NOP undocumented */
			}
			break;
		case 0x11: cpustate->icount -= 10;	/* LXI  D,nnnn */
			cpustate->DE.w.l = ARG16(cpustate);
			break;
		case 0x12: cpustate->icount -= 7;	/* STAX D */
			WM(cpustate, cpustate->DE.d, cpustate->AF.b.h);
			break;
		case 0x13: cpustate->icount -= (cpustate->cputype) ? 6 : 5;	/* INX  D */
			cpustate->DE.w.l++;
			if( cpustate->cputype )
			{
				if (cpustate->DE.b.l == 0x00) cpustate->AF.b.l |= XF; else cpustate->AF.b.l &= ~XF;
			}
			break;
		case 0x14: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* INR  D */
			M_INR(cpustate->DE.b.h);
			break;
		case 0x15: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* DCR  D */
			M_DCR(cpustate->DE.b.h);
			break;
		case 0x16: cpustate->icount -= 7;	/* MVI  D,nn */
			M_MVI(cpustate->DE.b.h);
			break;
		case 0x17: cpustate->icount -= 4;	/* RAL  */
			M_RAL;
			break;

		case 0x18:
			if( cpustate->cputype ) {
				cpustate->icount -= 10;		/* RLDE */
				cpustate->AF.b.l = (cpustate->AF.b.l & ~(CF | VF)) | (cpustate->DE.b.h >> 7);
				cpustate->DE.w.l = (cpustate->DE.w.l << 1) | (cpustate->DE.w.l >> 15);
				if (0 != (((cpustate->DE.w.l >> 15) ^ cpustate->AF.b.l) & CF))
					cpustate->AF.b.l |= VF;
			} else {
				cpustate->icount -= 4;		/* NOP undocumented */
			}
			break;
		case 0x19: cpustate->icount -= 10;	/* DAD  D */
			M_DAD(DE);
			break;
		case 0x1a: cpustate->icount -= 7;	/* LDAX D */
			cpustate->AF.b.h = RM(cpustate, cpustate->DE.d);
			break;
		case 0x1b: cpustate->icount -= (cpustate->cputype) ? 6 : 5;	/* DCX  D */
			cpustate->DE.w.l--;
			if( cpustate->cputype )
			{
				if (cpustate->DE.b.l == 0xff) cpustate->AF.b.l |= XF; else cpustate->AF.b.l &= ~XF;
			}
			break;
		case 0x1c: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* INR  E */
			M_INR(cpustate->DE.b.l);
			break;
		case 0x1d: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* DCR  E */
			M_DCR(cpustate->DE.b.l);
			break;
		case 0x1e: cpustate->icount -= 7;	/* MVI  E,nn */
			M_MVI(cpustate->DE.b.l);
			break;
		case 0x1f: cpustate->icount -= 4;	/* RAR  */
			M_RAR;
			break;

		case 0x20:
			if( cpustate->cputype ) {
				cpustate->icount -= 7;		/* RIM  */
				cpustate->AF.b.h = cpustate->IM;
				if (cpustate->sid_callback)
					cpustate->AF.b.h = (cpustate->AF.b.h & 0x7f) | ((*cpustate->sid_callback)(cpustate->device) ? 0x80 : 0);
				cpustate->AF.b.h |= cpustate->rim_ien; cpustate->rim_ien = 0; //AT: read and clear IEN status latch
			} else {
				cpustate->icount -= 4;		/* NOP undocumented */
			}
			break;
		case 0x21: cpustate->icount -= 10;	/* LXI  H,nnnn */
			cpustate->HL.w.l = ARG16(cpustate);
			break;
		case 0x22: cpustate->icount -= 16;	/* SHLD nnnn */
			cpustate->XX.w.l = ARG16(cpustate);
			WM(cpustate, cpustate->XX.d, cpustate->HL.b.l);
			cpustate->XX.w.l++;
			WM(cpustate, cpustate->XX.d, cpustate->HL.b.h);
			break;
		case 0x23: cpustate->icount -= (cpustate->cputype) ? 6 : 5;	/* INX  H */
			cpustate->HL.w.l++;
			if( cpustate->cputype )
			{
				if (cpustate->HL.b.l == 0x00) cpustate->AF.b.l |= XF; else cpustate->AF.b.l &= ~XF;
			}
			break;
		case 0x24: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* INR  H */
			M_INR(cpustate->HL.b.h);
			break;
		case 0x25: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* DCR  H */
			M_DCR(cpustate->HL.b.h);
			break;
		case 0x26: cpustate->icount -= 7;	/* MVI  H,nn */
			M_MVI(cpustate->HL.b.h);
			break;
		case 0x27: cpustate->icount -= 4;	/* DAA  */
			cpustate->XX.d = cpustate->AF.b.h;
			if (cpustate->AF.b.l & CF) cpustate->XX.d |= 0x100;
			if (cpustate->AF.b.l & HF) cpustate->XX.d |= 0x200;
			if (cpustate->AF.b.l & NF) cpustate->XX.d |= 0x400;
			cpustate->AF.w.l = DAA[cpustate->XX.d];
			if( cpustate->cputype==0 )
			{
				cpustate->AF.b.l &= 0xd5; // Ignore not used flags
			}
			break;

		case 0x28:
			if( cpustate->cputype ) {
				cpustate->icount -= 10;		/* LDEH nn */
				cpustate->XX.d = ARG(cpustate);
				cpustate->DE.d = (cpustate->HL.d + cpustate->XX.d) & 0xffff;
			} else {
				cpustate->icount -= 4;		/* NOP undocumented */
			}
			break;
		case 0x29: cpustate->icount -= 10;	/* DAD  H */
			M_DAD(HL);
			break;
		case 0x2a: cpustate->icount -= 16;	/* LHLD nnnn */
			cpustate->XX.d = ARG16(cpustate);
			cpustate->HL.b.l = RM(cpustate, cpustate->XX.d);
			cpustate->XX.w.l++;
			cpustate->HL.b.h = RM(cpustate, cpustate->XX.d);
			break;
		case 0x2b: cpustate->icount -= (cpustate->cputype) ? 6 : 5;	/* DCX  H */
			cpustate->HL.w.l--;
			if( cpustate->cputype )
			{
				if (cpustate->HL.b.l == 0xff) cpustate->AF.b.l |= XF; else cpustate->AF.b.l &= ~XF;
			}
			break;
		case 0x2c: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* INR  L */
			M_INR(cpustate->HL.b.l);
			break;
		case 0x2d: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* DCR  L */
			M_DCR(cpustate->HL.b.l);
			break;
		case 0x2e: cpustate->icount -= 7;	/* MVI  L,nn */
			M_MVI(cpustate->HL.b.l);
			break;
		case 0x2f: cpustate->icount -= 4;	/* CMA  */
			if( cpustate->cputype )
			{
				cpustate->AF.b.h ^= 0xff;
				cpustate->AF.b.l |= HF + NF;
			}
			else
			{
				cpustate->AF.b.h ^= 0xff;	/* 8080 */
			}
			break;

		case 0x30:
			if( cpustate->cputype )
			{
				cpustate->icount -= 7;		/* SIM  */

				if (cpustate->AF.b.h & 0x40) //SOE - only when bit 0x40 is set!
				{
					cpustate->IM &=~IM_SOD;
					if (cpustate->AF.b.h & 0x80) cpustate->IM |= IM_SOD; //is it needed ?
					if (cpustate->sod_callback) (*cpustate->sod_callback)(cpustate->device, cpustate->AF.b.h >> 7); //SOD - data = bit 0x80
				}
//AT
				//cpustate->IM &= (IM_SID + IM_IEN + IM_TRAP);
				//cpustate->IM |= (cpustate->AF.b.h & ~(IM_SID + IM_SOD + IM_IEN + IM_TRAP));

				// overwrite RST5.5-7.5 interrupt masks only when bit 0x08 of the accumulator is set
				if (cpustate->AF.b.h & 0x08)
					cpustate->IM = (cpustate->IM & ~(IM_RST55+IM_RST65+IM_RST75)) | (cpustate->AF.b.h & (IM_RST55+IM_RST65+IM_RST75));

			} else {
				cpustate->icount -= 4;		/* NOP undocumented */
			}
			break;

		case 0x31: cpustate->icount -= 10;	/* LXI SP,nnnn */
			cpustate->SP.w.l = ARG16(cpustate);
			break;
		case 0x32: cpustate->icount -= 13;	/* STAX nnnn */
			cpustate->XX.d = ARG16(cpustate);
			WM(cpustate, cpustate->XX.d, cpustate->AF.b.h);
			break;
		case 0x33: cpustate->icount -= (cpustate->cputype) ? 6 : 5;	/* INX  SP */
			cpustate->SP.w.l++;
			if( cpustate->cputype )
			{
				if (cpustate->SP.b.l == 0x00) cpustate->AF.b.l |= XF; else cpustate->AF.b.l &= ~XF;
			}
			break;
		case 0x34: cpustate->icount -= 10;	/* INR  M */
			cpustate->XX.b.l = RM(cpustate, cpustate->HL.d);
			M_INR(cpustate->XX.b.l);
			WM(cpustate, cpustate->HL.d, cpustate->XX.b.l);
			break;
		case 0x35: cpustate->icount -= 10;	/* DCR  M */
			cpustate->XX.b.l = RM(cpustate, cpustate->HL.d);
			M_DCR(cpustate->XX.b.l);
			WM(cpustate, cpustate->HL.d, cpustate->XX.b.l);
			break;
		case 0x36: cpustate->icount -= 10;	/* MVI  M,nn */
			cpustate->XX.b.l = ARG(cpustate);
			WM(cpustate, cpustate->HL.d, cpustate->XX.b.l);
			break;
		case 0x37: cpustate->icount -= 4;	/* STC  */
			cpustate->AF.b.l = (cpustate->AF.b.l & 0xfe) | CF;
			break;

		case 0x38:
			if( cpustate->cputype ) {
				cpustate->icount -= 10;		/* LDES nn */
				cpustate->XX.d = ARG(cpustate);
				cpustate->DE.d = (cpustate->SP.d + cpustate->XX.d) & 0xffff;
			} else {
				cpustate->icount -= 4;		/* NOP undocumented */
			}
			break;
		case 0x39: cpustate->icount -= 10;	/* DAD SP */
			M_DAD(SP);
			break;
		case 0x3a: cpustate->icount -= 13;	/* LDAX nnnn */
			cpustate->XX.d = ARG16(cpustate);
			cpustate->AF.b.h = RM(cpustate, cpustate->XX.d);
			break;
		case 0x3b: cpustate->icount -= (cpustate->cputype) ? 6 : 5;	/* DCX  SP */
			cpustate->SP.w.l--;
			if( cpustate->cputype )
			{
				if (cpustate->SP.b.l == 0xff) cpustate->AF.b.l |= XF; else cpustate->AF.b.l &= ~XF;
			}
			break;
		case 0x3c: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* INR  A */
			M_INR(cpustate->AF.b.h);
			break;
		case 0x3d: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* DCR  A */
			M_DCR(cpustate->AF.b.h);
			break;
		case 0x3e: cpustate->icount -= 7;	/* MVI  A,nn */
			M_MVI(cpustate->AF.b.h);
			break;
		case 0x3f: cpustate->icount -= 4;	/* CMC  */
			cpustate->AF.b.l = (cpustate->AF.b.l & 0xfe) | ((cpustate->AF.b.l & CF)==1 ? 0 : 1);
			break;

		case 0x40: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  B,B */
			/* no op */
			break;
		case 0x41: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  B,C */
			cpustate->BC.b.h = cpustate->BC.b.l;
			break;
		case 0x42: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  B,D */
			cpustate->BC.b.h = cpustate->DE.b.h;
			break;
		case 0x43: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  B,E */
			cpustate->BC.b.h = cpustate->DE.b.l;
			break;
		case 0x44: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  B,H */
			cpustate->BC.b.h = cpustate->HL.b.h;
			break;
		case 0x45: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  B,L */
			cpustate->BC.b.h = cpustate->HL.b.l;
			break;
		case 0x46: cpustate->icount -= 7;	/* MOV  B,M */
			cpustate->BC.b.h = RM(cpustate, cpustate->HL.d);
			break;
		case 0x47: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  B,A */
			cpustate->BC.b.h = cpustate->AF.b.h;
			break;

		case 0x48: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  C,B */
			cpustate->BC.b.l = cpustate->BC.b.h;
			break;
		case 0x49: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  C,C */
			/* no op */
			break;
		case 0x4a: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  C,D */
			cpustate->BC.b.l = cpustate->DE.b.h;
			break;
		case 0x4b: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  C,E */
			cpustate->BC.b.l = cpustate->DE.b.l;
			break;
		case 0x4c: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  C,H */
			cpustate->BC.b.l = cpustate->HL.b.h;
			break;
		case 0x4d: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  C,L */
			cpustate->BC.b.l = cpustate->HL.b.l;
			break;
		case 0x4e: cpustate->icount -= 7;	/* MOV  C,M */
			cpustate->BC.b.l = RM(cpustate, cpustate->HL.d);
			break;
		case 0x4f: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  C,A */
			cpustate->BC.b.l = cpustate->AF.b.h;
			break;

		case 0x50: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  D,B */
			cpustate->DE.b.h = cpustate->BC.b.h;
			break;
		case 0x51: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  D,C */
			cpustate->DE.b.h = cpustate->BC.b.l;
			break;
		case 0x52: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  D,D */
			/* no op */
			break;
		case 0x53: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  D,E */
			cpustate->DE.b.h = cpustate->DE.b.l;
			break;
		case 0x54: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  D,H */
			cpustate->DE.b.h = cpustate->HL.b.h;
			break;
		case 0x55: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  D,L */
			cpustate->DE.b.h = cpustate->HL.b.l;
			break;
		case 0x56: cpustate->icount -= 7;	/* MOV  D,M */
			cpustate->DE.b.h = RM(cpustate, cpustate->HL.d);
			break;
		case 0x57: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  D,A */
			cpustate->DE.b.h = cpustate->AF.b.h;
			break;

		case 0x58: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  E,B */
			cpustate->DE.b.l = cpustate->BC.b.h;
			break;
		case 0x59: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  E,C */
			cpustate->DE.b.l = cpustate->BC.b.l;
			break;
		case 0x5a: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  E,D */
			cpustate->DE.b.l = cpustate->DE.b.h;
			break;
		case 0x5b: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  E,E */
			/* no op */
			break;
		case 0x5c: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  E,H */
			cpustate->DE.b.l = cpustate->HL.b.h;
			break;
		case 0x5d: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  E,L */
			cpustate->DE.b.l = cpustate->HL.b.l;
			break;
		case 0x5e: cpustate->icount -= 7;	/* MOV  E,M */
			cpustate->DE.b.l = RM(cpustate, cpustate->HL.d);
			break;
		case 0x5f: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  E,A */
			cpustate->DE.b.l = cpustate->AF.b.h;
			break;

		case 0x60: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  H,B */
			cpustate->HL.b.h = cpustate->BC.b.h;
			break;
		case 0x61: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  H,C */
			cpustate->HL.b.h = cpustate->BC.b.l;
			break;
		case 0x62: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  H,D */
			cpustate->HL.b.h = cpustate->DE.b.h;
			break;
		case 0x63: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  H,E */
			cpustate->HL.b.h = cpustate->DE.b.l;
			break;
		case 0x64: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  H,H */
			/* no op */
			break;
		case 0x65: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  H,L */
			cpustate->HL.b.h = cpustate->HL.b.l;
			break;
		case 0x66: cpustate->icount -= 7;	/* MOV  H,M */
			cpustate->HL.b.h = RM(cpustate, cpustate->HL.d);
			break;
		case 0x67: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  H,A */
			cpustate->HL.b.h = cpustate->AF.b.h;
			break;

		case 0x68: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  L,B */
			cpustate->HL.b.l = cpustate->BC.b.h;
			break;
		case 0x69: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  L,C */
			cpustate->HL.b.l = cpustate->BC.b.l;
			break;
		case 0x6a: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  L,D */
			cpustate->HL.b.l = cpustate->DE.b.h;
			break;
		case 0x6b: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  L,E */
			cpustate->HL.b.l = cpustate->DE.b.l;
			break;
		case 0x6c: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  L,H */
			cpustate->HL.b.l = cpustate->HL.b.h;
			break;
		case 0x6d: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  L,L */
			/* no op */
			break;
		case 0x6e: cpustate->icount -= 7;	/* MOV  L,M */
			cpustate->HL.b.l = RM(cpustate, cpustate->HL.d);
			break;
		case 0x6f: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  L,A */
			cpustate->HL.b.l = cpustate->AF.b.h;
			break;

		case 0x70: cpustate->icount -= 7;	/* MOV  M,B */
			WM(cpustate, cpustate->HL.d, cpustate->BC.b.h);
			break;
		case 0x71: cpustate->icount -= 7;	/* MOV  M,C */
			WM(cpustate, cpustate->HL.d, cpustate->BC.b.l);
			break;
		case 0x72: cpustate->icount -= 7;	/* MOV  M,D */
			WM(cpustate, cpustate->HL.d, cpustate->DE.b.h);
			break;
		case 0x73: cpustate->icount -= 7;	/* MOV  M,E */
			WM(cpustate, cpustate->HL.d, cpustate->DE.b.l);
			break;
		case 0x74: cpustate->icount -= 7;	/* MOV  M,H */
			WM(cpustate, cpustate->HL.d, cpustate->HL.b.h);
			break;
		case 0x75: cpustate->icount -= 7;	/* MOV  M,L */
			WM(cpustate, cpustate->HL.d, cpustate->HL.b.l);
			break;
		case 0x76: cpustate->icount -= (cpustate->cputype) ? 5 : 7;	/* HLT */
			cpustate->PC.w.l--;
			cpustate->HALT = 1;
			cpustate->STATUS = 0x8a; // halt acknowledge
			if (cpustate->icount > 0) cpustate->icount = 0;
			break;
		case 0x77: cpustate->icount -= 7;	/* MOV  M,A */
			WM(cpustate, cpustate->HL.d, cpustate->AF.b.h);
			break;

		case 0x78: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  A,B */
			cpustate->AF.b.h = cpustate->BC.b.h;
			break;
		case 0x79: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  A,C */
			cpustate->AF.b.h = cpustate->BC.b.l;
			break;
		case 0x7a: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  A,D */
			cpustate->AF.b.h = cpustate->DE.b.h;
			break;
		case 0x7b: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  A,E */
			cpustate->AF.b.h = cpustate->DE.b.l;
			break;
		case 0x7c: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  A,H */
			cpustate->AF.b.h = cpustate->HL.b.h;
			break;
		case 0x7d: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  A,L */
			cpustate->AF.b.h = cpustate->HL.b.l;
			break;
		case 0x7e: cpustate->icount -= 7;	/* MOV  A,M */
			cpustate->AF.b.h = RM(cpustate, cpustate->HL.d);
			break;
		case 0x7f: cpustate->icount -= (cpustate->cputype) ? 4 : 5;	/* MOV  A,A */
			/* no op */
			break;

		case 0x80: cpustate->icount -= 4;	/* ADD  B */
			M_ADD(cpustate->BC.b.h);
			break;
		case 0x81: cpustate->icount -= 4;	/* ADD  C */
			M_ADD(cpustate->BC.b.l);
			break;
		case 0x82: cpustate->icount -= 4;	/* ADD  D */
			M_ADD(cpustate->DE.b.h);
			break;
		case 0x83: cpustate->icount -= 4;	/* ADD  E */
			M_ADD(cpustate->DE.b.l);
			break;
		case 0x84: cpustate->icount -= 4;	/* ADD  H */
			M_ADD(cpustate->HL.b.h);
			break;
		case 0x85: cpustate->icount -= 4;	/* ADD  L */
			M_ADD(cpustate->HL.b.l);
			break;
		case 0x86: cpustate->icount -= 7;	/* ADD  M */
			M_ADD(RM(cpustate, cpustate->HL.d));
			break;
		case 0x87: cpustate->icount -= 4;	/* ADD  A */
			M_ADD(cpustate->AF.b.h);
			break;

		case 0x88: cpustate->icount -= 4;	/* ADC  B */
			M_ADC(cpustate->BC.b.h);
			break;
		case 0x89: cpustate->icount -= 4;	/* ADC  C */
			M_ADC(cpustate->BC.b.l);
			break;
		case 0x8a: cpustate->icount -= 4;	/* ADC  D */
			M_ADC(cpustate->DE.b.h);
			break;
		case 0x8b: cpustate->icount -= 4;	/* ADC  E */
			M_ADC(cpustate->DE.b.l);
			break;
		case 0x8c: cpustate->icount -= 4;	/* ADC  H */
			M_ADC(cpustate->HL.b.h);
			break;
		case 0x8d: cpustate->icount -= 4;	/* ADC  L */
			M_ADC(cpustate->HL.b.l);
			break;
		case 0x8e: cpustate->icount -= 7;	/* ADC  M */
			M_ADC(RM(cpustate, cpustate->HL.d));
			break;
		case 0x8f: cpustate->icount -= 4;	/* ADC  A */
			M_ADC(cpustate->AF.b.h);
			break;

		case 0x90: cpustate->icount -= 4;	/* SUB  B */
			M_SUB(cpustate->BC.b.h);
			break;
		case 0x91: cpustate->icount -= 4;	/* SUB  C */
			M_SUB(cpustate->BC.b.l);
			break;
		case 0x92: cpustate->icount -= 4;	/* SUB  D */
			M_SUB(cpustate->DE.b.h);
			break;
		case 0x93: cpustate->icount -= 4;	/* SUB  E */
			M_SUB(cpustate->DE.b.l);
			break;
		case 0x94: cpustate->icount -= 4;	/* SUB  H */
			M_SUB(cpustate->HL.b.h);
			break;
		case 0x95: cpustate->icount -= 4;	/* SUB  L */
			M_SUB(cpustate->HL.b.l);
			break;
		case 0x96: cpustate->icount -= 7;	/* SUB  M */
			M_SUB(RM(cpustate, cpustate->HL.d));
			break;
		case 0x97: cpustate->icount -= 4;	/* SUB  A */
			M_SUB(cpustate->AF.b.h);
			break;

		case 0x98: cpustate->icount -= 4;	/* SBB  B */
			M_SBB(cpustate->BC.b.h);
			break;
		case 0x99: cpustate->icount -= 4;	/* SBB  C */
			M_SBB(cpustate->BC.b.l);
			break;
		case 0x9a: cpustate->icount -= 4;	/* SBB  D */
			M_SBB(cpustate->DE.b.h);
			break;
		case 0x9b: cpustate->icount -= 4;	/* SBB  E */
			M_SBB(cpustate->DE.b.l);
			break;
		case 0x9c: cpustate->icount -= 4;	/* SBB  H */
			M_SBB(cpustate->HL.b.h);
			break;
		case 0x9d: cpustate->icount -= 4;	/* SBB  L */
			M_SBB(cpustate->HL.b.l);
			break;
		case 0x9e: cpustate->icount -= 7;	/* SBB  M */
			M_SBB(RM(cpustate, cpustate->HL.d));
			break;
		case 0x9f: cpustate->icount -= 4;	/* SBB  A */
			M_SBB(cpustate->AF.b.h);
			break;

		case 0xa0: cpustate->icount -= 4;	/* ANA  B */
			M_ANA(cpustate->BC.b.h);
			break;
		case 0xa1: cpustate->icount -= 4;	/* ANA  C */
			M_ANA(cpustate->BC.b.l);
			break;
		case 0xa2: cpustate->icount -= 4;	/* ANA  D */
			M_ANA(cpustate->DE.b.h);
			break;
		case 0xa3: cpustate->icount -= 4;	/* ANA  E */
			M_ANA(cpustate->DE.b.l);
			break;
		case 0xa4: cpustate->icount -= 4;	/* ANA  H */
			M_ANA(cpustate->HL.b.h);
			break;
		case 0xa5: cpustate->icount -= 4;	/* ANA  L */
			M_ANA(cpustate->HL.b.l);
			break;
		case 0xa6: cpustate->icount -= 7;	/* ANA  M */
			M_ANA(RM(cpustate, cpustate->HL.d));
			break;
		case 0xa7: cpustate->icount -= 4;	/* ANA  A */
			M_ANA(cpustate->AF.b.h);
			break;

		case 0xa8: cpustate->icount -= 4;	/* XRA  B */
			M_XRA(cpustate->BC.b.h);
			break;
		case 0xa9: cpustate->icount -= 4;	/* XRA  C */
			M_XRA(cpustate->BC.b.l);
			break;
		case 0xaa: cpustate->icount -= 4;	/* XRA  D */
			M_XRA(cpustate->DE.b.h);
			break;
		case 0xab: cpustate->icount -= 4;	/* XRA  E */
			M_XRA(cpustate->DE.b.l);
			break;
		case 0xac: cpustate->icount -= 4;	/* XRA  H */
			M_XRA(cpustate->HL.b.h);
			break;
		case 0xad: cpustate->icount -= 4;	/* XRA  L */
			M_XRA(cpustate->HL.b.l);
			break;
		case 0xae: cpustate->icount -= 7;	/* XRA  M */
			M_XRA(RM(cpustate, cpustate->HL.d));
			break;
		case 0xaf: cpustate->icount -= 4;	/* XRA  A */
			M_XRA(cpustate->AF.b.h);
			break;

		case 0xb0: cpustate->icount -= 4;	/* ORA  B */
			M_ORA(cpustate->BC.b.h);
			break;
		case 0xb1: cpustate->icount -= 4;	/* ORA  C */
			M_ORA(cpustate->BC.b.l);
			break;
		case 0xb2: cpustate->icount -= 4;	/* ORA  D */
			M_ORA(cpustate->DE.b.h);
			break;
		case 0xb3: cpustate->icount -= 4;	/* ORA  E */
			M_ORA(cpustate->DE.b.l);
			break;
		case 0xb4: cpustate->icount -= 4;	/* ORA  H */
			M_ORA(cpustate->HL.b.h);
			break;
		case 0xb5: cpustate->icount -= 4;	/* ORA  L */
			M_ORA(cpustate->HL.b.l);
			break;
		case 0xb6: cpustate->icount -= 7;	/* ORA  M */
			M_ORA(RM(cpustate, cpustate->HL.d));
			break;
		case 0xb7: cpustate->icount -= 4;	/* ORA  A */
			M_ORA(cpustate->AF.b.h);
			break;

		case 0xb8: cpustate->icount -= 4;	/* CMP  B */
			M_CMP(cpustate->BC.b.h);
			break;
		case 0xb9: cpustate->icount -= 4;	/* CMP  C */
			M_CMP(cpustate->BC.b.l);
			break;
		case 0xba: cpustate->icount -= 4;	/* CMP  D */
			M_CMP(cpustate->DE.b.h);
			break;
		case 0xbb: cpustate->icount -= 4;	/* CMP  E */
			M_CMP(cpustate->DE.b.l);
			break;
		case 0xbc: cpustate->icount -= 4;	/* CMP  H */
			M_CMP(cpustate->HL.b.h);
			break;
		case 0xbd: cpustate->icount -= 4;	/* CMP  L */
			M_CMP(cpustate->HL.b.l);
			break;
		case 0xbe: cpustate->icount -= 7;	/* CMP  M */
			M_CMP(RM(cpustate, cpustate->HL.d));
			break;
		case 0xbf: cpustate->icount -= 4;	/* CMP  A */
			M_CMP(cpustate->AF.b.h);
			break;

		case 0xc0: cpustate->icount -= (cpustate->cputype) ? 6 : 5;	/* RNZ  */
			M_RET( !(cpustate->AF.b.l & ZF) );
			break;
		case 0xc1: cpustate->icount -= 10;	/* POP  B */
			M_POP(BC);
			break;
		case 0xc2: cpustate->icount -= 10;	/* JNZ  nnnn */
			M_JMP( !(cpustate->AF.b.l & ZF) );
			break;
		case 0xc3: cpustate->icount -= 10;	/* JMP  nnnn */
			M_JMP(1);
			break;
		case 0xc4: cpustate->icount -= 11;	/* CNZ  nnnn */
			M_CALL( !(cpustate->AF.b.l & ZF) );
			break;
		case 0xc5: cpustate->icount -= (cpustate->cputype) ? 12 : 11;	/* PUSH B */
			M_PUSH(BC);
			break;
		case 0xc6: cpustate->icount -= 7;	/* ADI  nn */
			cpustate->XX.b.l = ARG(cpustate);
			M_ADD(cpustate->XX.b.l);
				break;
		case 0xc7: cpustate->icount -= (cpustate->cputype) ? 12 : 11;	/* RST  0 */
			M_RST(0);
			break;

		case 0xc8: cpustate->icount -= (cpustate->cputype) ? 6 : 5;	/* RZ   */
			M_RET( cpustate->AF.b.l & ZF );
			break;
		case 0xc9: cpustate->icount -= 10;	/* RET  */
			M_RET(1);
			break;
		case 0xca: cpustate->icount -= 10;	/* JZ   nnnn */
			M_JMP( cpustate->AF.b.l & ZF );
			break;
		case 0xcb:
			if( cpustate->cputype ) {
				if (cpustate->AF.b.l & VF) {
					cpustate->icount -= 12;
					M_RST(8);			/* call 0x40 */
				} else {
					cpustate->icount -= 6;	/* RST  V */
				}
			} else {
				cpustate->icount -= 10;	/* JMP  nnnn undocumented*/
				M_JMP(1);
			}
			break;
		case 0xcc: cpustate->icount -= 11;	/* CZ   nnnn */
			M_CALL( cpustate->AF.b.l & ZF );
			break;
		case 0xcd: cpustate->icount -= 17;	/* CALL nnnn */
			M_CALL(1);
			break;
		case 0xce: cpustate->icount -= 7;	/* ACI  nn */
			cpustate->XX.b.l = ARG(cpustate);
			M_ADC(cpustate->XX.b.l);
			break;
		case 0xcf: cpustate->icount -= (cpustate->cputype) ? 12 : 11;	/* RST  1 */
			M_RST(1);
			break;

		case 0xd0: cpustate->icount -= (cpustate->cputype) ? 6 : 5;	/* RNC  */
			M_RET( !(cpustate->AF.b.l & CF) );
			break;
		case 0xd1: cpustate->icount -= 10;	/* POP  D */
			M_POP(DE);
			break;
		case 0xd2: cpustate->icount -= 10;	/* JNC  nnnn */
			M_JMP( !(cpustate->AF.b.l & CF) );
			break;
		case 0xd3: cpustate->icount -= 10;	/* OUT  nn */
			M_OUT;
			break;
		case 0xd4: cpustate->icount -= 11;	/* CNC  nnnn */
			M_CALL( !(cpustate->AF.b.l & CF) );
			break;
		case 0xd5: cpustate->icount -= (cpustate->cputype) ? 12 : 11;	/* PUSH D */
			M_PUSH(DE);
			break;
		case 0xd6: cpustate->icount -= 7;	/* SUI  nn */
			cpustate->XX.b.l = ARG(cpustate);
			M_SUB(cpustate->XX.b.l);
			break;
		case 0xd7: cpustate->icount -= (cpustate->cputype) ? 12 : 11;	/* RST  2 */
			M_RST(2);
			break;

		case 0xd8: cpustate->icount -= (cpustate->cputype) ? 6 : 5;	/* RC   */
			M_RET( cpustate->AF.b.l & CF );
			break;
		case 0xd9:
			if( cpustate->cputype ) {
				cpustate->icount -= 10;		/* SHLX */
				cpustate->XX.w.l = cpustate->DE.w.l;
				WM(cpustate, cpustate->XX.d, cpustate->HL.b.l);
				cpustate->XX.w.l++;
				WM(cpustate, cpustate->XX.d, cpustate->HL.b.h);
			} else {
				cpustate->icount -= 10;	/* RET undocumented */
				M_RET(1);
			}
			break;
		case 0xda: cpustate->icount -= 10;	/* JC   nnnn */
			M_JMP( cpustate->AF.b.l & CF );
			break;
		case 0xdb: cpustate->icount -= 10;	/* IN   nn */
			M_IN;
			break;
		case 0xdc: cpustate->icount -= 11;	/* CC   nnnn */
			M_CALL( cpustate->AF.b.l & CF );
			break;
		case 0xdd:
			if( cpustate->cputype ) {
				cpustate->icount -= 7;		/* JNX  nnnn */
				M_JMP( !(cpustate->AF.b.l & XF) );
			} else {
				cpustate->icount -= 17;	/* CALL nnnn undocumented */
				M_CALL(1);
			}
			break;
		case 0xde: cpustate->icount -= 7;	/* SBI  nn */
			cpustate->XX.b.l = ARG(cpustate);
			M_SBB(cpustate->XX.b.l);
			break;
		case 0xdf: cpustate->icount -= (cpustate->cputype) ? 12 : 11;	/* RST  3 */
			M_RST(3);
			break;

		case 0xe0: cpustate->icount -= (cpustate->cputype) ? 6 : 5;	/* RPO    */
			M_RET( !(cpustate->AF.b.l & VF) );
			break;
		case 0xe1: cpustate->icount -= 10;	/* POP  H */
			M_POP(HL);
			break;
		case 0xe2: cpustate->icount -= 10;	/* JPO  nnnn */
			M_JMP( !(cpustate->AF.b.l & VF) );
			break;
		case 0xe3: cpustate->icount -= (cpustate->cputype) ? 16 : 18;	/* XTHL */
			M_POP(XX);
			M_PUSH(HL);
			cpustate->HL.d = cpustate->XX.d;
			break;
		case 0xe4: cpustate->icount -= 11;	/* CPO  nnnn */
			M_CALL( !(cpustate->AF.b.l & VF) );
			break;
		case 0xe5: cpustate->icount -= (cpustate->cputype) ? 12 : 11;	/* PUSH H */
			M_PUSH(HL);
			break;
		case 0xe6: cpustate->icount -= 7;	/* ANI  nn */
			cpustate->XX.b.l = ARG(cpustate);
			M_ANA(cpustate->XX.b.l);
			break;
		case 0xe7: cpustate->icount -= (cpustate->cputype) ? 12 : 11;	/* RST  4 */
			M_RST(4);
			break;

		case 0xe8: cpustate->icount -= (cpustate->cputype) ? 6 : 5;	/* RPE  */
			M_RET( cpustate->AF.b.l & VF );
			break;
		case 0xe9: cpustate->icount -= (cpustate->cputype) ? 6 : 5;	/* PCHL */
			cpustate->PC.d = cpustate->HL.w.l;
			break;
		case 0xea: cpustate->icount -= 10;	/* JPE  nnnn */
			M_JMP( cpustate->AF.b.l & VF );
			break;
		case 0xeb: cpustate->icount -= 5;	/* XCHG */
			cpustate->XX.d = cpustate->DE.d;
			cpustate->DE.d = cpustate->HL.d;
			cpustate->HL.d = cpustate->XX.d;
			break;
		case 0xec: cpustate->icount -= 11;	/* CPE  nnnn */
			M_CALL( cpustate->AF.b.l & VF );
			break;
		case 0xed:
			if( cpustate->cputype ) {
				cpustate->icount -= 10;		/* LHLX */
				cpustate->XX.w.l = cpustate->DE.w.l;
				cpustate->HL.b.l = RM(cpustate, cpustate->XX.d);
				cpustate->XX.w.l++;
				cpustate->HL.b.h = RM(cpustate, cpustate->XX.d);
			} else {
				cpustate->icount -= 17;	/* CALL nnnn undocumented */
				M_CALL(1);
			}
			break;
		case 0xee: cpustate->icount -= 7;	/* XRI  nn */
			cpustate->XX.b.l = ARG(cpustate);
			M_XRA(cpustate->XX.b.l);
			break;
		case 0xef: cpustate->icount -= (cpustate->cputype) ? 12 : 11;	/* RST  5 */
			M_RST(5);
			break;

		case 0xf0: cpustate->icount -= (cpustate->cputype) ? 6 : 5;	/* RP   */
			M_RET( !(cpustate->AF.b.l&SF) );
			break;
		case 0xf1: cpustate->icount -= 10;	/* POP  A */
			M_POP(AF);
			break;
		case 0xf2: cpustate->icount -= 10;	/* JP   nnnn */
			M_JMP( !(cpustate->AF.b.l & SF) );
			break;
		case 0xf3: cpustate->icount -= 4;	/* DI   */
			/* remove interrupt enable */
			cpustate->IM &= ~IM_IEN;
			if (cpustate->inte_callback) (*cpustate->inte_callback)(cpustate->device, (cpustate->IM & IM_IEN) ? 1 : 0);
			break;
		case 0xf4: cpustate->icount -= 11;	/* CP   nnnn */
			M_CALL( !(cpustate->AF.b.l & SF) );
			break;
		case 0xf5: cpustate->icount -= (cpustate->cputype) ? 12 : 11;	/* PUSH A */
			M_PUSH(AF);
			break;
		case 0xf6: cpustate->icount -= 7;	/* ORI  nn */
			cpustate->XX.b.l = ARG(cpustate);
			M_ORA(cpustate->XX.b.l);
			break;
		case 0xf7: cpustate->icount -= (cpustate->cputype) ? 12 : 11;	/* RST  6 */
			M_RST(6);
			break;

		case 0xf8: cpustate->icount -= (cpustate->cputype) ? 6 : 5;	/* RM   */
			M_RET( cpustate->AF.b.l & SF );
			break;
		case 0xf9: cpustate->icount -= (cpustate->cputype) ? 6 : 5;	/* SPHL */
			cpustate->SP.d = cpustate->HL.d;
			break;
		case 0xfa: cpustate->icount -= 10;	/* JM   nnnn */
			M_JMP( cpustate->AF.b.l & SF );
			break;
		case 0xfb: cpustate->icount -= 4;	/* EI */
			/* set interrupt enable */
			cpustate->IM |= IM_IEN;
			if (cpustate->inte_callback) (*cpustate->inte_callback)(cpustate->device, (cpustate->IM & IM_IEN) ? 1 : 0);
			cpustate->after_ei = TRUE;
			break;
		case 0xfc: cpustate->icount -= 11;	/* CM   nnnn */
			M_CALL( cpustate->AF.b.l & SF );
			break;
		case 0xfd:
			if( cpustate->cputype ) {
				cpustate->icount -= 7;		/* JX   nnnn */
				M_JMP( cpustate->AF.b.l & XF );
			} else {
				cpustate->icount -= 17;	/* CALL nnnn undocumented */
				M_CALL(1);
			}
			break;
		case 0xfe: cpustate->icount -= 7;	/* CPI  nn */
			cpustate->XX.b.l = ARG(cpustate);
			M_CMP(cpustate->XX.b.l);
			break;
		case 0xff: cpustate->icount -= (cpustate->cputype) ? 12 : 11;	/* RST  7 */
			M_RST(7);
			break;
	}
}

static void Interrupt(i8085_state *cpustate)
{

	if( cpustate->HALT )		/* if the CPU was halted */
	{
		cpustate->PC.w.l++; 	/* skip HALT instr */
		cpustate->HALT = 0;
		cpustate->STATUS = 0x26; // int ack while halt
	} else {
		cpustate->STATUS = 0x23; // int ack
	}
//AT
	cpustate->IREQ &= ~cpustate->ISRV; // remove serviced IRQ flag
	cpustate->rim_ien = (cpustate->ISRV==IM_TRAP) ? cpustate->IM & IM_IEN : 0; // latch general interrupt enable bit on TRAP or NMI
//ZT
	cpustate->IM &= ~IM_IEN;      /* remove general interrupt enable bit */

	if( cpustate->ISRV == IM_INTR )
	{
		LOG(("Interrupt get INTR vector\n"));
		cpustate->IRQ1 = (cpustate->irq_callback)(cpustate->device, 0);
	}

	if( cpustate->cputype )
	{
		if( cpustate->ISRV == IM_RST55 )
		{
			LOG(("Interrupt get RST5.5 vector\n"));
			//cpustate->IRQ1 = (cpustate->irq_callback)(cpustate->device, 1);
			cpustate->irq_state[I8085_RST55_LINE] = CLEAR_LINE; //AT: processing RST5.5, reset interrupt line
		}

		if( cpustate->ISRV == IM_RST65	)
		{
			LOG(("Interrupt get RST6.5 vector\n"));
			//cpustate->IRQ1 = (cpustate->irq_callback)(cpustate->device, 2);
			cpustate->irq_state[I8085_RST65_LINE] = CLEAR_LINE; //AT: processing RST6.5, reset interrupt line
		}

		if( cpustate->ISRV == IM_RST75 )
		{
			LOG(("Interrupt get RST7.5 vector\n"));
			//cpustate->IRQ1 = (cpustate->irq_callback)(cpustate->device, 3);
			cpustate->irq_state[I8085_RST75_LINE] = CLEAR_LINE; //AT: processing RST7.5, reset interrupt line
		}
	}

	switch( cpustate->IRQ1 & 0xff0000 )
	{
		case 0xcd0000:	/* CALL nnnn */
			cpustate->icount -= 7;
			M_PUSH(PC);
		case 0xc30000:	/* JMP  nnnn */
			cpustate->icount -= 10;
			cpustate->PC.d = cpustate->IRQ1 & 0xffff;
			break;
		default:
			switch( cpustate->ISRV )
			{
				case IM_TRAP:
				case IM_RST75:
				case IM_RST65:
				case IM_RST55:
					M_PUSH(PC);
					if (cpustate->IRQ1 != (1 << I8085_RST75_LINE))
						cpustate->PC.d = cpustate->IRQ1;
					else
						cpustate->PC.d = 0x3c;
					break;
				default:
					LOG(("i8085 take int $%02x\n", cpustate->IRQ1));
					execute_one(cpustate, cpustate->IRQ1 & 0xff);
			}
	}
	cpustate->ISRV = 0;
}

static CPU_EXECUTE( i8085 )
{
	i8085_state *cpustate = device->token;
	
	cpustate->icount = cycles;
	do
	{
		debugger_instruction_hook(device, cpustate->PC.d);

		/* interrupts enabled or TRAP pending ? */
		if ( ((cpustate->IM & IM_IEN) && (!cpustate->after_ei)) || (cpustate->IREQ & IM_TRAP) )
		{
			/* copy scheduled to executed interrupt request */
			cpustate->IRQ1 = cpustate->IRQ2;
			/* reset scheduled interrupt request */
			cpustate->IRQ2 = 0;
			/* interrupt now ? */
			if (cpustate->IRQ1) Interrupt(cpustate);
		}

		cpustate->after_ei = FALSE;
		/* here we go... */
		execute_one(cpustate, ROP(cpustate));

	} while (cpustate->icount > 0);

	return cycles - cpustate->icount;
}

/****************************************************************************
 * Initialise the various lookup tables used by the emulation code
 ****************************************************************************/
static void init_tables (void)
{
	UINT8 zs;
	int i, p;
	for (i = 0; i < 256; i++)
	{
		zs = 0;
		if (i==0) zs |= ZF;
		if (i&128) zs |= SF;
		p = 0;
		if (i&1) ++p;
		if (i&2) ++p;
		if (i&4) ++p;
		if (i&8) ++p;
		if (i&16) ++p;
		if (i&32) ++p;
		if (i&64) ++p;
		if (i&128) ++p;
		ZS[i] = zs;
		ZSP[i] = zs | ((p&1) ? 0 : VF);
	}
}

/****************************************************************************
 * Init the 8085 emulation
 ****************************************************************************/
static CPU_INIT( i8085 )
{
	i8085_state *cpustate = device->token;

	init_tables();
	cpustate->cputype = 1;
	cpustate->irq_callback = irqcallback;
	cpustate->device = device;
	cpustate->program = memory_find_address_space(device, ADDRESS_SPACE_PROGRAM);
	cpustate->io = memory_find_address_space(device, ADDRESS_SPACE_IO);

	state_save_register_device_item(device, 0, cpustate->AF.w.l);
	state_save_register_device_item(device, 0, cpustate->BC.w.l);
	state_save_register_device_item(device, 0, cpustate->DE.w.l);
	state_save_register_device_item(device, 0, cpustate->HL.w.l);
	state_save_register_device_item(device, 0, cpustate->SP.w.l);
	state_save_register_device_item(device, 0, cpustate->PC.w.l);
	state_save_register_device_item(device, 0, cpustate->HALT);
	state_save_register_device_item(device, 0, cpustate->IM);
	state_save_register_device_item(device, 0, cpustate->IREQ);
	state_save_register_device_item(device, 0, cpustate->ISRV);
	state_save_register_device_item(device, 0, cpustate->INTR);
	state_save_register_device_item(device, 0, cpustate->IRQ2);
	state_save_register_device_item(device, 0, cpustate->IRQ1);
	state_save_register_device_item(device, 0, cpustate->STATUS);
	state_save_register_device_item(device, 0, cpustate->after_ei);
	state_save_register_device_item_array(device, 0, cpustate->irq_state);
}

/****************************************************************************
 * Reset the 8085 emulation
 ****************************************************************************/
static CPU_RESET( i8085 )
{
	i8085_state *cpustate = device->token;
	cpu_irq_callback save_irqcallback;
	i8085_sod_func save_sodcallback;
	i8085_sid_func save_sidcallback;
	i8085_inte_func save_intecallback;
	i8085_status_func save_statuscallback;
	int cputype_bak = cpustate->cputype;

	init_tables();
	save_irqcallback = cpustate->irq_callback;
	save_sodcallback = cpustate->sod_callback;
	save_sidcallback = cpustate->sid_callback;
	save_intecallback = cpustate->inte_callback;
	save_statuscallback = cpustate->status_callback;
	memset(cpustate, 0, sizeof(*cpustate));
	cpustate->irq_callback = save_irqcallback;
	cpustate->sod_callback = save_sodcallback;
	cpustate->sid_callback = save_sidcallback;
	cpustate->inte_callback = save_intecallback;
	cpustate->status_callback = save_statuscallback;
	cpustate->device = device;
	cpustate->program = memory_find_address_space(device, ADDRESS_SPACE_PROGRAM);
	cpustate->io = memory_find_address_space(device, ADDRESS_SPACE_IO);

	cpustate->cputype = cputype_bak;
	cpustate->after_ei = FALSE;

	if (cpustate->inte_callback) (*cpustate->inte_callback)(cpustate->device, (cpustate->IM & IM_IEN) ? 1 : 0);
}

/****************************************************************************
 * Shut down the CPU emulation
 ****************************************************************************/
static CPU_EXIT( i8085 )
{
	/* nothing to do */
}

/****************************************************************************/
/* Set TRAP signal state                                                    */
/****************************************************************************/
static void i8085_set_TRAP(i8085_state *cpustate, int state)
{
	LOG(("i8085: TRAP %d\n", state));
	if (state)
	{
		cpustate->IREQ |= IM_TRAP;
		if( cpustate->ISRV & IM_TRAP ) return;	/* already servicing TRAP ? */
		cpustate->ISRV = IM_TRAP;				/* service TRAP */
		cpustate->IRQ2 = ADDR_TRAP;
	}
	else
	{
		cpustate->IREQ &= ~IM_TRAP; 			/* remove request for TRAP */
	}
}

/****************************************************************************/
/* Set RST7.5 signal state                                                  */
/****************************************************************************/
static void i8085_set_RST75(i8085_state *cpustate, int state)
{
	LOG(("i8085: RST7.5 %d\n", state));
	if( state )
	{

		cpustate->IREQ |= IM_RST75; 			/* request RST7.5 */
		cpustate->irq_state[I8085_RST75_LINE] = CLEAR_LINE;	/* clear latch */
		if( cpustate->IM & IM_RST75 ) return;	/* if masked, ignore it for now */
		if( !cpustate->ISRV )					/* if no higher priority IREQ is serviced */
		{
			cpustate->ISRV = IM_RST75;			/* service RST7.5 */
			cpustate->IRQ2 = ADDR_RST75;
		}
	}
	/* RST7.5 is reset only by SIM or end of service routine ! */
}

/****************************************************************************/
/* Set RST6.5 signal state                                                  */
/****************************************************************************/
static void i8085_set_RST65(i8085_state *cpustate, int state)
{
	LOG(("i8085: RST6.5 %d\n", state));
	if( state )
	{
		cpustate->IREQ |= IM_RST65; 			/* request RST6.5 */
		if( cpustate->IM & IM_RST65 ) return;	/* if masked, ignore it for now */
		if( !cpustate->ISRV )					/* if no higher priority IREQ is serviced */
		{
			cpustate->ISRV = IM_RST65;			/* service RST6.5 */
			cpustate->IRQ2 = ADDR_RST65;
		}
	}
	else
	{
		cpustate->IREQ &= ~IM_RST65;			/* remove request for RST6.5 */
	}
}

/****************************************************************************/
/* Set RST5.5 signal state                                                  */
/****************************************************************************/
static void i8085_set_RST55(i8085_state *cpustate, int state)
{
	LOG(("i8085: RST5.5 %d\n", state));
	if( state )
	{
		cpustate->IREQ |= IM_RST55; 			/* request RST5.5 */
		if( cpustate->IM & IM_RST55 ) return;	/* if masked, ignore it for now */
		if( !cpustate->ISRV )					/* if no higher priority IREQ is serviced */
		{
			cpustate->ISRV = IM_RST55;			/* service RST5.5 */
			cpustate->IRQ2 = ADDR_RST55;
		}
	}
	else
	{
		cpustate->IREQ &= ~IM_RST55;			/* remove request for RST5.5 */
	}
}

/****************************************************************************/
/* Set INTR signal                                                          */
/****************************************************************************/
static void i8085_set_INTR(i8085_state *cpustate, int state)
{
	LOG(("i8085: INTR %d\n", state));
	if( state )
	{
		cpustate->IREQ |= IM_INTR;				/* request INTR */
		//cpustate->INTR = state;
		cpustate->INTR = I8085_INTR; //AT: cpustate->INTR is supposed to hold IRQ0 vector(0x38) (0xff in this implementation)
		if( cpustate->IM & IM_INTR ) return;	/* if masked, ignore it for now */
		if( !cpustate->ISRV )					/* if no higher priority IREQ is serviced */
		{
			cpustate->ISRV = IM_INTR;			/* service INTR */
			cpustate->IRQ2 = cpustate->INTR;
		}
	}
	else
	{
		cpustate->IREQ &= ~IM_INTR; 			/* remove request for INTR */
	}
}

static void i8085_set_irq_line(i8085_state *cpustate, int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
	{
		if( state != CLEAR_LINE )
			i8085_set_TRAP(cpustate, 1);
	}
	else if (irqline < 4)
	{
		if (irqline == I8085_RST75_LINE)	/* RST7.5 is latched on rising edge, the others are sampled */
		{
			if( state != CLEAR_LINE )
				cpustate->irq_state[irqline] = state;
		}
		else
			cpustate->irq_state[irqline] = state;

		if (state == CLEAR_LINE)
		{
			if( !(cpustate->IM & IM_IEN) )
			{
				switch (irqline)
				{
					case I8085_INTR_LINE: i8085_set_INTR(cpustate, 0); break;
					case I8085_RST55_LINE: i8085_set_RST55(cpustate, 0); break;
					case I8085_RST65_LINE: i8085_set_RST65(cpustate, 0); break;
					case I8085_RST75_LINE: i8085_set_RST75(cpustate, 0); break;
				}
			}
		}
		else
		{
			if( cpustate->IM & IM_IEN )
			{
				switch( irqline )
				{
					case I8085_INTR_LINE: i8085_set_INTR(cpustate, 1); break;
					case I8085_RST55_LINE: i8085_set_RST55(cpustate, 1); break;
					case I8085_RST65_LINE: i8085_set_RST65(cpustate, 1); break;
					case I8085_RST75_LINE: i8085_set_RST75(cpustate, 1); break;
				}
			}
		}
	}
}


/**************************************************************************
 * 8080 section
 **************************************************************************/
#if (HAS_8080)

static CPU_INIT( i8080 )
{
	i8085_state *cpustate = device->token;

	init_tables();
	cpustate->cputype = 0;
	cpustate->irq_callback = irqcallback;
	cpustate->device = device;

	state_save_register_device_item(device, 0, cpustate->AF.w.l);
	state_save_register_device_item(device, 0, cpustate->BC.w.l);
	state_save_register_device_item(device, 0, cpustate->DE.w.l);
	state_save_register_device_item(device, 0, cpustate->HL.w.l);
	state_save_register_device_item(device, 0, cpustate->SP.w.l);
	state_save_register_device_item(device, 0, cpustate->PC.w.l);
	state_save_register_device_item(device, 0, cpustate->HALT);
	state_save_register_device_item(device, 0, cpustate->IM);
	state_save_register_device_item(device, 0, cpustate->IREQ);
	state_save_register_device_item(device, 0, cpustate->ISRV);
	state_save_register_device_item(device, 0, cpustate->INTR);
	state_save_register_device_item(device, 0, cpustate->IRQ2);
	state_save_register_device_item(device, 0, cpustate->IRQ1);
	state_save_register_device_item(device, 0, cpustate->STATUS);
	state_save_register_device_item_array(device, 0, cpustate->irq_state);
}

static void i8080_set_irq_line(i8085_state *cpustate, int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
	{
		if( state != CLEAR_LINE )
			i8085_set_TRAP(cpustate, 1);
	}
	else
	{
		cpustate->irq_state[irqline] = state;
		if (state == CLEAR_LINE)
		{
			if (!(cpustate->IM & IM_IEN))
				i8085_set_INTR(cpustate, 0);
		}
		else
		{
			if (cpustate->IM & IM_IEN)
				i8085_set_INTR(cpustate, 1);
		}
	}
}
#endif


/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( i8085 )
{
	i8085_state *cpustate = device->token;
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + I8085_INTR_LINE:	i8085_set_irq_line(cpustate, I8085_INTR_LINE, info->i); break;
		case CPUINFO_INT_INPUT_STATE + I8085_RST55_LINE:i8085_set_irq_line(cpustate, I8085_RST55_LINE, info->i); break;
		case CPUINFO_INT_INPUT_STATE + I8085_RST65_LINE:i8085_set_irq_line(cpustate, I8085_RST65_LINE, info->i); break;
		case CPUINFO_INT_INPUT_STATE + I8085_RST75_LINE:i8085_set_irq_line(cpustate, I8085_RST75_LINE, info->i); break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	i8085_set_irq_line(cpustate, INPUT_LINE_NMI, info->i); break;

		case CPUINFO_INT_PC:							cpustate->PC.w.l = info->i; 					break;
		case CPUINFO_INT_REGISTER + I8085_PC:			cpustate->PC.w.l = info->i;						break;
		case CPUINFO_INT_SP:							cpustate->SP.w.l = info->i;						break;
		case CPUINFO_INT_REGISTER + I8085_SP:			cpustate->SP.w.l = info->i;						break;
		case CPUINFO_INT_REGISTER + I8085_AF:			cpustate->AF.w.l = info->i;						break;
		case CPUINFO_INT_REGISTER + I8085_BC:			cpustate->BC.w.l = info->i;						break;
		case CPUINFO_INT_REGISTER + I8085_DE:			cpustate->DE.w.l = info->i;						break;
		case CPUINFO_INT_REGISTER + I8085_HL:			cpustate->HL.w.l = info->i;						break;
		case CPUINFO_INT_REGISTER + I8085_IM:			cpustate->IM = info->i;							break;
		case CPUINFO_INT_REGISTER + I8085_HALT:			cpustate->HALT = info->i;						break;
		case CPUINFO_INT_REGISTER + I8085_IREQ:			cpustate->IREQ = info->i;						break;
		case CPUINFO_INT_REGISTER + I8085_ISRV:			cpustate->ISRV = info->i;						break;
		case CPUINFO_INT_REGISTER + I8085_VECTOR:		cpustate->INTR = info->i;						break;
		case CPUINFO_INT_REGISTER + I8085_STATUS:		cpustate->STATUS = info->i;						break;

		case CPUINFO_INT_I8085_SID:						if (info->i) cpustate->IM |= IM_SID; else cpustate->IM &= ~IM_SID; break;

		/* --- the following bits of info are set as pointers to data or functions --- */
		case CPUINFO_PTR_I8085_SOD_CALLBACK:			cpustate->sod_callback = (i8085_sod_func)info->f; break;
		case CPUINFO_PTR_I8085_SID_CALLBACK:			cpustate->sid_callback = (i8085_sid_func)info->f; break;
		case CPUINFO_PTR_I8085_INTE_CALLBACK:			cpustate->inte_callback = (i8085_inte_func)info->f; break;
		case CPUINFO_PTR_I8085_STATUS_CALLBACK:			cpustate->status_callback = (i8085_status_func)info->f; break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

CPU_GET_INFO( i8085 )
{
	i8085_state *cpustate = (device != NULL) ? device->token : NULL;
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(i8085_state);			break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 4;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0xff;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;			break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 2;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 3;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 4;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 16;							break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + I8085_INTR_LINE:	info->i = (cpustate->IREQ & IM_INTR) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + I8085_RST55_LINE:info->i = (cpustate->IREQ & IM_RST55) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + I8085_RST65_LINE:info->i = (cpustate->IREQ & IM_RST65) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + I8085_RST75_LINE:info->i = (cpustate->IREQ & IM_RST75) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	info->i = (cpustate->IREQ & IM_TRAP) ? ASSERT_LINE : CLEAR_LINE; break;

		case CPUINFO_INT_PREVIOUSPC:					/* not supported */						break;

		case CPUINFO_INT_PC:							info->i = cpustate->PC.d;						break;
		case CPUINFO_INT_REGISTER + I8085_PC:			info->i = cpustate->PC.w.l;						break;
		case CPUINFO_INT_SP:							info->i = cpustate->SP.d;						break;
		case CPUINFO_INT_REGISTER + I8085_SP:			info->i = cpustate->SP.w.l;						break;
		case CPUINFO_INT_REGISTER + I8085_AF:			info->i = cpustate->AF.w.l;						break;
		case CPUINFO_INT_REGISTER + I8085_BC:			info->i = cpustate->BC.w.l;						break;
		case CPUINFO_INT_REGISTER + I8085_DE:			info->i = cpustate->DE.w.l;						break;
		case CPUINFO_INT_REGISTER + I8085_HL:			info->i = cpustate->HL.w.l;						break;
		case CPUINFO_INT_REGISTER + I8085_IM:			info->i = cpustate->IM;							break;
		case CPUINFO_INT_REGISTER + I8085_HALT:			info->i = cpustate->HALT;						break;
		case CPUINFO_INT_REGISTER + I8085_IREQ:			info->i = cpustate->IREQ;						break;
		case CPUINFO_INT_REGISTER + I8085_ISRV:			info->i = cpustate->ISRV;						break;
		case CPUINFO_INT_REGISTER + I8085_VECTOR:		info->i = cpustate->INTR;						break;
		case CPUINFO_INT_REGISTER + I8085_STATUS:		info->i = cpustate->STATUS;						break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(i8085);			break;
		case CPUINFO_PTR_INIT:							info->init = CPU_INIT_NAME(i8085);				break;
		case CPUINFO_PTR_RESET:							info->reset = CPU_RESET_NAME(i8085);				break;
		case CPUINFO_PTR_EXIT:							info->exit = CPU_EXIT_NAME(i8085);				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = CPU_EXECUTE_NAME(i8085);			break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(i8085);			break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "8085A");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "Intel 8080");			break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "1.1");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright Juergen Buchmueller, all rights reserved."); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c%c%c%c%c",
				cpustate->AF.b.l & 0x80 ? 'S':'.',
				cpustate->AF.b.l & 0x40 ? 'Z':'.',
				cpustate->AF.b.l & 0x20 ? '?':'.',
				cpustate->AF.b.l & 0x10 ? 'H':'.',
				cpustate->AF.b.l & 0x08 ? '?':'.',
				cpustate->AF.b.l & 0x04 ? 'P':'.',
				cpustate->AF.b.l & 0x02 ? 'N':'.',
				cpustate->AF.b.l & 0x01 ? 'C':'.');
			break;

		case CPUINFO_STR_REGISTER + I8085_AF:			sprintf(info->s, "AF:%04X", cpustate->AF.w.l);	break;
		case CPUINFO_STR_REGISTER + I8085_BC:			sprintf(info->s, "BC:%04X", cpustate->BC.w.l);	break;
		case CPUINFO_STR_REGISTER + I8085_DE:			sprintf(info->s, "DE:%04X", cpustate->DE.w.l);	break;
		case CPUINFO_STR_REGISTER + I8085_HL:			sprintf(info->s, "HL:%04X", cpustate->HL.w.l);	break;
		case CPUINFO_STR_REGISTER + I8085_SP:			sprintf(info->s, "SP:%04X", cpustate->SP.w.l);	break;
		case CPUINFO_STR_REGISTER + I8085_PC:			sprintf(info->s, "PC:%04X", cpustate->PC.w.l);	break;
		case CPUINFO_STR_REGISTER + I8085_IM:			sprintf(info->s, "IM:%02X", cpustate->IM);		break;
		case CPUINFO_STR_REGISTER + I8085_HALT:			sprintf(info->s, "HALT:%d", cpustate->HALT);	break;
		case CPUINFO_STR_REGISTER + I8085_IREQ:			sprintf(info->s, "IREQ:%02X", cpustate->IREQ);	break;
		case CPUINFO_STR_REGISTER + I8085_ISRV:			sprintf(info->s, "ISRV:%02X", cpustate->ISRV);	break;
		case CPUINFO_STR_REGISTER + I8085_VECTOR:		sprintf(info->s, "VEC:%02X", cpustate->INTR);	break;
		case CPUINFO_STR_REGISTER + I8085_STATUS:		sprintf(info->s, "SW:%02X", cpustate->STATUS);	break;
	}
}


#if (HAS_8080)
/**************************************************************************
 * CPU-specific get_info/set_info
 **************************************************************************/

static CPU_SET_INFO( i8080 )
{
	i8085_state *cpustate = device->token;
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + I8080_INTR_LINE:	i8080_set_irq_line(cpustate, I8080_INTR_LINE, info->i); break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	i8080_set_irq_line(cpustate, INPUT_LINE_NMI, info->i); break;

		default:										CPU_SET_INFO_CALL(i8085); break;
	}
}

CPU_GET_INFO( i8080 )
{
	i8085_state *cpustate = (device != NULL) ? device->token : NULL;
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 1;							break;
		case CPUINFO_INT_INPUT_STATE + I8085_INTR_LINE:	info->i = (cpustate->IREQ & IM_INTR) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	info->i = (cpustate->IREQ & IM_TRAP) ? ASSERT_LINE : CLEAR_LINE; break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(i8080);			break;
		case CPUINFO_PTR_INIT:							info->init = CPU_INIT_NAME(i8080);				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "8080");				break;

		default:										CPU_GET_INFO_CALL(i8085); break;
	}
}
#endif
