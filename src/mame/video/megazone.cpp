// license:BSD-3-Clause
// copyright-holders:Chris Hardy
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/megazone.h"

/***************************************************************************
Based on driver from MAME 0.55
Changes by Martin M. (pfloyd@gmx.net) 14.10.2001:

 - Added support for screen flip in cocktail mode (tricky!) */


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Megazone has one 32x8 palette PROM and two 256x4 lookup table PROMs
  (one for characters, one for sprites).
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

PALETTE_INIT_MEMBER(megazone_state, megazone)
{
	const UINT8 *color_prom = memregion("proms")->base();
	static const int resistances_rg[3] = { 1000, 470, 220 };
	static const int resistances_b [2] = { 470, 220 };
	double rweights[3], gweights[3], bweights[2];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0, 255, -1.0,
			3, &resistances_rg[0], rweights, 1000, 0,
			3, &resistances_rg[0], gweights, 1000, 0,
			2, &resistances_b[0],  bweights, 1000, 0);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		r = combine_3_weights(rweights, bit0, bit1, bit2);

		/* green component */
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		g = combine_3_weights(gweights, bit0, bit1, bit2);

		/* blue component */
		bit0 = BIT(color_prom[i], 6);
		bit1 = BIT(color_prom[i], 7);
		b = combine_2_weights(bweights, bit0, bit1);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x20;

	/* sprites */
	for (i = 0; i < 0x100; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		palette.set_pen_indirect(i, ctabentry);
	}

	/* characters */
	for (i = 0x100; i < 0x200; i++)
	{
		UINT8 ctabentry = (color_prom[i] & 0x0f) | 0x10;
		palette.set_pen_indirect(i, ctabentry);
	}
}

WRITE8_MEMBER(megazone_state::megazone_flipscreen_w)
{
	m_flipscreen = data & 1;
}

void megazone_state::video_start()
{
	m_tmpbitmap = std::make_unique<bitmap_ind16>(256, 256);

	save_item(NAME(*m_tmpbitmap));
}


UINT32 megazone_state::screen_update_megazone(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs;
	int x, y;

	/* for every character in the Video RAM */
	for (offs = m_videoram.bytes() - 1; offs >= 0; offs--)
	{
		int sx, sy, flipx, flipy;

		sx = offs % 32;
		sy = offs / 32;
		flipx = m_colorram[offs] & (1 << 6);
		flipy = m_colorram[offs] & (1 << 5);

		if (m_flipscreen)
		{
			sx = 31 - sx;
			sy = 31 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(1)->opaque(*m_tmpbitmap,m_tmpbitmap->cliprect(),
				((int)m_videoram[offs]) + ((m_colorram[offs] & (1 << 7) ? 256 : 0) ),
				(m_colorram[offs] & 0x0f) + 0x10,
				flipx,flipy,
				8*sx,8*sy);
	}

	/* copy the temporary bitmap to the screen */
	{
		int scrollx;
		int scrolly;

		if (m_flipscreen)
		{
			scrollx = *m_scrolly;
			scrolly = *m_scrollx;
		}
		else
		{
			scrollx = - *m_scrolly + 4 * 8; // leave space for credit&score overlay
			scrolly = - *m_scrollx;
		}


		copyscrollbitmap(bitmap, *m_tmpbitmap, 1, &scrollx, 1, &scrolly, cliprect);
	}


	/* Draw the sprites. */
	{
		UINT8 *spriteram = m_spriteram;
		for (offs = m_spriteram.bytes() - 4; offs >= 0; offs -= 4)
		{
			int sx = spriteram[offs + 3];
			int sy = 255 - ((spriteram[offs + 1] + 16) & 0xff);
			int color =  spriteram[offs + 0] & 0x0f;
			int flipx = ~spriteram[offs + 0] & 0x40;
			int flipy =  spriteram[offs + 0] & 0x80;

			if (m_flipscreen)
			{
				sx = sx - 11;
				sy = sy + 2;
			}
			else
				sx = sx + 32;

			m_gfxdecode->gfx(0)->transmask(bitmap,cliprect,
					spriteram[offs + 2],
					color,
					flipx,flipy,
					sx,sy,
					m_palette->transpen_mask(*m_gfxdecode->gfx(0), color, 0));
		}
	}

	for (y = 0; y < 32;y++)
	{
		offs = y * 32;
		for (x = 0; x < 6; x++)
		{
			int sx, sy, flipx, flipy;

			sx = x;
			sy = y;

			flipx = m_colorram2[offs] & (1 << 6);
			flipy = m_colorram2[offs] & (1 << 5);

			if (m_flipscreen)
			{
				sx = 35 - sx;
				sy = 31 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}




			m_gfxdecode->gfx(1)->opaque(bitmap,cliprect,
					((int)m_videoram2[offs]) + ((m_colorram2[offs] & (1 << 7) ? 256 : 0) ),
					(m_colorram2[offs] & 0x0f) + 0x10,
					flipx,flipy,
					8*sx,8*sy);
			offs++;
		}
	}
	return 0;
}
