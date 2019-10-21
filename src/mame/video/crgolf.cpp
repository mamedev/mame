// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Kitco Crowns Golf hardware

***************************************************************************/

#include "emu.h"
#include "includes/crgolf.h"


#define NUM_PENS        (0x20)
#define VIDEORAM_SIZE   (0x2000 * 3)





/*************************************
 *
 *  Video startup
 *
 *************************************/

void crgolf_state::crgolf_palette(palette_device &palette) const
{
	uint8_t const *const prom = memregion("proms")->base();

	for (offs_t offs = 0; offs < NUM_PENS; offs++)
	{
		int bit0, bit1, bit2;

		uint8_t const data = prom[offs];

		// red component
		bit0 = BIT(data, 0);
		bit1 = BIT(data, 1);
		bit2 = BIT(data, 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// green component
		bit0 = BIT(data, 3);
		bit1 = BIT(data, 4);
		bit2 = BIT(data, 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// blue component
		bit0 = BIT(data, 6);
		bit1 = BIT(data, 7);
		int const b = 0x4f * bit0 + 0xa8 * bit1;

		m_palette->set_pen_color(offs, r, g, b);
	}
}

void crgolf_state::mastrglf_palette(palette_device &palette) const
{
}

/*************************************
 *
 *  Video update
 *
 *************************************/

uint32_t crgolf_state::screen_update_crgolf(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int flip = m_screen_flip;

	offs_t offs;

	/* for each byte in the video RAM */
	for (offs = 0; offs < VIDEORAM_SIZE / 3; offs++)
	{
		int i;

		uint8_t y = (offs & 0x1fe0) >> 5;
		uint8_t x = (offs & 0x001f) << 3;

		uint8_t data_a0 = m_videoram_a[0x2000 | offs];
		uint8_t data_a1 = m_videoram_a[0x0000 | offs];
		uint8_t data_a2 = m_videoram_a[0x4000 | offs];
		uint8_t data_b0 = m_videoram_b[0x2000 | offs];
		uint8_t data_b1 = m_videoram_b[0x0000 | offs];
		uint8_t data_b2 = m_videoram_b[0x4000 | offs];

		if (flip)
		{
			y = ~y;
			x = ~x;
		}

		/* for each pixel in the byte */
		for (i = 0; i < 8; i++)
		{
			offs_t color;
			uint8_t data_b = 0;
			uint8_t data_a = 0;

			if (!m_screena_enable)
				data_a = ((data_a0 & 0x80) >> 7) | ((data_a1 & 0x80) >> 6) | ((data_a2 & 0x80) >> 5);

			if (!m_screenb_enable)
				data_b = ((data_b0 & 0x80) >> 7) | ((data_b1 & 0x80) >> 6) | ((data_b2 & 0x80) >> 5);

			/* screen A has priority over B */
			if (data_a)
				color = data_a;
			else
				color = data_b | 0x08;

			/* add HI bit if enabled */
			if (m_color_select)
				color = color | 0x10;

			bitmap.pix16(y, x) = color;

			/* next pixel */
			data_a0 = data_a0 << 1;
			data_a1 = data_a1 << 1;
			data_a2 = data_a2 << 1;
			data_b0 = data_b0 << 1;
			data_b1 = data_b1 << 1;
			data_b2 = data_b2 << 1;

			if (flip)
				x = x - 1;
			else
				x = x + 1;
		}
	}

	return 0;
}
