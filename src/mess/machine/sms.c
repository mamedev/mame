#include "emu.h"
#include "crsshair.h"
#include "video/315_5124.h"
#include "sound/2413intf.h"
#include "imagedev/cartslot.h"
#include "machine/eeprom.h"
#include "includes/sms.h"

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

#define CF_CODEMASTERS_MAPPER      0x001
#define CF_KOREAN_MAPPER           0x002
#define CF_KOREAN_ZEMINA_MAPPER    0x004
#define CF_KOREAN_NOBANK_MAPPER    0x008
#define CF_93C46_EEPROM            0x010
#define CF_ONCART_RAM              0x020
#define CF_GG_SMS_MODE             0x040
#define CF_KOREAN_ZEMINA_NEMESIS   0x080
#define CF_4PAK_MAPPER             0x100
#define CF_JANGGUN_MAPPER          0x200
#define CF_TVDRAW                  0x400
#define CF_MAPPER_BITS             ( CF_CODEMASTERS_MAPPER | CF_KOREAN_MAPPER | CF_KOREAN_ZEMINA_MAPPER | \
                                     CF_KOREAN_NOBANK_MAPPER | CF_4PAK_MAPPER | CF_JANGGUN_MAPPER )

#define LGUN_RADIUS           6
#define LGUN_X_INTERVAL       4


static void setup_rom(address_space &space);


TIMER_CALLBACK_MEMBER(sms_state::rapid_fire_callback)
{
	m_rapid_fire_state_1 ^= 0xff;
	m_rapid_fire_state_2 ^= 0xff;
}


void sms_state::map_cart_16k( UINT16 address, UINT16 bank )
{
	switch ( address )
	{
	case 0x0000:
		map_cart_8k( 0x0000, bank * 2 );
		map_cart_8k( 0x2000, bank * 2 + 1 );
		break;

	case 0x0400:
		map_cart_8k( 0x0400, bank * 2 );
		map_cart_8k( 0x2000, bank * 2 + 1 );
		break;

	case 0x4000:
		map_cart_8k( 0x4000, bank * 2 );
		map_cart_8k( 0x6000, bank * 2 + 1 );
		break;

	case 0x8000:
		map_cart_8k( 0x8000, bank * 2 );
		map_cart_8k( 0xA000, bank * 2 + 1 );
		break;

	default:
		fatalerror("map_cart_16k: Unsupported map address %04x passed\n", address);
		break;
	}
}


void sms_state::map_cart_8k( UINT16 address, UINT16 bank )
{
	UINT8 *bank_start = m_banking_none;

	if ( m_cartridge[m_current_cartridge].ROM )
	{
		UINT16 rom_bank_count = m_cartridge[m_current_cartridge].size / 0x2000;
		bank_start = m_cartridge[m_current_cartridge].ROM + ((rom_bank_count > 0) ? bank % rom_bank_count : 0) * 0x2000;
	}

	switch ( address )
	{
	case 0x0000:
		m_banking_cart[1] = bank_start;
		m_banking_cart[2] = bank_start + 0x400;
		membank("bank1")->set_base(m_banking_cart[1]);
		membank("bank2")->set_base(m_banking_cart[2]);
		break;

	case 0x0400:
		m_banking_cart[2] = bank_start + 0x400;
		membank("bank2")->set_base(m_banking_cart[2]);
		break;

	case 0x2000:
		m_banking_cart[7] = bank_start;
		membank("bank7")->set_base(m_banking_cart[7]);
		break;

	case 0x4000:
		m_banking_cart[3] = bank_start;
		membank("bank3")->set_base(m_banking_cart[3]);
		break;

	case 0x6000:
		m_banking_cart[4] = bank_start;
		membank("bank4")->set_base(m_banking_cart[4]);
		break;

	case 0x8000:
		m_banking_cart[5] = bank_start;
		membank("bank5")->set_base(m_banking_cart[5]);
		break;

	case 0xA000:
		m_banking_cart[6] = bank_start;
		membank("bank6")->set_base(m_banking_cart[6]);
		break;

	default:
		fatalerror("map_cart_8k: Unsuppored map address %04x passed\n", address);
		break;
	}
}


void sms_state::map_bios_16k( UINT16 address, UINT16 bank)
{
	switch ( address )
	{
	case 0x0000:
		map_bios_8k( 0x0000, bank * 2 );
		map_bios_8k( 0x2000, bank * 2 + 1 );
		break;

	case 0x0400:
		map_bios_8k( 0x0400, bank * 2 );
		map_bios_8k( 0x2000, bank * 2 + 1 );
		break;

	case 0x4000:
		map_bios_8k( 0x4000, bank * 2 );
		map_bios_8k( 0x6000, bank * 2 + 1 );
		break;

	case 0x8000:
		map_bios_8k( 0x8000, bank * 2 );
		map_bios_8k( 0xA000, bank * 2 + 1 );
		break;

	default:
		fatalerror("map_bios_16k: Unsupported map address %04x passed\n", address);
		break;
	}
}


void sms_state::map_bios_8k( UINT16 address, UINT16 bank )
{
	UINT8 *bank_start = m_banking_none;

	if ( m_BIOS )
	{
		bank_start = m_BIOS + ((m_bios_page_count > 0) ? bank % ( m_bios_page_count << 1 ) : 0) * 0x2000;
	}

	switch ( address )
	{
	case 0x0000:
		m_banking_bios[1] = bank_start;
		membank("bank1")->set_base(m_banking_bios[1]);
		break;

	case 0x0400:
		if ( m_has_bios_0400 )
		{
			break;
		}
		m_banking_bios[2] = bank_start + 0x400;
		membank("bank2")->set_base(m_banking_bios[2]);
		break;

	case 0x2000:
		if ( m_has_bios_0400 || m_has_bios_2000 )
		{
			break;
		}
		m_banking_bios[7] = bank_start;
		membank("bank7")->set_base(m_banking_bios[7]);
		break;

	case 0x4000:
		if ( m_has_bios_0400 || m_has_bios_2000 )
		{
			break;
		}
		m_banking_bios[3] = bank_start;
		membank("bank3")->set_base(m_banking_bios[3]);
		break;

	case 0x6000:
		if ( m_has_bios_0400 || m_has_bios_2000 )
		{
			break;
		}
		m_banking_bios[4] = bank_start;
		membank("bank4")->set_base(m_banking_bios[4]);
		break;

	case 0x8000:
		if ( m_has_bios_0400 || m_has_bios_2000 )
		{
			break;
		}
		m_banking_bios[5] = bank_start;
		membank("bank5")->set_base(m_banking_bios[5]);
		break;

	case 0xA000:
		if ( m_has_bios_0400 || m_has_bios_2000 )
		{
			break;
		}
		m_banking_bios[6] = bank_start;
		membank("bank6")->set_base(m_banking_bios[6]);
		break;

	default:
		fatalerror("map_cart_8k: Unsuppored map address %04x passed\n", address);
		break;
	}
}


WRITE8_MEMBER(sms_state::sms_input_write)
{

	switch (offset)
	{
	case 0:
		switch (ioport("CTRLSEL")->read_safe(0x00) & 0x0f)
		{
		case 0x04:	/* Sports Pad */
			if (data != m_sports_pad_last_data_1)
			{
				UINT32 cpu_cycles = downcast<cpu_device *>(&space.device())->total_cycles();

				m_sports_pad_last_data_1 = data;
				if (cpu_cycles - m_last_sports_pad_time_1 > 512)
				{
					m_sports_pad_state_1 = 3;
					m_sports_pad_1_x = ioport("SPORT0")->read();
					m_sports_pad_1_y = ioport("SPORT1")->read();
				}
				m_last_sports_pad_time_1 = cpu_cycles;
				m_sports_pad_state_1 = (m_sports_pad_state_1 + 1) & 3;
			}
			break;
		}
		break;

	case 1:
		switch (ioport("CTRLSEL")->read_safe(0x00) & 0xf0)
		{
		case 0x40:	/* Sports Pad */
			if (data != m_sports_pad_last_data_2)
			{
				UINT32 cpu_cycles = downcast<cpu_device *>(&space.device())->total_cycles();

				m_sports_pad_last_data_2 = data;
				if (cpu_cycles - m_last_sports_pad_time_2 > 2048)
				{
					m_sports_pad_state_2 = 3;
					m_sports_pad_2_x = ioport("SPORT2")->read();
					m_sports_pad_2_y = ioport("SPORT3")->read();
				}
				m_last_sports_pad_time_2 = cpu_cycles;
				m_sports_pad_state_2 = (m_sports_pad_state_2 + 1) & 3;
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
	state->m_vdp->hcount_latch_write(*state->m_space, 0, tmp);
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
			bitmap_rgb32 &bitmap = state->m_vdp->get_bitmap();

			/* brightness of the lightgray color in the frame drawn by Light Phaser games */
			const UINT8 sensor_min_brightness = 0x7f;

			/* TODO: Check how Light Phaser behaves for border areas. For Gangster Town, should */
			/* a shot at right border (HC~=0x90) really appear at active scr, near to left border? */
			if (beam_x < SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH || beam_x >= SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH + 256)
				return 0;

			rgb_t color = bitmap.pix32(beam_y, beam_x);

			/* reference: http://www.w3.org/TR/AERT#color-contrast */
			UINT8 brightness = (RGB_RED(color) * 0.299) + (RGB_GREEN(color) * 0.587) + (RGB_BLUE(color) * 0.114);
			//printf ("color brightness: %2X for x %d y %d\n", brightness, beam_x, beam_y);

			result = (brightness >= sensor_min_brightness) ? 1 : 0;

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


static void sms_vdp_hcount_latch( address_space &space )
{
	sms_state *state = space.machine().driver_data<sms_state>();
	UINT8 value = sms_vdp_hcount(space.machine());

	state->m_vdp->hcount_latch_write(space, 0, value);
}


static UINT16 screen_hpos_nonscaled( screen_device &screen, int scaled_hpos )
{
	const rectangle &visarea = screen.visible_area();
	int offset_x = (scaled_hpos * visarea.width()) / 255;
	return visarea.min_x + offset_x;
}


static UINT16 screen_vpos_nonscaled( screen_device &screen, int scaled_vpos )
{
	const rectangle &visarea = screen.visible_area();
	int offset_y = (scaled_vpos * (visarea.max_y - visarea.min_y)) / 255;
	return visarea.min_y + offset_y;
}


static void lphaser1_sensor_check( running_machine &machine )
{
	sms_state *state = machine.driver_data<sms_state>();
	const int x = screen_hpos_nonscaled(*machine.first_screen(), machine.root_device().ioport("LPHASER0")->read());
	const int y = screen_vpos_nonscaled(*machine.first_screen(), machine.root_device().ioport("LPHASER1")->read());

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
	const int x = screen_hpos_nonscaled(*machine.first_screen(), machine.root_device().ioport("LPHASER2")->read());
	const int y = screen_vpos_nonscaled(*machine.first_screen(), machine.root_device().ioport("LPHASER3")->read());

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
TIMER_CALLBACK_MEMBER(sms_state::lightgun_tick)
{

	if ((ioport("CTRLSEL")->read_safe(0x00) & 0x0f) == 0x01)
	{
		/* enable crosshair */
		crosshair_set_screen(machine(), 0, CROSSHAIR_SCREEN_ALL);
		if (!m_lphaser_1_timer->enabled())
			lphaser1_sensor_check(machine());
	}
	else
	{
		/* disable crosshair */
		crosshair_set_screen(machine(), 0, CROSSHAIR_SCREEN_NONE);
		m_lphaser_1_timer->enable(0);
	}

	if ((ioport("CTRLSEL")->read_safe(0x00) & 0xf0) == 0x10)
	{
		/* enable crosshair */
		crosshair_set_screen(machine(), 1, CROSSHAIR_SCREEN_ALL);
		if (!m_lphaser_2_timer->enabled())
			lphaser2_sensor_check(machine());
	}
	else
	{
		/* disable crosshair */
		crosshair_set_screen(machine(), 1, CROSSHAIR_SCREEN_NONE);
		m_lphaser_2_timer->enable(0);
	}
}


TIMER_CALLBACK_MEMBER(sms_state::lphaser_1_callback)
{
	lphaser1_sensor_check(machine());
}


TIMER_CALLBACK_MEMBER(sms_state::lphaser_2_callback)
{
	lphaser2_sensor_check(machine());
}


INPUT_CHANGED_MEMBER(sms_state::lgun1_changed)
{
	if (!m_lphaser_1_timer ||
		(machine().root_device().ioport("CTRLSEL")->read_safe(0x00) & 0x0f) != 0x01)
		return;

	if (newval != oldval)
		lphaser1_sensor_check(machine());
}

INPUT_CHANGED_MEMBER(sms_state::lgun2_changed)
{
	if (!m_lphaser_2_timer ||
		(machine().root_device().ioport("CTRLSEL")->read_safe(0x00) & 0xf0) != 0x10)
		return;

	if (newval != oldval)
		lphaser2_sensor_check(machine());
}


static void sms_get_inputs( address_space &space )
{
	sms_state *state = space.machine().driver_data<sms_state>();
	UINT8 data = 0x00;
	UINT32 cpu_cycles = downcast<cpu_device *>(&space.device())->total_cycles();
	running_machine &machine = space.machine();

	state->m_input_port0 = 0xff;
	state->m_input_port1 = 0xff;

	if (cpu_cycles - state->m_last_paddle_read_time > 256)
	{
		state->m_paddle_read_state ^= 0xff;
		state->m_last_paddle_read_time = cpu_cycles;
	}

	/* Check if lightgun has been chosen as input: if so, enable crosshair */
	machine.scheduler().timer_set(attotime::zero, timer_expired_delegate(FUNC(sms_state::lightgun_tick),state));

	/* Player 1 */
	switch (machine.root_device().ioport("CTRLSEL")->read_safe(0x00) & 0x0f)
	{
	case 0x00:  /* Joystick */
		data = machine.root_device().ioport("PORT_DC")->read();
		/* Check Rapid Fire setting for Button A */
		if (!(data & 0x10) && (machine.root_device().ioport("RFU")->read() & 0x01))
			data |= state->m_rapid_fire_state_1 & 0x10;

		/* Check Rapid Fire setting for Button B */
		if (!(data & 0x20) && (machine.root_device().ioport("RFU")->read() & 0x02))
			data |= state->m_rapid_fire_state_1 & 0x20;

		state->m_input_port0 = (state->m_input_port0 & 0xc0) | (data & 0x3f);
		break;

	case 0x01:  /* Light Phaser */
		data = (machine.root_device().ioport("CTRLIPT")->read() & 0x01) << 4;
		if (!(data & 0x10))
		{
			if (machine.root_device().ioport("RFU")->read() & 0x01)
				data |= state->m_rapid_fire_state_1 & 0x10;
		}
		/* just consider the button (trigger) bit */
		data |= ~0x10;
		state->m_input_port0 = (state->m_input_port0 & 0xc0) | (data & 0x3f);
		break;

	case 0x02:  /* Paddle Control */
		/* Get button A state */
		data = machine.root_device().ioport("PADDLE0")->read();

		if (state->m_paddle_read_state)
			data = data >> 4;

		state->m_input_port0 = (state->m_input_port0 & 0xc0) | (data & 0x0f) | (state->m_paddle_read_state & 0x20)
		                | ((machine.root_device().ioport("CTRLIPT")->read() & 0x02) << 3);
		break;

	case 0x04:	/* Sega Sports Pad */
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
		state->m_input_port0 = (state->m_input_port0 & 0xc0) | data | ((machine.root_device().ioport("CTRLIPT")->read() & 0x0c) << 2);
		break;
	}

	/* Player 2 */
	switch (machine.root_device().ioport("CTRLSEL")->read_safe(0x00)  & 0xf0)
	{
	case 0x00:	/* Joystick */
		data = machine.root_device().ioport("PORT_DC")->read();
		state->m_input_port0 = (state->m_input_port0 & 0x3f) | (data & 0xc0);

		data = machine.root_device().ioport("PORT_DD")->read();
		/* Check Rapid Fire setting for Button A */
		if (!(data & 0x04) && (machine.root_device().ioport("RFU")->read() & 0x04))
			data |= state->m_rapid_fire_state_2 & 0x04;

		/* Check Rapid Fire setting for Button B */
		if (!(data & 0x08) && (machine.root_device().ioport("RFU")->read() & 0x08))
			data |= state->m_rapid_fire_state_2 & 0x08;

		state->m_input_port1 = (state->m_input_port1 & 0xf0) | (data & 0x0f);
		break;

	case 0x10:	/* Light Phaser */
		data = (machine.root_device().ioport("CTRLIPT")->read() & 0x10) >> 2;
		if (!(data & 0x04))
		{
			if (machine.root_device().ioport("RFU")->read() & 0x04)
				data |= state->m_rapid_fire_state_2 & 0x04;
		}
		/* just consider the button (trigger) bit */
		data |= ~0x04;
		state->m_input_port1 = (state->m_input_port1 & 0xf0) | (data & 0x0f);
		break;

	case 0x20:	/* Paddle Control */
		/* Get button A state */
		data = machine.root_device().ioport("PADDLE1")->read();
		if (state->m_paddle_read_state)
			data = data >> 4;

		state->m_input_port0 = (state->m_input_port0 & 0x3f) | ((data & 0x03) << 6);
		state->m_input_port1 = (state->m_input_port1 & 0xf0) | ((data & 0x0c) >> 2) | (state->m_paddle_read_state & 0x08)
		                | ((machine.root_device().ioport("CTRLIPT")->read() & 0x20) >> 3);
		break;

	case 0x40:	/* Sega Sports Pad */
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
		state->m_input_port1 = (state->m_input_port1 & 0xf0) | (data >> 2) | ((machine.root_device().ioport("CTRLIPT")->read() & 0xc0) >> 4);
		break;
	}
}


WRITE8_MEMBER(sms_state::sms_fm_detect_w)
{

	if (m_has_fm)
		m_fm_detect = (data & 0x01);
}


READ8_MEMBER(sms_state::sms_fm_detect_r)
{

	if (m_has_fm)
	{
		return m_fm_detect;
	}
	else
	{
		if (m_bios_port & IO_CHIP)
		{
			return 0xff;
		}
		else
		{
			sms_get_inputs(space);
			return m_input_port0;
		}
	}
}

WRITE8_MEMBER(sms_state::sms_io_control_w)
{
	bool hcount_latch = false;

	if (data & 0x08)
	{
		/* check if TH pin level is high (1) and was low last time */
		if (data & 0x80 && !(m_ctrl_reg & 0x80))
		{
			hcount_latch = true;
		}
		sms_input_write(space, 0, (data & 0x20) >> 5);
	}

	if (data & 0x02)
	{
		if (data & 0x20 && !(m_ctrl_reg & 0x20))
		{
			hcount_latch = true;
		}
		sms_input_write(space, 1, (data & 0x80) >> 7);
	}

	if (hcount_latch)
	{
		sms_vdp_hcount_latch(space);
	}

	m_ctrl_reg = data;
}


READ8_MEMBER(sms_state::sms_count_r)
{

	if (offset & 0x01)
		return m_vdp->hcount_latch_read(*m_space, offset);
	else
		return m_vdp->vcount_read(*m_space, offset);
}


/*
 Check if the pause button is pressed.
 If the gamegear is in sms mode, check if the start button is pressed.
 */
WRITE_LINE_MEMBER(sms_state::sms_pause_callback)
{
	if (m_is_gamegear && !(m_cartridge[m_current_cartridge].features & CF_GG_SMS_MODE))
		return;

	if (!(ioport(m_is_gamegear ? "START" : "PAUSE")->read() & 0x80))
	{
		if (!m_paused)
		{
			machine().device("maincpu")->execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
		}
		m_paused = 1;
	}
	else
	{
		m_paused = 0;
	}

	/* clear Light Phaser latch flags for next frame */
	m_lphaser_1_latch = 0;
	m_lphaser_2_latch = 0;
}

READ8_MEMBER(sms_state::sms_input_port_0_r)
{

	if (m_bios_port & IO_CHIP)
	{
		return 0xff;
	}
	else
	{
		sms_get_inputs(space);
		return m_input_port0;
	}
}


READ8_MEMBER(sms_state::sms_input_port_1_r)
{

	if (m_bios_port & IO_CHIP)
		return 0xff;

	sms_get_inputs(space);

	/* Reset Button */
	m_input_port1 = (m_input_port1 & 0xef) | (ioport("RESET")->read_safe(0x01) & 0x01) << 4;

	/* Do region detection if TH of ports A and B are set to output (0) */
	if (!(m_ctrl_reg & 0x0a))
	{
		/* Move bits 7,5 of IO control port into bits 7, 6 */
		m_input_port1 = (m_input_port1 & 0x3f) | (m_ctrl_reg & 0x80) | (m_ctrl_reg & 0x20) << 1;

		/* Inverse region detect value for Japanese machines */
		if (m_is_region_japan)
			m_input_port1 ^= 0xc0;
	}
	else
	{
		if (m_ctrl_reg & 0x02 && m_lphaser_1_latch)
		{
			m_input_port1 &= ~0x40;
			m_lphaser_1_latch = 0;
		}

		if (m_ctrl_reg & 0x08 && m_lphaser_2_latch)
		{
			m_input_port1 &= ~0x80;
			m_lphaser_2_latch = 0;
		}
	}

	return m_input_port1;
}



WRITE8_MEMBER(sms_state::sms_ym2413_register_port_0_w)
{

	if (m_has_fm)
		ym2413_w(m_ym, space, 0, (data & 0x3f));
}


WRITE8_MEMBER(sms_state::sms_ym2413_data_port_0_w)
{

	if (m_has_fm)
	{
		logerror("data_port_0_w %x %x\n", offset, data);
		ym2413_w(m_ym, space, 1, data);
	}
}


READ8_MEMBER(sms_state::gg_input_port_2_r)
{

	//logerror("joy 2 read, val: %02x, pc: %04x\n", ((m_is_region_japan ? 0x00 : 0x40) | (machine.root_device().ioport("START")->read() & 0x80)), activecpu_get_pc());
	return ((m_is_region_japan ? 0x00 : 0x40) | (ioport("START")->read() & 0x80));
}


READ8_MEMBER(sms_state::sms_sscope_r)
{
	return m_sscope_state;
}


WRITE8_MEMBER(sms_state::sms_sscope_w)
{
	m_sscope_state = data;
}


READ8_MEMBER(sms_state::sms_mapper_r)
{
	return m_mapper[offset];
}

/* Terebi Oekaki */
/* The following code comes from sg1000.c. We should eventually merge these TV Draw implementations */
WRITE8_MEMBER(sms_state::sms_tvdraw_axis_w)
{
	UINT8 tvboard_on = ioport("TVDRAW")->read_safe(0x00);

	if (data & 0x01)
	{
		m_cartridge[m_current_cartridge].m_tvdraw_data = tvboard_on ? ioport("TVDRAW_X")->read() : 0x80;

		if (m_cartridge[m_current_cartridge].m_tvdraw_data < 4) m_cartridge[m_current_cartridge].m_tvdraw_data = 4;
		if (m_cartridge[m_current_cartridge].m_tvdraw_data > 251) m_cartridge[m_current_cartridge].m_tvdraw_data = 251;
	}
	else
	{
		m_cartridge[m_current_cartridge].m_tvdraw_data = tvboard_on ? ioport("TVDRAW_Y")->read() + 0x20 : 0x80;
	}
}

READ8_MEMBER(sms_state::sms_tvdraw_status_r)
{
	UINT8 tvboard_on = ioport("TVDRAW")->read_safe(0x00);
	return tvboard_on ? ioport("TVDRAW_PEN")->read() : 0x01;
}

READ8_MEMBER(sms_state::sms_tvdraw_data_r)
{
	return m_cartridge[m_current_cartridge].m_tvdraw_data;
}


WRITE8_MEMBER(sms_state::sms_93c46_w)
{

	if ( m_cartridge[m_current_cartridge].m_93c46_enabled )
	{
		m_cartridge[m_current_cartridge].m_93c46_lines = data;

		logerror( "sms_93c46_w: setting eeprom lines: DI=%s CLK=%s CS=%s\n", data & 0x01 ? "1" : "0", data & 0x02 ? "1" : "0", data & 0x04 ? "1" : "0" );
		m_eeprom->write_bit( ( data & 0x01 ) ? ASSERT_LINE : CLEAR_LINE );
		m_eeprom->set_cs_line( !( data & 0x04 ) ? ASSERT_LINE : CLEAR_LINE );
		m_eeprom->set_clock_line( ( data & 0x02 ) ? ASSERT_LINE : CLEAR_LINE );
	}
}


READ8_MEMBER(sms_state::sms_93c46_r)
{
	UINT8 data = m_banking_cart[5][0];

	if ( m_cartridge[m_current_cartridge].m_93c46_enabled )
	{
		data = ( m_cartridge[m_current_cartridge].m_93c46_lines & 0xFC ) | 0x02;
		data |= m_eeprom->read_bit() ? 1 : 0;
	}

	return data;
}


WRITE8_MEMBER(sms_state::sms_mapper_w)
{
	bool bios_selected = false;
	bool cartridge_selected = false;

	offset &= 3;

	m_mapper[offset] = data;
	m_mapper_ram[offset] = data;

	if (m_bios_port & IO_BIOS_ROM || (m_is_gamegear && m_BIOS == NULL))
	{
		if (!(m_bios_port & IO_CARTRIDGE) || (m_is_gamegear && m_BIOS == NULL))
		{
			if (!m_cartridge[m_current_cartridge].ROM)
				return;
			cartridge_selected = true;
		}
		else
		{
			/* nothing to page in */
			return;
		}
	}
	else
	{
		if (!m_BIOS)
			return;
		bios_selected = true;
	}

	switch (offset)
	{
	case 0: /* Control */
		/* Is it ram or rom? */
		if (data & 0x08) /* it's ram */
		{
			if ( m_cartridge[m_current_cartridge].features & CF_93C46_EEPROM )
			{
				if ( data & 0x80 )
				{
					m_eeprom->reset();
					logerror("sms_mapper_w: eeprom CS=1\n");
					m_eeprom->set_cs_line( ASSERT_LINE );
				}
				logerror("sms_mapper_w: eeprom enabled\n");
				m_cartridge[m_current_cartridge].m_93c46_enabled = true;
			}
			else
			{
				UINT8 *sram = NULL;
				m_cartridge[m_current_cartridge].sram_save = 1;			/* SRAM should be saved on exit. */
				if (data & 0x04)
				{
					sram = m_cartridge[m_current_cartridge].cartSRAM + 0x4000;
				}
				else
				{
					sram = m_cartridge[m_current_cartridge].cartSRAM;
				}
				membank("bank5")->set_base(sram);
				membank("bank6")->set_base(sram + 0x2000);
			}
		}
		else /* it's rom */
		{
			if (m_bios_port & IO_BIOS_ROM || ! m_has_bios)
			{
				if ( ! ( m_cartridge[m_current_cartridge].features & ( CF_KOREAN_NOBANK_MAPPER | CF_KOREAN_ZEMINA_MAPPER ) ) )
				{
					if ( m_cartridge[m_current_cartridge].features & CF_93C46_EEPROM )
					{
						if ( data & 0x80 )
						{
							m_eeprom->reset();
							logerror("sms_mapper_w: eeprom CS=1\n");
							m_eeprom->set_cs_line( ASSERT_LINE );
						}
						logerror("sms_mapper_w: eeprom disabled\n");
						m_cartridge[m_current_cartridge].m_93c46_enabled = false;
					}
					else
					{
						map_cart_16k( 0x8000, m_mapper[3] );
					}
				}
			}
			else
			{
				map_bios_16k( 0x8000, m_mapper[3] );
			}
		}
		break;

	case 1: /* Select 16k ROM bank for 0400-3FFF */
		if ( cartridge_selected || m_is_gamegear )
		{
			if ( ! ( m_cartridge[m_current_cartridge].features & ( CF_KOREAN_NOBANK_MAPPER | CF_KOREAN_ZEMINA_MAPPER ) ) )
			{
				map_cart_16k( 0x400, data );
			}
		}
		if ( bios_selected )
		{
			map_bios_16k( 0x400, data );
		}
		break;

	case 2: /* Select 16k ROM bank for 4000-7FFF */
		if ( cartridge_selected || m_is_gamegear )
		{
			if ( ! ( m_cartridge[m_current_cartridge].features & ( CF_KOREAN_NOBANK_MAPPER | CF_KOREAN_ZEMINA_MAPPER ) ) )
			{
				map_cart_16k( 0x4000, data );
			}
		}
		if ( bios_selected )
		{
			map_bios_16k( 0x4000, data );
		}
		break;

	case 3: /* Select 16k ROM bank for 8000-BFFF */
		if ( cartridge_selected || m_is_gamegear )
		{
			if ( m_cartridge[m_current_cartridge].features & CF_CODEMASTERS_MAPPER)
			{
				return;
			}

			if ( ! ( m_mapper[0] & 0x08 ) )		// Is RAM disabled
			{
				if ( ! ( m_cartridge[m_current_cartridge].features & ( CF_KOREAN_NOBANK_MAPPER | CF_KOREAN_ZEMINA_MAPPER ) ) )
				{
					map_cart_16k( 0x8000, data );
				}
			}
		}

		if ( bios_selected )
		{
			if ( ! ( m_mapper[0] & 0x08 ) )		// Is RAM disabled
			{
				map_bios_16k( 0x8000, data );
			}
		}
		break;
	}
}

WRITE8_MEMBER(sms_state::sms_korean_zemina_banksw_w)
{

	if (m_cartridge[m_current_cartridge].features & CF_KOREAN_ZEMINA_MAPPER)
	{
		if (!m_cartridge[m_current_cartridge].ROM)
			return;

		switch (offset & 3)
		{
			case 0:
				map_cart_8k( 0x8000, data );
				break;
			case 1:
				map_cart_8k( 0xA000, data );
				break;
			case 2:
				map_cart_8k( 0x4000, data );
				break;
			case 3:
				map_cart_8k( 0x6000, data );
				break;
		}
		LOG(("Zemina mapper write: offset %x data %x.\n", offset, data));
	}
}

WRITE8_MEMBER(sms_state::sms_codemasters_page0_w)
{

	if (m_cartridge[m_current_cartridge].ROM && m_cartridge[m_current_cartridge].features & CF_CODEMASTERS_MAPPER)
	{
		map_cart_16k( 0x0000, data );
	}
}


WRITE8_MEMBER(sms_state::sms_codemasters_page1_w)
{

	if (m_cartridge[m_current_cartridge].ROM && m_cartridge[m_current_cartridge].features & CF_CODEMASTERS_MAPPER)
	{
		/* Check if we need to switch in some RAM */
		if (data & 0x80)
		{
			m_cartridge[m_current_cartridge].ram_page = data & 0x07;
			membank("bank6")->set_base(m_cartridge[m_current_cartridge].cartRAM + m_cartridge[m_current_cartridge].ram_page * 0x2000);
		}
		else
		{
			map_cart_16k( 0x4000, data );
			membank("bank6")->set_base(m_banking_cart[5] + 0x2000);
		}
	}
}


WRITE8_MEMBER(sms_state::sms_4pak_page0_w)
{

	m_cartridge[m_current_cartridge].m_4pak_page0 = data;

	map_cart_16k( 0x0000, data );
	map_cart_16k( 0x8000, ( m_cartridge[m_current_cartridge].m_4pak_page0 & 0x30 ) + m_cartridge[m_current_cartridge].m_4pak_page2 );
}


WRITE8_MEMBER(sms_state::sms_4pak_page1_w)
{

	m_cartridge[m_current_cartridge].m_4pak_page1 = data;

	map_cart_16k( 0x4000, data );
}


WRITE8_MEMBER(sms_state::sms_4pak_page2_w)
{

	m_cartridge[m_current_cartridge].m_4pak_page2 = data;

	map_cart_16k( 0x8000, ( m_cartridge[m_current_cartridge].m_4pak_page0 & 0x30 ) + m_cartridge[m_current_cartridge].m_4pak_page2 );
}


WRITE8_MEMBER(sms_state::sms_janggun_bank0_w)
{

	map_cart_8k( 0x4000, data );
}


WRITE8_MEMBER(sms_state::sms_janggun_bank1_w)
{

	map_cart_8k( 0x6000, data );
}


WRITE8_MEMBER(sms_state::sms_janggun_bank2_w)
{

	map_cart_8k( 0x8000, data );
}


WRITE8_MEMBER(sms_state::sms_janggun_bank3_w)
{

	map_cart_8k( 0xA000, data );
}


WRITE8_MEMBER(sms_state::sms_bios_w)
{
	m_bios_port = data;

	logerror("bios write %02x, pc: %04x\n", data, space.device().safe_pc());

	setup_rom(space);
}


WRITE8_MEMBER(sms_state::sms_cartram2_w)
{

	if (m_mapper[0] & 0x08)
	{
		logerror("write %02X to cartram at offset #%04X\n", data, offset + 0x2000);
		if (m_mapper[0] & 0x04)
		{
			m_cartridge[m_current_cartridge].cartSRAM[offset + 0x6000] = data;
		}
		else
		{
			m_cartridge[m_current_cartridge].cartSRAM[offset + 0x2000] = data;
		}
	}

	if (m_cartridge[m_current_cartridge].features & CF_CODEMASTERS_MAPPER)
	{
		m_cartridge[m_current_cartridge].cartRAM[m_cartridge[m_current_cartridge].ram_page * 0x2000 + offset] = data;
	}

	if (m_cartridge[m_current_cartridge].features & CF_KOREAN_MAPPER && offset == 0) /* Dodgeball King mapper */
	{
		map_cart_16k( 0x8000, data );
	}
}


WRITE8_MEMBER(sms_state::sms_cartram_w)
{
	int page;

	if (m_mapper[0] & 0x08)
	{
		logerror("write %02X to cartram at offset #%04X\n", data, offset);
		if (m_mapper[0] & 0x04)
		{
			m_cartridge[m_current_cartridge].cartSRAM[offset + 0x4000] = data;
		}
		else
		{
			m_cartridge[m_current_cartridge].cartSRAM[offset] = data;
		}
	}
	else
	{
		if (m_cartridge[m_current_cartridge].features & CF_CODEMASTERS_MAPPER && offset == 0) /* Codemasters mapper */
		{
			UINT8 rom_page_count = m_cartridge[m_current_cartridge].size / 0x4000;
			page = (rom_page_count > 0) ? data % rom_page_count : 0;
			if (!m_cartridge[m_current_cartridge].ROM)
				return;
			m_banking_cart[5] = m_cartridge[m_current_cartridge].ROM + page * 0x4000;
			membank("bank5")->set_base(m_banking_cart[5]);
			membank("bank6")->set_base(m_banking_cart[5] + 0x2000);
			LOG(("rom 2 paged in %x (Codemasters mapper).\n", page));
		}
		else if (m_cartridge[m_current_cartridge].features & CF_ONCART_RAM)
		{
			m_cartridge[m_current_cartridge].cartRAM[offset & (m_cartridge[m_current_cartridge].ram_size - 1)] = data;
		}
		else
		{
			logerror("INVALID write %02X to cartram at offset #%04X\n", data, offset);
		}
	}
}


WRITE8_MEMBER(sms_state::gg_sio_w)
{
	logerror("*** write %02X to SIO register #%d\n", data, offset);

	m_gg_sio[offset & 0x07] = data;
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


READ8_MEMBER(sms_state::gg_sio_r)
{
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

	return m_gg_sio[offset];
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


static void setup_rom( address_space &space )
{
	sms_state *state = space.machine().driver_data<sms_state>();

	/* 1. set up bank pointers to point to nothing */
	state->membank("bank1")->set_base(state->m_banking_none);
	state->membank("bank2")->set_base(state->m_banking_none);
	state->membank("bank7")->set_base(state->m_banking_none);
	state->membank("bank3")->set_base(state->m_banking_none);
	state->membank("bank4")->set_base(state->m_banking_none);
	state->membank("bank5")->set_base(state->m_banking_none);
	state->membank("bank6")->set_base(state->m_banking_none);

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
		state->membank("bank1")->set_base(state->m_banking_cart[1]);
		state->membank("bank2")->set_base(state->m_banking_cart[2]);
		state->membank("bank7")->set_base(state->m_banking_cart[7]);
		state->membank("bank3")->set_base(state->m_banking_cart[3]);
		state->membank("bank4")->set_base(state->m_banking_cart[3] + 0x2000);
		state->membank("bank5")->set_base(state->m_banking_cart[5]);
		state->membank("bank6")->set_base(state->m_banking_cart[5] + 0x2000);
		logerror("Switched in cartridge rom.\n");
	}

	/* 5. check and set up bios rom */
	if (!(state->m_bios_port & IO_BIOS_ROM))
	{
		/* 0x0400 bioses */
		if (state->m_has_bios_0400)
		{
			state->membank("bank1")->set_base(state->m_banking_bios[1]);
			logerror("Switched in 0x0400 bios.\n");
		}
		/* 0x2000 bioses */
		if (state->m_has_bios_2000)
		{
			state->membank("bank1")->set_base(state->m_banking_bios[1]);
			state->membank("bank2")->set_base(state->m_banking_bios[2]);
			logerror("Switched in 0x2000 bios.\n");
		}
		if (state->m_has_bios_full)
		{
			state->membank("bank1")->set_base(state->m_banking_bios[1]);
			state->membank("bank2")->set_base(state->m_banking_bios[2]);
			state->membank("bank7")->set_base(state->m_banking_bios[7]);
			state->membank("bank3")->set_base(state->m_banking_bios[3]);
			state->membank("bank4")->set_base(state->m_banking_bios[3] + 0x2000);
			state->membank("bank5")->set_base(state->m_banking_bios[5]);
			state->membank("bank6")->set_base(state->m_banking_bios[5] + 0x2000);
			logerror("Switched in full bios.\n");
		}
	}

	if (state->m_cartridge[state->m_current_cartridge].features & CF_ONCART_RAM)
	{
		state->membank("bank5")->set_base(state->m_cartridge[state->m_current_cartridge].cartRAM);
		state->membank("bank6")->set_base(state->m_cartridge[state->m_current_cartridge].cartRAM);
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
			retval = IMAGE_VERIFY_PASS;
		}

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

	if (strcmp(image.device().tag(), ":cart1") == 0)
		index = 0;
	if (strcmp(image.device().tag(), ":cart2") == 0)
		index = 1;
	if (strcmp(image.device().tag(), ":cart3") == 0)
		index = 2;
	if (strcmp(image.device().tag(), ":cart4") == 0)
		index = 3;
	if (strcmp(image.device().tag(), ":cart5") == 0)
		index = 4;
	if (strcmp(image.device().tag(), ":cart6") == 0)
		index = 5;
	if (strcmp(image.device().tag(), ":cart7") == 0)
		index = 6;
	if (strcmp(image.device().tag(), ":cart8") == 0)
		index = 7;
	if (strcmp(image.device().tag(), ":cart9") == 0)
		index = 8;
	if (strcmp(image.device().tag(), ":cart10") == 0)
		index = 9;
	if (strcmp(image.device().tag(), ":cart11") == 0)
		index = 10;
	if (strcmp(image.device().tag(), ":cart12") == 0)
		index = 11;
	if (strcmp(image.device().tag(), ":cart13") == 0)
		index = 12;
	if (strcmp(image.device().tag(), ":cart14") == 0)
		index = 13;
	if (strcmp(image.device().tag(), ":cart15") == 0)
		index = 14;
	if (strcmp(image.device().tag(), ":cart16") == 0)
		index = 15;

	state->m_cartridge[index].features = 0;

	if (image.software_entry() == NULL)
	{
		size = image.length();
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

		const char *pcb = image.get_feature("pcb");
		const char *mapper = image.get_feature("mapper");
		const char *pin_42 = image.get_feature("pin_42");
		const char *eeprom = image.get_feature("eeprom");

		// Check for special mappers (or lack of mappers)
		if ( pcb && !strcmp(pcb, "korean_nobank"))
		{
			state->m_cartridge[index].features |= CF_KOREAN_NOBANK_MAPPER;
		}

		if ( mapper )
		{
			if ( ! strcmp( mapper, "codemasters" ) )
			{
				state->m_cartridge[index].features |= CF_CODEMASTERS_MAPPER;
			}
			else if ( ! strcmp( mapper, "korean" ) )
			{
				state->m_cartridge[index].features |= CF_KOREAN_MAPPER;
			}
			else if ( ! strcmp( mapper, "zemina" ) )
			{
				state->m_cartridge[index].features |= CF_KOREAN_ZEMINA_MAPPER;
			}
			else if ( ! strcmp( mapper, "nemesis" ) )
			{
				state->m_cartridge[index].features |= CF_KOREAN_ZEMINA_MAPPER;
				state->m_cartridge[index].features |= CF_KOREAN_ZEMINA_NEMESIS;
			}
			else if ( ! strcmp( mapper, "4pak" ) )
			{
				state->m_cartridge[index].features |= CF_4PAK_MAPPER;
			}
			else if ( ! strcmp( mapper, "janggun" ) )
			{
				state->m_cartridge[index].features |= CF_JANGGUN_MAPPER;
			}
		}

		// Check for gamegear cartridges with PIN 42 set to SMS mode
		if ( pin_42 && ! strcmp(pin_42, "sms_mode"))
		{
			state->m_cartridge[index].features |= CF_GG_SMS_MODE;
		}

		// Check for presence of 93c46 eeprom
		if ( eeprom && ! strcmp( eeprom, "93c46" ) )
		{
			state->m_cartridge[index].features |= CF_93C46_EEPROM;
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

	// If no mapper bits are set attempt to autodetect the mapper
	if ( ! ( state->m_cartridge[index].features & CF_MAPPER_BITS ) )
	{
		/* If no extrainfo information is available try to find special information out on our own */
		/* Check for special cartridge features (new routine, courtesy of Omar Cornut, from MEKA)  */
		if (size >= 0x8000)
		{
			int c0002 = 0, c8000 = 0, cA000 = 0, cFFFF = 0, c3FFE = 0, c4000 = 0, c6000 = 0, i;
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
					if ( addr == 0x3FFE)
					{ i += 2; c3FFE++; continue; }
					if ( addr == 0x4000 )
					{ i += 2; c4000++; continue; }
					if ( addr == 0x6000 )
					{ i += 2; c6000++; continue; }
				}
			}

			LOG(("Mapper test: c002 = %d, c8000 = %d, cA000 = %d, cFFFF = %d\n", c0002, c8000, cA000, cFFFF));

			// 2 is a security measure, although tests on existing ROM showed it was not needed
			if (c0002 > cFFFF + 2 || (c0002 > 0 && cFFFF == 0))
			{
				UINT8 *rom = state->m_cartridge[index].ROM;

				state->m_cartridge[index].features |= CF_KOREAN_ZEMINA_MAPPER;
				// Check for special bank 0 signature
				if ( size == 0x20000 && rom[0] == 0x00 && rom[1] == 0x00 && rom[2] == 0x00 &&
				     rom[0x1e000] == 0xF3 && rom[0x1e001] == 0xED && rom[0x1e002] == 0x56 )
				{
					state->m_cartridge[index].features |= CF_KOREAN_ZEMINA_NEMESIS;
				}
			}
			else if (c8000 > cFFFF + 2 || (c8000 > 0 && cFFFF == 0))
			{
				state->m_cartridge[index].features |= CF_CODEMASTERS_MAPPER;
			}
			else if (cA000 > cFFFF + 2 || (cA000 > 0 && cFFFF == 0))
			{
				state->m_cartridge[index].features |= CF_KOREAN_MAPPER;
			}
			else if ( c3FFE > cFFFF + 2 || (c3FFE > 0) )
			{
				state->m_cartridge[index].features |= CF_4PAK_MAPPER;
			}
			else if ( c4000 > 0 && c6000 > 0 && c8000 > 0 && cA000 > 0 )
			{
				state->m_cartridge[index].features |= CF_JANGGUN_MAPPER;
			}
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
		state->m_cartridge[index].features |= CF_TVDRAW;
	}

	if (state->m_cartridge[index].features & CF_JANGGUN_MAPPER)
	{
		// Reverse bytes when bit 6 in the mapper is set

		if ( state->m_cartridge[index].size <= 0x40 * 0x4000 )
		{
			UINT8 *new_rom = auto_alloc_array(machine, UINT8, 0x80 * 0x4000);
			UINT32 dest = 0;

			while ( dest < 0x40 * 0x4000 )
			{
				memcpy( new_rom + dest, state->m_cartridge[index].ROM, state->m_cartridge[index].size );
				dest += state->m_cartridge[index].size;
			}

			for ( dest = 0; dest < 0x40 * 0x4000; dest++ )
			{
				new_rom[ 0x40 * 0x4000 + dest ] = BITSWAP8( new_rom[ dest ], 0, 1, 2, 3, 4, 5, 6, 7);
			}

			state->m_cartridge[index].ROM = new_rom;
			state->m_cartridge[index].size = 0x80 * 0x4000;
		}
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
		state->m_banking_cart[7] = state->m_cartridge[state->m_current_cartridge].ROM + 0x2000;
		/* Codemasters mapper points to bank 0 for page 2 */
		if ( state->m_cartridge[state->m_current_cartridge].features & CF_CODEMASTERS_MAPPER )
		{
			state->m_banking_cart[5] = state->m_cartridge[state->m_current_cartridge].ROM;
		}
		/* Nemesis starts with last 8kb bank in page 0 */
		if (state->m_cartridge[state->m_current_cartridge].features & CF_KOREAN_ZEMINA_NEMESIS )
		{
			state->m_banking_cart[1] = state->m_cartridge[state->m_current_cartridge].ROM + ( rom_page_count - 1 ) * 0x4000 + 0x2000;
			state->m_banking_cart[2] = state->m_cartridge[state->m_current_cartridge].ROM + ( rom_page_count - 1 ) * 0x4000 + 0x2000 + 0x400;
		}
	}
	else
	{
		state->m_banking_cart[1] = state->m_banking_none;
		state->m_banking_cart[2] = state->m_banking_none;
		state->m_banking_cart[3] = state->m_banking_none;
		state->m_banking_cart[5] = state->m_banking_none;
	}
}

static void setup_banks( running_machine &machine )
{
	sms_state *state = machine.driver_data<sms_state>();
	UINT8 *mem = machine.root_device().memregion("maincpu")->base();
	state->m_banking_none = mem;
	state->m_banking_bios[1] = state->m_banking_cart[1] = mem;
	state->m_banking_bios[2] = state->m_banking_cart[2] = mem;
	state->m_banking_bios[3] = state->m_banking_cart[3] = mem;
	state->m_banking_bios[4] = state->m_banking_cart[4] = mem;
	state->m_banking_bios[5] = state->m_banking_cart[5] = mem;
	state->m_banking_bios[6] = state->m_banking_cart[6] = mem;
	state->m_banking_bios[7] = state->m_banking_cart[7] = mem;

	state->m_BIOS = machine.root_device().memregion("user1")->base();

	state->m_bios_page_count = (state->m_BIOS ? state->memregion("user1")->bytes() / 0x4000 : 0);

	setup_cart_banks(machine);

	if (state->m_BIOS == NULL || state->m_BIOS[0] == 0x00)
	{
		state->m_BIOS = NULL;
		state->m_bios_port |= IO_BIOS_ROM;
		state->m_has_bios_0400 = 0;
		state->m_has_bios_2000 = 0;
		state->m_has_bios_full = 0;
		state->m_has_bios = 0;
	}

	if (state->m_BIOS)
	{
		state->m_banking_bios[1] = state->m_BIOS;
		state->m_banking_bios[2] = state->m_BIOS + 0x0400;
		state->m_banking_bios[3] = state->m_BIOS + ((1 < state->m_bios_page_count) ? 0x4000 : 0);
		state->m_banking_bios[5] = state->m_BIOS + ((2 < state->m_bios_page_count) ? 0x8000 : 0);
	}
}

MACHINE_START_MEMBER(sms_state,sms)
{

	machine().add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(sms_machine_stop),&machine()));
	m_rapid_fire_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(sms_state::rapid_fire_callback),this));
	m_rapid_fire_timer->adjust(attotime::from_hz(10), 0, attotime::from_hz(10));

	m_lphaser_1_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(sms_state::lphaser_1_callback),this));
	m_lphaser_2_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(sms_state::lphaser_2_callback),this));

	m_main_cpu = machine().device("maincpu");
	m_control_cpu = machine().device("control");
	m_vdp = machine().device<sega315_5124_device>("sms_vdp");
	m_eeprom = machine().device<eeprom_device>("eeprom");
	m_ym = machine().device("ym2413");
	m_main_scr = machine().device("screen");
	m_left_lcd = machine().device("left_lcd");
	m_right_lcd = machine().device("right_lcd");
	m_space = &machine().device("maincpu")->memory().space(AS_PROGRAM);

	/* Check if lightgun has been chosen as input: if so, enable crosshair */
	machine().scheduler().timer_set(attotime::zero, timer_expired_delegate(FUNC(sms_state::lightgun_tick),this));
}

MACHINE_RESET_MEMBER(sms_state,sms)
{
	address_space &space = machine().device("maincpu")->memory().space(AS_PROGRAM);

	m_ctrl_reg = 0xff;
	if (m_has_fm)
		m_fm_detect = 0x01;

	m_mapper_ram = (UINT8*)space.get_write_ptr(0xdffc);

	m_bios_port = 0;

	if ( m_cartridge[m_current_cartridge].features & CF_CODEMASTERS_MAPPER )
	{
		/* Install special memory handlers */
		space.install_write_handler(0x0000, 0x0000, write8_delegate(FUNC(sms_state::sms_codemasters_page0_w),this));
		space.install_write_handler(0x4000, 0x4000, write8_delegate(FUNC(sms_state::sms_codemasters_page1_w),this));
	}

	if ( m_cartridge[m_current_cartridge].features & CF_KOREAN_ZEMINA_MAPPER )
	{
		space.install_write_handler(0x0000, 0x0003, write8_delegate(FUNC(sms_state::sms_korean_zemina_banksw_w),this));
	}

	if ( m_cartridge[m_current_cartridge].features & CF_JANGGUN_MAPPER )
	{
		space.install_write_handler(0x4000, 0x4000, write8_delegate(FUNC(sms_state::sms_janggun_bank0_w),this));
		space.install_write_handler(0x6000, 0x6000, write8_delegate(FUNC(sms_state::sms_janggun_bank1_w),this));
		space.install_write_handler(0x8000, 0x8000, write8_delegate(FUNC(sms_state::sms_janggun_bank2_w),this));
		space.install_write_handler(0xA000, 0xA000,write8_delegate(FUNC(sms_state::sms_janggun_bank3_w),this));
	}

	if ( m_cartridge[m_current_cartridge].features & CF_4PAK_MAPPER )
	{
		space.install_write_handler(0x3ffe, 0x3ffe, write8_delegate(FUNC(sms_state::sms_4pak_page0_w),this));
		space.install_write_handler(0x7fff, 0x7fff, write8_delegate(FUNC(sms_state::sms_4pak_page1_w),this));
		space.install_write_handler(0xbfff, 0xbfff, write8_delegate(FUNC(sms_state::sms_4pak_page2_w),this));
	}

	if ( m_cartridge[m_current_cartridge].features & CF_TVDRAW )
	{
		space.install_write_handler(0x6000, 0x6000, write8_delegate(FUNC(sms_state::sms_tvdraw_axis_w),this));
		space.install_read_handler(0x8000, 0x8000, read8_delegate(FUNC(sms_state::sms_tvdraw_status_r),this));
		space.install_read_handler(0xa000, 0xa000, read8_delegate(FUNC(sms_state::sms_tvdraw_data_r),this));
		space.nop_write(0xa000, 0xa000);
		m_cartridge[m_current_cartridge].m_tvdraw_data = 0;
	}

	if ( m_cartridge[m_current_cartridge].features & CF_93C46_EEPROM )
	{
		space.install_write_handler(0x8000,0x8000, write8_delegate(FUNC(sms_state::sms_93c46_w),this));
		space.install_read_handler(0x8000,0x8000, read8_delegate(FUNC(sms_state::sms_93c46_r),this));
	}

	if (m_cartridge[m_current_cartridge].features & CF_GG_SMS_MODE)
	{
		m_vdp->set_sega315_5124_compatibility_mode(true);
	}

	/* Initialize SIO stuff for GG */
	m_gg_sio[0] = 0x7f;
	m_gg_sio[1] = 0xff;
	m_gg_sio[2] = 0x00;
	m_gg_sio[3] = 0xff;
	m_gg_sio[4] = 0x00;

	m_store_control = 0;

	setup_banks(machine());

	setup_rom(space);

	m_rapid_fire_state_1 = 0;
	m_rapid_fire_state_2 = 0;

	m_last_paddle_read_time = 0;
	m_paddle_read_state = 0;

	m_last_sports_pad_time_1 = 0;
	m_last_sports_pad_time_2 = 0;
	m_sports_pad_state_1 = 0;
	m_sports_pad_state_2 = 0;
	m_sports_pad_last_data_1 = 0;
	m_sports_pad_last_data_2 = 0;
	m_sports_pad_1_x = 0;
	m_sports_pad_1_y = 0;
	m_sports_pad_2_x = 0;
	m_sports_pad_2_y = 0;

	m_lphaser_1_latch = 0;
	m_lphaser_2_latch = 0;

	m_sscope_state = 0;
}

READ8_MEMBER(sms_state::sms_store_cart_select_r)
{
	return 0xff;
}


WRITE8_MEMBER(sms_state::sms_store_cart_select_w)
{
	UINT8 slot = data >> 4;
	UINT8 slottype = data & 0x08;

	logerror("switching in part of %s slot #%d\n", slottype ? "card" : "cartridge", slot );
	/* cartridge? slot #0 */
	if (slottype == 0)
		m_current_cartridge = slot;

	setup_cart_banks(machine());
	membank("bank10")->set_base(m_banking_cart[3] + 0x2000);
	setup_rom(space);
}


READ8_MEMBER(sms_state::sms_store_select1)
{
	return 0xff;
}


READ8_MEMBER(sms_state::sms_store_select2)
{
	return 0xff;
}


READ8_MEMBER(sms_state::sms_store_control_r)
{
	return m_store_control;
}


WRITE8_MEMBER(sms_state::sms_store_control_w)
{
	logerror("0x%04X: sms_store_control write 0x%02X\n", space.device().safe_pc(), data);
	if (data & 0x02)
	{
		machine().device<cpu_device>("maincpu")->resume(SUSPEND_REASON_HALT);
	}
	else
	{
		/* Pull reset line of CPU #0 low */
		machine().device("maincpu")->reset();
		machine().device<cpu_device>("maincpu")->suspend(SUSPEND_REASON_HALT, 1);
	}
	m_store_control = data;
}

WRITE_LINE_MEMBER(sms_state::sms_store_int_callback)
{
	(m_store_control & 0x01 ? m_control_cpu : m_main_cpu)->execute().set_input_line(0, state);
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

DRIVER_INIT_MEMBER(sms_state,sg1000m3)
{
	sms_set_zero_flag(machine());
	m_is_region_japan = 1;
	m_has_fm = 1;
}


DRIVER_INIT_MEMBER(sms_state,sms1)
{
	sms_set_zero_flag(machine());
	m_has_bios_full = 1;
}


DRIVER_INIT_MEMBER(sms_state,smsj)
{
	sms_set_zero_flag(machine());
	m_is_region_japan = 1;
	m_has_bios_2000 = 1;
	m_has_fm = 1;
}


DRIVER_INIT_MEMBER(sms_state,sms2kr)
{
	sms_set_zero_flag(machine());
	m_is_region_japan = 1;
	m_has_bios_full = 1;
	m_has_fm = 1;
}


DRIVER_INIT_MEMBER(sms_state,smssdisp)
{
	sms_set_zero_flag(machine());
}


DRIVER_INIT_MEMBER(sms_state,gamegear)
{
	sms_set_zero_flag(machine());
	m_is_gamegear = 1;
	m_has_bios_0400 = 1;
}


DRIVER_INIT_MEMBER(sms_state,gamegeaj)
{
	sms_set_zero_flag(machine());
	m_is_region_japan = 1;
	m_is_gamegear = 1;
	m_has_bios_0400 = 1;
}


VIDEO_START_MEMBER(sms_state,sms1)
{
	screen_device *screen = machine().first_screen();

	screen->register_screen_bitmap(m_prevleft_bitmap);
	screen->register_screen_bitmap(m_prevright_bitmap);
	save_item(NAME(m_prevleft_bitmap));
	save_item(NAME(m_prevright_bitmap));
}

UINT32 sms_state::screen_update_sms1(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT8 sscope = 0;
	UINT8 sscope_binocular_hack;
	UINT8 occluded_view = 0;

	if (&screen != m_main_scr)
	{
		sscope = machine().root_device().ioport("SEGASCOPE")->read_safe(0x00);
		if (!sscope)
		{
			occluded_view = 1;
		}
		else if (&screen == m_left_lcd)
		{
			// with SegaScope, sscope_state 0 = left screen OFF, right screen ON
			if (!(m_sscope_state & 0x01))
				occluded_view = 1;
		}
		else // it's right LCD
		{
			// with SegaScope, sscope_state 1 = left screen ON, right screen OFF
			if (m_sscope_state & 0x01)
				occluded_view = 1;
		}
	}

	if (!occluded_view)
	{
		m_vdp->screen_update(screen, bitmap, cliprect);

		// HACK: fake 3D->2D handling (if enabled, it repeats each frame twice on the selected lens)
		// save a copy of current bitmap for the binocular hack
		if (sscope)
		{
			sscope_binocular_hack = machine().root_device().ioport("SSCOPE_BINOCULAR")->read_safe(0x00);

			if (&screen == m_left_lcd)
			{
				if (sscope_binocular_hack & 0x01)
					copybitmap(m_prevleft_bitmap, bitmap, 0, 0, 0, 0, cliprect);
			}
			else // it's right LCD
			{
				if (sscope_binocular_hack & 0x02)
					copybitmap(m_prevright_bitmap, bitmap, 0, 0, 0, 0, cliprect);
			}
		}
	}
	else
	{
		// HACK: fake 3D->2D handling (if enabled, it repeats each frame twice on the selected lens)
		// use the copied bitmap for the binocular hack
		if (sscope)
		{
			sscope_binocular_hack = machine().root_device().ioport("SSCOPE_BINOCULAR")->read_safe(0x00);

			if (&screen == m_left_lcd)
			{
				if (sscope_binocular_hack & 0x01)
				{
					copybitmap(bitmap, m_prevleft_bitmap, 0, 0, 0, 0, cliprect);
					return 0;
				}
			}
			else // it's right LCD
			{
				if (sscope_binocular_hack & 0x02)
				{
					copybitmap(bitmap, m_prevright_bitmap, 0, 0, 0, 0, cliprect);
					return 0;
				}
			}
		}
		bitmap.fill(RGB_BLACK);
	}

	return 0;
}

UINT32 sms_state::screen_update_sms(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_vdp->screen_update(screen, bitmap, cliprect);
	return 0;
}

VIDEO_START_MEMBER(sms_state,gamegear)
{
	screen_device *screen = machine().first_screen();

	screen->register_screen_bitmap(m_prev_bitmap);
	save_item(NAME(m_prev_bitmap));
}

UINT32 sms_state::screen_update_gamegear(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int width = screen.width();
	int height = screen.height();
	int x, y;

	bitmap_rgb32 &vdp_bitmap = m_vdp->get_bitmap();

	// HACK: fake LCD persistence effect
	// (it would be better to generalize this in the core, to be used for all LCD systems)
	for (y = 0; y < height; y++)
	{
		UINT32 *linedst = &bitmap.pix32(y);
		UINT32 *line0 = &vdp_bitmap.pix32(y);
		UINT32 *line1 = &m_prev_bitmap.pix32(y);
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
			linedst[x] = (r << 16) | (g << 8) | b;
		}
	}
	copybitmap(m_prev_bitmap, vdp_bitmap, 0, 0, 0, 0, cliprect);

	return 0;
}
