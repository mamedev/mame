// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Roberto Fresca
/***************************************************************************

Tehkan World Cup - (c) Tehkan 1985


Ernesto Corvi
ernesto@imagina.com

Roberto Juan Fresca
robbiex@rocketmail.com

***************************************************************************/

#include "emu.h"
#include "includes/tehkanwc.h"


void tehkanwc_state::videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void tehkanwc_state::colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_colorram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void tehkanwc_state::videoram2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_videoram2[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

void tehkanwc_state::scroll_x_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_scroll_x[offset] = data;
}

void tehkanwc_state::scroll_y_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_bg_tilemap->set_scrolly(0, data);
}

void tehkanwc_state::flipscreen_x_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	flip_screen_x_set(data & 0x40);
}

void tehkanwc_state::flipscreen_y_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	flip_screen_y_set(data & 0x40);
}

void tehkanwc_state::gridiron_led0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_led0 = data;
}
void tehkanwc_state::gridiron_led1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_led1 = data;
}

void tehkanwc_state::get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index)
{
	int offs = tile_index * 2;
	int attr = m_videoram2[offs + 1];
	int code = m_videoram2[offs] + ((attr & 0x30) << 4);
	int color = attr & 0x0f;
	int flags = ((attr & 0x40) ? TILE_FLIPX : 0) | ((attr & 0x80) ? TILE_FLIPY : 0);

	SET_TILE_INFO_MEMBER(2, code, color, flags);
}

void tehkanwc_state::get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index)
{
	int attr = m_colorram[tile_index];
	int code = m_videoram[tile_index] + ((attr & 0x10) << 4);
	int color = attr & 0x0f;
	int flags = ((attr & 0x40) ? TILE_FLIPX : 0) | ((attr & 0x80) ? TILE_FLIPY : 0);

	tileinfo.category = (attr & 0x20) ? 0 : 1;

	SET_TILE_INFO_MEMBER(0, code, color, flags);
}

void tehkanwc_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(
			*m_gfxdecode, tilemap_get_info_delegate(FUNC(tehkanwc_state::get_bg_tile_info),this),
			TILEMAP_SCAN_ROWS, 16, 8, 32, 32);

	m_fg_tilemap = &machine().tilemap().create(
			*m_gfxdecode, tilemap_get_info_delegate(FUNC(tehkanwc_state::get_fg_tile_info),this),
			TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_fg_tilemap->set_transparent_pen(0);

	save_item(NAME(m_scroll_x));
	save_item(NAME(m_led0));
	save_item(NAME(m_led1));
}

/*
   Gridiron Fight has a LED display on the control panel, to let each player
   choose the formation without letting the other know.

    ---0---
   |       |
   5       1
   |       |
    ---6---
   |       |
   4       2
   |       |
    ---3---

   bit 7 = enable (0 = display off)
 */

void tehkanwc_state::gridiron_draw_led(bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t led,int player)
{
	if (led&0x80)
		output().set_digit_value(player, led&0x7f);
		else
		output().set_digit_value(player, 0x00);
}

void tehkanwc_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = 0;offs < m_spriteram.bytes();offs += 4)
	{
		int attr = m_spriteram[offs + 1];
		int code = m_spriteram[offs] + ((attr & 0x08) << 5);
		int color = attr & 0x07;
		int flipx = attr & 0x40;
		int flipy = attr & 0x80;
		int sx = m_spriteram[offs + 2] + ((attr & 0x20) << 3) - 128;
		int sy = m_spriteram[offs + 3];

		if (flip_screen_x())
		{
			sx = 240 - sx;
			flipx = !flipx;
		}

		if (flip_screen_y())
		{
			sy = 240 - sy;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
			code, color, flipx, flipy, sx, sy, 0);
	}
}

uint32_t tehkanwc_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0, m_scroll_x[0] + 256 * m_scroll_x[1]);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 1, 0);
	gridiron_draw_led(bitmap, cliprect, m_led0, 0);
	gridiron_draw_led(bitmap, cliprect, m_led1, 1);
	return 0;
}
