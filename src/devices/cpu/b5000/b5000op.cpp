// license:BSD-3-Clause
// copyright-holders:hap

// B5000 opcode handlers

#include "emu.h"
#include "b5000.h"


// internal helpers

inline u8 b5000_cpu_device::ram_r()
{
	return 0;
	//return m_data->read_byte(m_b) & 0xf;
}

inline void b5000_cpu_device::ram_w(u8 data)
{
	//m_data->write_byte(m_b, data & 0xf);
}

void b5000_cpu_device::pop_pc()
{
	//m_pc = m_sa;
}

void b5000_cpu_device::push_pc()
{
	//m_sa = m_pc;
}


// opcodes
