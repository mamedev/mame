// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "tutankhm.h"
#include "video/resnet.h"

static constexpr uint32_t STAR_RNG_PERIOD = (1 << 17) - 1;
static constexpr unsigned RGB_MAXIMUM     = 224;

/*************************************
 *
 *  Write handlers
 *
 *************************************/

void tutankhm_state::flip_screen_x_w(int state)
{
	m_flipscreen_x = state;
}


void tutankhm_state::flip_screen_y_w(int state)
{
	m_flipscreen_y = state;
}


/*************************************
 *
 *  Video update
 *
 *************************************/

uint32_t tutankhm_state::screen_update_bootleg(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t::black(), cliprect);

	int const xorx = m_flipscreen_x ? 255 : 0;
	int const xory = m_flipscreen_y ? 255 : 0;

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		uint32_t *const dst = &bitmap.pix(y);

		for (int x = cliprect.min_x / GALAXIAN_XSCALE; x <= cliprect.max_x / GALAXIAN_XSCALE; x++)
		{
			uint8_t const effx = x ^ xorx;
			uint8_t const yscroll = (effx < 192 && m_scroll.found()) ? *m_scroll : 0;
			uint8_t const effy = (y ^ xory) + yscroll;
			uint8_t const vrambyte = m_videoram[effy * 128 + effx / 2];
			uint8_t const shifted = vrambyte >> (4 * (effx & 1));

			uint8_t const blink_state = m_stars_blink_state & 3;
			bool enab = false;
			switch (blink_state)
			{
				case 0: enab = true; break;
				case 1: enab = BIT(y, 0); break;
				case 2: enab = BIT(y, 1); break;
				case 3: enab = BIT(~x, 3); break;
			}
			//enab &= (((y>>1) ^ (x >> 3)) & 1);

			int const offset = y * 384 + x + 84;

			uint8_t const star = m_stars[offset % STAR_RNG_PERIOD];
			if (m_stars_enabled && enab && BIT(~shifted, 1) && BIT(star, 7)
				&& x > 63)
			{
				bitmap.pix(y, GALAXIAN_XSCALE*x + 0) = m_star_color[star & 0x3f];
				bitmap.pix(y, GALAXIAN_XSCALE*x + 1) = m_star_color[star & 0x3f];
				bitmap.pix(y, GALAXIAN_XSCALE*x + 2) = m_star_color[star & 0x3f];
			}

			else
			{
				auto color = m_palette->pen_color(shifted & 0x0f);
				u32 *const dbase = dst + x * GALAXIAN_XSCALE;
				if(shifted || dbase[0] == 0xff000000) dbase[0] = color;
				if(shifted || dbase[1] == 0xff000000) dbase[1] = color;
				if(shifted || dbase[2] == 0xff000000) dbase[2] = color;
			}
		}
	}

	return 0;
}

uint32_t tutankhm_state::screen_update_scramble(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	scramble_draw_background(bitmap, cliprect);

	int const xorx = m_flipscreen_x ? 255 : 0;
	int const xory = m_flipscreen_y ? 255 : 0;

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		uint32_t *const dst = &bitmap.pix(y);

		for (int x = cliprect.min_x / GALAXIAN_XSCALE; x <= cliprect.max_x / GALAXIAN_XSCALE; x++)
		{
			uint8_t const effx = x ^ xorx;
			uint8_t const yscroll = (effx < 192 && m_scroll.found()) ? *m_scroll : 0;
			uint8_t const effy = (y ^ xory) + yscroll;
			uint8_t const vrambyte = m_videoram[effy * 128 + effx / 2];
			uint8_t const shifted = vrambyte >> (4 * (effx & 1));
			auto color = m_palette->pen_color(shifted & 0x0f);
			u32 *const dbase = dst + x * GALAXIAN_XSCALE;
			if(shifted || dbase[0] == 0xff000000) dbase[0] = color;
			if(shifted || dbase[1] == 0xff000000) dbase[1] = color;
			if(shifted || dbase[2] == 0xff000000) dbase[2] = color;
		}
	}

	return 0;
}

uint32_t tutankhm_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	u8 const mode = m_stars_config.read_safe(m_star_mode);
	if (mode != m_star_mode)
	{
		m_star_mode = mode;
		stars_init();
	}

	if (m_star_mode)
		return screen_update_scramble(screen, bitmap, cliprect);
	else
		return screen_update_bootleg(screen, bitmap, cliprect);
}

/*************************************
 *
 *  Copied from galaxian.cpp video code
 *
 *************************************/

rgb_t tutankhm_state::raw_to_rgb_func(u32 raw)
{
	static const int rgb_resistances[3] = { 1000, 470, 220 };
	/*
	    Sprite/tilemap colors are mapped through a color PROM as follows:

	      bit 7 -- 220 ohm resistor  -- BLUE
	            -- 470 ohm resistor  -- BLUE
	            -- 220 ohm resistor  -- GREEN
	            -- 470 ohm resistor  -- GREEN
	            -- 1  kohm resistor  -- GREEN
	            -- 220 ohm resistor  -- RED
	            -- 470 ohm resistor  -- RED
	      bit 0 -- 1  kohm resistor  -- RED

	    Note that not all boards have this configuration. Namco PCBs may
	    have 330 ohm resistors instead of 220, but the default setup has
	    also been used by Namco.

	    In parallel with these resistors are a pair of 150 ohm and 100 ohm
	    resistors on each R,G,B component that are connected to the star
	    generator.

	    And in parallel with the whole mess are a set of 100 ohm resistors
	    on each R,G,B component that are enabled when a shell/missile is
	    enabled.

	    When computing weights, we use RGB_MAXIMUM as the maximum to give
	    headroom for stars and shells/missiles. This is not fully accurate,
	    but if we included all possible sources in parallel, the brightness
	    of the main game would be very low to allow for all the oversaturation
	    of the stars and shells/missiles.
	*/
	double rweights[3], gweights[3], bweights[2];
	compute_resistor_weights(0, RGB_MAXIMUM, -1.0,
			3, &rgb_resistances[0], rweights, 470, 0,
			3, &rgb_resistances[0], gweights, 470, 0,
			2, &rgb_resistances[1], bweights, 470, 0);

	// decode the palette first

	uint8_t bit0, bit1, bit2;

	// red component
	bit0 = BIT(raw, 0);
	bit1 = BIT(raw, 1);
	bit2 = BIT(raw, 2);
	int const r = combine_weights(rweights, bit0, bit1, bit2);

	// green component
	bit0 = BIT(raw, 3);
	bit1 = BIT(raw, 4);
	bit2 = BIT(raw, 5);
	int const g = combine_weights(gweights, bit0, bit1, bit2);

	// blue component
	bit0 = BIT(raw, 6);
	bit1 = BIT(raw, 7);
	int const b = combine_weights(bweights, bit0, bit1);

	return rgb_t(r, g, b);
}

void tutankhm_state::galaxian_palette(palette_device &palette)
{
	/*
	    The maximum sprite/tilemap resistance is ~130 Ohms with all RGB
	    outputs enabled (1/(1/1000 + 1/470 + 1/220)). Since we normalized
	    to RGB_MAXIMUM, this maps RGB_MAXIMUM -> 130 Ohms.

	    The stars are at 150 Ohms for the LSB, and 100 Ohms for the MSB.
	    This means the 3 potential values are:

	        150 Ohms -> RGB_MAXIMUM * 130 / 150
	        100 Ohms -> RGB_MAXIMUM * 130 / 100
	         60 Ohms -> RGB_MAXIMUM * 130 / 60

	    Since we can't saturate that high, we instead approximate this
	    by compressing the values proportionally into the 194->255 range.
	*/
	int const minval = RGB_MAXIMUM * 130 / 150;
	int const midval = RGB_MAXIMUM * 130 / 100;
	int const maxval = RGB_MAXIMUM * 130 / 60;

	// compute the values for each of 4 possible star values
	uint8_t const starmap[4]{
			0,
			minval,
			minval + (255 - minval) * (midval - minval) / (maxval - minval),
			255 };

	// generate the colors for the stars
	for (int i = 0; i < 64; i++)
	{
		uint8_t bit0, bit1;

		// bit 5 = red @ 150 Ohm, bit 4 = red @ 100 Ohm
		bit0 = BIT(i, 5);
		bit1 = BIT(i, 4);
		int const r = starmap[(bit1 << 1) | bit0];

		// bit 3 = green @ 150 Ohm, bit 2 = green @ 100 Ohm
		bit0 = BIT(i, 3);
		bit1 = BIT(i, 2);
		int const g = starmap[(bit1 << 1) | bit0];

		// bit 1 = blue @ 150 Ohm, bit 0 = blue @ 100 Ohm
		bit0 = BIT(i, 1);
		bit1 = BIT(i, 0);
		int const b = starmap[(bit1 << 1) | bit0];

		// set the RGB color
		m_star_color[i] = rgb_t(r, g, b);
	}
}


void tutankhm_state::video_start()
{
	/* initialize globals */
	m_flipscreen_x = 0;
	m_flipscreen_y = 0;

	/* initialize stars */
	m_stars_enabled = 0;
	m_stars_blink_state = 0;
	stars_init();

	galaxian_palette(*m_palette);
}

void tutankhm_state::stars_init()
{
	(m_star_mode) ? stars_init_scramble() : stars_init_bootleg();
}

void tutankhm_state::stars_init_bootleg()
{
	/* reset the blink and enabled states */
	m_stars_enabled = false;
	m_stars_blink_state = 0;

	/* precalculate the RNG */
	m_stars = std::make_unique<uint8_t[]>(STAR_RNG_PERIOD);
	uint32_t shiftreg = 0;
	for (int i = 0; i < STAR_RNG_PERIOD; i++)
	{
		int const newbit = ((shiftreg >> 12) ^ ~shiftreg) & 1;

		/* stars are enabled if the upper 8 bits are 1 and the new bit is 0 */
		int const enabled = ((shiftreg & 0x1fe00) == 0x1fe00) && (newbit == 0);
		//int enabled = ((shiftreg & 0x1fe01) == 0x1fe00); // <- scramble

		/* color comes from the 6 bits below the top 8 bits */
		int const color = (~shiftreg & 0x1f8) >> 3;

		/* store the color value in the low 6 bits and the enable in the upper bit */
		m_stars[i] = color | (enabled << 7);

		/* the LFSR is fed based on the XOR of bit 12 and the inverse of bit 0 */
		shiftreg = (shiftreg >> 1) | (newbit << 16);
	}
}

void tutankhm_state::stars_init_scramble()
{
	/* precalculate the RNG */
	m_stars = std::make_unique<uint8_t[]>(STAR_RNG_PERIOD);
	uint32_t shiftreg = 0;
	for (int i = 0; i < STAR_RNG_PERIOD; i++)
	{
		uint8_t const shift = 12;
		/* stars are enabled if the upper 8 bits are 1 and the low bit is 0 */
		int const enabled = ((shiftreg & 0x1fe01) == 0x1fe00);

		/* color comes from the 6 bits below the top 8 bits */
		int const color = (~shiftreg & 0x1f8) >> 3;

		/* store the color value in the low 6 bits and the enable in the upper bit */
		m_stars[i] = color | (enabled << 7);

		/* the LFSR is fed based on the XOR of bit 12 and the inverse of bit 0 */
		//shiftreg = (shiftreg >> 1) | ((((shiftreg >> 12) ^ ~shiftreg) & 1) << 16);
		shiftreg = (shiftreg >> 1) | ((((shiftreg >> shift) ^ ~shiftreg) & 1) << 16);
	}
}



/*************************************
 *
 *  Star blinking
 *
 *************************************/

TIMER_DEVICE_CALLBACK_MEMBER(tutankhm_state::scramble_stars_blink_timer)
{
	m_stars_blink_state++;
}

void tutankhm_state::stars_draw_row(bitmap_rgb32 &bitmap, int maxx, int y, uint32_t star_offs)
{
	uint8_t const flipxor = (m_flipscreen_x ? 0xc0 : 0x00);

	/* ensure our star offset is valid */
	star_offs %= STAR_RNG_PERIOD;

	/* iterate over the specified number of 6MHz pixels */
	for (int x = 0; x < maxx; x++)
	{
		uint8_t const h8q = BIT(~x, 3); // H8 signal is inverted.
		/* stars are suppressed unless V1 ^ H8 == 1 */
		bool enable_star = BIT(y ^ h8q, 0);

		uint8_t const blink_state = m_stars_blink_state & 3;
		bool enab = false;
		switch (blink_state)
		{
			case 0: enab = true;      break;
			case 1: enab = BIT(y, 0); break;
			case 2: enab = BIT(y, 1); break;
			case 3: enab = h8q;       break; // H8 signal is inverted.
		}

		enable_star &= (enab && ((x & 0xc0) ^ flipxor) != 0xc0);

		/*
		    The RNG clock is the master clock (18MHz) ANDed with the pixel clock (6MHz).
		    The divide-by-3 circuit that produces the pixel clock generates a square wave
		    with a 2/3 duty cycle, so the result of the AND generates a clock like this:
		                _   _   _   _   _   _   _   _
		      MASTER: _| |_| |_| |_| |_| |_| |_| |_| |
		                _______     _______     ______
		      PIXEL:  _|       |___|       |___|
		                _   _       _   _       _   _
		      RNG:    _| |_| |_____| |_| |_____| |_| |

		    Thus for each pixel, there are 3 master clocks and 2 RNG clocks, and the RNG
		    is clocked asymmetrically. To simulate this, we expand the horizontal screen
		    size by 3 and handle the first RNG clock with one pixel and the second RNG
		    clock with two pixels.
		*/

		uint8_t star;
		/* first RNG clock: one pixel */
		star = m_stars[star_offs++];
		if (star_offs >= STAR_RNG_PERIOD)
			star_offs = 0;
		if (enable_star && BIT(star, 7))
			bitmap.pix(y, GALAXIAN_XSCALE*x + 0) = m_star_color[star & 0x3f];

		/* second RNG clock: two pixels */
		star = m_stars[star_offs++];
		if (star_offs >= STAR_RNG_PERIOD)
			star_offs = 0;
		if (enable_star && BIT(star, 7))
		{
			bitmap.pix(y, GALAXIAN_XSCALE*x + 1) = m_star_color[star & 0x3f];
			bitmap.pix(y, GALAXIAN_XSCALE*x + 2) = m_star_color[star & 0x3f];
		}
	}
}

void tutankhm_state::scramble_draw_stars(bitmap_rgb32 &bitmap, const rectangle &cliprect, int maxx)
{
	/* update the star origin to the current frame */
	//stars_update_origin();

	/* render stars if enabled */
	if (m_stars_enabled)
	{
		/* iterate over scanlines */
		for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
		{
			stars_draw_row(bitmap, maxx, y, y * 512);
		}
	}
}


void tutankhm_state::scramble_draw_background(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	/* blue background - 390 ohm resistor */
	bitmap.fill(rgb_t::black(), cliprect);

	scramble_draw_stars(bitmap, cliprect, 256);
}

void tutankhm_state::stars_enable_w(uint8_t data)
{
	if (BIT(m_stars_enabled ^ data, 0))
	{
//      m_screen->update_now();
		m_screen->update_partial(m_screen->vpos());
	}

	m_stars_enabled = BIT(data, 0);
}
