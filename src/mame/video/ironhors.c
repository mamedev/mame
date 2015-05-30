// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni
/***************************************************************************

  ironhors.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/ironhors.h"

/***************************************************************************

  Convert the color PROMs into a more useable format.

***************************************************************************/

PALETTE_INIT_MEMBER(ironhors_state, ironhors)
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

	/* characters use colors 0x10-0x1f of each 0x20 color bank,
	   while sprites use colors 0-0x0f */
	for (i = 0; i < 0x200; i++)
	{
		int j;

		for (j = 0; j < 8; j++)
		{
			UINT8 ctabentry = (j << 5) | ((~i & 0x100) >> 4) | (color_prom[i] & 0x0f);
			palette.set_pen_indirect(((i & 0x100) << 3) | (j << 8) | (i & 0xff), ctabentry);
		}
	}
}

WRITE8_MEMBER(ironhors_state::videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(ironhors_state::colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(ironhors_state::charbank_w)
{
	if (m_charbank != (data & 0x03))
	{
		m_charbank = data & 0x03;
		machine().tilemap().mark_all_dirty();
	}

	m_spriterambank = data & 0x08;

	/* other bits unknown */
}

WRITE8_MEMBER(ironhors_state::palettebank_w)
{
	if (m_palettebank != (data & 0x07))
	{
		m_palettebank = data & 0x07;
		machine().tilemap().mark_all_dirty();
	}

	coin_counter_w(machine(), 0, data & 0x10);
	coin_counter_w(machine(), 1, data & 0x20);

	/* bit 6 unknown - set after game over */

	if (data & 0x88)
		popmessage("palettebank_w %02x",data);
}

WRITE8_MEMBER(ironhors_state::flipscreen_w)
{
	if (flip_screen() != (~data & 0x08))
	{
		flip_screen_set(~data & 0x08);
		machine().tilemap().mark_all_dirty();
	}

	/* other bits are used too, but unknown */
}

TILE_GET_INFO_MEMBER(ironhors_state::get_bg_tile_info)
{
	int code = m_videoram[tile_index] + ((m_colorram[tile_index] & 0x40) << 2) +
		((m_colorram[tile_index] & 0x20) << 4) + (m_charbank << 10);
	int color = (m_colorram[tile_index] & 0x0f) + 16 * m_palettebank;
	int flags = ((m_colorram[tile_index] & 0x10) ? TILE_FLIPX : 0) |
		((m_colorram[tile_index] & 0x20) ? TILE_FLIPY : 0);

	SET_TILE_INFO_MEMBER(0, code, color, flags);
}

void ironhors_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(ironhors_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_bg_tilemap->set_scroll_rows(32);
}

void ironhors_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int offs;
	UINT8 *sr;

	if (m_spriterambank != 0)
		sr = m_spriteram;
	else
		sr = m_spriteram2;

	for (offs = 0; offs < m_spriteram.bytes(); offs += 5)
	{
		int sx = sr[offs + 3];
		int sy = sr[offs + 2];
		int flipx = sr[offs + 4] & 0x20;
		int flipy = sr[offs + 4] & 0x40;
		int code = (sr[offs] << 2) + ((sr[offs + 1] & 0x03) << 10) + ((sr[offs + 1] & 0x0c) >> 2);
		int color = ((sr[offs + 1] & 0xf0) >> 4) + 16 * m_palettebank;
	//  int mod = flip_screen() ? -8 : 8;

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		switch (sr[offs + 4] & 0x0c)
		{
			case 0x00:  /* 16x16 */
				m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
						code/4,
						color,
						flipx,flipy,
						sx,sy,0);
				break;

			case 0x04:  /* 16x8 */
				{
					if (flip_screen()) sy += 8; // this fixes the train wheels' position

					m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
							code & ~1,
							color,
							flipx,flipy,
							flipx?sx+8:sx,sy,0);
					m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
							code | 1,
							color,
							flipx,flipy,
							flipx?sx:sx+8,sy,0);
				}
				break;

			case 0x08:  /* 8x16 */
				{
					m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
							code & ~2,
							color,
							flipx,flipy,
							sx,flipy?sy+8:sy,0);
					m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
							code | 2,
							color,
							flipx,flipy,
							sx,flipy?sy:sy+8,0);
				}
				break;

			case 0x0c:  /* 8x8 */
				{
					m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
							code,
							color,
							flipx,flipy,
							sx,sy,0);
				}
				break;
		}
	}
}

UINT32 ironhors_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int row;

	for (row = 0; row < 32; row++)
		m_bg_tilemap->set_scrollx(row, m_scroll[row]);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}

TILE_GET_INFO_MEMBER(ironhors_state::farwest_get_bg_tile_info)
{
	int code = m_videoram[tile_index] + ((m_colorram[tile_index] & 0x40) << 2) +
		((m_colorram[tile_index] & 0x20) << 4) + (m_charbank << 10);
	int color = (m_colorram[tile_index] & 0x0f) + 16 * m_palettebank;
	int flags = 0;//((m_colorram[tile_index] & 0x10) ? TILE_FLIPX : 0) |  ((m_colorram[tile_index] & 0x20) ? TILE_FLIPY : 0);

	SET_TILE_INFO_MEMBER(0, code, color, flags);
}

VIDEO_START_MEMBER(ironhors_state,farwest)
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(ironhors_state::farwest_get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_bg_tilemap->set_scroll_rows(32);
}

void ironhors_state::farwest_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int offs;
	UINT8 *sr = m_spriteram2;
	UINT8 *sr2 = m_spriteram;

	for (offs = 0; offs < m_spriteram.bytes(); offs += 4)
	{
		int sx = sr[offs + 2];
		int sy = sr[offs + 1];
		int flipx = sr[offs + 3] & 0x20;
		int flipy = sr[offs + 3] & 0x40;
		int code = (sr[offs] << 2) + ((sr2[offs] & 0x03) << 10) + ((sr2[offs] & 0x0c) >> 2);
		int color = ((sr2[offs] & 0xf0) >> 4) + 16 * m_palettebank;

	//  int mod = flip_screen() ? -8 : 8;

//      if (flip_screen())
		{
		//  sx = 240 - sx;
			sy = 240 - sy;
		//  flipx = !flipx;
		//  flipy = !flipy;
		}

		switch (sr[offs + 3] & 0x0c)
		{
			case 0x00:  /* 16x16 */
				m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
						code/4,
						color,
						flipx,flipy,
						sx,sy,0);
				break;

			case 0x04:  /* 16x8 */
				{
					if (flip_screen()) sy += 8; // this fixes the train wheels' position

					m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
							code & ~1,
							color,
							flipx,flipy,
							flipx?sx+8:sx,sy,0);
					m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
							code | 1,
							color,
							flipx,flipy,
							flipx?sx:sx+8,sy,0);
				}
				break;

			case 0x08:  /* 8x16 */
				{
					m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
							code & ~2,
							color,
							flipx,flipy,
							sx,flipy?sy+8:sy,0);
					m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
							code | 2,
							color,
							flipx,flipy,
							sx,flipy?sy:sy+8,0);
				}
				break;

			case 0x0c:  /* 8x8 */
				{
					m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
							code,
							color,
							flipx,flipy,
							sx,sy,0);
				}
				break;
		}
	}
}

UINT32 ironhors_state::screen_update_farwest(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int row;

	for (row = 0; row < 32; row++)
		m_bg_tilemap->set_scrollx(row, m_scroll[row]);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	farwest_draw_sprites(bitmap, cliprect);
	return 0;
}
