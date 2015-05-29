// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria,Chris Hardy
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/circusc.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Circus Charlie has one 32x8 palette PROM and two 256x4 lookup table PROMs
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

PALETTE_INIT_MEMBER(circusc_state, circusc)
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

	/* characters map to the upper 16 palette entries */
	for (i = 0; i < 0x100; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		palette.set_pen_indirect(i, ctabentry + 0x10);
	}

	/* sprites map to the lower 16 palette entries */
	for (i = 0x100; i < 0x200; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		palette.set_pen_indirect(i, ctabentry);
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(circusc_state::get_tile_info)
{
	UINT8 attr = m_colorram[tile_index];
	tileinfo.category = (attr & 0x10) >> 4;

	SET_TILE_INFO_MEMBER(0,
					m_videoram[tile_index] + ((attr & 0x20) << 3),
					attr & 0x0f,
					TILE_FLIPYX((attr & 0xc0) >> 6));
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void circusc_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(circusc_state::get_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_bg_tilemap->set_scroll_cols(32);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(circusc_state::circusc_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(circusc_state::circusc_colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(circusc_state::circusc_flipscreen_w)
{
	flip_screen_set(data & 1);
}



/***************************************************************************

  Display refresh

***************************************************************************/

void circusc_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int offs;
	UINT8 *sr;

	if ((*m_spritebank & 0x01) != 0)
		sr = m_spriteram;
	else
		sr = m_spriteram_2;

	for (offs = 0; offs < m_spriteram.bytes(); offs += 4)
	{
		int code = sr[offs + 0] + 8 * (sr[offs + 1] & 0x20);
		int color = sr[offs + 1] & 0x0f;
		int sx = sr[offs + 2];
		int sy = sr[offs + 3];
		int flipx = sr[offs + 1] & 0x40;
		int flipy = sr[offs + 1] & 0x80;

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}


		m_gfxdecode->gfx(1)->transmask(bitmap,cliprect,
				code, color,
				flipx,flipy,
				sx,sy,
				m_palette->transpen_mask(*m_gfxdecode->gfx(1), color, 0));
	}
}

UINT32 circusc_state::screen_update_circusc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i;

	for (i = 0; i < 10; i++)
		m_bg_tilemap->set_scrolly(i, 0);
	for (i = 10; i < 32; i++)
		m_bg_tilemap->set_scrolly(i, *m_scroll);

	bitmap.fill(0, cliprect);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 1, 0);
	draw_sprites(bitmap, cliprect);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
