// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/***************************************************************************

    Atari Subs hardware

***************************************************************************/

#include "emu.h"
#include "includes/subs.h"


/***************************************************************************
machine initialization
***************************************************************************/

void subs_state::machine_start()
{
	save_item(NAME(m_steering_buf1));
	save_item(NAME(m_steering_buf2));
	save_item(NAME(m_steering_val1));
	save_item(NAME(m_steering_val2));
	save_item(NAME(m_last_val_1));
	save_item(NAME(m_last_val_2));
}

void subs_state::machine_reset()
{
	m_steering_buf1 = 0;
	m_steering_buf2 = 0;
	m_steering_val1 = 0x00;
	m_steering_val2 = 0x00;
}

/***************************************************************************
interrupt
***************************************************************************/
INTERRUPT_GEN_MEMBER(subs_state::interrupt)
{
	/* only do NMI interrupt if not in TEST mode */
	if ((ioport("IN1")->read() & 0x40)==0x40)
		device.execute().set_input_line(INPUT_LINE_NMI,PULSE_LINE);
}

/***************************************************************************
Steering

When D7 is high, the steering wheel has moved.
If D6 is high, it moved left.  If D6 is low, it moved right.
Be sure to keep returning a direction until steer_reset is called.
***************************************************************************/
int subs_state::steering_1()
{
	int this_val;
	int delta;

	this_val=ioport("DIAL2")->read();

	delta=this_val-m_last_val_1;
	m_last_val_1=this_val;
	if (delta>128) delta-=256;
	else if (delta<-128) delta+=256;
	/* Divide by four to make our steering less sensitive */
	m_steering_buf1+=(delta/4);

	if (m_steering_buf1>0)
	{
			m_steering_buf1--;
			m_steering_val1=0xC0;
	}
	else if (m_steering_buf1<0)
	{
			m_steering_buf1++;
			m_steering_val1=0x80;
	}

	return m_steering_val1;
}

int subs_state::steering_2()
{
	int this_val;
	int delta;

	this_val=ioport("DIAL1")->read();

	delta=this_val-m_last_val_2;
	m_last_val_2=this_val;
	if (delta>128) delta-=256;
	else if (delta<-128) delta+=256;
	/* Divide by four to make our steering less sensitive */
	m_steering_buf2+=(delta/4);

	if (m_steering_buf2>0)
	{
		m_steering_buf2--;
		m_steering_val2=0xC0;
	}
	else if (m_steering_buf2<0)
	{
		m_steering_buf2++;
		m_steering_val2=0x80;
	}

	return m_steering_val2;
}

/***************************************************************************
steer_reset
***************************************************************************/
WRITE8_MEMBER(subs_state::steer_reset_w)
{
	m_steering_val1 = 0x00;
	m_steering_val2 = 0x00;
}

/***************************************************************************
control_r
***************************************************************************/
READ8_MEMBER(subs_state::control_r)
{
	int inport = ioport("IN0")->read();

	switch (offset & 0x07)
	{
		case 0x00:      return ((inport & 0x01) << 7);  /* diag step */
		case 0x01:      return ((inport & 0x02) << 6);  /* diag hold */
		case 0x02:      return ((inport & 0x04) << 5);  /* slam */
		case 0x03:      return ((inport & 0x08) << 4);  /* spare */
		case 0x04:      return ((steering_1() & 0x40) << 1);  /* steer dir 1 */
		case 0x05:      return ((steering_1() & 0x80) << 0);  /* steer flag 1 */
		case 0x06:      return ((steering_2() & 0x40) << 1);  /* steer dir 2 */
		case 0x07:      return ((steering_2() & 0x80) << 0);  /* steer flag 2 */
	}

	return 0;
}

/***************************************************************************
coin_r
***************************************************************************/
READ8_MEMBER(subs_state::coin_r)
{
	int inport = ioport("IN1")->read();

	switch (offset & 0x07)
	{
		case 0x00:      return ((inport & 0x01) << 7);  /* coin 1 */
		case 0x01:      return ((inport & 0x02) << 6);  /* start 1 */
		case 0x02:      return ((inport & 0x04) << 5);  /* coin 2 */
		case 0x03:      return ((inport & 0x08) << 4);  /* start 2 */
		case 0x04:      return ((inport & 0x10) << 3);  /* VBLANK */
		case 0x05:      return ((inport & 0x20) << 2);  /* fire 1 */
		case 0x06:      return ((inport & 0x40) << 1);  /* test */
		case 0x07:      return ((inport & 0x80) << 0);  /* fire 2 */
	}

	return 0;
}

/***************************************************************************
options_r
***************************************************************************/
READ8_MEMBER(subs_state::options_r)
{
	int opts = ioport("DSW")->read();

	switch (offset & 0x03)
	{
		case 0x00:      return ((opts & 0xC0) >> 6);        /* language */
		case 0x01:      return ((opts & 0x30) >> 4);        /* credits */
		case 0x02:      return ((opts & 0x0C) >> 2);        /* game time */
		case 0x03:      return ((opts & 0x03) >> 0);        /* extended time */
	}

	return 0;
}

/***************************************************************************
lamp1_w
***************************************************************************/
WRITE8_MEMBER(subs_state::lamp1_w)
{
	output().set_led_value(0,~offset & 1);
}

/***************************************************************************
lamp2_w
***************************************************************************/
WRITE8_MEMBER(subs_state::lamp2_w)
{
	output().set_led_value(1,~offset & 1);
}
