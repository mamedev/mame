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

// RAM address instructions

void sm510_base_device::op_lb()
{
	// x
}

void sm510_base_device::op_lbl()
{
	// x
}

void sm510_base_device::op_sbm()
{
	// x
}

void sm510_base_device::op_exbla()
{
	// x
}

void sm510_base_device::op_incb()
{
	// x
}

void sm510_base_device::op_decb()
{
	// x
}


// ROM address instructions

void sm510_base_device::op_atpl()
{
	// x
}

void sm510_base_device::op_rtn0()
{
	// x
}

void sm510_base_device::op_rtn1()
{
	// x
}

void sm510_base_device::op_tl()
{
	// x
}

void sm510_base_device::op_tml()
{
	// x
}

void sm510_base_device::op_tm()
{
	// x
}

void sm510_base_device::op_t()
{
	// x
}


// Data transfer instructions

void sm510_base_device::op_exc()
{
	// x
}

void sm510_base_device::op_bdc()
{
	// x
}

void sm510_base_device::op_exci()
{
	// x
}

void sm510_base_device::op_excd()
{
	// x
}

void sm510_base_device::op_lda()
{
	// x
}

void sm510_base_device::op_lax()
{
	// x
}

void sm510_base_device::op_wr()
{
	// x
}

void sm510_base_device::op_ws()
{
	// x
}


// I/O instructions

void sm510_base_device::op_kta()
{
	// x
}

void sm510_base_device::op_atbp()
{
	// x
}

void sm510_base_device::op_atl()
{
	// x
}

void sm510_base_device::op_atfc()
{
	// x
}

void sm510_base_device::op_atr()
{
	// x
}


// Arithmetic instructions

void sm510_base_device::op_add()
{
	// x
}

void sm510_base_device::op_add11()
{
	// x
}

void sm510_base_device::op_adx()
{
	// x
}

void sm510_base_device::op_coma()
{
	// x
}

void sm510_base_device::op_rot()
{
	// x
}

void sm510_base_device::op_rc()
{
	// x
}

void sm510_base_device::op_sc()
{
	// x
}


// Test instructions

void sm510_base_device::op_tb()
{
	// x
}

void sm510_base_device::op_tc()
{
	// x
}

void sm510_base_device::op_tam()
{
	// x
}

void sm510_base_device::op_tmi()
{
	// x
}

void sm510_base_device::op_ta0()
{
	// x
}

void sm510_base_device::op_tabl()
{
	// x
}

void sm510_base_device::op_tis()
{
	// x
}

void sm510_base_device::op_tal()
{
	// x
}

void sm510_base_device::op_tf1()
{
	// x
}

void sm510_base_device::op_tf4()
{
	// x
}


// Bit manipulation instructions

void sm510_base_device::op_rm()
{
	// x
}

void sm510_base_device::op_sm()
{
	// x
}


// Special instructions

void sm510_base_device::op_skip()
{
	// no operation
}

void sm510_base_device::op_cend()
{
	// clock stop
}

void sm510_base_device::op_idiv()
{
	// reset DIV
}

void sm510_base_device::op_illegal()
{
	logerror("%s unknown opcode $%03X at $%04X\n", tag(), m_op, m_prev_pc);
}
