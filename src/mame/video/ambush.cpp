// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/ambush.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

  I'm not sure about the resistor value, I'm using the Galaxian ones.

***************************************************************************/

PALETTE_INIT_MEMBER(ambush_state, ambush)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	for (i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r,g,b));
	}
}


void ambush_state::draw_chars( bitmap_ind16 &bitmap, const rectangle &cliprect, int priority )
{
	int offs, transpen;

	transpen = (priority == 0) ? -1 : 0;

	for (offs = 0; offs < m_videoram.bytes(); offs++)
	{
		int code, sx, sy, col;
		UINT8 scroll;

		sy = (offs / 32);
		sx = (offs % 32);

		col = m_colorram[((sy & 0x1c) << 3) + sx];

		if (priority & ~col)
			continue;

		scroll = ~m_scrollram[sx];

		code = m_videoram[offs] | ((col & 0x60) << 3);

		if (flip_screen())
		{
			sx = 31 - sx;
			sy = 31 - sy;
			scroll = ~scroll - 1;
		}

		m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
			code,
			(col & 0x0f) | ((*m_colorbank & 0x03) << 4),
			flip_screen(), flip_screen(),
			8 * sx, (8 * sy + scroll) & 0xff, transpen);
	}
}


UINT32 ambush_state::screen_update_ambush(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs;

	bitmap.fill(0, cliprect);

	/* Draw the characters */
	draw_chars(bitmap, cliprect, 0x00);

	/* Draw the sprites. */
	for (offs = m_spriteram.bytes() - 4; offs >= 0; offs -= 4)
	{
		int code, col, sx, sy, flipx, flipy, gfx;

		sy = m_spriteram[offs + 0];
		sx = m_spriteram[offs + 3];

		if ( (sy == 0) ||
				(sy == 0xff) ||
			((sx <  0x40) && (  m_spriteram[offs + 2] & 0x10)) ||
			((sx >= 0xc0) && (!(m_spriteram[offs + 2] & 0x10))))
			continue;  /* prevent wraparound */


		code = (m_spriteram[offs + 1] & 0x3f) | ((m_spriteram[offs + 2] & 0x60) << 1);

		if (m_spriteram[offs + 2] & 0x80)
		{
			/* 16x16 sprites */
			gfx = 1;

			if (!flip_screen())
				sy = 240 - sy;
			else
				sx = 240 - sx;
		}
		else
		{
			/* 8x8 sprites */
			gfx = 0;
			code <<= 2;

			if (!flip_screen())
				sy = 248 - sy;
			else
				sx = 248 - sx;
		}

		col   = m_spriteram[offs + 2] & 0x0f;
		flipx = m_spriteram[offs + 1] & 0x40;
		flipy = m_spriteram[offs + 1] & 0x80;

		if (flip_screen())
		{
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(gfx)->transpen(bitmap,cliprect,
				code, col | ((*m_colorbank & 0x03) << 4),
				flipx, flipy,
				sx,sy,0);
	}

	/* Draw the foreground priority characters over the sprites */
	draw_chars(bitmap, cliprect, 0x10);
	return 0;
}
