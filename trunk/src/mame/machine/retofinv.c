#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/retofinv.h"


/***************************************************************************

 Return of Invaders 68705 protection interface

***************************************************************************/


READ8_HANDLER( retofinv_68705_portA_r )
{
	retofinv_state *state = space->machine().driver_data<retofinv_state>();
//logerror("%04x: 68705 port A read %02x\n",cpu_get_pc(&space->device()),state->m_portA_in);
	return (state->m_portA_out & state->m_ddrA) | (state->m_portA_in & ~state->m_ddrA);
}

WRITE8_HANDLER( retofinv_68705_portA_w )
{
	retofinv_state *state = space->machine().driver_data<retofinv_state>();
//logerror("%04x: 68705 port A write %02x\n",cpu_get_pc(&space->device()),data);
	state->m_portA_out = data;
}

WRITE8_HANDLER( retofinv_68705_ddrA_w )
{
	retofinv_state *state = space->machine().driver_data<retofinv_state>();
	state->m_ddrA = data;
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
	retofinv_state *state = space->machine().driver_data<retofinv_state>();
	return (state->m_portB_out & state->m_ddrB) | (state->m_portB_in & ~state->m_ddrB);
}

WRITE8_HANDLER( retofinv_68705_portB_w )
{
	retofinv_state *state = space->machine().driver_data<retofinv_state>();
//logerror("%04x: 68705 port B write %02x\n",cpu_get_pc(&space->device()),data);

	if ((state->m_ddrB & 0x02) && (~data & 0x02) && (state->m_portB_out & 0x02))
	{
		state->m_portA_in = state->m_from_main;
		if (state->m_main_sent) cputag_set_input_line(space->machine(), "68705", 0, CLEAR_LINE);
		state->m_main_sent = 0;
//logerror("read command %02x from main cpu\n",state->m_portA_in);
	}
	if ((state->m_ddrB & 0x04) && (data & 0x04) && (~state->m_portB_out & 0x04))
	{
//logerror("send command %02x to main cpu\n",state->m_portA_out);
		state->m_from_mcu = state->m_portA_out;
		state->m_mcu_sent = 1;
	}

	state->m_portB_out = data;
}

WRITE8_HANDLER( retofinv_68705_ddrB_w )
{
	retofinv_state *state = space->machine().driver_data<retofinv_state>();
	state->m_ddrB = data;
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
	retofinv_state *state = space->machine().driver_data<retofinv_state>();
	state->m_portC_in = 0;
	if (state->m_main_sent) state->m_portC_in |= 0x01;
	if (!state->m_mcu_sent) state->m_portC_in |= 0x02;
//logerror("%04x: 68705 port C read %02x\n",cpu_get_pc(&space->device()),state->m_portC_in);
	return (state->m_portC_out & state->m_ddrC) | (state->m_portC_in & ~state->m_ddrC);
}

WRITE8_HANDLER( retofinv_68705_portC_w )
{
	retofinv_state *state = space->machine().driver_data<retofinv_state>();
logerror("%04x: 68705 port C write %02x\n",cpu_get_pc(&space->device()),data);
	state->m_portC_out = data;
}

WRITE8_HANDLER( retofinv_68705_ddrC_w )
{
	retofinv_state *state = space->machine().driver_data<retofinv_state>();
	state->m_ddrC = data;
}


WRITE8_HANDLER( retofinv_mcu_w )
{
	retofinv_state *state = space->machine().driver_data<retofinv_state>();
logerror("%04x: mcu_w %02x\n",cpu_get_pc(&space->device()),data);
	state->m_from_main = data;
	state->m_main_sent = 1;
	cputag_set_input_line(space->machine(), "68705", 0, ASSERT_LINE);
}

READ8_HANDLER( retofinv_mcu_r )
{
	retofinv_state *state = space->machine().driver_data<retofinv_state>();
logerror("%04x: mcu_r %02x\n",cpu_get_pc(&space->device()),state->m_from_mcu);
	state->m_mcu_sent = 0;
	return state->m_from_mcu;
}

READ8_HANDLER( retofinv_mcu_status_r )
{
	retofinv_state *state = space->machine().driver_data<retofinv_state>();
	int res = 0;

	/* bit 4 = when 1, mcu is ready to receive data from main cpu */
	/* bit 5 = when 1, mcu has sent data to the main cpu */
//logerror("%04x: mcu_status_r\n",cpu_get_pc(&space->device()));
	if (!state->m_main_sent) res |= 0x10;
	if (state->m_mcu_sent) res |= 0x20;

	return res;
}
