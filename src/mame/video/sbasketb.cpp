// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/sbasketb.h"

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Super Basketball has three 256x4 palette PROMs (one per gun) and two 256x4
  lookup table PROMs (one for characters, one for sprites).
  I don't know for sure how the palette PROMs are connected to the RGB
  output, but it's probably the usual:

  bit 3 -- 220 ohm resistor  -- RED/GREEN/BLUE
        -- 470 ohm resistor  -- RED/GREEN/BLUE
        -- 1  kohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 2.2kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/

PALETTE_INIT_MEMBER(sbasketb_state, sbasketb)
{
	const UINT8 *color_prom = memregion("proms")->base();
	static const int resistances[4] = { 2000, 1000, 470, 220 };
	double rweights[4], gweights[4], bweights[4];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0, 255, -1.0,
			4, resistances, rweights, 1000, 0,
			4, resistances, gweights, 1000, 0,
			4, resistances, bweights, 1000, 0);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x100; i++)
	{
		int bit0, bit1, bit2, bit3;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i + 0x000] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x000] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x000] >> 2) & 0x01;
		bit3 = (color_prom[i + 0x000] >> 3) & 0x01;
		r = combine_4_weights(rweights, bit0, bit1, bit2, bit3);

		/* green component */
		bit0 = (color_prom[i + 0x100] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x100] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x100] >> 2) & 0x01;
		bit3 = (color_prom[i + 0x100] >> 3) & 0x01;
		g = combine_4_weights(gweights, bit0, bit1, bit2, bit3);

		/* blue component */
		bit0 = (color_prom[i + 0x200] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x200] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x200] >> 2) & 0x01;
		bit3 = (color_prom[i + 0x200] >> 3) & 0x01;
		b = combine_4_weights(bweights, bit0, bit1, bit2, bit3);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table,*/
	color_prom += 0x300;

	/* characters use colors 0xf0-0xff */
	for (i = 0; i < 0x100; i++)
	{
		UINT8 ctabentry = (color_prom[i] & 0x0f) | 0xf0;
		palette.set_pen_indirect(i, ctabentry);
	}

	/* sprites use colors 0-256 (?) in 16 banks */
	for (i = 0; i < 0x100; i++)
	{
		int j;

		for (j = 0; j < 0x10; j++)
		{
			UINT8 ctabentry = (j << 4) | (color_prom[i + 0x100] & 0x0f);
			palette.set_pen_indirect(0x100 + ((j << 8) | i), ctabentry);
		}
	}
}

WRITE8_MEMBER(sbasketb_state::sbasketb_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(sbasketb_state::sbasketb_colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(sbasketb_state::sbasketb_flipscreen_w)
{
	if (flip_screen() != data)
	{
		flip_screen_set(data);
		machine().tilemap().mark_all_dirty();
	}
}

TILE_GET_INFO_MEMBER(sbasketb_state::get_bg_tile_info)
{
	int code = m_videoram[tile_index] + ((m_colorram[tile_index] & 0x20) << 3);
	int color = m_colorram[tile_index] & 0x0f;
	int flags = ((m_colorram[tile_index] & 0x40) ? TILE_FLIPX : 0) | ((m_colorram[tile_index] & 0x80) ? TILE_FLIPY : 0);

	SET_TILE_INFO_MEMBER(0, code, color, flags);
}

void sbasketb_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(sbasketb_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap->set_scroll_cols(32);
}

void sbasketb_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	UINT8 *spriteram = m_spriteram;
	int offs = (*m_spriteram_select & 0x01) * 0x100;
	int i;

	for (i = 0; i < 64; i++, offs += 4)
	{
		int sx = spriteram[offs + 2];
		int sy = spriteram[offs + 3];

		if (sx || sy)
		{
			int code  =  spriteram[offs + 0] | ((spriteram[offs + 1] & 0x20) << 3);
			int color = (spriteram[offs + 1] & 0x0f) + 16 * *m_palettebank;
			int flipx =  spriteram[offs + 1] & 0x40;
			int flipy =  spriteram[offs + 1] & 0x80;

			if (flip_screen())
			{
				sx = 240 - sx;
				sy = 240 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}


				m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				code, color,
				flipx, flipy,
				sx, sy, 0);
		}
	}
}

UINT32 sbasketb_state::screen_update_sbasketb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int col;

	for (col = 6; col < 32; col++)
		m_bg_tilemap->set_scrolly(col, *m_scroll);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}
