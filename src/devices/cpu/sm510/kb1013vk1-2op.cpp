// license:BSD-3-Clause
// copyright-holders:hap, Igor

// KB1013VK1-2 opcode handlers

#include "kb1013vk1-2.h"


// instruction set

void kb1013vk12_device::op_bs0()
{
	// BS0: reset RAM address high bit
	m_bm &= ~4;
}

void kb1013vk12_device::op_bs1()
{
	// BS1: set RAM address high bit
	m_bm |= 4;
}
