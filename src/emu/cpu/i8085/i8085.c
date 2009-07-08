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

#include "debugger.h"
#include "i8085.h"
#include "i8085cpu.h"
#include "i8085daa.h"

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
	i8085_config 		config;
	int 				cputype;		/* 0 8080, 1 8085A */
	PAIR				PC,SP,AF,BC,DE,HL,XX;
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

	cpu_irq_callback 	irq_callback;
	const device_config *device;
	const address_space *program;
	const address_space *io;
	cpu_state_table 	state;
	int					icount;
};



/***************************************************************************
    CPU STATE DESCRIPTION
***************************************************************************/

#define I8085_STATE_ENTRY(_name, _format, _member, _datamask, _flags) \
	CPU_STATE_ENTRY(I8085_##_name, #_name, _format, i8085_state, _member, _datamask, ~0, _flags)

static const cpu_state_entry state_array[] =
{
	I8085_STATE_ENTRY(PC,  "%04X", PC.w.l, 0xffff, 0)
	I8085_STATE_ENTRY(GENPC, "%04X", PC.w.l, 0xffff, CPUSTATE_NOSHOW)
//  I8085_STATE_ENTRY(GENPCBASE, "%04X", prvpc.w.l, 0xffff, CPUSTATE_NOSHOW)

	I8085_STATE_ENTRY(SP,  "%04X", SP.w.l, 0xffff, 0)
	I8085_STATE_ENTRY(GENSP, "%04X", SP.w.l, 0xffff, CPUSTATE_NOSHOW)

	I8085_STATE_ENTRY(A, "%02X", AF.b.l, 0xff, CPUSTATE_NOSHOW)
	I8085_STATE_ENTRY(B, "%02X", BC.b.h, 0xff, CPUSTATE_NOSHOW)
	I8085_STATE_ENTRY(C, "%02X", BC.b.l, 0xff, CPUSTATE_NOSHOW)
	I8085_STATE_ENTRY(D, "%02X", DE.b.h, 0xff, CPUSTATE_NOSHOW)
	I8085_STATE_ENTRY(E, "%02X", DE.b.l, 0xff, CPUSTATE_NOSHOW)
	I8085_STATE_ENTRY(H, "%02X", HL.b.h, 0xff, CPUSTATE_NOSHOW)
	I8085_STATE_ENTRY(L, "%02X", HL.b.l, 0xff, CPUSTATE_NOSHOW)

	I8085_STATE_ENTRY(AF, "%04X", AF.w.l, 0xffff, 0)
	I8085_STATE_ENTRY(BC, "%04X", BC.w.l, 0xffff, 0)
	I8085_STATE_ENTRY(DE, "%04X", DE.w.l, 0xffff, 0)
	I8085_STATE_ENTRY(HL, "%04X", HL.w.l, 0xffff, 0)

	I8085_STATE_ENTRY(STATUS, "%02X", STATUS, 0xff, 0)
	I8085_STATE_ENTRY(SOD, "%1u", sod_state, 0x1, 0)
	I8085_STATE_ENTRY(SID, "%1u", ietemp, 0x1, CPUSTATE_EXPORT | CPUSTATE_IMPORT)
	I8085_STATE_ENTRY(INTE, "%1u", ietemp, 0x1, CPUSTATE_EXPORT | CPUSTATE_IMPORT)
};

static const cpu_state_table state_table_template =
{
	NULL,						/* pointer to the base of state (offsets are relative to this) */
	0,							/* subtype this table refers to */
	ARRAY_LENGTH(state_array),	/* number of entries */
	state_array					/* array of entries */
};



/***************************************************************************
    MACROS
***************************************************************************/

#define IS_8080(c)			((c)->cputype == CPUTYPE_8080)
#define IS_8085(c)			((c)->cputype == CPUTYPE_8085)



/***************************************************************************
    STATIC TABLES
***************************************************************************/

static UINT8 ZS[256];
static UINT8 ZSP[256];



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void execute_one(i8085_state *cpustate, int opcode);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE i8085_state *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == CPU);
	assert(cpu_get_type(device) == CPU_8080 ||
		   cpu_get_type(device) == CPU_8085A);
	return (i8085_state *)device->token;
}

INLINE void set_sod(i8085_state *cpustate, int state)
{
	if (state != 0 && cpustate->sod_state == 0)
	{
		cpustate->sod_state = 1;
		if (cpustate->config.sod != NULL)
			(*cpustate->config.sod)(cpustate->device, 1);
	}
	else if (state == 0 && cpustate->sod_state != 0)
	{
		cpustate->sod_state = 0;
		if (cpustate->config.sod != NULL)
			(*cpustate->config.sod)(cpustate->device, 0);
	}
}


INLINE void set_inte(i8085_state *cpustate, int state)
{
	if (state != 0 && (cpustate->IM & IM_IE) == 0)
	{
		cpustate->IM |= IM_IE;
		if (cpustate->config.inte != NULL)
			(*cpustate->config.inte)(cpustate->device, 1);
	}
	else if (state == 0 && (cpustate->IM & IM_IE) != 0)
	{
		cpustate->IM &= ~IM_IE;
		if (cpustate->config.inte != NULL)
			(*cpustate->config.inte)(cpustate->device, 0);
	}
}


INLINE void set_status(i8085_state *cpustate, UINT8 status)
{
	if (status != cpustate->STATUS && cpustate->config.status != NULL)
		(*cpustate->config.status)(cpustate->device, status);
	cpustate->STATUS = status;
}


INLINE UINT8 get_rim_value(i8085_state *cpustate)
{
	UINT8 result = cpustate->IM;

	/* copy live RST5.5 and RST6.5 states */
	result &= ~(IM_I65 | IM_I55);
	if (cpustate->irq_state[I8085_RST65_LINE] && !(cpustate->IM & IM_M65))
		result |= IM_I65;
	if (cpustate->irq_state[I8085_RST55_LINE] && !(cpustate->IM & IM_M55))
		result |= IM_I55;

	/* fetch the SID bit if we have a callback */
	if (cpustate->config.sid != NULL)
		result = (result & 0x7f) | ((*cpustate->config.sid)(cpustate->device) ? 0x80 : 0);
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
	return memory_decrypted_read_byte(cpustate->program, cpustate->PC.w.l++);
}

INLINE UINT8 ARG(i8085_state *cpustate)
{
	return memory_raw_read_byte(cpustate->program, cpustate->PC.w.l++);
}

INLINE UINT16 ARG16(i8085_state *cpustate)
{
	UINT16 w;
	w  = memory_raw_read_byte(cpustate->program, cpustate->PC.d);
	cpustate->PC.w.l++;
	w += memory_raw_read_byte(cpustate->program, cpustate->PC.d) << 8;
	cpustate->PC.w.l++;
	return w;
}

INLINE UINT8 RM(i8085_state *cpustate, UINT32 a)
{
	set_status(cpustate, 0x82); // memory read
	return memory_read_byte_8le(cpustate->program, a);
}

INLINE void WM(i8085_state *cpustate, UINT32 a, UINT8 v)
{
	set_status(cpustate, 0x00); // memory write
	memory_write_byte_8le(cpustate->program, a, v);
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
		cpustate->IM &= ~IM_IE;
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
		cpustate->IM &= ~IM_IE;
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
		cpustate->IM &= ~IM_IE;
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
		cpustate->IM &= ~IM_IE;
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
		cpustate->IM &= ~IM_IE;
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
		case 0x03: cpustate->icount -= IS_8085(cpustate) ? 6 : 5;	/* INX  B */
			cpustate->BC.w.l++;
			if (IS_8085(cpustate))
			{
				if (cpustate->BC.w.l == 0x0000) cpustate->AF.b.l |= XF; else cpustate->AF.b.l &= ~XF;
			}
			break;
		case 0x04: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* INR  B */
			M_INR(cpustate->BC.b.h);
			break;
		case 0x05: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* DCR  B */
			M_DCR(cpustate->BC.b.h);
			break;
		case 0x06: cpustate->icount -= 7;	/* MVI  B,nn */
			M_MVI(cpustate->BC.b.h);
			break;
		case 0x07: cpustate->icount -= 4;	/* RLC  */
			M_RLC;
			break;

		case 0x08:
			if (IS_8085(cpustate)) {
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
		case 0x0b: cpustate->icount -= IS_8085(cpustate) ? 6 : 5;	/* DCX  B */
			cpustate->BC.w.l--;
			if (IS_8085(cpustate))
			{
				if (cpustate->BC.w.l == 0xffff) cpustate->AF.b.l |= XF; else cpustate->AF.b.l &= ~XF;
			}
			break;
		case 0x0c: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* INR  C */
			M_INR(cpustate->BC.b.l);
			break;
		case 0x0d: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* DCR  C */
			M_DCR(cpustate->BC.b.l);
			break;
		case 0x0e: cpustate->icount -= 7;	/* MVI  C,nn */
			M_MVI(cpustate->BC.b.l);
			break;
		case 0x0f: cpustate->icount -= 4;	/* RRC  */
			M_RRC;
			break;

		case 0x10:
			if (IS_8085(cpustate)) {
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
		case 0x13: cpustate->icount -= IS_8085(cpustate) ? 6 : 5;	/* INX  D */
			cpustate->DE.w.l++;
			if (IS_8085(cpustate))
			{
				if (cpustate->DE.w.l == 0x0000) cpustate->AF.b.l |= XF; else cpustate->AF.b.l &= ~XF;
			}
			break;
		case 0x14: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* INR  D */
			M_INR(cpustate->DE.b.h);
			break;
		case 0x15: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* DCR  D */
			M_DCR(cpustate->DE.b.h);
			break;
		case 0x16: cpustate->icount -= 7;	/* MVI  D,nn */
			M_MVI(cpustate->DE.b.h);
			break;
		case 0x17: cpustate->icount -= 4;	/* RAL  */
			M_RAL;
			break;

		case 0x18:
			if (IS_8085(cpustate)) {
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
		case 0x1b: cpustate->icount -= IS_8085(cpustate) ? 6 : 5;	/* DCX  D */
			cpustate->DE.w.l--;
			if (IS_8085(cpustate))
			{
				if (cpustate->DE.w.l == 0xffff) cpustate->AF.b.l |= XF; else cpustate->AF.b.l &= ~XF;
			}
			break;
		case 0x1c: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* INR  E */
			M_INR(cpustate->DE.b.l);
			break;
		case 0x1d: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* DCR  E */
			M_DCR(cpustate->DE.b.l);
			break;
		case 0x1e: cpustate->icount -= 7;	/* MVI  E,nn */
			M_MVI(cpustate->DE.b.l);
			break;
		case 0x1f: cpustate->icount -= 4;	/* RAR  */
			M_RAR;
			break;

		case 0x20:
			if (IS_8085(cpustate)) {
				cpustate->icount -= 7;		/* RIM  */
				cpustate->AF.b.h = get_rim_value(cpustate);

				/* if we have remembered state from taking a TRAP, fix up the IE flag here */
				if (cpustate->trap_im_copy & 0x80)
					cpustate->AF.b.h = (cpustate->AF.b.h & ~IM_IE) | (cpustate->trap_im_copy & IM_IE);
				cpustate->trap_im_copy = 0;
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
		case 0x23: cpustate->icount -= IS_8085(cpustate) ? 6 : 5;	/* INX  H */
			cpustate->HL.w.l++;
			if (IS_8085(cpustate))
			{
				if (cpustate->HL.w.l == 0x0000) cpustate->AF.b.l |= XF; else cpustate->AF.b.l &= ~XF;
			}
			break;
		case 0x24: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* INR  H */
			M_INR(cpustate->HL.b.h);
			break;
		case 0x25: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* DCR  H */
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
			if (IS_8080(cpustate))
			{
				cpustate->AF.b.l &= 0xd5; // Ignore not used flags
			}
			break;

		case 0x28:
			if (IS_8085(cpustate)) {
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
		case 0x2b: cpustate->icount -= IS_8085(cpustate) ? 6 : 5;	/* DCX  H */
			cpustate->HL.w.l--;
			if (IS_8085(cpustate))
			{
				if (cpustate->HL.w.l == 0xffff) cpustate->AF.b.l |= XF; else cpustate->AF.b.l &= ~XF;
			}
			break;
		case 0x2c: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* INR  L */
			M_INR(cpustate->HL.b.l);
			break;
		case 0x2d: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* DCR  L */
			M_DCR(cpustate->HL.b.l);
			break;
		case 0x2e: cpustate->icount -= 7;	/* MVI  L,nn */
			M_MVI(cpustate->HL.b.l);
			break;
		case 0x2f: cpustate->icount -= 4;	/* CMA  */
			if (IS_8085(cpustate))
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
			if (IS_8085(cpustate))
			{
				cpustate->icount -= 7;		/* SIM  */

				/* if bit 3 is set, bits 0-2 become the new masks */
				if (cpustate->AF.b.h & 0x08)
				{
					cpustate->IM &= ~(IM_M55 | IM_M65 | IM_M75 | IM_I55 | IM_I65);
					cpustate->IM |= cpustate->AF.b.h & (IM_M55 | IM_M65 | IM_M75);

					/* update live state based on the new masks */
					if ((cpustate->IM & IM_M55) == 0 && cpustate->irq_state[I8085_RST55_LINE])
						cpustate->IM |= IM_I55;
					if ((cpustate->IM & IM_M65) == 0 && cpustate->irq_state[I8085_RST65_LINE])
						cpustate->IM |= IM_I65;
				}

				/* bit if 4 is set, the 7.5 flip-flop is cleared */
				if (cpustate->AF.b.h & 0x10)
					cpustate->IM &= ~IM_I75;

				/* if bit 6 is set, then bit 7 is the new SOD state */
				if (cpustate->AF.b.h & 0x40)
					set_sod(cpustate, cpustate->AF.b.h >> 7);

				/* check for revealed interrupts */
				check_for_interrupts(cpustate);
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
		case 0x33: cpustate->icount -= IS_8085(cpustate) ? 6 : 5;	/* INX  SP */
			cpustate->SP.w.l++;
			if (IS_8085(cpustate))
			{
				if (cpustate->SP.w.l == 0x0000) cpustate->AF.b.l |= XF; else cpustate->AF.b.l &= ~XF;
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
			if (IS_8085(cpustate)) {
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
		case 0x3b: cpustate->icount -= IS_8085(cpustate) ? 6 : 5;	/* DCX  SP */
			cpustate->SP.w.l--;
			if (IS_8085(cpustate))
			{
				if (cpustate->SP.w.l == 0xffff) cpustate->AF.b.l |= XF; else cpustate->AF.b.l &= ~XF;
			}
			break;
		case 0x3c: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* INR  A */
			M_INR(cpustate->AF.b.h);
			break;
		case 0x3d: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* DCR  A */
			M_DCR(cpustate->AF.b.h);
			break;
		case 0x3e: cpustate->icount -= 7;	/* MVI  A,nn */
			M_MVI(cpustate->AF.b.h);
			break;
		case 0x3f: cpustate->icount -= 4;	/* CMC  */
			cpustate->AF.b.l = (cpustate->AF.b.l & 0xfe) | ((cpustate->AF.b.l & CF)==1 ? 0 : 1);
			break;

		case 0x40: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  B,B */
			/* no op */
			break;
		case 0x41: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  B,C */
			cpustate->BC.b.h = cpustate->BC.b.l;
			break;
		case 0x42: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  B,D */
			cpustate->BC.b.h = cpustate->DE.b.h;
			break;
		case 0x43: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  B,E */
			cpustate->BC.b.h = cpustate->DE.b.l;
			break;
		case 0x44: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  B,H */
			cpustate->BC.b.h = cpustate->HL.b.h;
			break;
		case 0x45: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  B,L */
			cpustate->BC.b.h = cpustate->HL.b.l;
			break;
		case 0x46: cpustate->icount -= 7;	/* MOV  B,M */
			cpustate->BC.b.h = RM(cpustate, cpustate->HL.d);
			break;
		case 0x47: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  B,A */
			cpustate->BC.b.h = cpustate->AF.b.h;
			break;

		case 0x48: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  C,B */
			cpustate->BC.b.l = cpustate->BC.b.h;
			break;
		case 0x49: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  C,C */
			/* no op */
			break;
		case 0x4a: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  C,D */
			cpustate->BC.b.l = cpustate->DE.b.h;
			break;
		case 0x4b: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  C,E */
			cpustate->BC.b.l = cpustate->DE.b.l;
			break;
		case 0x4c: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  C,H */
			cpustate->BC.b.l = cpustate->HL.b.h;
			break;
		case 0x4d: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  C,L */
			cpustate->BC.b.l = cpustate->HL.b.l;
			break;
		case 0x4e: cpustate->icount -= 7;	/* MOV  C,M */
			cpustate->BC.b.l = RM(cpustate, cpustate->HL.d);
			break;
		case 0x4f: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  C,A */
			cpustate->BC.b.l = cpustate->AF.b.h;
			break;

		case 0x50: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  D,B */
			cpustate->DE.b.h = cpustate->BC.b.h;
			break;
		case 0x51: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  D,C */
			cpustate->DE.b.h = cpustate->BC.b.l;
			break;
		case 0x52: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  D,D */
			/* no op */
			break;
		case 0x53: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  D,E */
			cpustate->DE.b.h = cpustate->DE.b.l;
			break;
		case 0x54: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  D,H */
			cpustate->DE.b.h = cpustate->HL.b.h;
			break;
		case 0x55: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  D,L */
			cpustate->DE.b.h = cpustate->HL.b.l;
			break;
		case 0x56: cpustate->icount -= 7;	/* MOV  D,M */
			cpustate->DE.b.h = RM(cpustate, cpustate->HL.d);
			break;
		case 0x57: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  D,A */
			cpustate->DE.b.h = cpustate->AF.b.h;
			break;

		case 0x58: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  E,B */
			cpustate->DE.b.l = cpustate->BC.b.h;
			break;
		case 0x59: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  E,C */
			cpustate->DE.b.l = cpustate->BC.b.l;
			break;
		case 0x5a: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  E,D */
			cpustate->DE.b.l = cpustate->DE.b.h;
			break;
		case 0x5b: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  E,E */
			/* no op */
			break;
		case 0x5c: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  E,H */
			cpustate->DE.b.l = cpustate->HL.b.h;
			break;
		case 0x5d: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  E,L */
			cpustate->DE.b.l = cpustate->HL.b.l;
			break;
		case 0x5e: cpustate->icount -= 7;	/* MOV  E,M */
			cpustate->DE.b.l = RM(cpustate, cpustate->HL.d);
			break;
		case 0x5f: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  E,A */
			cpustate->DE.b.l = cpustate->AF.b.h;
			break;

		case 0x60: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  H,B */
			cpustate->HL.b.h = cpustate->BC.b.h;
			break;
		case 0x61: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  H,C */
			cpustate->HL.b.h = cpustate->BC.b.l;
			break;
		case 0x62: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  H,D */
			cpustate->HL.b.h = cpustate->DE.b.h;
			break;
		case 0x63: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  H,E */
			cpustate->HL.b.h = cpustate->DE.b.l;
			break;
		case 0x64: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  H,H */
			/* no op */
			break;
		case 0x65: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  H,L */
			cpustate->HL.b.h = cpustate->HL.b.l;
			break;
		case 0x66: cpustate->icount -= 7;	/* MOV  H,M */
			cpustate->HL.b.h = RM(cpustate, cpustate->HL.d);
			break;
		case 0x67: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  H,A */
			cpustate->HL.b.h = cpustate->AF.b.h;
			break;

		case 0x68: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  L,B */
			cpustate->HL.b.l = cpustate->BC.b.h;
			break;
		case 0x69: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  L,C */
			cpustate->HL.b.l = cpustate->BC.b.l;
			break;
		case 0x6a: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  L,D */
			cpustate->HL.b.l = cpustate->DE.b.h;
			break;
		case 0x6b: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  L,E */
			cpustate->HL.b.l = cpustate->DE.b.l;
			break;
		case 0x6c: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  L,H */
			cpustate->HL.b.l = cpustate->HL.b.h;
			break;
		case 0x6d: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  L,L */
			/* no op */
			break;
		case 0x6e: cpustate->icount -= 7;	/* MOV  L,M */
			cpustate->HL.b.l = RM(cpustate, cpustate->HL.d);
			break;
		case 0x6f: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  L,A */
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
		case 0x76: cpustate->icount -= IS_8085(cpustate) ? 5 : 7;	/* HLT */
			cpustate->PC.w.l--;
			cpustate->HALT = 1;
			set_status(cpustate, 0x8a); // halt acknowledge
			if (cpustate->icount > 0) cpustate->icount = 0;
			break;
		case 0x77: cpustate->icount -= 7;	/* MOV  M,A */
			WM(cpustate, cpustate->HL.d, cpustate->AF.b.h);
			break;

		case 0x78: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  A,B */
			cpustate->AF.b.h = cpustate->BC.b.h;
			break;
		case 0x79: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  A,C */
			cpustate->AF.b.h = cpustate->BC.b.l;
			break;
		case 0x7a: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  A,D */
			cpustate->AF.b.h = cpustate->DE.b.h;
			break;
		case 0x7b: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  A,E */
			cpustate->AF.b.h = cpustate->DE.b.l;
			break;
		case 0x7c: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  A,H */
			cpustate->AF.b.h = cpustate->HL.b.h;
			break;
		case 0x7d: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  A,L */
			cpustate->AF.b.h = cpustate->HL.b.l;
			break;
		case 0x7e: cpustate->icount -= 7;	/* MOV  A,M */
			cpustate->AF.b.h = RM(cpustate, cpustate->HL.d);
			break;
		case 0x7f: cpustate->icount -= IS_8085(cpustate) ? 4 : 5;	/* MOV  A,A */
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

		case 0xc0: cpustate->icount -= IS_8085(cpustate) ? 6 : 5;	/* RNZ  */
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
		case 0xc5: cpustate->icount -= IS_8085(cpustate) ? 12 : 11;	/* PUSH B */
			M_PUSH(BC);
			break;
		case 0xc6: cpustate->icount -= 7;	/* ADI  nn */
			cpustate->XX.b.l = ARG(cpustate);
			M_ADD(cpustate->XX.b.l);
				break;
		case 0xc7: cpustate->icount -= IS_8085(cpustate) ? 12 : 11;	/* RST  0 */
			M_RST(0);
			break;

		case 0xc8: cpustate->icount -= IS_8085(cpustate) ? 6 : 5;	/* RZ   */
			M_RET( cpustate->AF.b.l & ZF );
			break;
		case 0xc9: cpustate->icount -= 10;	/* RET  */
			M_RET(1);
			break;
		case 0xca: cpustate->icount -= 10;	/* JZ   nnnn */
			M_JMP( cpustate->AF.b.l & ZF );
			break;
		case 0xcb:
			if (IS_8085(cpustate)) {
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
		case 0xcf: cpustate->icount -= IS_8085(cpustate) ? 12 : 11;	/* RST  1 */
			M_RST(1);
			break;

		case 0xd0: cpustate->icount -= IS_8085(cpustate) ? 6 : 5;	/* RNC  */
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
		case 0xd5: cpustate->icount -= IS_8085(cpustate) ? 12 : 11;	/* PUSH D */
			M_PUSH(DE);
			break;
		case 0xd6: cpustate->icount -= 7;	/* SUI  nn */
			cpustate->XX.b.l = ARG(cpustate);
			M_SUB(cpustate->XX.b.l);
			break;
		case 0xd7: cpustate->icount -= IS_8085(cpustate) ? 12 : 11;	/* RST  2 */
			M_RST(2);
			break;

		case 0xd8: cpustate->icount -= IS_8085(cpustate) ? 6 : 5;	/* RC   */
			M_RET( cpustate->AF.b.l & CF );
			break;
		case 0xd9:
			if (IS_8085(cpustate)) {
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
			if (IS_8085(cpustate)) {
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
		case 0xdf: cpustate->icount -= IS_8085(cpustate) ? 12 : 11;	/* RST  3 */
			M_RST(3);
			break;

		case 0xe0: cpustate->icount -= IS_8085(cpustate) ? 6 : 5;	/* RPO    */
			M_RET( !(cpustate->AF.b.l & VF) );
			break;
		case 0xe1: cpustate->icount -= 10;	/* POP  H */
			M_POP(HL);
			break;
		case 0xe2: cpustate->icount -= 10;	/* JPO  nnnn */
			M_JMP( !(cpustate->AF.b.l & VF) );
			break;
		case 0xe3: cpustate->icount -= IS_8085(cpustate) ? 16 : 18;	/* XTHL */
			M_POP(XX);
			M_PUSH(HL);
			cpustate->HL.d = cpustate->XX.d;
			break;
		case 0xe4: cpustate->icount -= 11;	/* CPO  nnnn */
			M_CALL( !(cpustate->AF.b.l & VF) );
			break;
		case 0xe5: cpustate->icount -= IS_8085(cpustate) ? 12 : 11;	/* PUSH H */
			M_PUSH(HL);
			break;
		case 0xe6: cpustate->icount -= 7;	/* ANI  nn */
			cpustate->XX.b.l = ARG(cpustate);
			M_ANA(cpustate->XX.b.l);
			break;
		case 0xe7: cpustate->icount -= IS_8085(cpustate) ? 12 : 11;	/* RST  4 */
			M_RST(4);
			break;

		case 0xe8: cpustate->icount -= IS_8085(cpustate) ? 6 : 5;	/* RPE  */
			M_RET( cpustate->AF.b.l & VF );
			break;
		case 0xe9: cpustate->icount -= IS_8085(cpustate) ? 6 : 5;	/* PCHL */
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
			if (IS_8085(cpustate)) {
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
		case 0xef: cpustate->icount -= IS_8085(cpustate) ? 12 : 11;	/* RST  5 */
			M_RST(5);
			break;

		case 0xf0: cpustate->icount -= IS_8085(cpustate) ? 6 : 5;	/* RP   */
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
			set_inte(cpustate, 0);
			break;
		case 0xf4: cpustate->icount -= 11;	/* CP   nnnn */
			M_CALL( !(cpustate->AF.b.l & SF) );
			break;
		case 0xf5: cpustate->icount -= IS_8085(cpustate) ? 12 : 11;	/* PUSH A */
			M_PUSH(AF);
			break;
		case 0xf6: cpustate->icount -= 7;	/* ORI  nn */
			cpustate->XX.b.l = ARG(cpustate);
			M_ORA(cpustate->XX.b.l);
			break;
		case 0xf7: cpustate->icount -= IS_8085(cpustate) ? 12 : 11;	/* RST  6 */
			M_RST(6);
			break;

		case 0xf8: cpustate->icount -= IS_8085(cpustate) ? 6 : 5;	/* RM   */
			M_RET( cpustate->AF.b.l & SF );
			break;
		case 0xf9: cpustate->icount -= IS_8085(cpustate) ? 6 : 5;	/* SPHL */
			cpustate->SP.d = cpustate->HL.d;
			break;
		case 0xfa: cpustate->icount -= 10;	/* JM   nnnn */
			M_JMP( cpustate->AF.b.l & SF );
			break;
		case 0xfb: cpustate->icount -= 4;	/* EI */
			/* set interrupt enable */
			set_inte(cpustate, 1);
			cpustate->after_ei = 2;
			break;
		case 0xfc: cpustate->icount -= 11;	/* CM   nnnn */
			M_CALL( cpustate->AF.b.l & SF );
			break;
		case 0xfd:
			if (IS_8085(cpustate)) {
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
		case 0xff: cpustate->icount -= IS_8085(cpustate) ? 12 : 11;	/* RST  7 */
			M_RST(7);
			break;
	}
}


/***************************************************************************
    COMMON EXECUTION
***************************************************************************/

static CPU_EXECUTE( i808x )
{
	i8085_state *cpustate = get_safe_token(device);

	cpustate->icount = cycles;

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

	return cycles - cpustate->icount;
}



/***************************************************************************
    CORE INITIALIZATION
***************************************************************************/

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


static void init_808x_common(const device_config *device, cpu_irq_callback irqcallback, int type)
{
	i8085_state *cpustate = get_safe_token(device);

	init_tables();

	/* set up the state table */
	cpustate->state = state_table_template;
	cpustate->state.baseptr = cpustate;
	cpustate->state.subtypemask = 1 << type;

	if (device->static_config != NULL)
		cpustate->config = *(i8085_config *)device->static_config;
	cpustate->cputype = type;
	cpustate->irq_callback = irqcallback;
	cpustate->device = device;

	cpustate->program = memory_find_address_space(device, ADDRESS_SPACE_PROGRAM);
	cpustate->io = memory_find_address_space(device, ADDRESS_SPACE_IO);

	state_save_register_device_item(device, 0, cpustate->PC.w.l);
	state_save_register_device_item(device, 0, cpustate->SP.w.l);
	state_save_register_device_item(device, 0, cpustate->AF.w.l);
	state_save_register_device_item(device, 0, cpustate->BC.w.l);
	state_save_register_device_item(device, 0, cpustate->DE.w.l);
	state_save_register_device_item(device, 0, cpustate->HL.w.l);
	state_save_register_device_item(device, 0, cpustate->HALT);
	state_save_register_device_item(device, 0, cpustate->IM);
	state_save_register_device_item(device, 0, cpustate->STATUS);
	state_save_register_device_item(device, 0, cpustate->after_ei);
	state_save_register_device_item(device, 0, cpustate->nmi_state);
	state_save_register_device_item_array(device, 0, cpustate->irq_state);
	state_save_register_device_item(device, 0, cpustate->trap_pending);
	state_save_register_device_item(device, 0, cpustate->trap_im_copy);
	state_save_register_device_item(device, 0, cpustate->sod_state);
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

	switch (entry->index)
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

	switch (entry->index)
	{
		case I8085_SID:
			cpustate->ietemp = ((cpustate->IM & IM_SID) != 0);
			if (cpustate->config.sid != NULL)
				cpustate->ietemp = ((*cpustate->config.sid)(cpustate->device) != 0);
			break;

		case I8085_INTE:
			cpustate->ietemp = ((cpustate->IM & IM_IE) != 0);
			break;

		default:
			fatalerror("CPU_EXPORT_STATE(i808x) called for unexpected value\n");
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
	i8085_state *cpustate = (device != NULL && device->token != NULL) ? get_safe_token(device) : NULL;
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

		case CPUINFO_INT_DATABUS_WIDTH_PROGRAM:			info->i = 8;							break;
		case CPUINFO_INT_ADDRBUS_WIDTH_PROGRAM: 		info->i = 16;							break;
		case CPUINFO_INT_ADDRBUS_SHIFT_PROGRAM: 		info->i = 0;							break;
		case CPUINFO_INT_DATABUS_WIDTH_IO:				info->i = 8;							break;
		case CPUINFO_INT_ADDRBUS_WIDTH_IO: 				info->i = 8;							break;
		case CPUINFO_INT_ADDRBUS_SHIFT_IO: 				info->i = 0;							break;

		/* --- the following bits of info are returned as pointers to functions --- */
		case CPUINFO_FCT_SET_INFO:		info->setinfo = CPU_SET_INFO_NAME(i808x);				break;
		case CPUINFO_FCT_INIT:			info->init = CPU_INIT_NAME(i8085);						break;
		case CPUINFO_FCT_RESET:			info->reset = CPU_RESET_NAME(i808x);					break;
		case CPUINFO_FCT_EXECUTE:		info->execute = CPU_EXECUTE_NAME(i808x);				break;
		case CPUINFO_FCT_DISASSEMBLE:	info->disassemble = CPU_DISASSEMBLE_NAME(i8085);		break;
		case CPUINFO_FCT_IMPORT_STATE:	info->import_state = CPU_IMPORT_STATE_NAME(i808x);		break;
		case CPUINFO_FCT_EXPORT_STATE:	info->export_state = CPU_EXPORT_STATE_NAME(i808x);		break;

		/* --- the following bits of info are returned as pointers --- */
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;		break;
		case CPUINFO_PTR_STATE_TABLE:					info->state_table = &cpustate->state;	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "8085A");				break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Intel 8080");			break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.1");					break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);				break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Juergen Buchmueller, all rights reserved."); break;

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
		case CPUINFO_FCT_INIT:			info->init = CPU_INIT_NAME(i8080);						break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "8080");				break;

		default:										CPU_GET_INFO_CALL(i8085); break;
	}
}
