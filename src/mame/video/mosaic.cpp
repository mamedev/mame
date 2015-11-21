// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

  mosaic.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/mosaic.h"

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(mosaic_state::get_fg_tile_info)
{
	tile_index *= 2;
	SET_TILE_INFO_MEMBER(0,
			m_fgvideoram[tile_index] + (m_fgvideoram[tile_index+1] << 8),
			0,
			0);
}

TILE_GET_INFO_MEMBER(mosaic_state::get_bg_tile_info)
{
	tile_index *= 2;
	SET_TILE_INFO_MEMBER(1,
			m_bgvideoram[tile_index] + (m_bgvideoram[tile_index+1] << 8),
			0,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void mosaic_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(mosaic_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(mosaic_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_fg_tilemap->set_transparent_pen(0xff);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(mosaic_state::fgvideoram_w)
{
	m_fgvideoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset / 2);
}

WRITE8_MEMBER(mosaic_state::bgvideoram_w)
{
	m_bgvideoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}



UINT32 mosaic_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
