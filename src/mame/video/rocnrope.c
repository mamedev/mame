// license:BSD-3-Clause
// copyright-holders:Chris Hardy
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/rocnrope.h"

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Roc'n Rope has one 32x8 palette PROM and two 256x4 lookup table PROMs
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

PALETTE_INIT_MEMBER(rocnrope_state, rocnrope)
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

	/* sprites and characters */
	for (i = 0; i < 0x200; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		palette.set_pen_indirect(i, ctabentry);
	}
}

WRITE8_MEMBER(rocnrope_state::rocnrope_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(rocnrope_state::rocnrope_colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(rocnrope_state::rocnrope_flipscreen_w)
{
	if (flip_screen() != (~data & 0x01))
	{
		flip_screen_set(~data & 0x01);
		machine().tilemap().mark_all_dirty();
	}
}

TILE_GET_INFO_MEMBER(rocnrope_state::get_bg_tile_info)
{
	int attr = m_colorram[tile_index];
	int code = m_videoram[tile_index] + 2 * (attr & 0x80);
	int color = attr & 0x0f;
	int flags = ((attr & 0x40) ? TILE_FLIPX : 0) | ((attr & 0x20) ? TILE_FLIPY : 0);

	SET_TILE_INFO_MEMBER(1, code, color, flags);
}

void rocnrope_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(rocnrope_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

void rocnrope_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	UINT8 *spriteram = m_spriteram;
	UINT8 *spriteram_2 = m_spriteram2;
	int offs;

	for (offs = m_spriteram.bytes() - 2;offs >= 0;offs -= 2)
	{
		int color = spriteram_2[offs] & 0x0f;

		m_gfxdecode->gfx(0)->transmask(bitmap,cliprect,
				spriteram[offs + 1],
				color,
				spriteram_2[offs] & 0x40,~spriteram_2[offs] & 0x80,
				240 - spriteram[offs], spriteram_2[offs + 1],
				m_palette->transpen_mask(*m_gfxdecode->gfx(0), color, 0));
	}
}

UINT32 rocnrope_state::screen_update_rocnrope(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}
