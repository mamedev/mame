#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/retofinv.h"


/***************************************************************************

 Return of Invaders 68705 protection interface

***************************************************************************/


READ8_HANDLER( retofinv_68705_portA_r )
{
	retofinv_state *state = space->machine->driver_data<retofinv_state>();
//logerror("%04x: 68705 port A read %02x\n",cpu_get_pc(space->cpu),state->portA_in);
	return (state->portA_out & state->ddrA) | (state->portA_in & ~state->ddrA);
}

WRITE8_HANDLER( retofinv_68705_portA_w )
{
	retofinv_state *state = space->machine->driver_data<retofinv_state>();
//logerror("%04x: 68705 port A write %02x\n",cpu_get_pc(space->cpu),data);
	state->portA_out = data;
}

WRITE8_HANDLER( retofinv_68705_ddrA_w )
{
	retofinv_state *state = space->machine->driver_data<retofinv_state>();
	state->ddrA = data;
}



/*
 *  Port B connections:
 *
 *  all bits are logical 1 when read (+5V pullup)
 *
 *  1   W  IRQ ack and enable latch which holds data from main Z80 memory
 *  2   W  loads latch to Z80
 */


READ8_HANDLER( retofinv_68705_portB_r )
{
	retofinv_state *state = space->machine->driver_data<retofinv_state>();
	return (state->portB_out & state->ddrB) | (state->portB_in & ~state->ddrB);
}

WRITE8_HANDLER( retofinv_68705_portB_w )
{
	retofinv_state *state = space->machine->driver_data<retofinv_state>();
//logerror("%04x: 68705 port B write %02x\n",cpu_get_pc(space->cpu),data);

	if ((state->ddrB & 0x02) && (~data & 0x02) && (state->portB_out & 0x02))
	{
		state->portA_in = state->from_main;
		if (state->main_sent) cputag_set_input_line(space->machine, "68705", 0, CLEAR_LINE);
		state->main_sent = 0;
//logerror("read command %02x from main cpu\n",state->portA_in);
	}
	if ((state->ddrB & 0x04) && (data & 0x04) && (~state->portB_out & 0x04))
	{
//logerror("send command %02x to main cpu\n",state->portA_out);
		state->from_mcu = state->portA_out;
		state->mcu_sent = 1;
	}

	state->portB_out = data;
}

WRITE8_HANDLER( retofinv_68705_ddrB_w )
{
	retofinv_state *state = space->machine->driver_data<retofinv_state>();
	state->ddrB = data;
}


/*
 *  Port C connections:
 *
 *  all bits are logical 1 when read (+5V pullup)
 *
 *  0   R  1 when pending command Z80->68705
 *  1   R  0 when pending command 68705->Z80
 */


READ8_HANDLER( retofinv_68705_portC_r )
{
	retofinv_state *state = space->machine->driver_data<retofinv_state>();
	state->portC_in = 0;
	if (state->main_sent) state->portC_in |= 0x01;
	if (!state->mcu_sent) state->portC_in |= 0x02;
//logerror("%04x: 68705 port C read %02x\n",cpu_get_pc(space->cpu),state->portC_in);
	return (state->portC_out & state->ddrC) | (state->portC_in & ~state->ddrC);
}

WRITE8_HANDLER( retofinv_68705_portC_w )
{
	retofinv_state *state = space->machine->driver_data<retofinv_state>();
logerror("%04x: 68705 port C write %02x\n",cpu_get_pc(space->cpu),data);
	state->portC_out = data;
}

WRITE8_HANDLER( retofinv_68705_ddrC_w )
{
	retofinv_state *state = space->machine->driver_data<retofinv_state>();
	state->ddrC = data;
}


WRITE8_HANDLER( retofinv_mcu_w )
{
	retofinv_state *state = space->machine->driver_data<retofinv_state>();
logerror("%04x: mcu_w %02x\n",cpu_get_pc(space->cpu),data);
	state->from_main = data;
	state->main_sent = 1;
	cputag_set_input_line(space->machine, "68705", 0, ASSERT_LINE);
}

READ8_HANDLER( retofinv_mcu_r )
{
	retofinv_state *state = space->machine->driver_data<retofinv_state>();
logerror("%04x: mcu_r %02x\n",cpu_get_pc(space->cpu),state->from_mcu);
	state->mcu_sent = 0;
	return state->from_mcu;
}

READ8_HANDLER( retofinv_mcu_status_r )
{
	retofinv_state *state = space->machine->driver_data<retofinv_state>();
	int res = 0;

	/* bit 4 = when 1, mcu is ready to receive data from main cpu */
	/* bit 5 = when 1, mcu has sent data to the main cpu */
//logerror("%04x: mcu_status_r\n",cpu_get_pc(space->cpu));
	if (!state->main_sent) res |= 0x10;
	if (state->mcu_sent) res |= 0x20;

	return res;
}
