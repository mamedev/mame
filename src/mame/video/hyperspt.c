// license:BSD-3-Clause
// copyright-holders:Chris Hardy
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/hyperspt.h"

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Hyper Sports has one 32x8 palette PROM and two 256x4 lookup table PROMs
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

PALETTE_INIT_MEMBER(hyperspt_state, hyperspt)
{
	const UINT8 *color_prom = memregion("proms")->base();
	static const int resistances_rg[3] = { 1000, 470, 220 };
	static const int resistances_b[2] = { 470, 220 };
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

WRITE8_MEMBER(hyperspt_state::hyperspt_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(hyperspt_state::hyperspt_colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(hyperspt_state::hyperspt_flipscreen_w)
{
	if (flip_screen() != (data & 0x01))
	{
		flip_screen_set(data & 0x01);
		machine().tilemap().mark_all_dirty();
	}
}

TILE_GET_INFO_MEMBER(hyperspt_state::get_bg_tile_info)
{
	int code = m_videoram[tile_index] + ((m_colorram[tile_index] & 0x80) << 1) + ((m_colorram[tile_index] & 0x40) << 3);
	int color = m_colorram[tile_index] & 0x0f;
	int flags = ((m_colorram[tile_index] & 0x10) ? TILE_FLIPX : 0) | ((m_colorram[tile_index] & 0x20) ? TILE_FLIPY : 0);

	SET_TILE_INFO_MEMBER(1, code, color, flags);
}

void hyperspt_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(hyperspt_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_bg_tilemap->set_scroll_rows(32);
}

void hyperspt_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	UINT8 *spriteram = m_spriteram;
	int offs;

	for (offs = m_spriteram.bytes() - 4;offs >= 0;offs -= 4)
	{
		int sx = spriteram[offs + 3];
		int sy = 240 - spriteram[offs + 1];
		int code = spriteram[offs + 2] + 8 * (spriteram[offs] & 0x20);
		int color = spriteram[offs] & 0x0f;
		int flipx = ~spriteram[offs] & 0x40;
		int flipy = spriteram[offs] & 0x80;

		if (flip_screen())
		{
			sy = 240 - sy;
			flipy = !flipy;
		}

		/* Note that this adjustment must be done AFTER handling flip_screen(), thus */
		/* proving that this is a hardware related "feature" */

		sy += 1;


			m_gfxdecode->gfx(0)->transmask(bitmap,cliprect,
			code, color,
			flipx, flipy,
			sx, sy,
			m_palette->transpen_mask(*m_gfxdecode->gfx(0), color, 0));

		/* redraw with wraparound */


			m_gfxdecode->gfx(0)->transmask(bitmap,cliprect,
			code, color,
			flipx, flipy,
			sx - 256, sy,
			m_palette->transpen_mask(*m_gfxdecode->gfx(0), color, 0));
	}
}

UINT32 hyperspt_state::screen_update_hyperspt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int row;

	for (row = 0; row < 32; row++)
	{
		int scrollx = m_scroll[row * 2] + (m_scroll[(row * 2) + 1] & 0x01) * 256;
		if (flip_screen()) scrollx = -scrollx;
		m_bg_tilemap->set_scrollx(row, scrollx);
	}

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}

/* Road Fighter */
TILE_GET_INFO_MEMBER(hyperspt_state::roadf_get_bg_tile_info)
{
	int code = m_videoram[tile_index] + ((m_colorram[tile_index] & 0x80) << 1) + ((m_colorram[tile_index] & 0x60) << 4);
	int color = m_colorram[tile_index] & 0x0f;
	int flags = (m_colorram[tile_index] & 0x10) ? TILE_FLIPX : 0;

	SET_TILE_INFO_MEMBER(1, code, color, flags);
}

VIDEO_START_MEMBER(hyperspt_state,roadf)
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(hyperspt_state::roadf_get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_bg_tilemap->set_scroll_rows(32);
}
