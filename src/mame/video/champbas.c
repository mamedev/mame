// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Jarek Parchanski, Nicola Salmoria, hap
/*************************************************************************

    Talbot - Champion Base Ball - Exciting Soccer

*************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/champbas.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

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

PALETTE_INIT_MEMBER(champbas_state,champbas)
{
	const UINT8 *color_prom = memregion("proms")->base();
	static const int resistances_rg[3] = { 1000, 470, 220 };
	static const int resistances_b [2] = { 470, 220 };
	double rweights[3], gweights[3], bweights[2];

	/* compute the color output resistor weights */
	compute_resistor_weights(0, 255, -1.0,
			3, &resistances_rg[0], rweights, 0, 0,
			3, &resistances_rg[0], gweights, 0, 0,
			2, &resistances_b[0],  bweights, 0, 0);

	/* create a lookup table for the palette */
	for (int i = 0; i < 0x20; i++)
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

	color_prom += 0x20;

	for (int i = 0; i < 0x200; i++)
	{
		UINT8 ctabentry = (color_prom[i & 0xff] & 0x0f) | ((i & 0x100) >> 4);
		palette.set_pen_indirect(i, ctabentry);
	}
}


PALETTE_INIT_MEMBER(champbas_state,exctsccr)
{
	const UINT8 *color_prom = memregion("proms")->base();

	/* create a lookup table for the palette */
	for (int i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = 0;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x20;

	/* characters / sprites (3bpp) */
	for (int i = 0; i < 0x100; i++)
	{
		int swapped_i = BITSWAP8(i, 2, 7, 6, 5, 4, 3, 1, 0);
		UINT8 ctabentry = (color_prom[swapped_i] & 0x0f) | ((i & 0x80) >> 3);
		palette.set_pen_indirect(i, ctabentry);
	}

	/* sprites (4bpp) */
	for (int i = 0; i < 0x100; i++)
	{
		UINT8 ctabentry = (color_prom[0x100 + i] & 0x0f) | 0x10;
		palette.set_pen_indirect(i + 0x100, ctabentry);
	}
}



/*************************************
 *
 *  Callbacks for the TileMap code
 *
 *************************************/

TILE_GET_INFO_MEMBER(champbas_state::champbas_get_bg_tile_info)
{
	int code = m_vram[tile_index] | (m_gfx_bank << 8);
	int color = (m_vram[tile_index + 0x400] & 0x1f) | 0x20;

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

TILE_GET_INFO_MEMBER(champbas_state::exctsccr_get_bg_tile_info)
{
	int code = m_vram[tile_index] | (m_gfx_bank << 8);
	int color = m_vram[tile_index + 0x400] & 0x0f;

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START_MEMBER(champbas_state,champbas)
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(champbas_state::champbas_get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

VIDEO_START_MEMBER(champbas_state,exctsccr)
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(champbas_state::exctsccr_get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

WRITE8_MEMBER(champbas_state::tilemap_w)
{
	m_vram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

WRITE8_MEMBER(champbas_state::gfxbank_w)
{
	data &= 1;

	if (m_gfx_bank != data)
	{
		m_gfx_bank = data;
		m_bg_tilemap->mark_all_dirty();
	}
}

WRITE8_MEMBER(champbas_state::palette_bank_w)
{
	m_palette_bank = data & 1;
	m_bg_tilemap->set_palette_offset(m_palette_bank << 8);
}

WRITE8_MEMBER(champbas_state::flipscreen_w)
{
	flip_screen_set(~data & 1);
}



/*************************************
 *
 *  Video update
 *
 *************************************/

void champbas_state::champbas_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	gfx_element* const gfx = m_gfxdecode->gfx(1);
	assert (m_spriteram.bytes() == 0x10);

	for (int offs = 0x10-2; offs >= 0; offs -= 2)
	{
		// spriteram holds x/y data
		int sx = m_spriteram[offs + 1] - 16;
		int sy = 255 - m_spriteram[offs];

		// attribute data is from last section of mainram
		UINT8 *attr = &(m_mainram[0x7f0]);
		int code = (attr[offs] >> 2 & 0x3f) | (m_gfx_bank << 6);
		int color = (attr[offs + 1] & 0x1f) | (m_palette_bank << 6);
		int flipx = ~attr[offs] & 1;
		int flipy = ~attr[offs] & 2;

		gfx->transmask(bitmap, cliprect,
			code, color,
			flipx, flipy,
			sx, sy,
			m_palette->transpen_mask(*gfx, color, 0));

		// wraparound
		gfx->transmask(bitmap, cliprect,
			code, color,
			flipx, flipy,
			sx + 256, sy,
			m_palette->transpen_mask(*gfx, color, 0));
	}
}

void champbas_state::exctsccr_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	gfx_element* const gfx1 = m_gfxdecode->gfx(1);
	gfx_element* const gfx2 = m_gfxdecode->gfx(2);
	assert (m_spriteram.bytes() == 0x10);

	for (int offs = 0x10-2; offs >= 0; offs -= 2)
	{
		// spriteram holds x/y data
		int sx = m_spriteram[offs + 1] - 16;
		int sy = 255 - m_spriteram[offs];

		// attribute data is from videoram
		int code = (m_vram[offs] >> 2 & 0x3f) | (m_vram[offs + 1] << 2 & 0x40);
		int flipx = ~m_vram[offs] & 1;
		int flipy = ~m_vram[offs] & 2;
		int color = m_vram[offs + 1] & 0xf;

		gfx1->transpen(bitmap, cliprect,
			code, color,
			flipx, flipy,
			sx, sy, 0);
	}

	for (int offs = 0x10-2; offs >= 0; offs -= 2)
	{
		// spriteram2 holds x/y data
		int sx = m_spriteram2[offs + 1] - 16;
		int sy = 255 - m_spriteram2[offs];

		// attribute data is from mainram
		int code = m_mainram[offs] >> 2 & 0x3f;
		int flipx = ~m_mainram[offs] & 1;
		int flipy = ~m_mainram[offs] & 2;
		int color = m_mainram[offs + 1] & 0xf;

		gfx2->transmask(bitmap, cliprect,
			code, color,
			flipx, flipy,
			sx, sy,
			m_palette->transpen_mask(*gfx2, color, 0x10));
	}
}


UINT32 champbas_state::screen_update_champbas(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	champbas_draw_sprites(bitmap, cliprect);
	return 0;
}

UINT32 champbas_state::screen_update_exctsccr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	exctsccr_draw_sprites(bitmap, cliprect);
	return 0;
}
