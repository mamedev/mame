// license:BSD-3-Clause
// copyright-holders:Dan Boris, Aaron Giles
/***************************************************************************

    Atari Return of the Jedi hardware

    driver by Dan Boris

    Return of the Jedi has a peculiar playfield/motion object
    priority system. That is, there is no priority system ;-)
    The color of the pixel which appears on screen depends on
    all three of the foreground, background and motion objects.
    The 1024 colors palette is appropriately set up by the program
    to "emulate" a priority system, but it can also be used to display
    completely different colors (see the palette test in service mode)

***************************************************************************/

#include "emu.h"
#include "includes/jedi.h"


#define NUM_PENS    (0x1000)



/*************************************
 *
 *  Start
 *
 *************************************/

void jedi_state::video_start()
{
	/* register for saving */
	save_item(NAME(m_vscroll));
	save_item(NAME(m_hscroll));
	save_item(NAME(m_foreground_bank));
	save_item(NAME(m_video_off));
}


WRITE_LINE_MEMBER(jedi_state::foreground_bank_w)
{
	m_foreground_bank = state;
}


WRITE_LINE_MEMBER(jedi_state::video_off_w)
{
	m_video_off = state;
}


/*************************************
 *
 *  Palette RAM
 *
 *************************************
 *
 *  Color RAM format
 *  Color RAM is 1024x12
 *
 *  RAM address: A0..A3 = Playfield color code
 *      A4..A7 = Motion object color code
 *      A8..A9 = Alphanumeric color code
 *
 *  RAM data:
 *      0..2 = Blue
 *      3..5 = Green
 *      6..8 = Blue
 *      9..11 = Intensity
 *
 *  Output resistor values:
 *      bit 0 = 22K
 *      bit 1 = 10K
 *      bit 2 = 4.7K
 *
 *************************************/

void jedi_state::get_pens(pen_t *pens)
{
	offs_t offs;

	for (offs = 0; offs < NUM_PENS; offs++)
	{
		int r, g, b, bits, intensity;

		uint16_t color = m_paletteram[offs] | (m_paletteram[offs | 0x400] << 8);

		intensity = (color >> 9) & 7;
		bits = (color >> 6) & 7;
		r = 5 * bits * intensity;
		bits = (color >> 3) & 7;
		g = 5 * bits * intensity;
		bits = (color >> 0) & 7;
		b = 5 * bits * intensity;

		pens[offs] = rgb_t(r, g, b);
	}
}


void jedi_state::do_pen_lookup(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int y, x;
	pen_t pens[NUM_PENS];

	get_pens(pens);

	for (y = cliprect.top(); y <= cliprect.bottom(); y++)
		for(x = cliprect.left(); x <= cliprect.right(); x++)
			bitmap.pix32(y, x) = pens[bitmap.pix32(y, x)];
}



/*************************************
 *
 *  Scroll offsets
 *
 *************************************/

WRITE8_MEMBER(jedi_state::jedi_vscroll_w)
{
	m_vscroll = data | (offset << 8);
}


WRITE8_MEMBER(jedi_state::jedi_hscroll_w)
{
	m_hscroll = data | (offset << 8);
}



/*************************************
 *
 *  Background/text layer drawing
 *  with smoothing
 *
 *************************************/

void jedi_state::draw_background_and_text(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int y;
	int background_line_buffer[0x200];  /* RAM chip at 2A */

	uint8_t *tx_gfx = memregion("gfx1")->base();
	uint8_t *bg_gfx = memregion("gfx2")->base();
	uint8_t *prom1 = &memregion("proms")->base()[0x0000 | ((*m_smoothing_table & 0x03) << 8)];
	uint8_t *prom2 = &memregion("proms")->base()[0x0800 | ((*m_smoothing_table & 0x03) << 8)];
	int vscroll = m_vscroll;
	int hscroll = m_hscroll;
	int tx_bank = m_foreground_bank;
	uint8_t *tx_ram = m_foregroundram;
	uint8_t *bg_ram = m_backgroundram;

	memset(background_line_buffer, 0, 0x200 * sizeof(int));

	for (y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		int x;
		int bg_last_col = 0;

		for (x = cliprect.left(); x <= cliprect.right(); x += 2)
		{
			int tx_col1, tx_col2, bg_col;
			int bg_tempcol;
			offs_t tx_gfx_offs, bg_gfx_offs;
			int tx_data, bg_data1, bg_data2;

			int sy = y + vscroll;
			int sx = x + hscroll;

			/* determine offsets into video memory */
			offs_t tx_offs = ((y & 0xf8) << 3) | (x >> 3);
			offs_t bg_offs = ((sy & 0x1f0) << 1) | ((sx & 0x1f0) >> 4);

			/* get the character codes */
			int tx_code = (tx_bank << 8) | tx_ram[tx_offs];
			int bg_bank = bg_ram[0x0400 | bg_offs];
			int bg_code = bg_ram[0x0000 | bg_offs] |
							((bg_bank & 0x01) << 8) |
							((bg_bank & 0x08) << 6) |
							((bg_bank & 0x02) << 9);

			/* background flip X */
			if (bg_bank & 0x04)
				sx = sx ^ 0x0f;

			/* calculate the address of the gfx data */
			tx_gfx_offs = (tx_code << 4) | ((y & 0x07) << 1) | ((( x & 0x04) >> 2));
			bg_gfx_offs = (bg_code << 4) | (sy & 0x0e)       | (((sx & 0x08) >> 3));

			/* get the gfx data */
			tx_data  = tx_gfx[         tx_gfx_offs];
			bg_data1 = bg_gfx[0x0000 | bg_gfx_offs];
			bg_data2 = bg_gfx[0x8000 | bg_gfx_offs];

			/* the text layer pixel determines pen address bits A8 and A9 */
			if (x & 0x02)
			{
				tx_col1 = ((tx_data  & 0x0c) << 6);
				tx_col2 = ((tx_data  & 0x03) << 8);
			}
			else
			{
				tx_col1 = ((tx_data  & 0xc0) << 2);
				tx_col2 = ((tx_data  & 0x30) << 4);
			}

			/* the background pixel determines pen address bits A0-A3 */
			switch (sx & 0x06)
			{
			case 0x00: bg_col = ((bg_data1 & 0x80) >> 4) | ((bg_data1 & 0x08) >> 1) | ((bg_data2 & 0x80) >> 6) | ((bg_data2 & 0x08) >> 3); break;
			case 0x02: bg_col = ((bg_data1 & 0x40) >> 3) | ((bg_data1 & 0x04) >> 0) | ((bg_data2 & 0x40) >> 5) | ((bg_data2 & 0x04) >> 2); break;
			case 0x04: bg_col = ((bg_data1 & 0x20) >> 2) | ((bg_data1 & 0x02) << 1) | ((bg_data2 & 0x20) >> 4) | ((bg_data2 & 0x02) >> 1); break;
			default:   bg_col = ((bg_data1 & 0x10) >> 1) | ((bg_data1 & 0x01) << 2) | ((bg_data2 & 0x10) >> 3) | ((bg_data2 & 0x01) >> 0); break;
			}

			/* the first pixel is smoothed via a lookup using the current and last pixel value -
			   the next pixel just uses the current value directly. After we done with a pixel
			   save it for later in the line buffer RAM */
			bg_tempcol = prom1[(bg_last_col << 4) | bg_col];
			bitmap.pix32(y, x + 0) = tx_col1 | prom2[(background_line_buffer[x + 0] << 4) | bg_tempcol];
			bitmap.pix32(y, x + 1) = tx_col2 | prom2[(background_line_buffer[x + 1] << 4) | bg_col];
			background_line_buffer[x + 0] = bg_tempcol;
			background_line_buffer[x + 1] = bg_col;

			bg_last_col = bg_col;
		}
	}
}



/*************************************
 *
 *  Sprite drawing
 *
 *************************************/

void jedi_state::draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	offs_t offs;
	uint8_t *spriteram = m_spriteram;
	uint8_t *gfx3 = memregion("gfx3")->base();

	for (offs = 0x00; offs < 0x30; offs++)
	{
		int sy;
		int y_size;
		uint8_t *gfx;

		/* coordinates adjustments made to match screenshot */
		uint8_t y = 240 - spriteram[offs + 0x80] + 1;
		int flip_x = spriteram[offs + 0x40] & 0x10;
		int flip_y = spriteram[offs + 0x40] & 0x20;
		int tall = spriteram[offs + 0x40] & 0x08;

		/* shuffle the bank bits in */
		uint16_t code = spriteram[offs] |
						((spriteram[offs + 0x40] & 0x04) << 8) |
						((spriteram[offs + 0x40] & 0x40) << 3) |
						((spriteram[offs + 0x40] & 0x02) << 7);

		/* adjust for double-height */
		if (tall)
		{
			code &= ~1;
			y_size = 0x20;
			y = y - 0x10;
		}
		else
			y_size = 0x10;

		gfx = &gfx3[code << 5];

		if (flip_y)
			y = y + y_size - 1;

		for (sy = 0; sy < y_size; sy++)
		{
			int i;
			uint16_t x = spriteram[offs + 0x100] + ((spriteram[offs + 0x40] & 0x01) << 8) - 2;

			if ((y < cliprect.top()) || (y > cliprect.bottom()))
				continue;

			if (flip_x)
				x = x + 7;

			for (i = 0; i < 2; i++)
			{
				int sx;
				uint8_t data1 = *(0x00000 + gfx);
				uint8_t data2 = *(0x10000 + gfx);

				for (sx = 0; sx < 4; sx++)
				{
					/* the sprite pixel determines pen address bits A4-A7 */
					uint32_t col = ((data1 & 0x80) >> 0) | ((data1 & 0x08) << 3) | ((data2 & 0x80) >> 2) | ((data2 & 0x08) << 1);

					x = x & 0x1ff;

					if (col)
						bitmap.pix32(y, x) = (bitmap.pix32(y, x) & 0x30f) | col;

					/* next pixel */
					if (flip_x)
						x = x - 1;
					else
						x = x + 1;

					data1 = data1 << 1;
					data2 = data2 << 1;
				}

				gfx = gfx + 1;
			}

			if (flip_y)
				y = y - 1;
			else
				y = y + 1;
		}
	}
}



/*************************************
 *
 *  Core video refresh
 *
 *************************************/

uint32_t jedi_state::screen_update_jedi(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	/* if no video, clear it all to black */
	if (m_video_off)
		bitmap.fill(rgb_t::black(), cliprect);
	else
	{
		/* draw the background/text layers, followed by the sprites
		   - it needs to be done in this order*/
		draw_background_and_text(bitmap, cliprect);
		draw_sprites(bitmap, cliprect);
		do_pen_lookup(bitmap, cliprect);
	}

	return 0;
}



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void jedi_state::jedi_video(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(64*8, 262); /* verify vert size */
	m_screen->set_visarea(0*8, 37*8-1, 0*8, 30*8-1);
	m_screen->set_screen_update(FUNC(jedi_state::screen_update_jedi));
}
