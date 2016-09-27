// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller, hap
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
 * Revisions:
 *
 * xx-xx-2002 Acho A. Tang
 * - 8085 emulation was in fact never used. It's been treated as a plain 8080.
 * - protected IRQ0 vector from being overwritten
 * - modified interrupt handler to properly process 8085-specific IRQ's
 * - corrected interrupt masking, RIM and SIM behaviors according to Intel's documentation
 *
 * 20-Jul-2002 Krzysztof Strzecha
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
 * - ADC r instructions should affect parity flag.
 *   Fixed only for non x86 asm version (#define i8080_EXACT 1).
 *   There are probably more opcodes which should affect this flag, but don't.
 *
 * 05-Sep-2003 Krzysztof Strzecha
 * - INR r, DCR r, ADD r, SUB r, CMP r instructions should affect parity flag.
 *   Fixed only for non x86 asm version (#define i8080_EXACT 1).
 *
 * 23-Dec-2006 Tomasz Slanina
 * - SIM fixed
 *
 * 28-Jan-2007 Zsolt Vasvari
 * - Removed archaic i8080_EXACT flag.
 *
 * 08-June-2008 Miodrag Milanovic
 * - Flag setting fix for some instructions and cycle count update
 *
 * August 2009, hap
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
 * - added 8080A variant
 * - refactored callbacks to use devcb
 *
 * October 2012, hap
 * - fixed H flag on subtraction opcodes
 * - on 8080, don't push the unsupported flags(X5, X3, V) to stack
 * - 8080 passes on 8080/8085 CPU Exerciser, 8085 errors only on the DAA test
 *   (ref: http://www.idb.me.uk/sunhillow/8080.html - tests only 8080 opcodes)
 *
 *****************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "i8085.h"
#include "i8085cpu.h"

#define VERBOSE 0

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

#define CPUTYPE_8080    0
#define CPUTYPE_8085    1



/***************************************************************************
    MACROS
***************************************************************************/

#define IS_8080()          (m_cputype == CPUTYPE_8080)
#define IS_8085()          (m_cputype == CPUTYPE_8085)



/***************************************************************************
    STATIC TABLES
***************************************************************************/

/* cycles lookup */
const UINT8 i8085a_cpu_device::lut_cycles_8080[256]={
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
const UINT8 i8085a_cpu_device::lut_cycles_8085[256]={
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


const device_type I8080 = &device_creator<i8080_cpu_device>;
const device_type I8080A = &device_creator<i8080a_cpu_device>;
const device_type I8085A = &device_creator<i8085a_cpu_device>;


i8085a_cpu_device::i8085a_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, I8085A, "8085A", tag, owner, clock, "i8085a", __FILE__)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 16, 0)
	, m_io_config("io", ENDIANNESS_LITTLE, 8, 8, 0)
	, m_out_status_func(*this)
	, m_out_inte_func(*this)
	, m_in_sid_func(*this)
	, m_out_sod_func(*this)
	, m_cputype(CPUTYPE_8085)
{
}


i8085a_cpu_device::i8085a_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source, int cputype)
	: cpu_device(mconfig, type, name, tag, owner, clock, shortname, source)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 16, 0)
	, m_io_config("io", ENDIANNESS_LITTLE, 8, 8, 0)
	, m_out_status_func(*this)
	, m_out_inte_func(*this)
	, m_in_sid_func(*this)
	, m_out_sod_func(*this)
	, m_cputype(cputype)
{
}


i8080_cpu_device::i8080_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: i8085a_cpu_device(mconfig, I8080, "8080", tag, owner, clock, "i8080", __FILE__, CPUTYPE_8080)
{
}


i8080a_cpu_device::i8080a_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: i8085a_cpu_device(mconfig, I8080A, "8080A", tag, owner, clock, "i8080a", __FILE__, CPUTYPE_8080)
{
}


void i8085a_cpu_device::set_sod(int state)
{
	if (state != 0 && m_sod_state == 0)
	{
		m_sod_state = 1;
		m_out_sod_func(m_sod_state);
	}
	else if (state == 0 && m_sod_state != 0)
	{
		m_sod_state = 0;
		m_out_sod_func(m_sod_state);
	}
}


void i8085a_cpu_device::set_inte(int state)
{
	if (state != 0 && (m_IM & IM_IE) == 0)
	{
		m_IM |= IM_IE;
		m_out_inte_func(1);
	}
	else if (state == 0 && (m_IM & IM_IE) != 0)
	{
		m_IM &= ~IM_IE;
		m_out_inte_func(0);
	}
}


void i8085a_cpu_device::set_status(UINT8 status)
{
	if (status != m_STATUS)
		m_out_status_func(status);

	m_STATUS = status;
}


UINT8 i8085a_cpu_device::get_rim_value()
{
	UINT8 result = m_IM;
	int sid = m_in_sid_func();

	/* copy live RST5.5 and RST6.5 states */
	result &= ~(IM_I65 | IM_I55);
	if (m_irq_state[I8085_RST65_LINE]) result |= IM_I65;
	if (m_irq_state[I8085_RST55_LINE]) result |= IM_I55;

	/* fetch the SID bit if we have a callback */
	result = (result & ~IM_SID) | (sid ? IM_SID : 0);

	return result;
}


void i8085a_cpu_device::break_halt_for_interrupt()
{
	/* de-halt if necessary */
	if (m_HALT)
	{
		m_PC.w.l++;
		m_HALT = 0;
		set_status(0x26); /* int ack while halt */
	}
	else
		set_status(0x23); /* int ack */
}


UINT8 i8085a_cpu_device::ROP()
{
	set_status(0xa2); // instruction fetch
	return m_direct->read_byte(m_PC.w.l++);
}

UINT8 i8085a_cpu_device::ARG()
{
	return m_direct->read_byte(m_PC.w.l++);
}

UINT16 i8085a_cpu_device::ARG16()
{
	UINT16 w;
	w  = m_direct->read_byte(m_PC.d);
	m_PC.w.l++;
	w += m_direct->read_byte(m_PC.d) << 8;
	m_PC.w.l++;
	return w;
}

UINT8 i8085a_cpu_device::RM(UINT32 a)
{
	set_status(0x82); // memory read
	return m_program->read_byte(a);
}

void i8085a_cpu_device::WM(UINT32 a, UINT8 v)
{
	set_status(0x00); // memory write
	m_program->write_byte(a, v);
}


void i8085a_cpu_device::check_for_interrupts()
{
	/* TRAP is the highest priority */
	if (m_trap_pending)
	{
		/* the first RIM after a TRAP reflects the original IE state; remember it here,
		   setting the high bit to indicate it is valid */
		m_trap_im_copy = m_IM | 0x80;

		/* reset the pending state */
		m_trap_pending = FALSE;

		/* break out of HALT state and call the IRQ ack callback */
		break_halt_for_interrupt();
		standard_irq_callback(INPUT_LINE_NMI);

		/* push the PC and jump to $0024 */
		M_PUSH(PC);
		set_inte(0);
		m_PC.w.l = ADDR_TRAP;
		m_icount -= 11;
	}

	/* followed by RST7.5 */
	else if ((m_IM & IM_I75) && !(m_IM & IM_M75) && (m_IM & IM_IE))
	{
		/* reset the pending state (which is CPU-visible via the RIM instruction) */
		m_IM &= ~IM_I75;

		/* break out of HALT state and call the IRQ ack callback */
		break_halt_for_interrupt();
		standard_irq_callback(I8085_RST75_LINE);

		/* push the PC and jump to $003C */
		M_PUSH(PC);
		set_inte(0);
		m_PC.w.l = ADDR_RST75;
		m_icount -= 11;
	}

	/* followed by RST6.5 */
	else if (m_irq_state[I8085_RST65_LINE] && !(m_IM & IM_M65) && (m_IM & IM_IE))
	{
		/* break out of HALT state and call the IRQ ack callback */
		break_halt_for_interrupt();
		standard_irq_callback(I8085_RST65_LINE);

		/* push the PC and jump to $0034 */
		M_PUSH(PC);
		set_inte(0);
		m_PC.w.l = ADDR_RST65;
		m_icount -= 11;
	}

	/* followed by RST5.5 */
	else if (m_irq_state[I8085_RST55_LINE] && !(m_IM & IM_M55) && (m_IM & IM_IE))
	{
		/* break out of HALT state and call the IRQ ack callback */
		break_halt_for_interrupt();
		standard_irq_callback(I8085_RST55_LINE);

		/* push the PC and jump to $002C */
		M_PUSH(PC);
		set_inte(0);
		m_PC.w.l = ADDR_RST55;
		m_icount -= 11;
	}

	/* followed by classic INTR */
	else if (m_irq_state[I8085_INTR_LINE] && (m_IM & IM_IE))
	{
		UINT32 vector;

		/* break out of HALT state and call the IRQ ack callback */
		break_halt_for_interrupt();
		vector = standard_irq_callback(I8085_INTR_LINE);

		/* use the resulting vector as an opcode to execute */
		set_inte(0);
		switch (vector & 0xff0000)
		{
			case 0xcd0000:  /* CALL nnnn */
				m_icount -= 7;
				M_PUSH(PC);
			case 0xc30000:  /* JMP  nnnn */
				m_icount -= 10;
				m_PC.d = vector & 0xffff;
				break;

			default:
				LOG(("i8085 take int $%02x\n", vector));
				execute_one(vector & 0xff);
				break;
		}
	}
}


void i8085a_cpu_device::execute_one(int opcode)
{
	m_icount -= lut_cycles[opcode];

	switch (opcode)
	{
		case 0x00:                                                      break;  /* NOP  */
		case 0x01:  m_BC.w.l = ARG16();                 break;  /* LXI  B,nnnn */
		case 0x02:  WM(m_BC.d, m_AF.b.h);     break;  /* STAX B */
		case 0x03:  m_BC.w.l++;                                         /* INX  B */
					if (IS_8085()) { if (m_BC.w.l == 0x0000) m_AF.b.l |= X5F; else m_AF.b.l &= ~X5F; }
					break;
		case 0x04:  M_INR(m_BC.b.h);                            break;  /* INR  B */
		case 0x05:  M_DCR(m_BC.b.h);                            break;  /* DCR  B */
		case 0x06:  M_MVI(m_BC.b.h);                            break;  /* MVI  B,nn */
		case 0x07:  M_RLC;                                              break;  /* RLC  */

		case 0x08:  if (IS_8085()) { M_DSUB(); }                /* DSUB */
					/* else { ; } */                                            /* NOP  undocumented */
					break;
		case 0x09:  M_DAD(BC);                                          break;  /* DAD  B */
		case 0x0a:  m_AF.b.h = RM(m_BC.d);    break;  /* LDAX B */
		case 0x0b:  m_BC.w.l--;                                         /* DCX  B */
					if (IS_8085()) { if (m_BC.w.l == 0xffff) m_AF.b.l |= X5F; else m_AF.b.l &= ~X5F; }
					break;
		case 0x0c:  M_INR(m_BC.b.l);                            break;  /* INR  C */
		case 0x0d:  M_DCR(m_BC.b.l);                            break;  /* DCR  C */
		case 0x0e:  M_MVI(m_BC.b.l);                            break;  /* MVI  C,nn */
		case 0x0f:  M_RRC;                                              break;  /* RRC  */

		case 0x10:  if (IS_8085()) {                                    /* ASRH */
						m_AF.b.l = (m_AF.b.l & ~CF) | (m_HL.b.l & CF);
						m_HL.w.l = (m_HL.w.l >> 1);
					} /* else { ; } */                                          /* NOP  undocumented */
					break;
		case 0x11:  m_DE.w.l = ARG16();                 break;  /* LXI  D,nnnn */
		case 0x12:  WM(m_DE.d, m_AF.b.h);     break;  /* STAX D */
		case 0x13:  m_DE.w.l++;                                         /* INX  D */
					if (IS_8085()) { if (m_DE.w.l == 0x0000) m_AF.b.l |= X5F; else m_AF.b.l &= ~X5F; }
					break;
		case 0x14:  M_INR(m_DE.b.h);                            break;  /* INR  D */
		case 0x15:  M_DCR(m_DE.b.h);                            break;  /* DCR  D */
		case 0x16:  M_MVI(m_DE.b.h);                            break;  /* MVI  D,nn */
		case 0x17:  M_RAL;                                              break;  /* RAL  */

		case 0x18:  if (IS_8085()) {                                    /* RLDE */
						m_AF.b.l = (m_AF.b.l & ~(CF | VF)) | (m_DE.b.h >> 7);
						m_DE.w.l = (m_DE.w.l << 1) | (m_DE.w.l >> 15);
						if (0 != (((m_DE.w.l >> 15) ^ m_AF.b.l) & CF)) m_AF.b.l |= VF;
					} /* else { ; } */                                          /* NOP  undocumented */
					break;
		case 0x19:  M_DAD(DE);                                          break;  /* DAD  D */
		case 0x1a:  m_AF.b.h = RM(m_DE.d);    break;  /* LDAX D */
		case 0x1b:  m_DE.w.l--;                                         /* DCX  D */
					if (IS_8085()) { if (m_DE.w.l == 0xffff) m_AF.b.l |= X5F; else m_AF.b.l &= ~X5F; }
					break;
		case 0x1c:  M_INR(m_DE.b.l);                            break;  /* INR  E */
		case 0x1d:  M_DCR(m_DE.b.l);                            break;  /* DCR  E */
		case 0x1e:  M_MVI(m_DE.b.l);                            break;  /* MVI  E,nn */
		case 0x1f:  M_RAR;                                              break;  /* RAR  */

		case 0x20:  if (IS_8085()) {                                    /* RIM  */
						m_AF.b.h = get_rim_value();

						/* if we have remembered state from taking a TRAP, fix up the IE flag here */
						if (m_trap_im_copy & 0x80) m_AF.b.h = (m_AF.b.h & ~IM_IE) | (m_trap_im_copy & IM_IE);
						m_trap_im_copy = 0;
					} /* else { ; } */                                          /* NOP  undocumented */
					break;
		case 0x21:  m_HL.w.l = ARG16();                 break;  /* LXI  H,nnnn */
		case 0x22:  m_WZ.w.l = ARG16();                         /* SHLD nnnn */
					WM(m_WZ.d, m_HL.b.l); m_WZ.w.l++;
					WM(m_WZ.d, m_HL.b.h);
					break;
		case 0x23:  m_HL.w.l++;                                         /* INX  H */
					if (IS_8085()) { if (m_HL.w.l == 0x0000) m_AF.b.l |= X5F; else m_AF.b.l &= ~X5F; }
					break;
		case 0x24:  M_INR(m_HL.b.h);                            break;  /* INR  H */
		case 0x25:  M_DCR(m_HL.b.h);                            break;  /* DCR  H */
		case 0x26:  M_MVI(m_HL.b.h);                            break;  /* MVI  H,nn */
		case 0x27:  m_WZ.b.h = m_AF.b.h;                        /* DAA  */
					if (IS_8085() && m_AF.b.l&VF) {
						if ((m_AF.b.l&HF) | ((m_AF.b.h&0xf)>9)) m_WZ.b.h-=6;
						if ((m_AF.b.l&CF) | (m_AF.b.h>0x99)) m_WZ.b.h-=0x60;
					}
					else {
						if ((m_AF.b.l&HF) | ((m_AF.b.h&0xf)>9)) m_WZ.b.h+=6;
						if ((m_AF.b.l&CF) | (m_AF.b.h>0x99)) m_WZ.b.h+=0x60;
					}

					m_AF.b.l=(m_AF.b.l&3) | (m_AF.b.h&0x28) | (m_AF.b.h>0x99) | ((m_AF.b.h^m_WZ.b.h)&0x10) | ZSP[m_WZ.b.h];
					m_AF.b.h=m_WZ.b.h;
					break;

		case 0x28:  if (IS_8085()) {                                    /* LDEH nn */
						m_WZ.d = ARG();
						m_DE.d = (m_HL.d + m_WZ.d) & 0xffff;
					} /* else { ; } */                                          /* NOP  undocumented */
					break;
		case 0x29:  M_DAD(HL);                                          break;  /* DAD  H */
		case 0x2a:  m_WZ.d = ARG16();                           /* LHLD nnnn */
					m_HL.b.l = RM(m_WZ.d); m_WZ.w.l++;
					m_HL.b.h = RM(m_WZ.d);
					break;
		case 0x2b:  m_HL.w.l--;                                         /* DCX  H */
					if (IS_8085()) { if (m_HL.w.l == 0xffff) m_AF.b.l |= X5F; else m_AF.b.l &= ~X5F; }
					break;
		case 0x2c:  M_INR(m_HL.b.l);                            break;  /* INR  L */
		case 0x2d:  M_DCR(m_HL.b.l);                            break;  /* DCR  L */
		case 0x2e:  M_MVI(m_HL.b.l);                            break;  /* MVI  L,nn */
		case 0x2f:  m_AF.b.h ^= 0xff;                                   /* CMA  */
					if (IS_8085()) m_AF.b.l |= HF | VF;
					break;

		case 0x30:  if (IS_8085()) {                                    /* SIM  */
						/* if bit 3 is set, bits 0-2 become the new masks */
						if (m_AF.b.h & 0x08) {
							m_IM &= ~(IM_M55 | IM_M65 | IM_M75 | IM_I55 | IM_I65);
							m_IM |= m_AF.b.h & (IM_M55 | IM_M65 | IM_M75);

							/* update live state based on the new masks */
							if ((m_IM & IM_M55) == 0 && m_irq_state[I8085_RST55_LINE]) m_IM |= IM_I55;
							if ((m_IM & IM_M65) == 0 && m_irq_state[I8085_RST65_LINE]) m_IM |= IM_I65;
						}

						/* bit if 4 is set, the 7.5 flip-flop is cleared */
						if (m_AF.b.h & 0x10) m_IM &= ~IM_I75;

						/* if bit 6 is set, then bit 7 is the new SOD state */
						if (m_AF.b.h & 0x40) set_sod(m_AF.b.h >> 7);

						/* check for revealed interrupts */
						check_for_interrupts();
					} /* else { ; } */                                          /* NOP  undocumented */
					break;
		case 0x31:  m_SP.w.l = ARG16();                 break;  /* LXI  SP,nnnn */
		case 0x32:  m_WZ.d = ARG16();                           /* STAX nnnn */
					WM(m_WZ.d, m_AF.b.h);
					break;
		case 0x33:  m_SP.w.l++;                                         /* INX  SP */
					if (IS_8085()) { if (m_SP.w.l == 0x0000) m_AF.b.l |= X5F; else m_AF.b.l &= ~X5F; }
					break;
		case 0x34:  m_WZ.b.l = RM(m_HL.d);            /* INR  M */
					M_INR(m_WZ.b.l);
					WM(m_HL.d, m_WZ.b.l);
					break;
		case 0x35:  m_WZ.b.l = RM(m_HL.d);            /* DCR  M */
					M_DCR(m_WZ.b.l);
					WM(m_HL.d, m_WZ.b.l);
					break;
		case 0x36:  m_WZ.b.l = ARG();                           /* MVI  M,nn */
					WM(m_HL.d, m_WZ.b.l);
					break;
		case 0x37:  m_AF.b.l = (m_AF.b.l & 0xfe) | CF;  break;  /* STC  */

		case 0x38:  if (IS_8085()) {                                    /* LDES nn */
						m_WZ.d = ARG();
						m_DE.d = (m_SP.d + m_WZ.d) & 0xffff;
					} /* else { ; } */                                          /* NOP  undocumented */
					break;
		case 0x39:  M_DAD(SP);                                          break;  /* DAD  SP */
		case 0x3a:  m_WZ.d = ARG16();                           /* LDAX nnnn */
					m_AF.b.h = RM(m_WZ.d);
					break;
		case 0x3b:  m_SP.w.l--;                                         /* DCX  SP */
					if (IS_8085()) { if (m_SP.w.l == 0xffff) m_AF.b.l |= X5F; else m_AF.b.l &= ~X5F; }
					break;
		case 0x3c:  M_INR(m_AF.b.h);                            break;  /* INR  A */
		case 0x3d:  M_DCR(m_AF.b.h);                            break;  /* DCR  A */
		case 0x3e:  M_MVI(m_AF.b.h);                            break;  /* MVI  A,nn */
		case 0x3f:  m_AF.b.l = (m_AF.b.l & 0xfe) | (~m_AF.b.l & CF); break; /* CMC  */

		case 0x40:                                                      break;  /* MOV  B,B */
		case 0x41:  m_BC.b.h = m_BC.b.l;                break;  /* MOV  B,C */
		case 0x42:  m_BC.b.h = m_DE.b.h;                break;  /* MOV  B,D */
		case 0x43:  m_BC.b.h = m_DE.b.l;                break;  /* MOV  B,E */
		case 0x44:  m_BC.b.h = m_HL.b.h;                break;  /* MOV  B,H */
		case 0x45:  m_BC.b.h = m_HL.b.l;                break;  /* MOV  B,L */
		case 0x46:  m_BC.b.h = RM(m_HL.d);    break;  /* MOV  B,M */
		case 0x47:  m_BC.b.h = m_AF.b.h;                break;  /* MOV  B,A */

		case 0x48:  m_BC.b.l = m_BC.b.h;                break;  /* MOV  C,B */
		case 0x49:                                                      break;  /* MOV  C,C */
		case 0x4a:  m_BC.b.l = m_DE.b.h;                break;  /* MOV  C,D */
		case 0x4b:  m_BC.b.l = m_DE.b.l;                break;  /* MOV  C,E */
		case 0x4c:  m_BC.b.l = m_HL.b.h;                break;  /* MOV  C,H */
		case 0x4d:  m_BC.b.l = m_HL.b.l;                break;  /* MOV  C,L */
		case 0x4e:  m_BC.b.l = RM(m_HL.d);    break;  /* MOV  C,M */
		case 0x4f:  m_BC.b.l = m_AF.b.h;                break;  /* MOV  C,A */

		case 0x50:  m_DE.b.h = m_BC.b.h;                break;  /* MOV  D,B */
		case 0x51:  m_DE.b.h = m_BC.b.l;                break;  /* MOV  D,C */
		case 0x52:                                                      break;  /* MOV  D,D */
		case 0x53:  m_DE.b.h = m_DE.b.l;                break;  /* MOV  D,E */
		case 0x54:  m_DE.b.h = m_HL.b.h;                break;  /* MOV  D,H */
		case 0x55:  m_DE.b.h = m_HL.b.l;                break;  /* MOV  D,L */
		case 0x56:  m_DE.b.h = RM(m_HL.d);    break;  /* MOV  D,M */
		case 0x57:  m_DE.b.h = m_AF.b.h;                break;  /* MOV  D,A */

		case 0x58:  m_DE.b.l = m_BC.b.h;                break;  /* MOV  E,B */
		case 0x59:  m_DE.b.l = m_BC.b.l;                break;  /* MOV  E,C */
		case 0x5a:  m_DE.b.l = m_DE.b.h;                break;  /* MOV  E,D */
		case 0x5b:                                                      break;  /* MOV  E,E */
		case 0x5c:  m_DE.b.l = m_HL.b.h;                break;  /* MOV  E,H */
		case 0x5d:  m_DE.b.l = m_HL.b.l;                break;  /* MOV  E,L */
		case 0x5e:  m_DE.b.l = RM(m_HL.d);    break;  /* MOV  E,M */
		case 0x5f:  m_DE.b.l = m_AF.b.h;                break;  /* MOV  E,A */

		case 0x60:  m_HL.b.h = m_BC.b.h;                break;  /* MOV  H,B */
		case 0x61:  m_HL.b.h = m_BC.b.l;                break;  /* MOV  H,C */
		case 0x62:  m_HL.b.h = m_DE.b.h;                break;  /* MOV  H,D */
		case 0x63:  m_HL.b.h = m_DE.b.l;                break;  /* MOV  H,E */
		case 0x64:                                                      break;  /* MOV  H,H */
		case 0x65:  m_HL.b.h = m_HL.b.l;                break;  /* MOV  H,L */
		case 0x66:  m_HL.b.h = RM(m_HL.d);    break;  /* MOV  H,M */
		case 0x67:  m_HL.b.h = m_AF.b.h;                break;  /* MOV  H,A */

		case 0x68:  m_HL.b.l = m_BC.b.h;                break;  /* MOV  L,B */
		case 0x69:  m_HL.b.l = m_BC.b.l;                break;  /* MOV  L,C */
		case 0x6a:  m_HL.b.l = m_DE.b.h;                break;  /* MOV  L,D */
		case 0x6b:  m_HL.b.l = m_DE.b.l;                break;  /* MOV  L,E */
		case 0x6c:  m_HL.b.l = m_HL.b.h;                break;  /* MOV  L,H */
		case 0x6d:                                                      break;  /* MOV  L,L */
		case 0x6e:  m_HL.b.l = RM(m_HL.d);    break;  /* MOV  L,M */
		case 0x6f:  m_HL.b.l = m_AF.b.h;                break;  /* MOV  L,A */

		case 0x70:  WM(m_HL.d, m_BC.b.h);     break;  /* MOV  M,B */
		case 0x71:  WM(m_HL.d, m_BC.b.l);     break;  /* MOV  M,C */
		case 0x72:  WM(m_HL.d, m_DE.b.h);     break;  /* MOV  M,D */
		case 0x73:  WM(m_HL.d, m_DE.b.l);     break;  /* MOV  M,E */
		case 0x74:  WM(m_HL.d, m_HL.b.h);     break;  /* MOV  M,H */
		case 0x75:  WM(m_HL.d, m_HL.b.l);     break;  /* MOV  M,L */
		case 0x76:  m_PC.w.l--; m_HALT = 1;                     /* HLT */
					set_status(0x8a); // halt acknowledge
					break;
		case 0x77:  WM(m_HL.d, m_AF.b.h);     break;  /* MOV  M,A */

		case 0x78:  m_AF.b.h = m_BC.b.h;                break;  /* MOV  A,B */
		case 0x79:  m_AF.b.h = m_BC.b.l;                break;  /* MOV  A,C */
		case 0x7a:  m_AF.b.h = m_DE.b.h;                break;  /* MOV  A,D */
		case 0x7b:  m_AF.b.h = m_DE.b.l;                break;  /* MOV  A,E */
		case 0x7c:  m_AF.b.h = m_HL.b.h;                break;  /* MOV  A,H */
		case 0x7d:  m_AF.b.h = m_HL.b.l;                break;  /* MOV  A,L */
		case 0x7e:  m_AF.b.h = RM(m_HL.d);    break;  /* MOV  A,M */
		case 0x7f:                                                      break;  /* MOV  A,A */

		case 0x80:  M_ADD(m_BC.b.h);                            break;  /* ADD  B */
		case 0x81:  M_ADD(m_BC.b.l);                            break;  /* ADD  C */
		case 0x82:  M_ADD(m_DE.b.h);                            break;  /* ADD  D */
		case 0x83:  M_ADD(m_DE.b.l);                            break;  /* ADD  E */
		case 0x84:  M_ADD(m_HL.b.h);                            break;  /* ADD  H */
		case 0x85:  M_ADD(m_HL.b.l);                            break;  /* ADD  L */
		case 0x86:  m_WZ.b.l = RM(m_HL.d); M_ADD(m_WZ.b.l); break; /* ADD  M */
		case 0x87:  M_ADD(m_AF.b.h);                            break;  /* ADD  A */

		case 0x88:  M_ADC(m_BC.b.h);                            break;  /* ADC  B */
		case 0x89:  M_ADC(m_BC.b.l);                            break;  /* ADC  C */
		case 0x8a:  M_ADC(m_DE.b.h);                            break;  /* ADC  D */
		case 0x8b:  M_ADC(m_DE.b.l);                            break;  /* ADC  E */
		case 0x8c:  M_ADC(m_HL.b.h);                            break;  /* ADC  H */
		case 0x8d:  M_ADC(m_HL.b.l);                            break;  /* ADC  L */
		case 0x8e:  m_WZ.b.l = RM(m_HL.d); M_ADC(m_WZ.b.l); break; /* ADC  M */
		case 0x8f:  M_ADC(m_AF.b.h);                            break;  /* ADC  A */

		case 0x90:  M_SUB(m_BC.b.h);                            break;  /* SUB  B */
		case 0x91:  M_SUB(m_BC.b.l);                            break;  /* SUB  C */
		case 0x92:  M_SUB(m_DE.b.h);                            break;  /* SUB  D */
		case 0x93:  M_SUB(m_DE.b.l);                            break;  /* SUB  E */
		case 0x94:  M_SUB(m_HL.b.h);                            break;  /* SUB  H */
		case 0x95:  M_SUB(m_HL.b.l);                            break;  /* SUB  L */
		case 0x96:  m_WZ.b.l = RM(m_HL.d); M_SUB(m_WZ.b.l); break; /* SUB  M */
		case 0x97:  M_SUB(m_AF.b.h);                            break;  /* SUB  A */

		case 0x98:  M_SBB(m_BC.b.h);                            break;  /* SBB  B */
		case 0x99:  M_SBB(m_BC.b.l);                            break;  /* SBB  C */
		case 0x9a:  M_SBB(m_DE.b.h);                            break;  /* SBB  D */
		case 0x9b:  M_SBB(m_DE.b.l);                            break;  /* SBB  E */
		case 0x9c:  M_SBB(m_HL.b.h);                            break;  /* SBB  H */
		case 0x9d:  M_SBB(m_HL.b.l);                            break;  /* SBB  L */
		case 0x9e:  m_WZ.b.l = RM(m_HL.d); M_SBB(m_WZ.b.l); break; /* SBB  M */
		case 0x9f:  M_SBB(m_AF.b.h);                            break;  /* SBB  A */

		case 0xa0:  M_ANA(m_BC.b.h);                            break;  /* ANA  B */
		case 0xa1:  M_ANA(m_BC.b.l);                            break;  /* ANA  C */
		case 0xa2:  M_ANA(m_DE.b.h);                            break;  /* ANA  D */
		case 0xa3:  M_ANA(m_DE.b.l);                            break;  /* ANA  E */
		case 0xa4:  M_ANA(m_HL.b.h);                            break;  /* ANA  H */
		case 0xa5:  M_ANA(m_HL.b.l);                            break;  /* ANA  L */
		case 0xa6:  m_WZ.b.l = RM(m_HL.d); M_ANA(m_WZ.b.l); break; /* ANA  M */
		case 0xa7:  M_ANA(m_AF.b.h);                            break;  /* ANA  A */

		case 0xa8:  M_XRA(m_BC.b.h);                            break;  /* XRA  B */
		case 0xa9:  M_XRA(m_BC.b.l);                            break;  /* XRA  C */
		case 0xaa:  M_XRA(m_DE.b.h);                            break;  /* XRA  D */
		case 0xab:  M_XRA(m_DE.b.l);                            break;  /* XRA  E */
		case 0xac:  M_XRA(m_HL.b.h);                            break;  /* XRA  H */
		case 0xad:  M_XRA(m_HL.b.l);                            break;  /* XRA  L */
		case 0xae:  m_WZ.b.l = RM(m_HL.d); M_XRA(m_WZ.b.l); break; /* XRA  M */
		case 0xaf:  M_XRA(m_AF.b.h);                            break;  /* XRA  A */

		case 0xb0:  M_ORA(m_BC.b.h);                            break;  /* ORA  B */
		case 0xb1:  M_ORA(m_BC.b.l);                            break;  /* ORA  C */
		case 0xb2:  M_ORA(m_DE.b.h);                            break;  /* ORA  D */
		case 0xb3:  M_ORA(m_DE.b.l);                            break;  /* ORA  E */
		case 0xb4:  M_ORA(m_HL.b.h);                            break;  /* ORA  H */
		case 0xb5:  M_ORA(m_HL.b.l);                            break;  /* ORA  L */
		case 0xb6:  m_WZ.b.l = RM(m_HL.d); M_ORA(m_WZ.b.l); break; /* ORA  M */
		case 0xb7:  M_ORA(m_AF.b.h);                            break;  /* ORA  A */

		case 0xb8:  M_CMP(m_BC.b.h);                            break;  /* CMP  B */
		case 0xb9:  M_CMP(m_BC.b.l);                            break;  /* CMP  C */
		case 0xba:  M_CMP(m_DE.b.h);                            break;  /* CMP  D */
		case 0xbb:  M_CMP(m_DE.b.l);                            break;  /* CMP  E */
		case 0xbc:  M_CMP(m_HL.b.h);                            break;  /* CMP  H */
		case 0xbd:  M_CMP(m_HL.b.l);                            break;  /* CMP  L */
		case 0xbe:  m_WZ.b.l = RM(m_HL.d); M_CMP(m_WZ.b.l); break; /* CMP  M */
		case 0xbf:  M_CMP(m_AF.b.h);                            break;  /* CMP  A */

		case 0xc0:  M_RET( !(m_AF.b.l & ZF) );                  break;  /* RNZ  */
		case 0xc1:  M_POP(BC);                                          break;  /* POP  B */
		case 0xc2:  M_JMP( !(m_AF.b.l & ZF) );                  break;  /* JNZ  nnnn */
		case 0xc3:  M_JMP(1);                                           break;  /* JMP  nnnn */
		case 0xc4:  M_CALL( !(m_AF.b.l & ZF) );                 break;  /* CNZ  nnnn */
		case 0xc5:  M_PUSH(BC);                                         break;  /* PUSH B */
		case 0xc6:  m_WZ.b.l = ARG(); M_ADD(m_WZ.b.l); break; /* ADI  nn */
		case 0xc7:  M_RST(0);                                           break;  /* RST  0 */

		case 0xc8:  M_RET( m_AF.b.l & ZF );                     break;  /* RZ   */
		case 0xc9:  M_POP(PC);                                          break;  /* RET  */
		case 0xca:  M_JMP( m_AF.b.l & ZF );                     break;  /* JZ   nnnn */
		case 0xcb:  if (IS_8085()) {                                    /* RST  V */
						if (m_AF.b.l & VF) { M_RST(8); }
						else m_icount += 6; // RST not taken
					} else { M_JMP(1); }                                        /* JMP  nnnn undocumented */
					break;
		case 0xcc:  M_CALL( m_AF.b.l & ZF );                    break;  /* CZ   nnnn */
		case 0xcd:  M_CALL(1);                                          break;  /* CALL nnnn */
		case 0xce:  m_WZ.b.l = ARG(); M_ADC(m_WZ.b.l); break; /* ACI  nn */
		case 0xcf:  M_RST(1);                                           break;  /* RST  1 */

		case 0xd0:  M_RET( !(m_AF.b.l & CF) );                  break;  /* RNC  */
		case 0xd1:  M_POP(DE);                                          break;  /* POP  D */
		case 0xd2:  M_JMP( !(m_AF.b.l & CF) );                  break;  /* JNC  nnnn */
		case 0xd3:  M_OUT;                                              break;  /* OUT  nn */
		case 0xd4:  M_CALL( !(m_AF.b.l & CF) );                 break;  /* CNC  nnnn */
		case 0xd5:  M_PUSH(DE);                                         break;  /* PUSH D */
		case 0xd6:  m_WZ.b.l = ARG(); M_SUB(m_WZ.b.l); break; /* SUI  nn */
		case 0xd7:  M_RST(2);                                           break;  /* RST  2 */

		case 0xd8:  M_RET( m_AF.b.l & CF );                     break;  /* RC   */
		case 0xd9:  if (IS_8085()) {                                    /* SHLX */
						m_WZ.w.l = m_DE.w.l;
						WM(m_WZ.d, m_HL.b.l); m_WZ.w.l++;
						WM(m_WZ.d, m_HL.b.h);
					} else { M_POP(PC); }                                       /* RET  undocumented */
					break;
		case 0xda:  M_JMP( m_AF.b.l & CF );                     break;  /* JC   nnnn */
		case 0xdb:  M_IN;                                               break;  /* IN   nn */
		case 0xdc:  M_CALL( m_AF.b.l & CF );                    break;  /* CC   nnnn */
		case 0xdd:  if (IS_8085()) { M_JMP( !(m_AF.b.l & X5F) ); } /* JNX  nnnn */
					else { M_CALL(1); }                                         /* CALL nnnn undocumented */
					break;
		case 0xde:  m_WZ.b.l = ARG(); M_SBB(m_WZ.b.l); break; /* SBI  nn */
		case 0xdf:  M_RST(3);                                           break;  /* RST  3 */

		case 0xe0:  M_RET( !(m_AF.b.l & PF) );                  break;  /* RPO    */
		case 0xe1:  M_POP(HL);                                          break;  /* POP  H */
		case 0xe2:  M_JMP( !(m_AF.b.l & PF) );                  break;  /* JPO  nnnn */
		case 0xe3:  M_POP(WZ); M_PUSH(HL);                                      /* XTHL */
					m_HL.d = m_WZ.d;
					break;
		case 0xe4:  M_CALL( !(m_AF.b.l & PF) );                 break;  /* CPO  nnnn */
		case 0xe5:  M_PUSH(HL);                                         break;  /* PUSH H */
		case 0xe6:  m_WZ.b.l = ARG(); M_ANA(m_WZ.b.l); break; /* ANI  nn */
		case 0xe7:  M_RST(4);                                           break;  /* RST  4 */

		case 0xe8:  M_RET( m_AF.b.l & PF );                     break;  /* RPE  */
		case 0xe9:  m_PC.d = m_HL.w.l;                  break;  /* PCHL */
		case 0xea:  M_JMP( m_AF.b.l & PF );                     break;  /* JPE  nnnn */
		case 0xeb:  m_WZ.d = m_DE.d;                            /* XCHG */
					m_DE.d = m_HL.d;
					m_HL.d = m_WZ.d;
					break;
		case 0xec:  M_CALL( m_AF.b.l & PF );                    break;  /* CPE  nnnn */
		case 0xed:  if (IS_8085()) {                                    /* LHLX */
						m_WZ.w.l = m_DE.w.l;
						m_HL.b.l = RM(m_WZ.d); m_WZ.w.l++;
						m_HL.b.h = RM(m_WZ.d);
					} else { M_CALL(1); }                                       /* CALL nnnn undocumented */
					break;
		case 0xee:  m_WZ.b.l = ARG(); M_XRA(m_WZ.b.l); break; /* XRI  nn */
		case 0xef:  M_RST(5);                                           break;  /* RST  5 */

		case 0xf0:  M_RET( !(m_AF.b.l&SF) );                    break;  /* RP   */
		case 0xf1:  M_POP(AF);                                          break;  /* POP  A */
		case 0xf2:  M_JMP( !(m_AF.b.l & SF) );                  break;  /* JP   nnnn */
		case 0xf3:  set_inte(0);                              break;  /* DI   */
		case 0xf4:  M_CALL( !(m_AF.b.l & SF) );                 break;  /* CP   nnnn */
		case 0xf5:  if (IS_8080()) m_AF.b.l = (m_AF.b.l&~(X3F|X5F))|VF; // on 8080, VF=1 and X3F=0 and X5F=0 always! (we don't have to check for it elsewhere)
					M_PUSH(AF);                                         break;  /* PUSH A */
		case 0xf6:  m_WZ.b.l = ARG(); M_ORA(m_WZ.b.l); break; /* ORI  nn */
		case 0xf7:  M_RST(6);                                           break;  /* RST  6 */

		case 0xf8:  M_RET( m_AF.b.l & SF );                     break;  /* RM   */
		case 0xf9:  m_SP.d = m_HL.d;                    break;  /* SPHL */
		case 0xfa:  M_JMP( m_AF.b.l & SF );                     break;  /* JM   nnnn */
		case 0xfb:  set_inte(1); m_after_ei = 2;      break;  /* EI */
		case 0xfc:  M_CALL( m_AF.b.l & SF );                    break;  /* CM   nnnn */
		case 0xfd:  if (IS_8085()) { M_JMP( m_AF.b.l & X5F ); } /* JX   nnnn */
					else { M_CALL(1); }                                         /* CALL nnnn undocumented */
					break;
		case 0xfe:  m_WZ.b.l = ARG(); M_CMP(m_WZ.b.l); break; /* CPI  nn */
		case 0xff:  M_RST(7);                                           break;  /* RST  7 */
	}
}


/***************************************************************************
    COMMON EXECUTION
***************************************************************************/

void i8085a_cpu_device::execute_run()
{
	/* check for TRAPs before diving in (can't do others because of after_ei) */
	if (m_trap_pending || m_after_ei == 0)
		check_for_interrupts();

	do
	{
		debugger_instruction_hook(this, m_PC.d);

		/* the instruction after an EI does not take an interrupt, so
		   we cannot check immediately; handle post-EI behavior here */
		if (m_after_ei != 0 && --m_after_ei == 0)
			check_for_interrupts();

		/* here we go... */
		execute_one(ROP());

	} while (m_icount > 0);
}



/***************************************************************************
    CORE INITIALIZATION
***************************************************************************/

void i8085a_cpu_device::init_tables()
{
	UINT8 zs;
	int i, p;
	for (i = 0; i < 256; i++)
	{
		/* cycles */
		lut_cycles[i] = m_cputype?lut_cycles_8085[i]:lut_cycles_8080[i];

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


void i8085a_cpu_device::device_start()
{
	m_PC.d = 0;
	m_SP.d = 0;
	m_AF.d = 0;
	m_BC.d = 0;
	m_DE.d = 0;
	m_HL.d = 0;
	m_WZ.d = 0;
	m_HALT = 0;
	m_IM = 0;
	m_STATUS = 0;
	m_after_ei = 0;
	m_nmi_state = 0;
	m_irq_state[3] = m_irq_state[2] = m_irq_state[1] = m_irq_state[0] = 0;
	m_trap_pending = 0;
	m_trap_im_copy = 0;
	m_sod_state = 0;
	m_ietemp = false;

	init_tables();

	/* set up the state table */
	{
		state_add(I8085_PC,     "PC",     m_PC.w.l);
		state_add(STATE_GENPC,  "GENPC",  m_PC.w.l).noshow();
		state_add(I8085_SP,     "SP",     m_SP.w.l);
		state_add(STATE_GENSP,  "GENSP",  m_SP.w.l).noshow();
		state_add(STATE_GENFLAGS, "GENFLAGS", m_AF.b.l).noshow().formatstr("%8s");
		state_add(I8085_A,      "A",      m_AF.b.h).noshow();
		state_add(I8085_B,      "B",      m_BC.b.h).noshow();
		state_add(I8085_C,      "C",      m_BC.b.l).noshow();
		state_add(I8085_D,      "D",      m_DE.b.h).noshow();
		state_add(I8085_E,      "E",      m_DE.b.l).noshow();
		state_add(I8085_F,      "F",      m_AF.b.l).noshow();
		state_add(I8085_H,      "H",      m_HL.b.h).noshow();
		state_add(I8085_L,      "L",      m_HL.b.l).noshow();
		state_add(I8085_AF,     "AF",     m_AF.w.l);
		state_add(I8085_BC,     "BC",     m_BC.w.l);
		state_add(I8085_DE,     "DE",     m_DE.w.l);
		state_add(I8085_HL,     "HL",     m_HL.w.l);
		state_add(I8085_STATUS, "STATUS", m_STATUS);
		state_add(I8085_SOD,    "SOD",    m_sod_state).mask(0x1);
		state_add(I8085_SID,    "SID",    m_ietemp).mask(0x1).callimport().callexport();
		state_add(I8085_INTE,   "INTE",   m_ietemp).mask(0x1).callimport().callexport();
	}

	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();
	m_io = &space(AS_IO);

	/* resolve callbacks */
	m_out_status_func.resolve_safe();
	m_out_inte_func.resolve_safe();
	m_in_sid_func.resolve_safe(0);
	m_out_sod_func.resolve_safe();

	/* register for state saving */
	save_item(NAME(m_PC.w.l));
	save_item(NAME(m_SP.w.l));
	save_item(NAME(m_AF.w.l));
	save_item(NAME(m_BC.w.l));
	save_item(NAME(m_DE.w.l));
	save_item(NAME(m_HL.w.l));
	save_item(NAME(m_HALT));
	save_item(NAME(m_IM));
	save_item(NAME(m_STATUS));
	save_item(NAME(m_after_ei));
	save_item(NAME(m_nmi_state));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_trap_pending));
	save_item(NAME(m_trap_im_copy));
	save_item(NAME(m_sod_state));

	m_icountptr = &m_icount;
}


/***************************************************************************
    COMMON RESET
***************************************************************************/

void i8085a_cpu_device::device_reset()
{
	m_PC.d = 0;
	m_HALT = 0;
	m_IM &= ~IM_I75;
	m_IM |= IM_M55 | IM_M65 | IM_M75;
	m_after_ei = FALSE;
	m_trap_pending = FALSE;
	m_trap_im_copy = 0;
	set_inte(0);
	set_sod(0);
}



/***************************************************************************
    COMMON STATE IMPORT/EXPORT
***************************************************************************/

void i8085a_cpu_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case I8085_SID:
			if (m_ietemp)
			{
				m_IM |= IM_SID;
			}
			else
			{
				m_IM &= ~IM_SID;
			}
			break;

		case I8085_INTE:
			if (m_ietemp)
			{
				m_IM |= IM_IE;
			}
			else
			{
				m_IM &= ~IM_IE;
			}
			break;

		default:
			fatalerror("CPU_IMPORT_STATE(i808x) called for unexpected value\n");
	}
}


void i8085a_cpu_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case I8085_SID:
			m_ietemp = ((m_IM & IM_SID) != 0) && m_in_sid_func() != 0;
			break;

		case I8085_INTE:
			m_ietemp = ((m_IM & IM_IE) != 0);
			break;

		default:
			fatalerror("CPU_EXPORT_STATE(i808x) called for unexpected value\n");
	}
}

void i8085a_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%c%c%c%c%c%c",
				m_AF.b.l & 0x80 ? 'S':'.',
				m_AF.b.l & 0x40 ? 'Z':'.',
				m_AF.b.l & 0x20 ? 'X':'.', // X5
				m_AF.b.l & 0x10 ? 'H':'.',
				m_AF.b.l & 0x08 ? '?':'.',
				m_AF.b.l & 0x04 ? 'P':'.',
				m_AF.b.l & 0x02 ? 'V':'.',
				m_AF.b.l & 0x01 ? 'C':'.');
			break;
	}
}


void i8085a_cpu_device::execute_set_input(int irqline, int state)
{
	int newstate = (state != CLEAR_LINE);

	/* NMI is edge-triggered */
	if (irqline == INPUT_LINE_NMI)
	{
		if (!m_nmi_state && newstate)
			m_trap_pending = TRUE;
		m_nmi_state = newstate;
	}

	/* RST7.5 is edge-triggered */
	else if (irqline == I8085_RST75_LINE)
	{
		if (!m_irq_state[I8085_RST75_LINE] && newstate)
			m_IM |= IM_I75;
		m_irq_state[I8085_RST75_LINE] = newstate;
	}

	/* remaining sources are level triggered */
	else if (irqline < ARRAY_LENGTH(m_irq_state))
		m_irq_state[irqline] = state;
}


offs_t i8085a_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( i8085 );
	return CPU_DISASSEMBLE_NAME(i8085)(this, buffer, pc, oprom, opram, options);
}
