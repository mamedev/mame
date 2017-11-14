// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller, hap
// thanks-to:Marcel De Kogel
/*****************************************************************************
 *
 *   i8085.c
 *   Portable I8085A emulator V1.2
 *
 *   Copyright Juergen Buchmueller, all rights reserved.
 *   Partially based on information out of Z80Em by Marcel De Kogel
 *
 *   TODO:
 *   - 8085 DAA fails on 8080/8085 CPU Exerciser
 *   - not sure if 8085 DSUB H flag is correct
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
 * - 8080 passes on 8080/8085 CPU Exerciser, 8085 errors only on the DAA test
 *   (ref: http://www.idb.me.uk/sunhillow/8080.html - tests only 8080 opcodes)
 *
 *****************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "i8085.h"

#define VERBOSE 0
#include "logmacro.h"

#define CPUTYPE_8080    0
#define CPUTYPE_8085    1



/***************************************************************************
    MACROS AND CONSTANTS
***************************************************************************/

#define IS_8080()          (m_cputype == CPUTYPE_8080)
#define IS_8085()          (m_cputype == CPUTYPE_8085)

#define SF              0x80
#define ZF              0x40
#define X5F             0x20
#define HF              0x10
#define X3F             0x08
#define PF              0x04
#define VF              0x02
#define CF              0x01

#define IM_SID          0x80
#define IM_I75          0x40
#define IM_I65          0x20
#define IM_I55          0x10
#define IM_IE           0x08
#define IM_M75          0x04
#define IM_M65          0x02
#define IM_M55          0x01

#define ADDR_TRAP       0x0024
#define ADDR_RST55      0x002c
#define ADDR_RST65      0x0034
#define ADDR_RST75      0x003c
#define ADDR_INTR       0x0038



/***************************************************************************
    STATIC TABLES
***************************************************************************/

/* cycles lookup */
const uint8_t i8085a_cpu_device::lut_cycles_8080[256]={
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
const uint8_t i8085a_cpu_device::lut_cycles_8085[256]={
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


DEFINE_DEVICE_TYPE(I8080,  i8080_cpu_device,  "i8080",  "8080")
DEFINE_DEVICE_TYPE(I8080A, i8080a_cpu_device, "i8080a", "8080A")
DEFINE_DEVICE_TYPE(I8085A, i8085a_cpu_device, "i8085a", "8085A")


i8085a_cpu_device::i8085a_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: i8085a_cpu_device(mconfig, I8085A, tag, owner, clock, CPUTYPE_8085)
{
}

i8085a_cpu_device::i8085a_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int cputype)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 16, 0)
	, m_io_config("io", ENDIANNESS_LITTLE, 8, 8, 0)
	, m_out_status_func(*this)
	, m_out_inte_func(*this)
	, m_in_sid_func(*this)
	, m_out_sod_func(*this)
	, m_cputype(cputype)
{
}

i8080_cpu_device::i8080_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: i8085a_cpu_device(mconfig, I8080, tag, owner, clock, CPUTYPE_8080)
{
}

i8080a_cpu_device::i8080a_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: i8085a_cpu_device(mconfig, I8080A, tag, owner, clock, CPUTYPE_8080)
{
}

device_memory_interface::space_config_vector i8085a_cpu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_IO,      &m_io_config)
	};
}


void i8085a_cpu_device::device_config_complete()
{
	m_clk_out_func.bind_relative_to(*owner());
	if (!m_clk_out_func.isnull())
		m_clk_out_func(clock() / 2);
}


void i8085a_cpu_device::device_clock_changed()
{
	if (!m_clk_out_func.isnull())
		m_clk_out_func(clock() / 2);
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


void i8085a_cpu_device::set_status(uint8_t status)
{
	if (status != m_STATUS)
		m_out_status_func(status);

	m_STATUS = status;
}


uint8_t i8085a_cpu_device::get_rim_value()
{
	uint8_t result = m_IM;
	int sid = m_in_sid_func();

	/* copy live RST5.5 and RST6.5 states */
	result &= ~(IM_I65 | IM_I55);
	if (m_irq_state[I8085_RST65_LINE]) result |= IM_I65;
	if (m_irq_state[I8085_RST55_LINE]) result |= IM_I55;

	/* fetch the SID bit if we have a callback */
	result = (result & ~IM_SID) | (sid ? IM_SID : 0);

	return result;
}

uint8_t i8085a_cpu_device::read_op()
{
	set_status(0xa2); // instruction fetch
	return m_direct->read_byte(m_PC.w.l++);
}

uint8_t i8085a_cpu_device::read_arg()
{
	return m_direct->read_byte(m_PC.w.l++);
}

uint16_t i8085a_cpu_device::read_arg16()
{
	uint16_t w;
	w  = m_direct->read_byte(m_PC.d);
	m_PC.w.l++;
	w += m_direct->read_byte(m_PC.d) << 8;
	m_PC.w.l++;
	return w;
}

uint8_t i8085a_cpu_device::read_mem(uint32_t a)
{
	set_status(0x82); // memory read
	return m_program->read_byte(a);
}

void i8085a_cpu_device::write_mem(uint32_t a, uint8_t v)
{
	set_status(0x00); // memory write
	m_program->write_byte(a, v);
}




/* logical */
#define M_ORA(R) m_AF.b.h|=R; m_AF.b.l=lut_zsp[m_AF.b.h]
#define M_XRA(R) m_AF.b.h^=R; m_AF.b.l=lut_zsp[m_AF.b.h]
#define M_ANA(R) {uint8_t hc = ((m_AF.b.h | R)<<1) & HF; m_AF.b.h&=R; m_AF.b.l=lut_zsp[m_AF.b.h]; if(IS_8085()) { m_AF.b.l |= HF; } else {m_AF.b.l |= hc; } }

/* increase / decrease */
#define M_INR(R) {uint8_t hc = ((R & 0x0f) == 0x0f) ? HF : 0; ++R; m_AF.b.l= (m_AF.b.l & CF ) | lut_zsp[R] | hc; }
#define M_DCR(R) {uint8_t hc = ((R & 0x0f) != 0x00) ? HF : 0; --R; m_AF.b.l= (m_AF.b.l & CF ) | lut_zsp[R] | hc | VF; }

/* arithmetic */
#define M_ADD(R) { \
	int q = m_AF.b.h+R; \
	m_AF.b.l=lut_zsp[q&255]|((q>>8)&CF)|((m_AF.b.h^q^R)&HF); \
	m_AF.b.h=q; \
}

#define M_ADC(R) { \
	int q = m_AF.b.h+R+(m_AF.b.l&CF); \
	m_AF.b.l=lut_zsp[q&255]|((q>>8)&CF)|((m_AF.b.h^q^R)&HF); \
	m_AF.b.h=q; \
}

#define M_SUB(R) { \
	int q = m_AF.b.h-R; \
	m_AF.b.l=lut_zsp[q&255]|((q>>8)&CF)|(~(m_AF.b.h^q^R)&HF)|VF; \
	m_AF.b.h=q; \
}

#define M_SBB(R) { \
	int q = m_AF.b.h-R-(m_AF.b.l&CF); \
	m_AF.b.l=lut_zsp[q&255]|((q>>8)&CF)|(~(m_AF.b.h^q^R)&HF)|VF; \
	m_AF.b.h=q; \
}

#define M_CMP(R) { \
	int q = m_AF.b.h-R; \
	m_AF.b.l=lut_zsp[q&255]|((q>>8)&CF)|(~(m_AF.b.h^q^R)&HF)|VF; \
}

#define M_DAD(R) { \
	int q = m_HL.d + m_##R.d; \
	m_AF.b.l = (m_AF.b.l & ~CF) | (q>>16 & CF ); \
	m_HL.w.l = q; \
}

/* stack */
#define M_PUSH(R) { \
	set_status(0x04); \
	m_program->write_byte(--m_SP.w.l, m_##R.b.h); \
	m_program->write_byte(--m_SP.w.l, m_##R.b.l); \
}

#define M_POP(R) { \
	set_status(0x86); \
	m_##R.b.l = m_program->read_byte(m_SP.w.l++); \
	m_##R.b.h = m_program->read_byte(m_SP.w.l++); \
}

/* jumps */
// On 8085 jump if condition is not satisfied is shorter
#define M_JMP(cc) { \
	if (cc) { \
		m_PC.w.l = read_arg16(); \
	} else { \
		m_PC.w.l += 2; \
		m_icount += (IS_8085()) ? 3 : 0; \
	} \
}

// On 8085 call if condition is not satisfied is 9 ticks
#define M_CALL(cc) \
{ \
	if (cc) \
	{ \
		uint16_t a = read_arg16(); \
		m_icount -= (IS_8085()) ? 7 : 6 ; \
		M_PUSH(PC); \
		m_PC.d = a; \
	} else { \
		m_PC.w.l += 2; \
		m_icount += (IS_8085()) ? 2 : 0; \
	} \
}

// conditional RET only
#define M_RET(cc) \
{ \
	if (cc) \
	{ \
		m_icount -= 6; \
		M_POP(PC); \
	} \
}

#define M_RST(nn) { \
	M_PUSH(PC); \
	m_PC.d = 8 * nn; \
}

/***************************************************************************
    INTERRUPTS
***************************************************************************/

void i8085a_cpu_device::execute_set_input(int irqline, int state)
{
	int newstate = (state != CLEAR_LINE);

	/* NMI is edge-triggered */
	if (irqline == INPUT_LINE_NMI)
	{
		if (!m_nmi_state && newstate)
			m_trap_pending = true;
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

void i8085a_cpu_device::check_for_interrupts()
{
	/* TRAP is the highest priority */
	if (m_trap_pending)
	{
		/* the first RIM after a TRAP reflects the original IE state; remember it here,
		   setting the high bit to indicate it is valid */
		m_trap_im_copy = m_IM | 0x80;

		/* reset the pending state */
		m_trap_pending = false;

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
		uint32_t vector;

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
				LOG("i8085 take int $%02x\n", vector);
				execute_one(vector & 0xff);
				break;
		}
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
			m_BC.w.l = read_arg16();
			break;
		case 0x02: // STAX B
			write_mem(m_BC.d, m_AF.b.h);
			break;
		case 0x03: // INX B
			m_BC.w.l++;
			if (IS_8085())
			{
				if (m_BC.w.l == 0x0000)
					m_AF.b.l |= X5F;
				else
					m_AF.b.l &= ~X5F;
			}
			break;
		case 0x04: // INR B
			M_INR(m_BC.b.h);
			break;
		case 0x05: // DCR B
			M_DCR(m_BC.b.h);
			break;
		case 0x06: // MVI B,nn
			m_BC.b.h = read_arg();
			break;
		case 0x07: // RLC
			m_AF.b.h = (m_AF.b.h << 1) | (m_AF.b.h >> 7);
			m_AF.b.l = (m_AF.b.l & 0xfe) | (m_AF.b.h & CF);
			break;

		case 0x08: // 8085: DSUB, otherwise undocumented NOP
			if (IS_8085())
			{
				int q = m_HL.b.l - m_BC.b.l;
				m_AF.b.l = lut_zs[q & 255] | ((q >> 8) & CF) | VF | ((m_HL.b.l ^ q ^ m_BC.b.l) & HF) | (((m_BC.b.l ^ m_HL.b.l) & (m_HL.b.l ^ q) & SF) >> 5);
				m_HL.b.l = q;
				q = m_HL.b.h - m_BC.b.h - (m_AF.b.l & CF);
				m_AF.b.l = lut_zs[q & 255] | ((q >> 8) & CF) | VF | ((m_HL.b.h ^ q ^ m_BC.b.h) & HF) | (((m_BC.b.h ^ m_HL.b.h) & (m_HL.b.h ^ q) & SF) >> 5);
				if (m_HL.b.l != 0 )
					m_AF.b.l &= ~ZF;
			}
			break;
		case 0x09: // DAD B
			M_DAD(BC);
			break;
		case 0x0a: // LDAX B
			m_AF.b.h = read_mem(m_BC.d);
			break;
		case 0x0b: // DCX B
			m_BC.w.l--;
			if (IS_8085())
			{
				if (m_BC.w.l == 0xffff)
					m_AF.b.l |= X5F;
				else
					m_AF.b.l &= ~X5F;
			}
			break;
		case 0x0c: // INR C
			M_INR(m_BC.b.l);
			break;
		case 0x0d: // DCR C
			M_DCR(m_BC.b.l);
			break;
		case 0x0e: // MVI C,nn
			m_BC.b.l = read_arg();
			break;
		case 0x0f: // RRC
			m_AF.b.l = (m_AF.b.l & 0xfe) | (m_AF.b.h & CF);
			m_AF.b.h = (m_AF.b.h >> 1) | (m_AF.b.h << 7);
			break;

		case 0x10: // 8085: ASRH, otherwise undocumented NOP
			if (IS_8085())
			{
				m_AF.b.l = (m_AF.b.l & ~CF) | (m_HL.b.l & CF);
				m_HL.w.l = (m_HL.w.l >> 1);
			}
			break;
		case 0x11: // LXI D,nnnn
			m_DE.w.l = read_arg16();
			break;
		case 0x12: // STAX D
			write_mem(m_DE.d, m_AF.b.h);
			break;
		case 0x13: // INX D
			m_DE.w.l++;
			if (IS_8085())
			{
				if (m_DE.w.l == 0x0000)
					m_AF.b.l |= X5F;
				else
					m_AF.b.l &= ~X5F;
			}
			break;
		case 0x14: // INR D
			M_INR(m_DE.b.h);
			break;
		case 0x15: // DCR D
			M_DCR(m_DE.b.h);
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

		case 0x18: // 8085: RLDE, otherwise undocumented NOP
			if (IS_8085())
			{
				m_AF.b.l = (m_AF.b.l & ~(CF | VF)) | (m_DE.b.h >> 7);
				m_DE.w.l = (m_DE.w.l << 1) | (m_DE.w.l >> 15);
				if ((((m_DE.w.l >> 15) ^ m_AF.b.l) & CF) != 0)
					m_AF.b.l |= VF;
			}
			break;
		case 0x19: // DAD D
			M_DAD(DE);
			break;
		case 0x1a: // LDAX D
			m_AF.b.h = read_mem(m_DE.d);
			break;
		case 0x1b: // DCX D
			m_DE.w.l--;
			if (IS_8085())
			{
				if (m_DE.w.l == 0xffff)
					m_AF.b.l |= X5F;
				else
					m_AF.b.l &= ~X5F;
			}
			break;
		case 0x1c: // INR E
			M_INR(m_DE.b.l);
			break;
		case 0x1d: // DCR E
			M_DCR(m_DE.b.l);
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
			if (IS_8085())
			{
				m_AF.b.h = get_rim_value();

				// if we have remembered state from taking a TRAP, fix up the IE flag here
				if (m_trap_im_copy & 0x80)
					m_AF.b.h = (m_AF.b.h & ~IM_IE) | (m_trap_im_copy & IM_IE);
				m_trap_im_copy = 0;
			}
			break;
		case 0x21: // LXI H,nnnn
			m_HL.w.l = read_arg16();
			break;
		case 0x22: // SHLD nnnn
			m_WZ.w.l = read_arg16();
			write_mem(m_WZ.d, m_HL.b.l);
			m_WZ.w.l++;
			write_mem(m_WZ.d, m_HL.b.h);
			break;
		case 0x23: // INX H
			m_HL.w.l++;
			if (IS_8085())
			{
				if (m_HL.w.l == 0x0000)
					m_AF.b.l |= X5F;
				else
					m_AF.b.l &= ~X5F;
			}
			break;
		case 0x24: // INR H
			M_INR(m_HL.b.h);
			break;
		case 0x25: // DCR H
			M_DCR(m_HL.b.h);
			break;
		case 0x26: // MVI H,nn
			m_HL.b.h = read_arg();
			break;
		case 0x27: // DAA
			m_WZ.b.h = m_AF.b.h;
			if (IS_8085() && m_AF.b.l & VF)
			{
				if ((m_AF.b.l & HF) || ((m_AF.b.h & 0xf) > 9))
					m_WZ.b.h -= 6;
				if ((m_AF.b.l & CF) || (m_AF.b.h > 0x99))
					m_WZ.b.h -= 0x60;
			}
			else
			{
				if ((m_AF.b.l & HF) || ((m_AF.b.h & 0xf) > 9))
					m_WZ.b.h += 6;
				if ((m_AF.b.l & CF) || (m_AF.b.h > 0x99))
					m_WZ.b.h += 0x60;
			}

			m_AF.b.l = (m_AF.b.l & 3) | (m_AF.b.h & 0x28) | ((m_AF.b.h > 0x99) ? 1 : 0) | ((m_AF.b.h ^ m_WZ.b.h) & 0x10) | lut_zsp[m_WZ.b.h];
			m_AF.b.h = m_WZ.b.h;
			break;

		case 0x28: // 8085: LDEH nn, otherwise undocumented NOP
			if (IS_8085())
			{
				m_WZ.d = read_arg();
				m_DE.d = (m_HL.d + m_WZ.d) & 0xffff;
			}
			break;
		case 0x29: // DAD H
			M_DAD(HL);
			break;
		case 0x2a: // LHLD nnnn
			m_WZ.d = read_arg16();
			m_HL.b.l = read_mem(m_WZ.d);
			m_WZ.w.l++;
			m_HL.b.h = read_mem(m_WZ.d);
			break;
		case 0x2b: // DCX H
			m_HL.w.l--;
			if (IS_8085())
			{
				if (m_HL.w.l == 0xffff)
					m_AF.b.l |= X5F;
				else
					m_AF.b.l &= ~X5F;
			}
			break;
		case 0x2c: // INR L
			M_INR(m_HL.b.l);
			break;
		case 0x2d: // DCR L
			M_DCR(m_HL.b.l);
			break;
		case 0x2e: // MVI L,nn
			m_HL.b.l = read_arg();
			break;
		case 0x2f: // CMA
			m_AF.b.h ^= 0xff;
			if (IS_8085())
				m_AF.b.l |= HF | VF;
			break;

		case 0x30: // 8085: SIM, otherwise undocumented NOP
			if (IS_8085())
			{
				// if bit 3 is set, bits 0-2 become the new masks
				if (m_AF.b.h & 0x08)
				{
					m_IM &= ~(IM_M55 | IM_M65 | IM_M75 | IM_I55 | IM_I65);
					m_IM |= m_AF.b.h & (IM_M55 | IM_M65 | IM_M75);

					// update live state based on the new masks
					if ((m_IM & IM_M55) == 0 && m_irq_state[I8085_RST55_LINE])
						m_IM |= IM_I55;
					if ((m_IM & IM_M65) == 0 && m_irq_state[I8085_RST65_LINE])
						m_IM |= IM_I65;
				}

				// bit if 4 is set, the 7.5 flip-flop is cleared
				if (m_AF.b.h & 0x10)
					m_IM &= ~IM_I75;

				// if bit 6 is set, then bit 7 is the new SOD state
				if (m_AF.b.h & 0x40)
					set_sod(m_AF.b.h >> 7);

				// check for revealed interrupts
				check_for_interrupts();
			}
			break;
		case 0x31: // LXI SP,nnnn
			m_SP.w.l = read_arg16();
			break;
		case 0x32: // STAX nnnn
			m_WZ.d = read_arg16();
			write_mem(m_WZ.d, m_AF.b.h);
			break;
		case 0x33: // INX SP
			m_SP.w.l++;
			if (IS_8085())
			{
				if (m_SP.w.l == 0x0000)
					m_AF.b.l |= X5F;
				else
					m_AF.b.l &= ~X5F;
			}
			break;
		case 0x34: // INR M
			m_WZ.b.l = read_mem(m_HL.d);
			M_INR(m_WZ.b.l);
			write_mem(m_HL.d, m_WZ.b.l);
			break;
		case 0x35: // DCR M
			m_WZ.b.l = read_mem(m_HL.d);
			M_DCR(m_WZ.b.l);
			write_mem(m_HL.d, m_WZ.b.l);
			break;
		case 0x36: // MVI M,nn
			m_WZ.b.l = read_arg();
			write_mem(m_HL.d, m_WZ.b.l);
			break;
		case 0x37: // STC
			m_AF.b.l = (m_AF.b.l & 0xfe) | CF;
			break;

		case 0x38: // 8085: LDES nn, otherwise undocumented NOP
			if (IS_8085())
			{
				m_WZ.d = read_arg();
				m_DE.d = (m_SP.d + m_WZ.d) & 0xffff;
			}
			break;
		case 0x39: // DAD SP
			M_DAD(SP);
			break;
		case 0x3a: // LDAX nnnn
			m_WZ.d = read_arg16();
			m_AF.b.h = read_mem(m_WZ.d);
			break;
		case 0x3b: // DCX SP
			m_SP.w.l--;
			if (IS_8085())
			{
				if (m_SP.w.l == 0xffff)
					m_AF.b.l |= X5F;
				else
					m_AF.b.l &= ~X5F;
			}
			break;
		case 0x3c: // INR A
			M_INR(m_AF.b.h);
			break;
		case 0x3d: // DCR A
			M_DCR(m_AF.b.h);
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
			m_HALT = 1;
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
		case 0x80: M_ADD(m_BC.b.h); break;
		case 0x81: M_ADD(m_BC.b.l); break;
		case 0x82: M_ADD(m_DE.b.h); break;
		case 0x83: M_ADD(m_DE.b.l); break;
		case 0x84: M_ADD(m_HL.b.h); break;
		case 0x85: M_ADD(m_HL.b.l); break;
		case 0x86: m_WZ.b.l = read_mem(m_HL.d); M_ADD(m_WZ.b.l); break;
		case 0x87: M_ADD(m_AF.b.h); break;

		case 0x88: M_ADC(m_BC.b.h); break;
		case 0x89: M_ADC(m_BC.b.l); break;
		case 0x8a: M_ADC(m_DE.b.h); break;
		case 0x8b: M_ADC(m_DE.b.l); break;
		case 0x8c: M_ADC(m_HL.b.h); break;
		case 0x8d: M_ADC(m_HL.b.l); break;
		case 0x8e: m_WZ.b.l = read_mem(m_HL.d); M_ADC(m_WZ.b.l); break;
		case 0x8f: M_ADC(m_AF.b.h); break;

		case 0x90: M_SUB(m_BC.b.h); break;
		case 0x91: M_SUB(m_BC.b.l); break;
		case 0x92: M_SUB(m_DE.b.h); break;
		case 0x93: M_SUB(m_DE.b.l); break;
		case 0x94: M_SUB(m_HL.b.h); break;
		case 0x95: M_SUB(m_HL.b.l); break;
		case 0x96: m_WZ.b.l = read_mem(m_HL.d); M_SUB(m_WZ.b.l); break;
		case 0x97: M_SUB(m_AF.b.h); break;

		case 0x98: M_SBB(m_BC.b.h); break;
		case 0x99: M_SBB(m_BC.b.l); break;
		case 0x9a: M_SBB(m_DE.b.h); break;
		case 0x9b: M_SBB(m_DE.b.l); break;
		case 0x9c: M_SBB(m_HL.b.h); break;
		case 0x9d: M_SBB(m_HL.b.l); break;
		case 0x9e: m_WZ.b.l = read_mem(m_HL.d); M_SBB(m_WZ.b.l); break;
		case 0x9f: M_SBB(m_AF.b.h); break;

		case 0xa0: M_ANA(m_BC.b.h); break;
		case 0xa1: M_ANA(m_BC.b.l); break;
		case 0xa2: M_ANA(m_DE.b.h); break;
		case 0xa3: M_ANA(m_DE.b.l); break;
		case 0xa4: M_ANA(m_HL.b.h); break;
		case 0xa5: M_ANA(m_HL.b.l); break;
		case 0xa6: m_WZ.b.l = read_mem(m_HL.d); M_ANA(m_WZ.b.l); break;
		case 0xa7: M_ANA(m_AF.b.h); break;

		case 0xa8: M_XRA(m_BC.b.h); break;
		case 0xa9: M_XRA(m_BC.b.l); break;
		case 0xaa: M_XRA(m_DE.b.h); break;
		case 0xab: M_XRA(m_DE.b.l); break;
		case 0xac: M_XRA(m_HL.b.h); break;
		case 0xad: M_XRA(m_HL.b.l); break;
		case 0xae: m_WZ.b.l = read_mem(m_HL.d); M_XRA(m_WZ.b.l); break;
		case 0xaf: M_XRA(m_AF.b.h); break;

		case 0xb0: M_ORA(m_BC.b.h); break;
		case 0xb1: M_ORA(m_BC.b.l); break;
		case 0xb2: M_ORA(m_DE.b.h); break;
		case 0xb3: M_ORA(m_DE.b.l); break;
		case 0xb4: M_ORA(m_HL.b.h); break;
		case 0xb5: M_ORA(m_HL.b.l); break;
		case 0xb6: m_WZ.b.l = read_mem(m_HL.d); M_ORA(m_WZ.b.l); break;
		case 0xb7: M_ORA(m_AF.b.h); break;

		case 0xb8: M_CMP(m_BC.b.h); break;
		case 0xb9: M_CMP(m_BC.b.l); break;
		case 0xba: M_CMP(m_DE.b.h); break;
		case 0xbb: M_CMP(m_DE.b.l); break;
		case 0xbc: M_CMP(m_HL.b.h); break;
		case 0xbd: M_CMP(m_HL.b.l); break;
		case 0xbe: m_WZ.b.l = read_mem(m_HL.d); M_CMP(m_WZ.b.l); break;
		case 0xbf: M_CMP(m_AF.b.h); break;

		case 0xc0: // RNZ
			M_RET(!(m_AF.b.l & ZF));
			break;
		case 0xc1: // POP B
			M_POP(BC);
			break;
		case 0xc2: // JNZ nnnn
			M_JMP(!(m_AF.b.l & ZF));
			break;
		case 0xc3: // JMP nnnn
			M_JMP(1);
			break;
		case 0xc4: // CNZ nnnn
			M_CALL(!(m_AF.b.l & ZF));
			break;
		case 0xc5: // PUSH B
			M_PUSH(BC);
			break;
		case 0xc6: // ADI nn
			m_WZ.b.l = read_arg();
			M_ADD(m_WZ.b.l);
			break;
		case 0xc7: // RST 0
			M_RST(0);
			break;

		case 0xc8: // RZ
			M_RET(m_AF.b.l & ZF);
			break;
		case 0xc9: // RET
			M_POP(PC);
			break;
		case 0xca: // JZ  nnnn
			M_JMP(m_AF.b.l & ZF);
			break;
		case 0xcb: // 8085: RST V, otherwise undocumented JMP nnnn
			if (IS_8085())
			{
				if (m_AF.b.l & VF)
				{
					M_RST(8);
				}
				else
					m_icount += 6; // RST not taken
			}
			else
			{
				M_JMP(1);
			}
			break;
		case 0xcc: // CZ  nnnn
			M_CALL(m_AF.b.l & ZF);
			break;
		case 0xcd: // CALL nnnn
			M_CALL(1);
			break;
		case 0xce: // ACI nn
			m_WZ.b.l = read_arg();
			M_ADC(m_WZ.b.l);
			break;
		case 0xcf: // RST 1
			M_RST(1);
			break;

		case 0xd0: // RNC
			M_RET(!(m_AF.b.l & CF));
			break;
		case 0xd1: // POP D
			M_POP(DE);
			break;
		case 0xd2: // JNC nnnn
			M_JMP(!(m_AF.b.l & CF));
			break;
		case 0xd3: // OUT nn
			set_status(0x10);
			m_WZ.d = read_arg();
			m_io->write_byte(m_WZ.d, m_AF.b.h);
			break;
		case 0xd4: // CNC nnnn
			M_CALL(!(m_AF.b.l & CF));
			break;
		case 0xd5: // PUSH D
			M_PUSH(DE);
			break;
		case 0xd6: // SUI nn
			m_WZ.b.l = read_arg();
			M_SUB(m_WZ.b.l);
			break;
		case 0xd7: // RST 2
			M_RST(2);
			break;

		case 0xd8: // RC
			M_RET(m_AF.b.l & CF);
			break;
		case 0xd9: // 8085: SHLX, otherwise undocumented RET
			if (IS_8085())
			{
				m_WZ.w.l = m_DE.w.l;
				write_mem(m_WZ.d, m_HL.b.l);
				m_WZ.w.l++;
				write_mem(m_WZ.d, m_HL.b.h);
			}
			else
			{
				M_POP(PC);
			}
			break;
		case 0xda: // JC nnnn
			M_JMP(m_AF.b.l & CF);
			break;
		case 0xdb: // IN nn
			set_status(0x42);
			m_WZ.d = read_arg();
			m_AF.b.h = m_io->read_byte(m_WZ.d);
			break;
		case 0xdc: // CC nnnn
			M_CALL(m_AF.b.l & CF);
			break;
		case 0xdd: // 8085: JNX nnnn, otherwise undocumented CALL nnnn
			if (IS_8085())
			{
				M_JMP(!(m_AF.b.l & X5F));
			}
			else
			{
				M_CALL(1);
			}
			break;
		case 0xde: // SBI nn
			m_WZ.b.l = read_arg();
			M_SBB(m_WZ.b.l);
			break;
		case 0xdf: // RST 3
			M_RST(3);
			break;

		case 0xe0: // RPO
			M_RET(!(m_AF.b.l & PF));
			break;
		case 0xe1: // POP H
			M_POP(HL);
			break;
		case 0xe2: // JPO nnnn
			M_JMP(!(m_AF.b.l & PF));
			break;
		case 0xe3: // XTHL
			M_POP(WZ);
			M_PUSH(HL);
			m_HL.d = m_WZ.d;
			break;
		case 0xe4: // CPO nnnn
			M_CALL(!(m_AF.b.l & PF));
			break;
		case 0xe5: // PUSH H
			M_PUSH(HL);
			break;
		case 0xe6: // ANI nn
			m_WZ.b.l = read_arg();
			M_ANA(m_WZ.b.l);
			break;
		case 0xe7: // RST 4
			M_RST(4);
			break;

		case 0xe8: // RPE
			M_RET(m_AF.b.l & PF);
			break;
		case 0xe9: // PCHL
			m_PC.d = m_HL.w.l;
			break;
		case 0xea: // JPE nnnn
			M_JMP(m_AF.b.l & PF);
			break;
		case 0xeb: // XCHG
			m_WZ.d = m_DE.d;
			m_DE.d = m_HL.d;
			m_HL.d = m_WZ.d;
			break;
		case 0xec: // CPE nnnn
			M_CALL(m_AF.b.l & PF);
			break;
		case 0xed: // 8085: LHLX, otherwise undocumented CALL nnnn
			if (IS_8085())
			{
				m_WZ.w.l = m_DE.w.l;
				m_HL.b.l = read_mem(m_WZ.d);
				m_WZ.w.l++;
				m_HL.b.h = read_mem(m_WZ.d);
			}
			else
			{
				M_CALL(1);
			}
			break;
		case 0xee: // XRI nn
			m_WZ.b.l = read_arg();
			M_XRA(m_WZ.b.l);
			break;
		case 0xef: // RST 5
			M_RST(5);
			break;

		case 0xf0: // RP
			M_RET(!(m_AF.b.l & SF));
			break;
		case 0xf1: // POP A
			M_POP(AF);
			break;
		case 0xf2: // JP nnnn
			M_JMP(!(m_AF.b.l & SF));
			break;
		case 0xf3: // DI
			set_inte(0);
			break;
		case 0xf4: // CP nnnn
			M_CALL(!(m_AF.b.l & SF));
			break;
		case 0xf5: // PUSH A
			// on 8080, VF=1 and X3F=0 and X5F=0 always! (we don't have to check for it elsewhere)
			if (IS_8080())
				m_AF.b.l = (m_AF.b.l & ~(X3F | X5F)) | VF;
			M_PUSH(AF);
			break;
		case 0xf6: // ORI nn
			m_WZ.b.l = read_arg();
			M_ORA(m_WZ.b.l);
			break;
		case 0xf7: // RST 6
			M_RST(6);
			break;

		case 0xf8: // RM
			M_RET(m_AF.b.l & SF);
			break;
		case 0xf9: // SPHL
			m_SP.d = m_HL.d;
			break;
		case 0xfa: // JM nnnn
			M_JMP(m_AF.b.l & SF);
			break;
		case 0xfb: // EI
			set_inte(1);
			m_after_ei = 2;
			break;
		case 0xfc: // CM nnnn
			M_CALL(m_AF.b.l & SF);
			break;
		case 0xfd: // 8085: JX nnnn, otherwise undocumented CALL nnnn
			if (IS_8085())
			{
				M_JMP(m_AF.b.l & X5F);
			}
			else
			{
				M_CALL(1);
			}
			break;
		case 0xfe: // CPI nn
			m_WZ.b.l = read_arg();
			M_CMP(m_WZ.b.l);
			break;
		case 0xff: // RST 7
			M_RST(7);
			break;
	} // end big switch
}





/***************************************************************************
    CORE INITIALIZATION
***************************************************************************/

void i8085a_cpu_device::init_tables()
{
	uint8_t zs;
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
		lut_zs[i] = zs;
		lut_zsp[i] = zs | ((p&1) ? 0 : PF);
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
		state_add(STATE_GENPCBASE, "CURPC", m_PC.w.l).noshow();
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
	m_after_ei = false;
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





offs_t i8085a_cpu_device::disasm_disassemble(std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options)
{
	extern CPU_DISASSEMBLE( i8085 );
	return CPU_DISASSEMBLE_NAME(i8085)(this, stream, pc, oprom, opram, options);
}
