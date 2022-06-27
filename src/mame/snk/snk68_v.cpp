// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Acho A. Tang, Nicola Salmoria
/***************************************************************************

    SNK 68000 video routines

Notes:
    Search & Rescue uses Y flip on sprites only.
    Street Smart uses X flip on sprites only.

    Seems to be controlled in same byte as flipscreen.

    Emulation by Bryan McPhail, mish@tendril.co.uk

***************************************************************************/

#include "emu.h"
#include "snk68.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(snk68_state::get_tile_info)
{
	int tile = m_fg_tile_offset + (m_fg_videoram[2*tile_index] & 0xff);
	int color = m_fg_videoram[2*tile_index+1] & 0x07;

	tileinfo.set(0, tile, color, 0);
}

TILE_GET_INFO_MEMBER(searchar_state::get_tile_info)
{
	int data = m_fg_videoram[2*tile_index];
	int tile = data & 0x7ff;
	int color = (data & 0x7000) >> 12;

	// used in the ikari3 intro
	int flags = (data & 0x8000) ? TILE_FORCE_LAYER0 : 0;

	tileinfo.set(0, tile, color, flags);
}

/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void snk68_state::common_video_start()
{
	m_fg_tilemap->set_transparent_pen(0);
	save_item(NAME(m_sprite_flip_axis));
}

void snk68_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(snk68_state::get_tile_info)), TILEMAP_SCAN_COLS, 8, 8, 32, 32);
	m_fg_tile_offset = 0;

	common_video_start();

	save_item(NAME(m_fg_tile_offset));
}

void searchar_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(searchar_state::get_tile_info)), TILEMAP_SCAN_COLS, 8, 8, 32, 32);

	snk68_state::common_video_start();
}

/***************************************************************************

  Memory handlers

***************************************************************************/

uint16_t snk68_state::fg_videoram_r(offs_t offset)
{
	// RAM is only 8-bit
	return m_fg_videoram[offset] | 0xff00;
}

void snk68_state::fg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	data |= 0xff00;
	COMBINE_DATA(&m_fg_videoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset >> 1);
}

void searchar_state::fg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// RAM is full 16-bit, though only half of it is used by the hardware
	COMBINE_DATA(&m_fg_videoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset >> 1);
}

void snk68_state::flipscreen_w(uint8_t data)
{
	flip_screen_set(BIT(data, 3));
	m_sprites->set_flip(BIT(data, 3));
	m_sprite_flip_axis = BIT(data, 2);   // for streetsm? though might not be present on this board

	if (m_fg_tile_offset != ((data & 0x70) << 4))
	{
		m_fg_tile_offset = (data & 0x70) << 4;
		m_fg_tilemap->mark_all_dirty();
	}
}

void searchar_state::flipscreen_w(uint8_t data)
{
	flip_screen_set(BIT(data, 3));
	m_sprites->set_flip(BIT(data, 3));
	m_sprite_flip_axis = BIT(data, 2);
}


/***************************************************************************

  Display refresh

***************************************************************************/


uint32_t snk68_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->get_backdrop_pen(), cliprect);

	m_sprites->draw_sprites_all(bitmap, cliprect);

	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
