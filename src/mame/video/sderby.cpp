// license:BSD-3-Clause
// copyright-holders:David Haywood, Roberto Fresca
#include "emu.h"
#include "includes/sderby.h"

/* BG Layer */

TILE_GET_INFO_MEMBER(sderby_state::get_sderby_tile_info)
{
	int tileno,colour;

	tileno = m_videoram[tile_index*2];
	colour = m_videoram[tile_index*2+1] & 0x0f;

	SET_TILE_INFO_MEMBER(1,tileno,colour,0);
}

WRITE16_MEMBER(sderby_state::sderby_videoram_w)
{
	COMBINE_DATA(&m_videoram[offset]);
	m_tilemap->mark_tile_dirty(offset/2);
}

/* MD Layer */

TILE_GET_INFO_MEMBER(sderby_state::get_sderby_md_tile_info)
{
	int tileno,colour;

	tileno = m_md_videoram[tile_index*2];
	colour = m_md_videoram[tile_index*2+1] & 0x0f;

	SET_TILE_INFO_MEMBER(1,tileno,colour+16,0);
}

WRITE16_MEMBER(sderby_state::sderby_md_videoram_w)
{
	COMBINE_DATA(&m_md_videoram[offset]);
	m_md_tilemap->mark_tile_dirty(offset/2);
}

/* FG Layer */

TILE_GET_INFO_MEMBER(sderby_state::get_sderby_fg_tile_info)
{
	int tileno,colour;

	tileno = m_fg_videoram[tile_index*2];
	colour = m_fg_videoram[tile_index*2+1] & 0x0f;

	SET_TILE_INFO_MEMBER(0,tileno,colour+32,0);
}

WRITE16_MEMBER(sderby_state::sderby_fg_videoram_w)
{
	COMBINE_DATA(&m_fg_videoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset/2);
}


void sderby_state::draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect,int codeshift)
{
	UINT16 *spriteram16 = m_spriteram;
	int offs;
	int height = m_gfxdecode->gfx(0)->height();
	int colordiv = m_gfxdecode->gfx(0)->granularity() / 16;

	for (offs = 4;offs < m_spriteram.bytes()/2;offs += 4)
	{
		int sx,sy,code,color,flipx;

		sy = spriteram16[offs+3-4]; /* -4? what the... ??? */
		if (sy == 0x2000) return;   /* end of list marker */

		flipx = sy & 0x4000;
		sx = (spriteram16[offs+1] & 0x01ff) - 16-7;
		sy = (256-8-height - sy) & 0xff;
		code = spriteram16[offs+2] >> codeshift;
		color = (spriteram16[offs+1] & 0x3e00) >> 9;

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				code,
				color/colordiv+48,
				flipx,0,
				sx,sy,0);
	}
}


void sderby_state::video_start()
{
	m_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(sderby_state::get_sderby_tile_info),this),TILEMAP_SCAN_ROWS, 16, 16,32,32);
	m_md_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(sderby_state::get_sderby_md_tile_info),this),TILEMAP_SCAN_ROWS, 16, 16,32,32);

	m_md_tilemap->set_transparent_pen(0);

	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(sderby_state::get_sderby_fg_tile_info),this),TILEMAP_SCAN_ROWS, 8, 8,64,32);
	m_fg_tilemap->set_transparent_pen(0);
}

UINT32 sderby_state::screen_update_sderby(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap->draw(screen, bitmap, cliprect, 0,0);
	draw_sprites(bitmap,cliprect,0);
	m_md_tilemap->draw(screen, bitmap, cliprect, 0,0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}

UINT32 sderby_state::screen_update_pmroulet(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap->draw(screen, bitmap, cliprect, 0,0);
	m_md_tilemap->draw(screen, bitmap, cliprect, 0,0);
	draw_sprites(bitmap,cliprect,0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}


WRITE16_MEMBER(sderby_state::sderby_scroll_w)
{
	data = COMBINE_DATA(&m_scroll[offset]);

	switch (offset)
	{
		case 0: m_fg_tilemap->set_scrollx(0,data+2);break;
		case 1: m_fg_tilemap->set_scrolly(0,data-8);break;
		case 2: m_md_tilemap->set_scrollx(0,data+4);break;
		case 3: m_md_tilemap->set_scrolly(0,data-8);break;
		case 4: m_tilemap->set_scrollx(0,data+6);   break;
		case 5: m_tilemap->set_scrolly(0,data-8);   break;
	}
}
