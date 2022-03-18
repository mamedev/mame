// license:BSD-3-Clause
// copyright-holders:hap

// B5000 opcode handlers

#include "emu.h"
#include "b5000.h"


// internal helpers

inline u8 b5000_cpu_device::ram_r()
{
	return m_data->read_byte(m_ram_addr) & 0xf;
}

inline void b5000_cpu_device::ram_w(u8 data)
{
	m_data->write_byte(m_ram_addr, data & 0xf);
}

void b5000_cpu_device::pop_pc()
{
	m_pc = m_s;
}

void b5000_cpu_device::push_pc()
{
	m_s = m_pc;
}

void b5000_cpu_device::set_bu(u8 bu)
{
	m_bu = bu & 3;

	// changing from 0 to non-0 or vice versa delays RAM address modification
	if ((m_bu && !m_prev_bu) || (!m_bu && m_prev_bu))
		m_bu_delay = true;
}

void b5000_cpu_device::op_illegal()
{
	logerror("unknown opcode $%02X at $%03X\n", m_op, m_prev_pc);
}


// opcodes

// ROM addressing instructions

void b5000_cpu_device::op_tl()
{
	// x
}

void b5000_cpu_device::op_tra1(u8 step)
{
	// x
}

void b5000_cpu_device::op_tra0(u8 step)
{
	// x
}

void b5000_cpu_device::op_ret(u8 step)
{
	// x
}

void b5000_cpu_device::op_nop()
{
	// NOP: no operation
}


// RAM addressing instructions

void b5000_cpu_device::op_lb(u8 bl)
{
	// LB x,y: load B from x,y (successive LB are ignored)
	if (!op_is_lb(m_prev_op))
	{
		m_bl = bl;
		set_bu(m_op & 3);
	}
}

void b5000_cpu_device::op_atb()
{
	// ATB: load Bl from A (ignore if previous opcode was LB)
	if (!op_is_lb(m_prev_op))
		m_bl = m_a;
}

void b5000_cpu_device::op_lda()
{
	// LDA x: load A from RAM, XOR Bu with x
	m_a = ram_r();
	set_bu(m_op ^ m_bu);
}

void b5000_cpu_device::op_exc0()
{
	// EXC x,0: exchange A with RAM, XOR Bu with x
	u8 a = m_a;
	m_a = ram_r();
	ram_w(a);
	set_bu(m_op ^ m_bu);
}

void b5000_cpu_device::op_excp()
{
	// EXC x,+1: EXC x,0 + increment Bl and skip on 3-bit overflow
	op_exc0();
	m_bl = (m_bl + 1) & 0xf;
	m_skip = (m_bl & 7) == 0;
	m_bl_delay = true;
}

void b5000_cpu_device::op_excm()
{
	// EXC x,-1: EXC x,0 + decrement Bl and skip on overflow
	op_exc0();
	m_bl = (m_bl - 1) & 0xf;
	m_skip = (m_bl == 0xf);
	m_bl_delay = true;
}

void b5000_cpu_device::op_sm()
{
	// SM x: set bit x in RAM
	ram_w(ram_r() | (1 << (m_op & 3)));
}

void b5000_cpu_device::op_rsm()
{
	// RSM x: reset bit x in RAM
	ram_w(ram_r() & ~(1 << (m_op & 3)));
}

void b5000_cpu_device::op_tm()
{
	// TM x: skip next if bit x in RAM is clear
	m_skip = !BIT(ram_r(), m_op & 3);
}

void b5000_cpu_device::op_tam()
{
	// TAM: skip next if A equals RAM
	m_skip = (m_a == ram_r());
}


// arithmetic instructions

void b5000_cpu_device::op_lax()
{
	// LAX x: load A from x
	m_a = ~m_op & 0xf;
}

void b5000_cpu_device::op_comp()
{
	// COMP: complement A
	m_a ^= 0xf;
}

void b5000_cpu_device::op_adx()
{
	// ADX x: add x to A, skip on no overflow
	m_a += ~m_op & 0xf;
	m_skip = !(m_a & 0x10);
	m_a &= 0xf;
}

void b5000_cpu_device::op_add()
{
	// ADD (C),(S): add RAM to A (optional carry/skip)
	m_a += ram_r();
	if (~m_op & 2)
	{
		m_a += m_c;
		m_c = BIT(m_a, 4);
	}
	if (m_op & 1)
		m_skip = !BIT(m_a, 4);
	m_a &= 0xf;
}

void b5000_cpu_device::op_sc()
{
	// SC: set carry
	m_c = 1;
}

void b5000_cpu_device::op_rsc()
{
	// RSC: reset carry
	m_c = 0;
}

void b5000_cpu_device::op_tc()
{
	// TC: skip next on carry
	m_skip = bool(m_c);
}


// I/O instructions

void b5000_cpu_device::op_kseg()
{
	// KSEG: reset segment outputs
}

void b5000_cpu_device::op_atbz(u8 step)
{
	// ATBZ (aka ATB on B5xxx): ATB + load strobe (multi step)
	if (step)
		m_atbz_step++;
}

void b5000_cpu_device::op_tkb()
{
	// x
}

void b5000_cpu_device::op_tkbs(u8 step)
{
	// x
}

void b5000_cpu_device::op_read()
{
	// x
}

void b5000_cpu_device::op_tdin()
{
	// x
}
