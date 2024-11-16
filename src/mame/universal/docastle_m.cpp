// license:BSD-3-Clause
// copyright-holders:Brad Oliver
/***************************************************************************

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "docastle.h"

#define LOG_MAINSUB (1U << 1)

#define VERBOSE (0)
#include "logmacro.h"

/*

Communication between the two CPUs happens through a single bidirectional latch.
Whenever maincpu reads or writes it, its WAIT input is asserted. It is implicitly
cleared by subcpu, when it accesses the latch. This enforces synchronization
between the two CPUs.

It is initiated by maincpu triggering an NMI on subcpu. During this process,
timing needs to be cycle-accurate, both CPUs do LDIR opcodes in lockstep.

*/

uint8_t docastle_state::main_from_sub_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		m_maincpu_wait = !m_maincpu_wait;

		if (m_maincpu_wait)
		{
			// steal 1 cycle ahead of WAIT to avoid race condition
			m_maincpu->adjust_icount(-1);

			machine().scheduler().perfect_quantum(attotime::from_usec(100));
			m_maincpu->set_input_line(Z80_INPUT_LINE_WAIT, ASSERT_LINE);

			// defer access to let subcpu write the latch and clear WAIT
			m_maincpu->defer_access();
		}
		else
		{
			LOGMASKED(LOG_MAINSUB, "%dR%02X%c", offset, m_shared_latch, (offset == 8) ? '\n' : ' ');

			// give back stolen cycle
			m_maincpu->adjust_icount(1);
		}
	}

	return m_shared_latch;
}

void docastle_state::main_to_sub_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_MAINSUB, "%dW%02X ", offset, data);

	m_shared_latch = data;
	machine().scheduler().perfect_quantum(attotime::from_usec(100));
	m_maincpu->set_input_line(Z80_INPUT_LINE_WAIT, ASSERT_LINE);
}

uint8_t docastle_state::sub_from_main_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		LOGMASKED(LOG_MAINSUB, "%dr%02X%c", offset, m_shared_latch, (offset == 8) ? '\n' : ' ');
		m_maincpu->set_input_line(Z80_INPUT_LINE_WAIT, CLEAR_LINE);
	}

	return m_shared_latch;
}

void docastle_state::sub_to_main_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_MAINSUB, "%dw%02X ", offset, data);

	m_shared_latch = data;
	m_maincpu->set_input_line(Z80_INPUT_LINE_WAIT, CLEAR_LINE);
}


void docastle_state::subcpu_nmi_w(uint8_t data)
{
	LOGMASKED(LOG_MAINSUB, "%s trigger subcpu NMI\n", machine().describe_context());

	machine().scheduler().perfect_quantum(attotime::from_usec(100));
	m_subcpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}
