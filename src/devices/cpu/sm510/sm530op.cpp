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

void sm530_device::op_incb()
{
	// INCB: increment BL, but overflow on 3rd bit!
	sm511_device::op_incb();
	m_skip = ((m_bl & 7) == 0);
}

void sm530_device::op_lb()
{
	// LB x: load BM/BL with 4-bit immediate value (partial)
	m_bl = (m_op << 2 & 8) | (m_op & 1) | 6;
	m_bm = m_op >> 2 & 3;
}

void sm530_device::op_sabl()
{
	// SABL: set BL high bit for next opcode - handled in execute_one()
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
	do_branch(bitswap<3>(jump, 5,7,6), jump & 0x1f);
}


// Data transfer instructions

void sm530_device::op_dta()
{
	// DTA: transfer 1/100s counter to ACC
	m_acc = m_count_10ms;
}


// Arithmetic instructions

void sm530_device::op_adx()
{
	// ADX x: add immediate value to ACC, skip next on carry
	m_acc += (m_op & 0xf);
	m_skip = bool(m_acc & 0x10);
	m_acc &= 0xf;
}


// Test instructions

void sm530_device::op_tg()
{
	// TG x: skip next if gamma flag is set, reset it after
	m_skip = bool(m_gamma & bitmask(m_op));
	m_gamma &= ~bitmask(m_op);
}


// I/O instructions

void sm530_device::op_keta()
{
	// KETA: input KE to ACC
	m_acc = m_read_k() >> 4 & 0xf;
}

void sm530_device::op_ats()
{
	// ATS: output ACC to S
	m_write_s(m_acc);
}

void sm530_device::op_atf()
{
	// ATF: output ACC to F
	m_write_f(m_acc);
}

void sm530_device::op_sds()
{
	// SDS: set display enable
	m_ds = true;
}

void sm530_device::op_rds()
{
	// RDS: reset display enable
	m_ds = false;
}


// Special instructions

void sm530_device::op_idiv()
{
	// IDIV: reset divider and 1s counter
	m_div = m_subdiv = 0;
	m_count_1s = 0;
}

void sm530_device::op_inis()
{
	// INIS: reset 1/100s counter
	m_count_10ms = 0;
}
