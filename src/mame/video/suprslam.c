// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Super Slams - video, see notes in driver file */

#include "emu.h"

#include "vsystem_spr.h"
#include "includes/suprslam.h"

/* FG 'SCREEN' LAYER */

WRITE16_MEMBER(suprslam_state::suprslam_screen_videoram_w)
{
	m_screen_videoram[offset] = data;
	m_screen_tilemap->mark_tile_dirty(offset);
}


TILE_GET_INFO_MEMBER(suprslam_state::get_suprslam_tile_info)
{
	int tileno = m_screen_videoram[tile_index] & 0x0fff;
	int colour = m_screen_videoram[tile_index] & 0xf000;

	tileno += m_screen_bank;
	colour = colour >> 12;

	SET_TILE_INFO_MEMBER(0, tileno, colour, 0);
}


/* BG LAYER */
WRITE16_MEMBER(suprslam_state::suprslam_bg_videoram_w)
{
	m_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


TILE_GET_INFO_MEMBER(suprslam_state::get_suprslam_bg_tile_info)
{
	int tileno = m_bg_videoram[tile_index] & 0x0fff;
	int colour = m_bg_videoram[tile_index] & 0xf000;

	tileno += m_bg_bank;
	colour = colour >> 12;

	SET_TILE_INFO_MEMBER(2, tileno, colour, 0);
}


UINT32 suprslam_state::suprslam_tile_callback( UINT32 code )
{
	return m_sp_videoram[code];
}



void suprslam_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(suprslam_state::get_suprslam_bg_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);
	m_screen_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(suprslam_state::get_suprslam_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_screen_tilemap->set_transparent_pen(15);
}

UINT32 suprslam_state::screen_update_suprslam(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_screen_tilemap->set_scrollx(0, m_screen_vregs[0x04/2] );

	bitmap.fill(m_palette->black_pen(), cliprect);
	m_k053936->zoom_draw(screen, bitmap, cliprect, m_bg_tilemap, 0, 0, 1);
	if(!(m_spr_ctrl[0] & 8))
		m_spr->draw_sprites(m_spriteram, m_spriteram.bytes(), screen, bitmap, cliprect);
	m_screen_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	if(m_spr_ctrl[0] & 8)
		m_spr->draw_sprites(m_spriteram, m_spriteram.bytes(), screen, bitmap, cliprect);
	return 0;
}

WRITE16_MEMBER(suprslam_state::suprslam_bank_w)
{
	UINT16 old_screen_bank, old_bg_bank;
	old_screen_bank = m_screen_bank;
	old_bg_bank = m_bg_bank;

	m_screen_bank = data & 0xf000;
	m_bg_bank = (data & 0x0f00) << 4;

	if (m_screen_bank != old_screen_bank)
		m_screen_tilemap->mark_all_dirty();
	if (m_bg_bank != old_bg_bank)
		m_bg_tilemap->mark_all_dirty();
}
