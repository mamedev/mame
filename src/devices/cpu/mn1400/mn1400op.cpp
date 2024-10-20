// license:BSD-3-Clause
// copyright-holders:hap

// MN1400 common opcode handlers

#include "emu.h"
#include "mn1400.h"


// internal helpers

u8 mn1400_cpu_device::ram_r()
{
	return m_data->read_byte(m_ram_address) & 0xf;
}

void mn1400_cpu_device::ram_w(u8 data)
{
	m_data->write_byte(m_ram_address, data & 0xf);
}

void mn1400_cpu_device::set_z(u8 data)
{
	if ((data & 0xf) == 0)
		m_status |= FLAG_Z;
	else
		m_status &= ~FLAG_Z;
}

void mn1400_cpu_device::set_cz(u8 data)
{
	set_z(data);

	if (data & 0x10)
		m_status |= FLAG_C;
	else
		m_status &= ~FLAG_C;
}

void mn1400_cpu_device::op_illegal()
{
	logerror("unknown opcode $%02X at $%03X\n", m_op, m_prev_pc);
}


// opcodes

// data transfer instructions

void mn1400_cpu_device::op_l()
{
	// L: load A from memory
	m_a = ram_r();
	set_z(m_a);
}

void mn1400_cpu_device::op_ld()
{
	// LD: load A direct from memory
	m_ram_address = m_op & 3;
	op_l();
}

void mn1400_cpu_device::op_li()
{
	// LI: load A immediate
	m_a = m_op & 0xf;
	set_z(m_a);
}

void mn1400_cpu_device::op_lic()
{
	// LIC: L + increment Y
	cycle();
	op_l();
	op_icy();
}

void mn1400_cpu_device::op_ldc()
{
	// LDC: L + decrement Y
	cycle();
	op_l();
	op_dcy();
}

void mn1400_cpu_device::op_st()
{
	// ST: store A into memory
	ram_w(m_a);
}

void mn1400_cpu_device::op_std()
{
	// STD: store A direct into memory
	m_ram_address = m_op & 3;
	op_st();
}

void mn1400_cpu_device::op_stic()
{
	// STIC: ST + increment Y
	cycle();
	op_st();
	op_icy();
}

void mn1400_cpu_device::op_stdc()
{
	// STDC: ST + decrement Y
	cycle();
	op_st();
	op_dcy();
}

void mn1400_cpu_device::op_lx()
{
	// LX: load X immediate
	m_x = m_op & 7;
}

void mn1400_cpu_device::op_ly()
{
	// LY: load Y immediate
	m_y = m_op & 0xf;
}

void mn1400_cpu_device::op_tax()
{
	// TAX: transfer A to X
	m_x = m_a;
}

void mn1400_cpu_device::op_tay()
{
	// TAY: transfer A to Y
	m_y = m_a;
}

void mn1400_cpu_device::op_tya()
{
	// TYA: transfer Y to A
	m_a = m_y;
	set_z(m_a);
}

void mn1400_cpu_device::op_tacu()
{
	// TACU: transfer A to counter upper
	m_counter = (m_counter & 0xf) | (m_a << 4);
}

void mn1400_cpu_device::op_tacl()
{
	// TACL: transfer A to counter lower
	m_counter = (m_counter & 0xf0) | m_a;
}

void mn1400_cpu_device::op_tcau()
{
	// TCAU: transfer counter upper to A
	m_a = m_counter >> 4;
	set_z(m_a);
}

void mn1400_cpu_device::op_tcal()
{
	// TCAL: transfer counter lower to A
	m_a = m_counter & 0xf;
	set_z(m_a);
}


// arithmetic instructions

void mn1400_cpu_device::op_nop()
{
	// NOP: no operation
}

void mn1400_cpu_device::op_and()
{
	// AND: AND A with memory
	m_a &= ram_r();
	set_z(m_a);
}

void mn1400_cpu_device::op_andi()
{
	// ANDI: AND A with immediate
	m_a &= (m_op & 0xf);
	set_z(m_a);
}

void mn1400_cpu_device::op_or()
{
	// OR: OR A with memory
	m_a |= ram_r();
	set_z(m_a);
}

void mn1400_cpu_device::op_xor()
{
	// XOR: XOR A with memory
	m_a ^= ram_r();
	set_z(m_a);
}

void mn1400_cpu_device::op_a()
{
	// A: add memory + CF to A
	u8 cf = (m_status & FLAG_C) ? 1 : 0;
	m_a += ram_r() + cf;
	set_cz(m_a);
	m_a &= 0xf;
}

void mn1400_cpu_device::op_ai()
{
	// AI: add immediate to A
	m_a += m_op & 0xf;
	set_cz(m_a);
	m_a &= 0xf;
}

void mn1400_cpu_device::op_cpl()
{
	// CPL: complement A
	m_a ^= 0xf;
	set_z(m_a);
}

void mn1400_cpu_device::op_c()
{
	// C: compare A with memory
	set_cz((m_a ^ 0xf) + ram_r() + 1);
}

void mn1400_cpu_device::op_ci()
{
	// CI: compare A with immediate
	set_cz(m_a + (~m_op & 0xf) + 1);
}

void mn1400_cpu_device::op_cy()
{
	// CY: compare Y with immediate
	set_z(m_y ^ (m_op & 0xf));
}

void mn1400_cpu_device::op_sl()
{
	// SL: shift left A
	m_a += m_a;
	set_cz(m_a);
	m_a &= 0xf;
}

void mn1400_cpu_device::op_icy()
{
	// ICY: increment Y
	m_y = (m_y + 1) & 0xf;
	set_z(m_y);
}

void mn1400_cpu_device::op_dcy()
{
	// DCY: decrement Y
	m_y = (m_y - 1) & 0xf;
	set_z(m_y);
}

void mn1400_cpu_device::op_icm()
{
	// ICM: increment memory
	cycle();
	u8 temp = ram_r() + 1;
	ram_w(temp);
	set_cz(temp);
}

void mn1400_cpu_device::op_dcm()
{
	// DCM: decrement memory
	cycle();
	u8 temp = ram_r() + 0xf;
	ram_w(temp);
	set_cz(temp);
}

void mn1400_cpu_device::op_sm()
{
	// SM: set memory bits
	cycle();
	ram_w(ram_r() | (m_op & 0xf));
}

void mn1400_cpu_device::op_rm()
{
	// RM: reset memory bits
	cycle();
	ram_w(ram_r() & (~m_op & 0xf));
}

void mn1400_cpu_device::op_tb()
{
	// TB: test A bits
	set_z(m_a & (m_op & 0xf));
}


// I/O instructions

void mn1400_cpu_device::op_ina()
{
	// INA: input from port A
	m_a = m_read_a() & 0xf;
	set_z(m_a);
}

void mn1400_cpu_device::op_inb()
{
	// INB: input from port B
	m_a = m_read_b() & 0xf;
	set_z(m_a);
}

void mn1400_cpu_device::op_otd()
{
	// OTD: output A + PS to port D
	u8 ps = (m_status & FLAG_P) ? 1 : 0;
	write_d(ps << 4 | m_a);
}

void mn1400_cpu_device::op_otmd()
{
	// OTMD: output memory + PS to port D
	u8 ps = (m_status & FLAG_P) ? 1 : 0;
	write_d(ps << 4 | ram_r());
}

void mn1400_cpu_device::op_ote()
{
	// OTE: output A to port E
	m_write_e(m_a);
}

void mn1400_cpu_device::op_otie()
{
	// OTIE: output immediate to port E
	m_write_e(m_op & 0xf);
}

void mn1400_cpu_device::op_rco()
{
	// RCO: reset C pin
	write_c(m_c & ~(1 << m_y));
}

void mn1400_cpu_device::op_sco()
{
	// SCO: set C pin
	write_c(m_c | (1 << m_y));
}

void mn1400_cpu_device::op_cco()
{
	// CCO: clear C port
	write_c(0);
}


// control/branch instructions

void mn1400_cpu_device::op_rc()
{
	// RC: reset CF
	m_status &= ~FLAG_C;
}

void mn1400_cpu_device::op_rp()
{
	// RP: reset PS
	m_status &= ~FLAG_P;
}

void mn1400_cpu_device::op_sc()
{
	// SC: set CF
	m_status |= FLAG_C;
}

void mn1400_cpu_device::op_sp()
{
	// SP: set PS
	m_status |= FLAG_P;
}

void mn1400_cpu_device::op_bs01()
{
	// BS(N)0/1: branch on S pins
	u8 mask = m_read_sns() & (m_op >> 1 & 3);
	if (bool(m_op & 1) == bool(mask))
		m_pc = (m_prev_pc & ~0xff) | m_param;
}

void mn1400_cpu_device::op_bpcz()
{
	// B(N)P/C/Z: branch on status
	u8 mask = m_status & (m_op >> 1 & 7);
	if (bool(m_op & 1) == bool(mask))
		m_pc = (m_prev_pc & ~0xff) | m_param;
}

void mn1400_cpu_device::op_jmp()
{
	// JMP: jump
	m_pc = ((m_op & 7) << 8 | m_param) & m_prgmask;
}

void mn1400_cpu_device::op_cal()
{
	// CAL: call subroutine
	m_stack[m_sp] = m_pc;
	m_sp = (m_sp + 1) % m_stack_levels;
	op_jmp();
}

void mn1400_cpu_device::op_ret()
{
	// RET: return from subroutine
	m_sp = (m_stack_levels + m_sp - 1) % m_stack_levels;
	m_pc = m_stack[m_sp] & m_prgmask;
}

void mn1400_cpu_device::op_ec()
{
	// EC: enable counter
	m_ec = true;
}

void mn1400_cpu_device::op_dc()
{
	// DC: disable counter
	m_ec = false;
}
