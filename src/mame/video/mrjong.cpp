// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/mrjong.h"


/***************************************************************************

  Convert the color PROMs. (from video/pengo.c)

***************************************************************************/

PALETTE_INIT_MEMBER(mrjong_state, mrjong)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	/* create a lookup table for the palette */
	for (i = 0; i < 0x10; i++)
	{
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = 0;
		bit1 = BIT(color_prom[i], 6);
		bit2 = BIT(color_prom[i], 7);
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x20;

	/* characters/sprites */
	for (i = 0; i < 0x80; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		palette.set_pen_indirect(i, ctabentry);
	}
}


/***************************************************************************

  Display control parameter.

***************************************************************************/

WRITE8_MEMBER(mrjong_state::mrjong_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(mrjong_state::mrjong_colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(mrjong_state::mrjong_flipscreen_w)
{
	if (flip_screen() != BIT(data, 2))
	{
		flip_screen_set(BIT(data, 2));
		machine().tilemap().mark_all_dirty();
	}
}

TILE_GET_INFO_MEMBER(mrjong_state::get_bg_tile_info)
{
	int code = m_videoram[tile_index] | ((m_colorram[tile_index] & 0x20) << 3);
	int color = m_colorram[tile_index] & 0x1f;
	int flags = ((m_colorram[tile_index] & 0x40) ? TILE_FLIPX : 0) | ((m_colorram[tile_index] & 0x80) ? TILE_FLIPY : 0);

	SET_TILE_INFO_MEMBER(0, code, color, flags);
}

void mrjong_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(mrjong_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS_FLIP_XY, 8, 8, 32, 32);
}

/*
Note: First 0x40 entries in the videoram are actually spriteram
*/
void mrjong_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int offs;

	for (offs = (0x40 - 4); offs >= 0; offs -= 4)
	{
		int sprt;
		int color;
		int sx, sy;
		int flipx, flipy;

		sprt = (((m_videoram[offs + 1] >> 2) & 0x3f) | ((m_videoram[offs + 3] & 0x20) << 1));
		flipx = (m_videoram[offs + 1] & 0x01) >> 0;
		flipy = (m_videoram[offs + 1] & 0x02) >> 1;
		color = (m_videoram[offs + 3] & 0x1f);

		sx = 224 - m_videoram[offs + 2];
		sy = m_videoram[offs + 0];
		if (flip_screen())
		{
			sx = 208 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				sprt,
				color,
				flipx, flipy,
				sx, sy, 0);
	}
}

UINT32 mrjong_state::screen_update_mrjong(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}
