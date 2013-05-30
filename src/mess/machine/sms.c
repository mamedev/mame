#include "emu.h"
#include "crsshair.h"
#include "video/315_5124.h"
#include "sound/2413intf.h"
#include "includes/sms.h"

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

#define ENABLE_NONE      0
#define ENABLE_EXPANSION 1
#define ENABLE_CARD      2
#define ENABLE_CART      3
#define ENABLE_BIOS      4

#define LGUN_RADIUS           6
#define LGUN_X_INTERVAL       4

void sms_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_RAPID_FIRE:
		rapid_fire_callback(ptr, param);
		break;
	case TIMER_LIGHTGUN_TICK:
		lightgun_tick(ptr, param);
		break;
	case TIMER_LPHASER_1:
		lphaser_1_callback(ptr, param);
		break;
	case TIMER_LPHASER_2:
		lphaser_2_callback(ptr, param);
		break;
	default:
		assert_always(FALSE, "Unknown id in sms_state::device_timer");
	}
}


TIMER_CALLBACK_MEMBER(sms_state::rapid_fire_callback)
{
	m_rapid_fire_state_1 ^= 0xff;
	m_rapid_fire_state_2 ^= 0xff;
}



WRITE8_MEMBER(sms_state::sms_input_write)
{
	switch (offset)
	{
	case 0:
		switch (m_port_ctrlsel->read_safe(0x00) & 0x0f)
		{
		case 0x04:  /* Sports Pad */
			if (data != m_sports_pad_last_data_1)
			{
				UINT32 cpu_cycles = m_maincpu->total_cycles();

				m_sports_pad_last_data_1 = data;
				if (cpu_cycles - m_last_sports_pad_time_1 > 512)
				{
					m_sports_pad_state_1 = 3;
					m_sports_pad_1_x = m_port_sport0->read();
					m_sports_pad_1_y = m_port_sport1->read();
				}
				m_last_sports_pad_time_1 = cpu_cycles;
				m_sports_pad_state_1 = (m_sports_pad_state_1 + 1) & 3;
			}
			break;
		}
		break;

	case 1:
		switch (m_port_ctrlsel->read_safe(0x00) & 0xf0)
		{
		case 0x40:  /* Sports Pad */
			if (data != m_sports_pad_last_data_2)
			{
				UINT32 cpu_cycles = m_maincpu->total_cycles();

				m_sports_pad_last_data_2 = data;
				if (cpu_cycles - m_last_sports_pad_time_2 > 2048)
				{
					m_sports_pad_state_2 = 3;
					m_sports_pad_2_x = m_port_sport2->read();
					m_sports_pad_2_y = m_port_sport3->read();
				}
				m_last_sports_pad_time_2 = cpu_cycles;
				m_sports_pad_state_2 = (m_sports_pad_state_2 + 1) & 3;
			}
			break;
		}
		break;
	}
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
int sms_state::lgun_bright_aim_area( emu_timer *timer, int lgun_x, int lgun_y )
{
	const int r_x_r = LGUN_RADIUS * LGUN_RADIUS;
	const rectangle &visarea = m_main_scr->visible_area();
	int beam_x = m_main_scr->hpos();
	int beam_y = m_main_scr->vpos();
	int dx, dy;
	int result = 0;
	int pos_changed = 0;
	double dx_radius;

	while (1)
	{
		/* If beam's y isn't at a line where the aim area is, change it
		   the next line it enters that area. */
		dy = abs(beam_y - lgun_y);
		if (dy > LGUN_RADIUS || beam_y < visarea.min_y || beam_y > visarea.max_y)
		{
			beam_y = lgun_y - LGUN_RADIUS;
			if (beam_y < visarea.min_y)
				beam_y = visarea.min_y;
			dy = abs(beam_y - lgun_y);
			pos_changed = 1;
		}

		/* Caculate distance in x of the radius, relative to beam's y distance.
		   First try some shortcuts. */
		switch (dy)
		{
		case LGUN_RADIUS:
			dx_radius = 0;
			break;
		case 0:
			dx_radius = LGUN_RADIUS;
			break;
		default:
			/* step 1: r^2 = dx^2 + dy^2 */
			/* step 2: dx^2 = r^2 - dy^2 */
			/* step 3: dx = sqrt(r^2 - dy^2) */
			dx_radius = ceil((float) sqrt((float) (r_x_r - (dy * dy))));
		}

		/* If beam's x isn't in the circular aim area, change it
		   to the next point it enters that area. */
		dx = abs(beam_x - lgun_x);
		if (dx > dx_radius || beam_x < visarea.min_x || beam_x > visarea.max_x)
		{
			/* If beam's x has passed the aim area, advance to
			   next line and recheck y/x coordinates. */
			if (beam_x > lgun_x)
			{
				beam_x = 0;
				beam_y++;
				continue;
			}
			beam_x = lgun_x - dx_radius;
			if (beam_x < visarea.min_x)
				beam_x = visarea.min_x;
			pos_changed = 1;
		}

		if (!pos_changed)
		{
			bitmap_rgb32 &bitmap = m_vdp->get_bitmap();

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
	timer->adjust(m_main_scr->time_until_pos(beam_y, beam_x));

	return result;
}


void sms_state::lphaser_hcount_latch( int hpos )
{
	/* A delay seems to occur when the Light Phaser latches the
	   VDP hcount, then an offset is added here to the hpos. */
	m_vdp->hcount_latch_at_hpos(hpos + m_lphaser_x_offs);
}


UINT16 sms_state::screen_hpos_nonscaled(int scaled_hpos)
{
	const rectangle &visarea = m_main_scr->visible_area();
	int offset_x = (scaled_hpos * (visarea.max_x - visarea.min_x)) / 255;
	return visarea.min_x + offset_x;
}


UINT16 sms_state::screen_vpos_nonscaled(int scaled_vpos)
{
	const rectangle &visarea = m_main_scr->visible_area();
	int offset_y = (scaled_vpos * (visarea.max_y - visarea.min_y)) / 255;
	return visarea.min_y + offset_y;
}


void sms_state::lphaser1_sensor_check()
{
	const int x = screen_hpos_nonscaled(m_port_lphas0->read());
	const int y = screen_vpos_nonscaled(m_port_lphas1->read());

	if (lgun_bright_aim_area(m_lphaser_1_timer, x, y))
	{
		if (m_lphaser_1_latch == 0)
		{
			m_lphaser_1_latch = 1;
			lphaser_hcount_latch(x);
		}
	}
}

void sms_state::lphaser2_sensor_check()
{
	const int x = screen_hpos_nonscaled(m_port_lphas2->read());
	const int y = screen_vpos_nonscaled(m_port_lphas3->read());

	if (lgun_bright_aim_area(m_lphaser_2_timer, x, y))
	{
		if (m_lphaser_2_latch == 0)
		{
			m_lphaser_2_latch = 1;
			lphaser_hcount_latch(x);
		}
	}
}


// at each input port read we check if lightguns are enabled in one of the ports:
// if so, we turn on crosshair and the lightgun timer
TIMER_CALLBACK_MEMBER(sms_state::lightgun_tick)
{
	if ((m_port_ctrlsel->read_safe(0x00) & 0x0f) == 0x01)
	{
		/* enable crosshair */
		crosshair_set_screen(machine(), 0, CROSSHAIR_SCREEN_ALL);
		if (!m_lphaser_1_timer->enabled())
			lphaser1_sensor_check();
	}
	else
	{
		/* disable crosshair */
		crosshair_set_screen(machine(), 0, CROSSHAIR_SCREEN_NONE);
		m_lphaser_1_timer->enable(0);
	}

	if ((m_port_ctrlsel->read_safe(0x00) & 0xf0) == 0x10)
	{
		/* enable crosshair */
		crosshair_set_screen(machine(), 1, CROSSHAIR_SCREEN_ALL);
		if (!m_lphaser_2_timer->enabled())
			lphaser2_sensor_check();
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
	lphaser1_sensor_check();
}


TIMER_CALLBACK_MEMBER(sms_state::lphaser_2_callback)
{
	lphaser2_sensor_check();
}


INPUT_CHANGED_MEMBER(sms_state::lgun1_changed)
{
	if (!m_lphaser_1_timer ||
		(m_port_ctrlsel->read_safe(0x00) & 0x0f) != 0x01)
		return;

	if (newval != oldval)
		lphaser1_sensor_check();
}

INPUT_CHANGED_MEMBER(sms_state::lgun2_changed)
{
	if (!m_lphaser_2_timer ||
		(m_port_ctrlsel->read_safe(0x00) & 0xf0) != 0x10)
		return;

	if (newval != oldval)
		lphaser2_sensor_check();
}


void sms_state::sms_get_inputs( address_space &space )
{
	UINT8 data = 0x00;
	UINT32 cpu_cycles = m_maincpu->total_cycles();

	m_input_port0 = 0xff;
	m_input_port1 = 0xff;

	if (cpu_cycles - m_last_paddle_read_time > 256)
	{
		m_paddle_read_state ^= 0xff;
		m_last_paddle_read_time = cpu_cycles;
	}

	/* Check if lightgun has been chosen as input: if so, enable crosshair */
	timer_set(attotime::zero, TIMER_LIGHTGUN_TICK);

	/* Player 1 */
	switch (m_port_ctrlsel->read_safe(0x00) & 0x0f)
	{
	case 0x00:  /* Joystick */
		data = m_port_dc->read();
		/* Check Rapid Fire setting for Button A */
		if (!(data & 0x10) && (m_port_rfu->read() & 0x01))
			data |= m_rapid_fire_state_1 & 0x10;

		/* Check Rapid Fire setting for Button B */
		if (!(data & 0x20) && (m_port_rfu->read() & 0x02))
			data |= m_rapid_fire_state_1 & 0x20;

		m_input_port0 = (m_input_port0 & 0xc0) | (data & 0x3f);
		break;

	case 0x01:  /* Light Phaser */
		data = (m_port_ctrlipt->read() & 0x01) << 4;
		/* Check Rapid Fire setting for Trigger */
		if (!(data & 0x10) && (m_port_rfu->read() & 0x01))
			data |= m_rapid_fire_state_1 & 0x10;

		/* just consider the trigger (button) bit */
		data |= ~0x10;

		m_input_port0 = (m_input_port0 & 0xc0) | (data & 0x3f);
		break;

	case 0x02:  /* Paddle Control */
		/* Get button A state */
		data = m_port_paddle0->read();

		if (m_paddle_read_state)
			data = data >> 4;

		m_input_port0 = (m_input_port0 & 0xc0) | (data & 0x0f) | (m_paddle_read_state & 0x20)
						| ((m_port_ctrlipt->read() & 0x02) << 3);
		break;

	case 0x04:  /* Sega Sports Pad */
		switch (m_sports_pad_state_1)
		{
		case 0:
			data = (m_sports_pad_1_x >> 4) & 0x0f;
			break;
		case 1:
			data = m_sports_pad_1_x & 0x0f;
			break;
		case 2:
			data = (m_sports_pad_1_y >> 4) & 0x0f;
			break;
		case 3:
			data = m_sports_pad_1_y & 0x0f;
			break;
		}
		m_input_port0 = (m_input_port0 & 0xc0) | data | ((m_port_ctrlipt->read() & 0x0c) << 2);
		break;
	}

	/* Player 2 */
	switch (m_port_ctrlsel->read_safe(0x00)  & 0xf0)
	{
	case 0x00:  /* Joystick */
		data = m_port_dc->read();
		m_input_port0 = (m_input_port0 & 0x3f) | (data & 0xc0);

		data = m_port_dd->read();
		/* Check Rapid Fire setting for Button A */
		if (!(data & 0x04) && (m_port_rfu->read() & 0x04))
			data |= m_rapid_fire_state_2 & 0x04;

		/* Check Rapid Fire setting for Button B */
		if (!(data & 0x08) && (m_port_rfu->read() & 0x08))
			data |= m_rapid_fire_state_2 & 0x08;

		m_input_port1 = (m_input_port1 & 0xf0) | (data & 0x0f);
		break;

	case 0x10:  /* Light Phaser */
		data = (m_port_ctrlipt->read() & 0x10) >> 2;
		/* Check Rapid Fire setting for Trigger */
		if (!(data & 0x04) && (m_port_rfu->read() & 0x04))
			data |= m_rapid_fire_state_2 & 0x04;

		/* just consider the trigger (button) bit */
		data |= ~0x04;

		m_input_port1 = (m_input_port1 & 0xf0) | (data & 0x0f);
		break;

	case 0x20:  /* Paddle Control */
		/* Get button A state */
		data = m_port_paddle1->read();
		if (m_paddle_read_state)
			data = data >> 4;

		m_input_port0 = (m_input_port0 & 0x3f) | ((data & 0x03) << 6);
		m_input_port1 = (m_input_port1 & 0xf0) | ((data & 0x0c) >> 2) | (m_paddle_read_state & 0x08)
						| ((m_port_ctrlipt->read() & 0x20) >> 3);
		break;

	case 0x40:  /* Sega Sports Pad */
		switch (m_sports_pad_state_2)
		{
		case 0:
			data = m_sports_pad_2_x & 0x0f;
			break;
		case 1:
			data = (m_sports_pad_2_x >> 4) & 0x0f;
			break;
		case 2:
			data = m_sports_pad_2_y & 0x0f;
			break;
		case 3:
			data = (m_sports_pad_2_y >> 4) & 0x0f;
			break;
		}
		m_input_port0 = (m_input_port0 & 0x3f) | ((data & 0x03) << 6);
		m_input_port1 = (m_input_port1 & 0xf0) | (data >> 2) | ((m_port_ctrlipt->read() & 0xc0) >> 4);
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
	bool latch_hcount = false;

	if (data & 0x08)
	{
		/* check if TH pin level is high (1) and was low last time */
		if (data & 0x80 && !(m_ctrl_reg & 0x80))
		{
			latch_hcount = true;
		}
		sms_input_write(space, 0, (data & 0x20) >> 5);
	}

	if (data & 0x02)
	{
		if (data & 0x20 && !(m_ctrl_reg & 0x20))
		{
			latch_hcount = true;
		}
		sms_input_write(space, 1, (data & 0x80) >> 7);
	}

	if (latch_hcount)
	{
		m_vdp->hcount_latch();
	}

	m_ctrl_reg = data;
}


READ8_MEMBER(sms_state::sms_count_r)
{
	if (offset & 0x01)
		return m_vdp->hcount_read(*m_space, offset);
	else
		return m_vdp->vcount_read(*m_space, offset);
}


/*
 Check if the pause button is pressed.
 If the gamegear is in sms mode, check if the start button is pressed.
 */
WRITE_LINE_MEMBER(sms_state::sms_pause_callback)
{
	if (m_is_gamegear && m_cartslot->m_cart && !m_cartslot->m_cart->get_sms_mode())
		return;

	if ((m_is_gamegear && !(m_port_start->read() & 0x80)) || (!m_is_gamegear && !(m_port_pause->read() & 0x80)))
	{
		if (!m_paused)
			m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);

		m_paused = 1;
	}
	else
		m_paused = 0;

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
	m_input_port1 = (m_input_port1 & 0xef) | (m_port_reset->read_safe(0x01) & 0x01) << 4;

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
		m_ym->write(space, 0, (data & 0x3f));
}


WRITE8_MEMBER(sms_state::sms_ym2413_data_port_0_w)
{
	if (m_has_fm)
	{
		logerror("data_port_0_w %x %x\n", offset, data);
		m_ym->write(space, 1, data);
	}
}


READ8_MEMBER(sms_state::gg_input_port_2_r)
{
	//logerror("joy 2 read, val: %02x, pc: %04x\n", ((m_is_region_japan ? 0x00 : 0x40) | (m_port_start->read() & 0x80)), activecpu_get_pc());
	return ((m_is_region_japan ? 0x00 : 0x40) | (m_port_start->read() & 0x80));
}


READ8_MEMBER(sms_state::sms_sscope_r)
{
	int sscope = m_port_scope->read_safe(0x00);

	if ( sscope )
	{
		// Scope is attached
		return m_sscope_state;
	}

	return m_mainram[0x1FF8 + offset];
}


WRITE8_MEMBER(sms_state::sms_sscope_w)
{
	m_mainram[0x1FF8 + offset] = data;

	int sscope = m_port_scope->read_safe(0x00);

	if ( sscope )
	{
		// Scope is attached
		m_sscope_state = data;

		// There are occurrences when Sega Scope's state changes after VBLANK, or at
		// active screen. Most cases are solid-color frames of scene transitions, but
		// one exception is the first frame of Zaxxon 3-D's title screen. In that
		// case, this method is enough for setting the intended state for the frame.
		// No information found about a minimum time need for switch open/closed lens.
		if (m_main_scr->vpos() < (m_main_scr->height() >> 1))
		{
			m_frame_sscope_state = m_sscope_state;
		}
	}
}


READ8_MEMBER(sms_state::sms_mapper_r)
{
	return m_mainram[0x1ffc + offset];
}


WRITE8_MEMBER(sms_state::sms_mapper_w)
{
	bool bios_selected = false;
	bool cartridge_selected = false;
	
	m_mapper[offset] = data;
	m_mainram[0x1ffc + offset] = data;

	if (m_bios_port & IO_BIOS_ROM || (m_is_gamegear && m_BIOS == NULL))
	{
		if (!(m_bios_port & IO_CARTRIDGE) || (m_is_gamegear && m_BIOS == NULL))
			cartridge_selected = true;
		else
			return;	// nothing to page in
	}
	else
	{
		if (!m_BIOS)
			return;	// nothing to page in
		bios_selected = true;
	}
	
	switch (offset)
	{
		case 0: // Control RAM/ROM
			if (!(data & 0x08) && !(m_bios_port & IO_BIOS_ROM) && bios_selected)	// BIOS ROM
			{
				if (!m_has_bios_0400 && !m_has_bios_2000)
				{
					m_bank_enabled[2] = ENABLE_BIOS;
					m_bios_page[2] = m_mapper[3];
				}
			}
			else if (cartridge_selected)	// CART ROM/RAM
			{
				m_bank_enabled[2] = ENABLE_CART;
				m_cartslot->write_mapper(space, offset, data);
			}
			break;
			
		case 1: // Select 16k ROM bank for 0400-3fff
		case 2: // Select 16k ROM bank for 4000-7fff
		case 3: // Select 16k ROM bank for 8000-bfff
			if (bios_selected)
			{
				if (!m_has_bios_0400 && !m_has_bios_2000)
				{
					m_bank_enabled[offset - 1] = ENABLE_BIOS;
					m_bios_page[offset - 1] = data & (m_bios_page_count - 1);
				}
			}
			else if (cartridge_selected || m_is_gamegear)
			{
				m_bank_enabled[offset - 1] = ENABLE_CART;
				m_cartslot->write_mapper(space, offset, data);
			}
			break;
	}
}


READ8_MEMBER(sms_state::read_0000)
{
	if (offset < 0x400)
	{
		if (m_bank_enabled[3] == ENABLE_BIOS)
		{
			if (m_BIOS)
				return m_BIOS[(m_bios_page[3] * 0x4000) + (offset & 0x3fff)];
		}
		if (m_bank_enabled[3] == ENABLE_CART)
			return m_cartslot->read_cart(space, offset); 
		if (m_card && m_bank_enabled[3] == ENABLE_CARD)
			return m_card->read_cart(space, offset); 
	}
	else
	{
		if (m_bank_enabled[0] == ENABLE_BIOS)
		{
			if (m_BIOS)
				return m_BIOS[(m_bios_page[0] * 0x4000) + (offset & 0x3fff)];
		}
		if (m_bank_enabled[0] == ENABLE_CART)
			return m_cartslot->read_cart(space, offset); 
		if (m_card && m_bank_enabled[0] == ENABLE_CARD)
			return m_card->read_cart(space, offset); 
	}	
	return m_region_maincpu->base()[offset];
}

READ8_MEMBER(sms_state::read_4000)
{
	if (m_bank_enabled[1] == ENABLE_BIOS)
	{
		if (m_BIOS)
			return m_BIOS[(m_bios_page[1] * 0x4000) + (offset & 0x3fff)];
	}

	if (m_bank_enabled[1] == ENABLE_CART)
		return m_cartslot->read_cart(space, offset + 0x4000); 
	if (m_card && m_bank_enabled[1] == ENABLE_CARD)
		return m_card->read_cart(space, offset + 0x4000); 

	return m_region_maincpu->base()[offset];
}

READ8_MEMBER(sms_state::read_8000)
{
	if (m_bank_enabled[2] == ENABLE_BIOS)
	{
		if (m_BIOS)
			return m_BIOS[(m_bios_page[2] * 0x4000) + (offset & 0x3fff)];
	}

	if (m_bank_enabled[2] == ENABLE_CART)
		return m_cartslot->read_cart(space, offset + 0x8000); 
	if (m_card && m_bank_enabled[2] == ENABLE_CARD)
		return m_card->read_cart(space, offset + 0x8000); 

	return m_region_maincpu->base()[offset];
}

WRITE8_MEMBER(sms_state::write_cart)
{
	m_cartslot->write_cart(space, offset, data);
}

READ8_MEMBER(smssdisp_state::store_read_0000)
{
	if (offset < 0x400)
	{
		if (m_bank_enabled[3] == ENABLE_BIOS)
		{
			if (m_BIOS)
				return m_BIOS[(m_bios_page[3] * 0x4000) + (offset & 0x3fff)];
		}
		if (m_bank_enabled[3] == ENABLE_CART)
			return m_slots[m_current_cartridge]->read_cart(space, offset); 
		if (m_bank_enabled[3] == ENABLE_CARD)
			return m_cards[m_current_cartridge]->read_cart(space, offset); 
	}
	else
	{
		if (m_bank_enabled[0] == ENABLE_BIOS)
		{
			if (m_BIOS)
				return m_BIOS[(m_bios_page[0] * 0x4000) + (offset & 0x3fff)];
		}
		if (m_bank_enabled[0] == ENABLE_CART)
			return m_slots[m_current_cartridge]->read_cart(space, offset); 
		if (m_bank_enabled[0] == ENABLE_CARD)
			return m_cards[m_current_cartridge]->read_cart(space, offset); 
	}	

	return m_region_maincpu->base()[offset];
}

READ8_MEMBER(smssdisp_state::store_read_4000)
{
	if (m_bank_enabled[1] == ENABLE_BIOS)
	{
		if (m_BIOS)
			return m_BIOS[(m_bios_page[1] * 0x4000) + (offset & 0x3fff)];
	}
	if (m_bank_enabled[1] == ENABLE_CART)
		return m_slots[m_current_cartridge]->read_cart(space, offset + 0x4000); 
	if (m_bank_enabled[1] == ENABLE_CARD)
		return m_cards[m_current_cartridge]->read_cart(space, offset + 0x4000); 
	
	return m_region_maincpu->base()[offset];
}

READ8_MEMBER(smssdisp_state::store_read_8000)
{
	if (m_bank_enabled[2] == ENABLE_BIOS)
	{
		if (m_BIOS)
			return m_BIOS[(m_bios_page[2] * 0x4000) + (offset & 0x3fff)];
	}	
	if (m_bank_enabled[2] == ENABLE_CART)
		return m_slots[m_current_cartridge]->read_cart(space, offset + 0x8000); 
	if (m_bank_enabled[2] == ENABLE_CARD)
		return m_cards[m_current_cartridge]->read_cart(space, offset + 0x8000); 
	
	return m_region_maincpu->base()[offset];
}

WRITE8_MEMBER(smssdisp_state::store_write_cart)
{
	// this might only work because we are not emulating properly the cart/card selection system
	// it will have to be reviewed when proper emulation is worked on!
	if (m_bank_enabled[0] == ENABLE_CART)
		m_slots[m_current_cartridge]->write_cart(space, offset, data); 
	if (m_bank_enabled[0] == ENABLE_CARD)
		m_cards[m_current_cartridge]->write_cart(space, offset, data); 
}

READ8_MEMBER(smssdisp_state::store_cart_peek)
{
	if (m_bank_enabled[1] == ENABLE_CART)
		return m_slots[m_current_cartridge]->read_cart(space, 0x4000 + (offset & 0x1fff)); 
	if (m_bank_enabled[1] == ENABLE_CARD)
		return m_cards[m_current_cartridge]->read_cart(space, 0x4000 + (offset & 0x1fff)); 
	
	return m_region_maincpu->base()[offset];
}

WRITE8_MEMBER(sms_state::sms_bios_w)
{
	m_bios_port = data;

	logerror("bios write %02x, pc: %04x\n", data, space.device().safe_pc());

	setup_rom();
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


void sms_state::setup_rom()
{
	m_bank_enabled[3] = ENABLE_NONE;
	m_bank_enabled[0] = ENABLE_NONE;
	m_bank_enabled[1] = ENABLE_NONE;
	m_bank_enabled[2] = ENABLE_NONE;

	/* 2. check and set up expansion port */
	if (!(m_bios_port & IO_EXPANSION) && (m_bios_port & IO_CARTRIDGE) && (m_bios_port & IO_CARD))
	{
		/* TODO: Implement me */
		m_bank_enabled[3] = ENABLE_EXPANSION;
		m_bank_enabled[0] = ENABLE_EXPANSION;
		m_bank_enabled[1] = ENABLE_EXPANSION;
		m_bank_enabled[2] = ENABLE_EXPANSION;
		logerror("Switching to unsupported expansion port.\n");
	}

	/* 3. check and set up card rom */
	if (!(m_bios_port & IO_CARD) && (m_bios_port & IO_CARTRIDGE) && (m_bios_port & IO_EXPANSION))
	{
		/* TODO: Implement me */
		m_bank_enabled[3] = ENABLE_CARD;
		m_bank_enabled[0] = ENABLE_CARD;
		m_bank_enabled[1] = ENABLE_CARD;
		m_bank_enabled[2] = ENABLE_CARD;
		logerror("Switching to unsupported card rom port.\n");
	}

	/* 4. check and set up cartridge rom */
	/* if ((!(bios_port & IO_CARTRIDGE) && (bios_port & IO_EXPANSION) && (bios_port & IO_CARD)) || state->m_is_gamegear) { */
	/* Out Run Europa initially writes a value to port 3E where IO_CARTRIDGE, IO_EXPANSION and IO_CARD are reset */
	if ((!(m_bios_port & IO_CARTRIDGE)) || m_is_gamegear)
	{
		m_bank_enabled[3] = ENABLE_CART;
		m_bank_enabled[0] = ENABLE_CART;
		m_bank_enabled[1] = ENABLE_CART;
		m_bank_enabled[2] = ENABLE_CART;
		logerror("Switched in cartridge rom.\n");
	}

	/* 5. check and set up bios rom */
	if (!(m_bios_port & IO_BIOS_ROM))
	{
		if (m_has_bios_0400)
		{
			// 1K bios
			m_bank_enabled[3] = ENABLE_BIOS;
			logerror("Switched in 0x0400 bios.\n");
		}
		if (m_has_bios_2000)
		{
			// 8K bios
			m_bank_enabled[3] = ENABLE_BIOS;
			m_bank_enabled[0] = ENABLE_BIOS;
			logerror("Switched in 0x2000 bios.\n");
		}
		if (m_has_bios_full)
		{
			// larger bios
			m_bank_enabled[3] = ENABLE_BIOS;
			m_bank_enabled[0] = ENABLE_BIOS;
			m_bank_enabled[1] = ENABLE_BIOS;
			m_bank_enabled[2] = ENABLE_BIOS;
			logerror("Switched in full bios.\n");
		}
	}
}


void sms_state::setup_bios()
{
	m_BIOS = memregion("user1")->base();
	m_bios_page_count = (m_BIOS ? memregion("user1")->bytes() / 0x4000 : 0);

	if (m_BIOS == NULL || m_BIOS[0] == 0x00)
	{
		m_BIOS = NULL;
		m_bios_port |= IO_BIOS_ROM;
		m_has_bios_0400 = 0;
		m_has_bios_2000 = 0;
		m_has_bios_full = 0;
		m_has_bios = 0;
	}

	if (m_BIOS)
	{
		m_bios_page[3] = 0;
		m_bios_page[0] = 0;
		m_bios_page[1] = (1 < m_bios_page_count) ? 1 : 0;
		m_bios_page[2] = (2 < m_bios_page_count) ? 2 : 0;
	}
}

MACHINE_START_MEMBER(sms_state,sms)
{
	char str[7];
	
	m_rapid_fire_timer = timer_alloc(TIMER_RAPID_FIRE);
	m_rapid_fire_timer->adjust(attotime::from_hz(10), 0, attotime::from_hz(10));

	m_lphaser_1_timer = timer_alloc(TIMER_LPHASER_1);
	m_lphaser_2_timer = timer_alloc(TIMER_LPHASER_2);

	m_left_lcd = machine().device("left_lcd");
	m_right_lcd = machine().device("right_lcd");
	m_space = &m_maincpu->space(AS_PROGRAM);

	/* Check if lightgun has been chosen as input: if so, enable crosshair */
	timer_set(attotime::zero, TIMER_LIGHTGUN_TICK);

	// alibaba and blockhol are ports of games for the MSX system. The
	// MSX bios usually initializes callback "vectors" at the top of RAM.
	// The code in alibaba does not do this so the IRQ vector only contains
	// the "call $4010" without a following RET statement. That is basically
	// a bug in the program code. The only way this cartridge could have run
	// successfully on a real unit is if the RAM would be initialized with
	// a F0 pattern on power up; F0 = RET P. Do that only for consoles in
	// Japan region (including KR), until confirmed on other consoles.
	if (m_is_region_japan)
	{
		memset((UINT8*)m_space->get_write_ptr(0xc000), 0xf0, 0x1fff);
	}

	save_item(NAME(m_fm_detect));
	save_item(NAME(m_ctrl_reg));
	save_item(NAME(m_paused));
	save_item(NAME(m_bios_port));
	save_item(NAME(m_mapper));
	save_item(NAME(m_input_port0));
	save_item(NAME(m_input_port1));
	save_item(NAME(m_gg_sio));
	save_item(NAME(m_bank_enabled));
	save_item(NAME(m_bios_page));

	save_item(NAME(m_rapid_fire_state_1));
	save_item(NAME(m_rapid_fire_state_2));
	save_item(NAME(m_last_paddle_read_time));
	save_item(NAME(m_paddle_read_state));
	save_item(NAME(m_last_sports_pad_time_1));
	save_item(NAME(m_last_sports_pad_time_2));
	save_item(NAME(m_sports_pad_state_1));
	save_item(NAME(m_sports_pad_state_2));
	save_item(NAME(m_sports_pad_last_data_1));
	save_item(NAME(m_sports_pad_last_data_2));
	save_item(NAME(m_sports_pad_1_x));
	save_item(NAME(m_sports_pad_1_y));
	save_item(NAME(m_sports_pad_2_x));
	save_item(NAME(m_sports_pad_2_y));
	save_item(NAME(m_lphaser_1_latch));
	save_item(NAME(m_lphaser_2_latch));
	save_item(NAME(m_sscope_state));
	save_item(NAME(m_frame_sscope_state));

	if (m_is_sdisp)
	{
		save_item(NAME(m_store_control));
		save_item(NAME(m_current_cartridge));

		m_slots[0] = m_cartslot;
		for (int i = 1; i < 16; i++)
		{
			sprintf(str,"slot%i",i + 1);
			m_slots[i] = machine().device<sega8_cart_slot_device>(str);
		}
		for (int i = 0; i < 16; i++)
		{
			sprintf(str,"slot%i",i + 16 + 1);
			m_cards[i] = machine().device<sega8_card_slot_device>(str);
		}
	}

	// a bunch of SG1000 carts (compatible with SG1000 Mark III) can access directly system RAM... let's install here the necessary handlers
	// TODO: are BASIC and Music actually compatible with Mark III??
	if (m_cartslot->get_type() == SEGA8_BASIC_L3 || m_cartslot->get_type() == SEGA8_MUSIC_EDITOR 
			|| m_cartslot->get_type() == SEGA8_DAHJEE_TYPEA || m_cartslot->get_type() == SEGA8_DAHJEE_TYPEB)
	{
		m_maincpu->space(AS_PROGRAM).install_read_handler(0xc000, 0xffff, 0, 0, read8_delegate(FUNC(sega8_cart_slot_device::read_ram),(sega8_cart_slot_device*)m_cartslot));
		m_maincpu->space(AS_PROGRAM).install_write_handler(0xc000, 0xffff, 0, 0, write8_delegate(FUNC(sega8_cart_slot_device::write_ram),(sega8_cart_slot_device*)m_cartslot));
	}
}

MACHINE_RESET_MEMBER(sms_state,sms)
{
	m_ctrl_reg = 0xff;
	if (m_has_fm)
		m_fm_detect = 0x01;

	m_bios_port = 0;

	if (m_cartslot->m_cart && m_cartslot->m_cart->get_sms_mode())
		m_vdp->set_sega315_5124_compatibility_mode(true);

	/* Initialize SIO stuff for GG */
	m_gg_sio[0] = 0x7f;
	m_gg_sio[1] = 0xff;
	m_gg_sio[2] = 0x00;
	m_gg_sio[3] = 0xff;
	m_gg_sio[4] = 0x00;

	if (m_is_sdisp)
	{
		m_store_control = 0;
		m_current_cartridge = 0;
	}

	setup_bios();

	setup_rom();

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
	m_lphaser_x_offs = (m_cartslot->m_cart) ? m_cartslot->m_cart->get_lphaser_xoffs() : 51;

	m_sscope_state = 0;
	m_frame_sscope_state = 0;
}

READ8_MEMBER(smssdisp_state::sms_store_cart_select_r)
{
	return 0xff;
}


WRITE8_MEMBER(smssdisp_state::sms_store_cart_select_w)
{
	UINT8 slot = data >> 4;
	UINT8 slottype = data & 0x08;

	logerror("switching in part of %s slot #%d\n", slottype ? "card" : "cartridge", slot );
	/* cartridge? slot #0 */
//	if (slottype == 0)
//		m_current_cartridge = slot;

	setup_rom();
}


READ8_MEMBER(smssdisp_state::sms_store_select1)
{
	return 0xff;
}


READ8_MEMBER(smssdisp_state::sms_store_select2)
{
	return 0xff;
}


READ8_MEMBER(smssdisp_state::sms_store_control_r)
{
	return m_store_control;
}


WRITE8_MEMBER(smssdisp_state::sms_store_control_w)
{
	logerror("0x%04X: sms_store_control write 0x%02X\n", space.device().safe_pc(), data);
	if (data & 0x02)
	{
		m_maincpu->resume(SUSPEND_REASON_HALT);
	}
	else
	{
		/* Pull reset line of CPU #0 low */
		m_maincpu->reset();
		m_maincpu->suspend(SUSPEND_REASON_HALT, 1);
	}
	m_store_control = data;
}

WRITE_LINE_MEMBER(smssdisp_state::sms_store_int_callback)
{
	if ( m_store_control & 0x01 )
	{
		m_control_cpu->set_input_line( 0, state );
	}
	else
	{
		m_maincpu->set_input_line( 0, state );
	}
}

void sms_state::setup_sms_cart()
{
	m_bios_port = (IO_EXPANSION | IO_CARTRIDGE | IO_CARD);
	if (!m_is_gamegear && !m_has_bios)
	{
		m_bios_port &= ~(IO_CARTRIDGE);
		m_bios_port |= IO_BIOS_ROM;
	}
}


DRIVER_INIT_MEMBER(sms_state,sg1000m3)
{
	m_is_region_japan = 1;
	m_has_fm = 1;
	setup_sms_cart();
}


DRIVER_INIT_MEMBER(sms_state,sms1)
{
	m_has_bios_full = 1;
	setup_sms_cart();
}


DRIVER_INIT_MEMBER(sms_state,smsj)
{
	m_is_region_japan = 1;
	m_has_bios_2000 = 1;
	m_has_fm = 1;
	setup_sms_cart();
}


DRIVER_INIT_MEMBER(sms_state,sms2kr)
{
	m_is_region_japan = 1;
	m_has_bios_full = 1;
	m_has_fm = 1;
	setup_sms_cart();
}


DRIVER_INIT_MEMBER(smssdisp_state,smssdisp)
{
	m_is_sdisp = 1;
	setup_sms_cart();
}


DRIVER_INIT_MEMBER(sms_state,gamegear)
{
	m_is_gamegear = 1;
	m_has_bios_0400 = 1;
	setup_sms_cart();
}


DRIVER_INIT_MEMBER(sms_state,gamegeaj)
{
	m_is_region_japan = 1;
	m_is_gamegear = 1;
	m_has_bios_0400 = 1;
	setup_sms_cart();
}


VIDEO_START_MEMBER(sms_state,sms1)
{
	m_main_scr->register_screen_bitmap(m_prevleft_bitmap);
	m_main_scr->register_screen_bitmap(m_prevright_bitmap);
	save_item(NAME(m_prevleft_bitmap));
	save_item(NAME(m_prevright_bitmap));
}


void sms_state::screen_vblank_sms1(screen_device &screen, bool state)
{
	// on falling edge
	if (!state)
	{
		// Most of the 3-D games usually set Sega Scope's state for next frame
		// soon after the active area of current frame was drawn, but before
		// it is displayed by the screen update function. That function needs to
		// check the state used at the time of frame drawing, to display it on
		// the correct side. So here, when the frame is about to be drawn, the
		// Sega Scope's state is stored, to be checked by that function.
		m_frame_sscope_state = m_sscope_state;
	}
}


UINT32 sms_state::screen_update_sms1(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT8 sscope = 0;
	UINT8 sscope_binocular_hack;
	UINT8 occluded_view = 0;

	if (&screen != m_main_scr)
	{
		sscope = m_port_scope->read_safe(0x00);
		if (!sscope)
		{
			// without SegaScope, both LCDs for glasses go black
			occluded_view = 1;
		}
		else if (&screen == m_left_lcd)
		{
			// with SegaScope, state 0 = left screen OFF, right screen ON
			if (!(m_frame_sscope_state & 0x01))
				occluded_view = 1;
		}
		else // it's right LCD
		{
			// with SegaScope, state 1 = left screen ON, right screen OFF
			if (m_frame_sscope_state & 0x01)
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
			sscope_binocular_hack = ioport("SSCOPE_BINOCULAR")->read_safe(0x00);

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
			sscope_binocular_hack = ioport("SSCOPE_BINOCULAR")->read_safe(0x00);

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
	m_main_scr->register_screen_bitmap(m_prev_bitmap);
	save_item(NAME(m_prev_bitmap));
}

UINT32 sms_state::screen_update_gamegear(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int x, y;
	bitmap_rgb32 &vdp_bitmap = m_vdp->get_bitmap();

	if (!m_port_persist->read())
	{
		copybitmap(bitmap, vdp_bitmap, 0, 0, 0, 0, cliprect);
		copybitmap(m_prev_bitmap, vdp_bitmap, 0, 0, 0, 0, cliprect);
	}
	else
	{
		// HACK: fake LCD persistence effect
		// (it would be better to generalize this in the core, to be used for all LCD systems)
		for (y = cliprect.min_y; y <= cliprect.max_y; y++)
		{
			UINT32 *linedst = &bitmap.pix32(y);
			UINT32 *line0 = &vdp_bitmap.pix32(y);
			UINT32 *line1 = &m_prev_bitmap.pix32(y);
			for (x = cliprect.min_x; x <= cliprect.max_x; x++)
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
	}
	return 0;
}
