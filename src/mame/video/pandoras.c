// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
#include "emu.h"
#include "includes/pandoras.h"
#include "video/resnet.h"

/***********************************************************************

  Convert the color PROMs into a more useable format.

  Pandora's Palace has one 32x8 palette PROM and two 256x4 lookup table
  PROMs (one for characters, one for sprites).
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

PALETTE_INIT_MEMBER(pandoras_state, pandoras)
{
	const UINT8 *color_prom = memregion("proms")->base();
	static const int resistances_rg[3] = { 1000, 470, 220 };
	static const int resistances_b [2] = { 470, 220 };
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

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(pandoras_state::get_tile_info0)
{
	UINT8 attr = m_colorram[tile_index];
	SET_TILE_INFO_MEMBER(1,
			m_videoram[tile_index] + ((attr & 0x10) << 4),
			attr & 0x0f,
			TILE_FLIPYX((attr & 0xc0) >> 6));
	tileinfo.category = (attr & 0x20) >> 5;
}

/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

void pandoras_state::video_start()
{
	m_layer0 = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(pandoras_state::get_tile_info0),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	save_item(NAME(m_flipscreen));
}

/***************************************************************************

  Memory Handlers

***************************************************************************/

WRITE8_MEMBER(pandoras_state::pandoras_vram_w)
{
	m_layer0->mark_tile_dirty(offset);
	m_videoram[offset] = data;
}

WRITE8_MEMBER(pandoras_state::pandoras_cram_w)
{
	m_layer0->mark_tile_dirty(offset);
	m_colorram[offset] = data;
}

WRITE8_MEMBER(pandoras_state::pandoras_scrolly_w)
{
	m_layer0->set_scrolly(0, data);
}

WRITE8_MEMBER(pandoras_state::pandoras_flipscreen_w)
{
	m_flipscreen = data;
	machine().tilemap().set_flip_all(m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
}

/***************************************************************************

  Screen Refresh

***************************************************************************/

void pandoras_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, UINT8* sr )
{
	int offs;

	for (offs = 0; offs < 0x100; offs += 4)
	{
		int sx = sr[offs + 1];
		int sy = 240 - sr[offs];
		int color = sr[offs + 3] & 0x0f;
		int nflipx = sr[offs + 3] & 0x40;
		int nflipy = sr[offs + 3] & 0x80;

		m_gfxdecode->gfx(0)->transmask(bitmap,cliprect,
			sr[offs + 2],
			color,
			!nflipx,!nflipy,
			sx,sy,
			m_palette->transpen_mask(*m_gfxdecode->gfx(0), color, 0));
	}
}

UINT32 pandoras_state::screen_update_pandoras(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_layer0->draw(screen, bitmap, cliprect, 1 ,0);
	draw_sprites(bitmap, cliprect, &m_spriteram[0x800] );
	m_layer0->draw(screen, bitmap, cliprect, 0 ,0);
	return 0;
}
