/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "emu.h"
#include "includes/matmania.h"


/***************************************************************************

 Mania Challenge 68705 protection interface

 The following is ENTIRELY GUESSWORK!!!

***************************************************************************/

READ8_HANDLER( maniach_68705_port_a_r )
{
	matmania_state *state = space->machine().driver_data<matmania_state>();

	//logerror("%04x: 68705 port A read %02x\n", space->device().safe_pc(), state->m_port_a_in);
	return (state->m_port_a_out & state->m_ddr_a) | (state->m_port_a_in & ~state->m_ddr_a);
}

WRITE8_HANDLER( maniach_68705_port_a_w )
{
	matmania_state *state = space->machine().driver_data<matmania_state>();

	//logerror("%04x: 68705 port A write %02x\n", space->device().safe_pc(), data);
	state->m_port_a_out = data;
}

WRITE8_HANDLER( maniach_68705_ddr_a_w )
{
	matmania_state *state = space->machine().driver_data<matmania_state>();
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

READ8_HANDLER( maniach_68705_port_b_r )
{
	matmania_state *state = space->machine().driver_data<matmania_state>();
	return (state->m_port_b_out & state->m_ddr_b) | (state->m_port_b_in & ~state->m_ddr_b);
}

WRITE8_HANDLER( maniach_68705_port_b_w )
{
	matmania_state *state = space->machine().driver_data<matmania_state>();

	//logerror("%04x: 68705 port B write %02x\n", space->device().safe_pc(), data);

	if (BIT(state->m_ddr_b, 1) && BIT(~data, 1) && BIT(state->m_port_b_out, 1))
	{
		state->m_port_a_in = state->m_from_main;
		state->m_main_sent = 0;
		//logerror("read command %02x from main cpu\n", state->m_port_a_in);
	}
	if (BIT(state->m_ddr_b, 2) && BIT(data, 2) && BIT(~state->m_port_b_out, 2))
	{
		//logerror("send command %02x to main cpu\n", state->m_port_a_out);
		state->m_from_mcu = state->m_port_a_out;
		state->m_mcu_sent = 1;
	}

	state->m_port_b_out = data;
}

WRITE8_HANDLER( maniach_68705_ddr_b_w )
{
	matmania_state *state = space->machine().driver_data<matmania_state>();
	state->m_ddr_b = data;
}


READ8_HANDLER( maniach_68705_port_c_r )
{
	matmania_state *state = space->machine().driver_data<matmania_state>();

	state->m_port_c_in = 0;

	if (state->m_main_sent)
		state->m_port_c_in |= 0x01;

	if (!state->m_mcu_sent)
		state->m_port_c_in |= 0x02;

	//logerror("%04x: 68705 port C read %02x\n",state->m_space->device().safe_pc(), state->m_port_c_in);

	return (state->m_port_c_out & state->m_ddr_c) | (state->m_port_c_in & ~state->m_ddr_c);
}

WRITE8_HANDLER( maniach_68705_port_c_w )
{
	matmania_state *state = space->machine().driver_data<matmania_state>();

	//logerror("%04x: 68705 port C write %02x\n", space->device().safe_pc(), data);
	state->m_port_c_out = data;
}

WRITE8_HANDLER( maniach_68705_ddr_c_w )
{
	matmania_state *state = space->machine().driver_data<matmania_state>();
	state->m_ddr_c = data;
}


WRITE8_HANDLER( maniach_mcu_w )
{
	matmania_state *state = space->machine().driver_data<matmania_state>();

	//logerror("%04x: 3040_w %02x\n", space->device().safe_pc(), data);
	state->m_from_main = data;
	state->m_main_sent = 1;
}

READ8_HANDLER( maniach_mcu_r )
{
	matmania_state *state = space->machine().driver_data<matmania_state>();

	//logerror("%04x: 3040_r %02x\n", space->device().safe_pc(), state->m_from_mcu);
	state->m_mcu_sent = 0;
	return state->m_from_mcu;
}

READ8_HANDLER( maniach_mcu_status_r )
{
	matmania_state *state = space->machine().driver_data<matmania_state>();
	int res = 0;

	/* bit 0 = when 0, mcu has sent data to the main cpu */
	/* bit 1 = when 1, mcu is ready to receive data from main cpu */
	//logerror("%04x: 3041_r\n", space->device().safe_pc());
	if (!state->m_mcu_sent)
		res |= 0x01;
	if (!state->m_main_sent)
		res |= 0x02;

	return res;
}
