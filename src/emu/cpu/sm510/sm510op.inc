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
	// LB x: load BM/BL with 4-bit immediate value (partial)
	op_illegal();
}

void sm510_base_device::op_lbl()
{
	// LBL xy: load BM/BL with 8-bit immediate value
	op_illegal();
}

void sm510_base_device::op_sbm()
{
	// SBM: set BM high bit for next opcode
	op_illegal();
}

void sm510_base_device::op_exbla()
{
	// EXBLA: exchange BL with ACC
	UINT8 a = m_acc;
	m_acc = m_bl;
	m_bl = a;
}

void sm510_base_device::op_incb()
{
	// INCB: increment BL, skip next on overflow
	m_bl = (m_bl + 1) & 0xf;
	m_skip = (m_bl == 0);
}

void sm510_base_device::op_decb()
{
	// DECB: decrement BL, skip next on overflow
	m_bl = (m_bl - 1) & 0xf;
	m_skip = (m_bl == 0xf);
}


// ROM address instructions

void sm510_base_device::op_atpl()
{
	// ATPL: load PC low bits with ACC
	m_pc = (m_pc & ~0xf) | m_acc;
}

void sm510_base_device::op_rtn0()
{
	// RTN0: return from subroutine
	pop_stack();
}

void sm510_base_device::op_rtn1()
{
	// RTN1: return from subroutine, skip next
	op_rtn0();
	m_skip = true;
}

void sm510_base_device::op_tl()
{
	// TL xyz: longjump
	op_illegal();
}

void sm510_base_device::op_tml()
{
	// TML xyz: x
	op_illegal();
}

void sm510_base_device::op_tm()
{
	// TM xyz: x
	op_illegal();
}

void sm510_base_device::op_t()
{
	// T xy: jump within current page
	m_pc = (m_pc & ~0x3f) | (m_op & 0x3f);
}


// Data transfer instructions

void sm510_base_device::op_exc()
{
	// EXC x: exchange ACC with RAM, xor BM with x
	UINT8 a = m_acc;
	m_acc = ram_r();
	ram_w(a);
	m_bm ^= (m_op & 3);
}

void sm510_base_device::op_bdc()
{
	// BDC: x
	op_illegal();
}

void sm510_base_device::op_exci()
{
	// EXCI x: EXC x, INCB
	op_exc();
	op_incb();
}

void sm510_base_device::op_excd()
{
	// EXCD x: EXC x, DECB
	op_exc();
	op_decb();
}

void sm510_base_device::op_lda()
{
	// LDA x: load ACC with RAM, xor BM with x
	m_acc = ram_r();
	m_bm ^= (m_op & 3);
}

void sm510_base_device::op_lax()
{
	// LAX x: load ACC with immediate value, skip any next LAX
	if ((m_op & ~0xf) != (m_prev_op & ~0xf))
		m_acc = m_op & 0xf;
}

void sm510_base_device::op_wr()
{
	// WR: shift 0 into W
	op_illegal();
}

void sm510_base_device::op_ws()
{
	// WR: shift 1 into W
	op_illegal();
}


// I/O instructions

void sm510_base_device::op_kta()
{
	// KTA: input K to ACC
	op_illegal();
}

void sm510_base_device::op_atbp()
{
	// ATBP: output ACC to BP
	op_illegal();
}

void sm510_base_device::op_atl()
{
	// ATL: input L to ACC
	op_illegal();
}

void sm510_base_device::op_atfc()
{
	// ATFC: input Y to ACC
	op_illegal();
}

void sm510_base_device::op_atr()
{
	// ATR: output ACC to R
	op_illegal();
}


// Arithmetic instructions

void sm510_base_device::op_add()
{
	// ADD: add RAM to ACC
	m_acc = (m_acc + ram_r()) & 0xf;
}

void sm510_base_device::op_add11()
{
	// ADD11: add RAM and carry to ACC and carry, skip next on carry
	m_acc += ram_r() + m_c;
	m_c = m_acc >> 4 & 1;
	m_skip = (m_c == 1);
	m_acc &= 0xf;
}

void sm510_base_device::op_adx()
{
	// ADX x: add immediate value to ACC, skip next on carry
	m_acc += (m_op & 0xf);
	m_skip = (m_acc & 0x10) ? true : false;
	m_acc &= 0xf;
}

void sm510_base_device::op_coma()
{
	// COMA: complement ACC
	m_acc ^= 0xf;
}

void sm510_base_device::op_rot()
{
	// ROT: rotate ACC left through carry
	m_acc = m_acc << 1 | m_c;
	m_c = m_acc >> 4 & 1;
	m_acc &= 0xf;
}

void sm510_base_device::op_rc()
{
	// RC: reset carry
	m_c = 0;
}

void sm510_base_device::op_sc()
{
	// SC: set carry
	m_c = 1;
}


// Test instructions

void sm510_base_device::op_tb()
{
	// TB: x
	op_illegal();
}

void sm510_base_device::op_tc()
{
	// TC: skip next if no carry
	m_skip = !m_c;
}

void sm510_base_device::op_tam()
{
	// TAM: skip next if ACC equals RAM
	m_skip = (m_acc == ram_r());
}

void sm510_base_device::op_tmi()
{
	// TMI x: skip next if RAM bit is set
	m_skip = (ram_r() & 1 << (m_op & 3)) ? true : false;
}

void sm510_base_device::op_ta0()
{
	// TA0: skip next if ACC is clear
	m_skip = !m_acc;
}

void sm510_base_device::op_tabl()
{
	// TABL: skip next of ACC equals BL
	m_skip = (m_acc == m_bl);
}

void sm510_base_device::op_tis()
{
	// TIS: x
	op_illegal();
}

void sm510_base_device::op_tal()
{
	// TAL: x
	op_illegal();
}

void sm510_base_device::op_tf1()
{
	// TF1: x
	op_illegal();
}

void sm510_base_device::op_tf4()
{
	// TF4: x
	op_illegal();
}


// Bit manipulation instructions

void sm510_base_device::op_rm()
{
	// RM x: reset RAM bit
	ram_w(ram_r() & ~(1 << (m_op & 3)));
}

void sm510_base_device::op_sm()
{
	// SM x: set RAM bit
	ram_w(ram_r() | (1 << (m_op & 3)));
}


// Special instructions

void sm510_base_device::op_skip()
{
	// SKIP: no operation
}

void sm510_base_device::op_cend()
{
	// CEND: stop clock
	op_illegal();
}

void sm510_base_device::op_idiv()
{
	// IDIV: reset divider
	op_illegal();
}

void sm510_base_device::op_illegal()
{
	logerror("%s unknown opcode $%03X at $%04X\n", tag(), m_op, m_prev_pc);
}
