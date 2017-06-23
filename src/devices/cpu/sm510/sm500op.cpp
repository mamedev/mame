// license:BSD-3-Clause
// copyright-holders:hap, Igor

// SM500 shared opcode handlers

#include "emu.h"
#include "sm500.h"


// internal helpers

void sm500_device::shift_w()
{
	// shifts internal W' latches
	for (int i = 0; i < m_o_mask; i++)
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
	
	return lut_digits[m_cn << 4 | m_acc] | (~m_cn & m_mx);
}



// instruction set

// RAM address instructions

void sm500_device::op_lb()
{
	// LB x: load BM/BL with 4-bit immediate value (partial)
	// BL bit 2 is clearned, bit 3 is param bit 2|3
	m_bm = (m_op & 3);
	m_bl = ((m_op << 1 | m_op) & 8) | (m_op >> 2 & 3);
}

void sm500_device::op_incb()
{
	// INCB: same as SM510, but overflow on 3rd bit!
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
}

void sm500_device::op_ssr()
{
}

void sm500_device::op_trs()
{
}


// Data transfer instructions

void sm500_device::op_atbp()
{
	// ATBP: same as SM510, and set Cn with ACC3
	sm510_base_device::op_atbp();
	m_cn = m_acc >> 3 & 1;
}

void sm500_device::op_ptw()
{
	// PTW: partial transfer W' to W
	m_o[m_o_mask] = m_ox[m_o_mask];
	m_o[m_o_mask-1] = m_ox[m_o_mask-1];
}

void sm500_device::op_tw()
{
	// TW: transfer W' to W
	for (int i = 0; i <= m_o_mask; i++)
		m_o[i] = m_ox[i];
}

void sm500_device::op_pdtw()
{
	// PDTW: partial shift digit into W'
	m_ox[m_o_mask-1] = m_ox[m_o_mask];
	m_ox[m_o_mask] = get_digit();
}

void sm500_device::op_dtw()
{
	// DTW: shift digit into W'
	shift_w();
	m_ox[m_o_mask] = get_digit();
}

void sm500_device::op_wr()
{
	// WR: shift ACC into W', reset last bit
	shift_w();
	m_ox[m_o_mask] = m_acc & 7;
}

void sm500_device::op_ws()
{
	// WR: shift ACC into W', set last bit
	shift_w();
	m_ox[m_o_mask] = m_acc | 8;
}


// I/O instructions

void sm500_device::op_ats()
{
}

void sm500_device::op_exksa()
{
}

void sm500_device::op_exkfa()
{
}


// Divider manipulation instructions


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
	// COMCN: complement Cn flag
	m_cn ^= 1;
}
