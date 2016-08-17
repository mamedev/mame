// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Ernesto Corvi
#include "emu.h"
#include "includes/iqblock.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(iqblock_state::get_bg_tile_info)
{
	int code = m_bgvideoram[tile_index] + (m_bgvideoram[tile_index + 0x800] << 8);
	SET_TILE_INFO_MEMBER(0,
			code &(m_video_type ? 0x1fff : 0x3fff),
			m_video_type? (2*(code >> 13)+1) : (4*(code >> 14)+3),
			0);
}

TILE_GET_INFO_MEMBER(iqblock_state::get_fg_tile_info)
{
	int code = m_fgvideoram[tile_index];
	SET_TILE_INFO_MEMBER(1,
			code & 0x7f,
			(code & 0x80) ? 3 : 0,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void iqblock_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(iqblock_state::get_bg_tile_info),this),TILEMAP_SCAN_ROWS,     8, 8,64,32);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(iqblock_state::get_fg_tile_info),this),TILEMAP_SCAN_ROWS,8,32,64, 8);

	m_bg_tilemap->set_transparent_pen(0);
	m_fg_tilemap->set_scroll_cols(64);

	save_item(NAME(m_videoenable));
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(iqblock_state::fgvideoram_w)
{
	m_fgvideoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(iqblock_state::bgvideoram_w)
{
	m_bgvideoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x7ff);
}

WRITE8_MEMBER(iqblock_state::fgscroll_w)
{
	m_fg_tilemap->set_scrolly(offset,data);
}



/***************************************************************************

  Display refresh

***************************************************************************/

UINT32 iqblock_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (!m_videoenable) return 0;
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0,0);

	return 0;
}
