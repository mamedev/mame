// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Aquarium */

#include "emu.h"
#include "includes/aquarium.h"


/* TXT Layer */
TILE_GET_INFO_MEMBER(aquarium_state::get_aquarium_txt_tile_info)
{
	int tileno, colour;

	tileno = (m_txt_videoram[tile_index] & 0x0fff);
	colour = (m_txt_videoram[tile_index] & 0xf000) >> 12;
	SET_TILE_INFO_MEMBER(2, tileno, colour, 0);
}

WRITE16_MEMBER(aquarium_state::aquarium_txt_videoram_w)
{
	COMBINE_DATA(&m_txt_videoram[offset]);
	m_txt_tilemap->mark_tile_dirty(offset);
}

/* MID Layer */
TILE_GET_INFO_MEMBER(aquarium_state::get_aquarium_mid_tile_info)
{
	int tileno, colour, flag;

	tileno = (m_mid_videoram[tile_index * 2] & 0x0fff);
	colour = (m_mid_videoram[tile_index * 2 + 1] & 0x001f);
	flag = TILE_FLIPYX((m_mid_videoram[tile_index * 2 + 1] & 0x300) >> 8);

	SET_TILE_INFO_MEMBER(1, tileno, colour, flag);

	tileinfo.category = (m_mid_videoram[tile_index * 2 + 1] & 0x20) >> 5;
}

WRITE16_MEMBER(aquarium_state::aquarium_mid_videoram_w)
{
	COMBINE_DATA(&m_mid_videoram[offset]);
	m_mid_tilemap->mark_tile_dirty(offset / 2);
}

/* BAK Layer */
TILE_GET_INFO_MEMBER(aquarium_state::get_aquarium_bak_tile_info)
{
	int tileno, colour, flag;

	tileno = (m_bak_videoram[tile_index * 2] & 0x0fff);
	colour = (m_bak_videoram[tile_index * 2 + 1] & 0x001f);
	flag = TILE_FLIPYX((m_bak_videoram[tile_index * 2 + 1] & 0x300) >> 8);

	SET_TILE_INFO_MEMBER(3, tileno, colour, flag);

	tileinfo.category = (m_bak_videoram[tile_index * 2 + 1] & 0x20) >> 5;
}

WRITE16_MEMBER(aquarium_state::aquarium_bak_videoram_w)
{
	COMBINE_DATA(&m_bak_videoram[offset]);
	m_bak_tilemap->mark_tile_dirty(offset / 2);
}

void aquarium_state::video_start()
{
	m_txt_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(aquarium_state::get_aquarium_txt_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_bak_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(aquarium_state::get_aquarium_bak_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_mid_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(aquarium_state::get_aquarium_mid_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	m_txt_tilemap->set_transparent_pen(0);
	m_mid_tilemap->set_transparent_pen(0);
}

UINT32 aquarium_state::screen_update_aquarium(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_mid_tilemap->set_scrollx(0, m_scroll[0]);
	m_mid_tilemap->set_scrolly(0, m_scroll[1]);
	m_bak_tilemap->set_scrollx(0, m_scroll[2]);
	m_bak_tilemap->set_scrolly(0, m_scroll[3]);
	m_txt_tilemap->set_scrollx(0, m_scroll[4]);
	m_txt_tilemap->set_scrolly(0, m_scroll[5]);

	m_bak_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_mid_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	m_sprgen->aquarium_draw_sprites(bitmap, cliprect, m_gfxdecode, 16);

	m_bak_tilemap->draw(screen, bitmap, cliprect, 1, 0);
	m_mid_tilemap->draw(screen, bitmap, cliprect, 1, 0);
	m_txt_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
