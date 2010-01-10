#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/buggychl.h"


/***************************************************************************

 Buggy CHallenge 68705 protection interface

 This is accurate. FairyLand Story seems to be identical.

***************************************************************************/

READ8_HANDLER( buggychl_68705_port_a_r )
{
	buggychl_state *state = (buggychl_state *)space->machine->driver_data;
	//logerror("%04x: 68705 port A read %02x\n", cpu_get_pc(space->cpu), state->port_a_in);
	return (state->port_a_out & state->ddr_a) | (state->port_a_in & ~state->ddr_a);
}

WRITE8_HANDLER( buggychl_68705_port_a_w )
{
	buggychl_state *state = (buggychl_state *)space->machine->driver_data;
	//logerror("%04x: 68705 port A write %02x\n", cpu_get_pc(space->cpu), data);
	state->port_a_out = data;
}

WRITE8_HANDLER( buggychl_68705_ddr_a_w )
{
	buggychl_state *state = (buggychl_state *)space->machine->driver_data;
	state->ddr_a = data;
}



/*
 *  Port B connections:
 *  parts in [ ] are optional (not used by buggychl)
 *
 *  all bits are logical 1 when read (+5V pullup)
 *
 *  0   n.c.
 *  1   W  IRQ ack and enable latch which holds data from main Z80 memory
 *  2   W  loads latch to Z80
 *  3   W  to Z80 BUSRQ (put it on hold?)
 *  4   W  n.c.
 *  5   W  [selects Z80 memory access direction (0 = write 1 = read)]
 *  6   W  [loads the latch which holds the low 8 bits of the address of
 *               the main Z80 memory location to access]
 *  7   W  [loads the latch which holds the high 8 bits of the address of
 *               the main Z80 memory location to access]
 */


READ8_HANDLER( buggychl_68705_port_b_r )
{
	buggychl_state *state = (buggychl_state *)space->machine->driver_data;
	return (state->port_b_out & state->ddr_b) | (state->port_b_in & ~state->ddr_b);
}

WRITE8_HANDLER( buggychl_68705_port_b_w )
{
	buggychl_state *state = (buggychl_state *)space->machine->driver_data;
	logerror("%04x: 68705 port B write %02x\n", cpu_get_pc(space->cpu), data);

	if ((state->ddr_b & 0x02) && (~data & 0x02) && (state->port_b_out & 0x02))
	{
		state->port_a_in = state->from_main;
		if (state->main_sent)
			cpu_set_input_line(state->mcu, 0, CLEAR_LINE);
		state->main_sent = 0;
		logerror("read command %02x from main cpu\n", state->port_a_in);
	}
	if ((state->ddr_b & 0x04) && (data & 0x04) && (~state->port_b_out & 0x04))
	{
		logerror("send command %02x to main cpu\n", state->port_a_out);
		state->from_mcu = state->port_a_out;
		state->mcu_sent = 1;
	}

	state->port_b_out = data;
}

WRITE8_HANDLER( buggychl_68705_ddr_b_w )
{
	buggychl_state *state = (buggychl_state *)space->machine->driver_data;
	state->ddr_b = data;
}


/*
 *  Port C connections:
 *
 *  all bits are logical 1 when read (+5V pullup)
 *
 *  0   R  1 when pending command Z80->68705
 *  1   R  0 when pending command 68705->Z80
 */

READ8_HANDLER( buggychl_68705_port_c_r )
{
	buggychl_state *state = (buggychl_state *)space->machine->driver_data;
	state->port_c_in = 0;
	if (state->main_sent)
		state->port_c_in |= 0x01;
	if (!state->mcu_sent)
		state->port_c_in |= 0x02;
	logerror("%04x: 68705 port C read %02x\n", cpu_get_pc(space->cpu), state->port_c_in);
	return (state->port_c_out & state->ddr_c) | (state->port_c_in & ~state->ddr_c);
}

WRITE8_HANDLER( buggychl_68705_port_c_w )
{
	buggychl_state *state = (buggychl_state *)space->machine->driver_data;
	logerror("%04x: 68705 port C write %02x\n", cpu_get_pc(space->cpu), data);
	state->port_c_out = data;
}

WRITE8_HANDLER( buggychl_68705_ddr_c_w )
{
	buggychl_state *state = (buggychl_state *)space->machine->driver_data;
	state->ddr_c = data;
}


WRITE8_HANDLER( buggychl_mcu_w )
{
	buggychl_state *state = (buggychl_state *)space->machine->driver_data;
	logerror("%04x: mcu_w %02x\n", cpu_get_pc(space->cpu), data);
	state->from_main = data;
	state->main_sent = 1;
	cpu_set_input_line(state->mcu, 0, ASSERT_LINE);
}

READ8_HANDLER( buggychl_mcu_r )
{
	buggychl_state *state = (buggychl_state *)space->machine->driver_data;
	logerror("%04x: mcu_r %02x\n", cpu_get_pc(space->cpu), state->from_mcu);
	state->mcu_sent = 0;
	return state->from_mcu;
}

READ8_HANDLER( buggychl_mcu_status_r )
{
	buggychl_state *state = (buggychl_state *)space->machine->driver_data;
	int res = 0;

	/* bit 0 = when 1, mcu is ready to receive data from main cpu */
	/* bit 1 = when 1, mcu has sent data to the main cpu */
	//logerror("%04x: mcu_status_r\n",cpu_get_pc(space->cpu));
	if (!state->main_sent)
		res |= 0x01;
	if (state->mcu_sent)
		res |= 0x02;

	return res;
}
