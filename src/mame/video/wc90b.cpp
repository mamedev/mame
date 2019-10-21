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
	SET_TILE_INFO_MEMBER(1,
			 ((((attr & 3) + ((attr >> 1) & 4)))<<8) | tile | 0x800,
			(attr >> 4) | 0x10,
			0);
}

TILE_GET_INFO_MEMBER(wc90b_state::get_fg_tile_info)
{
	int attr = m_fgvideoram[tile_index];
	int tile = m_fgvideoram[tile_index + 0x800];
	SET_TILE_INFO_MEMBER(1,
			((((attr & 3) + ((attr >> 1) & 4)))<<8) | tile,
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
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(wc90b_state::get_bg_tile_info), this), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(wc90b_state::get_fg_tile_info), this), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(wc90b_state::get_tx_tile_info), this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

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
	/* draw all visible sprites of specified priority */
	for ( int offs = m_spriteram.bytes() - 8 ; offs >= 0 ; offs -= 8 )
	{
		if ( ( ~( m_spriteram[offs+3] >> 7 ) & 1 ) == priority )
		{

			// 0   bbbb bbff   b = tile lower , f = flip bits
			// 1   yyyy yyyy
			// 2   xxxx xxxx
			// 3   PXcc cccc   P = priority X = x high, c = tile upper
			// 4   pppp ----   palette

			int tilehigh = ( m_spriteram[offs + 3] & 0x3f ) << 6;
			int tilelow = m_spriteram[offs + 0];
			int flags = m_spriteram[offs + 4];

			tilehigh += ( tilelow & 0xfc ) >> 2;

			int sx = m_spriteram[offs + 2];
			if (!(m_spriteram[offs + 3] & 0x40)) sx -= 0x0100;

			int sy = 240 - m_spriteram[offs + 1];

			m_gfxdecode->gfx(2)->transpen(bitmap,cliprect, tilehigh,
					flags >> 4, /* color */
					tilelow & 1,   /* flipx */
					tilelow & 2,   /* flipy */
					sx,
					sy,15 );
		}
	}
}

uint32_t wc90b_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0,8 * (m_scroll2x[0] & 0x7f) + 256 - 4 + (m_scroll_x_lo[0] & 0x07));
	m_bg_tilemap->set_scrolly(0,m_scroll2y[0] + 1 + ((m_scroll2x[0] & 0x80) ? 256 : 0));
	m_fg_tilemap->set_scrollx(0,8 * (m_scroll1x[0] & 0x7f) + 256 - 6 + ((m_scroll_x_lo[0] & 0x38) >> 3));
	m_fg_tilemap->set_scrolly(0,m_scroll1y[0] + 1 + ((m_scroll1x[0] & 0x80) ? 256 : 0));

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	draw_sprites(bitmap,cliprect, 1 );
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0,0);
	// TODO: if scoring on same Y as GOAL message, ball will be above it. Might be a btanb (or needs single pass draw + mix?)
	draw_sprites(bitmap,cliprect, 0 );
	return 0;
}

void eurogael_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority )
{
	/* draw all visible sprites of specified priority */

	// entry at start of RAM might not be a sprite
	for ( int offs = 0x200 - 4 ; offs >= 4 ; offs -= 4 )
	{
		if ( ( ( m_spriteram[offs+3] >> 4 ) & 1 ) == priority )
		{
			// this is wrong

			// 0      bbbb bbbb   b = tile lower
			// 1      yyyy yyyy
			// 2      xxxx xxxx
			// 3      ffXP cccc   f = flip bits, P = priority (inverted vs. other bootlegs) X = X high?, c = tile upper
			// 0x200  ---- -ppp   p = palette

			int tilehigh = ( m_spriteram[offs + 3] & 0x0f ) << 8;
			int attr = ( m_spriteram[offs + 3] & 0xf0 ) >> 4;

			int tilelow = m_spriteram[offs + 0];
			int flags = m_spriteram[offs + 0x200];

			tilehigh += tilelow;

			int sx = m_spriteram[offs + 2];
			if (!(attr & 0x02)) sx -= 0x0100;

			int sy = 240 - m_spriteram[offs + 1];

			m_gfxdecode->gfx(2)->transpen(bitmap,cliprect, tilehigh,
					(flags & 0x7) | 8, /* color - palettes 0x0 - 0x7 never written? */
					attr & 4,   /* flipx */
					attr & 8,   /* flipy */
					sx,
					sy,15 );
		}
	}
}

uint32_t eurogael_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// the code to write / clear tilemaps for fb and tx layers has been specifically modified to avoid writing to the last 4 bytes
	// and the game instead writes scroll values there instead, there is no code to copy from there, so it looks like these are the scroll regs

	// each of the 3 layer has its own PCB, all PCBs look identical, so why does handling differ slightly?

	int fg_scrollx = ((m_fgvideoram[0xffc]) | (m_fgvideoram[0xffd]<<8)) + 33;
	int fg_scrolly = ((m_fgvideoram[0xffe]) | (m_fgvideoram[0xfff]<<8)) + 1;
	int bg_scrollx = ((m_bgscroll[0xf00]) | (m_bgscroll[0xf01]<<8)) + 33;
	int bg_scrolly = ((m_bgscroll[0xf02]) | (m_bgscroll[0xf03]<<8)) + 1;
	int tx_scrollx = ((m_txvideoram[0xffc]) | (m_txvideoram[0xffd]<<8)) + 33;
	int tx_scrolly = ((m_txvideoram[0xffe]) | (m_txvideoram[0xfff]<<8)) + 1;

	m_bg_tilemap->set_scrollx(0, bg_scrollx);
	m_bg_tilemap->set_scrolly(0, bg_scrolly);
	m_fg_tilemap->set_scrollx(0, fg_scrollx);
	m_fg_tilemap->set_scrolly(0, fg_scrolly);
	m_tx_tilemap->set_scrollx(0, tx_scrollx);
	m_tx_tilemap->set_scrolly(0, tx_scrolly);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	draw_sprites(bitmap,cliprect, 1 );
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0,0);
	draw_sprites(bitmap,cliprect, 0 );
	return 0;
}
