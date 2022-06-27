// license:BSD-3-Clause
// copyright-holders:Uki
/*******************************************************************************

    Momoko 120% (c) 1986 Jaleco

    Video hardware driver by Uki 02/Mar/2001

    Video consists of
    - Sprites
    - 2x ROM based tilemaps (bg and fg) with priority over some sprites
    - 1x RAM based tilemap (text layer) (no wrapping?)

    TODO:
    - update to use tilemap system?
    - check if any of this is common Jaleco/NMK etc. hardware and use shared
      devices if possible

*******************************************************************************/

#include "emu.h"
#include "momoko.h"


void momoko_state::fg_scrollx_w(u8 data)
{
	m_fg_scrollx = data;
}

void momoko_state::fg_scrolly_w(u8 data)
{
	m_fg_scrolly = data;
}

void momoko_state::fg_select_w(u8 data)
{
	m_fg_select = data & 0x0f;
	m_fg_mask = data & 0x10;
}

void momoko_state::text_scrolly_w(u8 data)
{
	m_text_scrolly = data;
}

void momoko_state::text_mode_w(u8 data)
{
	m_text_mode = data;
}

void momoko_state::bg_scrollx_w(offs_t offset, u8 data)
{
	m_bg_scrollx[offset] = data;
}

void momoko_state::bg_scrolly_w(offs_t offset, u8 data)
{
	m_bg_scrolly[offset] = data;
}

void momoko_state::bg_select_w(u8 data)
{
	m_bg_select = data & 0x0f;
	m_bg_mask = data & 0x10;
}

void momoko_state::bg_priority_w(u8 data)
{
	m_bg_priority = data & 0x01;
}

void momoko_state::flipscreen_w(u8 data)
{
	m_flipscreen = data & 0x01;
}

/****************************************************************************/

void momoko_state::draw_bg_pri(bitmap_ind16 &bitmap, int chr, int col, int flipx, int flipy, int x, int y, int pri)
{
	for (int sy = 0; sy < 8; sy++)
	{
		const u32 gfxadr = chr * 16 + sy * 2;
		for (int xx = 0; xx < 2; xx++)
		{
			u8 d0 = m_bg_gfx[gfxadr + xx * 4096];
			u8 d1 = m_bg_gfx[gfxadr + xx * 4096 + 1];

			for (int sx = 0; sx < 4; sx++)
			{
				const u8 dot = (d0 & 0x08) | ((d0 & 0x80) >> 5) | ((d1 & 0x08) >> 2) | ((d1 & 0x80) >> 7);
				const int px = (flipx == 0) ? (sx + xx * 4 + x) : (7 - sx - xx * 4 + x);
				const int py = (flipy == 0) ? (sy + y) : (7 - sy + y);

				if (dot >= pri)
					bitmap.pix(py, px) = col * 16 + dot + 256;

				d0 <<= 1;
				d1 <<= 1;
			}
		}
	}
}


void momoko_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int start, int end, int flip)
{
	for (int offs = start; offs < end; offs += 4)
	{
		int px, py;
		u32 chr = m_spriteram[offs + 1] | ((m_spriteram[offs + 2] & 0x60) << 3);
		chr = ((chr & 0x380) << 1) | (chr & 0x7f);
		int col = m_spriteram[offs + 2] & 0x07;
		const int fx = ((m_spriteram[offs + 2] & 0x10) >> 4) ^ flip;
		const int fy = ((m_spriteram[offs + 2] & 0x08) >> 3) ^ flip; /* ??? */
		int x = m_spriteram[offs + 3];
		int y = m_spriteram[offs + 0];

		if (flip == 0)
		{
			px = x;
			py = 239 - y;
		}
		else
		{
			px = 248 - x;
			py = y + 1;
		}
		m_gfxdecode->gfx(3)->transpen(bitmap, cliprect,
			chr,
			col,
			!fx, fy,
			px, py, 0);
	}
}

void momoko_state::draw_text_tilemap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int flip)
{
	for (int y = 16; y < 240; y++)
	{
		for (int x = 0; x < 32; x++)
		{
			int px, py, col;
			int sy = y;
			if (m_text_mode == 0)
				col = m_proms[(sy >> 3) + 0x100] & 0x0f;
			else
			{
				if (m_proms[y] < 0x08)
					sy += m_text_scrolly;
				col = (m_proms[y] & 0x07) + 0x10;
			}
			int dy = sy & 7;
			if (flip == 0)
			{
				px = x * 8;
				py = y;
			}
			else
			{
				px = 248 - x * 8;
				py = 255 - y;
			}
			int ramoffset = (sy >> 3) * 32 + x;

			if (ramoffset < 0x400) // high score table, no wrapping?
				m_gfxdecode->gfx(0)->transpen(bitmap, cliprect,
					m_videoram[ramoffset] * 8 + dy,
					col,
					flip, 0,
					px, py, 0);
		}
	}
}

void momoko_state::draw_fg_romtilemap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int flip)
{
	int dx = (7 - m_fg_scrollx) & 7;
	int dy = (7 - m_fg_scrolly) & 7;
	int rx = m_fg_scrollx >> 3;
	int ry = m_fg_scrolly >> 3;

	for (int y = 0; y < 29; y++)
	{
		for (int x = 0; x < 32; x++)
		{
			int px, py;
			const int radr = ((ry + y + 34) & 0x3f) * 0x20 + ((rx + x) & 0x1f) + (m_fg_select & 3) * 0x800;
			const u32 chr = m_fg_map[radr];
			if (flip == 0)
			{
				px = 8 * x + dx - 6;
				py = 8 * y + dy + 9;
			}
			else
			{
				px = 248 - (8 * x + dx - 8);
				py = 248 - (8 * y + dy + 9);
			}
			m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
				chr,
				0, /* color */
				flip,flip, /* flip */
				px,py,0);
		}
	}
}

void momoko_state::draw_bg_romtilemap(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect, int flip, bool high)
{
	int dx = (7 - m_bg_scrollx[0]) & 7;
	int dy = (7 - m_bg_scrolly[0]) & 7;
	int rx = (m_bg_scrollx[0] + m_bg_scrollx[1] * 256) >> 3;
	int ry = (m_bg_scrolly[0] + m_bg_scrolly[1] * 256) >> 3;

	for (int y = 0; y < 29; y++)
	{
		for (int x = 0; x < 32; x++)
		{
			int px, py;
			const int radr = ((ry + y + 2) & 0x3ff) * 128 + ((rx + x) & 0x7f);
			u32 chr = m_bg_map[radr];
			int col = m_bg_col_map[chr + m_bg_select * 512 + m_bg_priority * 256];
			chr = chr + m_bg_select * 512;

			if (flip == 0)
			{
				px = 8 * x + dx - 6;
				py = 8 * y + dy + 9;
			}
			else
			{
				px = 248 - (8 * x + dx - 8);
				py = 248 - (8 * y + dy + 9);
			}

			if (!high)
			{
				m_gfxdecode->gfx(1)->opaque(bitmap,cliprect,
					chr,
					col,
					flip,flip,
					px,py);
			}
			else
			{
				const u8 pri = (col & 0x10) >> 1;
				if (pri != 0)
				{
					col = col & 0x0f;
					draw_bg_pri(bitmap, chr, col, flip, flip, px, py, pri);
				}
			}
		}
	}
}

/****************************************************************************/

u32 momoko_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const int flip = m_flipscreen ^ (m_io_fake->read() & 0x01);

	/* draw BG layer - all tiles */

	if (m_bg_mask == 0)
		draw_bg_romtilemap(screen, bitmap, cliprect, flip, false);
	else
		bitmap.fill(256, cliprect);

	/* draw sprites (momoko) */
	draw_sprites(screen, bitmap, cliprect, 0, 9 * 4, flip);

	/* draw BG layer - high priority tiles */
	if (m_bg_mask == 0)
		draw_bg_romtilemap(screen, bitmap, cliprect, flip, true);

	/* draw sprites (others) */
	draw_sprites(screen, bitmap, cliprect, 9*4, m_spriteram.bytes(), flip);

	/* draw text layer */
	draw_text_tilemap(screen, bitmap, cliprect, flip);

	/* draw FG layer */
	if (m_fg_mask == 0)
		draw_fg_romtilemap(screen, bitmap, cliprect, flip);

	return 0;
}
