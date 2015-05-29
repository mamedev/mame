// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
#include "emu.h"
#include "includes/wc90b.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(wc90b_state::get_bg_tile_info)
{
	int attr = m_bgvideoram[tile_index];
	int tile = m_bgvideoram[tile_index + 0x800];
	SET_TILE_INFO_MEMBER(9 + ((attr & 3) + ((attr >> 1) & 4)),
			tile,
			attr >> 4,
			0);
}

TILE_GET_INFO_MEMBER(wc90b_state::get_fg_tile_info)
{
	int attr = m_fgvideoram[tile_index];
	int tile = m_fgvideoram[tile_index + 0x800];
	SET_TILE_INFO_MEMBER(1 + ((attr & 3) + ((attr >> 1) & 4)),
			tile,
			attr >> 4,
			0);
}

TILE_GET_INFO_MEMBER(wc90b_state::get_tx_tile_info)
{
	SET_TILE_INFO_MEMBER(0,
			m_txvideoram[tile_index + 0x800] + ((m_txvideoram[tile_index] & 0x07) << 8),
			m_txvideoram[tile_index] >> 4,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void wc90b_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(wc90b_state::get_bg_tile_info),this),TILEMAP_SCAN_ROWS,     16,16,64,32);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(wc90b_state::get_fg_tile_info),this),TILEMAP_SCAN_ROWS,16,16,64,32);
	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(wc90b_state::get_tx_tile_info),this),TILEMAP_SCAN_ROWS, 8, 8,64,32);

	m_fg_tilemap->set_transparent_pen(15);
	m_tx_tilemap->set_transparent_pen(15);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(wc90b_state::bgvideoram_w)
{
	m_bgvideoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x7ff);
}

WRITE8_MEMBER(wc90b_state::fgvideoram_w)
{
	m_fgvideoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset & 0x7ff);
}

WRITE8_MEMBER(wc90b_state::txvideoram_w)
{
	m_txvideoram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset & 0x7ff);
}



/***************************************************************************

  Display refresh

***************************************************************************/

void wc90b_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority )
{
	UINT8 *spriteram = m_spriteram;
	int offs, sx, sy;

	/* draw all visible sprites of specified priority */
	for ( offs = m_spriteram.bytes() - 8 ; offs >= 0 ; offs -= 8 )
	{
		if ( ( ~( spriteram[offs+3] >> 7 ) & 1 ) == priority )
		{
			int code = ( spriteram[offs + 3] & 0x3f ) << 4;
			int bank = spriteram[offs + 0];
			int flags = spriteram[offs + 4];

			code += ( bank & 0xf0 ) >> 4;
			code <<= 2;
			code += ( bank & 0x0f ) >> 2;

			sx = spriteram[offs + 2];
			if (!(spriteram[offs + 3] & 0x40)) sx -= 0x0100;

			sy = 240 - spriteram[offs + 1];

			m_gfxdecode->gfx(17)->transpen(bitmap,cliprect, code,
					flags >> 4, /* color */
					bank & 1,   /* flipx */
					bank & 2,   /* flipy */
					sx,
					sy,15 );
		}
	}
}

UINT32 wc90b_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0,8 * (m_scroll2x[0] & 0x7f) + 256 - 4 + (m_scroll_x_lo[0] & 0x07));
	m_bg_tilemap->set_scrolly(0,m_scroll2y[0] + 1 + ((m_scroll2x[0] & 0x80) ? 256 : 0));
	m_fg_tilemap->set_scrollx(0,8 * (m_scroll1x[0] & 0x7f) + 256 - 6 + ((m_scroll_x_lo[0] & 0x38) >> 3));
	m_fg_tilemap->set_scrolly(0,m_scroll1y[0] + 1 + ((m_scroll1x[0] & 0x80) ? 256 : 0));

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	draw_sprites(bitmap,cliprect, 1 );
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0,0);
	draw_sprites(bitmap,cliprect, 0 );
	return 0;
}
