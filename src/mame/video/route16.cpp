// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

  route16.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/route16.h"

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

WRITE8_MEMBER(route16_state::out0_w)
{
	m_palette_1 = data & 0x1f;

	machine().bookkeeping().coin_counter_w(0, (data >> 5) & 0x01);
}


WRITE8_MEMBER(route16_state::out1_w)
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

UINT32 route16_state::screen_update_route16(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	offs_t offs;

	UINT8 *color_prom1 = &memregion("proms")->base()[0x000];
	UINT8 *color_prom2 = &memregion("proms")->base()[0x100];

	for (offs = 0; offs < m_videoram1.bytes(); offs++)
	{
		int i;

		UINT8 y = offs >> 6;
		UINT8 x = offs << 2;

		UINT8 data1 = m_videoram1[offs];
		UINT8 data2 = m_videoram2[offs];

		for (i = 0; i < 4; i++)
		{
			UINT8 color1 = color_prom1[((m_palette_1 << 6) & 0x80) |
										(m_palette_1 << 2) |
										((data1 >> 3) & 0x02) |
										((data1 >> 0) & 0x01)];

			/* bit 7 of the 2nd color is the OR of the 1st color bits 0 and 1 - this is a guess */
			UINT8 color2 = color_prom2[((m_palette_2 << 6) & 0x80) | (((color1 << 6) & 0x80) | ((color1 << 7) & 0x80)) |
										(m_palette_2 << 2) |
										((data2 >> 3) & 0x02) |
										((data2 >> 0) & 0x01)];

			/* the final color is the OR of the two colors (verified) */
			UINT8 final_color = (color1 | color2) & 0x07;

			if (m_flipscreen)
				bitmap.pix32(255 - y, 255 - x) = m_palette->pen_color(final_color);
			else
				bitmap.pix32(y, x) = m_palette->pen_color(final_color);

			x = x + 1;
			data1 = data1 >> 1;
			data2 = data2 >> 1;
		}
	}

	return 0;
}


/*
 *  The Stratovox video connections have been verified from the schematics
 */

UINT32 route16_state::screen_update_ttmahjng(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	offs_t offs;

	UINT8 *color_prom1 = &memregion("proms")->base()[0x000];
	UINT8 *color_prom2 = &memregion("proms")->base()[0x100];

	for (offs = 0; offs < m_videoram1.bytes(); offs++)
	{
		int i;

		UINT8 y = offs >> 6;
		UINT8 x = offs << 2;

		UINT8 data1 = m_videoram1[offs];
		UINT8 data2 = m_videoram2[offs];

		for (i = 0; i < 4; i++)
		{
			UINT8 color1 = color_prom1[(m_palette_1 << 2) |
										((data1 >> 3) & 0x02) |
										((data1 >> 0) & 0x01)];

			/* bit 7 of the 2nd color is the OR of the 1st color bits 0 and 1 (verified) */
			UINT8 color2 = color_prom2[(((data1 << 3) & 0x80) | ((data1 << 7) & 0x80)) |
										(m_palette_2 << 2) |
										((data2 >> 3) & 0x02) |
										((data2 >> 0) & 0x01)];

			/* the final color is the OR of the two colors */
			UINT8 final_color = (color1 | color2) & 0x07;

			if (m_flipscreen)
				bitmap.pix32(255 - y, 255 - x) = m_palette->pen_color(final_color);
			else
				bitmap.pix32(y, x) = m_palette->pen_color(final_color);

			x = x + 1;
			data1 = data1 >> 1;
			data2 = data2 >> 1;
		}
	}

	return 0;
}
