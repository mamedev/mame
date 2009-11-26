/***************************************************************************

    Atari Basketball hardware

***************************************************************************/

#include "driver.h"
#include "bsktball.h"

/***************************************************************************
    bsktball_nmion_w
***************************************************************************/
WRITE8_HANDLER( bsktball_nmion_w )
{
	bsktball_state *state = (bsktball_state *)space->machine->driver_data;
	state->nmi_on = offset & 0x01;
}

/***************************************************************************
    bsktball_interrupt
***************************************************************************/
/* NMI every 32V, IRQ every VBLANK */
INTERRUPT_GEN( bsktball_interrupt )
{
	bsktball_state *state = (bsktball_state *)device->machine->driver_data;

	/* We mod by 8 because we're interrupting 8x per frame, 1 per 32V */
	state->i256v = (state->i256v + 1) % 8;

	if (state->i256v == 0)
		cpu_set_input_line(device, 0, HOLD_LINE);
	else if (state->nmi_on)
		cpu_set_input_line(device, INPUT_LINE_NMI, PULSE_LINE);
}

/***************************************************************************
    bsktball_ld_w
***************************************************************************/

WRITE8_HANDLER( bsktball_ld1_w )
{
	bsktball_state *state = (bsktball_state *)space->machine->driver_data;
	state->ld1 = (offset & 0x01);
}

WRITE8_HANDLER( bsktball_ld2_w )
{
	bsktball_state *state = (bsktball_state *)space->machine->driver_data;
	state->ld2 = (offset & 0x01);
}


/***************************************************************************
    bsktball_in0_r
***************************************************************************/

READ8_HANDLER( bsktball_in0_r )
{
	bsktball_state *state = (bsktball_state *)space->machine->driver_data;
	int p1_horiz;
	int p1_vert;
	int p2_horiz;
	int p2_vert;
	int temp;

	p1_horiz = input_port_read(space->machine, "TRACK0_X");
	p1_vert  = input_port_read(space->machine, "TRACK0_Y");
	p2_horiz = input_port_read(space->machine, "TRACK1_X");
	p2_vert  = input_port_read(space->machine, "TRACK1_Y");

	/* Set direction bits */

	/* P1 H DIR */
	if (p1_horiz > state->last_p1_horiz)
	{
		if ((p1_horiz - state->last_p1_horiz) > 128)
			state->dir2 = 0x40;
		else
			state->dir2 = 0;
	}
	else if (p1_horiz < state->last_p1_horiz)
	{
		if ((state->last_p1_horiz - p1_horiz) > 128)
			state->dir2 = 0;
		else
			state->dir2 = 0x40;
	}

	/* P1 V DIR */
	if (p1_vert > state->last_p1_vert)
	{
		if ((p1_vert - state->last_p1_vert) > 128)
			state->dir3 = 0;
		else
			state->dir3 = 0x80;
	}
	else if (p1_vert < state->last_p1_vert)
	{
		if ((state->last_p1_vert - p1_vert) > 128)
			state->dir3 = 0x80;
		else
			state->dir3 = 0;
	}

	/* P2 H DIR */
	if (p2_horiz > state->last_p2_horiz)
	{
		if ((p2_horiz - state->last_p2_horiz) > 128)
			state->dir0 = 0x10;
		else
			state->dir0 = 0;
	}
	else if (p2_horiz < state->last_p2_horiz)
	{
		if ((state->last_p2_horiz - p2_horiz) > 128)
			state->dir0 = 0;
		else
			state->dir0 = 0x10;
	}

	/* P2 V DIR */
	if (p2_vert > state->last_p2_vert)
	{
		if ((p2_vert - state->last_p2_vert) > 128)
			state->dir1 = 0;
		else
			state->dir1 = 0x20;
	}
	else if (p2_vert < state->last_p2_vert)
	{
		if ((state->last_p2_vert - p2_vert) > 128)
			state->dir1 = 0x20;
		else
			state->dir1 = 0;
	}

	state->last_p1_horiz = p1_horiz;
	state->last_p1_vert  = p1_vert;
	state->last_p2_horiz = p2_horiz;
	state->last_p2_vert  = p2_vert;

	/* D0-D3 = Plyr 1 Horiz, D4-D7 = Plyr 1 Vert */
	if ((state->ld1) & (state->ld2))
	{
		return ((p1_horiz & 0x0f) | ((p1_vert << 4) & 0xf0));
	}
	/* D0-D3 = Plyr 2 Horiz, D4-D7 = Plyr 2 Vert */
	else if (state->ld2)
	{
		return ((p2_horiz & 0x0f) | ((p2_vert << 4) & 0xf0));
	}
	else
	{
		temp = input_port_read(space->machine, "IN0") & 0x0f;

		return (temp | state->dir0 | state->dir1 | state->dir2 | state->dir3);
	}
}

/***************************************************************************
    bsktball_led_w
***************************************************************************/
WRITE8_HANDLER( bsktball_led1_w )
{
	set_led_status(space->machine, 0, offset & 0x01);
}

WRITE8_HANDLER( bsktball_led2_w )
{
	set_led_status(space->machine, 1, offset & 0x01);
}
