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
#include "jedi.h"

#include <algorithm>


/*************************************
 *
 *  Start
 *
 *************************************/

void jedi_state::video_start()
{
#if DEBUG_GFXDECODE
	/* the sprite pixel determines pen address bits A4-A7 */
	gfx_element *gx0 = m_gfxdecode->gfx(2);

	// allocate memory for the assembled data
	m_gfxdata = std::make_unique<u8[]>(gx0->elements() * gx0->width() * gx0->height());

	// loop over elements
	u8 *dest = m_gfxdata.get();
	for (int c = 0; c < gx0->elements(); c++)
	{
		const u8 *c0base = gx0->get_data(c);

		// loop over height
		for (int y = 0; y < gx0->height(); y++)
		{
			const u8 *c0 = c0base;

			for (int x = 0; x < gx0->width(); x++)
			{
				const u8 pix = (*c0++ & 0xf);
				*dest++ = pix << 4;
			}
			c0base += gx0->rowbytes();
		}
	}

	gx0->set_raw_layout(m_gfxdata.get(), gx0->width(), gx0->height(), gx0->elements(), 8 * gx0->width(), 8 * gx0->width() * gx0->height());
	gx0->set_granularity(1);
#endif

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

rgb_t jedi_state::jedi_IRGB_3333(u32 raw)
{
	const u8 intensity = (raw >> 9) & 7;
	u8 bits = (raw >> 6) & 7;
	const u8 r = 5 * bits * intensity;
	bits = (raw >> 3) & 7;
	const u8 g = 5 * bits * intensity;
	bits = (raw >> 0) & 7;
	const u8 b = 5 * bits * intensity;

	return rgb_t(r, g, b);
}

void jedi_state::do_pen_lookup(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
		for(int x = cliprect.left(); x <= cliprect.right(); x++)
			bitmap.pix(y, x) = m_palette->pen(bitmap.pix(y, x));
}


/*************************************
 *
 *  Scroll offsets
 *
 *************************************/

void jedi_state::vscroll_w(offs_t offset, u8 data)
{
	m_vscroll = data | (offset << 8);
}


void jedi_state::hscroll_w(offs_t offset, u8 data)
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
	u8 background_line_buffer[0x200];  /* RAM chip at 2A */

	const u8 *prom1 = &m_proms[0x0000 | ((*m_smoothing_table & 0x03) << 8)];
	const u8 *prom2 = &m_proms[0x0800 | ((*m_smoothing_table & 0x03) << 8)];

	std::fill(std::begin(background_line_buffer), std::end(background_line_buffer), 0);

	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		u8 bg_last_col = 0;

		for (int x = cliprect.left(); x <= cliprect.right(); x += 2)
		{
			u16 tx_col1, tx_col2, bg_col;

			const int sy = y + m_vscroll;
			int sx = x + m_hscroll;

			/* determine offsets into video memory */
			const offs_t tx_offs = ((y & 0xf8) << 3) | (x >> 3);
			const offs_t bg_offs = ((sy & 0x1f0) << 1) | ((sx & 0x1f0) >> 4);

			/* get the character codes */
			const int tx_code = (m_foreground_bank << 8) | m_foregroundram[tx_offs];
			const int bg_bank = m_backgroundram[0x0400 | bg_offs];
			const int bg_code = m_backgroundram[0x0000 | bg_offs] |
							((bg_bank & 0x01) << 8) |
							((bg_bank & 0x08) << 6) |
							((bg_bank & 0x02) << 9);

			/* background flip X */
			if (bg_bank & 0x04)
				sx = sx ^ 0x0f;

			/* calculate the address of the gfx data */
			const offs_t tx_gfx_offs = (tx_code << 4) | ((y & 0x07) << 1) | ((( x & 0x04) >> 2));
			const offs_t bg_gfx_offs = (bg_code << 4) | (sy & 0x0e)       | (((sx & 0x08) >> 3));

			/* get the gfx data */
			const u8 tx_data  = m_tx_gfx[         tx_gfx_offs];
			const u8 bg_data1 = m_bg_gfx[0x0000 | bg_gfx_offs];
			const u8 bg_data2 = m_bg_gfx[0x8000 | bg_gfx_offs];

			/* the text layer pixel determines pen address bits A8 and A9 */
			if (x & 0x02)
			{
				tx_col1 = ((tx_data & 0x0c) << 6);
				tx_col2 = ((tx_data & 0x03) << 8);
			}
			else
			{
				tx_col1 = ((tx_data & 0xc0) << 2);
				tx_col2 = ((tx_data & 0x30) << 4);
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
			const u8 bg_tempcol = prom1[(bg_last_col << 4) | bg_col];
			bitmap.pix(y, x + 0) = tx_col1 | prom2[(background_line_buffer[x + 0] << 4) | bg_tempcol];
			bitmap.pix(y, x + 1) = tx_col2 | prom2[(background_line_buffer[x + 1] << 4) | bg_col];
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
	for (offs_t offs = 0x00; offs < 0x30; offs++)
	{
		int y_size;

		/* coordinates adjustments made to match screenshot */
		u8 y = 240 - m_spriteram[offs + 0x80] + 1;
		const bool flip_x = m_spriteram[offs + 0x40] & 0x10;
		const bool flip_y = m_spriteram[offs + 0x40] & 0x20;
		const bool tall = m_spriteram[offs + 0x40] & 0x08;

		/* shuffle the bank bits in */
		u16 code = m_spriteram[offs] |
						((m_spriteram[offs + 0x40] & 0x04) << 8) |
						((m_spriteram[offs + 0x40] & 0x40) << 3) |
						((m_spriteram[offs + 0x40] & 0x02) << 7);

		/* adjust for double-height */
		if (tall)
		{
			code &= ~1;
			y_size = 0x20;
			y = y - 0x10;
		}
		else
			y_size = 0x10;

		const u8 *gfx = &m_spr_gfx[code << 5];

		if (flip_y)
			y = y + y_size - 1;

		for (int sy = 0; sy < y_size; sy++)
		{
			u16 x = m_spriteram[offs + 0x100] + ((m_spriteram[offs + 0x40] & 0x01) << 8) - 2;

			if ((y < cliprect.top()) || (y > cliprect.bottom()))
				continue;

			if (flip_x)
				x = x + 7;

			for (int i = 0; i < 2; i++)
			{
				u8 data1 = *(0x00000 + gfx);
				u8 data2 = *(0x10000 + gfx);

				for (int sx = 0; sx < 4; sx++)
				{
					/* the sprite pixel determines pen address bits A4-A7 */
					const u32 col = ((data1 & 0x80) >> 0) | ((data1 & 0x08) << 3) | ((data2 & 0x80) >> 2) | ((data2 & 0x08) << 1);

					x = x & 0x1ff;

					if (col)
						bitmap.pix(y, x) = (bitmap.pix(y, x) & 0x30f) | col;

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

u32 jedi_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
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

#if DEBUG_GFXDECODE
static const gfx_layout text_layout =
{
	8, 8,
	RGN_FRAC(1,1),
	2,
	{ 0, 1 },
	{ STEP8(0, 2) },
	{ STEP8(0, 2*8) },
	8*8*2
};

static const gfx_layout bg_layout =
{
	8, 8,
	RGN_FRAC(1,2),
	4,
	{ 0, 4, RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4 },
	{ STEP4(0, 1), STEP4(4*2, 1) },
	{ STEP8(0, 4*2*2) },
	8*8*2
};

static const gfx_layout sprite_layout =
{
	8, 16,
	RGN_FRAC(1,2),
	4,
	{ 0, 4, RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4 },
	{ STEP4(0, 1), STEP4(4*2, 1) },
	{ STEP16(0, 4*2*2) },
	8*16*2
};

static GFXDECODE_START( gfx_jedi )
	GFXDECODE_ENTRY( "tx_gfx",  0, text_layout,   0, 0x400/0x04 )
	GFXDECODE_SCALE( "bg_gfx",  0, bg_layout,     0, 0x400/0x10, 2, 2 ) // 8x8 but internally expanded related with smoothing
	GFXDECODE_ENTRY( "spr_gfx", 0, sprite_layout, 0, 0x310 )
GFXDECODE_END
#endif

void jedi_state::jedi_video(machine_config &config)
{
#if DEBUG_GFXDECODE
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_jedi);
#endif
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(64*8, 262); /* verify vert size */
	m_screen->set_visarea(0*8, 37*8-1, 0*8, 30*8-1);
	m_screen->set_screen_update(FUNC(jedi_state::screen_update));

	PALETTE(config, m_palette).set_format(2, &jedi_state::jedi_IRGB_3333, 0x400);
}
