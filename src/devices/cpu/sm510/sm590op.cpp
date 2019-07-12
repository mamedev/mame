// license:BSD-3-Clause
// copyright-holders:hap, Jonathan Gevaryahu

// SM590 opcode handlers

#include "emu.h"
#include "sm590.h"


// internal helpers

void sm590_device::do_branch(u8 pu, u8 pm, u8 pl)
{
	// set new PC(Pu/Pm/Pl)
	m_pc = (((u16)pu << 9 & 0x200) | ((u16)pm << 7 & 0x180) | (pl & 0x07f)) & m_prgmask;
}

// instruction set

void sm590_device::op_adx()
{
	// ADX x: add immediate value to ACC, skip next on carry
	m_acc += (m_op & 0xf);
	m_skip = bool(m_acc & 0x10);
	m_acc &= 0xf;
}

void sm590_device::op_tax()
{
	// TAX: skip next if ACC equals 4-bit immediate value
	m_skip = (m_acc == (m_op & 0xf));
}

void sm590_device::op_lblx()
{
	// LBL x: load BL with 4-bit immediate value
	m_bl = (m_op & 0xf);
}

void sm590_device::op_lda()
{
	// LDA: load ACC with RAM
	m_acc = ram_r();
}

void sm590_device::op_exc()
{
	// EXC: exchange ACC with RAM
	u8 a = m_acc;
	m_acc = ram_r();
	ram_w(a);
}

void sm590_device::op_atr()
{
	// ATR: output ACC to R(BL)
	m_rports[m_bl & 0x3] = m_acc; // is the mask for BL correct here? if BL is >= 4, do the writes just go nowhere?
}

void sm590_device::op_mtr()
{
	// MTR: output RAM to R(BL)
	m_rports[m_bl & 0x3] = ram_r(); // is the mask for BL correct here? if BL is >= 4, do the writes just go nowhere?
}

void sm590_device::op_str()
{
	// STR: output ACC to RAM
	ram_w(m_acc);
}

void sm590_device::op_inbm()
{
	// INBM: increment BM
	m_bm = (m_bm + 1) & 0x3; // is this mask correct?
}

void sm590_device::op_debm()
{
	// DEBM: decrement BM
	m_bm = (m_bm - 1) & 0x3; // is this mask correct?
}

void sm590_device::op_tc()
{
	// TC: skip next if carry
	m_skip = bool(m_c);
}

void sm590_device::op_rta()
{
	// RTA: load ACC with R(BL)
	m_acc = m_rports[m_bl & 0x3]; // TODO: need a read function for this; is the mask for BL correct here? if BL is >= 4, do we always read 0 or F?
}

void sm590_device::op_blta()
{
	// BLTA: load ACC with BL
	m_acc = m_bl;
}

void sm590_device::op_exax()
{
	// EXAX: exchange X with ACC
	u8 a = m_acc;
	m_acc = m_x;
	m_x = a;
}

void sm590_device::op_tba()
{
	// TBA x: skip next if ACC bit is set
	m_skip = ((m_acc & bitmask(m_op)) != 0);
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
	m_acc += ram_r() + m_c;
	m_c = m_acc >> 4 & 1;
	m_acc &= 0xf;
}

void sm590_device::op_lbmx()
{
	// LBM x: load BM with 2-bit immediate value
	m_bm = (m_op & 0x3);
}

void sm590_device::op_tl()
{
	// TL xyz: long jump (same as sm510 TL except m_op and m_param masks)
	do_branch((m_op & 2)>>1, (((m_op & 1)<<1)|((m_param&0x80)?1:0)), m_param & 0x7f);
}

void sm590_device::op_tml() // aka TLS
{
	// TLS xyz: long call (same as sm510 TML except m_param mask)
	push_stack();
	do_branch((m_op & 2)>>1, (((m_op & 1)<<1)|((m_param&0x80)?1:0)), m_param & 0x7f);
}

void sm590_device::op_t()
{
	// TR xy: jump(transfer) within current page (same as sm510 T except m_op/m_pc mask)
	m_pc = (m_pc & ~0x7f) | (m_op & 0x7f);
}
