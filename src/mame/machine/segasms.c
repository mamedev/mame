#include "emu.h"
#include "crsshair.h"
#include "image.h"
#include "includes/segasms.h"
#include "video/smsvdp.h"
#include "sound/2413intf.h"
#include "imagedev/cartslot.h"
#include "hashfile.h"

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

#define CF_CODEMASTERS_MAPPER    0x01
#define CF_KOREAN_MAPPER         0x02
#define CF_KOREAN_ZEMINA_MAPPER  0x04
#define CF_KOREAN_NOBANK_MAPPER  0x08
#define CF_93C46_EEPROM          0x10
#define CF_ONCART_RAM            0x20
#define CF_GG_SMS_MODE           0x40

#define LGUN_RADIUS           6
#define LGUN_X_INTERVAL       4


static void setup_rom(address_space *space);


static TIMER_CALLBACK( rapid_fire_callback )
{
	sms_state *state = machine.driver_data<sms_state>();
	state->m_rapid_fire_state_1 ^= 0xff;
	state->m_rapid_fire_state_2 ^= 0xff;
}


static WRITE8_HANDLER( sms_input_write )
{
	sms_state *state = space->machine().driver_data<sms_state>();

	switch (offset)
	{
	case 0:
		switch (input_port_read_safe(space->machine(), "CTRLSEL", 0x00) & 0x0f)
		{
		case 0x03:	/* Sports Pad */
			if (data != state->m_sports_pad_last_data_1)
			{
				UINT32 cpu_cycles = downcast<cpu_device *>(&space->device())->total_cycles();

				state->m_sports_pad_last_data_1 = data;
				if (cpu_cycles - state->m_last_sports_pad_time_1 > 512)
				{
					state->m_sports_pad_state_1 = 3;
					state->m_sports_pad_1_x = input_port_read(space->machine(), "SPORT0");
					state->m_sports_pad_1_y = input_port_read(space->machine(), "SPORT1");
				}
				state->m_last_sports_pad_time_1 = cpu_cycles;
				state->m_sports_pad_state_1 = (state->m_sports_pad_state_1 + 1) & 3;
			}
			break;
		}
		break;

	case 1:
		switch (input_port_read_safe(space->machine(), "CTRLSEL", 0x00) >> 4)
		{
		case 0x03:	/* Sports Pad */
			if (data != state->m_sports_pad_last_data_2)
			{
				UINT32 cpu_cycles = downcast<cpu_device *>(&space->device())->total_cycles();

				state->m_sports_pad_last_data_2 = data;
				if (cpu_cycles - state->m_last_sports_pad_time_2 > 2048)
				{
					state->m_sports_pad_state_2 = 3;
					state->m_sports_pad_2_x = input_port_read(space->machine(), "SPORT2");
					state->m_sports_pad_2_y = input_port_read(space->machine(), "SPORT3");
				}
				state->m_last_sports_pad_time_2 = cpu_cycles;
				state->m_sports_pad_state_2 = (state->m_sports_pad_state_2 + 1) & 3;
			}
			break;
		}
		break;
	}
}

/* FIXME: this function is a hack for Light Phaser emulation. Theoretically
   sms_vdp_hcount_latch() should be used instead, but it returns incorrect
   position for unknown reason (timing?) */
static void sms_vdp_hcount_lphaser( running_machine &machine, int hpos )
{
	sms_state *state = machine.driver_data<sms_state>();
	int hpos_tmp = hpos + state->m_lphaser_x_offs;
	UINT8 tmp = ((hpos_tmp - 46) >> 1) & 0xff;

	//printf ("sms_vdp_hcount_lphaser: hpos %3d hpos_tmp %3d => hcount %2X\n", hpos, hpos_tmp, tmp);
	sms_vdp_hcount_latch_w(state->m_vdp, 0, tmp);
}


/*
    Light Phaser (light gun) emulation notes:
    - The sensor is activated based on color brightness of some individual
      pixels being drawn by the beam, at circular area where the gun is aiming.
    - Currently, brightness is calculated based only on single pixels.
    - In general, after the trigger is pressed, games draw the next frame using
      a light color pattern, to make sure sensor will be activated. If emulation
      skips that frame, sensor may stay deactivated. Frameskip set to 0 (no skip) is
      recommended to avoid problems.
    - When sensor switches from off to on, a value is latched for HCount register
      and a flag is set. The emulation uses the flag to avoid sequential latches and
      to signal that TH line is activated when the status of the input port is read.
      When the status is read, the flag is cleared, or else it is cleared later when
      the Pause status is read (end of a frame). This is necessary because the
      "Color & Switch Test" ROM only reads the TH bit after the VINT line.
    - The gun test of "Color & Switch Test" is an example that requires checks
      of sensor status independent of other events, like trigger press or TH bit
      reads. Another example is the title screen of "Hang-On & Safari Hunt", where
      the game only reads HCount register in a loop, expecting a latch by the gun.
    - The whole procedure is managed by a timer callback, that always reschedule
      itself to run in some intervals when the beam is at the circular area.
*/
static int lgun_bright_aim_area( running_machine &machine, emu_timer *timer, int lgun_x, int lgun_y )
{
	sms_state *state = machine.driver_data<sms_state>();
	const int r_x_r = LGUN_RADIUS * LGUN_RADIUS;
	screen_device *screen = machine.first_screen();
	const rectangle &visarea = screen->visible_area();
	int beam_x = screen->hpos();
	int beam_y = screen->vpos();
	int dx, dy;
	int result = 0;
	int pos_changed = 0;
	double dx_circ;

    while (1)
	{
		dy = abs(beam_y - lgun_y);

		if (dy > LGUN_RADIUS || beam_y < visarea.min_y || beam_y > visarea.max_y)
		{
			beam_y = lgun_y - LGUN_RADIUS;
			if (beam_y < visarea.min_y)
				beam_y = visarea.min_y;
			dy = abs(beam_y - lgun_y);
			pos_changed = 1;
		}
		/* step 1: r^2 = dx^2 + dy^2 */
		/* step 2: dx^2 = r^2 - dy^2 */
		/* step 3: dx = sqrt(r^2 - dy^2) */
		dx_circ = ceil((float) sqrt((float) (r_x_r - (dy * dy))));
		dx = abs(beam_x - lgun_x);

		if (dx > dx_circ || beam_x < visarea.min_x || beam_x > visarea.max_x)
		{
			if (beam_x > lgun_x)
			{
				beam_x = 0;
				beam_y++;
				continue;
			}
			beam_x = lgun_x - dx_circ;
			if (beam_x < visarea.min_x)
				beam_x = visarea.min_x;
			pos_changed = 1;
		}

		if (!pos_changed)
		{
			result = sms_vdp_check_brightness(state->m_vdp, beam_x, beam_y);
			/* next check at same line */
			beam_x += LGUN_X_INTERVAL;
			pos_changed = 1;
		}
		else
			break;
	}
	timer->adjust(machine.first_screen()->time_until_pos(beam_y, beam_x));

	return result;
}

static UINT8 sms_vdp_hcount( running_machine &machine )
{
	UINT8 tmp;
	screen_device *screen = machine.first_screen();
	int hpos = screen->hpos();

	/* alternative method: pass HCounter test, but some others fail */
	//int hpos_tmp = hpos;
	//if ((hpos + 2) % 6 == 0) hpos_tmp--;
	//tmp = ((hpos_tmp - 46) >> 1) & 0xff;

	UINT64 calc_cycles;
	attotime time_end;
	int vpos = screen->vpos();
	int max_hpos = screen->width() - 1;

	if (hpos == max_hpos)
		time_end = attotime::zero;
	else
		time_end = screen->time_until_pos(vpos, max_hpos);
	calc_cycles = machine.device<cpu_device>("maincpu")->attotime_to_clocks(time_end);

	/* equation got from SMSPower forum, posted by Flubba. */
	tmp = ((590 - (calc_cycles * 3)) / 4) & 0xff;

	//printf ("sms_vdp_hcount: hpos %3d => hcount %2X\n", hpos, tmp);
	return tmp;
}


static void sms_vdp_hcount_latch( running_machine &machine )
{
	sms_state *state = machine.driver_data<sms_state>();
	UINT8 value = sms_vdp_hcount(machine);

	sms_vdp_hcount_latch_w(state->m_vdp, 0, value);
}


static UINT16 screen_hpos_nonscaled( screen_device *screen, int scaled_hpos )
{
	const rectangle &visarea = screen->visible_area();
	int offset_x = (scaled_hpos * (visarea.max_x - visarea.min_x)) / 255;
	return visarea.min_x + offset_x;
}


static UINT16 screen_vpos_nonscaled( screen_device *screen, int scaled_vpos )
{
	const rectangle &visarea = screen->visible_area();
	int offset_y = (scaled_vpos * (visarea.max_y - visarea.min_y)) / 255;
	return visarea.min_y + offset_y;
}


static void lphaser1_sensor_check( running_machine &machine )
{
	sms_state *state = machine.driver_data<sms_state>();
	const int x = screen_hpos_nonscaled(machine.first_screen(), input_port_read(machine, "LPHASER0"));
	const int y = screen_vpos_nonscaled(machine.first_screen(), input_port_read(machine, "LPHASER1"));

	if (lgun_bright_aim_area(machine, state->m_lphaser_1_timer, x, y))
	{
		if (state->m_lphaser_1_latch == 0)
		{
			state->m_lphaser_1_latch = 1;
			sms_vdp_hcount_lphaser(machine, x);
		}
	}
}

static void lphaser2_sensor_check( running_machine &machine )
{
	sms_state *state = machine.driver_data<sms_state>();
	const int x = screen_hpos_nonscaled(machine.first_screen(), input_port_read(machine, "LPHASER2"));
	const int y = screen_vpos_nonscaled(machine.first_screen(), input_port_read(machine, "LPHASER3"));

	if (lgun_bright_aim_area(machine, state->m_lphaser_2_timer, x, y))
	{
		if (state->m_lphaser_2_latch == 0)
		{
			state->m_lphaser_2_latch = 1;
			sms_vdp_hcount_lphaser(machine, x);
		}
	}
}


// at each input port read we check if lightguns are enabled in one of the ports:
// if so, we turn on crosshair and the lightgun timer
static TIMER_CALLBACK( lightgun_tick )
{
	sms_state *state = machine.driver_data<sms_state>();

	if ((input_port_read_safe(machine, "CTRLSEL", 0x00) & 0x0f) == 0x01)
	{
		/* enable crosshair */
		crosshair_set_screen(machine, 0, CROSSHAIR_SCREEN_ALL);
		if (!state->m_lphaser_1_timer->enabled())
			lphaser1_sensor_check(machine);
	}
	else
	{
		/* disable crosshair */
		crosshair_set_screen(machine, 0, CROSSHAIR_SCREEN_NONE);
		state->m_lphaser_1_timer->enable(0);
	}

	if ((input_port_read_safe(machine, "CTRLSEL", 0x00) & 0xf0) == 0x10)
	{
		/* enable crosshair */
		crosshair_set_screen(machine, 1, CROSSHAIR_SCREEN_ALL);
		if (!state->m_lphaser_2_timer->enabled())
			lphaser2_sensor_check(machine);
	}
	else
	{
		/* disable crosshair */
		crosshair_set_screen(machine, 1, CROSSHAIR_SCREEN_NONE);
		state->m_lphaser_2_timer->enable(0);
	}
}


static TIMER_CALLBACK( lphaser_1_callback )
{
	lphaser1_sensor_check(machine);
}


static TIMER_CALLBACK( lphaser_2_callback )
{
	lphaser2_sensor_check(machine);
}


INPUT_CHANGED( lgun1_changed )
{
	sms_state *state = field.machine().driver_data<sms_state>();
	if (!state->m_lphaser_1_timer ||
		(input_port_read_safe(field.machine(), "CTRLSEL", 0x00) & 0x0f) != 0x01)
		return;

	if (newval != oldval)
		lphaser1_sensor_check(field.machine());
}

INPUT_CHANGED( lgun2_changed )
{
	sms_state *state = field.machine().driver_data<sms_state>();
	if (!state->m_lphaser_2_timer ||
		(input_port_read_safe(field.machine(), "CTRLSEL", 0x00) & 0xf0) != 0x10)
		return;

	if (newval != oldval)
		lphaser2_sensor_check(field.machine());
}


static void sms_get_inputs( address_space *space )
{
	sms_state *state = space->machine().driver_data<sms_state>();
	UINT8 data = 0x00;
	UINT32 cpu_cycles = downcast<cpu_device *>(&space->device())->total_cycles();
	running_machine &machine = space->machine();

	state->m_input_port0 = 0xff;
	state->m_input_port1 = 0xff;

	if (cpu_cycles - state->m_last_paddle_read_time > 256)
	{
		state->m_paddle_read_state ^= 0xff;
		state->m_last_paddle_read_time = cpu_cycles;
	}

	/* Check if lightgun has been chosen as input: if so, enable crosshair */
	machine.scheduler().timer_set(attotime::zero, FUNC(lightgun_tick));

	/* Player 1 */
	switch (input_port_read_safe(machine, "CTRLSEL", 0x00) & 0x0f)
	{
	case 0x00:  /* Joystick */
		data = input_port_read(machine, "PORT_DC");
		/* Check Rapid Fire setting for Button A */
		if (!(data & 0x10) && (input_port_read(machine, "RFU") & 0x01))
			data |= state->m_rapid_fire_state_1 & 0x10;

		/* Check Rapid Fire setting for Button B */
		if (!(data & 0x20) && (input_port_read(machine, "RFU") & 0x02))
			data |= state->m_rapid_fire_state_1 & 0x20;

		state->m_input_port0 = (state->m_input_port0 & 0xc0) | (data & 0x3f);
		break;

	case 0x01:  /* Light Phaser */
		data = (input_port_read(machine, "CTRLIPT") & 0x01) << 4;
		if (!(data & 0x10))
		{
			if (input_port_read(machine, "RFU") & 0x01)
				data |= state->m_rapid_fire_state_1 & 0x10;
		}
		/* just consider the button (trigger) bit */
		data |= ~0x10;
		state->m_input_port0 = (state->m_input_port0 & 0xc0) | (data & 0x3f);
		break;

	case 0x02:  /* Paddle Control */
		/* Get button A state */
		data = input_port_read(machine, "PADDLE0");

		if (state->m_paddle_read_state)
			data = data >> 4;

		state->m_input_port0 = (state->m_input_port0 & 0xc0) | (data & 0x0f) | (state->m_paddle_read_state & 0x20)
		                | ((input_port_read(machine, "CTRLIPT") & 0x02) << 3);
		break;

	case 0x03:	/* Sega Sports Pad */
		switch (state->m_sports_pad_state_1)
		{
		case 0:
			data = (state->m_sports_pad_1_x >> 4) & 0x0f;
			break;
		case 1:
			data = state->m_sports_pad_1_x & 0x0f;
			break;
		case 2:
			data = (state->m_sports_pad_1_y >> 4) & 0x0f;
			break;
		case 3:
			data = state->m_sports_pad_1_y & 0x0f;
			break;
		}
		state->m_input_port0 = (state->m_input_port0 & 0xc0) | data | ((input_port_read(machine, "CTRLIPT") & 0x0c) << 2);
		break;
	}

	/* Player 2 */
	switch (input_port_read_safe(machine, "CTRLSEL", 0x00)  & 0xf0)
	{
	case 0x00:	/* Joystick */
		data = input_port_read(machine, "PORT_DC");
		state->m_input_port0 = (state->m_input_port0 & 0x3f) | (data & 0xc0);

		data = input_port_read(machine, "PORT_DD");
		/* Check Rapid Fire setting for Button A */
		if (!(data & 0x04) && (input_port_read(machine, "RFU") & 0x04))
			data |= state->m_rapid_fire_state_2 & 0x04;

		/* Check Rapid Fire setting for Button B */
		if (!(data & 0x08) && (input_port_read(machine, "RFU") & 0x08))
			data |= state->m_rapid_fire_state_2 & 0x08;

		state->m_input_port1 = (state->m_input_port1 & 0xf0) | (data & 0x0f);
		break;

	case 0x10:	/* Light Phaser */
		data = (input_port_read(machine, "CTRLIPT") & 0x10) >> 2;
		if (!(data & 0x04))
		{
			if (input_port_read(machine, "RFU") & 0x04)
				data |= state->m_rapid_fire_state_2 & 0x04;
		}
		/* just consider the button (trigger) bit */
		data |= ~0x04;
		state->m_input_port1 = (state->m_input_port1 & 0xf0) | (data & 0x0f);
		break;

	case 0x20:	/* Paddle Control */
		/* Get button A state */
		data = input_port_read(machine, "PADDLE1");
		if (state->m_paddle_read_state)
			data = data >> 4;

		state->m_input_port0 = (state->m_input_port0 & 0x3f) | ((data & 0x03) << 6);
		state->m_input_port1 = (state->m_input_port1 & 0xf0) | ((data & 0x0c) >> 2) | (state->m_paddle_read_state & 0x08)
		                | ((input_port_read(machine, "CTRLIPT") & 0x20) >> 3);
		break;

	case 0x30:	/* Sega Sports Pad */
		switch (state->m_sports_pad_state_2)
		{
		case 0:
			data = state->m_sports_pad_2_x & 0x0f;
			break;
		case 1:
			data = (state->m_sports_pad_2_x >> 4) & 0x0f;
			break;
		case 2:
			data = state->m_sports_pad_2_y & 0x0f;
			break;
		case 3:
			data = (state->m_sports_pad_2_y >> 4) & 0x0f;
			break;
		}
		state->m_input_port0 = (state->m_input_port0 & 0x3f) | ((data & 0x03) << 6);
		state->m_input_port1 = (state->m_input_port1 & 0xf0) | (data >> 2) | ((input_port_read(machine, "CTRLIPT") & 0xc0) >> 4);
		break;
	}
}


WRITE8_HANDLER( sms_fm_detect_w )
{
	sms_state *state = space->machine().driver_data<sms_state>();

	if (state->m_has_fm)
		state->m_fm_detect = (data & 0x01);
}


READ8_HANDLER( sms_fm_detect_r )
{
	sms_state *state = space->machine().driver_data<sms_state>();

	if (state->m_has_fm)
	{
		return state->m_fm_detect;
	}
	else
	{
		if (state->m_bios_port & IO_CHIP)
		{
			return 0xff;
		}
		else
		{
			sms_get_inputs(space);
			return state->m_input_port0;
		}
	}
}

WRITE8_HANDLER( sms_io_control_w )
{
	sms_state *state = space->machine().driver_data<sms_state>();

	if (data & 0x08)
	{
		/* check if TH pin level is high (1) and was low last time */
		if (data & 0x80 && !(state->m_ctrl_reg & 0x80))
		{
			sms_vdp_hcount_latch(space->machine());
		}
		sms_input_write(space, 0, (data & 0x20) >> 5);
	}

	if (data & 0x02)
	{
		if (data & 0x20 && !(state->m_ctrl_reg & 0x20))
		{
			sms_vdp_hcount_latch(space->machine());
		}
		sms_input_write(space, 1, (data & 0x80) >> 7);
	}

	state->m_ctrl_reg = data;
}


READ8_HANDLER( sms_count_r )
{
	sms_state *state = space->machine().driver_data<sms_state>();

	if (offset & 0x01)
		return sms_vdp_hcount_latch_r(state->m_vdp, offset);
	else
		return sms_vdp_vcount_r(state->m_vdp, offset);
}


/*
 Check if the pause button is pressed.
 If the gamegear is in sms mode, check if the start button is pressed.
 */
void sms_pause_callback( running_machine &machine )
{
	sms_state *state = machine.driver_data<sms_state>();

	if (state->m_is_gamegear && !(state->m_cartridge[state->m_current_cartridge].features & CF_GG_SMS_MODE))
		return;

	if (!(input_port_read(machine, state->m_is_gamegear ? "START" : "PAUSE") & 0x80))
	{
		if (!state->m_paused)
		{
			cputag_set_input_line(machine, "maincpu", INPUT_LINE_NMI, PULSE_LINE);
		}
		state->m_paused = 1;
	}
	else
	{
		state->m_paused = 0;
	}

	/* clear Light Phaser latch flags for next frame */
	state->m_lphaser_1_latch = 0;
	state->m_lphaser_2_latch = 0;
}

READ8_HANDLER( sms_input_port_0_r )
{
	sms_state *state = space->machine().driver_data<sms_state>();

	if (state->m_bios_port & IO_CHIP)
	{
		return 0xff;
	}
	else
	{
		sms_get_inputs(space);
		return state->m_input_port0;
	}
}


READ8_HANDLER( sms_input_port_1_r )
{
	sms_state *state = space->machine().driver_data<sms_state>();

	if (state->m_bios_port & IO_CHIP)
		return 0xff;

	sms_get_inputs(space);

	/* Reset Button */
	state->m_input_port1 = (state->m_input_port1 & 0xef) | (input_port_read_safe(space->machine(), "RESET", 0x01) & 0x01) << 4;

	/* Do region detection if TH of ports A and B are set to output (0) */
	if (!(state->m_ctrl_reg & 0x0a))
	{
		/* Move bits 7,5 of IO control port into bits 7, 6 */
		state->m_input_port1 = (state->m_input_port1 & 0x3f) | (state->m_ctrl_reg & 0x80) | (state->m_ctrl_reg & 0x20) << 1;

		/* Inverse region detect value for Japanese machines */
		if (state->m_is_region_japan)
			state->m_input_port1 ^= 0xc0;
	}
	else
	{
		if (state->m_ctrl_reg & 0x02 && state->m_lphaser_1_latch)
		{
			state->m_input_port1 &= ~0x40;
			state->m_lphaser_1_latch = 0;
		}

		if (state->m_ctrl_reg & 0x08 && state->m_lphaser_2_latch)
		{
			state->m_input_port1 &= ~0x80;
			state->m_lphaser_2_latch = 0;
		}
	}

	return state->m_input_port1;
}



WRITE8_HANDLER( sms_ym2413_register_port_0_w )
{
	sms_state *state = space->machine().driver_data<sms_state>();

	if (state->m_has_fm)
		ym2413_w(state->m_ym, 0, (data & 0x3f));
}


WRITE8_HANDLER( sms_ym2413_data_port_0_w )
{
	sms_state *state = space->machine().driver_data<sms_state>();

	if (state->m_has_fm)
	{
		logerror("data_port_0_w %x %x\n", offset, data);
		ym2413_w(state->m_ym, 1, data);
	}
}


READ8_HANDLER( gg_input_port_2_r )
{
	sms_state *state = space->machine().driver_data<sms_state>();

	//logerror("joy 2 read, val: %02x, pc: %04x\n", ((state->m_is_region_japan ? 0x00 : 0x40) | (input_port_read(machine, "START") & 0x80)), activecpu_get_pc());
	return ((state->m_is_region_japan ? 0x00 : 0x40) | (input_port_read(space->machine(), "START") & 0x80));
}


READ8_HANDLER( sms_sscope_r )
{
	sms_state *state = space->machine().driver_data<sms_state>();
	return state->m_sscope_state;
}


WRITE8_HANDLER( sms_sscope_w )
{
	sms_state *state = space->machine().driver_data<sms_state>();
	state->m_sscope_state = data;
}


READ8_HANDLER( sms_mapper_r )
{
	sms_state *state = space->machine().driver_data<sms_state>();
	return state->m_mapper[offset];
}

/* Terebi Oekaki */
/* The following code comes from sg1000.c. We should eventually merge these TV Draw implementations */
static WRITE8_HANDLER( sms_tvdraw_axis_w )
{
	sms_state *state = space->machine().driver_data<sms_state>();
	UINT8 tvboard_on = input_port_read_safe(space->machine(), "TVDRAW", 0x00);

	if (data & 0x01)
	{
		state->m_tvdraw_data = tvboard_on ? input_port_read(space->machine(), "TVDRAW_X") : 0x80;

		if (state->m_tvdraw_data < 4) state->m_tvdraw_data = 4;
		if (state->m_tvdraw_data > 251) state->m_tvdraw_data = 251;
	}
	else
	{
		state->m_tvdraw_data = tvboard_on ? input_port_read(space->machine(), "TVDRAW_Y") + 0x20 : 0x80;
	}
}

static READ8_HANDLER( sms_tvdraw_status_r )
{
	UINT8 tvboard_on = input_port_read_safe(space->machine(), "TVDRAW", 0x00);
	return tvboard_on ? input_port_read(space->machine(), "TVDRAW_PEN") : 0x01;
}

static READ8_HANDLER( sms_tvdraw_data_r )
{
	sms_state *state = space->machine().driver_data<sms_state>();
	return state->m_tvdraw_data;
}


WRITE8_HANDLER( sms_mapper_w )
{
	sms_state *state = space->machine().driver_data<sms_state>();
	int page;
	UINT8 *SOURCE_BIOS;
	UINT8 *SOURCE_CART;
	UINT8 *SOURCE;
	UINT8 rom_page_count = state->m_cartridge[state->m_current_cartridge].size / 0x4000;

	offset &= 3;

	state->m_mapper[offset] = data;
	state->m_mapper_ram[offset] = data;

	if (state->m_cartridge[state->m_current_cartridge].ROM)
	{
		SOURCE_CART = state->m_cartridge[state->m_current_cartridge].ROM + (( rom_page_count > 0) ? data % rom_page_count : 0) * 0x4000;
	}
	else
	{
		SOURCE_CART = state->m_banking_none[1];
	}

	if (state->m_BIOS)
	{
		SOURCE_BIOS = state->m_BIOS + ((state->m_bios_page_count > 0) ? data % state->m_bios_page_count : 0) * 0x4000;
	}
	else
	{
		SOURCE_BIOS = state->m_banking_none[1];
	}

	if (state->m_bios_port & IO_BIOS_ROM || (state->m_is_gamegear && state->m_BIOS == NULL))
	{
		if (!(state->m_bios_port & IO_CARTRIDGE) || (state->m_is_gamegear && state->m_BIOS == NULL))
		{
			page = (rom_page_count > 0) ? data % rom_page_count : 0;
			if (!state->m_cartridge[state->m_current_cartridge].ROM)
				return;
			SOURCE = SOURCE_CART;
		}
		else
		{
			/* nothing to page in */
			return;
		}
	}
	else
	{
		page = (state->m_bios_page_count > 0) ? data % state->m_bios_page_count : 0;
		if (!state->m_BIOS)
			return;
		SOURCE = SOURCE_BIOS;
	}

	switch (offset)
	{
	case 0: /* Control */
		/* Is it ram or rom? */
		if (data & 0x08) /* it's ram */
		{
			state->m_cartridge[state->m_current_cartridge].sram_save = 1;			/* SRAM should be saved on exit. */
			if (data & 0x04)
			{
				LOG(("ram 1 paged.\n"));
				SOURCE = state->m_cartridge[state->m_current_cartridge].cartSRAM + 0x4000;
			}
			else
			{
				LOG(("ram 0 paged.\n"));
				SOURCE = state->m_cartridge[state->m_current_cartridge].cartSRAM;
			}
			memory_set_bankptr(space->machine(),  "bank5", SOURCE);
			memory_set_bankptr(space->machine(),  "bank6", SOURCE + 0x2000);
		}
		else /* it's rom */
		{
			if (state->m_bios_port & IO_BIOS_ROM || ! state->m_has_bios)
			{
				page = (rom_page_count > 0) ? state->m_mapper[3] % rom_page_count : 0;
				SOURCE = state->m_banking_cart[5];
			}
			else
			{
				page = (state->m_bios_page_count > 0) ? state->m_mapper[3] % state->m_bios_page_count : 0;
				SOURCE = state->m_banking_bios[5];
			}
			LOG(("rom 2 paged in %x.\n", page));
			memory_set_bankptr(space->machine(), "bank5", SOURCE);
			memory_set_bankptr(space->machine(), "bank6", SOURCE + 0x2000);
		}
		break;

	case 1: /* Select 16k ROM bank for 0400-3FFF */
		LOG(("rom 0 paged in %x.\n", page));
		state->m_banking_bios[2] = SOURCE_BIOS + 0x0400;
		state->m_banking_cart[2] = SOURCE_CART + 0x0400;
		if (state->m_is_gamegear)
			SOURCE = SOURCE_CART;

		memory_set_bankptr(space->machine(), "bank2", SOURCE + 0x0400);
		break;

	case 2: /* Select 16k ROM bank for 4000-7FFF */
		LOG(("rom 1 paged in %x.\n", page));
		state->m_banking_bios[3] = SOURCE_BIOS;
		state->m_banking_cart[3] = SOURCE_CART;
		if (state->m_is_gamegear)
			SOURCE = SOURCE_CART;

		memory_set_bankptr(space->machine(), "bank3", SOURCE);
		memory_set_bankptr(space->machine(), "bank4", SOURCE + 0x2000);
		break;

	case 3: /* Select 16k ROM bank for 8000-BFFF */
		state->m_banking_bios[5] = SOURCE_BIOS;
		if (state->m_is_gamegear)
			SOURCE = SOURCE_CART;

		if ( state->m_cartridge[state->m_current_cartridge].features & CF_CODEMASTERS_MAPPER)
		{
			if (SOURCE == SOURCE_CART)
			{
				SOURCE = state->m_banking_cart[5];
			}
		}
		else
		{
			state->m_banking_cart[5] = SOURCE_CART;
		}

		if (!(state->m_mapper[0] & 0x08)) /* is RAM disabled? */
		{
			LOG(("rom 2 paged in %x.\n", page));
			memory_set_bankptr(space->machine(), "bank5", SOURCE);
			memory_set_bankptr(space->machine(), "bank6", SOURCE + 0x2000);
		}
		break;
	}
}

#ifdef MESS
static WRITE8_HANDLER( sms_korean_zemina_banksw_w )
{
	sms_state *state = space->machine().driver_data<sms_state>();

	if (state->m_cartridge[state->m_current_cartridge].features & CF_KOREAN_ZEMINA_MAPPER)
	{
		UINT8 rom_page_count = state->m_cartridge[state->m_current_cartridge].size / 0x2000;
		int page = (rom_page_count > 0) ? data % rom_page_count : 0;

		if (!state->m_cartridge[state->m_current_cartridge].ROM)
			return;

		switch (offset & 3)
		{
			case 0:
				state->m_banking_cart[5] = state->m_cartridge[state->m_current_cartridge].ROM + page * 0x2000;
				memory_set_bankptr(space->machine(), "bank5", state->m_banking_cart[5]);
				break;
			case 1:
				state->m_banking_cart[6] = state->m_cartridge[state->m_current_cartridge].ROM + page * 0x2000;
				memory_set_bankptr(space->machine(), "bank6", state->m_banking_cart[6]);
				break;
			case 2:
				state->m_banking_cart[3] = state->m_cartridge[state->m_current_cartridge].ROM + page * 0x2000;
				memory_set_bankptr(space->machine(), "bank3", state->m_banking_cart[3]);
				break;
			case 3:
				state->m_banking_cart[4] = state->m_cartridge[state->m_current_cartridge].ROM + page * 0x2000;
				memory_set_bankptr(space->machine(), "bank4", state->m_banking_cart[4]);
				break;
		}
		LOG(("Zemina mapper write: offset %x data %x.\n", offset, page));
	}
}

static WRITE8_HANDLER( sms_codemasters_page0_w )
{
	sms_state *state = space->machine().driver_data<sms_state>();

	if (state->m_cartridge[state->m_current_cartridge].ROM && state->m_cartridge[state->m_current_cartridge].features & CF_CODEMASTERS_MAPPER)
	{
		UINT8 rom_page_count = state->m_cartridge[state->m_current_cartridge].size / 0x4000;
		state->m_banking_cart[1] = state->m_cartridge[state->m_current_cartridge].ROM + ((rom_page_count > 0) ? data % rom_page_count : 0) * 0x4000;
		state->m_banking_cart[2] = state->m_banking_cart[1] + 0x0400;
		memory_set_bankptr(space->machine(), "bank1", state->m_banking_cart[1]);
		memory_set_bankptr(space->machine(), "bank2", state->m_banking_cart[2]);
	}
}


static WRITE8_HANDLER( sms_codemasters_page1_w )
{
	sms_state *state = space->machine().driver_data<sms_state>();

	if (state->m_cartridge[state->m_current_cartridge].ROM && state->m_cartridge[state->m_current_cartridge].features & CF_CODEMASTERS_MAPPER)
	{
		/* Check if we need to switch in some RAM */
		if (data & 0x80)
		{
			state->m_cartridge[state->m_current_cartridge].ram_page = data & 0x07;
			memory_set_bankptr(space->machine(), "bank6", state->m_cartridge[state->m_current_cartridge].cartRAM + state->m_cartridge[state->m_current_cartridge].ram_page * 0x2000);
		}
		else
		{
			UINT8 rom_page_count = state->m_cartridge[state->m_current_cartridge].size / 0x4000;
			state->m_banking_cart[3] = state->m_cartridge[state->m_current_cartridge].ROM + ((rom_page_count > 0) ? data % rom_page_count : 0) * 0x4000;
			memory_set_bankptr(space->machine(), "bank3", state->m_banking_cart[3]);
			memory_set_bankptr(space->machine(), "bank4", state->m_banking_cart[3] + 0x2000);
			memory_set_bankptr(space->machine(), "bank6", state->m_banking_cart[5] + 0x2000);
		}
	}
}
#endif

static READ8_HANDLER( sms_kor_nobank_r )
{
//  printf("read %x\n", offset);
	return space->read_byte(0xdffc + offset);
}

static WRITE8_HANDLER( sms_kor_nobank_w )
{
	sms_state *state = space->machine().driver_data<sms_state>();
	UINT8 *SOURCE_BIOS;
	UINT8 *SOURCE = NULL;

	space->write_byte(0xdffc + offset, data);
	state->m_mapper[offset] = data;

	// FIXME: the code below (which only handles the BIOS bankswitch at start) should be cleaned up
	// currently, it's just a stripped down version of sms_mapper_w
	if (state->m_BIOS)
		SOURCE_BIOS = state->m_BIOS + ((state->m_bios_page_count > 0) ? data % state->m_bios_page_count : 0) * 0x4000;
	else
		SOURCE_BIOS = state->m_banking_none[1];

	if (state->m_bios_port & IO_BIOS_ROM || (state->m_is_gamegear && state->m_BIOS == NULL))
		return;
	else
	{
		if (!state->m_BIOS)
			return;
		SOURCE = SOURCE_BIOS;
	}

//  printf("write %x, %x\n", offset, data);

	switch (offset)
	{
		case 0: /* Control */
			if (!(data & 0x08)) /* it's not ram */
			{
				if (!(state->m_bios_port & IO_BIOS_ROM) && state->m_has_bios)
				{
					SOURCE = state->m_banking_bios[5];
					memory_set_bankptr(space->machine(), "bank5", SOURCE);
					memory_set_bankptr(space->machine(), "bank6", SOURCE + 0x2000);
				}
			}
			break;

		case 1: /* Select 16k ROM bank for 0400-3FFF */
			state->m_banking_bios[2] = SOURCE_BIOS + 0x0400;
			memory_set_bankptr(space->machine(), "bank2", SOURCE + 0x0400);
			break;

		case 2: /* Select 16k ROM bank for 4000-7FFF */
			state->m_banking_bios[3] = SOURCE_BIOS;
			memory_set_bankptr(space->machine(), "bank3", SOURCE);
			memory_set_bankptr(space->machine(), "bank4", SOURCE + 0x2000);
			break;

		case 3: /* Select 16k ROM bank for 8000-BFFF */
			state->m_banking_bios[5] = SOURCE_BIOS;
			if (!(state->m_mapper[0] & 0x08)) /* is RAM disabled? */
			{
				memory_set_bankptr(space->machine(), "bank5", SOURCE);
				memory_set_bankptr(space->machine(), "bank6", SOURCE + 0x2000);
			}
			break;
	}
}

WRITE8_HANDLER( sms_bios_w )
{
	sms_state *state = space->machine().driver_data<sms_state>();
	state->m_bios_port = data;

	logerror("bios write %02x, pc: %04x\n", data, cpu_get_pc(&space->device()));

	setup_rom(space);
}


WRITE8_HANDLER( sms_cartram2_w )
{
	sms_state *state = space->machine().driver_data<sms_state>();

	if (state->m_mapper[0] & 0x08)
	{
		logerror("write %02X to cartram at offset #%04X\n", data, offset + 0x2000);
		if (state->m_mapper[0] & 0x04)
		{
			state->m_cartridge[state->m_current_cartridge].cartSRAM[offset + 0x6000] = data;
		}
		else
		{
			state->m_cartridge[state->m_current_cartridge].cartSRAM[offset + 0x2000] = data;
		}
	}

	if (state->m_cartridge[state->m_current_cartridge].features & CF_CODEMASTERS_MAPPER)
	{
		state->m_cartridge[state->m_current_cartridge].cartRAM[state->m_cartridge[state->m_current_cartridge].ram_page * 0x2000 + offset] = data;
	}

	if (state->m_cartridge[state->m_current_cartridge].features & CF_KOREAN_MAPPER && offset == 0) /* Dodgeball King mapper */
	{
		UINT8 rom_page_count = state->m_cartridge[state->m_current_cartridge].size / 0x4000;
		int page = (rom_page_count > 0) ? data % rom_page_count : 0;

		if (!state->m_cartridge[state->m_current_cartridge].ROM)
			return;

		state->m_banking_cart[5] = state->m_cartridge[state->m_current_cartridge].ROM + page * 0x4000;
		memory_set_bankptr(space->machine(), "bank5", state->m_banking_cart[5]);
		memory_set_bankptr(space->machine(), "bank6", state->m_banking_cart[5] + 0x2000);
		LOG(("rom 2 paged in %x (Korean mapper).\n", page));
	}
}


WRITE8_HANDLER( sms_cartram_w )
{
	sms_state *state = space->machine().driver_data<sms_state>();
	int page;

	if (state->m_mapper[0] & 0x08)
	{
		logerror("write %02X to cartram at offset #%04X\n", data, offset);
		if (state->m_mapper[0] & 0x04)
		{
			state->m_cartridge[state->m_current_cartridge].cartSRAM[offset + 0x4000] = data;
		}
		else
		{
			state->m_cartridge[state->m_current_cartridge].cartSRAM[offset] = data;
		}
	}
	else
	{
		if (state->m_cartridge[state->m_current_cartridge].features & CF_CODEMASTERS_MAPPER && offset == 0) /* Codemasters mapper */
		{
			UINT8 rom_page_count = state->m_cartridge[state->m_current_cartridge].size / 0x4000;
			page = (rom_page_count > 0) ? data % rom_page_count : 0;
			if (!state->m_cartridge[state->m_current_cartridge].ROM)
				return;
			state->m_banking_cart[5] = state->m_cartridge[state->m_current_cartridge].ROM + page * 0x4000;
			memory_set_bankptr(space->machine(), "bank5", state->m_banking_cart[5]);
			memory_set_bankptr(space->machine(), "bank6", state->m_banking_cart[5] + 0x2000);
			LOG(("rom 2 paged in %x (Codemasters mapper).\n", page));
		}
		else if (state->m_cartridge[state->m_current_cartridge].features & CF_ONCART_RAM)
		{
			state->m_cartridge[state->m_current_cartridge].cartRAM[offset & (state->m_cartridge[state->m_current_cartridge].ram_size - 1)] = data;
		}
		else
		{
			logerror("INVALID write %02X to cartram at offset #%04X\n", data, offset);
		}
	}
}


WRITE8_HANDLER( gg_sio_w )
{
	sms_state *state = space->machine().driver_data<sms_state>();
	logerror("*** write %02X to SIO register #%d\n", data, offset);

	state->m_gg_sio[offset & 0x07] = data;
	switch (offset & 7)
	{
		case 0x00: /* Parallel Data */
			break;

		case 0x01: /* Data Direction/ NMI Enable */
			break;

		case 0x02: /* Serial Output */
			break;

		case 0x03: /* Serial Input */
			break;

		case 0x04: /* Serial Control / Status */
			break;
	}
}


READ8_HANDLER( gg_sio_r )
{
	sms_state *state = space->machine().driver_data<sms_state>();
	logerror("*** read SIO register #%d\n", offset);

	switch (offset & 7)
	{
		case 0x00: /* Parallel Data */
			break;

		case 0x01: /* Data Direction/ NMI Enable */
			break;

		case 0x02: /* Serial Output */
			break;

		case 0x03: /* Serial Input */
			break;

		case 0x04: /* Serial Control / Status */
			break;
	}

	return state->m_gg_sio[offset];
}

static void sms_machine_stop( running_machine &machine )
{
	sms_state *state = machine.driver_data<sms_state>();

	/* Does the cartridge have SRAM that should be saved? */
	if (state->m_cartridge[state->m_current_cartridge].sram_save) {
		device_image_interface *image = dynamic_cast<device_image_interface *>(machine.device("cart1"));
		image->battery_save(state->m_cartridge[state->m_current_cartridge].cartSRAM, sizeof(UINT8) * NVRAM_SIZE );
	}
}


static void setup_rom( address_space *space )
{
	sms_state *state = space->machine().driver_data<sms_state>();
	running_machine &machine = space->machine();

	/* 1. set up bank pointers to point to nothing */
	memory_set_bankptr(machine, "bank1", state->m_banking_none[1]);
	memory_set_bankptr(machine, "bank2", state->m_banking_none[2]);
	memory_set_bankptr(machine, "bank3", state->m_banking_none[3]);
	memory_set_bankptr(machine, "bank4", state->m_banking_none[3] + 0x2000);
	memory_set_bankptr(machine, "bank5", state->m_banking_none[5]);
	memory_set_bankptr(machine, "bank6", state->m_banking_none[5] + 0x2000);

	/* 2. check and set up expansion port */
	if (!(state->m_bios_port & IO_EXPANSION) && (state->m_bios_port & IO_CARTRIDGE) && (state->m_bios_port & IO_CARD))
	{
		/* TODO: Implement me */
		logerror("Switching to unsupported expansion port.\n");
	}

	/* 3. check and set up card rom */
	if (!(state->m_bios_port & IO_CARD) && (state->m_bios_port & IO_CARTRIDGE) && (state->m_bios_port & IO_EXPANSION))
	{
		/* TODO: Implement me */
		logerror("Switching to unsupported card rom port.\n");
	}

	/* 4. check and set up cartridge rom */
	/* if ((!(bios_port & IO_CARTRIDGE) && (bios_port & IO_EXPANSION) && (bios_port & IO_CARD)) || state->m_is_gamegear) { */
	/* Out Run Europa initially writes a value to port 3E where IO_CARTRIDGE, IO_EXPANSION and IO_CARD are reset */
	if ((!(state->m_bios_port & IO_CARTRIDGE)) || state->m_is_gamegear)
	{
		memory_set_bankptr(machine, "bank1", state->m_banking_cart[1]);
		memory_set_bankptr(machine, "bank2", state->m_banking_cart[2]);
		memory_set_bankptr(machine, "bank3", state->m_banking_cart[3]);
		memory_set_bankptr(machine, "bank4", state->m_banking_cart[3] + 0x2000);
		memory_set_bankptr(machine, "bank5", state->m_banking_cart[5]);
		memory_set_bankptr(machine, "bank6", state->m_banking_cart[5] + 0x2000);
		logerror("Switched in cartridge rom.\n");
	}

	/* 5. check and set up bios rom */
	if (!(state->m_bios_port & IO_BIOS_ROM))
	{
		/* 0x0400 bioses */
		if (state->m_has_bios_0400)
		{
			memory_set_bankptr(machine, "bank1", state->m_banking_bios[1]);
			logerror("Switched in 0x0400 bios.\n");
		}
		/* 0x2000 bioses */
		if (state->m_has_bios_2000)
		{
			memory_set_bankptr(machine, "bank1", state->m_banking_bios[1]);
			memory_set_bankptr(machine, "bank2", state->m_banking_bios[2]);
			logerror("Switched in 0x2000 bios.\n");
		}
		if (state->m_has_bios_full)
		{
			memory_set_bankptr(machine, "bank1", state->m_banking_bios[1]);
			memory_set_bankptr(machine, "bank2", state->m_banking_bios[2]);
			memory_set_bankptr(machine, "bank3", state->m_banking_bios[3]);
			memory_set_bankptr(machine, "bank4", state->m_banking_bios[3] + 0x2000);
			memory_set_bankptr(machine, "bank5", state->m_banking_bios[5]);
			memory_set_bankptr(machine, "bank6", state->m_banking_bios[5] + 0x2000);
			logerror("Switched in full bios.\n");
		}
	}

	if (state->m_cartridge[state->m_current_cartridge].features & CF_ONCART_RAM)
	{
		memory_set_bankptr(machine, "bank5", state->m_cartridge[state->m_current_cartridge].cartRAM);
		memory_set_bankptr(machine, "bank6", state->m_cartridge[state->m_current_cartridge].cartRAM);
	}
}


static int sms_verify_cart( UINT8 *magic, int size )
{
	int retval;

	retval = IMAGE_VERIFY_FAIL;

	/* Verify the file is a valid image - check $7ff0 for "TMR SEGA" */
	if (size >= 0x8000)
	{
		if (!strncmp((char*)&magic[0x7ff0], "TMR SEGA", 8))
		{
#if 0
			/* Technically, it should be this, but remove for now until verified: */
			if (!strcmp(sysname, "gamegear"))
			{
				if ((unsigned char)magic[0x7ffd] < 0x50)
					retval = IMAGE_VERIFY_PASS;
			}
			if (!strcmp(sysname, "sms"))
			{
				if ((unsigned char)magic[0x7ffd] >= 0x50)
					retval = IMAGE_VERIFY_PASS;
			}
#endif
			retval = IMAGE_VERIFY_PASS;
		}

#if 0
		/* Check at $81f0 also */
		if (!retval)
		{
			if (!strncmp(&magic[0x81f0], "TMR SEGA", 8))
			{
#if 0
				/* Technically, it should be this, but remove for now until verified: */
				if (!strcmp(sysname, "gamegear"))
				{
					if ((unsigned char)magic[0x81fd] < 0x50)
						retval = IMAGE_VERIFY_PASS;
				}
				if (!strcmp(sysname, "sms"))
				{
					if ((unsigned char)magic[0x81fd] >= 0x50)
						retval = IMAGE_VERIFY_PASS;
				}
#endif
				retval = IMAGE_VERIFY_PASS;
			}
		}
#endif

	}

	return retval;
}

#ifdef UNUSED_FUNCTION
// For the moment we switch to a different detection routine which allows to detect
// in a single run Codemasters mapper, Korean mapper (including Jang Pung 3 which
// uses a diff signature then the one below here) and Zemina mapper (used by Wonsiin, etc.).
// I leave these here to document alt. detection routines and in the case these functions
// can be updated

/* Check for Codemasters mapper
  0x7FE3 - 93 - sms Cosmis Spacehead
              - sms Dinobasher
              - sms The Excellent Dizzy Collection
              - sms Fantastic Dizzy
              - sms Micro Machines
              - gamegear Cosmic Spacehead
              - gamegear Micro Machines
         - 94 - gamegear Dropzone
              - gamegear Ernie Els Golf (also has 64KB additional RAM on the cartridge)
              - gamegear Pete Sampras Tennis
              - gamegear S.S. Lucifer
         - 95 - gamegear Micro Machines 2 - Turbo Tournament

The Korean game Jang Pung II also seems to use a codemasters style mapper.
 */
static int detect_codemasters_mapper( UINT8 *rom )
{
	static const UINT8 jang_pung2[16] = { 0x00, 0xba, 0x38, 0x0d, 0x00, 0xb8, 0x38, 0x0c, 0x00, 0xb6, 0x38, 0x0b, 0x00, 0xb4, 0x38, 0x0a };

	if (((rom[0x7fe0] & 0x0f ) <= 9) && (rom[0x7fe3] == 0x93 || rom[0x7fe3] == 0x94 || rom[0x7fe3] == 0x95) &&  rom[0x7fef] == 0x00)
		return 1;

	if (!memcmp(&rom[0x7ff0], jang_pung2, 16))
		return 1;

	return 0;
}


static int detect_korean_mapper( UINT8 *rom )
{
	static const UINT8 signatures[2][16] =
	{
		{ 0x3e, 0x11, 0x32, 0x00, 0xa0, 0x78, 0xcd, 0x84, 0x85, 0x3e, 0x02, 0x32, 0x00, 0xa0, 0xc9, 0xff }, /* Dodgeball King */
		{ 0x41, 0x48, 0x37, 0x37, 0x44, 0x37, 0x4e, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x20 },	/* Sangokushi 3 */
	};
	int i;

	for (i = 0; i < 2; i++)
	{
		if (!memcmp(&rom[0x7ff0], signatures[i], 16))
		{
			return 1;
		}
	}
	return 0;
}
#endif


static int detect_tvdraw( UINT8 *rom )
{
	static const UINT8 terebi_oekaki[7] = { 0x61, 0x6e, 0x6e, 0x61, 0x6b, 0x6d, 0x6e };	// "annakmn"

	if (!memcmp(&rom[0x13b3], terebi_oekaki, 7))
		return 1;

	return 0;
}


static int detect_lphaser_xoffset( running_machine &machine, UINT8 *rom )
{
	sms_state *state = machine.driver_data<sms_state>();

	static const UINT8 signatures[6][16] =
	{
		/* Spacegun */
		{ 0x54, 0x4d, 0x52, 0x20, 0x53, 0x45, 0x47, 0x41, 0xff, 0xff, 0x9d, 0x99, 0x10, 0x90, 0x00, 0x40 },
		/* Gangster Town */
		{ 0x54, 0x4d, 0x52, 0x20, 0x53, 0x45, 0x47, 0x41, 0x19, 0x87, 0x1b, 0xc9, 0x74, 0x50, 0x00, 0x4f },
		/* Shooting Gallery */
		{ 0x54, 0x4d, 0x52, 0x20, 0x53, 0x45, 0x47, 0x41, 0x20, 0x20, 0x8a, 0x3a, 0x72, 0x50, 0x00, 0x4f },
		/* Rescue Mission */
		{ 0x54, 0x4d, 0x52, 0x20, 0x53, 0x45, 0x47, 0x41, 0x20, 0x20, 0xfb, 0xd3, 0x06, 0x51, 0x00, 0x4f },
		/* Laser Ghost */
		{ 0x54, 0x4d, 0x52, 0x20, 0x53, 0x45, 0x47, 0x41, 0x00, 0x00, 0xb7, 0x55, 0x74, 0x70, 0x00, 0x40 },
		/* Assault City */
		{ 0x54, 0x4d, 0x52, 0x20, 0x53, 0x45, 0x47, 0x41, 0xff, 0xff, 0x9f, 0x74, 0x34, 0x70, 0x00, 0x40 },
	};

	if (!(state->m_bios_port & IO_CARTRIDGE) && state->m_cartridge[state->m_current_cartridge].size >= 0x8000)
	{
		if (!memcmp(&rom[0x7ff0], signatures[0], 16) || !memcmp(&rom[0x7ff0], signatures[1], 16))
			return 40;

		if (!memcmp(&rom[0x7ff0], signatures[2], 16))
			return 49;

		if (!memcmp(&rom[0x7ff0], signatures[3], 16))
			return 47;

		if (!memcmp(&rom[0x7ff0], signatures[4], 16))
			return 44;

		if (!memcmp(&rom[0x7ff0], signatures[5], 16))
			return 53;

	}
	return 50;
}


DEVICE_START( sms_cart )
{
	sms_state *state = device->machine().driver_data<sms_state>();
	int i;

	for (i = 0; i < MAX_CARTRIDGES; i++)
	{
		state->m_cartridge[i].ROM = NULL;
		state->m_cartridge[i].size = 0;
		state->m_cartridge[i].features = 0;
		state->m_cartridge[i].cartSRAM = NULL;
		state->m_cartridge[i].sram_save = 0;
		state->m_cartridge[i].cartRAM = NULL;
		state->m_cartridge[i].ram_size = 0;
		state->m_cartridge[i].ram_page = 0;
	}
	state->m_current_cartridge = 0;

	state->m_bios_port = (IO_EXPANSION | IO_CARTRIDGE | IO_CARD);
	if (!state->m_is_gamegear && !state->m_has_bios)
	{
		state->m_bios_port &= ~(IO_CARTRIDGE);
		state->m_bios_port |= IO_BIOS_ROM;
	}
}


DEVICE_IMAGE_LOAD( sms_cart )
{
	running_machine &machine = image.device().machine();
	sms_state *state = machine.driver_data<sms_state>();
	int size, index = 0, offset = 0;
	const char *extrainfo = NULL;

	if (strcmp(image.device().tag(), "cart1") == 0)
		index = 0;
	if (strcmp(image.device().tag(), "cart2") == 0)
		index = 1;
	if (strcmp(image.device().tag(), "cart3") == 0)
		index = 2;
	if (strcmp(image.device().tag(), "cart4") == 0)
		index = 3;
	if (strcmp(image.device().tag(), "cart5") == 0)
		index = 4;
	if (strcmp(image.device().tag(), "cart6") == 0)
		index = 5;
	if (strcmp(image.device().tag(), "cart7") == 0)
		index = 6;
	if (strcmp(image.device().tag(), "cart8") == 0)
		index = 7;
	if (strcmp(image.device().tag(), "cart9") == 0)
		index = 8;
	if (strcmp(image.device().tag(), "cart10") == 0)
		index = 9;
	if (strcmp(image.device().tag(), "cart11") == 0)
		index = 10;
	if (strcmp(image.device().tag(), "cart12") == 0)
		index = 11;
	if (strcmp(image.device().tag(), "cart13") == 0)
		index = 12;
	if (strcmp(image.device().tag(), "cart14") == 0)
		index = 13;
	if (strcmp(image.device().tag(), "cart15") == 0)
		index = 14;
	if (strcmp(image.device().tag(), "cart16") == 0)
		index = 15;

	state->m_cartridge[index].features = 0;

	if (image.software_entry() == NULL)
	{
		size = image.length();
		extrainfo = hashfile_extrainfo(image);
	}
	else
		size = image.get_software_region_length("rom");

	/* Check for 512-byte header */
	if ((size / 512) & 1)
	{
		offset = 512;
		size -= 512;
	}

	if (!size)
	{
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Invalid ROM image: ROM image is too small");
		return IMAGE_INIT_FAIL;
	}

	/* Create a new memory region to hold the ROM. */
	/* Make sure the region holds only complete (0x4000) rom banks */
	state->m_cartridge[index].size = (size & 0x3fff) ? (((size >> 14) + 1) << 14) : size;
	state->m_cartridge[index].ROM = auto_alloc_array(machine, UINT8, state->m_cartridge[index].size);
	state->m_cartridge[index].cartSRAM = auto_alloc_array(machine, UINT8, NVRAM_SIZE);

	/* Load ROM banks */
	if (image.software_entry() == NULL)
	{
		image.fseek(offset, SEEK_SET);

		if (image.fread( state->m_cartridge[index].ROM, size) != size)
			return IMAGE_INIT_FAIL;
	}
	else
	{
		memcpy(state->m_cartridge[index].ROM, image.get_software_region("rom") + offset, size);

		// check for mapper feature (currently only KOREAN_NOBANK, but eventually we will add the other mappers as well)
		if (image.get_feature("pcb") != NULL)
		{
			const char *mapper = image.get_feature("pcb");
			if (!strcmp(mapper, "korean_nobank"))
				state->m_cartridge[index].features |= CF_KOREAN_NOBANK_MAPPER;
		}
	}

	/* check the image */
	if (!state->m_has_bios)
	{
		if (sms_verify_cart(state->m_cartridge[index].ROM, size) == IMAGE_VERIFY_FAIL)
		{
			logerror("Warning loading image: sms_verify_cart failed\n");
		}
	}

	/* Detect special features from the extrainfo field */
	if (extrainfo)
	{
		/* Check for codemasters mapper */
		if (strstr(extrainfo, "CODEMASTERS"))
			state->m_cartridge[index].features |= CF_CODEMASTERS_MAPPER;

		/* Check for korean mapper */
		if (strstr(extrainfo, "KOREAN"))
			state->m_cartridge[index].features |= CF_KOREAN_MAPPER;

		/* Check for special SMS Compatibility mode gamegear cartridges */
		if (state->m_is_gamegear && strstr(extrainfo, "GGSMS"))
			state->m_cartridge[index].features |= CF_GG_SMS_MODE;

		/* Check for 93C46 eeprom */
		if (strstr(extrainfo, "93C46"))
			state->m_cartridge[index].features |= CF_93C46_EEPROM;

		/* Check for 8KB on-cart RAM */
		if (strstr(extrainfo, "8KB_CART_RAM"))
		{
			state->m_cartridge[index].features |= CF_ONCART_RAM;
			state->m_cartridge[index].ram_size = 0x2000;
			state->m_cartridge[index].cartRAM = auto_alloc_array(machine, UINT8, state->m_cartridge[index].ram_size);
		}
	}
	else
	{
		/* If no extrainfo information is available try to find special information out on our own */
		/* Check for special cartridge features (new routine, courtesy of Omar Cornut, from MEKA)  */
		if (size >= 0x8000)
		{
			int c0002 = 0, c8000 = 0, cA000 = 0, cFFFF = 0, i;
			for (i = 0; i < 0x8000; i++)
			{
				if (state->m_cartridge[index].ROM[i] == 0x32) // Z80 opcode for: LD (xxxx), A
				{
					UINT16 addr = (state->m_cartridge[index].ROM[i + 2] << 8) | state->m_cartridge[index].ROM[i + 1];
					if (addr == 0xFFFF)
					{ i += 2; cFFFF++; continue; }
					if (addr == 0x0002 || addr == 0x0003 || addr == 0x0004)
					{ i += 2; c0002++; continue; }
					if (addr == 0x8000)
					{ i += 2; c8000++; continue; }
					if (addr == 0xA000)
					{ i += 2; cA000++; continue; }
				}
			}

			LOG(("Mapper test: c002 = %d, c8000 = %d, cA000 = %d, cFFFF = %d\n", c0002, c8000, cA000, cFFFF));

			// 2 is a security measure, although tests on existing ROM showed it was not needed
			if (c0002 > cFFFF + 2 || (c0002 > 0 && cFFFF == 0))
				state->m_cartridge[index].features |= CF_KOREAN_ZEMINA_MAPPER;
			else if (c8000 > cFFFF + 2 || (c8000 > 0 && cFFFF == 0))
				state->m_cartridge[index].features |= CF_CODEMASTERS_MAPPER;
			else if (cA000 > cFFFF + 2 || (cA000 > 0 && cFFFF == 0))
				state->m_cartridge[index].features |= CF_KOREAN_MAPPER;

		}

		/* Check for special SMS Compatibility mode gamegear cartridges */
		if (state->m_is_gamegear && image.software_entry() == NULL)	// not sure about how to handle this with softlists
		{
			/* Just in case someone passes us an sms file */
			if (!mame_stricmp (image.filetype(), "sms"))
				state->m_cartridge[index].features |= CF_GG_SMS_MODE;
		}
	}

	if (state->m_cartridge[index].features & CF_CODEMASTERS_MAPPER)
	{
		state->m_cartridge[index].ram_size = 0x10000;
		state->m_cartridge[index].cartRAM = auto_alloc_array(machine, UINT8, state->m_cartridge[index].ram_size);
		state->m_cartridge[index].ram_page = 0;
	}

	/* For Light Phaser games, we have to detect the x offset */
	state->m_lphaser_x_offs = detect_lphaser_xoffset(machine, state->m_cartridge[index].ROM);

	/* Terebi Oekaki (TV Draw) is a SG1000 game with special input device which is compatible with SG1000 Mark III */
	if ((detect_tvdraw(state->m_cartridge[index].ROM)) && state->m_is_region_japan)
	{
		address_space *program = machine.device("maincpu")->memory().space(AS_PROGRAM);
		program->install_legacy_write_handler(0x6000, 0x6000, FUNC(sms_tvdraw_axis_w));
		program->install_legacy_read_handler(0x8000, 0x8000, FUNC(sms_tvdraw_status_r));
		program->install_legacy_read_handler(0xa000, 0xa000, FUNC(sms_tvdraw_data_r));
		program->nop_write(0xa000, 0xa000);
	}

	if (state->m_cartridge[index].features & CF_KOREAN_NOBANK_MAPPER)
	{
		// FIXME: we should have by default FFFD-FFFF to be only a mirror for DFFD-DFFF (with no bankswitch logic)
		// and then the handlers should be installed here for all but the KOREAN_NOBANK carts
		// However, to avoid memory map breakage, we currently go the other way around
		address_space *program = machine.device("maincpu")->memory().space(AS_PROGRAM);
		program->install_legacy_readwrite_handler(0xfffc, 0xffff, FUNC(sms_kor_nobank_r), FUNC(sms_kor_nobank_w));
	}

	LOG(("Cart Features: %x\n", state->m_cartridge[index].features));

	/* Load battery backed RAM, if available */
	image.battery_load(state->m_cartridge[index].cartSRAM, sizeof(UINT8) * NVRAM_SIZE, 0x00);

	return IMAGE_INIT_PASS;
}


static void setup_cart_banks( running_machine &machine )
{
	sms_state *state = machine.driver_data<sms_state>();
	if (state->m_cartridge[state->m_current_cartridge].ROM)
	{
		UINT8 rom_page_count = state->m_cartridge[state->m_current_cartridge].size / 0x4000;
		state->m_banking_cart[1] = state->m_cartridge[state->m_current_cartridge].ROM;
		state->m_banking_cart[2] = state->m_cartridge[state->m_current_cartridge].ROM + 0x0400;
		state->m_banking_cart[3] = state->m_cartridge[state->m_current_cartridge].ROM + ((1 < rom_page_count) ? 0x4000 : 0);
		state->m_banking_cart[5] = state->m_cartridge[state->m_current_cartridge].ROM + ((2 < rom_page_count) ? 0x8000 : 0);
		/* Codemasters mapper points to bank 0 for page 2 */
		if (state->m_cartridge[state->m_current_cartridge].features & CF_CODEMASTERS_MAPPER)
		{
			state->m_banking_cart[5] = state->m_cartridge[state->m_current_cartridge].ROM;
		}
	}
	else
	{
		state->m_banking_cart[1] = state->m_banking_none[1];
		state->m_banking_cart[2] = state->m_banking_none[2];
		state->m_banking_cart[3] = state->m_banking_none[3];
		state->m_banking_cart[5] = state->m_banking_none[5];
	}
}

#ifdef MESS
static void setup_banks( running_machine &machine )
{
	sms_state *state = machine.driver_data<sms_state>();
	UINT8 *mem = machine.region("maincpu")->base();
	state->m_banking_bios[1] = state->m_banking_cart[1] = state->m_banking_none[1] = mem;
	state->m_banking_bios[2] = state->m_banking_cart[2] = state->m_banking_none[2] = mem;
	state->m_banking_bios[3] = state->m_banking_cart[3] = state->m_banking_none[3] = mem;
	state->m_banking_bios[4] = state->m_banking_cart[4] = state->m_banking_none[4] = mem;
	state->m_banking_bios[5] = state->m_banking_cart[5] = state->m_banking_none[5] = mem;
	state->m_banking_bios[6] = state->m_banking_cart[6] = state->m_banking_none[6] = mem;

	state->m_BIOS = machine.region("user1")->base();

	state->m_bios_page_count = (state->m_BIOS ? machine.region("user1")->bytes() / 0x4000 : 0);

	setup_cart_banks(machine);

	if (state->m_BIOS == NULL || state->m_BIOS[0] == 0x00)
	{
		state->m_BIOS = NULL;
		state->m_bios_port |= IO_BIOS_ROM;
	}

	if (state->m_BIOS)
	{
		state->m_banking_bios[1] = state->m_BIOS;
		state->m_banking_bios[2] = state->m_BIOS + 0x0400;
		state->m_banking_bios[3] = state->m_BIOS + ((1 < state->m_bios_page_count) ? 0x4000 : 0);
		state->m_banking_bios[5] = state->m_BIOS + ((2 < state->m_bios_page_count) ? 0x8000 : 0);
	}
}
#endif

MACHINE_START( sms )
{
	sms_state *state = machine.driver_data<sms_state>();

	machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(sms_machine_stop),&machine));
	state->m_rapid_fire_timer = machine.scheduler().timer_alloc(FUNC(rapid_fire_callback));
	state->m_rapid_fire_timer->adjust(attotime::from_hz(10), 0, attotime::from_hz(10));

	state->m_lphaser_1_timer = machine.scheduler().timer_alloc(FUNC(lphaser_1_callback));
	state->m_lphaser_2_timer = machine.scheduler().timer_alloc(FUNC(lphaser_2_callback));

	state->m_main_cpu = machine.device("maincpu");
	state->m_control_cpu = machine.device("control");
	state->m_vdp = machine.device("sms_vdp");
	state->m_ym = machine.device("ym2413");
	state->m_main_scr = machine.device("screen");
	state->m_left_lcd = machine.device("left_lcd");
	state->m_right_lcd = machine.device("right_lcd");

	/* Check if lightgun has been chosen as input: if so, enable crosshair */
	machine.scheduler().timer_set(attotime::zero, FUNC(lightgun_tick));
}

#ifdef MESS
MACHINE_RESET( sms )
{
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);
	sms_state *state = machine.driver_data<sms_state>();

	state->m_ctrl_reg = 0xff;
	if (state->m_has_fm)
		state->m_fm_detect = 0x01;

	state->m_mapper_ram = (UINT8*)space->get_write_ptr(0xdffc);

	state->m_bios_port = 0;

	if (state->m_cartridge[state->m_current_cartridge].features & CF_CODEMASTERS_MAPPER)
	{
		/* Install special memory handlers */
		space->install_legacy_write_handler(0x0000, 0x0000, FUNC(sms_codemasters_page0_w));
		space->install_legacy_write_handler(0x4000, 0x4000, FUNC(sms_codemasters_page1_w));
	}

	if (state->m_cartridge[state->m_current_cartridge].features & CF_KOREAN_ZEMINA_MAPPER)
		space->install_legacy_write_handler(0x0000, 0x0003, FUNC(sms_korean_zemina_banksw_w));

	if (state->m_cartridge[state->m_current_cartridge].features & CF_GG_SMS_MODE)
		sms_vdp_set_ggsmsmode(state->m_vdp, 1);

	/* Initialize SIO stuff for GG */
	state->m_gg_sio[0] = 0x7f;
	state->m_gg_sio[1] = 0xff;
	state->m_gg_sio[2] = 0x00;
	state->m_gg_sio[3] = 0xff;
	state->m_gg_sio[4] = 0x00;

	state->m_store_control = 0;

	setup_banks(machine);

	setup_rom(space);

	state->m_rapid_fire_state_1 = 0;
	state->m_rapid_fire_state_2 = 0;

	state->m_last_paddle_read_time = 0;
	state->m_paddle_read_state = 0;

	state->m_last_sports_pad_time_1 = 0;
	state->m_last_sports_pad_time_2 = 0;
	state->m_sports_pad_state_1 = 0;
	state->m_sports_pad_state_2 = 0;
	state->m_sports_pad_last_data_1 = 0;
	state->m_sports_pad_last_data_2 = 0;
	state->m_sports_pad_1_x = 0;
	state->m_sports_pad_1_y = 0;
	state->m_sports_pad_2_x = 0;
	state->m_sports_pad_2_y = 0;

	state->m_lphaser_1_latch = 0;
	state->m_lphaser_2_latch = 0;

	state->m_sscope_state = 0;

	state->m_tvdraw_data = 0;
}
#endif

READ8_HANDLER( sms_store_cart_select_r )
{
	return 0xff;
}


WRITE8_HANDLER( sms_store_cart_select_w )
{
	sms_state *state = space->machine().driver_data<sms_state>();
	UINT8 slot = data >> 4;
	UINT8 slottype = data & 0x08;

	logerror("switching in part of %s slot #%d\n", slottype ? "card" : "cartridge", slot );
	/* cartridge? slot #0 */
	if (slottype == 0)
		state->m_current_cartridge = slot;

	setup_cart_banks(space->machine());
	memory_set_bankptr(space->machine(), "bank10", state->m_banking_cart[3] + 0x2000);
	setup_rom(space);
}


READ8_HANDLER( sms_store_select1 )
{
	return 0xff;
}


READ8_HANDLER( sms_store_select2 )
{
	return 0xff;
}


READ8_HANDLER( sms_store_control_r )
{
	sms_state *state = space->machine().driver_data<sms_state>();
	return state->m_store_control;
}


WRITE8_HANDLER( sms_store_control_w )
{
	sms_state *state = space->machine().driver_data<sms_state>();
	logerror("0x%04X: sms_store_control write 0x%02X\n", cpu_get_pc(&space->device()), data);
	if (data & 0x02)
	{
		space->machine().device<cpu_device>("maincpu")->resume(SUSPEND_REASON_HALT);
	}
	else
	{
		/* Pull reset line of CPU #0 low */
		space->machine().device("maincpu")->reset();
		space->machine().device<cpu_device>("maincpu")->suspend(SUSPEND_REASON_HALT, 1);
	}
	state->m_store_control = data;
}

void sms_store_int_callback( running_machine &machine, int state )
{
	sms_state *driver_state = machine.driver_data<sms_state>();
	device_set_input_line(driver_state->m_store_control & 0x01 ? driver_state->m_control_cpu : driver_state->m_main_cpu, 0, state);
}


static void sms_set_zero_flag( running_machine &machine )
{
	sms_state *state = machine.driver_data<sms_state>();
	state->m_is_gamegear = 0;
	state->m_is_region_japan = 0;
	state->m_has_bios_0400 = 0;
	state->m_has_bios_2000 = 0;
	state->m_has_bios_full = 0;
	state->m_has_bios = 0;
	state->m_has_fm = 0;
}

DRIVER_INIT( sg1000m3 )
{
	sms_state *state = machine.driver_data<sms_state>();
	sms_set_zero_flag(machine);
	state->m_is_region_japan = 1;
	state->m_has_fm = 1;
}


DRIVER_INIT( sms1 )
{
	sms_state *state = machine.driver_data<sms_state>();
	sms_set_zero_flag(machine);
	state->m_has_bios_full = 1;
}


DRIVER_INIT( smsj )
{
	sms_state *state = machine.driver_data<sms_state>();
	sms_set_zero_flag(machine);
	state->m_is_region_japan = 1;
	state->m_has_bios_2000 = 1;
	state->m_has_fm = 1;
}


DRIVER_INIT( sms2kr )
{
	sms_state *state = machine.driver_data<sms_state>();
	sms_set_zero_flag(machine);
	state->m_is_region_japan = 1;
	state->m_has_bios_full = 1;
	state->m_has_fm = 1;
}


DRIVER_INIT( smssdisp )
{
	sms_set_zero_flag(machine);
}


DRIVER_INIT( gamegear )
{
	sms_state *state = machine.driver_data<sms_state>();
	sms_set_zero_flag(machine);
	state->m_is_gamegear = 1;
}


DRIVER_INIT( gamegeaj )
{
	sms_state *state = machine.driver_data<sms_state>();
	sms_set_zero_flag(machine);
	state->m_is_region_japan = 1;
	state->m_is_gamegear = 1;
	state->m_has_bios_0400 = 1;
}


static void sms_black_bitmap( const screen_device *screen, bitmap_t *bitmap )
{
	const int width = screen->width();
	const int height = screen->height();
	int x, y;

	for (y = 0; y < height; y++)
		for (x = 0; x < width; x++)
			*BITMAP_ADDR32(bitmap, y, x) = MAKE_RGB(0,0,0);
}

VIDEO_START( sms1 )
{
	sms_state *state = machine.driver_data<sms_state>();
	screen_device *screen = machine.first_screen();
	int width = screen->width();
	int height = screen->height();

	state->m_prevleft_bitmap = auto_bitmap_alloc(machine, width, height, BITMAP_FORMAT_INDEXED32);
	state->m_prevright_bitmap = auto_bitmap_alloc(machine, width, height, BITMAP_FORMAT_INDEXED32);
	state->save_item(NAME(*state->m_prevleft_bitmap));
	state->save_item(NAME(*state->m_prevright_bitmap));
}

SCREEN_UPDATE( sms1 )
{
	sms_state *state = screen->machine().driver_data<sms_state>();
	UINT8 sscope = input_port_read_safe(screen->machine(), "SEGASCOPE", 0x00);
	UINT8 sscope_binocular_hack = input_port_read_safe(screen->machine(), "SSCOPE_BINOCULAR", 0x00);
	UINT8 occluded_view = 0;

	// without SegaScope, both LCDs for glasses go black
	if ((screen != state->m_main_scr) && !sscope)
		occluded_view = 1;

	// with SegaScope, sscope_state 0 = left screen OFF, right screen ON
	if (!(state->m_sscope_state & 0x01) && (screen == state->m_left_lcd))
		occluded_view = 1;

	// with SegaScope, sscope_state 1 = left screen ON, right screen OFF
	if ((state->m_sscope_state & 0x01) && (screen == state->m_right_lcd))
		occluded_view = 1;

	if (!occluded_view)
	{
		sms_vdp_update(state->m_vdp, bitmap, cliprect);

		// HACK: fake 3D->2D handling (if enabled, it repeats each frame twice on the selected lens)
		// save a copy of current bitmap for the binocular hack
		if (sscope && (screen == state->m_left_lcd) && (sscope_binocular_hack & 0x01))
			copybitmap(state->m_prevleft_bitmap, bitmap, 0, 0, 0, 0, cliprect);
		if (sscope && (screen == state->m_right_lcd) && (sscope_binocular_hack & 0x02))
			copybitmap(state->m_prevright_bitmap, bitmap, 0, 0, 0, 0, cliprect);
	}
	else
	{
		sms_black_bitmap(screen, bitmap);

		// HACK: fake 3D->2D handling (if enabled, it repeats each frame twice on the selected lens)
		// use the copied bitmap for the binocular hack
		if (sscope && (screen == state->m_left_lcd) && (sscope_binocular_hack & 0x01))
			copybitmap(bitmap, state->m_prevleft_bitmap, 0, 0, 0, 0, cliprect);
		if (sscope && (screen == state->m_right_lcd) && (sscope_binocular_hack & 0x02))
			copybitmap(bitmap, state->m_prevright_bitmap, 0, 0, 0, 0, cliprect);
	}

	return 0;
}

SCREEN_UPDATE( sms )
{
	sms_state *state = screen->machine().driver_data<sms_state>();
	sms_vdp_update(state->m_vdp, bitmap, cliprect);
	return 0;
}

VIDEO_START( gamegear )
{
	sms_state *state = machine.driver_data<sms_state>();
	screen_device *screen = machine.first_screen();
	int width = screen->width();
	int height = screen->height();

	state->m_prev_bitmap = auto_bitmap_alloc(machine, width, height, BITMAP_FORMAT_INDEXED32);
	state->m_tmp_bitmap = auto_bitmap_alloc(machine, width, height, BITMAP_FORMAT_INDEXED32);
	state->save_item(NAME(*state->m_prev_bitmap));
}

SCREEN_UPDATE( gamegear )
{
	sms_state *state = screen->machine().driver_data<sms_state>();
	int width = screen->width();
	int height = screen->height();
	int x, y;

	sms_vdp_update(state->m_vdp, state->m_tmp_bitmap, cliprect);

	// HACK: fake LCD persistence effect
	// (it would be better to generalize this in the core, to be used for all LCD systems)
	for (y = 0; y < height; y++)
	{
		UINT32 *line0 = BITMAP_ADDR32(state->m_tmp_bitmap, y, 0);
		UINT32 *line1 = BITMAP_ADDR32(state->m_prev_bitmap, y, 0);
		for (x = 0; x < width; x++)
		{
			UINT32 color0 = line0[x];
			UINT32 color1 = line1[x];
			UINT16 r0 = (color0 >> 16) & 0x000000ff;
			UINT16 g0 = (color0 >>  8) & 0x000000ff;
			UINT16 b0 = (color0 >>  0) & 0x000000ff;
			UINT16 r1 = (color1 >> 16) & 0x000000ff;
			UINT16 g1 = (color1 >>  8) & 0x000000ff;
			UINT16 b1 = (color1 >>  0) & 0x000000ff;
			UINT8 r = (UINT8)((r0 + r1) >> 1);
			UINT8 g = (UINT8)((g0 + g1) >> 1);
			UINT8 b = (UINT8)((b0 + b1) >> 1);
			*BITMAP_ADDR32(bitmap, y, x) = (r << 16) | (g << 8) | b;
		}
	}
	copybitmap(state->m_prev_bitmap, state->m_tmp_bitmap, 0, 0, 0, 0, cliprect);

	return 0;
}
