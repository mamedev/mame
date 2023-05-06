// license:BSD-3-Clause
// copyright-holders:hap

// MN1400 common opcode handlers

#include "emu.h"
#include "mn1400.h"


// internal helpers

void mn1400_cpu_device::op_illegal()
{
	logerror("unknown opcode $%02X at $%03X\n", m_op, m_prev_pc);
}


// opcodes

// load/store instructions

void mn1400_cpu_device::op_l()
{
	// L: load A from memory
}

void mn1400_cpu_device::op_ld()
{
	// LD: load A direct from memory
}

void mn1400_cpu_device::op_li()
{
	// LI: load A immediate
}

void mn1400_cpu_device::op_lic()
{
	// LIC: L + increment Y
}

void mn1400_cpu_device::op_ldc()
{
	// LDC: L + decrement Y
}

void mn1400_cpu_device::op_st()
{
	// S: store A into memory
}

void mn1400_cpu_device::op_std()
{
	// STD: store A direct into memory
}

void mn1400_cpu_device::op_stic()
{
	// STIC: S + increment Y
}

void mn1400_cpu_device::op_stdc()
{
	// STDC: S + decrement Y
}

void mn1400_cpu_device::op_lx()
{
	// LX: load X immediate
}

void mn1400_cpu_device::op_ly()
{
	// LY: load Y immediate
}

void mn1400_cpu_device::op_tax()
{
	// TAX: transfer A to X
}

void mn1400_cpu_device::op_tay()
{
	// TAY: transfer A to Y
}

void mn1400_cpu_device::op_tya()
{
	// TYA: transfer Y to A
}

void mn1400_cpu_device::op_tacu()
{
	// TACU: transfer A to counter upper
}

void mn1400_cpu_device::op_tacl()
{
	// TACL: transfer A to counter lower
}

void mn1400_cpu_device::op_tcau()
{
	// TCAU: transfer counter upper to A
}

void mn1400_cpu_device::op_tcal()
{
	// TCAL: transfer counter lower to A
}


// arithmetic instructions

void mn1400_cpu_device::op_nop()
{
	// NOP: no operation
}

void mn1400_cpu_device::op_and()
{
	// AND: AND A with memory
}

void mn1400_cpu_device::op_andi()
{
	// ANDI: AND A with immediate
}

void mn1400_cpu_device::op_or()
{
	// OR: OR A with memory
}

void mn1400_cpu_device::op_xor()
{
	// XOR: XOR A with memory
}

void mn1400_cpu_device::op_a()
{
	// A: add memory + carry to A
}

void mn1400_cpu_device::op_ai()
{
	// AI: add immediate to A
}

void mn1400_cpu_device::op_cpl()
{
	// CPL: complement A
}

void mn1400_cpu_device::op_c()
{
	// C: compare A to memory
}

void mn1400_cpu_device::op_ci()
{
	// CI: compare A to immediate
}

void mn1400_cpu_device::op_cy()
{
	// CY: compare A to Y
}

void mn1400_cpu_device::op_sl()
{
	// SL: shift left A
}

void mn1400_cpu_device::op_icy()
{
	// ICY: increment Y
}

void mn1400_cpu_device::op_dcy()
{
	// DCY: decrement Y
}

void mn1400_cpu_device::op_icm()
{
	// ICM: increment memory
}

void mn1400_cpu_device::op_dcm()
{
	// DCM: decrement memory
}

void mn1400_cpu_device::op_sm()
{
	// SM: set memory bits
}

void mn1400_cpu_device::op_rm()
{
	// RM: reset memory bits
}

void mn1400_cpu_device::op_tb()
{
	// TB: test A bits
}


// I/O instructions

void mn1400_cpu_device::op_ina()
{
	// INA: input from port A
}

void mn1400_cpu_device::op_inb()
{
	// INB: input from port B
}

void mn1400_cpu_device::op_otd()
{
	// OTD: output A + PS to port D
}

void mn1400_cpu_device::op_otmd()
{
	// OTMD: output memory + PS to port D
}

void mn1400_cpu_device::op_ote()
{
	// OTE: output A to port E
}

void mn1400_cpu_device::op_otie()
{
	// OTIE: output immediate to port E
}

void mn1400_cpu_device::op_rco()
{
	// RCO: reset C pin
}

void mn1400_cpu_device::op_sco()
{
	// SCO: set C pin
}

void mn1400_cpu_device::op_cco()
{
	// CCO: clear C port
}


// control/branch instructions

void mn1400_cpu_device::op_rc()
{
	// RC: reset CF
}

void mn1400_cpu_device::op_rp()
{
	// RP: reset PS
}

void mn1400_cpu_device::op_sc()
{
	// SC: set CF
}

void mn1400_cpu_device::op_sp()
{
	// SP: set PS
}

void mn1400_cpu_device::op_bs01()
{
	// BS(N)0/1: branch on S pins
}

void mn1400_cpu_device::op_bpcz()
{
	// B(N)P/C/Z: branch on status
}

void mn1400_cpu_device::op_jmp()
{
	// JMP: jump
}

void mn1400_cpu_device::op_cal()
{
	// CAL: call subroutine
}

void mn1400_cpu_device::op_ret()
{
	// RET: return from subroutine
}

void mn1400_cpu_device::op_ec()
{
	// EC: enable counter
}

void mn1400_cpu_device::op_dc()
{
	// DC: disable counter
}
