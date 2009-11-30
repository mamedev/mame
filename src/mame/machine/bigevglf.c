/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "driver.h"
#include "includes/bigevglf.h"


READ8_HANDLER( bigevglf_68705_port_a_r )
{
	bigevglf_state *state = (bigevglf_state *)space->machine->driver_data;
	return (state->port_a_out & state->ddr_a) | (state->port_a_in & ~state->ddr_a);
}

WRITE8_HANDLER( bigevglf_68705_port_a_w )
{
	bigevglf_state *state = (bigevglf_state *)space->machine->driver_data;
	state->port_a_out = data;
}

WRITE8_HANDLER( bigevglf_68705_ddr_a_w )
{
	bigevglf_state *state = (bigevglf_state *)space->machine->driver_data;
	state->ddr_a = data;
}

READ8_HANDLER( bigevglf_68705_port_b_r )
{
	bigevglf_state *state = (bigevglf_state *)space->machine->driver_data;
	return (state->port_b_out & state->ddr_b) | (state->port_b_in & ~state->ddr_b);
}

WRITE8_HANDLER( bigevglf_68705_port_b_w )
{
	bigevglf_state *state = (bigevglf_state *)space->machine->driver_data;

	if ((state->ddr_b & 0x02) && (~state->port_b_out & 0x02) && (data & 0x02)) /* positive going transition of the clock */
	{
		cpu_set_input_line(state->mcu, 0, CLEAR_LINE);
		state->main_sent = 0;

	}
	if ((state->ddr_b & 0x04) && (~state->port_b_out & 0x04) && (data & 0x04) ) /* positive going transition of the clock */
	{
		state->from_mcu = state->port_a_out;
		state->mcu_sent = 0;
	}

	state->port_b_out = data;
}

WRITE8_HANDLER( bigevglf_68705_ddr_b_w )
{
	bigevglf_state *state = (bigevglf_state *)space->machine->driver_data;
	state->ddr_b = data;
}

READ8_HANDLER( bigevglf_68705_port_c_r )
{
	bigevglf_state *state = (bigevglf_state *)space->machine->driver_data;

	state->port_c_in = 0;
	if (state->main_sent)
		state->port_c_in |= 0x01;
	if (state->mcu_sent)
		state->port_c_in |= 0x02;

	return (state->port_c_out & state->ddr_c) | (state->port_c_in & ~state->ddr_c);
}

WRITE8_HANDLER( bigevglf_68705_port_c_w )
{
	bigevglf_state *state = (bigevglf_state *)space->machine->driver_data;
	state->port_c_out = data;
}

WRITE8_HANDLER( bigevglf_68705_ddr_c_w )
{
	bigevglf_state *state = (bigevglf_state *)space->machine->driver_data;
	state->ddr_c = data;
}

WRITE8_HANDLER( bigevglf_mcu_w )
{
	bigevglf_state *state = (bigevglf_state *)space->machine->driver_data;

	state->port_a_in = data;
	state->main_sent = 1;
	cpu_set_input_line(state->mcu, 0, ASSERT_LINE);
}


READ8_HANDLER( bigevglf_mcu_r )
{
	bigevglf_state *state = (bigevglf_state *)space->machine->driver_data;

	state->mcu_sent = 1;
	return state->from_mcu;
}

READ8_HANDLER( bigevglf_mcu_status_r )
{
	bigevglf_state *state = (bigevglf_state *)space->machine->driver_data;
	int res = 0;

	if (!state->main_sent)
		res |= 0x08;
	if (!state->mcu_sent)
		res |= 0x10;

	return res;
}

