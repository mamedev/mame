// license:BSD-3-Clause
// copyright-holders:hap

// SM510 opcode handlers

// internal helpers

inline UINT8 sm510_base_device::ram_r()
{
	UINT8 address = (m_bm << 4 | m_bl) & m_datamask;
	return m_data->read_byte(address) & 0xf;
}

inline void sm510_base_device::ram_w(UINT8 data)
{
	UINT8 address = (m_bm << 4 | m_bl) & m_datamask;
	m_data->write_byte(address, data & 0xf);
}

void sm510_base_device::pop_stack()
{
	m_pc = m_stack[0] & m_prgmask;
	for (int i = 0; i < m_stack_levels-1; i++)
		m_stack[i] = m_stack[i+1];
}

void sm510_base_device::push_stack()
{
	for (int i = m_stack_levels-1; i >= 1; i--)
		m_stack[i] = m_stack[i-1];
	m_stack[0] = m_pc;
}



// instruction set

void sm510_base_device::op_illegal()
{
	logerror("%s unknown opcode $%03X at $%04X\n", tag(), m_op, m_prev_pc);
}
