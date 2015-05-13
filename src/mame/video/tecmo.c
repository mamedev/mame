// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

  video hardware for Tecmo games

***************************************************************************/

#include "emu.h"
#include "includes/tecmo.h"


/*
   video_type is used to distinguish Rygar, Silkworm and Gemini Wing.
   This is needed because there is a difference in the tile and sprite indexing.
*/



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(tecmo_state::get_bg_tile_info)
{
	UINT8 attr = m_bgvideoram[tile_index+0x200];
	SET_TILE_INFO_MEMBER(3,
			m_bgvideoram[tile_index] + ((attr & 0x07) << 8),
			attr >> 4,
			0);
}

TILE_GET_INFO_MEMBER(tecmo_state::get_fg_tile_info)
{
	UINT8 attr = m_fgvideoram[tile_index+0x200];
	SET_TILE_INFO_MEMBER(2,
			m_fgvideoram[tile_index] + ((attr & 0x07) << 8),
			attr >> 4,
			0);
}

TILE_GET_INFO_MEMBER(tecmo_state::gemini_get_bg_tile_info)
{
	UINT8 attr = m_bgvideoram[tile_index+0x200];
	SET_TILE_INFO_MEMBER(3,
			m_bgvideoram[tile_index] + ((attr & 0x70) << 4),
			attr & 0x0f,
			0);
}

TILE_GET_INFO_MEMBER(tecmo_state::gemini_get_fg_tile_info)
{
	UINT8 attr = m_fgvideoram[tile_index+0x200];
	SET_TILE_INFO_MEMBER(2,
			m_fgvideoram[tile_index] + ((attr & 0x70) << 4),
			attr & 0x0f,
			0);
}

TILE_GET_INFO_MEMBER(tecmo_state::get_tx_tile_info)
{
	UINT8 attr = m_txvideoram[tile_index+0x400];
	SET_TILE_INFO_MEMBER(0,
			m_txvideoram[tile_index] + ((attr & 0x03) << 8),
			attr >> 4,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void tecmo_state::video_start()
{
	if (m_video_type == 2)  /* gemini */
	{
		m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tecmo_state::gemini_get_bg_tile_info),this),TILEMAP_SCAN_ROWS,16,16,32,16);
		m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tecmo_state::gemini_get_fg_tile_info),this),TILEMAP_SCAN_ROWS,16,16,32,16);
	}
	else    /* rygar, silkworm */
	{
		m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tecmo_state::get_bg_tile_info),this),TILEMAP_SCAN_ROWS,16,16,32,16);
		m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tecmo_state::get_fg_tile_info),this),TILEMAP_SCAN_ROWS,16,16,32,16);
	}
	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tecmo_state::get_tx_tile_info),this),TILEMAP_SCAN_ROWS, 8, 8,32,32);

	m_bg_tilemap->set_transparent_pen(0);
	m_fg_tilemap->set_transparent_pen(0);
	m_tx_tilemap->set_transparent_pen(0);

	m_bg_tilemap->set_scrolldx(-48,256+48);
	m_fg_tilemap->set_scrolldx(-48,256+48);

	save_item(NAME(m_fgscroll));
	save_item(NAME(m_bgscroll));
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(tecmo_state::txvideoram_w)
{
	m_txvideoram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset & 0x3ff);
}

WRITE8_MEMBER(tecmo_state::fgvideoram_w)
{
	m_fgvideoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset & 0x1ff);
}

WRITE8_MEMBER(tecmo_state::bgvideoram_w)
{
	m_bgvideoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x1ff);
}

WRITE8_MEMBER(tecmo_state::fgscroll_w)
{
	m_fgscroll[offset] = data;

	m_fg_tilemap->set_scrollx(0, m_fgscroll[0] + 256 * m_fgscroll[1]);
	m_fg_tilemap->set_scrolly(0, m_fgscroll[2]);
}

WRITE8_MEMBER(tecmo_state::bgscroll_w)
{
	m_bgscroll[offset] = data;

	m_bg_tilemap->set_scrollx(0, m_bgscroll[0] + 256 * m_bgscroll[1]);
	m_bg_tilemap->set_scrolly(0, m_bgscroll[2]);
}

WRITE8_MEMBER(tecmo_state::flipscreen_w)
{
	flip_screen_set(data & 1);
}



/***************************************************************************

  Display refresh

***************************************************************************/



UINT32 tecmo_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);
	bitmap.fill(0x100, cliprect);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0,1);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0,2);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0,4);

	m_sprgen->draw_sprites_8bit(screen,bitmap,m_gfxdecode,cliprect, m_spriteram, m_spriteram.bytes(), m_video_type, flip_screen());
	return 0;
}
