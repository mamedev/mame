// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

  bagman.cpp

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "bagman.h"


void bagman_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void bagman_state::colorram_w(offs_t offset, uint8_t data)
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
void bagman_state::bagman_palette(palette_device &palette) const
{
	static constexpr int resistances_rg[3] = { 1000, 470, 220 };
	static constexpr int resistances_b [2] = { 470, 220 };

	double weights_r[3], weights_g[3], weights_b[2];
	compute_resistor_weights(0, 255,    -1.0,
			3,  resistances_rg, weights_r,  470,    0,
			3,  resistances_rg, weights_g,  470,    0,
			2,  resistances_b,  weights_b,  470,    0);

	uint8_t const *const color_prom = memregion("proms")->base();
	for (int i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int const r = combine_weights(weights_r, bit0, bit1, bit2);

		// green component
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = combine_weights(weights_g, bit0, bit1, bit2);

		// blue component
		bit0 = BIT(color_prom[i], 6);
		bit1 = BIT(color_prom[i], 7);
		int const b = combine_weights(weights_b, bit0, bit1);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

WRITE_LINE_MEMBER(bagman_state::flipscreen_x_w)
{
	flip_screen_x_set(state);
}

WRITE_LINE_MEMBER(bagman_state::flipscreen_y_w)
{
	flip_screen_y_set(state);
}

WRITE_LINE_MEMBER(bagman_state::video_enable_w)
{
	m_video_enable = state;
}

TILE_GET_INFO_MEMBER(bagman_state::get_bg_tile_info)
{
	int gfxbank = (m_gfxdecode->gfx(2) && (m_colorram[tile_index] & 0x10)) ? 2 : 0;
	int code = m_videoram[tile_index] + 8 * (m_colorram[tile_index] & 0x20);
	int color = m_colorram[tile_index] & 0x0f;

	tileinfo.set(gfxbank, code, color, 0);
}

void bagman_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(bagman_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS,
			8, 8, 32, 32);
}


void bagman_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// Spriteram is at the start of the colorram
	for (int offs = 0x20 - 4;offs >= 0;offs -= 4)
	{
		int sx,sy,flipx,flipy;

		sx = m_colorram[offs + 3];
		sy = 256 - m_colorram[offs + 2] - 16;
		flipx = m_colorram[offs] & 0x40;
		flipy = m_colorram[offs] & 0x80;
		if (flip_screen())
		{
			sx = 256 - sx - 15;
			sy = 255 - sy - 15;
			flipx = !flipx;
			flipy = !flipy;
		}

		if (m_colorram[offs + 2] && m_colorram[offs + 3])
			m_gfxdecode->gfx(1)->transpen(bitmap,
					cliprect,
					(m_colorram[offs] & 0x3f) + 2 * (m_colorram[offs + 1] & 0x20),
					m_colorram[offs + 1] & 0x1f,
					flipx,flipy,
					sx,sy,0);
	}
}

uint32_t bagman_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	if (!m_video_enable)
		return 0;

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}
