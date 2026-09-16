// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Igor

// SM500 shared opcode handlers

#include "emu.h"
#include "sm500.h"


// internal helpers

void sm500_device::shift_w()
{
	// shifts internal W' latches
	for (int i = 0; i < (m_o_pins-1); i++)
		m_ox[i] = m_ox[i + 1];
}

u8 sm500_device::get_digit()
{
	// default digit segments PLA
	static const u8 lut_digits[0x20] =
	{
		0xe, 0x0, 0xc, 0x8, 0x2, 0xa, 0xe, 0x2, 0xe, 0xa, 0x0, 0x0, 0x2, 0xa, 0x2, 0x2,
		0xb, 0x9, 0x7, 0xf, 0xd, 0xe, 0xe, 0xb, 0xf, 0xf, 0x4, 0x0, 0xd, 0xe, 0x4, 0x0
	};

	// row select is from BP d3 (aka CN flag)
	u8 sel = BIT(m_bp, 3);
	return lut_digits[sel << 4 | m_acc] | (~sel & m_mx);
}


// instruction set

// RAM address instructions

void sm500_device::op_lb()
{
	// LB x: load BM/BL with 4-bit immediate value (partial)
	m_bm = m_op & 3;
	m_bl = (m_op >> 2 & 3) | ((m_op & 0xc) ? 8 : 0);
}

void sm500_device::op_incb()
{
	// INCB: increment BL, but overflow on 3rd bit!
	sm510_base_device::op_incb();
	m_skip = (m_bl == 8);
}

void sm500_device::op_sbm()
{
	// SBM: set RAM address high bit
	m_bm |= 4;
}

void sm500_device::op_rbm()
{
	// RBM: reset RAM address high bit
	m_bm &= ~4;
}


// ROM address instructions

void sm500_device::op_comcb()
{
	// COMCB: complement CB
	m_cb ^= 1;
}

void sm500_device::op_rtn0()
{
	// RTN0(RTN): return from subroutine
	sm510_base_device::op_rtn0();
	m_rsub = false;
}

void sm500_device::op_ssr()
{
	// SSR x: set stack upper bits, also sets E flag for next opcode
	set_su(m_op & 0xf);
}

void sm500_device::op_tr()
{
	// TR x: jump (long or short)
	m_pc = (m_pc & ~0x3f) | (m_op & 0x3f);
	if (!m_rsub)
		do_branch(m_cb, get_su(), m_pc & 0x3f);
}

void sm500_device::op_trs()
{
	// TRS x: call subroutine
	if (!m_rsub)
	{
		m_rsub = true;
		u8 su = get_su();
		push_stack();
		do_branch(get_trs_field(), 0, m_op & 0x3f);

		// E flag was set?
		if ((m_prev_op & 0xf0) == 0x70)
			do_branch(m_cb, su, m_pc & 0x3f);
	}
	else
		m_pc = (m_pc & ~0xff) | (m_op << 2 & 0xc0) | (m_op & 0xf);
}


// Data transfer instructions

void sm500_device::op_ptw()
{
	// PTW: partial transfer W' to W
	m_o[m_o_pins-1] = m_ox[m_o_pins-1];
	m_o[m_o_pins-2] = m_ox[m_o_pins-2];
}

void sm500_device::op_tw()
{
	// TW: transfer W' to W
	for (int i = 0; i < m_o_pins; i++)
		m_o[i] = m_ox[i];
}

void sm500_device::op_pdtw()
{
	// PDTW: partial shift digit into W'
	m_ox[m_o_pins-2] = m_ox[m_o_pins-1];
	m_ox[m_o_pins-1] = get_digit();
}

void sm500_device::op_dtw()
{
	// DTW: shift digit into W'
	shift_w();
	m_ox[m_o_pins-1] = get_digit();
}

void sm500_device::op_wr()
{
	// WR: shift ACC into W', reset last bit
	shift_w();
	m_ox[m_o_pins-1] = m_acc & 7;
}

void sm500_device::op_ws()
{
	// WR: shift ACC into W', set last bit
	shift_w();
	m_ox[m_o_pins-1] = m_acc | 8;
}


// I/O instructions

void sm500_device::op_ats()
{
	// ATS: transfer ACC to S
	m_s = m_acc;
}

void sm500_device::op_exksa()
{
	// EXKSA: x
}

void sm500_device::op_exkfa()
{
	// EXKFA: x
}


// Divider manipulation instructions

void sm500_device::op_idiv()
{
	// IDIV: reset divider low 9 bits
	m_div &= 0x3f;
}


// Bit manipulation instructions

void sm500_device::op_rmf()
{
	// RMF: reset m' flag, also clears ACC
	m_mx = 0;
	m_acc = 0;
}

void sm500_device::op_smf()
{
	// SMF: set m' flag
	m_mx = 1;
}

void sm500_device::op_comcn()
{
	// COMCN: complement CN flag
	m_bp ^= 8;
}
