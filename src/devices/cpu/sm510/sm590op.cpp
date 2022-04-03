// license:BSD-3-Clause
// copyright-holders:hap, Jonathan Gevaryahu

// SM590 opcode handlers

#include "emu.h"
#include "sm590.h"


// internal helpers

void sm590_device::do_branch(u8 pu, u8 pm, u8 pl)
{
	// set new PC(Pu/Pm/Pl) (Pu is not used on SM590)
	m_pc = ((pu << 9 & 0x200) | (pm << 7 & 0x180) | (pl & 0x07f)) & m_prgmask;
}

void sm590_device::port_w(offs_t offset, u8 data)
{
	offset &= 3;
	data &= 0xf;
	m_rports[offset] = data;
	m_write_rx[offset](offset, data);
}


// instruction set

// ROM address instructions

void sm590_device::op_tl()
{
	// TL xyz: long jump
	do_branch(BIT(m_op, 1), (m_op << 1 & 2) | BIT(m_param, 7), m_param & 0x7f);
}

void sm590_device::op_tls()
{
	// TLS xyz: long call
	push_stack();
	do_branch(BIT(m_op, 1), (m_op << 1 & 2) | BIT(m_param, 7), m_param & 0x7f);
}


// Data transfer instructions

void sm590_device::op_lblx()
{
	// LBL x: load BL with 4-bit immediate value
	m_bl = (m_op & 0xf);
}

void sm590_device::op_lbmx()
{
	// LBM x: load BM with 2-bit immediate value
	m_bm = (m_op & 0x3);
}

void sm590_device::op_str()
{
	// STR: store ACC to RAM
	ram_w(m_acc);
}

void sm590_device::op_lda()
{
	// LDA: load ACC with RAM (no BM xor)
	m_acc = ram_r();
}

void sm590_device::op_exc()
{
	// EXC: exchange ACC with RAM (no BM xor)
	u8 a = m_acc;
	m_acc = ram_r();
	ram_w(a);
}

void sm590_device::op_exax()
{
	// EXAX: exchange X with ACC
	u8 a = m_acc;
	m_acc = m_x;
	m_x = a;
}

void sm590_device::op_blta()
{
	// BLTA: load ACC with BL
	m_acc = m_bl;
}


// Arithmetic instructions

void sm590_device::op_adx()
{
	// ADX x: add immediate value to ACC, skip next on carry
	m_acc += (m_op & 0xf);
	m_skip = bool(m_acc & 0x10);
	m_acc &= 0xf;
}

void sm590_device::op_ads()
{
	// ADS: add RAM to ACC, skip next on carry
	m_acc += ram_r();
	m_skip = bool(m_acc & 0x10);
	m_acc &= 0xf;
}

void sm590_device::op_adc()
{
	// ADC: add RAM and carry to ACC and carry
	op_add11();
	m_skip = false; // no skip
}

void sm590_device::op_inbm()
{
	// INBM: increment BM
	m_bm = (m_bm + 1) & m_datamask >> 4;
}

void sm590_device::op_debm()
{
	// DEBM: decrement BM
	m_bm = (m_bm - 1) & m_datamask >> 4;
}


// Test instructions

void sm590_device::op_tax()
{
	// TAX: skip next if ACC equals 4-bit immediate value
	m_skip = (m_acc == (m_op & 0xf));
}

void sm590_device::op_tba()
{
	// TBA x: skip next if ACC bit is set
	m_skip = ((m_acc & bitmask(m_op)) != 0);
}

void sm590_device::op_tc()
{
	// TC: skip next if carry
	m_skip = bool(m_c);
}


// I/O instructions

void sm590_device::op_atr()
{
	// ATR: output ACC to R(BL)
	port_w(m_bl, m_acc);
}

void sm590_device::op_mtr()
{
	// MTR: output RAM to R(BL)
	port_w(m_bl, ram_r());
}

void sm590_device::op_rta()
{
	// RTA: load ACC with R(BL)
	u8 offset = m_bl & 3;
	m_acc = (m_rports[offset] | m_read_rx[offset](offset)) & 0xf;
}
