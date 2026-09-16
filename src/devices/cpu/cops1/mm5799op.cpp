// license:BSD-3-Clause
// copyright-holders:hap

// MM5799 opcode handlers

#include "emu.h"
#include "mm5799.h"


// internal helpers

inline u8 mm5799_device::ram_r()
{
	return m_data->read_byte(m_b) & 0xf;
}

inline void mm5799_device::ram_w(u8 data)
{
	m_data->write_byte(m_b, data & 0xf);
}

void mm5799_device::pop_pc()
{
	m_pc = m_sa;
	m_sa = m_sb;
}

void mm5799_device::push_pc()
{
	m_sb = m_sa;
	m_sa = m_pc;
}


// opcodes

// arithmetic operations

void mm5799_device::op_ad()
{
	// add M to A
	m_a = (m_a + ram_r()) & 0xf;
}

void mm5799_device::op_add()
{
	// ADD: add M+carry to A
	m_a = m_a + ram_r() + m_c;
	m_c = (m_a >= 10) ? 1 : 0;
	m_a &= 0xf;
	m_skip = !bool(m_c);
}

void mm5799_device::op_sub()
{
	// SUB: subtract A+carry from M
	m_a = (m_a ^ 0xf) + ram_r() + m_c;
	m_c = m_a >> 4 & 1;
	m_a &= 0xf;
	m_skip = bool(m_c);
}

void mm5799_device::op_comp()
{
	// COMP: complement A
	m_a ^= 0xf;
}

void mm5799_device::op_0ta()
{
	// 0TA: clear A
	m_a = 0;
}

void mm5799_device::op_adx()
{
	// ADX x: add constant to A
	u8 x = m_op & 0xf;
	m_a += x;
	m_skip = ~m_a & 0x10 && x != 6;
	m_a &= 0xf;
}

void mm5799_device::op_hxa()
{
	// HXA: exchange A with H
	u8 h = m_h;
	m_h = m_a;
	m_a = h;
}

void mm5799_device::op_tam()
{
	// TAM: compare A with M
	m_skip = m_a == ram_r();
}

void mm5799_device::op_sc()
{
	// SC: set carry
	m_c = 1;
}

void mm5799_device::op_rsc()
{
	// RSC: reset carry
	m_c = 0;
}

void mm5799_device::op_tc()
{
	// TC: test carry
	m_skip = !m_c;
}


// input test

void mm5799_device::op_tin()
{
	// TIN: test INB pin
	bool t = !bool(m_read_inb() & 1);
	m_skip = m_option_inb_active_high ? t : !t;
}

void mm5799_device::op_tf()
{
	// TF x: test F pin
	u8 f = m_read_f.isunset() ? m_f : m_read_f();
	m_skip = !BIT(f, m_op >> 4 & 3);
}

void mm5799_device::op_tkb()
{
	// TKB: test K pins
	bool t = bool(m_read_k() & 0xf);
	m_skip = m_option_k_active_high ? t : !t;
}

void mm5799_device::op_tir()
{
	// TIR: test DO3 pin
	int d = m_read_do3.isunset() ? (m_do >> 2) : m_read_do3();
	m_skip = !bool(d & 1);
}


// input/output

void mm5799_device::op_btd()
{
	// BTD: transfer B(d) to digit output latches
	m_do = ~m_b & 0xf;
	m_write_do(m_do);
}

void mm5799_device::op_dspa()
{
	// DSPA: transfer A+H+C to segment output latches, direct
	u8 segs = bitswap<7>(m_h << 4 | m_a, 4,5,6,0,1,2,3);
	m_write_s((m_c << 7 ^ 0x80) | segs);
}

void mm5799_device::op_dsps()
{
	// DSPS: transfer A+C to segment output latches, via PLA
	u8 segs = ~m_opla->read((m_a + 1) & 0xf) & 0x7f;
	m_write_s((m_c << 7 ^ 0x80) | segs);
}

void mm5799_device::op_axo()
{
	// AXO: exchange A with serial
	u8 s = m_serial;
	m_serial = m_a;
	m_a = s;

	// mask option to read SI immediately
	if (m_option_axo_si)
		m_a = (m_a & 7) | (m_read_si() << 3 & 8);
}

void mm5799_device::op_ldf()
{
	// LDF x: load F pin(s)
	u8 mask = bitswap<4>(~m_arg, 7,5,3,1);
	u8 f = bitswap<4>(~m_arg, 6,4,2,0);
	m_f = (m_f & ~mask) | (f & mask);
	m_write_f(m_f);
}

void mm5799_device::op_read()
{
	// READ: read K pins to A
	m_a = m_read_k() & 0xf;
}


// control functions

void mm5799_device::op_go()
{
	// GO x: jump to address in page

	// jumps from page 0x1e/0x1f reset page to 0x1e
	if ((m_pc & 0x780) == 0x780)
		m_pc &= ~0x40;

	m_pc = (m_pc & ~0x3f) | (m_op & 0x3f);
}

void mm5799_device::op_call()
{
	// CALL x: call subroutine

	// calls from page 0x1e/0x1f don't push PC
	if ((m_pc & 0x780) != 0x780)
		push_pc();

	m_pc = (m_op & 0x3f) | 0x7c0;
}

void mm5799_device::op_ret()
{
	// RET: return from subroutine
	pop_pc();
}

void mm5799_device::op_rets()
{
	// RETS: return from subroutine, skip next
	op_ret();
	m_skip = true;
}

void mm5799_device::op_lg()
{
	// LG x: long go / long call
	if (~m_arg & 0x40)
		push_pc();

	m_pc = (~m_op << 7 & 0x780) | (m_arg >> 1 & 0x40) | (m_arg & 0x3f);
}

void mm5799_device::op_nop()
{
	// NOP: no operation
}


// memory digit operations

void mm5799_device::op_exc()
{
	// EXC x: exchange A with M, XOR B(r) with x
	u8 a = m_a;
	m_a = ram_r();
	ram_w(a);
	m_b ^= m_op & 0x30;
}

void mm5799_device::op_excm()
{
	// EXC- x: EXC + decrement B(d)
	op_exc();
	m_b = (m_b & ~0xf) | ((m_b - 1) & 0xf);
	m_skip = (m_b & 0xf) == 0xf;
}

void mm5799_device::op_excp()
{
	// EXC+ x: EXC + increment B(d)
	op_exc();
	m_b = (m_b & ~0xf) | ((m_b + 1) & 0xf);
	m_skip = (m_b & 0xf) == 0;

	// mask option to also skip on 13
	if (m_option_excp_skip && (m_b & 0xf) == 13)
		m_skip = true;
}

void mm5799_device::op_mta()
{
	// MTA x: load A with M, XOR B(r) with x
	m_a = ram_r();
	m_b ^= m_op & 0x30;
}

void mm5799_device::op_lm()
{
	// LM x: load M with constant, increment B(d)
	ram_w(m_op & 0xf);
	m_b = (m_b & ~0xf) | ((m_b + 1) & 0xf);
}


// memory bit operations

void mm5799_device::op_sm()
{
	// SM x: set memory bit
	u8 x = 0;

	// opcode param is scrambled for some reason
	switch (m_op & 0xf)
	{
		case 0x9: x = 1; break;
		case 0xe: x = 2; break;
		case 0xf: x = 4; break;
		case 0xa: x = 8; break;
	}
	ram_w(ram_r() | x);
}

void mm5799_device::op_rsm()
{
	// RSM x: reset memory bit
	u8 x = 0;

	// opcode param is scrambled for some reason
	switch (m_op & 0xf)
	{
		case 0x8: x = 1; break;
		case 0xc: x = 2; break;
		case 0xb: x = 4; break;
		case 0x2: x = 8; break;
	}
	ram_w(ram_r() & ~x);
}

void mm5799_device::op_tm()
{
	// TM x: test memory bit
	m_skip = !BIT(ram_r(), m_op & 3);
}


// memory address operations

void mm5799_device::op_lb()
{
	// LB x: load B (successive LB are ignored)
	if ((m_prev_op & 0xc0) == 0x00 && (m_prev_op & 0xf) >= 0xa)
		return;

	// d 10 actual value is a mask option
	u8 x = m_op & 0xf;
	if (x == 10)
		x = m_option_lb_10;

	m_b = ((m_op & 0x30) | x) & m_datamask;
}

void mm5799_device::op_lbl()
{
	// LBL x: load B fully
	m_b = m_arg & m_datamask;
}

void mm5799_device::op_atb()
{
	// ATB: transfer A to B(d)
	m_b = (m_b & ~0xf) | m_a;
}

void mm5799_device::op_bta()
{
	// BTA: transfer B(d) to A
	m_a = m_b & 0xf;
}

void mm5799_device::op_hxbr()
{
	// HXBR: exchange H with B(r)
	u8 h = m_h;
	m_h = m_b >> 4;
	m_b = (h << 4 | (m_b & 0xf)) & m_datamask;
}
