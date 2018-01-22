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

PALETTE_INIT_MEMBER(timelimt_state, timelimt)
{
	const uint8_t *color_prom = memregion("proms")->base();
	int i;
	static const int resistances_rg[3] = { 1000, 470, 220 };
	static const int resistances_b [2] = { 470, 220 };
	double weights_r[3], weights_g[3], weights_b[2];

	compute_resistor_weights(0, 255,    -1.0,
			3,  resistances_rg, weights_r,  0,  0,
			3,  resistances_rg, weights_g,  0,  0,
			2,  resistances_b,  weights_b,  0,  0);


	for (i = 0;i < palette.entries();i++)
	{
		int bit0,bit1,bit2,r,g,b;

		/* red component */
		bit0 = (*color_prom >> 0) & 0x01;
		bit1 = (*color_prom >> 1) & 0x01;
		bit2 = (*color_prom >> 2) & 0x01;
		r = combine_3_weights(weights_r, bit0, bit1, bit2);
		/* green component */
		bit0 = (*color_prom >> 3) & 0x01;
		bit1 = (*color_prom >> 4) & 0x01;
		bit2 = (*color_prom >> 5) & 0x01;
		g = combine_3_weights(weights_g, bit0, bit1, bit2);
		/* blue component */
		bit0 = (*color_prom >> 6) & 0x01;
		bit1 = (*color_prom >> 7) & 0x01;
		b = combine_2_weights(weights_b, bit0, bit1);

		palette.set_pen_color(i,rgb_t(r,g,b));
		color_prom++;
	}
}

/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

TILE_GET_INFO_MEMBER(timelimt_state::get_bg_tile_info)
{
	SET_TILE_INFO_MEMBER(1, m_bg_videoram[tile_index], 0, 0);
}

TILE_GET_INFO_MEMBER(timelimt_state::get_fg_tile_info)
{
	SET_TILE_INFO_MEMBER(0, m_videoram[tile_index], 0, 0);
}

void timelimt_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(timelimt_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS,
			8, 8, 64, 32);

	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(timelimt_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS,
			8, 8, 32, 32);

	m_fg_tilemap->set_transparent_pen(0);

	save_item(NAME(m_scrollx));
	save_item(NAME(m_scrolly));
}

/***************************************************************************/

WRITE8_MEMBER(timelimt_state::videoram_w)
{
	m_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(timelimt_state::bg_videoram_w)
{
	m_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(timelimt_state::scroll_x_lsb_w)
{
	m_scrollx &= 0x100;
	m_scrollx |= data & 0xff;
}

WRITE8_MEMBER(timelimt_state::scroll_x_msb_w)
{
	m_scrollx &= 0xff;
	m_scrollx |= ( data & 1 ) << 8;
}

WRITE8_MEMBER(timelimt_state::scroll_y_w)
{
	m_scrolly = data;
}


void timelimt_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for( int offs = m_spriteram.bytes(); offs >= 0; offs -= 4 )
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
