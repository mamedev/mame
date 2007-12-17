/***************************************************************************

    Atari Night Driver hardware

***************************************************************************/

#include "driver.h"
#include "nitedrvr.h"
#include "sound/discrete.h"

static UINT8 nitedrvr_gear = 1;
static UINT8 nitedrvr_track;
static INT32 nitedrvr_steering_buf;
static INT32 nitedrvr_steering_val;
static UINT8 nitedrvr_crash_en;
static UINT8 nitedrvr_crash_data = 0x0f;
static UINT8 nitedrvr_crash_data_en;	// IC D8
static UINT8 ac_line;
static INT32 last_steering_val;

/***************************************************************************
Steering

When D7 is high, the steering wheel has moved.
If D6 is low, it moved left.  If D6 is high, it moved right.
Be sure to keep returning a direction until steering_reset is called,
because D6 and D7 are apparently checked at different times, and a
change in-between can affect the direction you move.
***************************************************************************/
static int nitedrvr_steering(void)
{
	int this_val;
	int delta;

	this_val = readinputport(5);

	delta=this_val-last_steering_val;
	last_steering_val=this_val;
	if (delta>128) delta-=256;
	else if (delta<-128) delta+=256;
	/* Divide by four to make our steering less sensitive */
	nitedrvr_steering_buf+=(delta/4);

	if (nitedrvr_steering_buf>0)
	{
		nitedrvr_steering_buf--;
		nitedrvr_steering_val=0xC0;
	}
	else if (nitedrvr_steering_buf<0)
	{
		nitedrvr_steering_buf++;
		nitedrvr_steering_val=0x80;
	}
	else
	{
		nitedrvr_steering_val=0x00;
	}

	return nitedrvr_steering_val;
}

/***************************************************************************
nitedrvr_steering_reset
***************************************************************************/
READ8_HANDLER( nitedrvr_steering_reset_r )
{
	nitedrvr_steering_val = 0;
	return 0;
}

WRITE8_HANDLER( nitedrvr_steering_reset_w )
{
	nitedrvr_steering_val = 0;
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

READ8_HANDLER( nitedrvr_in0_r )
{
	int gear;

	gear = readinputport(2);
	if (gear & 0x10)				nitedrvr_gear=1;
	else if (gear & 0x20)			nitedrvr_gear=2;
	else if (gear & 0x40)			nitedrvr_gear=3;
	else if (gear & 0x80)			nitedrvr_gear=4;

	switch (offset & 0x03)
	{
		case 0x00:						/* No remapping necessary */
			return readinputport(0);
		case 0x01:						/* No remapping necessary */
			return readinputport(1);
		case 0x02:						/* Remap our gear shift */
			if (nitedrvr_gear==1)		return 0xE0;
			else if (nitedrvr_gear==2)	return 0xD0;
			else if (nitedrvr_gear==3)	return 0xB0;
			else						return 0x70;
		case 0x03:						/* Remap our steering */
			return (readinputport(3) | nitedrvr_steering());
		default:
			return 0xFF;
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

READ8_HANDLER( nitedrvr_in1_r )
{
	int port;

	ac_line=(ac_line+1) % 3;

	port = readinputport(4);
	if (port & 0x10)				nitedrvr_track=0;
	else if (port & 0x20)			nitedrvr_track=1;
	else if (port & 0x40)			nitedrvr_track=2;

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
			if (nitedrvr_track == 1) return 0x80; else return 0x00;
		case 0x05:
			if (nitedrvr_track == 0) return 0x80; else return 0x00;
		case 0x06:
			/* TODO: fix alternating signal? */
			if (ac_line==0) return 0x80; else return 0x00;
		case 0x07:
			return 0x00;
		default:
			return 0xFF;
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
WRITE8_HANDLER( nitedrvr_out0_w )
{
	discrete_sound_w(NITEDRVR_MOTOR_DATA, data & 0x0f);	// Motor freq data
	discrete_sound_w(NITEDRVR_SKID1_EN, data & 0x10);	// Skid1 enable
	discrete_sound_w(NITEDRVR_SKID2_EN, data & 0x20);	// Skid2 enable
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
WRITE8_HANDLER( nitedrvr_out1_w )
{
	set_led_status(0,data & 0x10);

	nitedrvr_crash_en = data & 0x01;
	discrete_sound_w(NITEDRVR_CRASH_EN, nitedrvr_crash_en);	// Crash enable
	discrete_sound_w(NITEDRVR_ATTRACT_EN, data & 0x02);		// Attract enable (sound disable)

	if (!nitedrvr_crash_en)
	{
		/* Crash reset, set counter high and enable output */
		nitedrvr_crash_data_en = 1;
		nitedrvr_crash_data = 0x0f;
		/* Invert video */
		palette_set_color(Machine,1,MAKE_RGB(0x00,0x00,0x00)); /* BLACK */
		palette_set_color(Machine,0,MAKE_RGB(0xff,0xff,0xff)); /* WHITE */
	}
	discrete_sound_w(NITEDRVR_BANG_DATA, nitedrvr_crash_data_en ? nitedrvr_crash_data : 0);	// Crash Volume
}


void nitedrvr_crash_toggle(running_machine *machine)
{
	if (nitedrvr_crash_en && nitedrvr_crash_data_en)
	{
		nitedrvr_crash_data--;
		discrete_sound_w(NITEDRVR_BANG_DATA, nitedrvr_crash_data);	// Crash Volume
		if (!nitedrvr_crash_data) nitedrvr_crash_data_en = 0;	// Done counting?
		if (nitedrvr_crash_data & 0x01)
		{
			/* Invert video */
			palette_set_color(machine,1,MAKE_RGB(0x00,0x00,0x00)); /* BLACK */
			palette_set_color(machine,0,MAKE_RGB(0xff,0xff,0xff)); /* WHITE */
		}
		else
		{
			/* Normal video */
			palette_set_color(machine,0,MAKE_RGB(0x00,0x00,0x00)); /* BLACK */
			palette_set_color(machine,1,MAKE_RGB(0xff,0xff,0xff)); /* WHITE */
		}
	}
}

void nitedrvr_register_machine_vars(void)
{/* save all the statics in this file. */
	state_save_register_global(nitedrvr_gear);
	state_save_register_global(nitedrvr_track);
	state_save_register_global(nitedrvr_steering_buf);
	state_save_register_global(nitedrvr_steering_val);
	state_save_register_global(nitedrvr_crash_en);
	state_save_register_global(nitedrvr_crash_data);
	state_save_register_global(nitedrvr_crash_data_en);
	state_save_register_global(ac_line);
	state_save_register_global(last_steering_val);
}
