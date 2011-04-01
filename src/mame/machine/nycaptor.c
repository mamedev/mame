/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "emu.h"
#include "includes/nycaptor.h"

READ8_HANDLER( nycaptor_68705_port_a_r )
{
	nycaptor_state *state = space->machine().driver_data<nycaptor_state>();
	return (state->m_port_a_out & state->m_ddr_a) | (state->m_port_a_in & ~state->m_ddr_a);
}

WRITE8_HANDLER( nycaptor_68705_port_a_w )
{
	nycaptor_state *state = space->machine().driver_data<nycaptor_state>();
	state->m_port_a_out = data;
}

WRITE8_HANDLER( nycaptor_68705_ddr_a_w )
{
	nycaptor_state *state = space->machine().driver_data<nycaptor_state>();
	state->m_ddr_a = data;
}

/*
 *  Port B connections:
 *
 *  all bits are logical 1 when read (+5V pullup)
 *
 *  1   W  when 1->0, enables latch which brings the command from main CPU (read from port A)
 *  2   W  when 0->1, copies port A to the latch for the main CPU
 */

READ8_HANDLER( nycaptor_68705_port_b_r )
{
	nycaptor_state *state = space->machine().driver_data<nycaptor_state>();
	return (state->m_port_b_out & state->m_ddr_b) | (state->m_port_b_in & ~state->m_ddr_b);
}

WRITE8_HANDLER( nycaptor_68705_port_b_w )
{
	nycaptor_state *state = space->machine().driver_data<nycaptor_state>();

	if (BIT(state->m_ddr_b, 1) && BIT(~data, 1) && BIT(state->m_port_b_out, 1))
	{
		state->m_port_a_in = state->m_from_main;

		if (state->m_main_sent)
			device_set_input_line(state->m_mcu, 0, CLEAR_LINE);
		state->m_main_sent = 0;

	}

	if (BIT(state->m_ddr_b, 2) && BIT(data, 2) && BIT(~state->m_port_b_out, 2))
	{
		state->m_from_mcu = state->m_port_a_out;
		state->m_mcu_sent = 1;
	}

	state->m_port_b_out = data;
}

WRITE8_HANDLER( nycaptor_68705_ddr_b_w )
{
	nycaptor_state *state = space->machine().driver_data<nycaptor_state>();
	state->m_ddr_b = data;
}


READ8_HANDLER( nycaptor_68705_port_c_r )
{
	nycaptor_state *state = space->machine().driver_data<nycaptor_state>();
	state->m_port_c_in = 0;

	if (state->m_main_sent)
		state->m_port_c_in |= 0x01;
	if (!state->m_mcu_sent)
		state->m_port_c_in |= 0x02;

	return (state->m_port_c_out & state->m_ddr_c) | (state->m_port_c_in & ~state->m_ddr_c);
}

WRITE8_HANDLER( nycaptor_68705_port_c_w )
{
	nycaptor_state *state = space->machine().driver_data<nycaptor_state>();
	state->m_port_c_out = data;
}

WRITE8_HANDLER( nycaptor_68705_ddr_c_w )
{
	nycaptor_state *state = space->machine().driver_data<nycaptor_state>();
	state->m_ddr_c = data;
}

WRITE8_HANDLER( nycaptor_mcu_w )
{
	nycaptor_state *state = space->machine().driver_data<nycaptor_state>();

	state->m_from_main = data;
	state->m_main_sent = 1;
	device_set_input_line(state->m_mcu, 0, ASSERT_LINE);
}

READ8_HANDLER( nycaptor_mcu_r )
{
	nycaptor_state *state = space->machine().driver_data<nycaptor_state>();

	state->m_mcu_sent = 0;
	return state->m_from_mcu;
}

READ8_HANDLER( nycaptor_mcu_status_r1 )
{
	nycaptor_state *state = space->machine().driver_data<nycaptor_state>();

	/* bit 1 = when 1, mcu has sent data to the main cpu */
	return state->m_mcu_sent ? 2 : 0;
}

READ8_HANDLER( nycaptor_mcu_status_r2 )
{
	nycaptor_state *state = space->machine().driver_data<nycaptor_state>();

	/* bit 0 = when 1, mcu is ready to receive data from main cpu */
	return state->m_main_sent ? 0 : 1;
}
