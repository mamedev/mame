/*************************************************************************

    Atari Football hardware

*************************************************************************/

#include "emu.h"
#include "deprecat.h"
#include "includes/atarifb.h"
#include "sound/discrete.h"


WRITE8_HANDLER( atarifb_out1_w )
{
	atarifb_state *state = space->machine->driver_data<atarifb_state>();
	device_t *discrete = space->machine->device("discrete");

	state->CTRLD = data;

	discrete_sound_w(discrete, ATARIFB_WHISTLE_EN, data & 0x01);		// Whistle
	discrete_sound_w(discrete, ATARIFB_HIT_EN, data & 0x02);			// Hit
	discrete_sound_w(discrete, ATARIFB_ATTRACT_EN, data & 0x10);		// Attract
	discrete_sound_w(discrete, ATARIFB_NOISE_EN, data & 0x04);			// Noise Enable / Kicker
}


WRITE8_HANDLER( atarifb4_out1_w )
{
	atarifb_state *state = space->machine->driver_data<atarifb_state>();
	device_t *discrete = space->machine->device("discrete");

	state->CTRLD = data;

	discrete_sound_w(discrete, ATARIFB_WHISTLE_EN, data & 0x01);		// Whistle
	discrete_sound_w(discrete, ATARIFB_HIT_EN, data & 0x02);			// Hit
	discrete_sound_w(discrete, ATARIFB_ATTRACT_EN, data & 0x10);		// Attract
	discrete_sound_w(discrete, ATARIFB_NOISE_EN, data & 0x04);			// Noise Enable / Kicker

	coin_counter_w(space->machine, 1, data & 0x80);
}


WRITE8_HANDLER( abaseb_out1_w )
{
	atarifb_state *state = space->machine->driver_data<atarifb_state>();
	device_t *discrete = space->machine->device("discrete");

	state->CTRLD = data;

	discrete_sound_w(discrete, ATARIFB_WHISTLE_EN, data & 0x01);		// Whistle
	discrete_sound_w(discrete, ATARIFB_HIT_EN, data & 0x02);			// Hit
	discrete_sound_w(discrete, ATARIFB_ATTRACT_EN, data & 0x10);		// Attract
	discrete_sound_w(discrete, ATARIFB_NOISE_EN, data & 0x04);			// Noise Enable / Kicker

	if (data & 0x80)
	{
		/* Invert video */
		palette_set_color(space->machine, 1, MAKE_RGB(0x00,0x00,0x00)); /* black  */
		palette_set_color(space->machine, 0, MAKE_RGB(0xff,0xff,0xff)); /* white  */
	}
	else
	{
		/* Regular video */
		palette_set_color(space->machine, 0, MAKE_RGB(0x00,0x00,0x00)); /* black  */
		palette_set_color(space->machine, 1, MAKE_RGB(0xff,0xff,0xff)); /* white  */
	}
}


WRITE8_HANDLER( soccer_out1_w )
{
	atarifb_state *state = space->machine->driver_data<atarifb_state>();
	device_t *discrete = space->machine->device("discrete");

	state->CTRLD = data;

	/* bit 0 = whistle */
	/* bit 1 = hit */
	/* bit 2 = kicker */
	/* bit 3 = unused */
	/* bit 4 = 2/4 Player LED */	// Say what?
	/* bit 5-6 = trackball CTRL bits */
	/* bit 7 = Rule LED */

	discrete_sound_w(discrete, ATARIFB_WHISTLE_EN, data & 0x01);		// Whistle
	discrete_sound_w(discrete, ATARIFB_HIT_EN, data & 0x02);			// Hit
	discrete_sound_w(discrete, ATARIFB_ATTRACT_EN, data & 0x10);		// Attract
	discrete_sound_w(discrete, ATARIFB_NOISE_EN, data & 0x04);			// Noise Enable / Kicker

//  set_led_status(space->machine, 0, data & 0x10);  // !!!!!!!!!! Is this correct????
	set_led_status(space->machine, 1, data & 0x80);
}


WRITE8_HANDLER( atarifb_out2_w )
{
	device_t *discrete = space->machine->device("discrete");

	discrete_sound_w(discrete, ATARIFB_CROWD_DATA, data & 0x0f);	// Crowd

	coin_counter_w (space->machine, 0, data & 0x10);
}


WRITE8_HANDLER( soccer_out2_w )
{
	device_t *discrete = space->machine->device("discrete");

	discrete_sound_w(discrete, ATARIFB_CROWD_DATA, data & 0x0f);	// Crowd

	coin_counter_w (space->machine, 0, data & 0x10);
	coin_counter_w (space->machine, 1, data & 0x20);
	coin_counter_w (space->machine, 2, data & 0x40);
}



/*************************************
 *
 *  LED control
 *
 *************************************/

WRITE8_HANDLER( atarifb_out3_w )
{
	atarifb_state *state = space->machine->driver_data<atarifb_state>();
	int loop = cpu_getiloops(state->maincpu);

	switch (loop)
	{
		case 0x00:
			/* Player 1 play select lamp */
			output_set_value("ledleft0", (data >> 0) & 1);
			output_set_value("ledleft1", (data >> 1) & 1);
			output_set_value("ledleft2", (data >> 2) & 1);
			output_set_value("ledleft3", (data >> 3) & 1);
			output_set_value("ledleft4", (data >> 4) & 1);
			break;
		case 0x01:
			break;
		case 0x02:
			/* Player 2 play select lamp */
			output_set_value("ledright0", (data >> 0) & 1);
			output_set_value("ledright1", (data >> 1) & 1);
			output_set_value("ledright2", (data >> 2) & 1);
			output_set_value("ledright3", (data >> 3) & 1);
			output_set_value("ledright4", (data >> 4) & 1);
			break;
		case 0x03:
			break;
	}
//  logerror("out3_w, %02x:%02x\n", loop, data);
}


READ8_HANDLER( atarifb_in0_r )
{
	atarifb_state *state = space->machine->driver_data<atarifb_state>();
	if ((state->CTRLD & 0x20) == 0x00)
	{
		int val;

		val = (state->sign_y_2 >> 7) |
			  (state->sign_x_2 >> 6) |
			  (state->sign_y_1 >> 5) |
			  (state->sign_x_1 >> 4) |
			  input_port_read(space->machine, "IN0");
		return val;
	}
	else
	{
		int new_x, new_y;

		/* Read player 1 trackball */
		new_x = input_port_read(space->machine, "IN3");
		if (new_x != state->counter_x_in0)
		{
			state->sign_x_1 = (new_x - state->counter_x_in0) & 0x80;
			state->counter_x_in0 = new_x;
		}

		new_y = input_port_read(space->machine, "IN2");
		if (new_y != state->counter_y_in0)
		{
			state->sign_y_1 = (new_y - state->counter_y_in0) & 0x80;
			state->counter_y_in0 = new_y;
		}

		return (((state->counter_y_in0 & 0x0f) << 4) | (state->counter_x_in0 & 0x0f));
	}
}


READ8_HANDLER( atarifb_in2_r )
{
	atarifb_state *state = space->machine->driver_data<atarifb_state>();

	if ((state->CTRLD & 0x20) == 0x00)
	{
		return input_port_read(space->machine, "IN1");
	}
	else
	{
		int new_x, new_y;

		/* Read player 2 trackball */
		new_x = input_port_read(space->machine, "IN5");
		if (new_x != state->counter_x_in2)
		{
			state->sign_x_2 = (new_x - state->counter_x_in2) & 0x80;
			state->counter_x_in2 = new_x;
		}

		new_y = input_port_read(space->machine, "IN4");
		if (new_y != state->counter_y_in2)
		{
			state->sign_y_2 = (new_y - state->counter_y_in2) & 0x80;
			state->counter_y_in2 = new_y;
		}

		return (((state->counter_y_in2 & 0x0f) << 4) | (state->counter_x_in2 & 0x0f));
	}
}

READ8_HANDLER( atarifb4_in0_r )
{
	atarifb_state *state = space->machine->driver_data<atarifb_state>();

	/* LD1 and LD2 low, return sign bits */
	if ((state->CTRLD & 0x60) == 0x00)
	{
		int val;

		val = (state->sign_x_4 >> 7) |
			  (state->sign_y_4 >> 6) |
			  (state->sign_x_2 >> 5) |
			  (state->sign_y_2 >> 4) |
			  (state->sign_x_3 >> 3) |
			  (state->sign_y_3 >> 2) |
			  (state->sign_x_1 >> 1) |
			  (state->sign_y_1 >> 0);
		return val;
	}
	else if ((state->CTRLD & 0x60) == 0x60)
	/* LD1 and LD2 both high, return Team 1 right player (player 1) */
	{
		int new_x, new_y;

		/* Read player 1 trackball */
		new_x = input_port_read(space->machine, "IN3");
		if (new_x != state->counter_x_in0)
		{
			state->sign_x_1 = (new_x - state->counter_x_in0) & 0x80;
			state->counter_x_in0 = new_x;
		}

		new_y = input_port_read(space->machine, "IN2");
		if (new_y != state->counter_y_in0)
		{
			state->sign_y_1 = (new_y - state->counter_y_in0) & 0x80;
			state->counter_y_in0 = new_y;
		}

		return (((state->counter_y_in0 & 0x0f) << 4) | (state->counter_x_in0 & 0x0f));
	}
	else if ((state->CTRLD & 0x60) == 0x40)
	/* LD1 high, LD2 low, return Team 1 left player (player 2) */
	{
		int new_x, new_y;

		/* Read player 2 trackball */
		new_x = input_port_read(space->machine, "IN5");
		if (new_x != state->counter_x_in0b)
		{
			state->sign_x_2 = (new_x - state->counter_x_in0b) & 0x80;
			state->counter_x_in0b = new_x;
		}

		new_y = input_port_read(space->machine, "IN4");
		if (new_y != state->counter_y_in0b)
		{
			state->sign_y_2 = (new_y - state->counter_y_in0b) & 0x80;
			state->counter_y_in0b = new_y;
		}

		return (((state->counter_y_in0b & 0x0f) << 4) | (state->counter_x_in0b & 0x0f));
	}

	else return 0;
}


READ8_HANDLER( atarifb4_in2_r )
{
	atarifb_state *state = space->machine->driver_data<atarifb_state>();

	if ((state->CTRLD & 0x40) == 0x00)
	{
		return input_port_read(space->machine, "IN1");
	}
	else if ((state->CTRLD & 0x60) == 0x60)
	/* LD1 and LD2 both high, return Team 2 right player (player 3) */
	{
		int new_x, new_y;

		/* Read player 3 trackball */
		new_x = input_port_read(space->machine, "IN7");
		if (new_x != state->counter_x_in2)
		{
			state->sign_x_3 = (new_x - state->counter_x_in2) & 0x80;
			state->counter_x_in2 = new_x;
		}

		new_y = input_port_read(space->machine, "IN6");
		if (new_y != state->counter_y_in2)
		{
			state->sign_y_3 = (new_y - state->counter_y_in2) & 0x80;
			state->counter_y_in2 = new_y;
		}

		return (((state->counter_y_in2 & 0x0f) << 4) | (state->counter_x_in2 & 0x0f));
	}
	else if ((state->CTRLD & 0x60) == 0x40)
	/* LD1 high, LD2 low, return Team 2 left player (player 4) */
	{
		int new_x, new_y;

		/* Read player 4 trackball */
		new_x = input_port_read(space->machine, "IN9");
		if (new_x != state->counter_x_in2b)
		{
			state->sign_x_4 = (new_x - state->counter_x_in2b) & 0x80;
			state->counter_x_in2b = new_x;
		}

		new_y = input_port_read(space->machine, "IN8");
		if (new_y != state->counter_y_in2b)
		{
			state->sign_y_4 = (new_y - state->counter_y_in2b) & 0x80;
			state->counter_y_in2b = new_y;
		}

		return (((state->counter_y_in2b & 0x0f) << 4) | (state->counter_x_in2b & 0x0f));
	}

	else return 0;
}
