// license:BSD-3-Clause
// copyright-holders:Frank Palazzolo
/****************************************************************************

    Sega "Space Tactics" Driver

    Frank Palazzolo (palazzol@home.com)


The Video system used in Space Tactics is unusual.
Here are my notes on how the video system works:

There are 4, 4K pages of Video RAM. (B,D,E & F)

The first 1K of each VRAM page contains the following:
0 0 V V V V V H H H H H   offset value for each 8x8 bitmap
     (v-tile)  (h-tile)

The offset values are used to generate an access into the
last 2K of the VRAM page:
1 D D D D D D D D V V V   here we find 8x8 character data
     (offset)    (line)

In addition, in Page B, the upper nibble of the offset is
also used to select the color palette for the tile.

Page B, D, E, and F all work similarly, except that pages
D, E, and F can be offset from Page B by a given
number of scanlines, based on the contents of the offset
RAM

The offset RAM is addressed in this format:
1 0 0 0 P P P V V V V V V V V V
        (Page)   (scanline)
Page 4=D, 5=E, 6=F

Page D, E, and F are drawn offset from the top of the screen,
starting on the first scanline which contains a 1 in the
appropriate memory location

---

The composited monitor image is seen in a mirror.  It appears
to move when the player moves the handle, due to motors which
tilt the mirror up and down, and the monitor left and right.

---

***************************************************************************/

#include "emu.h"
#include "includes/stactics.h"
#include "screen.h"



/*************************************
 *
 *  Palette
 *
 *************************************/

void stactics_state::stactics_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	for (int i = 0; i < 0x400; i++)
	{
		int const bit0 = BIT(color_prom[i], 0);
		int const bit1 = BIT(color_prom[i], 1);
		int const bit2 = BIT(color_prom[i], 2);
		int const bit3 = BIT(color_prom[i], 3);

		// red component
		int const r = 0xff * bit0;

		// green component
		int const g = 0xff * bit1 - 0xcc * bit3;

		// blue component
		int const b = 0xff * bit2;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}


WRITE_LINE_MEMBER(stactics_state::palette_bank_w)
{
	m_palette_bank = m_outlatch->q6_r() | (m_outlatch->q7_r() << 1);
}



/*************************************
 *
 *  Scrolling
 *
 *************************************/

WRITE8_MEMBER(stactics_state::scroll_ram_w)
{
	if (data & 0x01)
	{
		switch (offset >> 8)
		{
			case 4: m_y_scroll_d = offset & 0xff; break;
			case 5: m_y_scroll_e = offset & 0xff; break;
			case 6: m_y_scroll_f = offset & 0xff; break;
		}
	}
}



/*************************************
 *
 *  Frane counter
 *
 *************************************/

READ_LINE_MEMBER(stactics_state::frame_count_d3_r)
{
	return (m_frame_count >> 3) & 0x01;
}



/*************************************
 *
 *  Beam handling
 *
 *************************************/

WRITE8_MEMBER(stactics_state::speed_latch_w)
{
	/* This writes to a shift register which is clocked by   */
	/* a 555 oscillator.  This value determines the speed of */
	/* the LED fire beams as follows:                        */

	/*   555_freq / bits_in_SR * edges_in_SR / states_in_PR67 / frame_rate */
	/*      = num_led_states_per_frame  */
	/*   36439 / 8 * x / 32 / 60 ~= 19/8*x */

	/* Here, we will count the number of rising edges in the shift register */

	int i;
	int num_rising_edges = 0;

	for(i=0;i<8;i++)
	{
		if ( (((data>>i)&0x01) == 1) && (((data>>((i+1)%8))&0x01) == 0))
			num_rising_edges++;
	}

	m_beam_states_per_frame = num_rising_edges*19/8;
}


WRITE8_MEMBER(stactics_state::shot_trigger_w)
{
	m_shot_standby = 0;
}


WRITE8_MEMBER(stactics_state::shot_flag_clear_w)
{
	m_shot_arrive = 0;
}


READ_LINE_MEMBER(stactics_state::shot_standby_r)
{
	return m_shot_standby;
}


READ_LINE_MEMBER(stactics_state::not_shot_arrive_r)
{
	return !m_shot_arrive;
}


void stactics_state::update_beam()
{
	/* first, update the firebeam state */
	m_old_beam_state = m_beam_state;
	if (m_shot_standby == 0)
		m_beam_state = m_beam_state + m_beam_states_per_frame;

	/* These are thresholds for the two shots from the LED fire ROM */
	/* (Note: There are two more for sound triggers, */
	/*        whenever that gets implemented)        */
	if ((m_old_beam_state < 0x8b) & (m_beam_state >= 0x8b))
		m_shot_arrive = 1;

	if ((m_old_beam_state < 0xca) & (m_beam_state >= 0xca))
		m_shot_arrive = 1;

	if (m_beam_state >= 0x100)
	{
		m_beam_state = 0;
		m_shot_standby = 1;
	}
}



/*************************************
 *
 *  Video drawing
 *
 *************************************/

inline int stactics_state::get_pixel_on_plane(uint8_t *videoram, uint8_t y, uint8_t x, uint8_t y_scroll)
{
	uint8_t code;
	uint8_t gfx;

	/* compute effective row */
	y = y - y_scroll;

	/* get the character code at the given pixel */
	code = videoram[((y >> 3) << 5) | (x >> 3)];

	/* get the gfx byte */
	gfx = videoram[0x800 | (code << 3) | (y & 0x07)];

	/* return the appropriate pixel within the byte */
	return (gfx >> (~x & 0x07)) & 0x01;
}


void stactics_state::draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int y;

	bitmap.fill(0, cliprect);

	/* for every row */
	for (y = 0; y < 0x100; y++)
	{
		int x;

		/* for every pixel on the row */
		for (x = 0; x < 0x100; x++)
		{
			/* get the pixels for the four planes */
			int pixel_b = get_pixel_on_plane(m_videoram_b, y, x, 0);
			int pixel_d = get_pixel_on_plane(m_videoram_d, y, x, m_y_scroll_d);
			int pixel_e = get_pixel_on_plane(m_videoram_e, y, x, m_y_scroll_e);
			int pixel_f = get_pixel_on_plane(m_videoram_f, y, x, m_y_scroll_f);

			/* get the color for this pixel */
			uint8_t color = m_videoram_b[((y >> 3) << 5) | (x >> 3)] >> 4;

			/* assemble the pen index */
			int pen = color |
						(pixel_b << 4) |
						(pixel_f << 5) |
						(pixel_e << 6) |
						(pixel_d << 7) |
						(m_palette_bank << 8);

			/* compute the effective pixel coordinate after adjusting for the
			   mirror movement - this is mechanical on the real machine */
			int sy = y + m_vert_pos;
			int sx = x - m_horiz_pos;

			/* plot if visible */
			if ((sy >= 0) && (sy < 0x100) && (sx >= 0) && (sx < 0x100))
				bitmap.pix16(sy, sx) = pen;
		}
	}
}



/*************************************
 *
 *  Non-video artwork
 *
 *************************************/

/* from 7448 datasheet */
static constexpr int to_7seg[0x10] =
{
	0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07,
	0x7f, 0x67, 0x58, 0x4c, 0x62, 0x69, 0x78, 0x00
};


template <unsigned N> void stactics_state::set_indicator_leds(unsigned offset, output_finder<N> &outputs, int base_index)
{
	/* decode the data */
	int const data = to_7seg[~m_display_buffer[offset] & 0x0f];

	/* set the 4 LEDs */
	outputs[base_index + 0] = BIT(data, 2);
	outputs[base_index + 1] = BIT(data, 6);
	outputs[base_index + 2] = BIT(data, 5);
	outputs[base_index + 3] = BIT(data, 4);
}


WRITE_LINE_MEMBER(stactics_state::barrier_lamp_w)
{
	// this needs to flash on/off, not implemented
	m_barrier_lamp = state;
}


WRITE_LINE_MEMBER(stactics_state::start_lamp_w)
{
	m_start_lamp = state;
}


void stactics_state::update_artwork()
{
	uint8_t *beam_region = memregion("user1")->base();

	/* laser beam - loop for each LED */
	for (int i = 0; i < 0x40; i++)
	{
		offs_t const beam_data_offs = ((i & 0x08) << 7) | ((i & 0x30) << 4) | m_beam_state;
		uint8_t const beam_data = beam_region[beam_data_offs];
		int const on = BIT(beam_data, i & 0x07);

		m_beam_leds_left[i] = on;
		m_beam_leds_right[i] = on;
	}

	/* sight LED */
	m_sight_led = m_motor_on;

	/* score display */
	for (int i = 0x01; i < 0x07; i++)
		m_score_digits[i - 1] = to_7seg[~m_display_buffer[i] & 0x0f];

	/* credits indicator */
	set_indicator_leds(0x07, m_credit_leds, 0x00);
	set_indicator_leds(0x08, m_credit_leds, 0x04);

	/* barriers indicator */
	set_indicator_leds(0x09, m_barrier_leds, 0x00);
	set_indicator_leds(0x0a, m_barrier_leds, 0x04);
	set_indicator_leds(0x0b, m_barrier_leds, 0x08);

	/* rounds indicator */
	set_indicator_leds(0x0c, m_round_leds, 0x00);
	set_indicator_leds(0x0d, m_round_leds, 0x04);
	set_indicator_leds(0x0e, m_round_leds, 0x08);
	set_indicator_leds(0x0f, m_round_leds, 0x0c);
}



/*************************************
 *
 *  Start
 *
 *************************************/

void stactics_state::video_start()
{
	m_base_lamps.resolve();
	m_beam_leds_left.resolve();
	m_beam_leds_right.resolve();
	m_score_digits.resolve();
	m_credit_leds.resolve();
	m_barrier_leds.resolve();
	m_round_leds.resolve();
	m_barrier_lamp.resolve();
	m_start_lamp.resolve();
	m_sight_led.resolve();

	m_y_scroll_d = 0;
	m_y_scroll_e = 0;
	m_y_scroll_f = 0;

	m_frame_count = 0;
	m_shot_standby = 1;
	m_shot_arrive = 0;
	m_beam_state = 0;
	m_old_beam_state = 0;

	m_palette_bank = 0;

	save_item(NAME(m_y_scroll_d));
	save_item(NAME(m_y_scroll_e));
	save_item(NAME(m_y_scroll_f));
	save_item(NAME(m_frame_count));
	save_item(NAME(m_shot_standby));
	save_item(NAME(m_shot_arrive));
	save_item(NAME(m_beam_state));
	save_item(NAME(m_old_beam_state));
	save_item(NAME(m_beam_states_per_frame));
	save_item(NAME(m_palette_bank));
}



/*************************************
 *
 *  Update
 *
 *************************************/

uint32_t stactics_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	update_beam();
	draw_background(bitmap, cliprect);
	update_artwork();

	m_frame_count = (m_frame_count + 1) & 0x0f;

	return 0;
}



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void stactics_state::stactics_video(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_video_attributes(VIDEO_ALWAYS_UPDATE);
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 0*8, 30*8-1);
	screen.set_screen_update(FUNC(stactics_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(stactics_state::stactics_palette), 0x400);
}
