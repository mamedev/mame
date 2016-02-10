// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/***************************************************************************

    Atari Night Driver hardware

***************************************************************************/

#include "emu.h"
#include "includes/nitedrvr.h"
#include "sound/discrete.h"

/***************************************************************************
Steering

When D7 is high, the steering wheel has moved.
If D6 is low, it moved left.  If D6 is high, it moved right.
Be sure to keep returning a direction until steering_reset is called,
because D6 and D7 are apparently checked at different times, and a
change in-between can affect the direction you move.
***************************************************************************/

int nitedrvr_state::nitedrvr_steering(  )
{
	int this_val = ioport("STEER")->read();
	int delta = this_val - m_last_steering_val;

	m_last_steering_val = this_val;

	if (delta > 128)
		delta -= 256;
	else if (delta < -128)
		delta += 256;

	/* Divide by four to make our steering less sensitive */
	m_steering_buf += (delta / 4);

	if (m_steering_buf > 0)
	{
		m_steering_buf--;
		m_steering_val = 0xc0;
	}
	else if (m_steering_buf < 0)
	{
		m_steering_buf++;
		m_steering_val = 0x80;
	}
	else
	{
		m_steering_val = 0x00;
	}

	return m_steering_val;
}

/***************************************************************************
nitedrvr_steering_reset
***************************************************************************/

READ8_MEMBER(nitedrvr_state::nitedrvr_steering_reset_r)
{
	m_steering_val = 0;
	return 0;
}

WRITE8_MEMBER(nitedrvr_state::nitedrvr_steering_reset_w)
{
	m_steering_val = 0;
}


/***************************************************************************
nitedrvr_in0_r

Night Driver looks for the following:
    A: $00
        D4 - OPT1
        D5 - OPT2
        D6 - OPT3
        D7 - OPT4
    A: $01
        D4 - TRACK SET
        D5 - BONUS TIME ALLOWED
        D6 - VBLANK
        D7 - !TEST
    A: $02
        D4 - !GEAR 1
        D5 - !GEAR 2
        D6 - !GEAR 3
        D7 - SPARE
    A: $03
        D4 - SPARE
        D5 - DIFFICULT BONUS
        D6 - STEER A
        D7 - STEER B

Fill in the steering and gear bits in a special way.
***************************************************************************/

READ8_MEMBER(nitedrvr_state::nitedrvr_in0_r)
{
	int gear = ioport("GEARS")->read();

	if (gear & 0x10)                m_gear = 1;
	else if (gear & 0x20)           m_gear = 2;
	else if (gear & 0x40)           m_gear = 3;
	else if (gear & 0x80)           m_gear = 4;

	switch (offset & 0x03)
	{
		case 0x00:                      /* No remapping necessary */
			return ioport("DSW0")->read();
		case 0x01:                      /* No remapping necessary */
			return ioport("DSW1")->read();
		case 0x02:                      /* Remap our gear shift */
			if (m_gear == 1)
				return 0xe0;
			else if (m_gear == 2)
				return 0xd0;
			else if (m_gear == 3)
				return 0xb0;
			else
				return 0x70;
		case 0x03:                      /* Remap our steering */
			return (ioport("DSW2")->read() | nitedrvr_steering());
		default:
			return 0xff;
	}
}

/***************************************************************************
nitedrvr_in1_r

Night Driver looks for the following:
    A: $00
        D6 - SPARE
        D7 - COIN 1
    A: $01
        D6 - SPARE
        D7 - COIN 2
    A: $02
        D6 - SPARE
        D7 - !START
    A: $03
        D6 - SPARE
        D7 - !ACC
    A: $04
        D6 - SPARE
        D7 - EXPERT
    A: $05
        D6 - SPARE
        D7 - NOVICE
    A: $06
        D6 - SPARE
        D7 - Special Alternating Signal
    A: $07
        D6 - SPARE
        D7 - Ground

Fill in the track difficulty switch and special signal in a special way.
***************************************************************************/

READ8_MEMBER(nitedrvr_state::nitedrvr_in1_r)
{
	int port = ioport("IN0")->read();

	m_ac_line = (m_ac_line + 1) % 3;

	if (port & 0x10)                m_track = 0;
	else if (port & 0x20)           m_track = 1;
	else if (port & 0x40)           m_track = 2;

	switch (offset & 0x07)
	{
		case 0x00:
			return ((port & 0x01) << 7);
		case 0x01:
			return ((port & 0x02) << 6);
		case 0x02:
			return ((port & 0x04) << 5);
		case 0x03:
			return ((port & 0x08) << 4);
		case 0x04:
			if (m_track == 1) return 0x80; else return 0x00;
		case 0x05:
			if (m_track == 0) return 0x80; else return 0x00;
		case 0x06:
			/* TODO: fix alternating signal? */
			if (m_ac_line==0) return 0x80; else return 0x00;
		case 0x07:
			return 0x00;
		default:
			return 0xff;
	}
}

/***************************************************************************
nitedrvr_out0_w

Sound bits:

D0 = !SPEED1
D1 = !SPEED2
D2 = !SPEED3
D3 = !SPEED4
D4 = SKID1
D5 = SKID2
***************************************************************************/

WRITE8_MEMBER(nitedrvr_state::nitedrvr_out0_w)
{
	m_discrete->write(space, NITEDRVR_MOTOR_DATA, data & 0x0f);  // Motor freq data
	m_discrete->write(space, NITEDRVR_SKID1_EN, data & 0x10);    // Skid1 enable
	m_discrete->write(space, NITEDRVR_SKID2_EN, data & 0x20);    // Skid2 enable
}

/***************************************************************************
nitedrvr_out1_w

D0 = !CRASH - also drives a video invert signal
D1 = ATTRACT
D2 = Spare (Not used)
D3 = Not used?
D4 = LED START
D5 = Spare (Not used)
***************************************************************************/

WRITE8_MEMBER(nitedrvr_state::nitedrvr_out1_w)
{
	output().set_led_value(0, data & 0x10);

	m_crash_en = data & 0x01;

	m_discrete->write(space, NITEDRVR_CRASH_EN, m_crash_en); // Crash enable
	m_discrete->write(space, NITEDRVR_ATTRACT_EN, data & 0x02);      // Attract enable (sound disable)

	if (!m_crash_en)
	{
		/* Crash reset, set counter high and enable output */
		m_crash_data_en = 1;
		m_crash_data = 0x0f;
		/* Invert video */
		m_palette->set_pen_color(1, rgb_t(0x00,0x00,0x00)); /* BLACK */
		m_palette->set_pen_color(0, rgb_t(0xff,0xff,0xff)); /* WHITE */
	}
	m_discrete->write(space, NITEDRVR_BANG_DATA, m_crash_data_en ? m_crash_data : 0);    // Crash Volume
}


TIMER_DEVICE_CALLBACK_MEMBER(nitedrvr_state::nitedrvr_crash_toggle_callback)
{
	if (m_crash_en && m_crash_data_en)
	{
		m_crash_data--;
		address_space &space = machine().driver_data()->generic_space();
		m_discrete->write(space, NITEDRVR_BANG_DATA, m_crash_data);  // Crash Volume
		if (!m_crash_data)
			m_crash_data_en = 0;    // Done counting?

		if (m_crash_data & 0x01)
		{
			/* Invert video */
			m_palette->set_pen_color(1, rgb_t(0x00,0x00,0x00)); /* BLACK */
			m_palette->set_pen_color(0, rgb_t(0xff,0xff,0xff)); /* WHITE */
		}
		else
		{
			/* Normal video */
			m_palette->set_pen_color(0, rgb_t(0x00,0x00,0x00)); /* BLACK */
			m_palette->set_pen_color(1, rgb_t(0xff,0xff,0xff)); /* WHITE */
		}
	}
}

void nitedrvr_state::machine_start()
{
	save_item(NAME(m_gear));
	save_item(NAME(m_track));
	save_item(NAME(m_steering_buf));
	save_item(NAME(m_steering_val));
	save_item(NAME(m_crash_en));
	save_item(NAME(m_crash_data));
	save_item(NAME(m_crash_data_en));
	save_item(NAME(m_ac_line));
	save_item(NAME(m_last_steering_val));
}

void nitedrvr_state::machine_reset()
{
	m_gear = 1;
	m_track = 0;
	m_steering_buf = 0;
	m_steering_val = 0;
	m_crash_en = 0;
	m_crash_data = 0x0f;
	m_crash_data_en = 0;
	m_ac_line = 0;
	m_last_steering_val = 0;
}
