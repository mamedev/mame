// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
#include "emu.h"
#include "video/resnet.h"
#include "includes/kingobox.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

  King of Boxer has three 256x4 palette PROMs, connected to the RGB output
  this way:

  bit 3 -- 180 ohm resistor  -- RED/GREEN/BLUE
        -- 360 ohm resistor  -- RED/GREEN/BLUE
        -- 750 ohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 1.5kohm resistor  -- RED/GREEN/BLUE

  The foreground color code directly goes to the RGB output, this way:

  bit 5 --  51 ohm resistor  -- RED
  bit 4 --  51 ohm resistor  -- GREEN
  bit 3 --  51 ohm resistor  -- BLUE

***************************************************************************/

void kingofb_state::palette_init_common( palette_device &palette, const UINT8 *color_prom, void (kingofb_state::*get_rgb_data)(const UINT8 *, int, int *, int *, int *) )
{
	static const int resistances[4] = { 1500, 750, 360, 180 };
	static const int resistances_fg[1] = { 51 };
	double rweights[4], gweights[4], bweights[4];
	double rweights_fg[1], gweights_fg[1], bweights_fg[1];
	int i;

	/* compute the color output resistor weights */
	double scale = compute_resistor_weights(0, 255, -1.0,
						1, resistances_fg, rweights_fg, 0, 0,
						1, resistances_fg, gweights_fg, 0, 0,
						1, resistances_fg, bweights_fg, 0, 0);

					compute_resistor_weights(0, 255, scale,
						4, resistances, rweights, 470, 0,
						4, resistances, gweights, 470, 0,
						4, resistances, bweights, 470, 0);

	for (i = 0; i < 0x100; i++)
	{
		int r_data, g_data, b_data;
		int bit0, bit1, bit2, bit3;
		int r, g, b;

		(this->*get_rgb_data)(color_prom, i, &r_data, &g_data, &b_data);

		/* red component */
		bit0 = (r_data >> 0) & 0x01;
		bit1 = (r_data >> 1) & 0x01;
		bit2 = (r_data >> 2) & 0x01;
		bit3 = (r_data >> 3) & 0x01;
		r = combine_4_weights(rweights, bit0, bit1, bit2, bit3);

		/* green component */
		bit0 = (g_data >> 0) & 0x01;
		bit1 = (g_data >> 1) & 0x01;
		bit2 = (g_data >> 2) & 0x01;
		bit3 = (g_data >> 3) & 0x01;
		g = combine_4_weights(gweights, bit0, bit1, bit2, bit3);

		/* blue component */
		bit0 = (b_data >> 0) & 0x01;
		bit1 = (b_data >> 1) & 0x01;
		bit2 = (b_data >> 2) & 0x01;
		bit3 = (b_data >> 3) & 0x01;
		b = combine_4_weights(bweights, bit0, bit1, bit2, bit3);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	/* the foreground chars directly map to primary colors */
	for (i = 0x100; i < 0x108; i++)
	{
		int r, g, b;

		/* red component */
		r = (((i - 0x100) >> 2) & 0x01) * rweights_fg[0];

		/* green component */
		g = (((i - 0x100) >> 1) & 0x01) * gweights_fg[0];

		/* blue component */
		b = (((i - 0x100) >> 0) & 0x01) * bweights_fg[0];

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	for (i = 0; i < 0x100; i++)
		palette.set_pen_indirect(i, i);

	for (i = 0x101; i < 0x110; i += 2)
	{
		UINT16 ctabentry = ((i - 0x101) >> 1) | 0x100;
		palette.set_pen_indirect(i, ctabentry);
	}
}


void kingofb_state::kingofb_get_rgb_data( const UINT8 *color_prom, int i, int *r_data, int *g_data, int *b_data )
{
	*r_data = color_prom[i + 0x000] & 0x0f;
	*g_data = color_prom[i + 0x100] & 0x0f;
	*b_data = color_prom[i + 0x200] & 0x0f;
}


void kingofb_state::ringking_get_rgb_data( const UINT8 *color_prom, int i, int *r_data, int *g_data, int *b_data )
{
	*r_data = (color_prom[i + 0x000] >> 4) & 0x0f;
	*g_data = (color_prom[i + 0x000] >> 0) & 0x0f;
	*b_data = (color_prom[i + 0x100] >> 0) & 0x0f;
}


PALETTE_INIT_MEMBER(kingofb_state,kingofb)
{
	const UINT8 *color_prom = memregion("proms")->base();
	palette_init_common(palette, color_prom, &kingofb_state::kingofb_get_rgb_data);
}

PALETTE_INIT_MEMBER(kingofb_state,ringking)
{
	const UINT8 *color_prom = memregion("proms")->base();
	palette_init_common(palette, color_prom, &kingofb_state::ringking_get_rgb_data);
}

WRITE8_MEMBER(kingofb_state::kingofb_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(kingofb_state::kingofb_colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(kingofb_state::kingofb_videoram2_w)
{
	m_videoram2[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(kingofb_state::kingofb_colorram2_w)
{
	m_colorram2[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(kingofb_state::kingofb_f800_w)
{
	m_nmi_enable = data & 0x20;

	if (m_palette_bank != ((data & 0x18) >> 3))
	{
		m_palette_bank = (data & 0x18) >> 3;
		m_bg_tilemap->mark_all_dirty();
	}

	if (flip_screen() != (data & 0x80))
	{
		flip_screen_set(data & 0x80);
		machine().tilemap().mark_all_dirty();
	}
}

TILE_GET_INFO_MEMBER(kingofb_state::get_bg_tile_info)
{
	int attr = m_colorram[tile_index];
	int bank = ((attr & 0x04) >> 2) + 2;
	int code = (tile_index / 16) ? m_videoram[tile_index] + ((attr & 0x03) << 8) : 0;
	int color = ((attr & 0x70) >> 4) + 8 * m_palette_bank;

	SET_TILE_INFO_MEMBER(bank, code, color, 0);
}

TILE_GET_INFO_MEMBER(kingofb_state::get_fg_tile_info)
{
	int attr = m_colorram2[tile_index];
	int bank = (attr & 0x02) >> 1;
	int code = m_videoram2[tile_index] + ((attr & 0x01) << 8);
	int color = (attr & 0x38) >> 3;

	SET_TILE_INFO_MEMBER(bank, code, color, 0);
}

VIDEO_START_MEMBER(kingofb_state,kingofb)
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(kingofb_state::get_bg_tile_info),this), TILEMAP_SCAN_COLS_FLIP_Y, 16, 16, 16, 16);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(kingofb_state::get_fg_tile_info),this), TILEMAP_SCAN_COLS_FLIP_Y,  8,  8, 32, 32);

	m_fg_tilemap->set_transparent_pen(0);
}

void kingofb_state::kingofb_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 *spriteram = m_spriteram;
	int offs;

	for (offs = 0; offs < m_spriteram.bytes(); offs += 4)
	{
		int roffs, bank, code, color, flipx, flipy, sx, sy;

		/* the offset into spriteram seems scrambled */
		roffs = BITSWAP16(offs,15,14,13,12,11,10,4,7,6,5,9,8,3,2,1,0) ^ 0x3c;
		if (roffs & 0x200)
			roffs ^= 0x1c0;

		bank = (spriteram[roffs + 3] & 0x04) >> 2;
		code = spriteram[roffs + 2] + ((spriteram[roffs + 3] & 0x03) << 8);
		color = ((spriteram[roffs + 3] & 0x70) >> 4) + 8 * m_palette_bank;
		flipx = 0;
		flipy = spriteram[roffs + 3] & 0x80;
		sx = spriteram[roffs + 1];
		sy = spriteram[roffs];

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(2 + bank)->transpen(bitmap,cliprect,
			code, color,
			flipx, flipy,
			sx, sy, 0);
	}
}

UINT32 kingofb_state::screen_update_kingofb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrolly(0, -(*m_scroll_y));
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	kingofb_draw_sprites(bitmap, cliprect);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

/* Ring King */

TILE_GET_INFO_MEMBER(kingofb_state::ringking_get_bg_tile_info)
{
	int code = (tile_index / 16) ? m_videoram[tile_index] : 0;
	int color = ((m_colorram[tile_index] & 0x70) >> 4) + 8 * m_palette_bank;

	SET_TILE_INFO_MEMBER(4, code, color, 0);
}

VIDEO_START_MEMBER(kingofb_state,ringking)
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(kingofb_state::ringking_get_bg_tile_info),this), TILEMAP_SCAN_COLS_FLIP_Y, 16, 16, 16, 16);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(kingofb_state::get_fg_tile_info),this), TILEMAP_SCAN_COLS_FLIP_Y, 8, 8, 32, 32);

	m_fg_tilemap->set_transparent_pen(0);
}

void kingofb_state::ringking_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	UINT8 *spriteram = m_spriteram;
	int offs;

	for (offs = 0; offs < m_spriteram.bytes(); offs += 4)
	{
		int bank = (spriteram[offs + 1] & 0x04) >> 2;
		int code = spriteram[offs + 3] + ((spriteram[offs + 1] & 0x03) << 8);
		int color = ((spriteram[offs + 1] & 0x70) >> 4) + 8 * m_palette_bank;
		int flipx = 0;
		int flipy = (spriteram[offs + 1] & 0x80) ? 0 : 1;
		int sx = spriteram[offs + 2];
		int sy = spriteram[offs];

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(2 + bank)->transpen(bitmap,cliprect,
			code, color,
			flipx, flipy,
			sx, sy, 0);
	}
}

UINT32 kingofb_state::screen_update_ringking(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrolly(0, -(*m_scroll_y));
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	ringking_draw_sprites(bitmap, cliprect);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
