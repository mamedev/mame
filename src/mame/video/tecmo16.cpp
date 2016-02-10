// license:???
// copyright-holders:Eisuke Watanabe, Nicola Salmoria
/******************************************************************************

  Ganbare Ginkun  (Japan)  (c)1995 TECMO
  Final StarForce (US)     (c)1992 TECMO
  Riot            (Japan)  (c)1992 NMK

  Based on sprite drivers from video/wc90.c by Ernesto Corvi (ernesto@imagina.com)

******************************************************************************/

#include "emu.h"
#include "includes/tecmo16.h"


/******************************************************************************/


void tecmo16_state::save_state()
{
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_scroll_x_w));
	save_item(NAME(m_scroll_y_w));
	save_item(NAME(m_scroll2_x_w));
	save_item(NAME(m_scroll2_y_w));
	save_item(NAME(m_scroll_char_x_w));
	save_item(NAME(m_scroll_char_y_w));
}

TILE_GET_INFO_MEMBER(tecmo16_state::fg_get_tile_info)
{
	int tile = m_videoram[tile_index] & 0x1fff;
	int color = m_colorram[tile_index] & 0x1f;

	/* bit 4 controls blending */
	//tileinfo.category = (m_colorram[tile_index] & 0x10) >> 4;

	SET_TILE_INFO_MEMBER(1,
			tile,
			color,
			0);
}

TILE_GET_INFO_MEMBER(tecmo16_state::bg_get_tile_info)
{
	int tile = m_videoram2[tile_index] & 0x1fff;
	int color = (m_colorram2[tile_index] & 0x0f);

	SET_TILE_INFO_MEMBER(1,
			tile,
			color,
			0);
}

TILE_GET_INFO_MEMBER(tecmo16_state::tx_get_tile_info)
{
	int tile = m_charram[tile_index];
	SET_TILE_INFO_MEMBER(0,
			tile & 0x0fff,
			tile >> 12,
			0);
}

/******************************************************************************/

void tecmo16_state::video_start()
{
	/* set up tile layers */
	m_screen->register_screen_bitmap(m_tile_bitmap_bg);
	m_screen->register_screen_bitmap(m_tile_bitmap_fg);

	/* set up sprites */
	m_screen->register_screen_bitmap(m_sprite_bitmap);

	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tecmo16_state::fg_get_tile_info),this),TILEMAP_SCAN_ROWS,16,16,32,32);
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tecmo16_state::bg_get_tile_info),this),TILEMAP_SCAN_ROWS,16,16,32,32);
	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tecmo16_state::tx_get_tile_info),this),TILEMAP_SCAN_ROWS, 8, 8,64,32);

	m_fg_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_transparent_pen(0);
	m_tx_tilemap->set_transparent_pen(0);

	m_tx_tilemap->set_scrolly(0,-16);
	m_flipscreen = 0;
	m_game_is_riot = 0;

	save_state();
}

VIDEO_START_MEMBER(tecmo16_state,ginkun)
{
	/* set up tile layers */
	m_screen->register_screen_bitmap(m_tile_bitmap_bg);
	m_screen->register_screen_bitmap(m_tile_bitmap_fg);

	/* set up sprites */
	m_screen->register_screen_bitmap(m_sprite_bitmap);

	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tecmo16_state::fg_get_tile_info),this),TILEMAP_SCAN_ROWS,16,16,64,32);
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tecmo16_state::bg_get_tile_info),this),TILEMAP_SCAN_ROWS,16,16,64,32);
	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tecmo16_state::tx_get_tile_info),this),TILEMAP_SCAN_ROWS, 8, 8,64,32);

	m_fg_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_transparent_pen(0);
	m_tx_tilemap->set_transparent_pen(0);
	m_flipscreen = 0;
	m_game_is_riot = 0;

	save_state();
}

VIDEO_START_MEMBER(tecmo16_state,riot)
{
	/* set up tile layers */
	m_screen->register_screen_bitmap(m_tile_bitmap_bg);
	m_screen->register_screen_bitmap(m_tile_bitmap_fg);

	/* set up sprites */
	m_screen->register_screen_bitmap(m_sprite_bitmap);

	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tecmo16_state::fg_get_tile_info),this),TILEMAP_SCAN_ROWS,16,16,64,32);
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tecmo16_state::bg_get_tile_info),this),TILEMAP_SCAN_ROWS,16,16,64,32);
	m_tx_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tecmo16_state::tx_get_tile_info),this),TILEMAP_SCAN_ROWS, 8, 8,64,32);

	m_fg_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_transparent_pen(0);
	m_tx_tilemap->set_transparent_pen(0);
	m_tx_tilemap->set_scrolldy(-16,-16);
	m_flipscreen = 0;
	m_game_is_riot = 1;

	save_state();
}

/******************************************************************************/

WRITE16_MEMBER(tecmo16_state::videoram_w)
{
	COMBINE_DATA(&m_videoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(tecmo16_state::colorram_w)
{
	COMBINE_DATA(&m_colorram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(tecmo16_state::videoram2_w)
{
	COMBINE_DATA(&m_videoram2[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(tecmo16_state::colorram2_w)
{
	COMBINE_DATA(&m_colorram2[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}


WRITE16_MEMBER(tecmo16_state::charram_w)
{
	COMBINE_DATA(&m_charram[offset]);
	m_tx_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER(tecmo16_state::flipscreen_w)
{
	m_flipscreen = data & 0x01;
	flip_screen_set(m_flipscreen);
}

/******************************************************************************/

WRITE16_MEMBER(tecmo16_state::scroll_x_w)
{
	COMBINE_DATA(&m_scroll_x_w);
	m_fg_tilemap->set_scrollx(0,m_scroll_x_w);
}

WRITE16_MEMBER(tecmo16_state::scroll_y_w)
{
	COMBINE_DATA(&m_scroll_y_w);
	m_fg_tilemap->set_scrolly(0,m_scroll_y_w);
}

WRITE16_MEMBER(tecmo16_state::scroll2_x_w)
{
	COMBINE_DATA(&m_scroll2_x_w);
	m_bg_tilemap->set_scrollx(0,m_scroll2_x_w);
}

WRITE16_MEMBER(tecmo16_state::scroll2_y_w)
{
	COMBINE_DATA(&m_scroll2_y_w);
	m_bg_tilemap->set_scrolly(0,m_scroll2_y_w);
}

WRITE16_MEMBER(tecmo16_state::scroll_char_x_w)
{
	COMBINE_DATA(&m_scroll_char_x_w);
	m_tx_tilemap->set_scrollx(0,m_scroll_char_x_w);
}

WRITE16_MEMBER(tecmo16_state::scroll_char_y_w)
{
	COMBINE_DATA(&m_scroll_char_y_w);
	m_tx_tilemap->set_scrolly(0,m_scroll_char_y_w-16);
}

/******************************************************************************/


/******************************************************************************/

UINT32 tecmo16_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_tile_bitmap_bg.fill(0, cliprect);
	m_tile_bitmap_fg.fill(0, cliprect);
	m_sprite_bitmap.fill(0, cliprect);
	bitmap.fill(0, cliprect);

	if (m_game_is_riot)  m_sprgen->gaiden_draw_sprites(screen, m_gfxdecode, cliprect, m_spriteram, 0, 0, flip_screen(),  m_sprite_bitmap);
	else m_sprgen->gaiden_draw_sprites(screen, m_gfxdecode, cliprect, m_spriteram, 2, 0, flip_screen(),  m_sprite_bitmap);

	m_bg_tilemap->draw(screen, m_tile_bitmap_bg, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, m_tile_bitmap_fg, cliprect, 0, 0);

	m_mixer->mix_bitmaps(screen, bitmap, cliprect, m_palette, &m_tile_bitmap_bg, &m_tile_bitmap_fg, (bitmap_ind16*)nullptr, &m_sprite_bitmap);

	// todo, this should go through the mixer!
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);


	return 0;
}
