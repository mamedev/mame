// license:BSD-3-Clause
// copyright-holders:hap

// B5000 common opcode handlers

#include "emu.h"
#include "b5000.h"


// internal helpers

u8 b5000_cpu_device::ram_r()
{
	return m_data->read_byte(m_ram_addr) & 0xf;
}

void b5000_cpu_device::ram_w(u8 data)
{
	m_data->write_byte(m_ram_addr, data & 0xf);
}

void b5000_cpu_device::set_pc(u8 pu, u8 pl)
{
	m_pc = ((pu << 6) | (pl & 0x3f)) & m_prgmask;
}

void b5000_cpu_device::set_bu(u8 bu)
{
	m_bu = bu & 3;

	// changing to or from 0 delays RAM address modification
	if (bool(m_bu) != bool(m_prev_bu))
		m_bu_delay = true;
}

void b5000_cpu_device::seg_w(u16 seg)
{
	m_write_seg(m_seg = seg);
}

void b5000_cpu_device::op_illegal()
{
	logerror("unknown opcode $%02X at $%03X\n", m_op, m_prev_pc);
}


// opcodes

// ROM addressing instructions

void b5000_cpu_device::op_tl()
{
	// TL z: set Pu to z
	set_pc(m_op & 0xf, m_pc);

	// S is actually only 6-bit
	m_s = (m_pc & ~0x3f) | (m_s & 0x3f);
}

void b5000_cpu_device::op_tra_step()
{
	assert(m_tra_step > 0);

	// TRA 0/1,x: call/jump to x (multi step)
	switch (m_tra_step)
	{
		// step 1: skip next opcode
		// TL is unskippable, that's how it does long jumps
		case 1:
			m_skip = true;
			break;

		// step 2: handle the call/jump
		case 2:
			if (!m_sr && ~m_prev_op & 0x40)
			{
				// call: push P to save register
				m_sr = true;
				m_s = (m_s & ~0x3f) | (m_prev_pc & 0x3f);
			}
			if (m_sr)
			{
				// SR set: set Pu to subroutine page
				set_pc(sr_page() ^ BIT(m_prev_op, 6), m_pc);
			}

			// set Pl to x
			set_pc(m_pc >> 6, m_prev_op);
			m_tra_step = 0;
			return;

		default:
			break;
	}
	m_tra_step++;
}

void b5000_cpu_device::op_ret_step()
{
	assert(m_ret_step > 0);

	// RET: return from subroutine (multi step)
	switch (m_ret_step)
	{
		// step 1: skip next opcode
		// a TL after RET will return to the page specified by TL
		case 1:
			m_skip = true;
			break;

		// step 2: handle the ret
		case 2:
			m_pc = m_s;
			m_sr = false;
			m_ret_step = 0;
			return;

		default:
			break;
	}
	m_ret_step++;
}

void b5000_cpu_device::op_nop()
{
	// NOP: no operation
}


// RAM addressing instructions

void b5000_cpu_device::op_lb(u8 bl)
{
	// LB x,y: load B from x,y (successive LB/ATB are ignored)
	if (!op_is_lb(m_prev_op) && !op_is_atb(m_prev_op))
	{
		m_bl = bl;
		set_bu(m_op & 3);
	}
}

void b5000_cpu_device::op_atb()
{
	// ATB: load Bl from A (successive LB/ATB are ignored)
	if (!op_is_lb(m_prev_op) && !op_is_atb(m_prev_op))
	{
		m_bl = m_a;
		m_bl_delay = true;
	}
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
	m_skip = !BIT(m_a, 4);
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
	seg_w(0);
}

void b5000_cpu_device::op_atb_step()
{
	assert(m_atb_step > 0);

	// ATB: ATB + load strobe (multi step)
	switch (m_atb_step)
	{
		// step 1: ATB + KSEG
		case 1:
			op_atb();
			op_kseg();
			break;

		// step 3: disable strobe
		case 3:
			m_write_str(0);
			break;

		// step 4: load strobe from Bl
		case 4:
			m_write_str(1 << m_prev_bl);
			m_atb_step = 0;
			return;

		default:
			break;
	}
	m_atb_step++;
}

void b5000_cpu_device::op_tkb()
{
	// TKB: skip next if any KB is high
	m_skip = (m_read_kb() & 0xf) != 0;
}

void b5000_cpu_device::op_tkbs()
{
	// TKBS: TKB + load segments
	op_tkb();

	// note: SEG0(DP) from C flag is delayed 2 cycles
	seg_w(m_seg | decode_digit(m_prev3_c << 4 | ram_r()));
}

void b5000_cpu_device::op_read()
{
	// READ: add _KB to A, skip next on no overflow
	m_a += (~m_read_kb() & 0xf);
	m_skip = !BIT(m_a, 4);
	m_a &= 0xf;
}

void b5000_cpu_device::op_tdin()
{
	// TDIN x: skip next if DIN x is high
	m_skip = bool(BIT(m_read_din(), (m_op - 1) & 3));
}
