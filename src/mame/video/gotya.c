// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
#include "emu.h"
#include "video/resnet.h"
#include "includes/gotya.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

***************************************************************************/

PALETTE_INIT_MEMBER(gotya_state, gotya)
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
	color_prom += 32;

	for (i = 0; i < 0x40; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x07;
		palette.set_pen_indirect(i, ctabentry);
	}
}

WRITE8_MEMBER(gotya_state::gotya_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(gotya_state::gotya_colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(gotya_state::gotya_video_control_w)
{
	/* bit 0 - scroll bit 8
	   bit 1 - flip screen
	   bit 2 - sound disable ??? */

	m_scroll_bit_8 = data & 0x01;

	if (flip_screen() != (data & 0x02))
	{
		flip_screen_set(data & 0x02);
		machine().tilemap().mark_all_dirty();
	}
}

TILE_GET_INFO_MEMBER(gotya_state::get_bg_tile_info)
{
	int code = m_videoram[tile_index];
	int color = m_colorram[tile_index] & 0x0f;

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

TILEMAP_MAPPER_MEMBER(gotya_state::tilemap_scan_rows_thehand)
{
	/* logical (col,row) -> memory offset */
	row = 31 - row;
	col = 63 - col;
	return ((row) * (num_cols >> 1)) + (col & 31) + ((col >> 5) * 0x400);
}

void gotya_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(gotya_state::get_bg_tile_info),this), tilemap_mapper_delegate(FUNC(gotya_state::tilemap_scan_rows_thehand),this), 8, 8, 64, 32);
}

void gotya_state::draw_status_row( bitmap_ind16 &bitmap, const rectangle &cliprect, int sx, int col )
{
	int row;

	if (flip_screen())
	{
		sx = 35 - sx;
	}

	for (row = 29; row >= 0; row--)
	{
		int sy;

		if (flip_screen())
			sy = row;
		else
			sy = 31 - row;


			m_gfxdecode->gfx(0)->opaque(bitmap,cliprect,
			m_videoram2[row * 32 + col],
			m_videoram2[row * 32 + col + 0x10] & 0x0f,
			flip_screen_x(), flip_screen_y(),
			8 * sx, 8 * sy);
	}
}

void gotya_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	UINT8 *spriteram = m_spriteram;
	int offs;

	for (offs = 2; offs < 0x0e; offs += 2)
	{
		int code = spriteram[offs + 0x01] >> 2;
		int color = spriteram[offs + 0x11] & 0x0f;
		int sx = 256 - spriteram[offs + 0x10] + (spriteram[offs + 0x01] & 0x01) * 256;
		int sy = spriteram[offs + 0x00];

		if (flip_screen())
			sy = 240 - sy;


			m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
			code, color,
			flip_screen_x(), flip_screen_y(),
			sx, sy, 0);
	}
}

void gotya_state::draw_status( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	draw_status_row(bitmap, cliprect, 0,  1);
	draw_status_row(bitmap, cliprect, 1,  0);
	draw_status_row(bitmap, cliprect, 2,  2);  /* these two are blank, but I dont' know if the data comes */
	draw_status_row(bitmap, cliprect, 33, 13); /* from RAM or 'hardcoded' into the hardware. Likely the latter */
	draw_status_row(bitmap, cliprect, 35, 14);
	draw_status_row(bitmap, cliprect, 34, 15);
}

UINT32 gotya_state::screen_update_gotya(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0, -(*m_scroll + (m_scroll_bit_8 * 256)) - 2 * 8);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	draw_status(bitmap, cliprect);
	return 0;
}
