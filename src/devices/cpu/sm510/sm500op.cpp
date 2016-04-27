// license:BSD-3-Clause
// copyright-holders:hap

// SM500 opcode handlers

#include "sm500.h"


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
	// INCB: increment BL, skip next on overflow, of 3rd bit!
	m_bl = (m_bl + 1) & 0xf;
	m_skip = (m_bl == 8);
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


// Arithmetic instructions


// Data transfer instructions

void sm500_device::op_pdtw()
{
}

void sm500_device::op_tw()
{
}

void sm500_device::op_dtw()
{
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
}

void sm500_device::op_smf()
{
}

void sm500_device::op_comcn()
{
}

// Test instructions
