// license:BSD-3-Clause
// copyright-holders:hap

// SM530 shared opcode handlers

#include "emu.h"
#include "sm530.h"


// internal helpers

void sm530_device::do_branch(u8 pu, u8 pl)
{
	// set new PC(Pu/Pl)
	m_pc = ((pu << 6) | (pl & 0x3f)) & m_prgmask;
}



// instruction set

// RAM address instructions

void sm530_device::op_lb()
{
	// LB x: load BM/BL with 4-bit immediate value (partial)
	m_bl = (m_op << 2 & 8) | (m_op & 1) | 6;
	m_bm = m_op >> 2 & 3;
}

void sm530_device::op_incb()
{
	// INCB: increment BL, but overflow on 3rd bit!
	sm510_base_device::op_incb();
	m_skip = (m_bl == 8);
}


// ROM address instructions

void sm530_device::op_tl()
{
	// TL xy: long jump
	do_branch((m_op << 2) | (m_param >> 6 & 3), m_param & 0x3f);
}

void sm530_device::op_trs()
{
	// TRS x: indirect subroutine call, jump vectors are on page 14
	m_icount--;
	push_stack();
	u8 jump = m_program->read_byte((14 << 6) | (m_op & 0x3f));
	do_branch(jump >> 5 & 7, jump & 0x1f);
}


// Arithmetic instructions

void sm530_device::op_adx()
{
	// ADX x: add immediate value to ACC, skip next on carry
	m_acc += (m_op & 0xf);
	m_skip = bool(m_acc & 0x10);
	m_acc &= 0xf;
}


// I/O instructions

void sm530_device::op_atbp()
{
	// ATBP: output ACC to BP
	m_bp = m_acc;
}
