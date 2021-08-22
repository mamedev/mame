// license:BSD-3-Clause
// copyright-holders:hap

// MM77LA/MM78LA opcode handlers

#include "emu.h"
#include "mm78la.h"


// changed opcodes

void mm78la_device::op_ioa()
{
	// IOA: output A+carry to lower half of RO pins
	u16 mask = (1 << (m_r_pins / 2)) - 1;
	m_r_output = (m_r_output & ~mask) | (m_c_in << 4 | m_a);
	m_write_r(m_r_output);
}

void mm78la_device::op_ox()
{
	// OX: output A+carry to upper half of RO pins
	u16 mask = (1 << (m_r_pins / 2)) - 1;
	m_r_output = (m_r_output & mask) | (m_c_in << 4 | m_a) << (m_r_pins / 2);
	m_write_r(m_r_output);
}

void mm78la_device::op_ix()
{
	// IX: output A+carry to RO pins through PLA (MM78LA)
	u32 out = ~m_opla->read(m_a) & 0x0fff'ffff;
	out = bitswap<28>(out, 26,22,18,14,10,6,2,1,5,9,13,17,21,25, 27,23,19,15,11,7,3,0,4,8,12,16,20,24);
	m_r_output = out >> (m_c_in ? 0 : 14) & 0x3fff;
	m_write_r(m_r_output);
}

void mm77la_device::op_ix()
{
	// IX: output A to RO pins through PLA (MM77LA)
	u16 out = ~m_opla->read(m_a) & 0x3ff;
	m_r_output = bitswap<10>(out, 9,7,5,3,1,0,2,4,6,8);
	m_write_r(m_r_output);
}

void mm78la_device::op_ios()
{
	// IOS: set tone frequency
	m_tone_freq = m_tone_freq >> 4 | (m_a << 4);

	// state 1->2: turn on
	if (m_ios_state == 1)
	{
		m_tone_on = true;
		reset_tone_count();
	}
	else
		m_tone_on = false;

	// increment state machine
	m_ios_state = (m_ios_state + 1) % 3;
}

void mm78la_device::op_int0h()
{
	// INT0H: toggle speaker
	toggle_speaker();
}

void mm78la_device::op_int1l()
{
	// INT1L: ?
	op_todo();
}
