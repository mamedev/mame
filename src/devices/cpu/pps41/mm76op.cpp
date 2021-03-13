// license:BSD-3-Clause
// copyright-holders:hap

// MM76/shared opcode handlers

#include "emu.h"
#include "mm76.h"


// internal helpers

u8 mm76_device::ram_r()
{
	return m_data->read_byte(m_ram_addr & m_datamask) & 0xf;
}

void mm76_device::ram_w(u8 data)
{
	m_data->write_byte(m_ram_addr & m_datamask, data & 0xf);
}

void mm76_device::pop_pc()
{
	m_pc = m_stack[0] & m_prgmask;
	for (int i = 0; i < m_stack_levels-1; i++)
		m_stack[i] = m_stack[i+1];
}

void mm76_device::push_pc()
{
	for (int i = m_stack_levels-1; i >= 1; i--)
		m_stack[i] = m_stack[i-1];
	m_stack[0] = m_pc;
}

void mm76_device::op_illegal()
{
	logerror("unknown opcode $%02X at $%03X\n", m_op, m_prev_pc);
}

void mm76_device::op_todo()
{
	logerror("unimplemented opcode $%02X at $%03X\n", m_op, m_prev_pc);
}


// opcodes

// RAM addressing instructions

void mm76_device::op_xab()
{
	// XAB: exchange A with Bl
	u8 a = m_a;
	m_a = m_b & 0xf;
	m_b = (m_b & ~0xf) | a;
	m_ram_delay = true;
}

void mm76_device::op_lba()
{
	// LBA: load Bl from A
	m_b = (m_b & ~0xf) | m_a;
	m_ram_delay = true;
}

void mm76_device::op_lb()
{
	// LB x: load B from x (successive LB/EOB are ignored)
	if (!(op_is_lb(m_prev_op) && !op_is_tr(m_prev2_op)) && !op_is_eob(m_prev_op))
		m_b = m_op & 0xf;
}

void mm76_device::op_eob()
{
	// EOB x: XOR Bu with x (successive LB/EOB are ignored, except after first executed LB)
	bool first_lb = (op_is_lb(m_prev_op) && !op_is_tr(m_prev2_op)) && !(op_is_lb(m_prev2_op) && !op_is_tr(m_prev3_op)) && !op_is_eob(m_prev2_op);
	if ((!(op_is_lb(m_prev_op) && !op_is_tr(m_prev2_op)) && !op_is_eob(m_prev_op)) || first_lb)
		m_b ^= m_op << 4 & m_datamask;
}


// bit manipulation instructions

void mm76_device::op_sb()
{
	// SB x: set memory bit / SOS: set output

	// Bu rising: opcode is invalid
	if ((m_prev2_b & 0x30) != 0x30 && (m_prev_b & 0x30) == 0x30)
	{
		logerror("SB/SOS invalid access at $%03X\n", m_prev_pc);
		return;
	}

	// Bu falling or Bu == 3: SOS
	if (((m_prev2_b & 0x30) == 0x30 && (m_prev_b & 0x30) != 0x30) || (m_prev_b & 0x30) == 0x30)
	{
		u8 bl = m_ram_addr & 0xf;
		if (bl > m_d_pins)
			logerror("SOS invalid pin %d at $%03X\n", bl, m_prev_pc);
		else
		{
			m_d_output = (m_d_output | (1 << bl)) & m_d_mask;
			m_write_d(m_d_output);
		}
	}

	// Bu != 3: SB
	if ((m_prev_b & 0x30) != 0x30)
		ram_w(ram_r() | (1 << (m_op & 3)));
}

void mm76_device::op_rb()
{
	// RB x: reset memory bit / ROS: reset output

	// Bu rising: opcode is invalid
	if ((m_prev2_b & 0x30) != 0x30 && (m_prev_b & 0x30) == 0x30)
	{
		logerror("RB/ROS invalid access at $%03X\n", m_prev_pc);
		return;
	}

	// Bu falling or Bu == 3: ROS
	if (((m_prev2_b & 0x30) == 0x30 && (m_prev_b & 0x30) != 0x30) || (m_prev_b & 0x30) == 0x30)
	{
		u8 bl = m_ram_addr & 0xf;
		if (bl > m_d_pins)
			logerror("ROS invalid pin %d at $%03X\n", bl, m_prev_pc);
		else
		{
			m_d_output = m_d_output & ~(1 << bl);
			m_write_d(m_d_output);
		}
	}

	// Bu != 3: RB
	if ((m_prev_b & 0x30) != 0x30)
		ram_w(ram_r() & ~(1 << (m_op & 3)));
}

void mm76_device::op_skbf()
{
	// SKBF x: test memory bit / SKISL: test input

	// Bu rising: opcode is invalid
	if ((m_prev2_b & 0x30) != 0x30 && (m_prev_b & 0x30) == 0x30)
	{
		logerror("SKBF/SKISL invalid access at $%03X\n", m_prev_pc);
		return;
	}

	// Bu falling or Bu == 3: SKISL
	if (((m_prev2_b & 0x30) == 0x30 && (m_prev_b & 0x30) != 0x30) || (m_prev_b & 0x30) == 0x30)
	{
		u8 bl = m_ram_addr & 0xf;
		if (bl > m_d_pins)
			logerror("SKISL invalid pin %d at $%03X\n", bl, m_prev_pc);
		else
			m_skip = !BIT((m_d_output | m_read_d()) & m_d_mask, bl);
	}

	// Bu != 3: SKBF
	if ((m_prev_b & 0x30) != 0x30)
		m_skip = m_skip || !BIT(ram_r(), m_op & 3);
}


// register to register instructions

void mm76_device::op_xas()
{
	// XAS: exchange A with S
	u8 a = m_a;
	m_a = m_s;
	m_s = a;
	m_write_sdo(BIT(m_s, 3));
}

void mm76_device::op_lsa()
{
	// LSA: load S from A
	m_s = m_a;
	m_write_sdo(BIT(m_s, 3));
}


// register memory instructions

void mm76_device::op_l()
{
	// L x: load A from memory, XOR Bu with x
	m_a = ram_r();
	m_b ^= m_op << 4 & 0x30;
}

void mm76_device::op_x()
{
	// X x: exchange A with memory, XOR Bu with x
	u8 a = m_a;
	m_a = ram_r();
	ram_w(a);
	m_b ^= m_op << 4 & 0x30;
}

void mm76_device::op_xdsk()
{
	// XDSK x: X x + decrement Bl
	op_x();
	m_b = (m_b & ~0xf) | ((m_b - 1) & 0xf);
	m_skip = (m_b & 0xf) == 0xf;
	m_ram_delay = true;
}

void mm76_device::op_xnsk()
{
	// XNSK x: X x + increment Bl
	op_x();
	m_b = (m_b & ~0xf) | ((m_b + 1) & 0xf);
	m_skip = (m_b & 0xf) == 0;
	m_ram_delay = true;
}


// arithmetic instructions

void mm76_device::op_a()
{
	// A: add memory to A
	m_a = (m_a + ram_r()) & 0xf;
}

void mm76_device::op_ac()
{
	// AC: add memory and carry to A
	m_a += ram_r() + m_c_in;
	m_c = m_a >> 4 & 1;
	m_a &= 0xf;
	m_c_delay = true;
}

void mm76_device::op_acsk()
{
	// ACSK: AC + skip on no overflow
	op_ac();
	m_skip = !m_c;
}

void mm76_device::op_ask()
{
	// ASK: A + skip on no overflow
	u8 a = m_a;
	op_a();
	m_skip = m_a >= a;
}

void mm76_device::op_com()
{
	// COM: complement A
	m_a ^= 0xf;
}

void mm76_device::op_rc()
{
	// RC: reset carry
	m_c = 0;
}

void mm76_device::op_sc()
{
	// SC: set carry
	m_c = 1;
}

void mm76_device::op_sknc()
{
	// SKNC: skip on no carry
	m_skip = !m_c_in;
}

void mm76_device::op_lai()
{
	// LAI x: load A from x (successive LAI are ignored)
	if (!(op_is_lai(m_prev_op) && !op_is_tr(m_prev2_op)))
		m_a = m_op & 0xf;
}

void mm76_device::op_aisk()
{
	// AISK x: add x to A, skip on no overflow
	m_a += m_op & 0xf;
	m_skip = !(m_a & 0x10);
	m_a &= 0xf;
}


// ROM addressing instructions

void mm76_device::op_rt()
{
	// RT: return from subroutine
	cycle();
	pop_pc();
}

void mm76_device::op_rtsk()
{
	// RTSK: RT + skip next instruction
	op_rt();
	m_skip = true;
}

void mm76_device::op_t()
{
	// T x: transfer on-page
	cycle();

	// jumps from subroutine pages reset page to SR1
	u16 mask = m_prgmask & ~0x7f;
	if ((m_pc & mask) == mask)
		m_pc &= ~0x40;

	m_pc = (m_pc & ~0x3f) | (~m_op & 0x3f);
}

void mm76_device::op_tl()
{
	// TL x: transfer long off-page
	cycle();
	m_pc = (~m_prev_op & 0xf) << 6 | (~m_op & 0x3f);
}

void mm76_device::op_tm()
{
	// TM x: transfer and mark to SR0
	cycle();

	// calls from subroutine pages don't push PC
	u16 mask = m_prgmask & ~0x7f;
	if ((m_pc & mask) != mask)
		push_pc();

	m_pc = ((m_prgmask & ~0x3f) | (~m_op & 0x3f));
}

void mm76_device::op_tml()
{
	// TML x: transfer and mark long
	cycle();
	push_pc();
	m_pc = (~m_prev_op & 0xf) << 6 | (~m_op & 0x3f);
}

void mm76_device::op_tr()
{
	// TR x: prefix for extended opcode
}

void mm76_device::op_nop()
{
	// NOP: no operation
}


// logical comparison instructions

void mm76_device::op_skmea()
{
	// SKMEA: skip on memory equals A
	m_skip = m_a == ram_r();
}

void mm76_device::op_skbei()
{
	// SKBEI x: skip on Bl equals x
	m_skip = (m_b & 0xf) == (m_op & 0xf);
}

void mm76_device::op_skaei()
{
	// SKAEI x: skip on A equals X
	m_skip = m_a == (~m_op & 0xf);
}


// input/output instructions

void mm76_device::op_ibm()
{
	// IBM: input channel B to A
	m_a &= (m_read_r() & m_r_output) >> 4;
}

void mm76_device::op_ob()
{
	// OB: output from A to channel B
	m_r_output = (m_r_output & 0xf) | m_a << 4;
	m_write_r(m_r_output);
}

void mm76_device::op_iam()
{
	// IAM: input channel A to A
	m_a &= m_read_r() & m_r_output;
}

void mm76_device::op_oa()
{
	// OA: output from A to channel A
	m_r_output = (m_r_output & ~0xf) | m_a;
	m_write_r(m_r_output);
}

void mm76_device::op_ios()
{
	// IOS: start serial I/O
	m_sclock_count = 8;
}

void mm76_device::op_i1()
{
	// I1: input channel 1 to A
	m_a = m_read_p() & 0xf;
}

void mm76_device::op_i2c()
{
	// I2C: input channel 2 to A
	m_a = ~m_read_p() >> 4 & 0xf;
}

void mm76_device::op_int1h()
{
	// INT1H: skip on INT1 high
	m_skip = bool(m_int_line[1]);
}

void mm76_device::op_din1()
{
	// DIN1: test INT1 flip-flop
	m_skip = !m_int_ff[1];
	m_int_ff[1] = 1;
}

void mm76_device::op_int0l()
{
	// INT0L: skip on INT0 low
	m_skip = !m_int_line[0];
}

void mm76_device::op_din0()
{
	// DIN0: test INT0 flip-flop
	m_skip = !m_int_ff[0];
	m_int_ff[0] = 1;
}

void mm76_device::op_seg1()
{
	// SEG1: output A+carry through PLA to channel A
	u8 out = bitswap<8>(m_opla->read((m_c_in << 4 | (ram_r() & ~m_a)) ^ 0x1f), 7,5,3,1,0,2,4,6);
	m_r_output = (m_r_output & ~0xf) | (out & 0xf);
	m_write_r(m_r_output);
}

void mm76_device::op_seg2()
{
	// SEG2: output A+carry through PLA to channel B
	u8 out = bitswap<8>(m_opla->read((m_c_in << 4 | (ram_r() & ~m_a)) ^ 0x1f), 7,5,3,1,0,2,4,6);
	m_r_output = (m_r_output & 0xf) | (out & 0xf0);
	m_write_r(m_r_output);
}
