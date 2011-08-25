/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "emu.h"
#include "includes/bigevglf.h"


READ8_HANDLER( bigevglf_68705_port_a_r )
{
	bigevglf_state *state = space->machine().driver_data<bigevglf_state>();
	return (state->m_port_a_out & state->m_ddr_a) | (state->m_port_a_in & ~state->m_ddr_a);
}

WRITE8_HANDLER( bigevglf_68705_port_a_w )
{
	bigevglf_state *state = space->machine().driver_data<bigevglf_state>();
	state->m_port_a_out = data;
}

WRITE8_HANDLER( bigevglf_68705_ddr_a_w )
{
	bigevglf_state *state = space->machine().driver_data<bigevglf_state>();
	state->m_ddr_a = data;
}

READ8_HANDLER( bigevglf_68705_port_b_r )
{
	bigevglf_state *state = space->machine().driver_data<bigevglf_state>();
	return (state->m_port_b_out & state->m_ddr_b) | (state->m_port_b_in & ~state->m_ddr_b);
}

WRITE8_HANDLER( bigevglf_68705_port_b_w )
{
	bigevglf_state *state = space->machine().driver_data<bigevglf_state>();

	if ((state->m_ddr_b & 0x02) && (~state->m_port_b_out & 0x02) && (data & 0x02)) /* positive going transition of the clock */
	{
		device_set_input_line(state->m_mcu, 0, CLEAR_LINE);
		state->m_main_sent = 0;

	}
	if ((state->m_ddr_b & 0x04) && (~state->m_port_b_out & 0x04) && (data & 0x04) ) /* positive going transition of the clock */
	{
		state->m_from_mcu = state->m_port_a_out;
		state->m_mcu_sent = 0;
	}

	state->m_port_b_out = data;
}

WRITE8_HANDLER( bigevglf_68705_ddr_b_w )
{
	bigevglf_state *state = space->machine().driver_data<bigevglf_state>();
	state->m_ddr_b = data;
}

READ8_HANDLER( bigevglf_68705_port_c_r )
{
	bigevglf_state *state = space->machine().driver_data<bigevglf_state>();

	state->m_port_c_in = 0;
	if (state->m_main_sent)
		state->m_port_c_in |= 0x01;
	if (state->m_mcu_sent)
		state->m_port_c_in |= 0x02;

	return (state->m_port_c_out & state->m_ddr_c) | (state->m_port_c_in & ~state->m_ddr_c);
}

WRITE8_HANDLER( bigevglf_68705_port_c_w )
{
	bigevglf_state *state = space->machine().driver_data<bigevglf_state>();
	state->m_port_c_out = data;
}

WRITE8_HANDLER( bigevglf_68705_ddr_c_w )
{
	bigevglf_state *state = space->machine().driver_data<bigevglf_state>();
	state->m_ddr_c = data;
}

WRITE8_HANDLER( bigevglf_mcu_w )
{
	bigevglf_state *state = space->machine().driver_data<bigevglf_state>();

	state->m_port_a_in = data;
	state->m_main_sent = 1;
	device_set_input_line(state->m_mcu, 0, ASSERT_LINE);
}


READ8_HANDLER( bigevglf_mcu_r )
{
	bigevglf_state *state = space->machine().driver_data<bigevglf_state>();

	state->m_mcu_sent = 1;
	return state->m_from_mcu;
}

READ8_HANDLER( bigevglf_mcu_status_r )
{
	bigevglf_state *state = space->machine().driver_data<bigevglf_state>();
	int res = 0;

	if (!state->m_main_sent)
		res |= 0x08;
	if (!state->m_mcu_sent)
		res |= 0x10;

	return res;
}

