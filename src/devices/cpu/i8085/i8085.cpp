// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller, hap
// thanks-to:Marcel De Kogel
/*****************************************************************************
 *
 *   Portable I8085A emulator V1.3
 *
 *   Copyright Juergen Buchmueller
 *   Partially based on information out of Z80Em by Marcel De Kogel
 *
 *   TODO:
 *   - not sure if 8085 DSUB H flag is correct
 *   - accurate 8085 undocumented V/K flags
 *   - most of those is_8085 can probably be done better, like function overrides
 *
 * ---------------------------------------------------------------------------
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
 * - it passes on 8080/8085 CPU Exerciser (ref: http://www.idb.me.uk/sunhillow/8080.html
 *   tests only 8080 opcodes, link is dead so go via archive.org)
 *
 *****************************************************************************/

#include "emu.h"
#include "i8085.h"
#include "8085dasm.h"

#define VERBOSE 0
#include "logmacro.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

constexpr u8 SF             = 0x80;
constexpr u8 ZF             = 0x40;
constexpr u8 KF             = 0x20;
constexpr u8 HF             = 0x10;
constexpr u8 X3F            = 0x08;
constexpr u8 PF             = 0x04;
constexpr u8 VF             = 0x02;
constexpr u8 CF             = 0x01;

constexpr u8 IM_SID         = 0x80;
constexpr u8 IM_I75         = 0x40;
constexpr u8 IM_I65         = 0x20;
constexpr u8 IM_I55         = 0x10;
constexpr u8 IM_IE          = 0x08;
constexpr u8 IM_M75         = 0x04;
constexpr u8 IM_M65         = 0x02;
constexpr u8 IM_M55         = 0x01;

constexpr u16 ADDR_TRAP     = 0x0024;
constexpr u16 ADDR_RST55    = 0x002c;
constexpr u16 ADDR_RST65    = 0x0034;
constexpr u16 ADDR_RST75    = 0x003c;


/***************************************************************************
    STATIC TABLES
***************************************************************************/

/* cycles lookup */
const u8 i8085a_cpu_device::lut_cycles_8080[256]={
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
/* E */ 5, 10,10,18,11,11,7, 11,5, 5, 10,4, 11,11,7, 11,
/* F */ 5, 10,10,4, 11,11,7, 11,5, 5, 10,4, 11,11,7, 11 };

const u8 i8085a_cpu_device::lut_cycles_8085[256]={
/*      0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F  */
/* 0 */ 4, 10,7, 6, 4, 4, 7, 4, 10,10,7, 6, 4, 4, 7, 4,
/* 1 */ 7, 10,7, 6, 4, 4, 7, 4, 10,10,7, 6, 4, 4, 7, 4,
/* 2 */ 4, 10,16,6, 4, 4, 7, 4, 10,10,16,6, 4, 4, 7, 4,
/* 3 */ 4, 10,13,6, 10,10,10,4, 10,10,13,6, 4, 4, 7, 4,
/* 4 */ 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
/* 5 */ 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
/* 6 */ 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
/* 7 */ 7, 7, 7, 7, 7, 7, 5, 7, 4, 4, 4, 4, 4, 4, 7, 4,
/* 8 */ 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
/* 9 */ 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
/* A */ 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
/* B */ 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
/* C */ 6, 10,7, 7, 9, 12,7, 12,6, 10,7, 6, 9, 9, 7, 12,
/* D */ 6, 10,7, 10,9, 12,7, 12,6, 10,7, 10,9, 7, 7, 12,
/* E */ 6, 10,7, 16,9, 12,7, 12,6, 6, 7, 4, 9, 10,7, 12,
/* F */ 6, 10,7, 4, 9, 12,7, 12,6, 6, 7, 4, 9, 7, 7, 12 };

/* special cases (partially taken care of elsewhere):
                base c    taken?
op_ret  8080    5         +6(11)    (conditional)
op_ret  8085    6         +6(12)    (conditional)
op_jmp  8080    10        +0
op_jmp  8085    7         +3(10)
op_call 8080    11        +6(17)
op_call 8085    9         +9(18)

*/


DEFINE_DEVICE_TYPE(I8080,  i8080_cpu_device,  "i8080",  "Intel 8080")
DEFINE_DEVICE_TYPE(I8080A, i8080a_cpu_device, "i8080a", "Intel 8080A")
DEFINE_DEVICE_TYPE(I8085A, i8085a_cpu_device, "i8085a", "Intel 8085A")


i8085a_cpu_device::i8085a_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 16, 0)
	, m_io_config("io", ENDIANNESS_LITTLE, 8, 8, 0)
	, m_opcode_config("opcodes", ENDIANNESS_LITTLE, 8, 16, 0)
	, m_in_inta_func(*this, 0)
	, m_out_status_func(*this)
	, m_out_inte_func(*this)
	, m_in_sid_func(*this, 0)
	, m_out_sod_func(*this)
	, m_clk_out_func(*this)
{ }

i8085a_cpu_device::i8085a_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: i8085a_cpu_device(mconfig, I8085A, tag, owner, clock)
{ }

i8080_cpu_device::i8080_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: i8085a_cpu_device(mconfig, type, tag, owner, clock)
{ }

i8080_cpu_device::i8080_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: i8080_cpu_device(mconfig, I8080, tag, owner, clock)
{ }

i8080a_cpu_device::i8080a_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: i8080_cpu_device(mconfig, I8080A, tag, owner, clock)
{ }


device_memory_interface::space_config_vector i8085a_cpu_device::memory_space_config() const
{
	return has_configured_map(AS_OPCODES) ? space_config_vector
	{
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_IO,      &m_io_config),
		std::make_pair(AS_OPCODES, &m_opcode_config)
	} : space_config_vector
	{
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_IO,      &m_io_config)
	};
}


void i8085a_cpu_device::device_config_complete()
{
	m_clk_out_func.resolve_safe();
	m_clk_out_func(clock() / 2);
}


void i8085a_cpu_device::device_clock_changed()
{
	m_clk_out_func(clock() / 2);
}


/***************************************************************************
    CORE INITIALIZATION
***************************************************************************/

void i8085a_cpu_device::init_tables()
{
	for (int i = 0; i < 256; i++)
	{
		/* cycles */
		lut_cycles[i] = is_8085() ? lut_cycles_8085[i] : lut_cycles_8080[i];

		/* flags */
		u8 zs = 0;
		if (i == 0) zs |= ZF;
		if (i & 0x80) zs |= SF;

		u8 p = (population_count_32(i) & 1) ? 0 : PF;

		lut_zs[i] = zs;
		lut_zsp[i] = zs | p;
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
	m_halt = 0;
	m_im = 0;
	m_status = 0;
	m_after_ei = 0;
	m_nmi_state = 0;
	m_irq_state[3] = m_irq_state[2] = m_irq_state[1] = m_irq_state[0] = 0;
	m_trap_pending = false;
	m_trap_im_copy = 0;
	m_sod_state = 1; // SOD will go low at reset
	m_in_acknowledge = false;
	m_ietemp = 0;

	init_tables();

	/* set up the state table */
	state_add(I8085_PC,     "PC",     m_PC.w.l);
	state_add(STATE_GENPC,  "GENPC",  m_PC.w.l).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_PC.w.l).noshow();
	state_add(I8085_SP,     "SP",     m_SP.w.l);
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
	if (is_8085())
	{
		state_add(I8085_IM,     "IM",     m_im);
		state_add(I8085_SOD,    "SOD",    m_sod_state).mask(0x1);
		state_add(I8085_SID,    "SID",    m_ietemp).mask(0x1).callimport().callexport();
	}
	else
	{
		state_add(I8085_STATUS, "STATUS", m_status);
		state_add(I8085_INTE,   "INTE",   m_ietemp).mask(0x1).callimport().callexport();
	}

	space(AS_PROGRAM).cache(m_cprogram);
	space(AS_PROGRAM).specific(m_program);
	space(has_space(AS_OPCODES) ? AS_OPCODES : AS_PROGRAM).cache(m_copcodes);
	space(AS_IO).specific(m_io);

	/* register for state saving */
	save_item(NAME(m_PC.w.l));
	save_item(NAME(m_SP.w.l));
	save_item(NAME(m_AF.w.l));
	save_item(NAME(m_BC.w.l));
	save_item(NAME(m_DE.w.l));
	save_item(NAME(m_HL.w.l));
	save_item(NAME(m_halt));
	save_item(NAME(m_im));
	save_item(NAME(m_status));
	save_item(NAME(m_after_ei));
	save_item(NAME(m_nmi_state));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_trap_pending));
	save_item(NAME(m_trap_im_copy));
	save_item(NAME(m_sod_state));
	save_item(NAME(m_in_acknowledge));

	set_icountptr(m_icount);
}


/***************************************************************************
    COMMON RESET
***************************************************************************/

void i8085a_cpu_device::device_reset()
{
	m_PC.d = 0;
	m_halt = 0;
	m_im &= ~IM_I75;
	m_im |= IM_M55 | IM_M65 | IM_M75;
	m_after_ei = 0;
	m_trap_pending = false;
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
				m_im |= IM_SID;
			}
			else
			{
				m_im &= ~IM_SID;
			}
			break;

		case I8085_INTE:
			if (m_ietemp)
			{
				m_im |= IM_IE;
			}
			else
			{
				m_im &= ~IM_IE;
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
			m_ietemp = ((m_im & IM_SID) && m_in_sid_func()) ? 1 : 0;
			break;

		case I8085_INTE:
			m_ietemp = (m_im & IM_IE) ? 1 : 0;
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
			str = string_format("%c%c%c%c.%c%c%c",
				m_AF.b.l & 0x80 ? 'S':'.',
				m_AF.b.l & 0x40 ? 'Z':'.',
				m_AF.b.l & 0x20 ? 'K':'.', // X5
				m_AF.b.l & 0x10 ? 'H':'.',
				m_AF.b.l & 0x04 ? 'P':'.',
				m_AF.b.l & 0x02 ? 'V':'.',
				m_AF.b.l & 0x01 ? 'C':'.');
			break;
	}
}

void i8080_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c.%c.%c.%c",
				m_AF.b.l & 0x80 ? 'S':'.',
				m_AF.b.l & 0x40 ? 'Z':'.',
				m_AF.b.l & 0x10 ? 'H':'.',
				m_AF.b.l & 0x04 ? 'P':'.',
				m_AF.b.l & 0x01 ? 'C':'.');
			break;
	}
}

std::unique_ptr<util::disasm_interface> i8085a_cpu_device::create_disassembler()
{
	return std::make_unique<i8085_disassembler>();
}


/***************************************************************************
    INTERRUPTS
***************************************************************************/

void i8085a_cpu_device::execute_set_input(int irqline, int state)
{
	int newstate = (state != CLEAR_LINE);

	/* TRAP is level and edge-triggered NMI */
	if (irqline == I8085_TRAP_LINE)
	{
		if (!m_nmi_state && newstate)
			m_trap_pending = true;
		else if (!newstate)
			m_trap_pending = false;
		m_nmi_state = newstate;
	}

	/* RST7.5 is edge-triggered */
	else if (irqline == I8085_RST75_LINE)
	{
		if (!m_irq_state[I8085_RST75_LINE] && newstate)
			m_im |= IM_I75;
		m_irq_state[I8085_RST75_LINE] = newstate;
	}

	/* remaining sources are level triggered */
	else if (irqline < std::size(m_irq_state))
		m_irq_state[irqline] = state;
}

void i8085a_cpu_device::break_halt_for_interrupt()
{
	/* de-halt if necessary */
	if (m_halt)
	{
		m_PC.w.l++;
		m_halt = 0;
		set_status(0x26); /* int ack while halt */
	}
	else
		set_status(0x23); /* int ack */

	m_in_acknowledge = true;
}

void i8085a_cpu_device::check_for_interrupts()
{
	/* TRAP is the highest priority */
	if (m_trap_pending)
	{
		/* the first RIM after a TRAP reflects the original IE state; remember it here,
		   setting the high bit to indicate it is valid */
		m_trap_im_copy = m_im | 0x80;

		/* reset the pending state */
		m_trap_pending = false;

		/* break out of HALT state and call the IRQ ack callback */
		break_halt_for_interrupt();
		standard_irq_callback(I8085_TRAP_LINE, m_PC.w.l);

		/* push the PC and jump to $0024 */
		op_push(m_PC);
		set_inte(0);
		m_PC.w.l = ADDR_TRAP;
		m_icount -= 11;
	}

	/* followed by RST7.5 */
	else if ((m_im & IM_I75) && !(m_im & IM_M75) && (m_im & IM_IE))
	{
		/* reset the pending state (which is CPU-visible via the RIM instruction) */
		m_im &= ~IM_I75;

		/* break out of HALT state and call the IRQ ack callback */
		break_halt_for_interrupt();
		standard_irq_callback(I8085_RST75_LINE, m_PC.w.l);

		/* push the PC and jump to $003C */
		op_push(m_PC);
		set_inte(0);
		m_PC.w.l = ADDR_RST75;
		m_icount -= 11;
	}

	/* followed by RST6.5 */
	else if (m_irq_state[I8085_RST65_LINE] && !(m_im & IM_M65) && (m_im & IM_IE))
	{
		/* break out of HALT state and call the IRQ ack callback */
		break_halt_for_interrupt();
		standard_irq_callback(I8085_RST65_LINE, m_PC.w.l);

		/* push the PC and jump to $0034 */
		op_push(m_PC);
		set_inte(0);
		m_PC.w.l = ADDR_RST65;
		m_icount -= 11;
	}

	/* followed by RST5.5 */
	else if (m_irq_state[I8085_RST55_LINE] && !(m_im & IM_M55) && (m_im & IM_IE))
	{
		/* break out of HALT state and call the IRQ ack callback */
		break_halt_for_interrupt();
		standard_irq_callback(I8085_RST55_LINE, m_PC.w.l);

		/* push the PC and jump to $002C */
		op_push(m_PC);
		set_inte(0);
		m_PC.w.l = ADDR_RST55;
		m_icount -= 11;
	}

	/* followed by classic INTR */
	else if (m_irq_state[I8085_INTR_LINE] && (m_im & IM_IE))
	{
		/* break out of HALT state and call the IRQ ack callback */
		if (!m_in_inta_func.isunset())
			standard_irq_callback(I8085_INTR_LINE, m_PC.w.l);
		break_halt_for_interrupt();

		u8 vector = read_inta();

		/* use the resulting vector as an opcode to execute */
		set_inte(0);
		LOG("i8085 take int $%02x\n", vector);
		execute_one(vector);
	}
}


/***************************************************************************
    OPCODE HELPERS
***************************************************************************/

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
	if (state != 0 && (m_im & IM_IE) == 0)
	{
		m_im |= IM_IE;
		m_out_inte_func(1);
	}
	else if (state == 0 && (m_im & IM_IE) != 0)
	{
		m_im &= ~IM_IE;
		m_out_inte_func(0);
	}
}

void i8085a_cpu_device::set_status(u8 status)
{
	if (status != m_status)
		m_out_status_func(status);

	m_status = status;
}

u8 i8085a_cpu_device::get_rim_value()
{
	u8 result = m_im;
	int sid = m_in_sid_func();

	/* copy live RST5.5 and RST6.5 states */
	result &= ~(IM_I65 | IM_I55);
	if (m_irq_state[I8085_RST65_LINE]) result |= IM_I65;
	if (m_irq_state[I8085_RST55_LINE]) result |= IM_I55;

	/* fetch the SID bit if we have a callback */
	result = (result & ~IM_SID) | (sid ? IM_SID : 0);

	return result;
}

// memory access
u8 i8085a_cpu_device::read_arg()
{
	set_status(0x82); // memory read
	if (m_in_acknowledge)
		return read_inta();
	else
		return m_cprogram.read_byte(m_PC.w.l++);
}

PAIR i8085a_cpu_device::read_arg16()
{
	PAIR p;
	set_status(0x82); // memory read
	if (m_in_acknowledge)
	{
		p.b.l = read_inta();
		p.b.h = read_inta();
	}
	else
	{
		p.b.l = m_cprogram.read_byte(m_PC.w.l++);
		p.b.h = m_cprogram.read_byte(m_PC.w.l++);
	}
	return p;
}

u8 i8085a_cpu_device::read_op()
{
	set_status(0xa2); // instruction fetch
	return m_copcodes.read_byte(m_PC.w.l++);
}

u8 i8085a_cpu_device::read_inta()
{
	if (m_in_inta_func.isunset())
		return standard_irq_callback(I8085_INTR_LINE, m_PC.w.l);
	else
		return m_in_inta_func(m_PC.w.l);
}

u8 i8085a_cpu_device::read_mem(u32 a)
{
	set_status(0x82); // memory read
	return m_program.read_byte(a);
}

void i8085a_cpu_device::write_mem(u32 a, u8 v)
{
	set_status(0x00); // memory write
	m_program.write_byte(a, v);
}

void i8085a_cpu_device::op_push(PAIR p)
{
	set_status(0x04); // stack push
	m_program.write_byte(--m_SP.w.l, p.b.h);
	m_program.write_byte(--m_SP.w.l, p.b.l);
}

PAIR i8085a_cpu_device::op_pop()
{
	PAIR p;
	set_status(0x86); // stack pop
	p.b.l = m_program.read_byte(m_SP.w.l++);
	p.b.h = m_program.read_byte(m_SP.w.l++);
	return p;
}

// logical
void i8085a_cpu_device::op_ora(u8 v)
{
	m_AF.b.h |= v;
	m_AF.b.l = lut_zsp[m_AF.b.h];
}

void i8085a_cpu_device::op_xra(u8 v)
{
	m_AF.b.h ^= v;
	m_AF.b.l = lut_zsp[m_AF.b.h];
}

void i8085a_cpu_device::op_ana(u8 v)
{
	u8 hc = ((m_AF.b.h | v) << 1) & HF;
	m_AF.b.h &= v;
	m_AF.b.l = lut_zsp[m_AF.b.h];
	if (is_8085())
		m_AF.b.l |= HF;
	else
		m_AF.b.l |= hc;
}

// increase / decrease
u8 i8085a_cpu_device::op_inr(u8 v)
{
	u8 hc = ((v & 0x0f) == 0x0f) ? HF : 0;
	m_AF.b.l = (m_AF.b.l & CF) | lut_zsp[++v] | hc;
	return v;
}

u8 i8085a_cpu_device::op_dcr(u8 v)
{
	u8 hc = ((v & 0x0f) != 0x00) ? HF : 0;
	m_AF.b.l = (m_AF.b.l & CF) | lut_zsp[--v] | hc | VF;
	return v;
}

// arithmetic
void i8085a_cpu_device::op_add(u8 v)
{
	int q = m_AF.b.h + v;
	m_AF.b.l = lut_zsp[q & 0xff] | ((q >> 8) & CF) | ((m_AF.b.h ^ q ^ v) & HF);
	m_AF.b.h = q;
}

void i8085a_cpu_device::op_adc(u8 v)
{
	int q = m_AF.b.h + v + (m_AF.b.l & CF);
	m_AF.b.l = lut_zsp[q & 0xff] | ((q >> 8) & CF) | ((m_AF.b.h ^ q ^ v) & HF);
	m_AF.b.h = q;
}

void i8085a_cpu_device::op_sub(u8 v)
{
	int q = m_AF.b.h - v;
	m_AF.b.l = lut_zsp[q & 0xff] | ((q >> 8) & CF) | (~(m_AF.b.h ^ q ^ v) & HF) | VF;
	m_AF.b.h = q;
}

void i8085a_cpu_device::op_sbb(u8 v)
{
	int q = m_AF.b.h - v - (m_AF.b.l & CF);
	m_AF.b.l = lut_zsp[q & 0xff] | ((q >> 8) & CF) | (~(m_AF.b.h ^ q ^ v) & HF) | VF;
	m_AF.b.h = q;
}

void i8085a_cpu_device::op_cmp(u8 v)
{
	int q = m_AF.b.h - v;
	m_AF.b.l = lut_zsp[q & 0xff] | ((q >> 8) & CF) | (~(m_AF.b.h ^ q ^ v) & HF) | VF;
}

void i8085a_cpu_device::op_dad(u16 v)
{
	int q = m_HL.w.l + v;
	m_AF.b.l = (m_AF.b.l & ~CF) | (q >> 16 & CF);
	m_HL.w.l = q;
}

// jumps
void i8085a_cpu_device::op_jmp(int cond)
{
	if (cond)
	{
		m_PC = read_arg16();
		m_icount -= jmp_taken();
	}
	else
	{
		m_PC.w.l += 2;
	}
}

void i8085a_cpu_device::op_call(int cond)
{
	if (cond)
	{
		PAIR p = read_arg16();
		m_icount -= call_taken();
		op_push(m_PC);
		m_PC = p;
	}
	else
	{
		m_PC.w.l += 2;
	}
}

void i8085a_cpu_device::op_ret(int cond)
{
	// conditional RET only
	if (cond)
	{
		m_icount -= ret_taken();
		m_PC = op_pop();
	}
}

void i8085a_cpu_device::op_rst(u8 v)
{
	op_push(m_PC);
	m_PC.d = 8 * v;
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
		/* the instruction after an EI does not take an interrupt, so
		   we cannot check immediately; handle post-EI behavior here */
		if (m_after_ei != 0 && --m_after_ei == 0)
			check_for_interrupts();

		m_in_acknowledge = false;
		debugger_instruction_hook(m_PC.d);

		/* here we go... */
		execute_one(read_op());

	} while (m_icount > 0);
}

void i8085a_cpu_device::execute_one(int opcode)
{
	m_icount -= lut_cycles[opcode];

	switch (opcode)
	{
		case 0x00: // NOP
			break;
		case 0x01: // LXI B,nnnn
			m_BC = read_arg16();
			break;
		case 0x02: // STAX B
			write_mem(m_BC.d, m_AF.b.h);
			break;
		case 0x03: // INX B
			m_BC.w.l++;
			if (is_8085())
			{
				if (m_BC.w.l == 0x0000)
					m_AF.b.l |= KF;
				else
					m_AF.b.l &= ~KF;
			}
			break;
		case 0x04: // INR B
			m_BC.b.h = op_inr(m_BC.b.h);
			break;
		case 0x05: // DCR B
			m_BC.b.h = op_dcr(m_BC.b.h);
			break;
		case 0x06: // MVI B,nn
			m_BC.b.h = read_arg();
			break;
		case 0x07: // RLC
			m_AF.b.h = (m_AF.b.h << 1) | (m_AF.b.h >> 7);
			m_AF.b.l = (m_AF.b.l & 0xfe) | (m_AF.b.h & CF);
			break;

		case 0x08: // 8085: undocumented DSUB, otherwise undocumented NOP
			if (is_8085())
			{
				int q = m_HL.b.l - m_BC.b.l;
				m_AF.b.l = lut_zs[q & 0xff] | ((q >> 8) & CF) | VF | ((m_HL.b.l ^ q ^ m_BC.b.l) & HF) | (((m_BC.b.l ^ m_HL.b.l) & (m_HL.b.l ^ q) & SF) >> 5);
				m_HL.b.l = q;
				q = m_HL.b.h - m_BC.b.h - (m_AF.b.l & CF);
				m_AF.b.l = lut_zs[q & 0xff] | ((q >> 8) & CF) | VF | ((m_HL.b.h ^ q ^ m_BC.b.h) & HF) | (((m_BC.b.h ^ m_HL.b.h) & (m_HL.b.h ^ q) & SF) >> 5);
				if (m_HL.b.l != 0)
					m_AF.b.l &= ~ZF;
			}
			break;
		case 0x09: // DAD B
			op_dad(m_BC.w.l);
			break;
		case 0x0a: // LDAX B
			m_AF.b.h = read_mem(m_BC.d);
			break;
		case 0x0b: // DCX B
			m_BC.w.l--;
			if (is_8085())
			{
				if (m_BC.w.l == 0xffff)
					m_AF.b.l |= KF;
				else
					m_AF.b.l &= ~KF;
			}
			break;
		case 0x0c: // INR C
			m_BC.b.l = op_inr(m_BC.b.l);
			break;
		case 0x0d: // DCR C
			m_BC.b.l = op_dcr(m_BC.b.l);
			break;
		case 0x0e: // MVI C,nn
			m_BC.b.l = read_arg();
			break;
		case 0x0f: // RRC
			m_AF.b.l = (m_AF.b.l & 0xfe) | (m_AF.b.h & CF);
			m_AF.b.h = (m_AF.b.h >> 1) | (m_AF.b.h << 7);
			break;

		case 0x10: // 8085: undocumented ARHL, otherwise undocumented NOP
			if (is_8085())
			{
				m_AF.b.l = (m_AF.b.l & ~CF) | (m_HL.b.l & CF);
				m_HL.w.l = (m_HL.w.l & 0x8000) | (m_HL.w.l >> 1);
			}
			break;
		case 0x11: // LXI D,nnnn
			m_DE = read_arg16();
			break;
		case 0x12: // STAX D
			write_mem(m_DE.d, m_AF.b.h);
			break;
		case 0x13: // INX D
			m_DE.w.l++;
			if (is_8085())
			{
				if (m_DE.w.l == 0x0000)
					m_AF.b.l |= KF;
				else
					m_AF.b.l &= ~KF;
			}
			break;
		case 0x14: // INR D
			m_DE.b.h = op_inr(m_DE.b.h);
			break;
		case 0x15: // DCR D
			m_DE.b.h = op_dcr(m_DE.b.h);
			break;
		case 0x16: // MVI D,nn
			m_DE.b.h = read_arg();
			break;
		case 0x17: // RAL
		{
			int c = m_AF.b.l & CF;
			m_AF.b.l = (m_AF.b.l & 0xfe) | (m_AF.b.h >> 7);
			m_AF.b.h = (m_AF.b.h << 1) | c;
			break;
		}

		case 0x18: // 8085: undocumented RDEL, otherwise undocumented NOP
			if (is_8085())
			{
				m_AF.b.l = (m_AF.b.l & ~(CF | VF)) | (m_DE.b.h >> 7);
				m_DE.w.l = (m_DE.w.l << 1) | (m_DE.w.l >> 15);
				if ((((m_DE.w.l >> 15) ^ m_AF.b.l) & CF) != 0)
					m_AF.b.l |= VF;
			}
			break;
		case 0x19: // DAD D
			op_dad(m_DE.w.l);
			break;
		case 0x1a: // LDAX D
			m_AF.b.h = read_mem(m_DE.d);
			break;
		case 0x1b: // DCX D
			m_DE.w.l--;
			if (is_8085())
			{
				if (m_DE.w.l == 0xffff)
					m_AF.b.l |= KF;
				else
					m_AF.b.l &= ~KF;
			}
			break;
		case 0x1c: // INR E
			m_DE.b.l = op_inr(m_DE.b.l);
			break;
		case 0x1d: // DCR E
			m_DE.b.l = op_dcr(m_DE.b.l);
			break;
		case 0x1e: // MVI E,nn
			m_DE.b.l = read_arg();
			break;
		case 0x1f: // RAR
		{
			int c = (m_AF.b.l & CF) << 7;
			m_AF.b.l = (m_AF.b.l & 0xfe) | (m_AF.b.h & CF);
			m_AF.b.h = (m_AF.b.h >> 1) | c;
			break;
		}

		case 0x20: // 8085: RIM, otherwise undocumented NOP
			if (is_8085())
			{
				m_AF.b.h = get_rim_value();

				// if we have remembered state from taking a TRAP, fix up the IE flag here
				if (m_trap_im_copy & 0x80)
					m_AF.b.h = (m_AF.b.h & ~IM_IE) | (m_trap_im_copy & IM_IE);
				m_trap_im_copy = 0;
			}
			break;
		case 0x21: // LXI H,nnnn
			m_HL = read_arg16();
			break;
		case 0x22: // SHLD nnnn
			m_WZ = read_arg16();
			write_mem(m_WZ.d, m_HL.b.l);
			m_WZ.w.l++;
			write_mem(m_WZ.d, m_HL.b.h);
			break;
		case 0x23: // INX H
			m_HL.w.l++;
			if (is_8085())
			{
				if (m_HL.w.l == 0x0000)
					m_AF.b.l |= KF;
				else
					m_AF.b.l &= ~KF;
			}
			break;
		case 0x24: // INR H
			m_HL.b.h = op_inr(m_HL.b.h);
			break;
		case 0x25: // DCR H
			m_HL.b.h = op_dcr(m_HL.b.h);
			break;
		case 0x26: // MVI H,nn
			m_HL.b.h = read_arg();
			break;
		case 0x27: // DAA
			m_WZ.b.h = m_AF.b.h;
			if ((m_AF.b.l & HF) || ((m_AF.b.h & 0xf) > 9))
				m_WZ.b.h += 6;
			if ((m_AF.b.l & CF) || (m_AF.b.h > 0x99))
				m_WZ.b.h += 0x60;

			m_AF.b.l = (m_AF.b.l & 0x23) | ((m_AF.b.h > 0x99) ? 1 : 0) | ((m_AF.b.h ^ m_WZ.b.h) & 0x10) | lut_zsp[m_WZ.b.h];
			m_AF.b.h = m_WZ.b.h;
			break;

		case 0x28: // 8085: undocumented LDHI nn, otherwise undocumented NOP
			if (is_8085())
			{
				m_WZ.d = read_arg();
				m_DE.d = (m_HL.d + m_WZ.d) & 0xffff;
			}
			break;
		case 0x29: // DAD H
			op_dad(m_HL.w.l);
			break;
		case 0x2a: // LHLD nnnn
			m_WZ = read_arg16();
			m_HL.b.l = read_mem(m_WZ.d);
			m_WZ.w.l++;
			m_HL.b.h = read_mem(m_WZ.d);
			break;
		case 0x2b: // DCX H
			m_HL.w.l--;
			if (is_8085())
			{
				if (m_HL.w.l == 0xffff)
					m_AF.b.l |= KF;
				else
					m_AF.b.l &= ~KF;
			}
			break;
		case 0x2c: // INR L
			m_HL.b.l = op_inr(m_HL.b.l);
			break;
		case 0x2d: // DCR L
			m_HL.b.l = op_dcr(m_HL.b.l);
			break;
		case 0x2e: // MVI L,nn
			m_HL.b.l = read_arg();
			break;
		case 0x2f: // CMA
			m_AF.b.h ^= 0xff;
			if (is_8085())
				m_AF.b.l |= VF;
			break;

		case 0x30: // 8085: SIM, otherwise undocumented NOP
			if (is_8085())
			{
				// if bit 3 is set, bits 0-2 become the new masks
				if (m_AF.b.h & 0x08)
				{
					m_im &= ~(IM_M55 | IM_M65 | IM_M75 | IM_I55 | IM_I65);
					m_im |= m_AF.b.h & (IM_M55 | IM_M65 | IM_M75);

					// update live state based on the new masks
					if ((m_im & IM_M55) == 0 && m_irq_state[I8085_RST55_LINE])
						m_im |= IM_I55;
					if ((m_im & IM_M65) == 0 && m_irq_state[I8085_RST65_LINE])
						m_im |= IM_I65;
				}

				// bit if 4 is set, the 7.5 flip-flop is cleared
				if (m_AF.b.h & 0x10)
					m_im &= ~IM_I75;

				// if bit 6 is set, then bit 7 is the new SOD state
				if (m_AF.b.h & 0x40)
					set_sod(m_AF.b.h >> 7);

				// check for revealed interrupts
				check_for_interrupts();
			}
			break;
		case 0x31: // LXI SP,nnnn
			m_SP = read_arg16();
			break;
		case 0x32: // STAX nnnn
			m_WZ = read_arg16();
			write_mem(m_WZ.d, m_AF.b.h);
			break;
		case 0x33: // INX SP
			m_SP.w.l++;
			if (is_8085())
			{
				if (m_SP.w.l == 0x0000)
					m_AF.b.l |= KF;
				else
					m_AF.b.l &= ~KF;
			}
			break;
		case 0x34: // INR M
			m_WZ.b.l = op_inr(read_mem(m_HL.d));
			write_mem(m_HL.d, m_WZ.b.l);
			break;
		case 0x35: // DCR M
			m_WZ.b.l = op_dcr(read_mem(m_HL.d));
			write_mem(m_HL.d, m_WZ.b.l);
			break;
		case 0x36: // MVI M,nn
			m_WZ.b.l = read_arg();
			write_mem(m_HL.d, m_WZ.b.l);
			break;
		case 0x37: // STC
			m_AF.b.l = (m_AF.b.l & 0xfe) | CF;
			break;

		case 0x38: // 8085: undocumented LDSI nn, otherwise undocumented NOP
			if (is_8085())
			{
				m_WZ.d = read_arg();
				m_DE.d = (m_SP.d + m_WZ.d) & 0xffff;
			}
			break;
		case 0x39: // DAD SP
			op_dad(m_SP.w.l);
			break;
		case 0x3a: // LDAX nnnn
			m_WZ = read_arg16();
			m_AF.b.h = read_mem(m_WZ.d);
			break;
		case 0x3b: // DCX SP
			m_SP.w.l--;
			if (is_8085())
			{
				if (m_SP.w.l == 0xffff)
					m_AF.b.l |= KF;
				else
					m_AF.b.l &= ~KF;
			}
			break;
		case 0x3c: // INR A
			m_AF.b.h = op_inr(m_AF.b.h);
			break;
		case 0x3d: // DCR A
			m_AF.b.h = op_dcr(m_AF.b.h);
			break;
		case 0x3e: // MVI A,nn
			m_AF.b.h = read_arg();
			break;
		case 0x3f: // CMC
			m_AF.b.l = (m_AF.b.l & 0xfe) | (~m_AF.b.l & CF);
			break;

		// MOV [B/C/D/E/H/L/M/A],[B/C/D/E/H/L/M/A]
		case 0x40: break; // MOV B,B
		case 0x41: m_BC.b.h = m_BC.b.l; break;
		case 0x42: m_BC.b.h = m_DE.b.h; break;
		case 0x43: m_BC.b.h = m_DE.b.l; break;
		case 0x44: m_BC.b.h = m_HL.b.h; break;
		case 0x45: m_BC.b.h = m_HL.b.l; break;
		case 0x46: m_BC.b.h = read_mem(m_HL.d); break;
		case 0x47: m_BC.b.h = m_AF.b.h; break;

		case 0x48: m_BC.b.l = m_BC.b.h; break;
		case 0x49: break; // MOV C,C
		case 0x4a: m_BC.b.l = m_DE.b.h; break;
		case 0x4b: m_BC.b.l = m_DE.b.l; break;
		case 0x4c: m_BC.b.l = m_HL.b.h; break;
		case 0x4d: m_BC.b.l = m_HL.b.l; break;
		case 0x4e: m_BC.b.l = read_mem(m_HL.d); break;
		case 0x4f: m_BC.b.l = m_AF.b.h; break;

		case 0x50: m_DE.b.h = m_BC.b.h; break;
		case 0x51: m_DE.b.h = m_BC.b.l; break;
		case 0x52: break; // MOV D,D
		case 0x53: m_DE.b.h = m_DE.b.l; break;
		case 0x54: m_DE.b.h = m_HL.b.h; break;
		case 0x55: m_DE.b.h = m_HL.b.l; break;
		case 0x56: m_DE.b.h = read_mem(m_HL.d); break;
		case 0x57: m_DE.b.h = m_AF.b.h; break;

		case 0x58: m_DE.b.l = m_BC.b.h; break;
		case 0x59: m_DE.b.l = m_BC.b.l; break;
		case 0x5a: m_DE.b.l = m_DE.b.h; break;
		case 0x5b: break; // MOV E,E
		case 0x5c: m_DE.b.l = m_HL.b.h; break;
		case 0x5d: m_DE.b.l = m_HL.b.l; break;
		case 0x5e: m_DE.b.l = read_mem(m_HL.d); break;
		case 0x5f: m_DE.b.l = m_AF.b.h; break;

		case 0x60: m_HL.b.h = m_BC.b.h; break;
		case 0x61: m_HL.b.h = m_BC.b.l; break;
		case 0x62: m_HL.b.h = m_DE.b.h; break;
		case 0x63: m_HL.b.h = m_DE.b.l; break;
		case 0x64: break; // MOV H,H
		case 0x65: m_HL.b.h = m_HL.b.l; break;
		case 0x66: m_HL.b.h = read_mem(m_HL.d); break;
		case 0x67: m_HL.b.h = m_AF.b.h; break;

		case 0x68: m_HL.b.l = m_BC.b.h; break;
		case 0x69: m_HL.b.l = m_BC.b.l; break;
		case 0x6a: m_HL.b.l = m_DE.b.h; break;
		case 0x6b: m_HL.b.l = m_DE.b.l; break;
		case 0x6c: m_HL.b.l = m_HL.b.h; break;
		case 0x6d: break; // MOV L,L
		case 0x6e: m_HL.b.l = read_mem(m_HL.d); break;
		case 0x6f: m_HL.b.l = m_AF.b.h; break;

		case 0x70: write_mem(m_HL.d, m_BC.b.h); break;
		case 0x71: write_mem(m_HL.d, m_BC.b.l); break;
		case 0x72: write_mem(m_HL.d, m_DE.b.h); break;
		case 0x73: write_mem(m_HL.d, m_DE.b.l); break;
		case 0x74: write_mem(m_HL.d, m_HL.b.h); break;
		case 0x75: write_mem(m_HL.d, m_HL.b.l); break;
		case 0x76: // HLT (instead of MOV M,M)
			m_PC.w.l--;
			m_halt = 1;
			set_status(0x8a); // halt acknowledge
			break;
		case 0x77: write_mem(m_HL.d, m_AF.b.h); break;

		case 0x78: m_AF.b.h = m_BC.b.h; break;
		case 0x79: m_AF.b.h = m_BC.b.l; break;
		case 0x7a: m_AF.b.h = m_DE.b.h; break;
		case 0x7b: m_AF.b.h = m_DE.b.l; break;
		case 0x7c: m_AF.b.h = m_HL.b.h; break;
		case 0x7d: m_AF.b.h = m_HL.b.l; break;
		case 0x7e: m_AF.b.h = read_mem(m_HL.d); break;
		case 0x7f: break; // MOV A,A

		// alu op [B/C/D/E/H/L/M/A]
		case 0x80: op_add(m_BC.b.h); break;
		case 0x81: op_add(m_BC.b.l); break;
		case 0x82: op_add(m_DE.b.h); break;
		case 0x83: op_add(m_DE.b.l); break;
		case 0x84: op_add(m_HL.b.h); break;
		case 0x85: op_add(m_HL.b.l); break;
		case 0x86: m_WZ.b.l = read_mem(m_HL.d); op_add(m_WZ.b.l); break;
		case 0x87: op_add(m_AF.b.h); break;

		case 0x88: op_adc(m_BC.b.h); break;
		case 0x89: op_adc(m_BC.b.l); break;
		case 0x8a: op_adc(m_DE.b.h); break;
		case 0x8b: op_adc(m_DE.b.l); break;
		case 0x8c: op_adc(m_HL.b.h); break;
		case 0x8d: op_adc(m_HL.b.l); break;
		case 0x8e: m_WZ.b.l = read_mem(m_HL.d); op_adc(m_WZ.b.l); break;
		case 0x8f: op_adc(m_AF.b.h); break;

		case 0x90: op_sub(m_BC.b.h); break;
		case 0x91: op_sub(m_BC.b.l); break;
		case 0x92: op_sub(m_DE.b.h); break;
		case 0x93: op_sub(m_DE.b.l); break;
		case 0x94: op_sub(m_HL.b.h); break;
		case 0x95: op_sub(m_HL.b.l); break;
		case 0x96: m_WZ.b.l = read_mem(m_HL.d); op_sub(m_WZ.b.l); break;
		case 0x97: op_sub(m_AF.b.h); break;

		case 0x98: op_sbb(m_BC.b.h); break;
		case 0x99: op_sbb(m_BC.b.l); break;
		case 0x9a: op_sbb(m_DE.b.h); break;
		case 0x9b: op_sbb(m_DE.b.l); break;
		case 0x9c: op_sbb(m_HL.b.h); break;
		case 0x9d: op_sbb(m_HL.b.l); break;
		case 0x9e: m_WZ.b.l = read_mem(m_HL.d); op_sbb(m_WZ.b.l); break;
		case 0x9f: op_sbb(m_AF.b.h); break;

		case 0xa0: op_ana(m_BC.b.h); break;
		case 0xa1: op_ana(m_BC.b.l); break;
		case 0xa2: op_ana(m_DE.b.h); break;
		case 0xa3: op_ana(m_DE.b.l); break;
		case 0xa4: op_ana(m_HL.b.h); break;
		case 0xa5: op_ana(m_HL.b.l); break;
		case 0xa6: m_WZ.b.l = read_mem(m_HL.d); op_ana(m_WZ.b.l); break;
		case 0xa7: op_ana(m_AF.b.h); break;

		case 0xa8: op_xra(m_BC.b.h); break;
		case 0xa9: op_xra(m_BC.b.l); break;
		case 0xaa: op_xra(m_DE.b.h); break;
		case 0xab: op_xra(m_DE.b.l); break;
		case 0xac: op_xra(m_HL.b.h); break;
		case 0xad: op_xra(m_HL.b.l); break;
		case 0xae: m_WZ.b.l = read_mem(m_HL.d); op_xra(m_WZ.b.l); break;
		case 0xaf: op_xra(m_AF.b.h); break;

		case 0xb0: op_ora(m_BC.b.h); break;
		case 0xb1: op_ora(m_BC.b.l); break;
		case 0xb2: op_ora(m_DE.b.h); break;
		case 0xb3: op_ora(m_DE.b.l); break;
		case 0xb4: op_ora(m_HL.b.h); break;
		case 0xb5: op_ora(m_HL.b.l); break;
		case 0xb6: m_WZ.b.l = read_mem(m_HL.d); op_ora(m_WZ.b.l); break;
		case 0xb7: op_ora(m_AF.b.h); break;

		case 0xb8: op_cmp(m_BC.b.h); break;
		case 0xb9: op_cmp(m_BC.b.l); break;
		case 0xba: op_cmp(m_DE.b.h); break;
		case 0xbb: op_cmp(m_DE.b.l); break;
		case 0xbc: op_cmp(m_HL.b.h); break;
		case 0xbd: op_cmp(m_HL.b.l); break;
		case 0xbe: m_WZ.b.l = read_mem(m_HL.d); op_cmp(m_WZ.b.l); break;
		case 0xbf: op_cmp(m_AF.b.h); break;

		case 0xc0: // RNZ
			op_ret(!(m_AF.b.l & ZF));
			break;
		case 0xc1: // POP B
			m_BC = op_pop();
			break;
		case 0xc2: // JNZ nnnn
			op_jmp(!(m_AF.b.l & ZF));
			break;
		case 0xc3: // JMP nnnn
			op_jmp(1);
			break;
		case 0xc4: // CNZ nnnn
			op_call(!(m_AF.b.l & ZF));
			break;
		case 0xc5: // PUSH B
			op_push(m_BC);
			break;
		case 0xc6: // ADI nn
			m_WZ.b.l = read_arg();
			op_add(m_WZ.b.l);
			break;
		case 0xc7: // RST 0
			op_rst(0);
			break;

		case 0xc8: // RZ
			op_ret(m_AF.b.l & ZF);
			break;
		case 0xc9: // RET
			m_PC = op_pop();
			break;
		case 0xca: // JZ  nnnn
			op_jmp(m_AF.b.l & ZF);
			break;
		case 0xcb: // 8085: undocumented RSTV, otherwise undocumented JMP nnnn
			if (is_8085())
			{
				if (m_AF.b.l & VF)
				{
					// RST taken = 6 more cycles
					m_icount -= ret_taken();
					op_rst(8);
				}
			}
			else
				op_jmp(1);
			break;
		case 0xcc: // CZ  nnnn
			op_call(m_AF.b.l & ZF);
			break;
		case 0xcd: // CALL nnnn
			op_call(1);
			break;
		case 0xce: // ACI nn
			m_WZ.b.l = read_arg();
			op_adc(m_WZ.b.l);
			break;
		case 0xcf: // RST 1
			op_rst(1);
			break;

		case 0xd0: // RNC
			op_ret(!(m_AF.b.l & CF));
			break;
		case 0xd1: // POP D
			m_DE = op_pop();
			break;
		case 0xd2: // JNC nnnn
			op_jmp(!(m_AF.b.l & CF));
			break;
		case 0xd3: // OUT nn
			set_status(0x10);
			m_WZ.d = read_arg();
			m_io.write_byte(m_WZ.d, m_AF.b.h);
			break;
		case 0xd4: // CNC nnnn
			op_call(!(m_AF.b.l & CF));
			break;
		case 0xd5: // PUSH D
			op_push(m_DE);
			break;
		case 0xd6: // SUI nn
			m_WZ.b.l = read_arg();
			op_sub(m_WZ.b.l);
			break;
		case 0xd7: // RST 2
			op_rst(2);
			break;

		case 0xd8: // RC
			op_ret(m_AF.b.l & CF);
			break;
		case 0xd9: // 8085: undocumented SHLX, otherwise undocumented RET
			if (is_8085())
			{
				m_WZ.w.l = m_DE.w.l;
				write_mem(m_WZ.d, m_HL.b.l);
				m_WZ.w.l++;
				write_mem(m_WZ.d, m_HL.b.h);
			}
			else
				m_PC = op_pop();
			break;
		case 0xda: // JC nnnn
			op_jmp(m_AF.b.l & CF);
			break;
		case 0xdb: // IN nn
			set_status(0x42);
			m_WZ.d = read_arg();
			m_AF.b.h = m_io.read_byte(m_WZ.d);
			break;
		case 0xdc: // CC nnnn
			op_call(m_AF.b.l & CF);
			break;
		case 0xdd: // 8085: undocumented JNX5 nnnn, otherwise undocumented CALL nnnn
			if (is_8085())
				op_jmp(!(m_AF.b.l & KF));
			else
				op_call(1);
			break;
		case 0xde: // SBI nn
			m_WZ.b.l = read_arg();
			op_sbb(m_WZ.b.l);
			break;
		case 0xdf: // RST 3
			op_rst(3);
			break;

		case 0xe0: // RPO
			op_ret(!(m_AF.b.l & PF));
			break;
		case 0xe1: // POP H
			m_HL = op_pop();
			break;
		case 0xe2: // JPO nnnn
			op_jmp(!(m_AF.b.l & PF));
			break;
		case 0xe3: // XTHL
			m_WZ = op_pop();
			op_push(m_HL);
			m_HL.d = m_WZ.d;
			break;
		case 0xe4: // CPO nnnn
			op_call(!(m_AF.b.l & PF));
			break;
		case 0xe5: // PUSH H
			op_push(m_HL);
			break;
		case 0xe6: // ANI nn
			m_WZ.b.l = read_arg();
			op_ana(m_WZ.b.l);
			break;
		case 0xe7: // RST 4
			op_rst(4);
			break;

		case 0xe8: // RPE
			op_ret(m_AF.b.l & PF);
			break;
		case 0xe9: // PCHL
			m_PC.d = m_HL.w.l;
			break;
		case 0xea: // JPE nnnn
			op_jmp(m_AF.b.l & PF);
			break;
		case 0xeb: // XCHG
			m_WZ.d = m_DE.d;
			m_DE.d = m_HL.d;
			m_HL.d = m_WZ.d;
			break;
		case 0xec: // CPE nnnn
			op_call(m_AF.b.l & PF);
			break;
		case 0xed: // 8085: undocumented LHLX, otherwise undocumented CALL nnnn
			if (is_8085())
			{
				m_WZ.w.l = m_DE.w.l;
				m_HL.b.l = read_mem(m_WZ.d);
				m_WZ.w.l++;
				m_HL.b.h = read_mem(m_WZ.d);
			}
			else
				op_call(1);
			break;
		case 0xee: // XRI nn
			m_WZ.b.l = read_arg();
			op_xra(m_WZ.b.l);
			break;
		case 0xef: // RST 5
			op_rst(5);
			break;

		case 0xf0: // RP
			op_ret(!(m_AF.b.l & SF));
			break;
		case 0xf1: // POP A
			m_AF = op_pop();
			break;
		case 0xf2: // JP nnnn
			op_jmp(!(m_AF.b.l & SF));
			break;
		case 0xf3: // DI
			set_inte(0);
			break;
		case 0xf4: // CP nnnn
			op_call(!(m_AF.b.l & SF));
			break;
		case 0xf5: // PUSH A
			// X3F is always 0, and on 8080, VF=1 and KF=0
			// (we don't have to check for it elsewhere)
			m_AF.b.l &= ~X3F;
			if (!is_8085())
				m_AF.b.l = (m_AF.b.l & ~KF) | VF;
			op_push(m_AF);
			break;
		case 0xf6: // ORI nn
			m_WZ.b.l = read_arg();
			op_ora(m_WZ.b.l);
			break;
		case 0xf7: // RST 6
			op_rst(6);
			break;

		case 0xf8: // RM
			op_ret(m_AF.b.l & SF);
			break;
		case 0xf9: // SPHL
			m_SP.d = m_HL.d;
			break;
		case 0xfa: // JM nnnn
			op_jmp(m_AF.b.l & SF);
			break;
		case 0xfb: // EI
			set_inte(1);
			m_after_ei = 2;
			break;
		case 0xfc: // CM nnnn
			op_call(m_AF.b.l & SF);
			break;
		case 0xfd: // 8085: undocumented JX5 nnnn, otherwise undocumented CALL nnnn
			if (is_8085())
				op_jmp(m_AF.b.l & KF);
			else
				op_call(1);
			break;
		case 0xfe: // CPI nn
			m_WZ.b.l = read_arg();
			op_cmp(m_WZ.b.l);
			break;
		case 0xff: // RST 7
			op_rst(7);
			break;
	} // end big switch
}
