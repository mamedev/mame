// license:BSD-3-Clause
// copyright-holders:hap

// HMCS400 opcode handlers

#include "emu.h"
#include "hmcs400.h"


// internal helpers



// instruction set

void hmcs400_cpu_device::op_illegal()
{
	logerror("unknown opcode $%03X at $%04X\n", m_op, m_prev_pc);
}
