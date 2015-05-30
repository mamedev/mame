// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
#include "emu.h"
#include "includes/wc90.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(wc90_state::get_bg_tile_info)
{
	int attr = m_bgvideoram[tile_index];
	int tile = m_bgvideoram[tile_index + 0x800] +
					256 * ((attr & 3) + ((attr >> 1) & 4));
	SET_TILE_INFO_MEMBER(2,
			tile,
			attr >> 4,
			0);
}

TILE_GET_INFO_MEMBER(wc90_state::get_fg_tile_info)
{
	int attr = m_fgvideoram[tile_index];
	int tile = m_fgvideoram[tile_index + 0x800] +
					256 * ((attr & 3) + ((attr >> 1) & 4));
	SET_TILE_INFO_MEMBER(1,
			tile,
			attr >> 4,
			0);
}

TILE_GET_INFO_MEMBER(wc90_state::get_tx_tile_info)
{
	SET_TILE_INFO_MEMBER(0,
			m_txvideoram[tile_index + 0x800] + ((m_txvideoram[tile_index] & 0x07) << 8),
			m_txvideoram[tile_index] >> 4,
			0);
}

TILE_GET_INFO_MEMBER(wc90_state::track_get_bg_tile_info)
{
	int attr = m_bgvideoram[tile_index];
	int tile = m_bgvideoram[tile_index + 0x800] +
					256 * (attr & 7);
	SET_TILE_INFO_MEMBER(2,
			tile,
			attr >> 4,
			0);
}

TILE_GET_INFO_MEMBER(wc90_state::track_get_fg_tile_info)
{
	int attr = m_fgvideoram[tile_index];
	int tile = m_fgvideoram[tile_index + 0x800] +
					256 * (attr & 7);
	SET_TILE_INFO_MEMBER(1,
			tile,
			attr >> 4,
			0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void wc90_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(wc90_state::get_bg_tile_info),this),TILEMAP_SCAN_ROWS,     16,16,64,32);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(wc90_state::get_fg_tile_info),this),TILEMAP_SCAN_ROWS,16,16,64,32);
	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(wc90_state::get_tx_tile_info),this),TILEMAP_SCAN_ROWS, 8, 8,64,32);

	m_fg_tilemap->set_transparent_pen(0);
	m_tx_tilemap->set_transparent_pen(0);
}

VIDEO_START_MEMBER(wc90_state,wc90t)
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(wc90_state::track_get_bg_tile_info),this),TILEMAP_SCAN_ROWS,     16,16,64,32);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(wc90_state::track_get_fg_tile_info),this),TILEMAP_SCAN_ROWS,16,16,64,32);
	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(wc90_state::get_tx_tile_info),this),TILEMAP_SCAN_ROWS, 8, 8,64,32);

	m_fg_tilemap->set_transparent_pen(0);
	m_tx_tilemap->set_transparent_pen(0);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(wc90_state::bgvideoram_w)
{
	m_bgvideoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x7ff);
}

WRITE8_MEMBER(wc90_state::fgvideoram_w)
{
	m_fgvideoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset & 0x7ff);
}

WRITE8_MEMBER(wc90_state::txvideoram_w)
{
	m_txvideoram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset & 0x7ff);
}



/***************************************************************************

  Display refresh

***************************************************************************/


UINT32 wc90_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0,m_scroll2xlo[0] + 256 * m_scroll2xhi[0]);
	m_bg_tilemap->set_scrolly(0,m_scroll2ylo[0] + 256 * m_scroll2yhi[0]);
	m_fg_tilemap->set_scrollx(0,m_scroll1xlo[0] + 256 * m_scroll1xhi[0]);
	m_fg_tilemap->set_scrolly(0,m_scroll1ylo[0] + 256 * m_scroll1yhi[0]);
	m_tx_tilemap->set_scrollx(0,m_scroll0xlo[0] + 256 * m_scroll0xhi[0]);
	m_tx_tilemap->set_scrolly(0,m_scroll0ylo[0] + 256 * m_scroll0yhi[0]);

//  m_sprgen->draw_sprites(bitmap,cliprect, m_gfxdecode, m_spriteram, m_spriteram.bytes(), 3); // unused
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	m_sprgen->draw_wc90_sprites(bitmap,cliprect, m_gfxdecode, m_spriteram, m_spriteram.bytes(), 2);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	m_sprgen->draw_wc90_sprites(bitmap,cliprect, m_gfxdecode, m_spriteram, m_spriteram.bytes(), 1);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0,0);
	m_sprgen->draw_wc90_sprites(bitmap,cliprect, m_gfxdecode, m_spriteram, m_spriteram.bytes(), 0);
	return 0;
}
