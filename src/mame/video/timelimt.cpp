// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
#include "emu.h"
#include "includes/timelimt.h"
#include "video/resnet.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Time Limit has three 32 bytes palette PROM, connected to the RGB output this
  way:

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

***************************************************************************/

void timelimt_state::timelimt_palette(palette_device &palette) const
{
	static constexpr int resistances_rg[3] = { 1000, 470, 220 };
	static constexpr int resistances_b [2] = { 470, 220 };

	uint8_t const *const color_prom = memregion("proms")->base();

	double weights_r[3], weights_g[3], weights_b[2];
	compute_resistor_weights(0, 255,    -1.0,
			3,  resistances_rg, weights_r,  0,  0,
			3,  resistances_rg, weights_g,  0,  0,
			2,  resistances_b,  weights_b,  0,  0);

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

/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

TILE_GET_INFO_MEMBER(timelimt_state::get_bg_tile_info)
{
	tileinfo.set(1, m_bg_videoram[tile_index], 0, 0);
}

TILE_GET_INFO_MEMBER(timelimt_state::get_fg_tile_info)
{
	tileinfo.set(0, m_videoram[tile_index], 0, 0);
}

void timelimt_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(timelimt_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS,
			8, 8, 64, 32);

	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(timelimt_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS,
			8, 8, 32, 32);

	m_fg_tilemap->set_transparent_pen(0);

	m_scrollx = 0;
	m_scrolly = 0;

	save_item(NAME(m_scrollx));
	save_item(NAME(m_scrolly));
}

/***************************************************************************/

void timelimt_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void timelimt_state::bg_videoram_w(offs_t offset, uint8_t data)
{
	m_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void timelimt_state::scroll_x_lsb_w(uint8_t data)
{
	m_scrollx &= 0x100;
	m_scrollx |= data & 0xff;
}

void timelimt_state::scroll_x_msb_w(uint8_t data)
{
	m_scrollx &= 0xff;
	m_scrollx |= ( data & 1 ) << 8;
}

void timelimt_state::scroll_y_w(uint8_t data)
{
	m_scrolly = data;
}


void timelimt_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for( int offs = m_spriteram.bytes() - 4; offs >= 0; offs -= 4 )
	{
		int sy = 240 - m_spriteram[offs];
		int sx = m_spriteram[offs+3];
		int code = m_spriteram[offs+1] & 0x3f;
		int attr = m_spriteram[offs+2];
		int flipy = m_spriteram[offs+1] & 0x80;
		int flipx = m_spriteram[offs+1] & 0x40;

		code += ( attr & 0x80 ) ? 0x40 : 0x00;
		code += ( attr & 0x40 ) ? 0x80 : 0x00;

		m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
				code,
				attr & 3, // was & 7, wrong for 3bpp and 32 colors
				flipx,flipy,
				sx,sy,0);
	}
}


uint32_t timelimt_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0, m_scrollx);
	m_bg_tilemap->set_scrolly(0, m_scrolly);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	draw_sprites(bitmap, cliprect);

	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
