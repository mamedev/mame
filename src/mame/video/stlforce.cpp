// license:BSD-3-Clause
// copyright-holders:David Haywood
/* video/stlforce.c - see main driver for other notes */

#include "emu.h"
#include "includes/stlforce.h"

/* background, appears to be the bottom layer */

TILE_GET_INFO_MEMBER(stlforce_state::get_stlforce_bg_tile_info)
{
	int tileno,colour;

	tileno = m_bg_videoram[tile_index] & 0x0fff;
	colour = m_bg_videoram[tile_index] & 0xe000;
	colour = colour >> 13;
	SET_TILE_INFO_MEMBER(0,tileno,colour,0);
}

WRITE16_MEMBER(stlforce_state::stlforce_bg_videoram_w)
{
	m_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

/* middle layer, low */

TILE_GET_INFO_MEMBER(stlforce_state::get_stlforce_mlow_tile_info)
{
	int tileno,colour;

	tileno = m_mlow_videoram[tile_index] & 0x0fff;
	colour = m_mlow_videoram[tile_index] & 0xe000;
	colour = colour >> 13;
	colour += 8;
	tileno += 0x1000;

	SET_TILE_INFO_MEMBER(0,tileno,colour,0);
}

WRITE16_MEMBER(stlforce_state::stlforce_mlow_videoram_w)
{
	m_mlow_videoram[offset] = data;
	m_mlow_tilemap->mark_tile_dirty(offset);
}

/* middle layer, high */

TILE_GET_INFO_MEMBER(stlforce_state::get_stlforce_mhigh_tile_info)
{
	int tileno,colour;

	tileno = m_mhigh_videoram[tile_index] & 0x0fff;
	colour = m_mhigh_videoram[tile_index] & 0xe000;
	colour = colour >> 13;
	colour += 16;
	tileno += 0x2000;

	SET_TILE_INFO_MEMBER(0,tileno,colour,0);
}

WRITE16_MEMBER(stlforce_state::stlforce_mhigh_videoram_w)
{
	m_mhigh_videoram[offset] = data;
	m_mhigh_tilemap->mark_tile_dirty(offset);
}

/* text layer, appears to be the top layer */

TILE_GET_INFO_MEMBER(stlforce_state::get_stlforce_tx_tile_info)
{
	int tileno,colour;

	tileno = m_tx_videoram[tile_index] & 0x0fff;
	colour = m_tx_videoram[tile_index] & 0xe000;
	colour = colour >> 13;

	tileno += 0xc000;

	colour += 24;
	SET_TILE_INFO_MEMBER(1,tileno,colour,0);
}

WRITE16_MEMBER(stlforce_state::stlforce_tx_videoram_w)
{
	m_tx_videoram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset);
}

/* sprites - quite a bit still needs doing .. */

void stlforce_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	const UINT16 *source = m_spriteram+0x0;
	const UINT16 *finish = m_spriteram+0x800;
	gfx_element *gfx = m_gfxdecode->gfx(2);
	int ypos, xpos, attr, num;

	while (source<finish)
	{
		if (source[0] & 0x0800)
		{
			ypos = source[0]& 0x01ff;
			attr = source[1]& 0x000f;
			xpos = source[3]& 0x03ff;
			num = (source[2] & 0x1fff);

			ypos = 512-ypos;


						gfx->transpen(bitmap,
						cliprect,
						num,
						64+attr,
						0,0,
						xpos+m_sprxoffs,ypos,0 );
		}

		source += 0x4;
	}
}

UINT32 stlforce_state::screen_update_stlforce(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i;

	if (m_vidattrram[6] & 1)
	{
		for(i=0;i<256;i++)
			m_bg_tilemap->set_scrollx(i, m_bg_scrollram[i]+9); //+9 for twinbrat
	}
	else
	{
		for(i=0;i<256;i++)
			m_bg_tilemap->set_scrollx(i, m_bg_scrollram[0]+9); //+9 for twinbrat
	}

	if (m_vidattrram[6] & 4)
	{
		for(i=0;i<256;i++)
			m_mlow_tilemap->set_scrollx(i, m_mlow_scrollram[i]+8);
	}
	else
	{
		for(i=0;i<256;i++)
			m_mlow_tilemap->set_scrollx(i, m_mlow_scrollram[0]+8);
	}

	if (m_vidattrram[6] & 0x10)
	{
		for(i=0;i<256;i++)
			m_mhigh_tilemap->set_scrollx(i, m_mhigh_scrollram[i]+8);
	}
	else
	{
		for(i=0;i<256;i++)
			m_mhigh_tilemap->set_scrollx(i, m_mhigh_scrollram[0]+8);
	}

	m_bg_tilemap->set_scrolly(0, m_vidattrram[1]);
	m_mlow_tilemap->set_scrolly(0, m_vidattrram[2]);
	m_mhigh_tilemap->set_scrolly(0, m_vidattrram[3]);

	m_tx_tilemap->set_scrollx(0, m_vidattrram[0]+8);
	m_tx_tilemap->set_scrolly(0,m_vidattrram[4]);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	m_mlow_tilemap->draw(screen, bitmap, cliprect, 0,0);
	m_mhigh_tilemap->draw(screen, bitmap, cliprect, 0,0);
	draw_sprites(bitmap,cliprect);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}

void stlforce_state::video_start()
{
	m_bg_tilemap    = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(stlforce_state::get_stlforce_bg_tile_info),this),   TILEMAP_SCAN_COLS,      16,16,64,16);
	m_mlow_tilemap  = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(stlforce_state::get_stlforce_mlow_tile_info),this), TILEMAP_SCAN_COLS, 16,16,64,16);
	m_mhigh_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(stlforce_state::get_stlforce_mhigh_tile_info),this),TILEMAP_SCAN_COLS, 16,16,64,16);
	m_tx_tilemap    = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(stlforce_state::get_stlforce_tx_tile_info),this),   TILEMAP_SCAN_ROWS,  8, 8,64,32);

	m_mlow_tilemap->set_transparent_pen(0);
	m_mhigh_tilemap->set_transparent_pen(0);
	m_tx_tilemap->set_transparent_pen(0);

	m_bg_tilemap->set_scroll_rows(256);
	m_mlow_tilemap->set_scroll_rows(256);
	m_mhigh_tilemap->set_scroll_rows(256);
}
