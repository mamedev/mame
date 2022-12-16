// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

  route16.cpp

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "route16.h"

void route16_state::video_start()
{
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_palette_1));
	save_item(NAME(m_palette_2));
}

/*************************************
 *
 *  Memory handlers
 *
 *************************************/

void route16_state::out0_w(uint8_t data)
{
	m_palette_1 = data & 0x1f;

	machine().bookkeeping().coin_counter_w(0, (data >> 5) & 0x01);
}


void route16_state::out1_w(uint8_t data)
{
	m_palette_2 = data & 0x1f;

	m_flipscreen = (data >> 5) & 0x01;
}



/*************************************
 *
 *  Video update
 *
 *************************************/

/*
 *  Game observation shows that Route 16 can blank each
 *  bitmap by setting bit 1 of the palette register.
 *  Since the schematics are missing the relevant pages, I
 *  cannot confirm how this works, but I am 99% sure the bit 1
 *  would be connected to A7 of the color PROM.  Since the
 *  color PROMs contain 0 in the upper half, this would produce
 *  a black output.
 */

uint32_t route16_state::screen_update_route16(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint8_t *color_prom1 = &memregion("proms")->base()[0x000];
	uint8_t *color_prom2 = &memregion("proms")->base()[0x100];

	for (offs_t offs = 0; offs < m_videoram1.bytes(); offs++)
	{
		uint8_t y = offs >> 6;
		uint8_t x = offs << 2;

		uint8_t data1 = m_videoram1[offs];
		uint8_t data2 = m_videoram2[offs];

		for (int i = 0; i < 4; i++)
		{
			uint8_t color1 = color_prom1[((m_palette_1 << 6) & 0x80) |
										(m_palette_1 << 2) |
										((data1 >> 3) & 0x02) |
										((data1 >> 0) & 0x01)];

			/* bit 7 of the 2nd color is the OR of the 1st color bits 0 and 1 - this is a guess */
			uint8_t color2 = color_prom2[((m_palette_2 << 6) & 0x80) | (((color1 << 6) & 0x80) | ((color1 << 7) & 0x80)) |
										(m_palette_2 << 2) |
										((data2 >> 3) & 0x02) |
										((data2 >> 0) & 0x01)];

			/* the final color is the OR of the two colors (verified) */
			uint8_t final_color = (color1 | color2) & 0x07;

			if (m_flipscreen)
				bitmap.pix(255 - y, 255 - x) = m_palette->pen_color(final_color);
			else
				bitmap.pix(y, x) = m_palette->pen_color(final_color);

			x++;
			data1 >>= 1;
			data2 >>= 1;
		}
	}

	return 0;
}


/*
 *  The Stratovox video connections have been verified from the schematics
 */

uint32_t route16_state::screen_update_jongpute(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint8_t *color_prom1 = &memregion("proms")->base()[0x000];
	uint8_t *color_prom2 = &memregion("proms")->base()[0x100];

	for (offs_t offs = 0; offs < m_videoram1.bytes(); offs++)
	{
		uint8_t y = offs >> 6;
		uint8_t x = offs << 2;

		uint8_t data1 = m_videoram1[offs];
		uint8_t data2 = m_videoram2[offs];

		for (int i = 0; i < 4; i++)
		{
			uint8_t color1 = color_prom1[(m_palette_1 << 2) |
										((data1 >> 3) & 0x02) |
										((data1 >> 0) & 0x01)];

			/* bit 7 of the 2nd color is the OR of the 1st color bits 0 and 1 (verified) */
			uint8_t color2 = color_prom2[(((data1 << 3) & 0x80) | ((data1 << 7) & 0x80)) |
										(m_palette_2 << 2) |
										((data2 >> 3) & 0x02) |
										((data2 >> 0) & 0x01)];

			/* the final color is the OR of the two colors */
			uint8_t final_color = (color1 | color2) & 0x07;

			if (m_flipscreen)
				bitmap.pix(255 - y, 255 - x) = m_palette->pen_color(final_color);
			else
				bitmap.pix(y, x) = m_palette->pen_color(final_color);

			x++;
			data1 >>= 1;
			data2 >>= 1;
		}
	}

	return 0;
}
