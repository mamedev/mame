// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

  video hardware for Tecmo games

***************************************************************************/

#include "emu.h"
#include "tecmo.h"


/*
   video_type is used to distinguish Rygar, Silkworm and Gemini Wing.
   This is needed because there is a difference in the tile and sprite indexing.
*/



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(tecmo_state::get_bg_tile_info)
{
	uint8_t const attr = m_bgvideoram[tile_index + 0x200];
	tileinfo.set(2,
			m_bgvideoram[tile_index] + ((attr & 0x07) << 8),
			attr >> 4,
			0);
}

TILE_GET_INFO_MEMBER(tecmo_state::get_fg_tile_info)
{
	uint8_t const attr = m_fgvideoram[tile_index + 0x200];
	tileinfo.set(1,
			m_fgvideoram[tile_index] + ((attr & 0x07) << 8),
			attr >> 4,
			0);
}

TILE_GET_INFO_MEMBER(tecmo_state::gemini_get_bg_tile_info)
{
	uint8_t const attr = m_bgvideoram[tile_index + 0x200];
	tileinfo.set(2,
			m_bgvideoram[tile_index] + ((attr & 0x70) << 4),
			attr & 0x0f,
			0);
}

TILE_GET_INFO_MEMBER(tecmo_state::gemini_get_fg_tile_info)
{
	uint8_t const attr = m_fgvideoram[tile_index + 0x200];
	tileinfo.set(1,
			m_fgvideoram[tile_index] + ((attr & 0x70) << 4),
			attr & 0x0f,
			0);
}

TILE_GET_INFO_MEMBER(tecmo_state::get_tx_tile_info)
{
	uint8_t const attr = m_txvideoram[tile_index + 0x400];
	tileinfo.set(0,
			m_txvideoram[tile_index] + ((attr & 0x03) << 8),
			attr >> 4,
			0);
}


/***************************************************************************

  Callbacks for the sprite priority

***************************************************************************/

uint32_t tecmo_state::pri_cb(uint8_t pri)
{
	// bg: 1; fg:2; text: 4
	switch (pri)
	{
		default:
		case 0x0: return 0;
		case 0x1: return 0xf0; // obscured by text layer
		case 0x2: return 0xf0|0xcc; // obscured by foreground
		case 0x3: return 0xf0|0xcc|0xaa; // obscured by bg and fg
	}
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void tecmo_state::video_start()
{
	if (m_video_type == 2)  // gemini
	{
		m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tecmo_state::gemini_get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16,16, 32,16);
		m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tecmo_state::gemini_get_fg_tile_info)), TILEMAP_SCAN_ROWS, 16,16, 32,16);
	}
	else    // rygar, silkworm
	{
		m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tecmo_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16,16, 32,16);
		m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tecmo_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 16,16, 32,16);
	}
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tecmo_state::get_tx_tile_info)), TILEMAP_SCAN_ROWS,  8, 8, 32,32);

	m_bg_tilemap->set_transparent_pen(0);
	m_fg_tilemap->set_transparent_pen(0);
	m_tx_tilemap->set_transparent_pen(0);

	m_bg_tilemap->set_scrolldx(-48,256+48);
	m_fg_tilemap->set_scrolldx(-48,256+48);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

void tecmo_state::txvideoram_w(offs_t offset, uint8_t data)
{
	m_txvideoram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset & 0x3ff);
}

void tecmo_state::fgvideoram_w(offs_t offset, uint8_t data)
{
	m_fgvideoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset & 0x1ff);
}

void tecmo_state::bgvideoram_w(offs_t offset, uint8_t data)
{
	m_bgvideoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x1ff);
}

void tecmo_state::fgscroll_w(offs_t offset, uint8_t data)
{
	m_fgscroll[offset] = data;

	m_fg_tilemap->set_scrollx(0, m_fgscroll[0] + 256 * m_fgscroll[1]);
	m_fg_tilemap->set_scrolly(0, m_fgscroll[2]);
	m_screen->update_partial(m_screen->vpos());
}

void tecmo_state::bgscroll_w(offs_t offset, uint8_t data)
{
	m_bgscroll[offset] = data;

	m_bg_tilemap->set_scrollx(0, m_bgscroll[0] + 256 * m_bgscroll[1]);
	m_bg_tilemap->set_scrolly(0, m_bgscroll[2]);
	m_screen->update_partial(m_screen->vpos());
}

void tecmo_state::flipscreen_w(uint8_t data)
{
	flip_screen_set(BIT(data, 0));
}



/***************************************************************************

  Display refresh

***************************************************************************/


uint32_t tecmo_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);
	bitmap.fill(0x100, cliprect);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 1);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 2);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 4);

	m_sprgen->draw_sprites_8bit(screen, bitmap, cliprect, m_spriteram, m_spriteram.bytes(), m_video_type, flip_screen());

	return 0;
}
