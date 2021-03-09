// license:BSD-3-Clause
// copyright-holders:hap

// MM77/MM78 opcode handlers

#include "emu.h"
#include "mm78.h"


// opcodes

// changed opcodes

void mm78_device::op_lba()
{
	// LBA: no RAM delay
	mm76_device::op_lba();
	m_ram_delay = false;
}

void mm78_device::op_acsk()
{
	// ACSK: skip logic is inverted
	mm76_device::op_acsk();
	m_skip = !m_skip;
}

void mm78_device::op_aisk()
{
	// AISK x: don't skip if x=6 (aka DC opcode)
	mm76_device::op_aisk();
	if ((m_op & 0xf) == 6)
		m_skip = false;
}

void mm78_device::op_sb()
{
	// SB x: SB/SOS opcodes are separated
	ram_w(ram_r() | (1 << (m_op & 3)));
}

void mm78_device::op_rb()
{
	// RB x: RB/ROS opcodes are separated
	ram_w(ram_r() & ~(1 << (m_op & 3)));
}

void mm78_device::op_skbf()
{
	// SKBF x: SKBF/SKISL opcodes are separated
	m_skip = !BIT(ram_r(), m_op & 3);
}

void mm78_device::op_sos()
{
	// SOS: SB/SOS opcodes are separated

	// B7 must be low
	if (m_ram_addr & 0x40)
	{
		logerror("SOS invalid access at $%03X\n", m_prev_pc);
		return;
	}

	u8 bl = m_ram_addr & 0xf;
	if (bl < 10)
	{
		m_d_output = (m_d_output | (1 << bl)) & m_d_mask;
		m_write_d(m_d_output);
	}
	else if (bl < 12)
		m_int_ff[~bl & 1] = 1;
	else
		logerror("SOS invalid pin %d at $%03X\n", bl, m_prev_pc);
}

void mm78_device::op_ros()
{
	// ROS: RB/ROS opcodes are separated

	// B7 must be low
	if (m_ram_addr & 0x40)
	{
		logerror("ROS invalid access at $%03X\n", m_prev_pc);
		return;
	}

	u8 bl = m_ram_addr & 0xf;
	if (bl < 10)
	{
		m_d_output = m_d_output & ~(1 << bl);
		m_write_d(m_d_output);
	}
	else if (bl < 12)
		m_int_ff[~bl & 1] = 0;
	else
		logerror("ROS invalid pin %d at $%03X\n", bl, m_prev_pc);
}

void mm78_device::op_skisl()
{
	// SKISL: SKBF/SKISL opcodes are separated

	// B7 must be low
	if (m_ram_addr & 0x40)
	{
		logerror("SKISL invalid access at $%03X\n", m_prev_pc);
		return;
	}

	u8 bl = m_ram_addr & 0xf;
	if (bl < 10)
		m_skip = !BIT((m_d_output | m_read_d()) & m_d_mask, bl);
	else if (bl < 12)
		m_skip = !m_int_ff[~bl & 1];
	else
		logerror("SKISL invalid pin %d at $%03X\n", bl, m_prev_pc);
}


// new opcodes

void mm78_device::op_sag()
{
	// SAG: set Bu to 3 for the next cycle
	m_sag = true;
}

void mm78_device::op_lxa()
{
	// LXA: load X from A
	m_x = m_a;
}

void mm78_device::op_xax()
{
	// XAX: exchange A with X
	u8 a = m_a;
	m_a = m_x;
	m_x = a;
}

void mm78_device::op_tlb()
{
	// TLB x: transfer long banked
	op_tl();
	m_pc |= 0x400;
}

void mm78_device::op_tmlb()
{
	// TMLB x: transfer and mark long banked
	op_tml();
	m_pc |= 0x400;
}

void mm78_device::op_tab()
{
	// TAB: table look up transfer
	m_skip_count = m_a + 1;
	m_a = 0xf;
}

void mm78_device::op_ix()
{
	// IX: input to X from channel X(aka B)
	m_x = (m_read_r() & m_r_output) >> 4;
}

void mm78_device::op_ox()
{
	// OX: output from X to channel X(aka B)
	m_r_output = (m_r_output & 0xf) | m_x << 4;
	m_write_r(m_r_output);
}

void mm78_device::op_ioa()
{
	// IOA: exchange A with channel A
	u8 a = m_read_r() & m_r_output & 0xf;
	m_r_output = (m_r_output & ~0xf) | m_a;
	m_write_r(m_r_output);
	m_a = a;
}

void mm78_device::op_i1sk()
{
	// I1SK: add channel 1 to A, skip on no overflow
	m_a += m_read_p() & 0xf;
	m_skip = !(m_a & 0x10);
	m_a &= 0xf;
}

void mm78_device::op_int0h()
{
	// INT0H: skip on INT0 high
	m_skip = bool(m_int_line[0]);
}

void mm78_device::op_int1l()
{
	// INT1L: skip on INT1 low
	m_skip = !m_int_line[1];
}
