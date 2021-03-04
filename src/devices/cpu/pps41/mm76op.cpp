// license:BSD-3-Clause
// copyright-holders:hap

// MM76 opcode handlers

#include "emu.h"
#include "mm76.h"


// internal helpers

inline u8 mm76_device::ram_r()
{
	return m_data->read_byte(m_b & m_datamask) & 0xf;
}

inline void mm76_device::ram_w(u8 data)
{
	m_data->write_byte(m_b & m_datamask, data & 0xf);
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


// opcodes

void mm76_device::op_nop()
{
}

void mm76_device::op_ios()
{
}
