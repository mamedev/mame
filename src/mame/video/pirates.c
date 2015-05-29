// license:BSD-3-Clause
// copyright-holders:David Haywood,Nicola Salmoria,Paul Priest
#include "emu.h"
#include "includes/pirates.h"


/* Video Hardware */

/* tilemaps */

TILE_GET_INFO_MEMBER(pirates_state::get_tx_tile_info)
{
	int code = m_tx_tileram[tile_index*2];
	int colr = m_tx_tileram[tile_index*2+1];

	SET_TILE_INFO_MEMBER(0,code,colr,0);
}

TILE_GET_INFO_MEMBER(pirates_state::get_fg_tile_info)
{
	int code = m_fg_tileram[tile_index*2];
	int colr = m_fg_tileram[tile_index*2+1]+0x80;

	SET_TILE_INFO_MEMBER(0,code,colr,0);
}

TILE_GET_INFO_MEMBER(pirates_state::get_bg_tile_info)
{
	int code = m_bg_tileram[tile_index*2];
	int colr = m_bg_tileram[tile_index*2+1]+ 0x100;

	SET_TILE_INFO_MEMBER(0,code,colr,0);
}


/* video start / update */

void pirates_state::video_start()
{
	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(pirates_state::get_tx_tile_info),this),TILEMAP_SCAN_COLS,8,8,36,32);

	/* Not sure how big they can be, Pirates uses only 32 columns, Genix 44 */
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(pirates_state::get_fg_tile_info),this),TILEMAP_SCAN_COLS,8,8,64,32);
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(pirates_state::get_bg_tile_info),this),TILEMAP_SCAN_COLS,     8,8,64,32);

	m_tx_tilemap->set_transparent_pen(0);
	m_fg_tilemap->set_transparent_pen(0);
}



WRITE16_MEMBER(pirates_state::tx_tileram_w)
{
	COMBINE_DATA(m_tx_tileram+offset);
	m_tx_tilemap->mark_tile_dirty(offset/2);
}

WRITE16_MEMBER(pirates_state::fg_tileram_w)
{
	COMBINE_DATA(m_fg_tileram+offset);
	m_fg_tilemap->mark_tile_dirty(offset/2);
}

WRITE16_MEMBER(pirates_state::bg_tileram_w)
{
	COMBINE_DATA(m_bg_tileram+offset);
	m_bg_tilemap->mark_tile_dirty(offset/2);
}



void pirates_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx = m_gfxdecode->gfx(1);
	UINT16 *source = m_spriteram + 4;
	UINT16 *finish = source + 0x800/2-4;

	while( source<finish )
	{
		int xpos, ypos, flipx, flipy, code, color;

		xpos = source[1] - 32;
		ypos = source[-1];  // indeed...

		if (ypos & 0x8000) break;   /* end-of-list marker */

		code = source[2] >> 2;
		color = source[0] & 0xff;
		flipx = source[2] & 2;
		flipy = source[2] & 1;

		ypos = 0xf2 - ypos;

		gfx->transpen(bitmap,cliprect,
				code,
				color,
				flipx,flipy,
				xpos,ypos,0);

		source+=4;
	}
}

UINT32 pirates_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0,m_scroll[0]);
	m_fg_tilemap->set_scrollx(0,m_scroll[0]);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	draw_sprites(bitmap,cliprect);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}
