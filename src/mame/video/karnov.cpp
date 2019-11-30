// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*******************************************************************************

    Karnov - Bryan McPhail, mish@tendril.co.uk

*******************************************************************************/

#include "emu.h"
#include "includes/karnov.h"

/******************************************************************************/

uint32_t karnov_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int const flip = BIT(m_scroll[0], 15);

	m_bg_tilemap->set_flip(flip ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	m_fix_tilemap->set_flip(flip ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	m_spritegen->set_flip_screen(flip);

	m_bg_tilemap->set_scrollx(m_scroll[0]);
	m_bg_tilemap->set_scrolly(m_scroll[1]);
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	m_spritegen->draw_sprites(screen, bitmap, cliprect, m_gfxdecode->gfx(2), m_spriteram->buffer(), 0x800);
	m_fix_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

/******************************************************************************/

TILE_GET_INFO_MEMBER(karnov_state::get_fix_tile_info)
{
	int tile = m_videoram[tile_index];
	SET_TILE_INFO_MEMBER(0,
			tile & 0xfff,
			tile >> 14,
			0);
}

TILE_GET_INFO_MEMBER(karnov_state::get_bg_tile_info)
{
	int tile = m_pf_data[tile_index];
	SET_TILE_INFO_MEMBER(1,
			tile & 0x7ff,
			tile >> 12,
			0);
}

WRITE16_MEMBER(karnov_state::videoram_w)
{
	COMBINE_DATA(&m_videoram[offset]);
	m_fix_tilemap->mark_tile_dirty(offset);
}

void karnov_state::playfield_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_pf_data[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}

/******************************************************************************/

VIDEO_START_MEMBER(karnov_state,karnov)
{
	/* Allocate bitmap & tilemap */
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(karnov_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_fix_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(karnov_state::get_fix_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_fix_tilemap->set_transparent_pen(0);
}

VIDEO_START_MEMBER(karnov_state,wndrplnt)
{
	/* Allocate bitmap & tilemap */
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(karnov_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_fix_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(karnov_state::get_fix_tile_info)), TILEMAP_SCAN_COLS, 8, 8, 32, 32);

	m_fix_tilemap->set_transparent_pen(0);
}

/******************************************************************************/
