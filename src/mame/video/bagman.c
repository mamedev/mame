// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

  bagman.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/bagman.h"


WRITE8_MEMBER(bagman_state::videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(bagman_state::colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Bagman has two 32 bytes palette PROMs, connected to the RGB output this
  way:

  bit 7 -- 220 ohm resistor  -- \
        -- 470 ohm resistor  -- | -- 470 ohm pulldown resistor -- BLUE

        -- 220 ohm resistor  -- \
        -- 470 ohm resistor  -- | -- 470 ohm pulldown resistor -- GREEN
        -- 1  kohm resistor  -- /

        -- 220 ohm resistor  -- \
        -- 470 ohm resistor  -- | -- 470 ohm pulldown resistor -- RED
  bit 0 -- 1  kohm resistor  -- /

***************************************************************************/
PALETTE_INIT_MEMBER(bagman_state,bagman)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;
	static const int resistances_rg[3] = { 1000, 470, 220 };
	static const int resistances_b [2] = { 470, 220 };
	double weights_r[3], weights_g[3], weights_b[2];


	compute_resistor_weights(0, 255,    -1.0,
			3,  resistances_rg, weights_r,  470,    0,
			3,  resistances_rg, weights_g,  470,    0,
			2,  resistances_b,  weights_b,  470,    0);


	for (i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = combine_3_weights(weights_r, bit0, bit1, bit2);
		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = combine_3_weights(weights_g, bit0, bit1, bit2);
		/* blue component */
		bit0 = (color_prom[i] >> 6) & 0x01;
		bit1 = (color_prom[i] >> 7) & 0x01;
		b = combine_2_weights(weights_b, bit0, bit1);

		palette.set_pen_color(i,rgb_t(r,g,b));
	}
}

WRITE8_MEMBER(bagman_state::flipscreen_w)
{
	flip_screen_set(data & 0x01);
}

TILE_GET_INFO_MEMBER(bagman_state::get_bg_tile_info)
{
	int gfxbank = (m_gfxdecode->gfx(2) && (m_colorram[tile_index] & 0x10)) ? 2 : 0;
	int code = m_videoram[tile_index] + 8 * (m_colorram[tile_index] & 0x20);
	int color = m_colorram[tile_index] & 0x0f;

	SET_TILE_INFO_MEMBER(gfxbank, code, color, 0);
}

void bagman_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(bagman_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS,
			8, 8, 32, 32);
}


void bagman_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = m_spriteram.bytes() - 4;offs >= 0;offs -= 4)
	{
		int sx,sy,flipx,flipy;

		sx = m_spriteram[offs + 3];
		sy = 256 - m_spriteram[offs + 2] - 16;
		flipx = m_spriteram[offs] & 0x40;
		flipy = m_spriteram[offs] & 0x80;
		if (flip_screen())
		{
			sx = 256 - sx - 15;
			sy = 255 - sy - 15;
			flipx = !flipx;
			flipy = !flipy;
		}

		if (m_spriteram[offs + 2] && m_spriteram[offs + 3])
			m_gfxdecode->gfx(1)->transpen(bitmap,
					cliprect,
					(m_spriteram[offs] & 0x3f) + 2 * (m_spriteram[offs + 1] & 0x20),
					m_spriteram[offs + 1] & 0x1f,
					flipx,flipy,
					sx,sy,0);
	}
}

UINT32 bagman_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	if (*m_video_enable == 0)
		return 0;

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}
