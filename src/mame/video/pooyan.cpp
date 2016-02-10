// license:BSD-3-Clause
// copyright-holders:Allard van der Bas
/***************************************************************************

    Pooyan

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/pooyan.h"

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Pooyan has one 32x8 palette PROM and two 256x4 lookup table PROMs
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

PALETTE_INIT_MEMBER(pooyan_state, pooyan)
{
	const UINT8 *color_prom = memregion("proms")->base();
	static const int resistances_rg[3] = { 1000, 470, 220 };
	static const int resistances_b [2] = { 470, 220 };
	double rweights[3], gweights[3], bweights[2];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0, 255, -1.0,
			3, resistances_rg, rweights, 1000, 0,
			3, resistances_rg, gweights, 1000, 0,
			2, resistances_b,  bweights, 1000, 0);

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

	/* characters */
	for (i = 0; i < 0x100; i++)
	{
		UINT8 ctabentry = (color_prom[i] & 0x0f) | 0x10;
		palette.set_pen_indirect(i, ctabentry);
	}

	/* sprites */
	for (i = 0x100; i < 0x200; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		palette.set_pen_indirect(i, ctabentry);
	}
}



/*************************************
 *
 *  Tilemap info callback
 *
 *************************************/

TILE_GET_INFO_MEMBER(pooyan_state::get_bg_tile_info)
{
	int attr = m_colorram[tile_index];
	int code = m_videoram[tile_index];
	int color = attr & 0x0f;
	int flags = TILE_FLIPYX(attr >> 6);

	SET_TILE_INFO_MEMBER(0, code, color, flags);
}



/*************************************
 *
 *  Video startup
 *
 *************************************/

void pooyan_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(pooyan_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}



/*************************************
 *
 *  Memory write handlers
 *
 *************************************/

WRITE8_MEMBER(pooyan_state::videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


WRITE8_MEMBER(pooyan_state::colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


WRITE8_MEMBER(pooyan_state::flipscreen_w)
{
	flip_screen_set(~data & 0x01);
}



/*************************************
 *
 *  Sprite rendering
 *
 *************************************/

void pooyan_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	for (int offs = 0x10; offs < 0x40; offs += 2)
	{
		int sx = m_spriteram[offs];
		int sy = 240 - m_spriteram2[offs + 1];

		int code = m_spriteram[offs + 1];
		int color = m_spriteram2[offs] & 0x0f;
		int flipx = ~m_spriteram2[offs] & 0x40;
		int flipy = m_spriteram2[offs] & 0x80;


			m_gfxdecode->gfx(1)->transmask(bitmap,cliprect,
			code,
			color,
			flipx, flipy,
			sx, sy,
			m_palette->transpen_mask(*m_gfxdecode->gfx(1), color, 0));
	}
}



/*************************************
 *
 *  Video update
 *
 *************************************/

UINT32 pooyan_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}
