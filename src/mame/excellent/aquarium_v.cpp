// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Aquarium */

#include "emu.h"
#include "aquarium.h"


/* TXT Layer */
TILE_GET_INFO_MEMBER(aquarium_state::get_txt_tile_info)
{
	const u32 tileno = (m_txt_videoram[tile_index] & 0x0fff);
	const u32 colour = (m_txt_videoram[tile_index] & 0xf000) >> 12;
	tileinfo.set(0, tileno, colour, 0);

	tileinfo.category = (m_txt_videoram[tile_index] & 0x8000) >> 15;
}

void aquarium_state::txt_videoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_txt_videoram[offset]);
	m_txt_tilemap->mark_tile_dirty(offset);
}

/* MID Layer */
TILE_GET_INFO_MEMBER(aquarium_state::get_mid_tile_info)
{
	const u32 tileno = (m_mid_videoram[tile_index * 2] & 0x0fff);
	const u32 colour = (m_mid_videoram[tile_index * 2 + 1] & 0x001f);
	const int flag = TILE_FLIPYX((m_mid_videoram[tile_index * 2 + 1] & 0x300) >> 8);

	tileinfo.set(1, tileno, colour, flag);

	tileinfo.category = (m_mid_videoram[tile_index * 2 + 1] & 0x20) >> 5;
}

void aquarium_state::mid_videoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_mid_videoram[offset]);
	m_mid_tilemap->mark_tile_dirty(offset / 2);
}

/* BAK Layer */
TILE_GET_INFO_MEMBER(aquarium_state::get_bak_tile_info)
{
	const u32 tileno = (m_bak_videoram[tile_index * 2] & 0x0fff);
	const u32 colour = (m_bak_videoram[tile_index * 2 + 1] & 0x001f);
	const int flag = TILE_FLIPYX((m_bak_videoram[tile_index * 2 + 1] & 0x300) >> 8);

	tileinfo.set(2, tileno, colour, flag);

	tileinfo.category = (m_bak_videoram[tile_index * 2 + 1] & 0x20) >> 5;
}

void aquarium_state::bak_videoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_bak_videoram[offset]);
	m_bak_tilemap->mark_tile_dirty(offset / 2);
}

void aquarium_state::video_start()
{
	m_txt_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(aquarium_state::get_txt_tile_info)), TILEMAP_SCAN_ROWS,  8,  8, 64, 64);
	m_mid_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(aquarium_state::get_mid_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_bak_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(aquarium_state::get_bak_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	m_txt_tilemap->set_transparent_pen(0);
	m_mid_tilemap->set_transparent_pen(0);
	m_bak_tilemap->set_transparent_pen(0);
}

void aquarium_state::aquarium_colpri_cb(u32 &colour, u32 &pri_mask)
{
	pri_mask = 0;
	if (colour & 8)
		pri_mask |= (GFX_PMASK_2 | GFX_PMASK_4 | GFX_PMASK_8);
}

uint32_t aquarium_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_mid_tilemap->set_scrollx(0, m_scroll[0]);
	m_mid_tilemap->set_scrolly(0, m_scroll[1]);
	m_bak_tilemap->set_scrollx(0, m_scroll[2]);
	m_bak_tilemap->set_scrolly(0, m_scroll[3]);
	m_txt_tilemap->set_scrollx(0, m_scroll[4]);
	m_txt_tilemap->set_scrolly(0, m_scroll[5]);

	screen.priority().fill(0, cliprect);
	bitmap.fill(0, cliprect); // WDUD logo suggests this

	m_bak_tilemap->draw(screen, bitmap, cliprect, 0, 1);
	m_mid_tilemap->draw(screen, bitmap, cliprect, 0, 2);
	m_txt_tilemap->draw(screen, bitmap, cliprect, 1, 4);

	m_bak_tilemap->draw(screen, bitmap, cliprect, 1, 8);
	m_sprgen->aquarium_draw_sprites(screen, bitmap, cliprect, 16);
	m_mid_tilemap->draw(screen, bitmap, cliprect, 1, 0);
	m_txt_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}
