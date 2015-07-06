// license:BSD-3-Clause
// copyright-holders:hap

// AMI S2000 opcode handlers

#include "amis2000.h"


// internal helpers

inline UINT8 amis2000_base_device::ram_r()
{
	UINT16 address = m_bu << 4 | m_bl;
	return m_data->read_byte(address) & 0xf;
}

inline void amis2000_base_device::ram_w(UINT8 data)
{
	UINT16 address = m_bu << 4 | m_bl;
	m_data->write_byte(address, data & 0xf);
}

void amis2000_base_device::pop_callstack()
{
	m_pc = (m_pc & ~m_callstack_mask) | (m_callstack[0] & m_callstack_mask);
	for (int i = 0; i < m_callstack_depth-1; i++)
		m_callstack[i] = m_callstack[i+1];
}

void amis2000_base_device::push_callstack()
{
	for (int i = m_callstack_depth-1; i >= 1; i--)
		m_callstack[i] = m_callstack[i-1];
	m_callstack[0] = m_pc & m_callstack_mask;
}

void amis2000_base_device::d_latch_out(bool active)
{
	m_write_d(0, active ? (m_d ^ m_d_polarity) : 0, 0xff);
	m_d_active = active;
}


// Register Instructions

void amis2000_base_device::op_lai()
{
	// LAI X: load ACC with X, select I and K inputs
	// note: only execute the first one in a sequence of LAI
	if ((m_prev_op & 0xf0) != (m_op & 0xf0))
	{
		UINT8 param = m_op & 0x0f;
		m_acc = param;
		m_ki_mask = param;
	}
}

void amis2000_base_device::op_lab()
{
	// LAB: load ACC with BL
	m_acc = m_bl;
}

void amis2000_base_device::op_lae()
{
	// LAE: load ACC with E
	m_acc = m_e;
}

void amis2000_base_device::op_xab()
{
	// XAB: exchange ACC with BL
	UINT8 old_acc = m_acc;
	m_acc = m_bl;
	m_bl = old_acc;
}

void amis2000_base_device::op_xabu()
{
	// XABU: exchange ACC with BU
	UINT8 old_acc = m_acc;
	m_acc = (m_acc & ~m_bu_mask) | (m_bu & m_bu_mask);
	m_bu = old_acc & m_bu_mask;
}

void amis2000_base_device::op_xae()
{
	// XAE: exchange ACC with E
	UINT8 old_acc = m_acc;
	m_acc = m_e;
	m_e = old_acc;
}

void amis2000_base_device::op_lbe()
{
	// LBE Y: load BU with Y, load BL with E
	// note: only execute the first one in a sequence of LB*
	if ((m_prev_op & 0xf0) != (m_op & 0xf0))
	{
		UINT8 param = m_op & 0x03;
		m_bu = param & m_bu_mask;
		m_bl = m_e;
	}
}

void amis2000_base_device::op_lbep()
{
	// LBEP Y: load BU with Y, load BL with E+1
	// note: only execute the first one in a sequence of LB*
	if ((m_prev_op & 0xf0) != (m_op & 0xf0))
	{
		UINT8 param = m_op & 0x03;
		m_bu = param & m_bu_mask;
		m_bl = (m_e + 1) & 0xf;
	}
}

void amis2000_base_device::op_lbz()
{
	// LBZ Y: load BU with Y, load BL with 0
	// note: only execute the first one in a sequence of LB*
	if ((m_prev_op & 0xf0) != (m_op & 0xf0))
	{
		UINT8 param = m_op & 0x03;
		m_bu = param & m_bu_mask;
		m_bl = 0;
	}
}

void amis2000_base_device::op_lbf()
{
	// LBF Y: load BU with Y, load BL with 15
	// note: only execute the first one in a sequence of LB*
	if ((m_prev_op & 0xf0) != (m_op & 0xf0))
	{
		UINT8 param = m_op & 0x03;
		m_bu = param & m_bu_mask;
		m_bl = 0xf;
	}
}


// RAM Instructions

void amis2000_base_device::op_lam()
{
	// LAM _Y: load ACC with RAM, xor BU with _Y
	m_acc = ram_r();
	UINT8 param = ~m_op & 0x03;
	m_bu ^= (param & m_bu_mask);
}

void amis2000_base_device::op_xc()
{
	// XC _Y: exchange ACC with RAM, xor BU with _Y
	UINT8 old_acc = m_acc;
	m_acc = ram_r();
	ram_w(old_acc);
	UINT8 param = ~m_op & 0x03;
	m_bu ^= (param & m_bu_mask);
}

void amis2000_base_device::op_xci()
{
	// XCI _Y: exchange ACC with RAM, increment BL(skip next on carry), xor BU with _Y
	op_xc();
	m_bl = (m_bl + 1) & 0xf;
	m_skip = (m_bl == 0);
}

void amis2000_base_device::op_xcd()
{
	// XCD _Y: exchange ACC with RAM, decrement BL(skip next on carry), xor BU with _Y
	op_xc();
	m_bl = (m_bl - 1) & 0xf;
	m_skip = (m_bl == 0xf);
}

void amis2000_base_device::op_stm()
{
	// STM Z: set RAM bit Z
	UINT8 param = 1 << (m_op & 0x03);
	ram_w(ram_r() | param);
}

void amis2000_base_device::op_rsm()
{
	// RSM Z: reset RAM bit Z
	UINT8 param = 1 << (m_op & 0x03);
	ram_w(ram_r() & ~param);
}


// Input/Output Instructions

void amis2000_base_device::op_inp()
{
	// INP: input D-pins to ACC and RAM
	UINT8 in = m_d_active ? m_d : m_read_d(0, 0xff);
	m_acc = in & 0xf;
	ram_w(in >> 4 & 0xf);
}

void amis2000_base_device::op_out()
{
	// OUT: pulse output ACC and RAM to D-pins
	logerror("%s unknown opcode $%02X at $%04X\n", tag(), m_op, m_pc);
}

void amis2000_base_device::op_disb()
{
	// DISB: set D-latch to ACC and RAM directly
	m_d = m_acc | ram_r() << 4;
	d_latch_out(true);
}

void amis2000_base_device::op_disn()
{
	// DISN: set D-latch to ACC+carry via on-die segment decoder
	static const UINT8 lut_segment_decoder[0x10] =
	{
		// 0-F digits in bit order [DP]abcdefg
		0x7e, 0x30, 0x6d, 0x79, 0x33, 0x5b, 0x5f, 0x70, 0x7f, 0x7b, 0x77, 0x1f, 0x4e, 0x3d, 0x4f, 0x47
	};
	const UINT8 *ptr = (m_7seg_table != NULL) ? m_7seg_table : lut_segment_decoder;
	m_d = ptr[m_acc] | (m_carry ? 0x80 : 0x00);
	d_latch_out(true);
}

void amis2000_base_device::op_mvs()
{
	// MVS: output master strobe latch to A-pins
	d_latch_out(false);
	m_write_a(0, m_a, 0xffff);
}

void amis2000_base_device::op_psh()
{
	// PSH: preset high(BL) master strobe latch
	switch (m_bl)
	{
		case 0xd:
			// set multiplex operation
			// ?
			break;

		case 0xe:
			// exit from floating mode on D-pins
			d_latch_out(true);
			break;

		case 0xf:
			// set all latch bits high
			m_a = 0x1fff;
			break;

		default:
			// set selected latch bit high
			m_a |= (1 << m_bl);
			break;
	}
}

void amis2000_base_device::op_psl()
{
	// PSL: preset low(BL) master strobe latch
	switch (m_bl)
	{
		case 0xd:
			// set static operation
			// ?
			break;

		case 0xe:
			// enter floating mode on D-pins
			d_latch_out(false);
			break;

		case 0xf:
			// set all latch bits low
			m_a = 0;
			break;

		default:
			// set selected latch bit low
			m_a &= ~(1 << m_bl);
			break;
	}
}

void amis2000_base_device::op_eur()
{
	// EUR: set timer frequency(European) and D-latch polarity, via ACC
	m_d_polarity = (m_acc & 1) ? 0x00 : 0xff;
	d_latch_out(m_d_active); // refresh
}


// Program Control Instructions

void amis2000_base_device::op_pp()
{
	// PP _X: prepare page/bank with _X
	UINT8 param = ~m_op & 0x0f;
	if ((m_prev_op & 0xf0) != (m_op & 0xf0))
		m_ppr = param;
	else
		m_pbr = param & 7;
}

void amis2000_base_device::op_jmp()
{
	// JMP X: jump to X(+PP)
	UINT16 mask = 0x3f;
	UINT16 param = m_op & mask;

	// if previous opcode was PP, change PC high bits too
	if ((m_prev_op & 0xf0) == 0x60)
	{
		param |= (m_ppr << 6) | (m_pbr << 10);
		mask = 0x1fff;
	}
	m_pc = (m_pc & ~mask) | param;
}

void amis2000_base_device::op_jms()
{
	// JMS X: call to X(+PP)
	m_icount--;
	push_callstack();
	op_jmp();

	// subroutines default location is page 15
	if ((m_prev_op & 0xf0) != 0x60)
		m_pc |= 0x3c0;
}

void amis2000_base_device::op_rt()
{
	// RT: return from subroutine
	pop_callstack();
}

void amis2000_base_device::op_rts()
{
	// RTS: return from subroutine and skip next
	op_rt();
	m_skip = true;
}

void amis2000_base_device::op_nop()
{
	// NOP: no operation
}

void amis2000_base_device::op_halt()
{
	// HALT: debugger breakpoint for devkit-use
	logerror("%s unknown opcode $%02X at $%04X\n", tag(), m_op, m_pc);
}


// Skip Instructions

void amis2000_base_device::op_szc()
{
	// SZC: skip next on zero(no) carry
	m_skip = !m_carry;
}

void amis2000_base_device::op_szm()
{
	// SZM Z: skip next on zero RAM bit Z
	UINT8 param = 1 << (m_op & 0x03);
	m_skip = !(ram_r() & param);
}

void amis2000_base_device::op_szi()
{
	// SZI: skip next on I pin(s)
	m_skip = ((~m_read_i(0, 0xff) & m_ki_mask) != 0);
}

void amis2000_base_device::op_szk()
{
	// SZK: skip next on K pin(s)
	m_skip = ((~m_read_k(0, 0xff) & m_ki_mask) != 0);
}

void amis2000_base_device::op_sbe()
{
	// SBE: skip next on BL equals E
	m_skip = (m_bl == m_e);
}

void amis2000_base_device::op_sam()
{
	// SAM: skip next on ACC equals RAM
	m_skip = (m_acc == ram_r());
}

void amis2000_base_device::op_sos()
{
	// SOS: skip next on SF(timer output), clear SF
	logerror("%s unknown opcode $%02X at $%04X\n", tag(), m_op, m_pc);
}

void amis2000_base_device::op_tf1()
{
	// TF1: skip next on flag 1
	m_skip = ((m_f & 0x01) != 0);
}

void amis2000_base_device::op_tf2()
{
	// TF2: skip next on flag 2
	m_skip = ((m_f & 0x02) != 0);
}


// Arithmetic and Logical Instructions

void amis2000_base_device::op_adcs()
{
	// ADCS: add RAM to ACC+carry, skip next on not carry
	m_acc += ram_r() + m_carry;
	m_carry = m_acc >> 4 & 1;
	m_skip = !m_carry;
	m_acc &= 0xf;
}

void amis2000_base_device::op_adis()
{
	// ADIS X: add X to ACC, skip next on not carry
	UINT8 param = m_op & 0x0f;
	m_acc += param;
	m_skip = !(m_acc & 0x10);
	m_acc &= 0xf;
}

void amis2000_base_device::op_add()
{
	// ADD: add RAM to ACC
	m_acc = (m_acc + ram_r()) & 0xf;
}

void amis2000_base_device::op_and()
{
	// AND: and ACC with RAM
	m_acc &= ram_r();
}

void amis2000_base_device::op_xor()
{
	// XOR: xor ACC with RAM
	m_acc ^= ram_r();
}

void amis2000_base_device::op_stc()
{
	// STC: set carry
	m_carry = 1;
}

void amis2000_base_device::op_rsc()
{
	// RSC: reset carry
	m_carry = 0;
}

void amis2000_base_device::op_cma()
{
	// CMA: complement ACC
	m_acc ^= 0xf;
}

void amis2000_base_device::op_sf1()
{
	// SF1: set flag 1
	m_f |= 0x01;
}

void amis2000_base_device::op_rf1()
{
	// RF1: reset flag 1
	m_f &= ~0x01;
}

void amis2000_base_device::op_sf2()
{
	// SF2: set flag 2
	m_f |= 0x02;
}

void amis2000_base_device::op_rf2()
{
	// RF2: reset flag 2
	m_f &= ~0x02;
}



// AMI S2152 specific handlers

void amis2152_cpu_device::d2f_timer_clock()
{
	// schedule next timeout (frequency is guessed)
	attotime base = attotime::from_ticks(4 * 64, unscaled_clock());
	m_d2f_timer->adjust(base * (0x10 - m_d2f_latch));
}

TIMER_CALLBACK_MEMBER(amis2152_cpu_device::d2f_timer_cb)
{
	m_write_f(m_fout_state);
	m_fout_state ^= 1;

	d2f_timer_clock();
}

void amis2152_cpu_device::op_szk()
{
	// instead of SZK: ???: load d2f latch with ACC(?)
	m_d2f_latch = m_acc;
}
