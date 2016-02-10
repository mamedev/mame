// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, David Haywood
/***************************************************************************

    Goal '92 video hardware

***************************************************************************/

#include "emu.h"
#include "includes/goal92.h"

READ16_MEMBER(goal92_state::goal92_fg_bank_r)
{
	return m_fg_bank;
}

WRITE16_MEMBER(goal92_state::goal92_fg_bank_w)
{
	COMBINE_DATA(&m_fg_bank);

	if (ACCESSING_BITS_0_7)
	{
		m_fg_layer->mark_all_dirty();
	}
}

WRITE16_MEMBER(goal92_state::goal92_text_w)
{
	COMBINE_DATA(&m_tx_data[offset]);
	m_tx_layer->mark_tile_dirty(offset);
}

WRITE16_MEMBER(goal92_state::goal92_background_w)
{
	COMBINE_DATA(&m_bg_data[offset]);
	m_bg_layer->mark_tile_dirty(offset);
}

WRITE16_MEMBER(goal92_state::goal92_foreground_w)
{
	COMBINE_DATA(&m_fg_data[offset]);
	m_fg_layer->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(goal92_state::get_text_tile_info)
{
	int tile = m_tx_data[tile_index];
	int color = (tile >> 12) & 0xf;

	tile &= 0xfff;

	tile |= 0xc000;

	SET_TILE_INFO_MEMBER(1, tile, color, 0);
}

TILE_GET_INFO_MEMBER(goal92_state::get_back_tile_info)
{
	int tile = m_bg_data[tile_index];
	int color = (tile >> 12) & 0xf;

	tile &= 0xfff;

	SET_TILE_INFO_MEMBER(2, tile, color, 0);
}

TILE_GET_INFO_MEMBER(goal92_state::get_fore_tile_info)
{
	int tile = m_fg_data[tile_index];
	int color = (tile >> 12) & 0xf;
	int region;

	tile &= 0xfff;

	if(m_fg_bank & 0xff)
	{
		region = 3;
		tile |= 0x1000;
	}
	else
	{
		region = 4;
		tile |= 0x2000;
	}

	SET_TILE_INFO_MEMBER(region, tile, color, 0);
}

void goal92_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int pri )
{
	UINT16 *buffered_spriteram16 = m_buffered_spriteram.get();
	int offs, fx, fy, x, y, color, sprite;

	for (offs = 3; offs <= 0x400 - 5; offs += 4)
	{
		UINT16 data = buffered_spriteram16[offs + 2];

		y = buffered_spriteram16[offs + 0];

		if (y & 0x8000)
			break;

		if (!(data & 0x8000))
			continue;

		sprite = buffered_spriteram16[offs + 1];

		if ((sprite >> 14) != pri)
			continue;

		x = buffered_spriteram16[offs + 3];

		sprite &= 0x1fff;

		x &= 0x1ff;
		y &= 0x1ff;

		color = (data & 0x3f) + 0x40;
		fx = (data & 0x4000) >> 14;
		fy = 0;

		x -= 320 / 4 - 16 - 1;

		y = 256 - (y + 7);

		m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
				sprite,
				color,fx,fy,x,y,15);
	}
}


void goal92_state::video_start()
{
	m_bg_layer = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(goal92_state::get_back_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_fg_layer = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(goal92_state::get_fore_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_tx_layer = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(goal92_state::get_text_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_buffered_spriteram = std::make_unique<UINT16[]>(0x400 * 2);
	save_pointer(NAME(m_buffered_spriteram.get()), 0x400 * 2);

	m_bg_layer->set_transparent_pen(15);
	m_fg_layer->set_transparent_pen(15);
	m_tx_layer->set_transparent_pen(15);
}

UINT32 goal92_state::screen_update_goal92(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_layer->set_scrollx(0, m_scrollram[0] + 60);
	m_bg_layer->set_scrolly(0, m_scrollram[1] + 8);

	if (m_fg_bank & 0xff)
	{
		m_fg_layer->set_scrollx(0, m_scrollram[0] + 60);
		m_fg_layer->set_scrolly(0, m_scrollram[1] + 8);
	}
	else
	{
		m_fg_layer->set_scrollx(0, m_scrollram[2] + 60);
		m_fg_layer->set_scrolly(0, m_scrollram[3] + 8);
	}

	bitmap.fill(m_palette->black_pen(), cliprect);

	m_bg_layer->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect, 2);

	if (!(m_fg_bank & 0xff))
		draw_sprites(bitmap, cliprect, 1);

	m_fg_layer->draw(screen, bitmap, cliprect, 0, 0);

	if(m_fg_bank & 0xff)
		draw_sprites(bitmap, cliprect, 1);

	draw_sprites(bitmap, cliprect, 0);
	draw_sprites(bitmap, cliprect, 3);
	m_tx_layer->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

void goal92_state::screen_eof_goal92(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		memcpy(m_buffered_spriteram.get(), m_spriteram, 0x400 * 2);
	}
}
