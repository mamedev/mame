// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Jaleco Exerion

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/exerion.h"


#define BACKGROUND_X_START      32

#define VISIBLE_X_MIN           (12*8)
#define VISIBLE_X_MAX           (52*8)
#define VISIBLE_Y_MIN           (2*8)
#define VISIBLE_Y_MAX           (30*8)


/***************************************************************************

  Convert the color PROMs into a more useable format.

  The palette PROM is connected to the RGB output this way:

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

***************************************************************************/

PALETTE_INIT_MEMBER(exerion_state, exerion)
{
	const UINT8 *color_prom = memregion("proms")->base();
	static const int resistances_rg[3] = { 1000, 470, 220 };
	static const int resistances_b [2] = { 470, 220 };
	double rweights[3], gweights[3], bweights[2];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0, 255, -1.0,
			3, &resistances_rg[0], rweights, 0, 0,
			3, &resistances_rg[0], gweights, 0, 0,
			2, &resistances_b[0],  bweights, 0, 0);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = combine_3_weights(rweights, bit0, bit1, bit2);

		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = combine_3_weights(gweights, bit0, bit1, bit2);

		/* blue component */
		bit0 = (color_prom[i] >> 6) & 0x01;
		bit1 = (color_prom[i] >> 7) & 0x01;
		b = combine_2_weights(bweights, bit0, bit1);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x20;

	/* fg chars and sprites */
	for (i = 0; i < 0x200; i++)
	{
		UINT8 ctabentry = 0x10 | (color_prom[(i & 0x1c0) | ((i & 3) << 4) | ((i >> 2) & 0x0f)] & 0x0f);
		palette.set_pen_indirect(i, ctabentry);
	}

	/* bg chars (this is not the full story... there are four layers mixed */
	/* using another PROM */
	for (i = 0x200; i < 0x300; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		palette.set_pen_indirect(i, ctabentry);
	}
}



/*************************************
 *
 *  Video system startup
 *
 *************************************/

void exerion_state::video_start()
{
	int i;
	UINT8 *gfx;

	/* get pointers to the mixing and lookup PROMs */
	m_background_mixer = memregion("proms")->base() + 0x320;

	/* allocate memory for the decoded background graphics */
	m_background_gfx[0] = auto_alloc_array(machine(), UINT16, 256 * 256 * 4);
	m_background_gfx[1] = m_background_gfx[0] + 256 * 256;
	m_background_gfx[2] = m_background_gfx[1] + 256 * 256;
	m_background_gfx[3] = m_background_gfx[2] + 256 * 256;

	save_pointer(NAME(m_background_gfx[0]), 256 * 256 * 4);

	/*---------------------------------
	 * Decode the background graphics
	 *
	 * We decode the 4 background layers separately, but shuffle the bits so that
	 * we can OR all four layers together. Each layer has 2 bits per pixel. Each
	 * layer is decoded into the following bit patterns:
	 *
	 *  000a 0000 00AA
	 *  00b0 0000 BB00
	 *  0c00 00CC 0000
	 *  d000 DD00 0000
	 *
	 * Where AA,BB,CC,DD are the 2bpp data for the pixel,and a,b,c,d are the OR
	 * of these two bits together.
	 */
	gfx = memregion("gfx3")->base();
	for (i = 0; i < 4; i++)
	{
		int y;

		UINT8 *src = gfx + i * 0x2000;
		UINT16 *dst = m_background_gfx[i];

		for (y = 0; y < 0x100; y++)
		{
			int x;

			for (x = 0; x < 0x80; x += 4)
			{
				UINT8 data = *src++;
				UINT16 val;

				val = ((data >> 3) & 2) | ((data >> 0) & 1);
				if (val) val |= 0x100 >> i;
				*dst++ = val << (2 * i);

				val = ((data >> 4) & 2) | ((data >> 1) & 1);
				if (val) val |= 0x100 >> i;
				*dst++ = val << (2 * i);

				val = ((data >> 5) & 2) | ((data >> 2) & 1);
				if (val) val |= 0x100 >> i;
				*dst++ = val << (2 * i);

				val = ((data >> 6) & 2) | ((data >> 3) & 1);
				if (val) val |= 0x100 >> i;
				*dst++ = val << (2 * i);
			}

			for (; x < 0x100; x++)
				*dst++ = 0;
		}
	}
}



/*************************************
 *
 *  Video register I/O
 *
 *************************************/

WRITE8_MEMBER(exerion_state::exerion_videoreg_w)
{
	/* bit 0 = flip screen and joystick input multiplexer */
	m_cocktail_flip = data & 1;

	/* bits 1-2 char lookup table bank */
	m_char_palette = (data & 0x06) >> 1;

	/* bits 3 char bank */
	m_char_bank = (data & 0x08) >> 3;

	/* bits 4-5 unused */

	/* bits 6-7 sprite lookup table bank */
	m_sprite_palette = (data & 0xc0) >> 6;
}


WRITE8_MEMBER(exerion_state::exerion_video_latch_w)
{
	int scanline = m_screen->vpos();
	if (scanline > 0)
		m_screen->update_partial(scanline - 1);
	m_background_latches[offset] = data;
}


READ8_MEMBER(exerion_state::exerion_video_timing_r)
{
	/* bit 0 is the SNMI signal, which is the negated value of H6, if H7=1 & H8=1 & VBLANK=0, otherwise 1 */
	/* bit 1 is VBLANK */

	UINT16 hcounter = m_screen->hpos() + EXERION_HCOUNT_START;
	UINT8 snmi = 1;

	if (((hcounter & 0x180) == 0x180) && !m_screen->vblank())
		snmi = !((hcounter >> 6) & 0x01);

	return (m_screen->vblank() << 1) | snmi;
}


/*************************************
 *
 *  Background rendering
 *
 *************************************/

void exerion_state::draw_background( bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x, y;

	/* loop over all visible scanlines */
	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		UINT16 *src0 = &m_background_gfx[0][m_background_latches[1] * 256];
		UINT16 *src1 = &m_background_gfx[1][m_background_latches[3] * 256];
		UINT16 *src2 = &m_background_gfx[2][m_background_latches[5] * 256];
		UINT16 *src3 = &m_background_gfx[3][m_background_latches[7] * 256];
		int xoffs0 = m_background_latches[0];
		int xoffs1 = m_background_latches[2];
		int xoffs2 = m_background_latches[4];
		int xoffs3 = m_background_latches[6];
		int start0 = m_background_latches[8] & 0x0f;
		int start1 = m_background_latches[9] & 0x0f;
		int start2 = m_background_latches[10] & 0x0f;
		int start3 = m_background_latches[11] & 0x0f;
		int stop0 = m_background_latches[8] >> 4;
		int stop1 = m_background_latches[9] >> 4;
		int stop2 = m_background_latches[10] >> 4;
		int stop3 = m_background_latches[11] >> 4;
		UINT8 *mixer = &m_background_mixer[(m_background_latches[12] << 4) & 0xf0];
		UINT16 scanline[VISIBLE_X_MAX];
		pen_t pen_base = 0x200 + ((m_background_latches[12] >> 4) << 4);

		/* the cocktail flip flag controls whether we count up or down in X */
		if (!m_cocktail_flip)
		{
			/* skip processing anything that's not visible */
			for (x = BACKGROUND_X_START; x < cliprect.min_x; x++)
			{
				if (!(++xoffs0 & 0x1f)) start0++, stop0++;
				if (!(++xoffs1 & 0x1f)) start1++, stop1++;
				if (!(++xoffs2 & 0x1f)) start2++, stop2++;
				if (!(++xoffs3 & 0x1f)) start3++, stop3++;
			}

			/* draw the rest of the scanline fully */
			for (x = cliprect.min_x; x <= cliprect.max_x; x++)
			{
				UINT16 combined = 0;
				UINT8 lookupval;

				/* the output enable is controlled by the carries on the start/stop counters */
				/* they are only active when the start has carried but the stop hasn't */
				if ((start0 ^ stop0) & 0x10) combined |= src0[xoffs0 & 0xff];
				if ((start1 ^ stop1) & 0x10) combined |= src1[xoffs1 & 0xff];
				if ((start2 ^ stop2) & 0x10) combined |= src2[xoffs2 & 0xff];
				if ((start3 ^ stop3) & 0x10) combined |= src3[xoffs3 & 0xff];

				/* bits 8-11 of the combined value contains the lookup for the mixer PROM */
				lookupval = mixer[combined >> 8] & 3;

				/* the color index comes from the looked up value combined with the pixel data */
				scanline[x] = pen_base | (lookupval << 2) | ((combined >> (2 * lookupval)) & 3);

				/* the start/stop counters are clocked when the low 5 bits of the X counter overflow */
				if (!(++xoffs0 & 0x1f)) start0++, stop0++;
				if (!(++xoffs1 & 0x1f)) start1++, stop1++;
				if (!(++xoffs2 & 0x1f)) start2++, stop2++;
				if (!(++xoffs3 & 0x1f)) start3++, stop3++;
			}
		}
		else
		{
			/* skip processing anything that's not visible */
			for (x = BACKGROUND_X_START; x < cliprect.min_x; x++)
			{
				if (!(xoffs0-- & 0x1f)) start0++, stop0++;
				if (!(xoffs1-- & 0x1f)) start1++, stop1++;
				if (!(xoffs2-- & 0x1f)) start2++, stop2++;
				if (!(xoffs3-- & 0x1f)) start3++, stop3++;
			}

			/* draw the rest of the scanline fully */
			for (x = cliprect.min_x; x <= cliprect.max_x; x++)
			{
				UINT16 combined = 0;
				UINT8 lookupval;

				/* the output enable is controlled by the carries on the start/stop counters */
				/* they are only active when the start has carried but the stop hasn't */
				if ((start0 ^ stop0) & 0x10) combined |= src0[xoffs0 & 0xff];
				if ((start1 ^ stop1) & 0x10) combined |= src1[xoffs1 & 0xff];
				if ((start2 ^ stop2) & 0x10) combined |= src2[xoffs2 & 0xff];
				if ((start3 ^ stop3) & 0x10) combined |= src3[xoffs3 & 0xff];

				/* bits 8-11 of the combined value contains the lookup for the mixer PROM */
				lookupval = mixer[combined >> 8] & 3;

				/* the color index comes from the looked up value combined with the pixel data */
				scanline[x] = pen_base | (lookupval << 2) | ((combined >> (2 * lookupval)) & 3);

				/* the start/stop counters are clocked when the low 5 bits of the X counter overflow */
				if (!(xoffs0-- & 0x1f)) start0++, stop0++;
				if (!(xoffs1-- & 0x1f)) start1++, stop1++;
				if (!(xoffs2-- & 0x1f)) start2++, stop2++;
				if (!(xoffs3-- & 0x1f)) start3++, stop3++;
			}
		}

		/* draw the scanline */
		draw_scanline16(bitmap, cliprect.min_x, y, cliprect.width(), &scanline[cliprect.min_x], NULL);
	}
}


/*************************************
 *
 *  Core refresh routine
 *
 *************************************/

UINT32 exerion_state::screen_update_exerion(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int sx, sy, offs, i;

	/* draw background */
	draw_background(bitmap, cliprect);

	/* draw sprites */
	for (i = 0; i < m_spriteram.bytes(); i += 4)
	{
		int flags = m_spriteram[i + 0];
		int y = m_spriteram[i + 1] ^ 255;
		int code = m_spriteram[i + 2];
		int x = m_spriteram[i + 3] * 2 + 72;

		int xflip = flags & 0x80;
		int yflip = flags & 0x40;
		int doubled = flags & 0x10;
		int wide = flags & 0x08;
		int code2 = code;

		int color = ((flags >> 1) & 0x03) | ((code >> 5) & 0x04) | (code & 0x08) | (m_sprite_palette * 16);
		gfx_element *gfx = doubled ? m_gfxdecode->gfx(2) : m_gfxdecode->gfx(1);

		if (m_cocktail_flip)
		{
			x = 64*8 - gfx->width() - x;
			y = 32*8 - gfx->height() - y;
			if (wide) y -= gfx->height();
			xflip = !xflip;
			yflip = !yflip;
		}

		if (wide)
		{
			if (yflip)
				code |= 0x10, code2 &= ~0x10;
			else
				code &= ~0x10, code2 |= 0x10;

				gfx->transmask(bitmap,cliprect, code2, color, xflip, yflip, x, y + gfx->height(),
					m_palette->transpen_mask(*gfx, color, 0x10));
		}

			gfx->transmask(bitmap,cliprect, code, color, xflip, yflip, x, y,
				m_palette->transpen_mask(*gfx, color, 0x10));

		if (doubled) i += 4;
	}

	/* draw the visible text layer */
	for (sy = cliprect.min_y/8; sy <= cliprect.max_y/8; sy++)
		for (sx = VISIBLE_X_MIN/8; sx < VISIBLE_X_MAX/8; sx++)
		{
			int x = m_cocktail_flip ? (63*8 - 8*sx) : 8*sx;
			int y = m_cocktail_flip ? (31*8 - 8*sy) : 8*sy;

			offs = sx + sy * 64;
			m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
				m_videoram[offs] + 256 * m_char_bank,
				((m_videoram[offs] & 0xf0) >> 4) + m_char_palette * 16,
				m_cocktail_flip, m_cocktail_flip, x, y, 0);
		}

	return 0;
}
