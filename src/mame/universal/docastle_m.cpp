// license:BSD-3-Clause
// copyright-holders:Brad Oliver
/***************************************************************************

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "docastle.h"

/*

Communication between the two CPUs happens through a single bidirectional latch.
Whenever maincpu reads or writes it, its WAIT input is asserted. It is implicitly
cleared by subcpu, when it accesses the latch. This enforces synchronization
between the two CPUs.

It is initiated by maincpu triggering an NMI on subcpu. During this process,
timing needs to be cycle-accurate, both CPUs do LDIR opcodes in lockstep.

*/

uint8_t docastle_state::main_from_sub_r()
{
	if (!machine().side_effects_disabled())
	{
		m_maincpu_defer_access = !m_maincpu_defer_access;

		if (m_maincpu_defer_access)
		{
			machine().scheduler().perfect_quantum(attotime::from_usec(100));
			m_maincpu->set_input_line(Z80_INPUT_LINE_WAIT, ASSERT_LINE);

			// defer access to let subcpu write the latch and clear WAIT
			m_maincpu->defer_access();
		}
	}

	return m_shared_latch;
}

void docastle_state::main_to_sub_w(uint8_t data)
{
	m_shared_latch = data;
	machine().scheduler().perfect_quantum(attotime::from_usec(100));
	m_maincpu->set_input_line(Z80_INPUT_LINE_WAIT, ASSERT_LINE);
}

uint8_t docastle_state::sub_from_main_r()
{
	if (!machine().side_effects_disabled())
	{
		m_subcpu_defer_access = !m_subcpu_defer_access;

		if (m_subcpu_defer_access)
		{
			// defer access to let maincpu react first
			m_subcpu->defer_access();
		}
		else
			m_maincpu->set_input_line(Z80_INPUT_LINE_WAIT, CLEAR_LINE);
	}

	return m_shared_latch;
}

void docastle_state::sub_to_main_w(uint8_t data)
{
	m_subcpu_defer_access = !m_subcpu_defer_access;

	if (m_subcpu_defer_access)
	{
		// defer access to let maincpu react first
		m_subcpu->defer_access();
	}
	else
	{
		m_shared_latch = data;
		m_maincpu->set_input_line(Z80_INPUT_LINE_WAIT, CLEAR_LINE);
	}
}


void docastle_state::subcpu_nmi_w(uint8_t data)
{
	m_subcpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}
