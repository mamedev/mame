// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
#include "emu.h"
#include "includes/timelimt.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Time Limit has two 32 bytes palette PROM, connected to the RGB output this
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

void timelimt_state::palette_init_timelimt(palette_device &palette){
	const uint8_t *color_prom = memregion("proms")->base();
	int i;

	for (i = 0;i < palette.entries();i++)
	{
		int bit0,bit1,bit2,r,g,b;

		/* red component */
		bit0 = (*color_prom >> 0) & 0x01;
		bit1 = (*color_prom >> 1) & 0x01;
		bit2 = (*color_prom >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (*color_prom >> 3) & 0x01;
		bit1 = (*color_prom >> 4) & 0x01;
		bit2 = (*color_prom >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = (*color_prom >> 6) & 0x01;
		bit1 = (*color_prom >> 7) & 0x01;
		b = 0x4f * bit0 + 0xa8 * bit1;

		palette.set_pen_color(i,rgb_t(r,g,b));
		color_prom++;
	}
}

/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

void timelimt_state::get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index)
{
	SET_TILE_INFO_MEMBER(1, m_bg_videoram[tile_index], 0, 0);
}

void timelimt_state::get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index)
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

void timelimt_state::videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void timelimt_state::bg_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void timelimt_state::scroll_x_lsb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_scrollx &= 0x100;
	m_scrollx |= data & 0xff;
}

void timelimt_state::scroll_x_msb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_scrollx &= 0xff;
	m_scrollx |= ( data & 1 ) << 8;
}

void timelimt_state::scroll_y_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
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
				attr & 7,
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
