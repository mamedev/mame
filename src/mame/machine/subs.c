/***************************************************************************

    Atari Subs hardware

***************************************************************************/

#include "emu.h"
#include "includes/subs.h"


/***************************************************************************
subs_init_machine
***************************************************************************/
MACHINE_RESET( subs )
{
	subs_state *state = machine.driver_data<subs_state>();
	state->m_steering_buf1 = 0;
	state->m_steering_buf2 = 0;
	state->m_steering_val1 = 0x00;
	state->m_steering_val2 = 0x00;
}

/***************************************************************************
subs_interrupt
***************************************************************************/
INTERRUPT_GEN( subs_interrupt )
{
	/* only do NMI interrupt if not in TEST mode */
	if ((input_port_read(device->machine(), "IN1") & 0x40)==0x40)
		device_set_input_line(device,INPUT_LINE_NMI,PULSE_LINE);
}

/***************************************************************************
Steering

When D7 is high, the steering wheel has moved.
If D6 is high, it moved left.  If D6 is low, it moved right.
Be sure to keep returning a direction until steer_reset is called.
***************************************************************************/
static int subs_steering_1(running_machine &machine)
{
	subs_state *state = machine.driver_data<subs_state>();
	int this_val;
	int delta;

	this_val=input_port_read(machine, "DIAL2");

	delta=this_val-state->m_last_val_1;
	state->m_last_val_1=this_val;
	if (delta>128) delta-=256;
	else if (delta<-128) delta+=256;
	/* Divide by four to make our steering less sensitive */
	state->m_steering_buf1+=(delta/4);

	if (state->m_steering_buf1>0)
	{
	      state->m_steering_buf1--;
	      state->m_steering_val1=0xC0;
	}
	else if (state->m_steering_buf1<0)
	{
	      state->m_steering_buf1++;
	      state->m_steering_val1=0x80;
	}

	return state->m_steering_val1;
}

static int subs_steering_2(running_machine &machine)
{
	subs_state *state = machine.driver_data<subs_state>();
	int this_val;
	int delta;

	this_val=input_port_read(machine, "DIAL1");

	delta=this_val-state->m_last_val_2;
	state->m_last_val_2=this_val;
	if (delta>128) delta-=256;
	else if (delta<-128) delta+=256;
	/* Divide by four to make our steering less sensitive */
	state->m_steering_buf2+=(delta/4);

	if (state->m_steering_buf2>0)
	{
		state->m_steering_buf2--;
		state->m_steering_val2=0xC0;
	}
	else if (state->m_steering_buf2<0)
	{
		state->m_steering_buf2++;
		state->m_steering_val2=0x80;
	}

	return state->m_steering_val2;
}

/***************************************************************************
subs_steer_reset
***************************************************************************/
WRITE8_MEMBER(subs_state::subs_steer_reset_w)
{
	m_steering_val1 = 0x00;
	m_steering_val2 = 0x00;
}

/***************************************************************************
subs_control_r
***************************************************************************/
READ8_MEMBER(subs_state::subs_control_r)
{
	int inport = input_port_read(machine(), "IN0");

	switch (offset & 0x07)
	{
		case 0x00:		return ((inport & 0x01) << 7);	/* diag step */
		case 0x01:		return ((inport & 0x02) << 6);	/* diag hold */
		case 0x02:		return ((inport & 0x04) << 5);	/* slam */
		case 0x03:		return ((inport & 0x08) << 4);	/* spare */
		case 0x04:		return ((subs_steering_1(machine()) & 0x40) << 1);	/* steer dir 1 */
		case 0x05:		return ((subs_steering_1(machine()) & 0x80) << 0);	/* steer flag 1 */
		case 0x06:		return ((subs_steering_2(machine()) & 0x40) << 1);	/* steer dir 2 */
		case 0x07:		return ((subs_steering_2(machine()) & 0x80) << 0);	/* steer flag 2 */
	}

	return 0;
}

/***************************************************************************
subs_coin_r
***************************************************************************/
READ8_MEMBER(subs_state::subs_coin_r)
{
	int inport = input_port_read(machine(), "IN1");

	switch (offset & 0x07)
	{
		case 0x00:		return ((inport & 0x01) << 7);	/* coin 1 */
		case 0x01:		return ((inport & 0x02) << 6);	/* start 1 */
		case 0x02:		return ((inport & 0x04) << 5);	/* coin 2 */
		case 0x03:		return ((inport & 0x08) << 4);	/* start 2 */
		case 0x04:		return ((inport & 0x10) << 3);	/* VBLANK */
		case 0x05:		return ((inport & 0x20) << 2);	/* fire 1 */
		case 0x06:		return ((inport & 0x40) << 1);	/* test */
		case 0x07:		return ((inport & 0x80) << 0);	/* fire 2 */
	}

	return 0;
}

/***************************************************************************
subs_options_r
***************************************************************************/
READ8_MEMBER(subs_state::subs_options_r)
{
	int opts = input_port_read(machine(), "DSW");

	switch (offset & 0x03)
	{
		case 0x00:		return ((opts & 0xC0) >> 6);		/* language */
		case 0x01:		return ((opts & 0x30) >> 4);		/* credits */
		case 0x02:		return ((opts & 0x0C) >> 2);		/* game time */
		case 0x03:		return ((opts & 0x03) >> 0);		/* extended time */
	}

	return 0;
}

/***************************************************************************
subs_lamp1_w
***************************************************************************/
WRITE8_MEMBER(subs_state::subs_lamp1_w)
{
	set_led_status(machine(), 0,~offset & 1);
}

/***************************************************************************
subs_lamp2_w
***************************************************************************/
WRITE8_MEMBER(subs_state::subs_lamp2_w)
{
	set_led_status(machine(), 1,~offset & 1);
}
