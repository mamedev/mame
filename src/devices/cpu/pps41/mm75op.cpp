// license:BSD-3-Clause
// copyright-holders:hap

// MM75 opcode handlers

#include "emu.h"
#include "mm75.h"


// opcodes (differences with mm76_device)

// unsupported opcodes

void mm75_device::op_ios()
{
	// IOS: does not have serial I/O
	op_illegal();
}

void mm75_device::op_i2c()
{
	// I2C: does not PI5-8 pins
	m_a = 0;
	op_illegal();
}


// changed opcodes

void mm75_device::op_ibm()
{
	// IBM: INT1 is shared with RIO8
	mm76_device::op_ibm();
	m_a &= ~(m_int_line[1] << 3);
}

void mm75_device::op_int1h()
{
	// INT1H: INT1 is shared with RIO8
	int state = (m_read_r() & m_r_output) >> 7 & m_int_line[1];
	m_skip = bool(state);
}
