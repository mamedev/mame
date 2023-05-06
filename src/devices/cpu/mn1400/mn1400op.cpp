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
