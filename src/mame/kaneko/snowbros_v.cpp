// license:BSD-3-Clause
// copyright-holders:David Haywood, Luca Elia
/* bootlegs of Kaneko Pandora chip, with modifications */

#include "emu.h"
#include "snowbros.h"

u32 snowbros_state::screen_update_honeydol(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* sprites clip on left / right edges when scrolling, but it seems correct,
	   no extra sprite attribute bits are set during this time, the sprite co-ordinates
	   are simply set to 0 */

	/* not standard snowbros video */

	bitmap.fill(0xf0, cliprect);

	for (int offs = 0x0000 / 2; offs < 0x2000 / 2; offs += 8)
	{
		int dx, dy, tilecolour, attr, flipx, flipy, tile;

		/* 8bpp gfx */
		dx         = (m_bootleg_spriteram[offs + 4] >> 8) & 0xff;
		dy         = (m_bootleg_spriteram[offs + 5] >> 8) & 0xff;
		tilecolour = (m_bootleg_spriteram[offs + 3] >> 8) & 3;
		attr       = m_bootleg_spriteram[offs + 7] >> 8;
		flipx      = BIT(attr, 7);
		flipy      = BIT(attr, 6);
		tile       = ((attr & 0x3f) << 8) + ((m_bootleg_spriteram[offs + 6] >> 8) & 0xff);

		if (flip_screen())
		{
			dx = 240 - dx;
			dy = 240 - dy;
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
				tile,
				tilecolour,
				flipx, flipy,
				dx, dy, 0);

		/* second list interleaved with first (4bpp) ??? */
		dx         = m_bootleg_spriteram[offs + 4] & 0xff;
		dy         = m_bootleg_spriteram[offs + 5] & 0xff;
		tilecolour = m_bootleg_spriteram[offs + 3];
		attr       = m_bootleg_spriteram[offs + 7];
		flipx      = BIT(attr, 7);
		flipy      = BIT(attr, 6);
		tile       = ((attr & 0x3f) << 8) + (m_bootleg_spriteram[offs + 6] & 0xff);

		if (flip_screen())
		{
			dx = 240 - dx;
			dy = 240 - dy;
			flipx = !flipx;
			flipy = !flipy;
		}

		tilecolour = (tilecolour & 0x03f0) >> 4;
		tilecolour ^= 0x3f; // unusual, but correct..

		m_gfxdecode->gfx(0)->transpen(bitmap, cliprect,
				tile,
				tilecolour,
				flipx, flipy,
				dx, dy, 0);
	}
	return 0;
}


u32 snowbros_state::screen_update_twinadv(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* sprites clip on left / right edges when scrolling, but it seems correct,
	   no extra sprite attribute bits are set during this time, the sprite co-ordinates
	   are simply set to 0 */

	/* not standard snowbros video */

	bitmap.fill(0xf0, cliprect);

	for (int offs = 0x0000 / 2; offs < 0x2000 / 2; offs += 8)
	{
		/* Similar to Honey Doll, but no 8bpp list / gfx */

		int dx         = m_bootleg_spriteram[offs + 4] & 0xff;
		int dy         = m_bootleg_spriteram[offs + 5] & 0xff;
		int tilecolour = m_bootleg_spriteram[offs + 3];
		const u8 attr  = m_bootleg_spriteram[offs + 7] & 0xff;
		bool flipx     = BIT(attr, 7);
		bool flipy     = BIT(attr, 6);
		const u32 tile = ((attr & 0x3f) << 8) + (m_bootleg_spriteram[offs + 6] & 0xff);

		if (flip_screen())
		{
			dx = 240 - dx;
			dy = 240 - dy;
			flipx = !flipx;
			flipy = !flipy;
		}

		tilecolour = (tilecolour & 0x00f0) >> 4;
		tilecolour ^= 0xf; // unusual, but correct..

		m_gfxdecode->gfx(0)->transpen(bitmap, cliprect,
				tile,
				tilecolour,
				flipx, flipy,
				dx, dy, 0);
	}
	return 0;
}


u32 snowbros_state::screen_update_wintbob(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	for (int offs = 0; offs < m_bootleg_spriteram.bytes() / 2; offs += 8)
	{
		const u8 attr = m_bootleg_spriteram[offs + 1] & 0xff;
		if (BIT(attr, 1))
			continue;

		int xpos           = m_bootleg_spriteram[offs] & 0xff;
		int ypos           = m_bootleg_spriteram[offs + 4] & 0xff;
/*      const bool unk1    = BIT(attr, 0);*/  /* Unknown .. Set for the Bottom Left part of Sprites */
/*      const bool unk2    = BIT(attr, 2);*/  /* Unknown .. Set for most things */
		const bool wrapr   = BIT(attr, 3);
		const u32 colr     = (attr & 0xf0) >> 4;
		const u8 tile_flip = m_bootleg_spriteram[offs + 2] & 0xff;
		const u32 tile     = (tile_flip << 8) + (m_bootleg_spriteram[offs + 3] & 0xff);
		bool flipx         = BIT(tile_flip, 7);
		bool flipy         = BIT(tile_flip, 6);

		if (wrapr)
			xpos -= 256;

		if (flip_screen())
		{
			xpos = 240 - xpos;
			ypos = 240 - ypos;
			flipx = !flipx;
			flipy = !flipy;
		}

		if ((xpos > -16) && (ypos > 0) && (xpos < 256) && (ypos < 240))
		{
			m_gfxdecode->gfx(0)->transpen(bitmap, cliprect,
					tile,
					colr,
					flipx, flipy,
					xpos, ypos, 0);
		}
	}
	return 0;
}


u32 snowbros3_state::screen_update_snowbro3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x = 0, y = 0;

	/*
	 * Sprite Tile Format (Only low bytes are used?)
	 * ------------------
	 *
	 * Byte | Bit(s)   | Use
	 * -----+-76543210-+----------------
	 *  0-5 | -------- | ?
	 *  6   | -------- | ?
	 *  7   | xxxx.... | Palette Bank
	 *  7   | .....x.. | Use Relative offsets
	 *  7   | ......x. | YPos - Sign Bit
	 *  7   | .......x | XPos - Sign Bit
	 *  9   | xxxxxxxx | XPos
	 *  B   | xxxxxxxx | YPos
	 *  C   | -------- | ?
	 *  D   | xxxxxxxx | Sprite Number (low 8 bits)
	 *  E   | -------- | ?
	 *  F   | x....... | Flip Sprite Y-Axis
	 *  F   | .x...... | Flip Sprite X-Axis
	 *  F   | ....xxxx | Sprite Number (high 4 bits)
	 */

	/* This clears & redraws the entire screen each pass */

	bitmap.fill(m_palette->black_pen(), cliprect);

	for (int offs = 0; offs < m_bootleg_spriteram.bytes() / 2; offs += 8)
	{
		u8 gfx         = 0;
		int dx         = m_bootleg_spriteram[offs + 4] & 0xff;
		int dy         = m_bootleg_spriteram[offs + 5] & 0xff;
		u8 tilecolour  = m_bootleg_spriteram[offs + 3] & 0xff;
		const u8 attr  = m_bootleg_spriteram[offs + 7] & 0xff;
		bool flipx     = BIT(attr, 7);
		bool flipy     = BIT(attr, 6);
		const u32 tile = ((attr & 0xff) << 8) + (m_bootleg_spriteram[offs + 6] & 0xff);

		if (BIT(tilecolour, 0))
			dx = -1 - (dx ^ 0xff);
		if (BIT(tilecolour, 1))
			dy = -1 - (dy ^ 0xff);

		if (BIT(tilecolour, 2))
		{
			x += dx;
			y += dy;
		}
		else
		{
			x = dx;
			y = dy;
		}

		if (x > 511) x &= 0x1ff;
		if (y > 511) y &= 0x1ff;

		int sx = x, sy = y;
		if (flip_screen())
		{
			sx = 240 - x;
			sy = 240 - y;
			flipx = !flipx;
			flipy = !flipy;
		}

		if (offs < 0x800) /* i guess this is the right way */
		{
			gfx = 1;
			tilecolour = 0x10;
		}

		m_gfxdecode->gfx(gfx)->transpen(bitmap, cliprect,
					tile,
					(tilecolour & 0xf0) >> 4,
					flipx, flipy,
					sx, sy, 0);
	}
	return 0;
}
