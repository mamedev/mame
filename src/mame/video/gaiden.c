// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/***************************************************************************

    Ninja Gaiden / Tecmo Knights Video Hardware

***************************************************************************/

#include "emu.h"
#include "includes/gaiden.h"

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(gaiden_state::get_bg_tile_info)
{
	UINT16 *videoram1 = &m_videoram3[0x0800];
	UINT16 *videoram2 = m_videoram3;
	SET_TILE_INFO_MEMBER(1,
			videoram1[tile_index] & 0x0fff,
			(videoram2[tile_index] & 0xf0) >> 4,
			0);
}

TILE_GET_INFO_MEMBER(gaiden_state::get_fg_tile_info)
{
	UINT16 *videoram1 = &m_videoram2[0x0800];
	UINT16 *videoram2 = m_videoram2;
	SET_TILE_INFO_MEMBER(2,
			videoram1[tile_index] & 0x0fff,
			(videoram2[tile_index] & 0xf0) >> 4,
			0);
}

TILE_GET_INFO_MEMBER(gaiden_state::get_fg_tile_info_raiga)
{
	UINT16 *videoram1 = &m_videoram2[0x0800];
	UINT16 *videoram2 = m_videoram2;

	int colour = ((videoram2[tile_index] & 0xf0) >> 4);

	/* bit 3 controls blending */
	if ((videoram2[tile_index] & 0x08))
		colour += 0x10;

	SET_TILE_INFO_MEMBER(2,
			videoram1[tile_index] & 0x0fff,
			colour,
			0);
}

TILE_GET_INFO_MEMBER(gaiden_state::get_tx_tile_info)
{
	UINT16 *videoram1 = &m_videoram[0x0400];
	UINT16 *videoram2 = m_videoram;
	SET_TILE_INFO_MEMBER(0,
			videoram1[tile_index] & 0x07ff,
			(videoram2[tile_index] & 0xf0) >> 4,
			0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START_MEMBER(gaiden_state,gaiden)
{
	/* set up tile layers */
	m_screen->register_screen_bitmap(m_tile_bitmap_bg);
	m_screen->register_screen_bitmap(m_tile_bitmap_fg);

	m_background = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(gaiden_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
	m_foreground = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(gaiden_state::get_fg_tile_info_raiga),this), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
	m_text_layer = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(gaiden_state::get_tx_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

//  m_background->set_transparent_pen(0);
//  m_foreground->set_transparent_pen(0);
	m_text_layer->set_transparent_pen(0);

	m_background->set_scrolldy(0, 33);
	m_foreground->set_scrolldy(0, 33);
	m_text_layer->set_scrolldy(0, 31);

	m_background->set_scrolldx(0, -1);
	m_foreground->set_scrolldx(0, -1);
	m_text_layer->set_scrolldx(0, -1);

	/* set up sprites */
	m_screen->register_screen_bitmap(m_sprite_bitmap);
}


VIDEO_START_MEMBER(gaiden_state,raiga)
{
	/* set up tile layers */
	m_screen->register_screen_bitmap(m_tile_bitmap_bg);
	m_screen->register_screen_bitmap(m_tile_bitmap_fg);

	m_background = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(gaiden_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
	m_foreground = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(gaiden_state::get_fg_tile_info_raiga),this), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
	m_text_layer = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(gaiden_state::get_tx_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

//  m_background->set_transparent_pen(0);
//  m_foreground->set_transparent_pen(0);
	m_text_layer->set_transparent_pen(0);

	/* set up sprites */
	m_screen->register_screen_bitmap(m_sprite_bitmap);
}

VIDEO_START_MEMBER(gaiden_state,drgnbowl)
{
	/* set up tile layers */
	m_background = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(gaiden_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
	m_foreground = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(gaiden_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
	m_text_layer = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(gaiden_state::get_tx_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_foreground->set_transparent_pen(15);
	m_text_layer->set_transparent_pen(15);

	m_background->set_scrolldx(-248, 248);
	m_foreground->set_scrolldx(-252, 252);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE16_MEMBER(gaiden_state::gaiden_flip_w)
{
	if (ACCESSING_BITS_0_7)
		flip_screen_set(data & 1);
}

WRITE16_MEMBER(gaiden_state::gaiden_txscrollx_w)
{
	COMBINE_DATA(&m_tx_scroll_x);
	m_text_layer->set_scrollx(0, m_tx_scroll_x);
}

WRITE16_MEMBER(gaiden_state::gaiden_txscrolly_w)
{
	COMBINE_DATA(&m_tx_scroll_y);
	m_text_layer->set_scrolly(0, (m_tx_scroll_y - m_tx_offset_y) & 0xffff);
}

WRITE16_MEMBER(gaiden_state::gaiden_fgscrollx_w)
{
	COMBINE_DATA(&m_fg_scroll_x);
	m_foreground->set_scrollx(0, m_fg_scroll_x);
}

WRITE16_MEMBER(gaiden_state::gaiden_fgscrolly_w)
{
	COMBINE_DATA(&m_fg_scroll_y);
	m_foreground->set_scrolly(0, (m_fg_scroll_y - m_fg_offset_y) & 0xffff);
}

WRITE16_MEMBER(gaiden_state::gaiden_bgscrollx_w)
{
	COMBINE_DATA(&m_bg_scroll_x);
	m_background->set_scrollx(0, m_bg_scroll_x);
}

WRITE16_MEMBER(gaiden_state::gaiden_bgscrolly_w)
{
	COMBINE_DATA(&m_bg_scroll_y);
	m_background->set_scrolly(0, (m_bg_scroll_y - m_bg_offset_y) & 0xffff);
}

WRITE16_MEMBER(gaiden_state::gaiden_txoffsety_w)
{
	if (ACCESSING_BITS_0_7) {
		m_tx_offset_y = data;
		m_text_layer->set_scrolly(0, (m_tx_scroll_y - m_tx_offset_y) & 0xffff);
	}
}

WRITE16_MEMBER(gaiden_state::gaiden_fgoffsety_w)
{
	if (ACCESSING_BITS_0_7) {
		m_fg_offset_y = data;
		m_foreground->set_scrolly(0, (m_fg_scroll_y - m_fg_offset_y) & 0xffff);
	}
}

WRITE16_MEMBER(gaiden_state::gaiden_bgoffsety_w)
{
	if (ACCESSING_BITS_0_7) {
		m_bg_offset_y = data;
		m_background->set_scrolly(0, (m_bg_scroll_y - m_bg_offset_y) & 0xffff);
	}
}

WRITE16_MEMBER(gaiden_state::gaiden_sproffsety_w)
{
	if (ACCESSING_BITS_0_7) {
		m_spr_offset_y = data;
		// handled in draw_sprites
	}
}


WRITE16_MEMBER(gaiden_state::gaiden_videoram3_w)
{
	COMBINE_DATA(&m_videoram3[offset]);
	m_background->mark_tile_dirty(offset & 0x07ff);
}

WRITE16_MEMBER(gaiden_state::gaiden_videoram2_w)
{
	COMBINE_DATA(&m_videoram2[offset]);
	m_foreground->mark_tile_dirty(offset & 0x07ff);
}
WRITE16_MEMBER(gaiden_state::gaiden_videoram_w)
{
	COMBINE_DATA(&m_videoram[offset]);
	m_text_layer->mark_tile_dirty(offset & 0x03ff);
}



/***************************************************************************

  Display refresh

***************************************************************************/

// dragon bowl uses a bootleg format
/* sprite format:
 *
 *  word        bit                 usage
 * --------+-fedcba9876543210-+----------------
 *    0    | --------xxxxxxxx | sprite code (lower bits)
 *         | ---xxxxx-------- | unused ?
 *    1    | --------xxxxxxxx | y position
 *         | ------x--------- | unused ?
 *    2    | --------xxxxxxxx | x position
 *         | -------x-------- | unused ?
 *    3    | -----------xxxxx | sprite code (upper bits)
 *         | ----------x----- | sprite-tile priority
 *         | ---------x------ | flip x
 *         | --------x------- | flip y
 * 0x400   |-------------xxxx | color
 *         |---------x------- | x position (high bit)
 */

void gaiden_state::drgnbowl_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT16 *spriteram = m_spriteram;
	int i, code, color, x, y, flipx, flipy, priority_mask;

	for( i = 0; i < 0x800/2; i += 4 )
	{
		code = (spriteram[i + 0] & 0xff) | ((spriteram[i + 3] & 0x1f) << 8);
		y = 256 - (spriteram[i + 1] & 0xff) - 12;
		x = spriteram[i + 2] & 0xff;
		color = (spriteram[(0x800/2) + i] & 0x0f);
		flipx = spriteram[i + 3] & 0x40;
		flipy = spriteram[i + 3] & 0x80;

		if(spriteram[(0x800/2) + i] & 0x80)
			x -= 256;

		x += 256;

		if(spriteram[i + 3] & 0x20)
			priority_mask = 0xf0 | 0xcc; /* obscured by foreground */
		else
			priority_mask = 0;

		m_gfxdecode->gfx(3)->prio_transpen_raw(bitmap,cliprect,
				code,
				m_gfxdecode->gfx(3)->colorbase() + color * m_gfxdecode->gfx(3)->granularity(),
				flipx,flipy,x,y,
				screen.priority(), priority_mask,15);

		/* wrap x*/
		m_gfxdecode->gfx(3)->prio_transpen_raw(bitmap,cliprect,
				code,
				m_gfxdecode->gfx(3)->colorbase() + color * m_gfxdecode->gfx(3)->granularity(),
				flipx,flipy,x-512,y,
				screen.priority(), priority_mask,15);

	}
}

UINT32 gaiden_state::screen_update_gaiden(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	screen_update_raiga(screen, bitmap, cliprect);
	return 0;

}

UINT32 gaiden_state::screen_update_raiga(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_tile_bitmap_bg.fill(0, cliprect);
	m_tile_bitmap_fg.fill(0, cliprect);
	m_sprite_bitmap.fill(0, cliprect);
	bitmap.fill(0, cliprect);

	m_sprgen->gaiden_draw_sprites(screen, m_gfxdecode, cliprect, m_spriteram, m_sprite_sizey, m_spr_offset_y, flip_screen(),  m_sprite_bitmap);
	m_background->draw(screen, m_tile_bitmap_bg, cliprect, 0, 0);
	m_foreground->draw(screen, m_tile_bitmap_fg, cliprect, 0, 0);

	m_mixer->mix_bitmaps(screen, bitmap, cliprect, m_palette, &m_tile_bitmap_bg, &m_tile_bitmap_fg, (bitmap_ind16*)0, &m_sprite_bitmap);

	// todo, this should go through the mixer!
	m_text_layer->draw(screen, bitmap, cliprect, 0, 0);


	return 0;
}

UINT32 gaiden_state::screen_update_drgnbowl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);

	m_background->draw(screen, bitmap, cliprect, 0, 1);
	m_foreground->draw(screen, bitmap, cliprect, 0, 2);
	m_text_layer->draw(screen, bitmap, cliprect, 0, 4);
	drgnbowl_draw_sprites(screen, bitmap, cliprect);
	return 0;
}
