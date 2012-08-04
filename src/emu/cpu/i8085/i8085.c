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
 * August 2009, hap
 *
 * - removed DAA table
 * - fixed accidental double memory reads due to macro overuse
 * - fixed cycle deduction on unconditional CALL / RET
 * - added cycle tables and cleaned up big switch source layout (1 tab = 4 spaces)
 * - removed HLT cycle eating (earlier, HLT after EI could theoretically fail)
 * - fixed parity flag on add/sub/cmp
 * - renamed temp register XX to official name WZ
 * - renamed flags from Z80 style S Z Y H X V N C  to  S Z X5 H X3 P V C, and
 *   fixed X5 / V flags where accidentally broken due to flag names confusion
 *
 * 21-Aug-2009, Curt Coder
 *
 * - added 8080A variant
 * - refactored callbacks to use devcb
 *
 *****************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "i8085.h"
#include "i8085cpu.h"

#define VERBOSE 0

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

#define CPUTYPE_8080	0
#define CPUTYPE_8085	1



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _i8085_state i8085_state;
struct _i8085_state
{
	i8085_config		config;

	devcb_resolved_write8		out_status_func;
	devcb_resolved_write_line	out_inte_func;
	devcb_resolved_read_line	in_sid_func;
	devcb_resolved_write_line	out_sod_func;

	int 				cputype;		/* 0 8080, 1 8085A */
	PAIR				PC,SP,AF,BC,DE,HL,WZ;
	UINT8				HALT;
	UINT8				IM; 			/* interrupt mask (8085A only) */
	UINT8   			STATUS;			/* status word */

	UINT8				after_ei;		/* post-EI processing; starts at 2, check for ints at 0 */
	UINT8				nmi_state;		/* raw NMI line state */
	UINT8				irq_state[4];	/* raw IRQ line states */
	UINT8				trap_pending;	/* TRAP interrupt latched? */
	UINT8				trap_im_copy;	/* copy of IM register when TRAP was taken */
	UINT8				sod_state;		/* state of the SOD line */

	UINT8				ietemp;			/* import/export temp space */

	device_irq_acknowledge_callback	irq_callback;
	legacy_cpu_device *device;
	address_space *program;
	direct_read_data *direct;
	address_space *io;
	int					icount;
};



/***************************************************************************
    MACROS
***************************************************************************/

#define IS_8080(c)			((c)->cputype == CPUTYPE_8080)
#define IS_8085(c)			((c)->cputype == CPUTYPE_8085)



/***************************************************************************
    STATIC TABLES
***************************************************************************/

/* cycles lookup */
static const UINT8 lut_cycles_8080[256]={
/*      0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F  */
/* 0 */ 4, 10,7, 5, 5, 5, 7, 4, 4, 10,7, 5, 5, 5, 7, 4,
/* 1 */ 4, 10,7, 5, 5, 5, 7, 4, 4, 10,7, 5, 5, 5, 7, 4,
/* 2 */ 4, 10,16,5, 5, 5, 7, 4, 4, 10,16,5, 5, 5, 7, 4,
/* 3 */ 4, 10,13,5, 10,10,10,4, 4, 10,13,5, 5, 5, 7, 4,
/* 4 */ 5, 5, 5, 5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 7, 5,
/* 5 */ 5, 5, 5, 5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 7, 5,
/* 6 */ 5, 5, 5, 5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 7, 5,
/* 7 */ 7, 7, 7, 7, 7, 7, 7, 7, 5, 5, 5, 5, 5, 5, 7, 5,
/* 8 */ 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
/* 9 */ 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
/* A */ 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
/* B */ 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
/* C */ 5, 10,10,10,11,11,7, 11,5, 10,10,10,11,11,7, 11,
/* D */ 5, 10,10,10,11,11,7, 11,5, 10,10,10,11,11,7, 11,
/* E */ 5, 10,10,18,11,11,7, 11,5, 5, 10,5, 11,11,7, 11,
/* F */ 5, 10,10,4, 11,11,7, 11,5, 5, 10,4, 11,11,7, 11 };
static const UINT8 lut_cycles_8085[256]={
/*      0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F  */
/* 0 */ 4, 10,7, 6, 4, 4, 7, 4, 10,10,7, 6, 4, 4, 7, 4,
/* 1 */ 7, 10,7, 6, 4, 4, 7, 4, 10,10,7, 6, 4, 4, 7, 4,
/* 2 */ 7, 10,16,6, 4, 4, 7, 4, 10,10,16,6, 4, 4, 7, 4,
/* 3 */ 7, 10,13,6, 10,10,10,4, 10,10,13,6, 4, 4, 7, 4,
/* 4 */ 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
/* 5 */ 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
/* 6 */ 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
/* 7 */ 7, 7, 7, 7, 7, 7, 5, 7, 4, 4, 4, 4, 4, 4, 7, 4,
/* 8 */ 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
/* 9 */ 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
/* A */ 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
/* B */ 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
/* C */ 6, 10,10,10,11,12,7, 12,6, 10,10,12,11,11,7, 12,
/* D */ 6, 10,10,10,11,12,7, 12,6, 10,10,10,11,10,7, 12,
/* E */ 6, 10,10,16,11,12,7, 12,6, 6, 10,5, 11,10,7, 12,
/* F */ 6, 10,10,4, 11,12,7, 12,6, 6, 10,4, 11,10,7, 12 };

/* special cases (partially taken care of elsewhere):
               base c    taken?   not taken?
M_RET  8080    5         +6(11)   -0            (conditional)
M_RET  8085    6         +6(12)   -0            (conditional)
M_JMP  8080    10        +0       -0
M_JMP  8085    10        +0       -3(7)
M_CALL 8080    11        +6(17)   -0
M_CALL 8085    11        +7(18)   -2(9)

*/
static UINT8 lut_cycles[256];

/* flags lookup */
static UINT8 ZS[256];
static UINT8 ZSP[256];



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void execute_one(i8085_state *cpustate, int opcode);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE i8085_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == I8080 ||
		   device->type() == I8080A||
		   device->type() == I8085A);
	return (i8085_state *)downcast<legacy_cpu_device *>(device)->token();
}

INLINE void set_sod(i8085_state *cpustate, int state)
{
	if (state != 0 && cpustate->sod_state == 0)
	{
		cpustate->sod_state = 1;
		cpustate->out_sod_func(cpustate->sod_state);
	}
	else if (state == 0 && cpustate->sod_state != 0)
	{
		cpustate->sod_state = 0;
		cpustate->out_sod_func(cpustate->sod_state);
	}
}


INLINE void set_inte(i8085_state *cpustate, int state)
{
	if (state != 0 && (cpustate->IM & IM_IE) == 0)
	{
		cpustate->IM |= IM_IE;
		cpustate->out_inte_func(1);
	}
	else if (state == 0 && (cpustate->IM & IM_IE) != 0)
	{
		cpustate->IM &= ~IM_IE;
		cpustate->out_inte_func(0);
	}
}


INLINE void set_status(i8085_state *cpustate, UINT8 status)
{
	if (status != cpustate->STATUS)
		cpustate->out_status_func(0, status);

	cpustate->STATUS = status;
}


INLINE UINT8 get_rim_value(i8085_state *cpustate)
{
	UINT8 result = cpustate->IM;
	int sid = cpustate->in_sid_func();

	/* copy live RST5.5 and RST6.5 states */
	result &= ~(IM_I65 | IM_I55);
	if (cpustate->irq_state[I8085_RST65_LINE]) result |= IM_I65;
	if (cpustate->irq_state[I8085_RST55_LINE]) result |= IM_I55;

	/* fetch the SID bit if we have a callback */
	result = (result & 0x7f) | (sid ? 0x80 : 0);

	return result;
}


INLINE void break_halt_for_interrupt(i8085_state *cpustate)
{
	/* de-halt if necessary */
	if (cpustate->HALT)
	{
		cpustate->PC.w.l++;
		cpustate->HALT = 0;
		set_status(cpustate, 0x26);	/* int ack while halt */
	}
	else
		set_status(cpustate, 0x23);	/* int ack */
}


INLINE UINT8 ROP(i8085_state *cpustate)
{
	set_status(cpustate, 0xa2); // instruction fetch
	return cpustate->direct->read_decrypted_byte(cpustate->PC.w.l++);
}

INLINE UINT8 ARG(i8085_state *cpustate)
{
	return cpustate->direct->read_raw_byte(cpustate->PC.w.l++);
}

INLINE UINT16 ARG16(i8085_state *cpustate)
{
	UINT16 w;
	w  = cpustate->direct->read_raw_byte(cpustate->PC.d);
	cpustate->PC.w.l++;
	w += cpustate->direct->read_raw_byte(cpustate->PC.d) << 8;
	cpustate->PC.w.l++;
	return w;
}

INLINE UINT8 RM(i8085_state *cpustate, UINT32 a)
{
	set_status(cpustate, 0x82); // memory read
	return cpustate->program->read_byte(a);
}

INLINE void WM(i8085_state *cpustate, UINT32 a, UINT8 v)
{
	set_status(cpustate, 0x00); // memory write
	cpustate->program->write_byte(a, v);
}


static void check_for_interrupts(i8085_state *cpustate)
{
	/* TRAP is the highest priority */
	if (cpustate->trap_pending)
	{
		/* the first RIM after a TRAP reflects the original IE state; remember it here,
           setting the high bit to indicate it is valid */
		cpustate->trap_im_copy = cpustate->IM | 0x80;

		/* reset the pending state */
		cpustate->trap_pending = FALSE;

		/* break out of HALT state and call the IRQ ack callback */
		break_halt_for_interrupt(cpustate);
		if (cpustate->irq_callback != NULL)
			(*cpustate->irq_callback)(cpustate->device, INPUT_LINE_NMI);

		/* push the PC and jump to $0024 */
		M_PUSH(PC);
		set_inte(cpustate, 0);
		cpustate->PC.w.l = ADDR_TRAP;
		cpustate->icount -= 11;
	}

	/* followed by RST7.5 */
	else if ((cpustate->IM & IM_I75) && !(cpustate->IM & IM_M75) && (cpustate->IM & IM_IE))
	{
		/* reset the pending state (which is CPU-visible via the RIM instruction) */
		cpustate->IM &= ~IM_I75;

		/* break out of HALT state and call the IRQ ack callback */
		break_halt_for_interrupt(cpustate);
		if (cpustate->irq_callback != NULL)
			(*cpustate->irq_callback)(cpustate->device, I8085_RST75_LINE);

		/* push the PC and jump to $003C */
		M_PUSH(PC);
		set_inte(cpustate, 0);
		cpustate->PC.w.l = ADDR_RST75;
		cpustate->icount -= 11;
	}

	/* followed by RST6.5 */
	else if (cpustate->irq_state[I8085_RST65_LINE] && !(cpustate->IM & IM_M65) && (cpustate->IM & IM_IE))
	{
		/* break out of HALT state and call the IRQ ack callback */
		break_halt_for_interrupt(cpustate);
		if (cpustate->irq_callback != NULL)
			(*cpustate->irq_callback)(cpustate->device, I8085_RST65_LINE);

		/* push the PC and jump to $0034 */
		M_PUSH(PC);
		set_inte(cpustate, 0);
		cpustate->PC.w.l = ADDR_RST65;
		cpustate->icount -= 11;
	}

	/* followed by RST5.5 */
	else if (cpustate->irq_state[I8085_RST55_LINE] && !(cpustate->IM & IM_M55) && (cpustate->IM & IM_IE))
	{
		/* break out of HALT state and call the IRQ ack callback */
		break_halt_for_interrupt(cpustate);
		if (cpustate->irq_callback != NULL)
			(*cpustate->irq_callback)(cpustate->device, I8085_RST55_LINE);

		/* push the PC and jump to $002C */
		M_PUSH(PC);
		set_inte(cpustate, 0);
		cpustate->PC.w.l = ADDR_RST55;
		cpustate->icount -= 11;
	}

	/* followed by classic INTR */
	else if (cpustate->irq_state[I8085_INTR_LINE] && (cpustate->IM & IM_IE))
	{
		UINT32 vector = 0;

		/* break out of HALT state and call the IRQ ack callback */
		break_halt_for_interrupt(cpustate);
		if (cpustate->irq_callback != NULL)
			vector = (*cpustate->irq_callback)(cpustate->device, I8085_INTR_LINE);

		/* use the resulting vector as an opcode to execute */
		set_inte(cpustate, 0);
		switch (vector & 0xff0000)
		{
			case 0xcd0000:	/* CALL nnnn */
				cpustate->icount -= 7;
				M_PUSH(PC);
			case 0xc30000:	/* JMP  nnnn */
				cpustate->icount -= 10;
				cpustate->PC.d = vector & 0xffff;
				break;

			default:
				LOG(("i8085 take int $%02x\n", vector));
				execute_one(cpustate, vector & 0xff);
				break;
		}
	}
}


static void execute_one(i8085_state *cpustate, int opcode)
{
	cpustate->icount -= lut_cycles[opcode];

	switch (opcode)
	{
		case 0x00:														break;	/* NOP  */
		case 0x01:	cpustate->BC.w.l = ARG16(cpustate);					break;	/* LXI  B,nnnn */
		case 0x02:	WM(cpustate, cpustate->BC.d, cpustate->AF.b.h);		break;	/* STAX B */
		case 0x03:	cpustate->BC.w.l++;											/* INX  B */
					if (IS_8085(cpustate)) { if (cpustate->BC.w.l == 0x0000) cpustate->AF.b.l |= X5F; else cpustate->AF.b.l &= ~X5F; }
					break;
		case 0x04:	M_INR(cpustate->BC.b.h);							break;	/* INR  B */
		case 0x05:	M_DCR(cpustate->BC.b.h);							break;	/* DCR  B */
		case 0x06:	M_MVI(cpustate->BC.b.h);							break;	/* MVI  B,nn */
		case 0x07:	M_RLC;												break;	/* RLC  */

		case 0x08:	if (IS_8085(cpustate)) { M_DSUB(cpustate); }				/* DSUB */
					/* else { ; } */											/* NOP  undocumented */
					break;
		case 0x09:	M_DAD(BC);											break;	/* DAD  B */
		case 0x0a:	cpustate->AF.b.h = RM(cpustate, cpustate->BC.d);	break;	/* LDAX B */
		case 0x0b:	cpustate->BC.w.l--;											/* DCX  B */
					if (IS_8085(cpustate)) { if (cpustate->BC.w.l == 0xffff) cpustate->AF.b.l |= X5F; else cpustate->AF.b.l &= ~X5F; }
					break;
		case 0x0c:	M_INR(cpustate->BC.b.l);							break;	/* INR  C */
		case 0x0d:	M_DCR(cpustate->BC.b.l);							break;	/* DCR  C */
		case 0x0e:	M_MVI(cpustate->BC.b.l);							break;	/* MVI  C,nn */
		case 0x0f:	M_RRC;												break;	/* RRC  */

		case 0x10:	if (IS_8085(cpustate)) {									/* ASRH */
						cpustate->AF.b.l = (cpustate->AF.b.l & ~CF) | (cpustate->HL.b.l & CF);
						cpustate->HL.w.l = (cpustate->HL.w.l >> 1);
					} /* else { ; } */											/* NOP  undocumented */
					break;
		case 0x11:	cpustate->DE.w.l = ARG16(cpustate);					break;	/* LXI  D,nnnn */
		case 0x12:	WM(cpustate, cpustate->DE.d, cpustate->AF.b.h);		break;	/* STAX D */
		case 0x13:	cpustate->DE.w.l++;											/* INX  D */
					if (IS_8085(cpustate)) { if (cpustate->DE.w.l == 0x0000) cpustate->AF.b.l |= X5F; else cpustate->AF.b.l &= ~X5F; }
					break;
		case 0x14:	M_INR(cpustate->DE.b.h);							break;	/* INR  D */
		case 0x15:	M_DCR(cpustate->DE.b.h);							break;	/* DCR  D */
		case 0x16:	M_MVI(cpustate->DE.b.h);							break;	/* MVI  D,nn */
		case 0x17:	M_RAL;												break;	/* RAL  */

		case 0x18:	if (IS_8085(cpustate)) {									/* RLDE */
						cpustate->AF.b.l = (cpustate->AF.b.l & ~(CF | VF)) | (cpustate->DE.b.h >> 7);
						cpustate->DE.w.l = (cpustate->DE.w.l << 1) | (cpustate->DE.w.l >> 15);
						if (0 != (((cpustate->DE.w.l >> 15) ^ cpustate->AF.b.l) & CF)) cpustate->AF.b.l |= VF;
					} /* else { ; } */											/* NOP  undocumented */
					break;
		case 0x19:	M_DAD(DE);											break;	/* DAD  D */
		case 0x1a:	cpustate->AF.b.h = RM(cpustate, cpustate->DE.d);	break;	/* LDAX D */
		case 0x1b:	cpustate->DE.w.l--;											/* DCX  D */
					if (IS_8085(cpustate)) { if (cpustate->DE.w.l == 0xffff) cpustate->AF.b.l |= X5F; else cpustate->AF.b.l &= ~X5F; }
					break;
		case 0x1c:	M_INR(cpustate->DE.b.l);							break;	/* INR  E */
		case 0x1d:	M_DCR(cpustate->DE.b.l);							break;	/* DCR  E */
		case 0x1e:	M_MVI(cpustate->DE.b.l);							break;	/* MVI  E,nn */
		case 0x1f:	M_RAR;												break;	/* RAR  */

		case 0x20:	if (IS_8085(cpustate)) {									/* RIM  */
						cpustate->AF.b.h = get_rim_value(cpustate);

						/* if we have remembered state from taking a TRAP, fix up the IE flag here */
						if (cpustate->trap_im_copy & 0x80) cpustate->AF.b.h = (cpustate->AF.b.h & ~IM_IE) | (cpustate->trap_im_copy & IM_IE);
						cpustate->trap_im_copy = 0;
					} /* else { ; } */											/* NOP  undocumented */
					break;
		case 0x21:	cpustate->HL.w.l = ARG16(cpustate);					break;	/* LXI  H,nnnn */
		case 0x22:	cpustate->WZ.w.l = ARG16(cpustate);							/* SHLD nnnn */
					WM(cpustate, cpustate->WZ.d, cpustate->HL.b.l); cpustate->WZ.w.l++;
					WM(cpustate, cpustate->WZ.d, cpustate->HL.b.h);
					break;
		case 0x23:	cpustate->HL.w.l++;											/* INX  H */
					if (IS_8085(cpustate)) { if (cpustate->HL.w.l == 0x0000) cpustate->AF.b.l |= X5F; else cpustate->AF.b.l &= ~X5F; }
					break;
		case 0x24:	M_INR(cpustate->HL.b.h);							break;	/* INR  H */
		case 0x25:	M_DCR(cpustate->HL.b.h);							break;	/* DCR  H */
		case 0x26:	M_MVI(cpustate->HL.b.h);							break;	/* MVI  H,nn */
		case 0x27:	cpustate->WZ.b.h = cpustate->AF.b.h;						/* DAA  */
					if (cpustate->AF.b.l&VF) {
						if ((cpustate->AF.b.l&HF) | ((cpustate->AF.b.h&0xf)>9)) cpustate->WZ.b.h-=6;
						if ((cpustate->AF.b.l&CF) | (cpustate->AF.b.h>0x99)) cpustate->WZ.b.h-=0x60;
					}
					else {
						if ((cpustate->AF.b.l&HF) | ((cpustate->AF.b.h&0xf)>9)) cpustate->WZ.b.h+=6;
						if ((cpustate->AF.b.l&CF) | (cpustate->AF.b.h>0x99)) cpustate->WZ.b.h+=0x60;
					}

					cpustate->AF.b.l=(cpustate->AF.b.l&3) | (cpustate->AF.b.h&0x28) | (cpustate->AF.b.h>0x99) | ((cpustate->AF.b.h^cpustate->WZ.b.h)&0x10) | ZSP[cpustate->WZ.b.h];
					cpustate->AF.b.h=cpustate->WZ.b.h;

					if (IS_8080(cpustate)) cpustate->AF.b.l &= 0xd5; // Ignore not used flags
					break;

		case 0x28:	if (IS_8085(cpustate)) {									/* LDEH nn */
						cpustate->WZ.d = ARG(cpustate);
						cpustate->DE.d = (cpustate->HL.d + cpustate->WZ.d) & 0xffff;
					} /* else { ; } */											/* NOP  undocumented */
					break;
		case 0x29:	M_DAD(HL);											break;	/* DAD  H */
		case 0x2a:	cpustate->WZ.d = ARG16(cpustate);							/* LHLD nnnn */
					cpustate->HL.b.l = RM(cpustate, cpustate->WZ.d); cpustate->WZ.w.l++;
					cpustate->HL.b.h = RM(cpustate, cpustate->WZ.d);
					break;
		case 0x2b:	cpustate->HL.w.l--;											/* DCX  H */
					if (IS_8085(cpustate)) { if (cpustate->HL.w.l == 0xffff) cpustate->AF.b.l |= X5F; else cpustate->AF.b.l &= ~X5F; }
					break;
		case 0x2c:	M_INR(cpustate->HL.b.l);							break;	/* INR  L */
		case 0x2d:	M_DCR(cpustate->HL.b.l);							break;	/* DCR  L */
		case 0x2e:	M_MVI(cpustate->HL.b.l);							break;	/* MVI  L,nn */
		case 0x2f:	cpustate->AF.b.h ^= 0xff;									/* CMA  */
					if (IS_8085(cpustate)) cpustate->AF.b.l |= HF | VF;
					break;

		case 0x30:	if (IS_8085(cpustate)) {									/* SIM  */
						/* if bit 3 is set, bits 0-2 become the new masks */
						if (cpustate->AF.b.h & 0x08) {
							cpustate->IM &= ~(IM_M55 | IM_M65 | IM_M75 | IM_I55 | IM_I65);
							cpustate->IM |= cpustate->AF.b.h & (IM_M55 | IM_M65 | IM_M75);

							/* update live state based on the new masks */
							if ((cpustate->IM & IM_M55) == 0 && cpustate->irq_state[I8085_RST55_LINE]) cpustate->IM |= IM_I55;
							if ((cpustate->IM & IM_M65) == 0 && cpustate->irq_state[I8085_RST65_LINE]) cpustate->IM |= IM_I65;
						}

						/* bit if 4 is set, the 7.5 flip-flop is cleared */
						if (cpustate->AF.b.h & 0x10) cpustate->IM &= ~IM_I75;

						/* if bit 6 is set, then bit 7 is the new SOD state */
						if (cpustate->AF.b.h & 0x40) set_sod(cpustate, cpustate->AF.b.h >> 7);

						/* check for revealed interrupts */
						check_for_interrupts(cpustate);
					} /* else { ; } */											/* NOP  undocumented */
					break;
		case 0x31:	cpustate->SP.w.l = ARG16(cpustate);					break;	/* LXI  SP,nnnn */
		case 0x32:	cpustate->WZ.d = ARG16(cpustate);							/* STAX nnnn */
					WM(cpustate, cpustate->WZ.d, cpustate->AF.b.h);
					break;
		case 0x33:	cpustate->SP.w.l++;											/* INX  SP */
					if (IS_8085(cpustate)) { if (cpustate->SP.w.l == 0x0000) cpustate->AF.b.l |= X5F; else cpustate->AF.b.l &= ~X5F; }
					break;
		case 0x34:	cpustate->WZ.b.l = RM(cpustate, cpustate->HL.d);			/* INR  M */
					M_INR(cpustate->WZ.b.l);
					WM(cpustate, cpustate->HL.d, cpustate->WZ.b.l);
					break;
		case 0x35:	cpustate->WZ.b.l = RM(cpustate, cpustate->HL.d);			/* DCR  M */
					M_DCR(cpustate->WZ.b.l);
					WM(cpustate, cpustate->HL.d, cpustate->WZ.b.l);
					break;
		case 0x36:	cpustate->WZ.b.l = ARG(cpustate);							/* MVI  M,nn */
					WM(cpustate, cpustate->HL.d, cpustate->WZ.b.l);
					break;
		case 0x37:	cpustate->AF.b.l = (cpustate->AF.b.l & 0xfe) | CF;	break;	/* STC  */

		case 0x38:	if (IS_8085(cpustate)) {									/* LDES nn */
						cpustate->WZ.d = ARG(cpustate);
						cpustate->DE.d = (cpustate->SP.d + cpustate->WZ.d) & 0xffff;
					} /* else { ; } */											/* NOP  undocumented */
					break;
		case 0x39:	M_DAD(SP);											break;	/* DAD  SP */
		case 0x3a:	cpustate->WZ.d = ARG16(cpustate);							/* LDAX nnnn */
					cpustate->AF.b.h = RM(cpustate, cpustate->WZ.d);
					break;
		case 0x3b:	cpustate->SP.w.l--;											/* DCX  SP */
					if (IS_8085(cpustate)) { if (cpustate->SP.w.l == 0xffff) cpustate->AF.b.l |= X5F; else cpustate->AF.b.l &= ~X5F; }
					break;
		case 0x3c:	M_INR(cpustate->AF.b.h);							break;	/* INR  A */
		case 0x3d:	M_DCR(cpustate->AF.b.h);							break;	/* DCR  A */
		case 0x3e:	M_MVI(cpustate->AF.b.h);							break;	/* MVI  A,nn */
		case 0x3f:	cpustate->AF.b.l = (cpustate->AF.b.l & 0xfe) | (~cpustate->AF.b.l & CF); break; /* CMC  */

		case 0x40:														break;	/* MOV  B,B */
		case 0x41:	cpustate->BC.b.h = cpustate->BC.b.l;				break;	/* MOV  B,C */
		case 0x42:	cpustate->BC.b.h = cpustate->DE.b.h;				break;	/* MOV  B,D */
		case 0x43:	cpustate->BC.b.h = cpustate->DE.b.l;				break;	/* MOV  B,E */
		case 0x44:	cpustate->BC.b.h = cpustate->HL.b.h;				break;	/* MOV  B,H */
		case 0x45:	cpustate->BC.b.h = cpustate->HL.b.l;				break;	/* MOV  B,L */
		case 0x46:	cpustate->BC.b.h = RM(cpustate, cpustate->HL.d);	break;	/* MOV  B,M */
		case 0x47:	cpustate->BC.b.h = cpustate->AF.b.h;				break;	/* MOV  B,A */

		case 0x48:	cpustate->BC.b.l = cpustate->BC.b.h;				break;	/* MOV  C,B */
		case 0x49:														break;	/* MOV  C,C */
		case 0x4a:	cpustate->BC.b.l = cpustate->DE.b.h;				break;	/* MOV  C,D */
		case 0x4b:	cpustate->BC.b.l = cpustate->DE.b.l;				break;	/* MOV  C,E */
		case 0x4c:	cpustate->BC.b.l = cpustate->HL.b.h;				break;	/* MOV  C,H */
		case 0x4d:	cpustate->BC.b.l = cpustate->HL.b.l;				break;	/* MOV  C,L */
		case 0x4e:	cpustate->BC.b.l = RM(cpustate, cpustate->HL.d);	break;	/* MOV  C,M */
		case 0x4f:	cpustate->BC.b.l = cpustate->AF.b.h;				break;	/* MOV  C,A */

		case 0x50:	cpustate->DE.b.h = cpustate->BC.b.h;				break;	/* MOV  D,B */
		case 0x51:	cpustate->DE.b.h = cpustate->BC.b.l;				break;	/* MOV  D,C */
		case 0x52:														break;	/* MOV  D,D */
		case 0x53:	cpustate->DE.b.h = cpustate->DE.b.l;				break;	/* MOV  D,E */
		case 0x54:	cpustate->DE.b.h = cpustate->HL.b.h;				break;	/* MOV  D,H */
		case 0x55:	cpustate->DE.b.h = cpustate->HL.b.l;				break;	/* MOV  D,L */
		case 0x56:	cpustate->DE.b.h = RM(cpustate, cpustate->HL.d);	break;	/* MOV  D,M */
		case 0x57:	cpustate->DE.b.h = cpustate->AF.b.h;				break;	/* MOV  D,A */

		case 0x58:	cpustate->DE.b.l = cpustate->BC.b.h;				break;	/* MOV  E,B */
		case 0x59:	cpustate->DE.b.l = cpustate->BC.b.l;				break;	/* MOV  E,C */
		case 0x5a:	cpustate->DE.b.l = cpustate->DE.b.h;				break;	/* MOV  E,D */
		case 0x5b:														break;	/* MOV  E,E */
		case 0x5c:	cpustate->DE.b.l = cpustate->HL.b.h;				break;	/* MOV  E,H */
		case 0x5d:	cpustate->DE.b.l = cpustate->HL.b.l;				break;	/* MOV  E,L */
		case 0x5e:	cpustate->DE.b.l = RM(cpustate, cpustate->HL.d);	break;	/* MOV  E,M */
		case 0x5f:	cpustate->DE.b.l = cpustate->AF.b.h;				break;	/* MOV  E,A */

		case 0x60:	cpustate->HL.b.h = cpustate->BC.b.h;				break;	/* MOV  H,B */
		case 0x61:	cpustate->HL.b.h = cpustate->BC.b.l;				break;	/* MOV  H,C */
		case 0x62:	cpustate->HL.b.h = cpustate->DE.b.h;				break;	/* MOV  H,D */
		case 0x63:	cpustate->HL.b.h = cpustate->DE.b.l;				break;	/* MOV  H,E */
		case 0x64:														break;	/* MOV  H,H */
		case 0x65:	cpustate->HL.b.h = cpustate->HL.b.l;				break;	/* MOV  H,L */
		case 0x66:	cpustate->HL.b.h = RM(cpustate, cpustate->HL.d);	break;	/* MOV  H,M */
		case 0x67:	cpustate->HL.b.h = cpustate->AF.b.h;				break;	/* MOV  H,A */

		case 0x68:	cpustate->HL.b.l = cpustate->BC.b.h;				break;	/* MOV  L,B */
		case 0x69:	cpustate->HL.b.l = cpustate->BC.b.l;				break;	/* MOV  L,C */
		case 0x6a:	cpustate->HL.b.l = cpustate->DE.b.h;				break;	/* MOV  L,D */
		case 0x6b:	cpustate->HL.b.l = cpustate->DE.b.l;				break;	/* MOV  L,E */
		case 0x6c:	cpustate->HL.b.l = cpustate->HL.b.h;				break;	/* MOV  L,H */
		case 0x6d:														break;	/* MOV  L,L */
		case 0x6e:	cpustate->HL.b.l = RM(cpustate, cpustate->HL.d);	break;	/* MOV  L,M */
		case 0x6f:	cpustate->HL.b.l = cpustate->AF.b.h;				break;	/* MOV  L,A */

		case 0x70:	WM(cpustate, cpustate->HL.d, cpustate->BC.b.h);		break;	/* MOV  M,B */
		case 0x71:	WM(cpustate, cpustate->HL.d, cpustate->BC.b.l);		break;	/* MOV  M,C */
		case 0x72:	WM(cpustate, cpustate->HL.d, cpustate->DE.b.h);		break;	/* MOV  M,D */
		case 0x73:	WM(cpustate, cpustate->HL.d, cpustate->DE.b.l);		break;	/* MOV  M,E */
		case 0x74:	WM(cpustate, cpustate->HL.d, cpustate->HL.b.h);		break;	/* MOV  M,H */
		case 0x75:	WM(cpustate, cpustate->HL.d, cpustate->HL.b.l);		break;	/* MOV  M,L */
		case 0x76:	cpustate->PC.w.l--; cpustate->HALT = 1;						/* HLT */
					set_status(cpustate, 0x8a); // halt acknowledge
					break;
		case 0x77:	WM(cpustate, cpustate->HL.d, cpustate->AF.b.h);		break;	/* MOV  M,A */

		case 0x78:	cpustate->AF.b.h = cpustate->BC.b.h;				break;	/* MOV  A,B */
		case 0x79:	cpustate->AF.b.h = cpustate->BC.b.l;				break;	/* MOV  A,C */
		case 0x7a:	cpustate->AF.b.h = cpustate->DE.b.h;				break;	/* MOV  A,D */
		case 0x7b:	cpustate->AF.b.h = cpustate->DE.b.l;				break;	/* MOV  A,E */
		case 0x7c:	cpustate->AF.b.h = cpustate->HL.b.h;				break;	/* MOV  A,H */
		case 0x7d:	cpustate->AF.b.h = cpustate->HL.b.l;				break;	/* MOV  A,L */
		case 0x7e:	cpustate->AF.b.h = RM(cpustate, cpustate->HL.d);	break;	/* MOV  A,M */
		case 0x7f:														break;	/* MOV  A,A */

		case 0x80:	M_ADD(cpustate->BC.b.h);							break;	/* ADD  B */
		case 0x81:	M_ADD(cpustate->BC.b.l);							break;	/* ADD  C */
		case 0x82:	M_ADD(cpustate->DE.b.h);							break;	/* ADD  D */
		case 0x83:	M_ADD(cpustate->DE.b.l);							break;	/* ADD  E */
		case 0x84:	M_ADD(cpustate->HL.b.h);							break;	/* ADD  H */
		case 0x85:	M_ADD(cpustate->HL.b.l);							break;	/* ADD  L */
		case 0x86:	cpustate->WZ.b.l = RM(cpustate, cpustate->HL.d); M_ADD(cpustate->WZ.b.l); break; /* ADD  M */
		case 0x87:	M_ADD(cpustate->AF.b.h);							break;	/* ADD  A */

		case 0x88:	M_ADC(cpustate->BC.b.h);							break;	/* ADC  B */
		case 0x89:	M_ADC(cpustate->BC.b.l);							break;	/* ADC  C */
		case 0x8a:	M_ADC(cpustate->DE.b.h);							break;	/* ADC  D */
		case 0x8b:	M_ADC(cpustate->DE.b.l);							break;	/* ADC  E */
		case 0x8c:	M_ADC(cpustate->HL.b.h);							break;	/* ADC  H */
		case 0x8d:	M_ADC(cpustate->HL.b.l);							break;	/* ADC  L */
		case 0x8e:	cpustate->WZ.b.l = RM(cpustate, cpustate->HL.d); M_ADC(cpustate->WZ.b.l); break; /* ADC  M */
		case 0x8f:	M_ADC(cpustate->AF.b.h);							break;	/* ADC  A */

		case 0x90:	M_SUB(cpustate->BC.b.h);							break;	/* SUB  B */
		case 0x91:	M_SUB(cpustate->BC.b.l);							break;	/* SUB  C */
		case 0x92:	M_SUB(cpustate->DE.b.h);							break;	/* SUB  D */
		case 0x93:	M_SUB(cpustate->DE.b.l);							break;	/* SUB  E */
		case 0x94:	M_SUB(cpustate->HL.b.h);							break;	/* SUB  H */
		case 0x95:	M_SUB(cpustate->HL.b.l);							break;	/* SUB  L */
		case 0x96:	cpustate->WZ.b.l = RM(cpustate, cpustate->HL.d); M_SUB(cpustate->WZ.b.l); break; /* SUB  M */
		case 0x97:	M_SUB(cpustate->AF.b.h);							break;	/* SUB  A */

		case 0x98:	M_SBB(cpustate->BC.b.h);							break;	/* SBB  B */
		case 0x99:	M_SBB(cpustate->BC.b.l);							break;	/* SBB  C */
		case 0x9a:	M_SBB(cpustate->DE.b.h);							break;	/* SBB  D */
		case 0x9b:	M_SBB(cpustate->DE.b.l);							break;	/* SBB  E */
		case 0x9c:	M_SBB(cpustate->HL.b.h);							break;	/* SBB  H */
		case 0x9d:	M_SBB(cpustate->HL.b.l);							break;	/* SBB  L */
		case 0x9e:	cpustate->WZ.b.l = RM(cpustate, cpustate->HL.d); M_SBB(cpustate->WZ.b.l); break; /* SBB  M */
		case 0x9f:	M_SBB(cpustate->AF.b.h);							break;	/* SBB  A */

		case 0xa0:	M_ANA(cpustate->BC.b.h);							break;	/* ANA  B */
		case 0xa1:	M_ANA(cpustate->BC.b.l);							break;	/* ANA  C */
		case 0xa2:	M_ANA(cpustate->DE.b.h);							break;	/* ANA  D */
		case 0xa3:	M_ANA(cpustate->DE.b.l);							break;	/* ANA  E */
		case 0xa4:	M_ANA(cpustate->HL.b.h);							break;	/* ANA  H */
		case 0xa5:	M_ANA(cpustate->HL.b.l);							break;	/* ANA  L */
		case 0xa6:	cpustate->WZ.b.l = RM(cpustate, cpustate->HL.d); M_ANA(cpustate->WZ.b.l); break; /* ANA  M */
		case 0xa7:	M_ANA(cpustate->AF.b.h);							break;	/* ANA  A */

		case 0xa8:	M_XRA(cpustate->BC.b.h);							break;	/* XRA  B */
		case 0xa9:	M_XRA(cpustate->BC.b.l);							break;	/* XRA  C */
		case 0xaa:	M_XRA(cpustate->DE.b.h);							break;	/* XRA  D */
		case 0xab:	M_XRA(cpustate->DE.b.l);							break;	/* XRA  E */
		case 0xac:	M_XRA(cpustate->HL.b.h);							break;	/* XRA  H */
		case 0xad:	M_XRA(cpustate->HL.b.l);							break;	/* XRA  L */
		case 0xae:	cpustate->WZ.b.l = RM(cpustate, cpustate->HL.d); M_XRA(cpustate->WZ.b.l); break; /* XRA  M */
		case 0xaf:	M_XRA(cpustate->AF.b.h);							break;	/* XRA  A */

		case 0xb0:	M_ORA(cpustate->BC.b.h);							break;	/* ORA  B */
		case 0xb1:	M_ORA(cpustate->BC.b.l);							break;	/* ORA  C */
		case 0xb2:	M_ORA(cpustate->DE.b.h);							break;	/* ORA  D */
		case 0xb3:	M_ORA(cpustate->DE.b.l);							break;	/* ORA  E */
		case 0xb4:	M_ORA(cpustate->HL.b.h);							break;	/* ORA  H */
		case 0xb5:	M_ORA(cpustate->HL.b.l);							break;	/* ORA  L */
		case 0xb6:	cpustate->WZ.b.l = RM(cpustate, cpustate->HL.d); M_ORA(cpustate->WZ.b.l); break; /* ORA  M */
		case 0xb7:	M_ORA(cpustate->AF.b.h);							break;	/* ORA  A */

		case 0xb8:	M_CMP(cpustate->BC.b.h);							break;	/* CMP  B */
		case 0xb9:	M_CMP(cpustate->BC.b.l);							break;	/* CMP  C */
		case 0xba:	M_CMP(cpustate->DE.b.h);							break;	/* CMP  D */
		case 0xbb:	M_CMP(cpustate->DE.b.l);							break;	/* CMP  E */
		case 0xbc:	M_CMP(cpustate->HL.b.h);							break;	/* CMP  H */
		case 0xbd:	M_CMP(cpustate->HL.b.l);							break;	/* CMP  L */
		case 0xbe:	cpustate->WZ.b.l = RM(cpustate, cpustate->HL.d); M_CMP(cpustate->WZ.b.l); break; /* CMP  M */
		case 0xbf:	M_CMP(cpustate->AF.b.h);							break;	/* CMP  A */

		case 0xc0:	M_RET( !(cpustate->AF.b.l & ZF) );					break;	/* RNZ  */
		case 0xc1:	M_POP(BC);											break;	/* POP  B */
		case 0xc2:	M_JMP( !(cpustate->AF.b.l & ZF) );					break;	/* JNZ  nnnn */
		case 0xc3:	M_JMP(1);											break;	/* JMP  nnnn */
		case 0xc4:	M_CALL( !(cpustate->AF.b.l & ZF) );					break;	/* CNZ  nnnn */
		case 0xc5:	M_PUSH(BC);											break;	/* PUSH B */
		case 0xc6:	cpustate->WZ.b.l = ARG(cpustate); M_ADD(cpustate->WZ.b.l); break; /* ADI  nn */
		case 0xc7:	M_RST(0);											break;	/* RST  0 */

		case 0xc8:	M_RET( cpustate->AF.b.l & ZF );						break;	/* RZ   */
		case 0xc9:	M_POP(PC);											break;	/* RET  */
		case 0xca:	M_JMP( cpustate->AF.b.l & ZF );						break;	/* JZ   nnnn */
		case 0xcb:	if (IS_8085(cpustate)) {									/* RST  V */
						if (cpustate->AF.b.l & VF) { M_RST(8); }
						else cpustate->icount += 6; // RST not taken
					} else { M_JMP(1); }										/* JMP  nnnn undocumented */
					break;
		case 0xcc:	M_CALL( cpustate->AF.b.l & ZF );					break;	/* CZ   nnnn */
		case 0xcd:	M_CALL(1);											break;	/* CALL nnnn */
		case 0xce:	cpustate->WZ.b.l = ARG(cpustate); M_ADC(cpustate->WZ.b.l); break; /* ACI  nn */
		case 0xcf:	M_RST(1);											break;	/* RST  1 */

		case 0xd0:	M_RET( !(cpustate->AF.b.l & CF) );					break;	/* RNC  */
		case 0xd1:	M_POP(DE);											break;	/* POP  D */
		case 0xd2:	M_JMP( !(cpustate->AF.b.l & CF) );					break;	/* JNC  nnnn */
		case 0xd3:	M_OUT;												break;	/* OUT  nn */
		case 0xd4:	M_CALL( !(cpustate->AF.b.l & CF) );					break;	/* CNC  nnnn */
		case 0xd5:	M_PUSH(DE);											break;	/* PUSH D */
		case 0xd6:	cpustate->WZ.b.l = ARG(cpustate); M_SUB(cpustate->WZ.b.l); break; /* SUI  nn */
		case 0xd7:	M_RST(2);											break;	/* RST  2 */

		case 0xd8:	M_RET( cpustate->AF.b.l & CF );						break;	/* RC   */
		case 0xd9:	if (IS_8085(cpustate)) {									/* SHLX */
						cpustate->WZ.w.l = cpustate->DE.w.l;
						WM(cpustate, cpustate->WZ.d, cpustate->HL.b.l); cpustate->WZ.w.l++;
						WM(cpustate, cpustate->WZ.d, cpustate->HL.b.h);
					} else { M_POP(PC); }										/* RET  undocumented */
					break;
		case 0xda:	M_JMP( cpustate->AF.b.l & CF );						break;	/* JC   nnnn */
		case 0xdb:	M_IN;												break;	/* IN   nn */
		case 0xdc:	M_CALL( cpustate->AF.b.l & CF );					break;	/* CC   nnnn */
		case 0xdd:	if (IS_8085(cpustate)) { M_JMP( !(cpustate->AF.b.l & X5F) ); } /* JNX  nnnn */
					else { M_CALL(1); }											/* CALL nnnn undocumented */
					break;
		case 0xde:	cpustate->WZ.b.l = ARG(cpustate); M_SBB(cpustate->WZ.b.l); break; /* SBI  nn */
		case 0xdf:	M_RST(3);											break;	/* RST  3 */

		case 0xe0:	M_RET( !(cpustate->AF.b.l & PF) );					break;	/* RPO    */
		case 0xe1:	M_POP(HL);											break;	/* POP  H */
		case 0xe2:	M_JMP( !(cpustate->AF.b.l & PF) );					break;	/* JPO  nnnn */
		case 0xe3:	M_POP(WZ); M_PUSH(HL);										/* XTHL */
					cpustate->HL.d = cpustate->WZ.d;
					break;
		case 0xe4:	M_CALL( !(cpustate->AF.b.l & PF) );					break;	/* CPO  nnnn */
		case 0xe5:	M_PUSH(HL);											break;	/* PUSH H */
		case 0xe6:	cpustate->WZ.b.l = ARG(cpustate); M_ANA(cpustate->WZ.b.l); break; /* ANI  nn */
		case 0xe7:	M_RST(4);											break;	/* RST  4 */

		case 0xe8:	M_RET( cpustate->AF.b.l & PF );						break;	/* RPE  */
		case 0xe9:	cpustate->PC.d = cpustate->HL.w.l;					break;	/* PCHL */
		case 0xea:	M_JMP( cpustate->AF.b.l & PF );						break;	/* JPE  nnnn */
		case 0xeb:	cpustate->WZ.d = cpustate->DE.d;							/* XCHG */
					cpustate->DE.d = cpustate->HL.d;
					cpustate->HL.d = cpustate->WZ.d;
					break;
		case 0xec:	M_CALL( cpustate->AF.b.l & PF );					break;	/* CPE  nnnn */
		case 0xed:	if (IS_8085(cpustate)) {									/* LHLX */
						cpustate->WZ.w.l = cpustate->DE.w.l;
						cpustate->HL.b.l = RM(cpustate, cpustate->WZ.d); cpustate->WZ.w.l++;
						cpustate->HL.b.h = RM(cpustate, cpustate->WZ.d);
					} else { M_CALL(1); }										/* CALL nnnn undocumented */
					break;
		case 0xee:	cpustate->WZ.b.l = ARG(cpustate); M_XRA(cpustate->WZ.b.l); break; /* XRI  nn */
		case 0xef:	M_RST(5);											break;	/* RST  5 */

		case 0xf0:	M_RET( !(cpustate->AF.b.l&SF) );					break;	/* RP   */
		case 0xf1:	M_POP(AF);											break;	/* POP  A */
		case 0xf2:	M_JMP( !(cpustate->AF.b.l & SF) );					break;	/* JP   nnnn */
		case 0xf3:	set_inte(cpustate, 0);								break;	/* DI   */
		case 0xf4:	M_CALL( !(cpustate->AF.b.l & SF) );					break;	/* CP   nnnn */
		case 0xf5:	M_PUSH(AF);											break;	/* PUSH A */
		case 0xf6:	cpustate->WZ.b.l = ARG(cpustate); M_ORA(cpustate->WZ.b.l); break; /* ORI  nn */
		case 0xf7:	M_RST(6);											break;	/* RST  6 */

		case 0xf8:	M_RET( cpustate->AF.b.l & SF );						break;	/* RM   */
		case 0xf9:	cpustate->SP.d = cpustate->HL.d;					break;	/* SPHL */
		case 0xfa:	M_JMP( cpustate->AF.b.l & SF );						break;	/* JM   nnnn */
		case 0xfb:	set_inte(cpustate, 1); cpustate->after_ei = 2;		break;	/* EI */
		case 0xfc:	M_CALL( cpustate->AF.b.l & SF );					break;	/* CM   nnnn */
		case 0xfd:	if (IS_8085(cpustate)) { M_JMP( cpustate->AF.b.l & X5F ); }	/* JX   nnnn */
					else { M_CALL(1); }											/* CALL nnnn undocumented */
					break;
		case 0xfe:	cpustate->WZ.b.l = ARG(cpustate); M_CMP(cpustate->WZ.b.l); break; /* CPI  nn */
		case 0xff:	M_RST(7);											break;	/* RST  7 */
	}
}


/***************************************************************************
    COMMON EXECUTION
***************************************************************************/

static CPU_EXECUTE( i808x )
{
	i8085_state *cpustate = get_safe_token(device);

	/* check for TRAPs before diving in (can't do others because of after_ei) */
	if (cpustate->trap_pending || cpustate->after_ei == 0)
		check_for_interrupts(cpustate);

	do
	{
		debugger_instruction_hook(device, cpustate->PC.d);

		/* the instruction after an EI does not take an interrupt, so
           we cannot check immediately; handle post-EI behavior here */
		if (cpustate->after_ei != 0 && --cpustate->after_ei == 0)
			check_for_interrupts(cpustate);

		/* here we go... */
		execute_one(cpustate, ROP(cpustate));

	} while (cpustate->icount > 0);
}



/***************************************************************************
    CORE INITIALIZATION
***************************************************************************/

static void init_tables (int type)
{
	UINT8 zs;
	int i, p;
	for (i = 0; i < 256; i++)
	{
		/* cycles */
		lut_cycles[i] = type?lut_cycles_8085[i]:lut_cycles_8080[i];

		/* flags */
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
		ZSP[i] = zs | ((p&1) ? 0 : PF);
	}
}


static void init_808x_common(legacy_cpu_device *device, device_irq_acknowledge_callback irqcallback, int type)
{
	i8085_state *cpustate = get_safe_token(device);

	init_tables(type);

	/* set up the state table */
	{
		device_state_interface *state;
		device->interface(state);
		state->state_add(I8085_PC,     "PC",     cpustate->PC.w.l);
		state->state_add(STATE_GENPC,  "GENPC",  cpustate->PC.w.l).noshow();
		state->state_add(I8085_SP,     "SP",     cpustate->SP.w.l);
		state->state_add(STATE_GENSP,  "GENSP",  cpustate->SP.w.l).noshow();
		state->state_add(STATE_GENFLAGS, "GENFLAGS", cpustate->AF.b.l).noshow().formatstr("%8s");
		state->state_add(I8085_A,      "A",      cpustate->AF.b.h).noshow();
		state->state_add(I8085_B,      "B",      cpustate->BC.b.h).noshow();
		state->state_add(I8085_C,      "C",      cpustate->BC.b.l).noshow();
		state->state_add(I8085_D,      "D",      cpustate->DE.b.h).noshow();
		state->state_add(I8085_E,      "E",      cpustate->DE.b.l).noshow();
		state->state_add(I8085_F,      "F",      cpustate->AF.b.l).noshow();
		state->state_add(I8085_H,      "H",      cpustate->HL.b.h).noshow();
		state->state_add(I8085_L,      "L",      cpustate->HL.b.l).noshow();
		state->state_add(I8085_AF,     "AF",     cpustate->AF.w.l);
		state->state_add(I8085_BC,     "BC",     cpustate->BC.w.l);
		state->state_add(I8085_DE,     "DE",     cpustate->DE.w.l);
		state->state_add(I8085_HL,     "HL",     cpustate->HL.w.l);
		state->state_add(I8085_STATUS, "STATUS", cpustate->STATUS);
		state->state_add(I8085_SOD,    "SOD",    cpustate->sod_state).mask(0x1);
		state->state_add(I8085_SID,    "SID",    cpustate->ietemp).mask(0x1).callimport().callexport();
		state->state_add(I8085_INTE,   "INTE",   cpustate->ietemp).mask(0x1).callimport().callexport();
	}

	if (device->static_config() != NULL)
		cpustate->config = *(i8085_config *)device->static_config();
	cpustate->cputype = type;
	cpustate->irq_callback = irqcallback;
	cpustate->device = device;

	cpustate->program = device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();
	cpustate->io = device->space(AS_IO);

	/* resolve callbacks */
	cpustate->out_status_func.resolve(cpustate->config.out_status_func, *device);
	cpustate->out_inte_func.resolve(cpustate->config.out_inte_func, *device);
	cpustate->in_sid_func.resolve(cpustate->config.in_sid_func, *device);
	cpustate->out_sod_func.resolve(cpustate->config.out_sod_func, *device);

	/* register for state saving */
	device->save_item(NAME(cpustate->PC.w.l));
	device->save_item(NAME(cpustate->SP.w.l));
	device->save_item(NAME(cpustate->AF.w.l));
	device->save_item(NAME(cpustate->BC.w.l));
	device->save_item(NAME(cpustate->DE.w.l));
	device->save_item(NAME(cpustate->HL.w.l));
	device->save_item(NAME(cpustate->HALT));
	device->save_item(NAME(cpustate->IM));
	device->save_item(NAME(cpustate->STATUS));
	device->save_item(NAME(cpustate->after_ei));
	device->save_item(NAME(cpustate->nmi_state));
	device->save_item(NAME(cpustate->irq_state));
	device->save_item(NAME(cpustate->trap_pending));
	device->save_item(NAME(cpustate->trap_im_copy));
	device->save_item(NAME(cpustate->sod_state));
}

static CPU_INIT( i8080 )
{
	init_808x_common(device, irqcallback, CPUTYPE_8080);
}

static CPU_INIT( i8085 )
{
	init_808x_common(device, irqcallback, CPUTYPE_8085);
}



/***************************************************************************
    COMMON RESET
***************************************************************************/

static CPU_RESET( i808x )
{
	i8085_state *cpustate = get_safe_token(device);

	cpustate->PC.d = 0;
	cpustate->HALT = 0;
	cpustate->IM &= ~IM_I75;
	cpustate->IM |= IM_M55 | IM_M65 | IM_M75;
	cpustate->after_ei = FALSE;
	cpustate->trap_pending = FALSE;
	cpustate->trap_im_copy = 0;
	set_inte(cpustate, 0);
	set_sod(cpustate, 0);
}



/***************************************************************************
    COMMON STATE IMPORT/EXPORT
***************************************************************************/

static CPU_IMPORT_STATE( i808x )
{
	i8085_state *cpustate = get_safe_token(device);

	switch (entry.index())
	{
		case I8085_SID:
			if (cpustate->ietemp)
				cpustate->IM |= IM_SID;
			else
				cpustate->IM &= ~IM_SID;
			break;

		case I8085_INTE:
			if (cpustate->ietemp)
				cpustate->IM |= IM_IE;
			else
				cpustate->IM &= ~IM_IE;
			break;

		default:
			fatalerror("CPU_IMPORT_STATE(i808x) called for unexpected value\n");
			break;
	}
}


static CPU_EXPORT_STATE( i808x )
{
	i8085_state *cpustate = get_safe_token(device);

	switch (entry.index())
	{
		case I8085_SID:
			{
			int sid = cpustate->in_sid_func();

			cpustate->ietemp = ((cpustate->IM & IM_SID) != 0);
			cpustate->ietemp = (sid != 0);
			}
			break;

		case I8085_INTE:
			cpustate->ietemp = ((cpustate->IM & IM_IE) != 0);
			break;

		default:
			fatalerror("CPU_EXPORT_STATE(i808x) called for unexpected value\n");
			break;
	}
}

static CPU_EXPORT_STRING( i808x )
{
	i8085_state *cpustate = get_safe_token(device);

	switch (entry.index())
	{
		case STATE_GENFLAGS:
			string.printf("%c%c%c%c%c%c%c%c",
				cpustate->AF.b.l & 0x80 ? 'S':'.',
				cpustate->AF.b.l & 0x40 ? 'Z':'.',
				cpustate->AF.b.l & 0x20 ? 'X':'.', // X5
				cpustate->AF.b.l & 0x10 ? 'H':'.',
				cpustate->AF.b.l & 0x08 ? '?':'.',
				cpustate->AF.b.l & 0x04 ? 'P':'.',
				cpustate->AF.b.l & 0x02 ? 'V':'.',
				cpustate->AF.b.l & 0x01 ? 'C':'.');
			break;
	}
}



/***************************************************************************
    COMMON SET INFO
***************************************************************************/

static void i808x_set_irq_line(i8085_state *cpustate, int irqline, int state)
{
	int newstate = (state != CLEAR_LINE);

	/* NMI is edge-triggered */
	if (irqline == INPUT_LINE_NMI)
	{
		if (!cpustate->nmi_state && newstate)
			cpustate->trap_pending = TRUE;
		cpustate->nmi_state = newstate;
	}

	/* RST7.5 is edge-triggered */
	else if (irqline == I8085_RST75_LINE)
	{
		if (!cpustate->irq_state[I8085_RST75_LINE] && newstate)
			cpustate->IM |= IM_I75;
		cpustate->irq_state[I8085_RST75_LINE] = newstate;
	}

	/* remaining sources are level triggered */
	else if (irqline < ARRAY_LENGTH(cpustate->irq_state))
		cpustate->irq_state[irqline] = state;
}


static CPU_SET_INFO( i808x )
{
	i8085_state *cpustate = get_safe_token(device);
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + I8085_INTR_LINE:
		case CPUINFO_INT_INPUT_STATE + I8085_RST55_LINE:
		case CPUINFO_INT_INPUT_STATE + I8085_RST65_LINE:
		case CPUINFO_INT_INPUT_STATE + I8085_RST75_LINE:
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:
			i808x_set_irq_line(cpustate, state - CPUINFO_INT_INPUT_STATE, info->i);
			break;
	}
}



/***************************************************************************
    8085/COMMON GET INFO
***************************************************************************/

CPU_GET_INFO( i8085 )
{
	i8085_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(i8085_state);			break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 4;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0xff;							break;
		case DEVINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;			break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 2;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 3;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 4;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 16;							break;

		case DEVINFO_INT_DATABUS_WIDTH + AS_PROGRAM:			info->i = 8;							break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:		info->i = 16;							break;
		case DEVINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM:		info->i = 0;							break;
		case DEVINFO_INT_DATABUS_WIDTH + AS_IO:				info->i = 8;							break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_IO:				info->i = 8;							break;
		case DEVINFO_INT_ADDRBUS_SHIFT + AS_IO:				info->i = 0;							break;

		/* --- the following bits of info are returned as pointers to functions --- */
		case CPUINFO_FCT_SET_INFO:		info->setinfo = CPU_SET_INFO_NAME(i808x);				break;
		case CPUINFO_FCT_INIT:			info->init = CPU_INIT_NAME(i8085);						break;
		case CPUINFO_FCT_RESET:			info->reset = CPU_RESET_NAME(i808x);					break;
		case CPUINFO_FCT_EXECUTE:		info->execute = CPU_EXECUTE_NAME(i808x);				break;
		case CPUINFO_FCT_DISASSEMBLE:	info->disassemble = CPU_DISASSEMBLE_NAME(i8085);		break;
		case CPUINFO_FCT_IMPORT_STATE:	info->import_state = CPU_IMPORT_STATE_NAME(i808x);		break;
		case CPUINFO_FCT_EXPORT_STATE:	info->export_state = CPU_EXPORT_STATE_NAME(i808x);		break;
		case CPUINFO_FCT_EXPORT_STRING:	info->export_string = CPU_EXPORT_STRING_NAME(i808x);	break;

		/* --- the following bits of info are returned as pointers --- */
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "8085A");				break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "MCS-85");				break;
		case DEVINFO_STR_VERSION:						strcpy(info->s, "1.1");					break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);				break;
		case DEVINFO_STR_CREDITS:						strcpy(info->s, "Copyright Juergen Buchmueller, all rights reserved."); break;
	}
}


/***************************************************************************
    8080-SPECIFIC GET INFO
***************************************************************************/

CPU_GET_INFO( i8080 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 1;							break;

		/* --- the following bits of info are returned as pointers to functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(i8080);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "8080");				break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "MCS-80");				break;

		default:										CPU_GET_INFO_CALL(i8085); break;
	}
}


/***************************************************************************
    8080A-SPECIFIC GET INFO
***************************************************************************/

CPU_GET_INFO( i8080a )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 1;							break;

		/* --- the following bits of info are returned as pointers to functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(i8080);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "8080A");				break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "MCS-80");				break;

		default:										CPU_GET_INFO_CALL(i8085); break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(I8080, i8080);
DEFINE_LEGACY_CPU_DEVICE(I8080A, i8080a);
DEFINE_LEGACY_CPU_DEVICE(I8085A, i8085);
