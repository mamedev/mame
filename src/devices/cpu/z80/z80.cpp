// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************

    ZiLOG Z80 emulator

TODO:
- Interrupt mode 0 should be able to execute arbitrary opcodes
- If LD A,I or LD A,R is interrupted, P/V flag gets reset, even if IFF2
  was set before this instruction (implemented, but not enabled: we need
  document Z80 types first, see below)
- Ideally, the tiny differences between Z80 types should be supported,
  currently known differences:
  - LD A,I/R P/V flag reset glitch is fixed on CMOS Z80
  - OUT (C),0 outputs 0 on NMOS Z80, $FF on CMOS Z80
  - SCF/CCF X/Y flags is ((flags | A) & 0x28) on SGS/SHARP/ZiLOG NMOS Z80,
    (flags & A & 0x28).
    However, recent findings say that SCF/CCF X/Y results depend on whether
    or not the previous instruction touched the flag register.
  This Z80 emulator assumes a ZiLOG NMOS model.

*****************************************************************************/

#include "emu.h"
#include "z80.h"
#include "z80dasm.h"

#include "z80.inc"

#include <cstdio>

#define LOG_INT   (1U << 1) // z80.lst
#define LOG_UNDOC (1U << 2)

#define VERBOSE (LOG_UNDOC)
#include "logmacro.h"


/***************************************************************
 * Flag helpers (for eg. POP/PUSH AF, EX AF,AF')
 ***************************************************************/
u8 z80_device::get_f()
{
	u8 f = 0;
	f |= m_f.s();
	f |= m_f.z();
	f |= m_f.yx();
	f |= m_f.h();
	f |= m_f.pv();
	f |= m_f.n ? NF : 0;
	f |= m_f.c ? CF : 0;
	return f;
}

void z80_device::set_f(u8 f)
{
	m_f.s_val  = f;
	m_f.z_val  = !(f & ZF);
	m_f.yx_val = f;
	m_f.h_val  = f;
	m_f.pv_val = !(f & PF);
	m_f.n      = f & NF;
	m_f.c      = f & CF;
}


/***************************************************************
 * Enter halt state; write 1 to callback on first execution
 ***************************************************************/
void z80_device::halt()
{
	if (!m_halt)
	{
		m_halt = 1;
		set_service_attention<SA_HALT, 1>();
		m_halt_cb(1);
	}
}

/***************************************************************
 * Leave halt state; write 0 to callback
 ***************************************************************/
void z80_device::leave_halt()
{
	if (m_halt)
	{
		m_halt = 0;
		set_service_attention<SA_HALT, 0>();
		m_halt_cb(0);
	}
}

/***************************************************************
 * Read a byte from given memory location
 ***************************************************************/
u8 z80_device::data_read(u16 addr)
{
	return m_data.read_interruptible(addr);
}

/***************************************************************
 * Write a byte to given memory location
 ***************************************************************/
void z80_device::data_write(u16 addr, u8 value)
{
	m_data.write_interruptible(addr, value);
}

/***************************************************************
 * rop() is identical to rm() except it is used for
 * reading opcodes. In case of system with memory mapped I/O,
 * this function can be used to greatly speed up emulation
 ***************************************************************/
u8 z80_device::opcode_read()
{
	return m_opcodes.read_byte(PC);
}

/****************************************************************
 * arg() is identical to rop() except it is used
 * for reading opcode arguments. This difference can be used to
 * support systems that use different encoding mechanisms for
 * opcodes and opcode arguments
 * out: TDAT8
 ***************************************************************/
u8 z80_device::arg_read()
{
	return m_args.read_byte(PC);
}

/***************************************************************
 * INC  r8
 ***************************************************************/
void z80_device::inc(u8 &r)
{
	++r;
	{
		QT = 0;
		// keep C
		m_f.s_val = m_f.z_val = m_f.yx_val = r;
		m_f.pv_val = r != 0x80;
		m_f.h_val = (r & 0x0f) == 0x00 ? HF : 0;
		m_f.n = 0;
	}
}

/***************************************************************
 * DEC  r8
 ***************************************************************/
void z80_device::dec(u8 &r)
{
	--r;
	{
		QT = 0;
		// keep C
		m_f.s_val = m_f.z_val = m_f.yx_val = r;
		m_f.pv_val = r != 0x7f;
		m_f.h_val = (r & 0x0f) == 0x0f ? HF : 0;
		m_f.n = 1;
	}
}

/***************************************************************
 * RLCA
 ***************************************************************/
void z80_device::rlca()
{
	A = (A << 1) | (A >> 7);
	{
		QT = 0;
		// keep SZP
		m_f.yx_val = A;
		m_f.h_val = m_f.n = 0;
		m_f.c = A & 0x01;
	}
}

/***************************************************************
 * RRCA
 ***************************************************************/
void z80_device::rrca()
{
	const u8 a0 = A;
	A = (a0 >> 1) | (a0 << 7);
	{
		QT = 0;
		// keep SZP
		m_f.yx_val = A;
		m_f.h_val = m_f.n = 0;
		m_f.c = a0 & 0x01;
	}
}

/***************************************************************
 * RLA
 ***************************************************************/
void z80_device::rla()
{
	u8 res = (A << 1) + m_f.c;
	{
		QT = 0;
		// keep SZP
		m_f.yx_val = res;
		m_f.h_val = m_f.n = 0;
		m_f.c = A & 0x80;
	}
	A = res;
}

/***************************************************************
 * RRA
 ***************************************************************/
void z80_device::rra()
{
	u8 res = (m_f.c << 7) | (A >> 1);
	{
		QT = 0;
		// keep SZP
		m_f.yx_val = res;
		m_f.h_val = m_f.n = 0;
		m_f.c = A & 0x01;
	}
	A = res;
}

/***************************************************************
 * ADD  A,n
 ***************************************************************/
void z80_device::add_a(u8 value)
{
	const u16 res = A + value;
	{
		QT = 0;
		m_f.s_val = m_f.z_val = m_f.yx_val = res;
		m_f.c = res & 0x100;
		m_f.h_val = (A & 0x0f) + (value & 0x0f);
		m_f.pv_val = !((A ^ res) & (value ^ res) & 0x80);
		m_f.n = 0;
	}
	A = res;
}

/***************************************************************
 * ADC  A,n
 ***************************************************************/
void z80_device::adc_a(u8 value)
{
	const int c = m_f.c;
	const u16 res = A + value + c;
	{
		QT = 0;
		m_f.s_val = m_f.z_val = m_f.yx_val = res;
		m_f.c = res & 0x100;
		m_f.h_val = (A & 0x0f) + (value & 0x0f) + c;
		m_f.pv_val = !((A ^ res) & (value ^ res) & 0x80);
		m_f.n = 0;
	}
	A = res;
}

/***************************************************************
 * SUB  A,n
 ***************************************************************/
void z80_device::sub_a(u8 value)
{
	const u16 res = A - value;
	{
		QT = 0;
		m_f.s_val = m_f.z_val = m_f.yx_val = res;
		m_f.c = res & 0x100;
		m_f.h_val = (A & 0x0f) - (value & 0x0f);
		m_f.pv_val = !((A ^ value) & (A ^ res) & 0x80);
		m_f.n = 1;
	}
	A = res;
}

/***************************************************************
 * SBC  A,n
 ***************************************************************/
void z80_device::sbc_a(u8 value)
{
	const int c = m_f.c;
	const u16 res = A - value - c;
	{
		QT = 0;
		m_f.s_val = m_f.z_val = m_f.yx_val = res;
		m_f.c = res & 0x100;
		m_f.h_val = (A & 0x0f) - (value & 0x0f) - c;
		m_f.pv_val = !((A ^ value) & (A ^ res) & 0x80);
		m_f.n = 1;
	}
	A = res;
}

/***************************************************************
 * NEG
 ***************************************************************/
void z80_device::neg()
{
	u8 value = A;
	A = 0;
	sub_a(value);
}

/***************************************************************
 * DAA
 ***************************************************************/
void z80_device::daa()
{
	u8 a = A;
	if (m_f.n)
	{
		if (m_f.h() || ((A & 0xf) > 9)) a -= 6;
		if (m_f.c || (A > 0x99)) a -= 0x60;
	}
	else
	{
		if (m_f.h() || ((A & 0xf) > 9)) a += 6;
		if (m_f.c || (A > 0x99)) a += 0x60;
	}
	{
		QT = 0;
		// keep N
		m_f.s_val = m_f.z_val = m_f.pv_val = m_f.yx_val = a;
		m_f.h_val = A ^ a;
		m_f.c = m_f.c || A > 0x99;
	}
	A = a;
}

/***************************************************************
 * AND  n
 ***************************************************************/
void z80_device::and_a(u8 value)
{
	A &= value;
	{
		QT = 0;
		m_f.s_val = m_f.z_val = m_f.pv_val = m_f.yx_val = A;
		m_f.n = m_f.c = 0;
		m_f.h_val = HF;
	}
}

/***************************************************************
 * OR   n
 ***************************************************************/
void z80_device::or_a(u8 value)
{
	A |= value;
	{
		QT = 0;
		m_f.s_val = m_f.z_val = m_f.pv_val = m_f.yx_val = A;
		m_f.h_val = m_f.n = m_f.c = 0;
	}
}

/***************************************************************
 * XOR  n
 ***************************************************************/
void z80_device::xor_a(u8 value)
{
	A ^= value;
	{
		QT = 0;
		m_f.s_val = m_f.z_val = m_f.pv_val = m_f.yx_val = A;
		m_f.h_val = m_f.n = m_f.c = 0;
	}
}

/***************************************************************
 * CP   n
 ***************************************************************/
void z80_device::cp(u8 value)
{
	const u16 res = A - value;
	{
		QT = 0;
		m_f.s_val = m_f.z_val = res;
		m_f.yx_val = value;
		m_f.c = res & 0x100;
		m_f.h_val = (A & 0x0f) - (value & 0x0f);
		m_f.pv_val = !((A ^ value) & (A ^ res) & 0x80);
		m_f.n = 1;
	}
}

/***************************************************************
 * EXX
 ***************************************************************/
void z80_device::exx()
{
	using std::swap;
	swap(m_bc, m_bc2);
	swap(m_de, m_de2);
	swap(m_hl, m_hl2);
}

/***************************************************************
 * RLC  r8
 ***************************************************************/
u8 z80_device::rlc(u8 value)
{
	const u8 res = ((value << 1) | (value >> 7)) & 0xff;
	{
		QT = 0;
		m_f.s_val = m_f.z_val = m_f.pv_val = m_f.yx_val = res;
		m_f.h_val = m_f.n = 0;
		m_f.c = value & 0x80;
	}

	return res;
}

/***************************************************************
 * RRC  r8
 ***************************************************************/
u8 z80_device::rrc(u8 value)
{
	const u8 res = ((value >> 1) | (value << 7)) & 0xff;
	{
		QT = 0;
		m_f.s_val = m_f.z_val = m_f.pv_val = m_f.yx_val = res;
		m_f.h_val = m_f.n = 0;
		m_f.c = value & 0x01;
	}

	return res;
}

/***************************************************************
 * RL   r8
 ***************************************************************/
u8 z80_device::rl(u8 value)
{
	const u8 res = ((value << 1) + m_f.c) & 0xff;
	{
		QT = 0;
		m_f.s_val = m_f.z_val = m_f.pv_val = m_f.yx_val = res;
		m_f.h_val = m_f.n = 0;
		m_f.c = value & 0x80;
	}

	return res;
}

/***************************************************************
 * RR   r8
 ***************************************************************/
u8 z80_device::rr(u8 value)
{
	const u8 res = ((value >> 1) | (m_f.c << 7)) & 0xff;
	{
		QT = 0;
		m_f.s_val = m_f.z_val = m_f.pv_val = m_f.yx_val = res;
		m_f.h_val = m_f.n = 0;
		m_f.c = value & 0x01;
	}

	return res;
}

/***************************************************************
 * SLA  r8
 ***************************************************************/
u8 z80_device::sla(u8 value)
{
	const u8 res = (value << 1) & 0xff;
	{
		QT = 0;
		m_f.s_val = m_f.z_val = m_f.pv_val = m_f.yx_val = res;
		m_f.h_val = m_f.n = 0;
		m_f.c = value & 0x80;
	}

	return res;
}

/***************************************************************
 * SRA  r8
 ***************************************************************/
u8 z80_device::sra(u8 value)
{
	const u8 res = ((value >> 1) | (value & 0x80)) & 0xff;
	{
		QT = 0;
		m_f.s_val = m_f.z_val = m_f.pv_val = m_f.yx_val = res;
		m_f.h_val = m_f.n = 0;
		m_f.c = value & 0x01;
	}

	return res;
}

/***************************************************************
 * SLL  r8
 ***************************************************************/
u8 z80_device::sll(u8 value)
{
	const u8 res = ((value << 1) | 0x01) & 0xff;
	{
		QT = 0;
		m_f.s_val = m_f.z_val = m_f.pv_val = m_f.yx_val = res;
		m_f.h_val = m_f.n = 0;
		m_f.c = value & 0x80;
	}

	return res;
}

/***************************************************************
 * SRL  r8
 ***************************************************************/
u8 z80_device::srl(u8 value)
{
	const u8 res = (value >> 1) & 0xff;
	{
		QT = 0;
		m_f.s_val = m_f.z_val = m_f.pv_val = m_f.yx_val = res;
		m_f.h_val = m_f.n = 0;
		m_f.c = value & 0x01;
	}

	return res;
}

/***************************************************************
 * BIT  bit,r8
 ***************************************************************/
void z80_device::bit(int bit, u8 value)
{
	QT = 0;
	m_f.s_val = m_f.z_val = m_f.pv_val = value & (1 << bit);
	m_f.h_val = HF;
	m_f.n = 0;
	m_f.yx_val = value;
}

/***************************************************************
 * BIT  bit,(HL)
 ***************************************************************/
void z80_device::bit_hl(int bit, u8 value)
{
	QT = 0;
	m_f.s_val = m_f.z_val = m_f.pv_val = value & (1 << bit);
	m_f.h_val = HF;
	m_f.n = 0;
	m_f.yx_val = WZ_H;
}

/***************************************************************
 * BIT  bit,(IX/Y+o)
 ***************************************************************/
void z80_device::bit_xy(int bit, u8 value)
{
	QT = 0;
	m_f.s_val = m_f.z_val = m_f.pv_val = value & (1 << bit);
	m_f.h_val = HF;
	m_f.n = 0;
	m_f.yx_val = m_ea >> 8;
}

/***************************************************************
 * RES  bit,r8
 ***************************************************************/
u8 z80_device::res(int bit, u8 value)
{
	return value & ~(1 << bit);
}

/***************************************************************
 * SET  bit,r8
 ***************************************************************/
u8 z80_device::set(int bit, u8 value)
{
	return value | (1 << bit);
}

void z80_device::block_io_interrupted_flags()
{
	m_f.yx_val = PC >> 8;

	const u8 pv_old = m_f.pv();
	if (m_f.c)
	{
		m_f.h_val = 0;
		if (TDAT8 & 0x80)
		{
			m_f.pv_val = (B - 1) & 0x07;
			if ((B & 0x0f) == 0x00) m_f.h_val = HF;
		}
		else
		{
			m_f.pv_val = (B + 1) & 0x07;
			if ((B & 0x0f) == 0x0f) m_f.h_val = HF;
		}
	}
	else
	{
		m_f.pv_val = B & 0x07;
	}
	m_f.pv_val = (pv_old ^ m_f.pv()) & PF;
}

/***************************************************************
 * EI
 ***************************************************************/
void z80_device::ei()
{
	m_iff1 = m_iff2 = 1;
	set_service_attention<SA_AFTER_EI, 1>();
}

void z80_device::illegal_1()
{
	LOGMASKED(LOG_UNDOC, "ill. opcode $%02x $%02x ($%04x)\n",
			m_opcodes.read_byte((PC - 1) & 0xffff), m_opcodes.read_byte(PC), PC - 1);
}

void z80_device::illegal_2()
{
	LOGMASKED(LOG_UNDOC, "ill. opcode $ed $%02x\n",
			m_opcodes.read_byte((PC - 1) & 0xffff));
}


/****************************************************************************
 * Processor initialization
 ****************************************************************************/
void z80_device::device_validity_check(validity_checker &valid) const
{
	cpu_device::device_validity_check(valid);

	if (4 > m_m1_cycles)
		osd_printf_error("M1 cycles %u is less than minimum 4\n", m_m1_cycles);
	if (3 > m_memrq_cycles)
		osd_printf_error("MEMRQ cycles %u is less than minimum 3\n", m_memrq_cycles);
	if (4 > m_iorq_cycles)
		osd_printf_error("IORQ cycles %u is less than minimum 4\n", m_iorq_cycles);
}

void z80_device::device_start()
{
	save_item(NAME(PRVPC));
	save_item(NAME(PC));
	save_item(NAME(SP));
	save_item(NAME(AF));
	save_item(NAME(BC));
	save_item(NAME(DE));
	save_item(NAME(HL));
	save_item(NAME(IX));
	save_item(NAME(IY));
	save_item(NAME(WZ));
	save_item(NAME(m_af2.w));
	save_item(NAME(m_bc2.w));
	save_item(NAME(m_de2.w));
	save_item(NAME(m_hl2.w));
	save_item(NAME(QT));
	save_item(NAME(Q));
	save_item(NAME(R));
	save_item(NAME(R2));
	save_item(NAME(m_iff1));
	save_item(NAME(m_iff2));
	save_item(NAME(m_halt));
	save_item(NAME(m_im));
	save_item(NAME(m_i));
	save_item(NAME(m_nmi_state));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_wait_state));
	save_item(NAME(m_busrq_state));
	save_item(NAME(m_busack_state));
	save_item(NAME(m_ea));
	save_item(NAME(m_service_attention));
	save_item(NAME(m_tmp_irq_vector));
	save_item(NAME(m_shared_data.w));
	save_item(NAME(m_shared_data2.w));
	save_item(NAME(m_rtemp));
	save_item(NAME(m_ref));

	// Reset registers to their initial values
	PRVPC = 0;
	PC = 0;
	SP = 0;
	AF = 0;
	set_f(0);
	Q = 0;
	QT = 0;
	BC = 0;
	DE = 0;
	HL = 0;
	IX = 0;
	IY = 0;
	WZ = 0;
	m_af2.w = 0;
	m_bc2.w = 0;
	m_de2.w = 0;
	m_hl2.w = 0;
	R = 0;
	R2 = 0;
	m_iff1 = 0;
	m_iff2 = 0;
	m_halt = 0;
	m_im = 0;
	m_i = 0;
	m_nmi_state = 0;
	m_irq_state = 0;
	m_wait_state = 0;
	m_busrq_state = 0;
	m_busack_state = 0;
	m_ea = 0;
	m_service_attention = 0;
	m_rtemp = 0;

	space(AS_PROGRAM).cache(m_args);
	space(has_space(AS_OPCODES) ? AS_OPCODES : AS_PROGRAM).cache(m_opcodes);
	space(AS_PROGRAM).specific(m_data);
	space(AS_IO).specific(m_io);

	IX = IY = 0xffff; // IX and IY are FFFF after a reset!
	m_f.z_val = 0; // Zero flag is set

	// set up the state table
	state_add(STATE_GENPC,     "PC",        m_pc.w).callimport();
	state_add(STATE_GENPCBASE, "CURPC",     m_prvpc.w).callimport().noshow();
	state_add(Z80_SP,          "SP",        SP);
	state_add(STATE_GENFLAGS,  "GENFLAGS",  F).noshow().formatstr("%8s");
	state_add(Z80_A,           "A",         A).noshow();
	state_add(Z80_F,           "F",         F).noshow().callimport().callexport();
	state_add(Z80_B,           "B",         B).noshow();
	state_add(Z80_C,           "C",         C).noshow();
	state_add(Z80_D,           "D",         D).noshow();
	state_add(Z80_E,           "E",         E).noshow();
	state_add(Z80_H,           "H",         H).noshow();
	state_add(Z80_L,           "L",         L).noshow();
	state_add(Z80_AF,          "AF",        AF).callimport().callexport();
	state_add(Z80_BC,          "BC",        BC);
	state_add(Z80_DE,          "DE",        DE);
	state_add(Z80_HL,          "HL",        HL);
	state_add(Z80_IX,          "IX",        IX);
	state_add(Z80_IY,          "IY",        IY);
	state_add(Z80_AF2,         "AF2",       m_af2.w);
	state_add(Z80_BC2,         "BC2",       m_bc2.w);
	state_add(Z80_DE2,         "DE2",       m_de2.w);
	state_add(Z80_HL2,         "HL2",       m_hl2.w);
	state_add(Z80_WZ,          "WZ",        WZ);
	state_add(Z80_R,           "R",         m_rtemp).callimport().callexport();
	state_add(Z80_I,           "I",         m_i);
	state_add(Z80_IM,          "IM",        m_im).mask(0x3);
	state_add(Z80_IFF1,        "IFF1",      m_iff1).mask(0x1);
	state_add(Z80_IFF2,        "IFF2",      m_iff2).mask(0x1);
	state_add(Z80_HALT,        "HALT",      m_halt).mask(0x1);

	// set our instruction counter
	set_icountptr(m_icount);
}

/****************************************************************************
 * Do a reset
 ****************************************************************************/
void z80_device::device_reset()
{
	leave_halt();

	m_ref = 0xffff00;
	PC = 0;
	WZ = PC;
	m_i = 0;
	m_r = 0;
	m_r2 = 0;
	m_iff1 = 0;
	m_iff2 = 0;

	set_service_attention<SA_NMI_PENDING, 0>();
	set_service_attention<SA_AFTER_EI, 0>();
	set_service_attention<SA_AFTER_LDAIR, 0>();
}

/****************************************************************************
 * Execute 'cycles' T-states.
 ****************************************************************************/
void z80_device::execute_run()
{
	#include "cpu/z80/z80.hxx"
}

void z80_device::execute_set_input(int inputnum, int state)
{
	switch (inputnum)
	{
	case Z80_INPUT_LINE_BUSRQ:
		if (m_busrq_state == CLEAR_LINE && state != CLEAR_LINE)
			set_service_attention<SA_BUSRQ, 1>();
		m_busrq_state = state;
		break;

	case INPUT_LINE_NMI:
		// mark an NMI pending on the rising edge
		if (m_nmi_state == CLEAR_LINE && state != CLEAR_LINE)
			set_service_attention<SA_NMI_PENDING, 1>();
		m_nmi_state = state;
		break;

	case INPUT_LINE_IRQ0:
		// update the IRQ state via the daisy chain
		m_irq_state = state;
		if (daisy_chain_present())
			m_irq_state = (daisy_update_irq_state() == ASSERT_LINE) ? ASSERT_LINE : m_irq_state;
		if (state != CLEAR_LINE)
			set_service_attention<SA_IRQ_ON, 1>();
		else
			set_service_attention<SA_IRQ_ON, 0>();

		// the main execute loop will take the interrupt
		break;

	case Z80_INPUT_LINE_WAIT:
		m_wait_state = state;
		break;

	default:
		break;
	}
}

/**************************************************************************
 * STATE IMPORT/EXPORT
 **************************************************************************/
void z80_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
	case STATE_GENPCBASE:
		m_pc = m_prvpc;
		[[fallthrough]];
	case STATE_GENPC:
		m_prvpc = m_pc;
		m_ref = 0xffff00;
		set_service_attention<SA_AFTER_EI, 0>();
		if (HAS_LDAIR_QUIRK)
			set_service_attention<SA_AFTER_LDAIR, 0>();
		break;

	case Z80_F: case Z80_AF:
		set_f(F);
		break;
	case Z80_R:
		m_r = m_rtemp & 0x7f;
		m_r2 = m_rtemp & 0x80;
		break;

	default:
		fatalerror("CPU_IMPORT_STATE() called for unexpected value\n");
	}
}

void z80_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
	case Z80_F: case Z80_AF:
		F = get_f();
		break;
	case Z80_R:
		m_rtemp = (m_r & 0x7f) | (m_r2 & 0x80);
		break;

	default:
		fatalerror("CPU_EXPORT_STATE() called for unexpected value\n");
	}
}

void z80_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
		{
			str = string_format("%c%c%c%c%c%c%c%c",
				m_f.s()         ? 'S':'.',
				m_f.z()         ? 'Z':'.',
				m_f.yx() & 0x20 ? 'Y':'.',
				m_f.h()         ? 'H':'.',
				m_f.yx() & 0x08 ? 'X':'.',
				m_f.pv()        ? 'P':'.',
				m_f.n           ? 'N':'.',
				m_f.c           ? 'C':'.');
		}
		break;
	}
}

/**************************************************************************
 * disassemble - call the disassembly helper function
 **************************************************************************/
std::unique_ptr<util::disasm_interface> z80_device::create_disassembler()
{
	return std::make_unique<z80_disassembler>();
}

z80_device::z80_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: z80_device(mconfig, Z80, tag, owner, clock)
{
}

z80_device::z80_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	cpu_device(mconfig, type, tag, owner, clock),
	z80_daisy_chain_interface(mconfig, *this),
	m_program_config("program", ENDIANNESS_LITTLE, 8, 16, 0),
	m_opcodes_config("opcodes", ENDIANNESS_LITTLE, 8, 16, 0),
	m_io_config("io", ENDIANNESS_LITTLE, 8, 16, 0),
	m_irqack_cb(*this),
	m_refresh_cb(*this),
	m_nomreq_cb(*this),
	m_halt_cb(*this),
	m_busack_cb(*this),
	m_m1_cycles(4),
	m_memrq_cycles(3),
	m_iorq_cycles(4)
{
}

device_memory_interface::space_config_vector z80_device::memory_space_config() const
{
	if (has_configured_map(AS_OPCODES))
	{
		return space_config_vector {
				std::make_pair(AS_PROGRAM, &m_program_config),
				std::make_pair(AS_OPCODES, &m_opcodes_config),
				std::make_pair(AS_IO,      &m_io_config) };
	}
	else
	{
		return space_config_vector {
				std::make_pair(AS_PROGRAM, &m_program_config),
				std::make_pair(AS_IO,      &m_io_config) };
	}
}

DEFINE_DEVICE_TYPE(Z80, z80_device, "z80", "Zilog Z80")
