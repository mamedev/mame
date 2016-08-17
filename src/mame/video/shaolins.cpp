// license:BSD-3-Clause
// copyright-holders:Allard van der Bas
/***************************************************************************

  shaolins.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/shaolins.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Shao-lin's Road has three 256x4 palette PROMs (one per gun) and two 256x4
  lookup table PROMs (one for characters, one for sprites).
  I don't know for sure how the palette PROMs are connected to the RGB
  output, but it's probably the usual:

  bit 3 -- 220 ohm resistor  -- RED/GREEN/BLUE
        -- 470 ohm resistor  -- RED/GREEN/BLUE
        -- 1  kohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 2.2kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/
PALETTE_INIT_MEMBER(shaolins_state, shaolins)
{
	const UINT8 *color_prom = memregion("proms")->base();
	static const int resistances[4] = { 2200, 1000, 470, 220 };
	double rweights[4], gweights[4], bweights[4];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0, 255, -1.0,
			4, resistances, rweights, 470, 0,
			4, resistances, gweights, 470, 0,
			4, resistances, bweights, 470, 0);

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

WRITE8_MEMBER(shaolins_state::videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(shaolins_state::colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(shaolins_state::palettebank_w)
{
	if (m_palettebank != (data & 0x07))
	{
		m_palettebank = data & 0x07;
		machine().tilemap().mark_all_dirty();
	}
}

WRITE8_MEMBER(shaolins_state::scroll_w)
{
	for (int col = 4; col < 32; col++)
		m_bg_tilemap->set_scrolly(col, data + 1);
}

WRITE8_MEMBER(shaolins_state::nmi_w)
{
	m_nmi_enable = data;

	if (flip_screen() != (data & 0x01))
	{
		flip_screen_set(data & 0x01);
		machine().tilemap().mark_all_dirty();
	}
}

TILE_GET_INFO_MEMBER(shaolins_state::get_bg_tile_info)
{
	int attr = m_colorram[tile_index];
	int code = m_videoram[tile_index] + ((attr & 0x40) << 2);
	int color = (attr & 0x0f) + 16 * m_palettebank;
	int flags = (attr & 0x20) ? TILE_FLIPY : 0;

	SET_TILE_INFO_MEMBER(0, code, color, flags);
}

void shaolins_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(shaolins_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS,
			8, 8, 32, 32);

	m_bg_tilemap->set_scroll_cols(32);

	save_item(NAME(m_palettebank));
	save_item(NAME(m_nmi_enable));
}

void shaolins_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = m_spriteram.bytes() - 32; offs >= 0; offs -= 32 ) /* max 24 sprites */
	{
		if (m_spriteram[offs] && m_spriteram[offs + 6]) /* stop rogue sprites on high score screen */
		{
			int code = m_spriteram[offs + 8];
			int color = (m_spriteram[offs + 9] & 0x0f) | (m_palettebank << 4);
			int flipx = !(m_spriteram[offs + 9] & 0x40);
			int flipy = m_spriteram[offs + 9] & 0x80;
			int sx = 240 - m_spriteram[offs + 6];
			int sy = 248 - m_spriteram[offs + 4];

			if (flip_screen())
			{
				sx = 240 - sx;
				sy = 248 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			m_gfxdecode->gfx(1)->transmask(bitmap,cliprect,
				code, color,
				flipx, flipy,
				sx, sy,
				m_palette->transpen_mask(*m_gfxdecode->gfx(1), color, m_palettebank << 5));
		}
	}
}

UINT32 shaolins_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}
