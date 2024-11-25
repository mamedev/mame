// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/***************************************************************************

    Ninja Gaiden / Tecmo Knights Video Hardware

***************************************************************************/

#include "emu.h"
#include "gaiden.h"

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(gaiden_state::get_bg_tile_info)
{
	uint16_t const *const videoram1 = &m_videoram[2][0x0800];
	uint16_t const *const videoram2 = m_videoram[2];
	tileinfo.set(1,
			videoram1[tile_index] & 0x0fff,
			(videoram2[tile_index] & 0xf0) >> 4,
			0);
}

TILE_GET_INFO_MEMBER(gaiden_state::get_fg_tile_info)
{
	uint16_t const *const videoram1 = &m_videoram[1][0x0800];
	uint16_t const *const videoram2 = m_videoram[1];
	tileinfo.set(2,
			videoram1[tile_index] & 0x0fff,
			(videoram2[tile_index] & 0xf0) >> 4,
			0);
}

TILE_GET_INFO_MEMBER(gaiden_state::get_fg_tile_info_raiga)
{
	uint16_t const *const videoram1 = &m_videoram[1][0x0800];
	uint16_t const *const videoram2 = m_videoram[1];

	uint32_t colour = ((videoram2[tile_index] & 0xf0) >> 4);

	// bit 3 controls blending
	if ((videoram2[tile_index] & 0x08))
		colour += 0x10;

	tileinfo.set(2,
			videoram1[tile_index] & 0x0fff,
			colour,
			0);
}

TILE_GET_INFO_MEMBER(gaiden_state::get_tx_tile_info)
{
	uint16_t const *const videoram1 = &m_videoram[0][0x0400];
	uint16_t const *const videoram2 = m_videoram[0];
	tileinfo.set(0,
			videoram1[tile_index] & 0x07ff,
			(videoram2[tile_index] & 0xf0) >> 4,
			0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START_MEMBER(gaiden_state,gaiden)
{
	// set up tile layers
	m_screen->register_screen_bitmap(m_tile_bitmap_bg);
	m_screen->register_screen_bitmap(m_tile_bitmap_fg);
	m_screen->register_screen_bitmap(m_tile_bitmap_tx);

	m_background = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gaiden_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
	m_foreground = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gaiden_state::get_fg_tile_info_raiga)), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
	m_text_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gaiden_state::get_tx_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

//  m_background->set_transparent_pen(0);
//  m_foreground->set_transparent_pen(0);
	m_text_layer->set_transparent_pen(0);

	m_background->set_scrolldy(0, 33);
	m_foreground->set_scrolldy(0, 33);
	m_text_layer->set_scrolldy(0, 33);

	// set up sprites
	m_screen->register_screen_bitmap(m_sprite_bitmap);
}


void raiga_state::video_start()
{
	// set up tile layers
	m_screen->register_screen_bitmap(m_tile_bitmap_bg);
	m_screen->register_screen_bitmap(m_tile_bitmap_fg);
	m_screen->register_screen_bitmap(m_tile_bitmap_tx);

	m_background = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(raiga_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
	m_foreground = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(raiga_state::get_fg_tile_info_raiga)), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
	m_text_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(raiga_state::get_tx_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

//  m_background->set_transparent_pen(0);
//  m_foreground->set_transparent_pen(0);
	m_text_layer->set_transparent_pen(0);

	// set up sprites
	m_screen->register_screen_bitmap(m_sprite_bitmap);

}

VIDEO_START_MEMBER(gaiden_state,drgnbowl)
{
	// set up tile layers
	m_background = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gaiden_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
	m_foreground = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gaiden_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
	m_text_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gaiden_state::get_tx_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_foreground->set_transparent_pen(15);
	m_text_layer->set_transparent_pen(15);

	m_background->set_scrolldx(-248, 248);
	m_foreground->set_scrolldx(-252, 252);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

void gaiden_state::flip_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
		flip_screen_set(BIT(data, 0));
}

void gaiden_state::txscrollx_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_tx_scroll_x);
	m_text_layer->set_scrollx(0, m_tx_scroll_x);
}

void gaiden_state::txscrolly_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_tx_scroll_y);
	m_text_layer->set_scrolly(0, (m_tx_scroll_y - m_tx_offset_y) & 0xffff);
}

void gaiden_state::fgscrollx_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_fg_scroll_x);
	m_foreground->set_scrollx(0, m_fg_scroll_x);
}

void gaiden_state::fgscrolly_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_fg_scroll_y);
	m_foreground->set_scrolly(0, (m_fg_scroll_y - m_fg_offset_y) & 0xffff);
}

void gaiden_state::bgscrollx_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bg_scroll_x);
	m_background->set_scrollx(0, m_bg_scroll_x);
}

void gaiden_state::bgscrolly_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bg_scroll_y);
	m_background->set_scrolly(0, (m_bg_scroll_y - m_bg_offset_y) & 0xffff);
}

void gaiden_state::txoffsety_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_tx_offset_y = data;
		m_text_layer->set_scrolly(0, (m_tx_scroll_y - m_tx_offset_y) & 0xffff);
	}
}

void gaiden_state::fgoffsety_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_fg_offset_y = data;
		m_foreground->set_scrolly(0, (m_fg_scroll_y - m_fg_offset_y) & 0xffff);
	}
}

void gaiden_state::bgoffsety_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_bg_offset_y = data;
		m_background->set_scrolly(0, (m_bg_scroll_y - m_bg_offset_y) & 0xffff);
	}
}

void gaiden_state::sproffsety_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		m_spr_offset_y = data;
		// handled in draw_sprites
	}
}


void gaiden_state::bg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_videoram[2][offset]);
	m_background->mark_tile_dirty(offset & 0x07ff);
}

void gaiden_state::fg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_videoram[1][offset]);
	m_foreground->mark_tile_dirty(offset & 0x07ff);
}

void gaiden_state::tx_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_videoram[0][offset]);
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
	uint16_t const *const spriteram = m_spriteram->live(); // not buffered?
	uint32_t priority_mask;

	for (int i = 0; i < 0x800 / 2; i += 4)
	{
		uint32_t const code = (spriteram[i + 0] & 0xff) | ((spriteram[i + 3] & 0x1f) << 8);
		int const y = 256 - (spriteram[i + 1] & 0xff) - 12;
		int x = spriteram[i + 2] & 0xff;
		uint32_t const color = (spriteram[(0x800/2) + i] & 0x0f);
		bool const flipx = BIT(spriteram[i + 3], 6);
		bool const flipy = BIT(spriteram[i + 3], 7);

		if (BIT(spriteram[(0x800 / 2) + i], 7))
			x -= 256;

		x += 256;

		if(spriteram[i + 3] & 0x20)
			priority_mask = 0xf0 | 0xcc; // obscured by foreground
		else
			priority_mask = 0;

		m_gfxdecode->gfx(3)->prio_transpen_raw(bitmap, cliprect,
				code,
				m_gfxdecode->gfx(3)->colorbase() + color * m_gfxdecode->gfx(3)->granularity(),
				flipx, flipy, x, y,
				screen.priority(), priority_mask, 15);

		// wrap x
		m_gfxdecode->gfx(3)->prio_transpen_raw(bitmap, cliprect,
				code,
				m_gfxdecode->gfx(3)->colorbase() + color * m_gfxdecode->gfx(3)->granularity(),
				flipx, flipy, x - 512, y,
				screen.priority(), priority_mask, 15);

	}
}

uint32_t gaiden_state::screen_update_gaiden(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_tile_bitmap_bg.fill(0, cliprect);
	m_tile_bitmap_fg.fill(0, cliprect);
	m_tile_bitmap_tx.fill(0, cliprect);
	m_sprite_bitmap.fill(0, cliprect);
	bitmap.fill(0, cliprect);

	// non buffered?
	m_sprgen->gaiden_draw_sprites(screen, m_sprite_bitmap, cliprect, m_spriteram->live(), m_sprite_sizey, flip_screen() ? -m_spr_offset_y : m_spr_offset_y, flip_screen());
	m_background->draw(screen, m_tile_bitmap_bg, cliprect, 0, 0);
	m_foreground->draw(screen, m_tile_bitmap_fg, cliprect, 0, 0);
	m_text_layer->draw(screen, m_tile_bitmap_tx, cliprect, 0, 0);

	m_mixer->mix_bitmaps(screen, bitmap, cliprect, *m_palette, &m_tile_bitmap_bg, &m_tile_bitmap_fg, &m_tile_bitmap_tx, &m_sprite_bitmap);

	return 0;
}

uint32_t raiga_state::screen_update_raiga(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_tile_bitmap_bg.fill(0, cliprect);
	m_tile_bitmap_fg.fill(0, cliprect);
	m_tile_bitmap_tx.fill(0, cliprect);
	bitmap.fill(0, cliprect);

	m_background->draw(screen, m_tile_bitmap_bg, cliprect, 0, 0);
	m_foreground->draw(screen, m_tile_bitmap_fg, cliprect, 0, 0);
	m_text_layer->draw(screen, m_tile_bitmap_tx, cliprect, 0, 0);

	m_mixer->mix_bitmaps(screen, bitmap, cliprect, *m_palette, &m_tile_bitmap_bg, &m_tile_bitmap_fg, &m_tile_bitmap_tx, &m_sprite_bitmap);

	return 0;
}

uint32_t gaiden_state::screen_update_drgnbowl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);

	m_background->draw(screen, bitmap, cliprect, 0, 1);
	m_foreground->draw(screen, bitmap, cliprect, 0, 2);
	m_text_layer->draw(screen, bitmap, cliprect, 0, 4);
	drgnbowl_draw_sprites(screen, bitmap, cliprect);
	return 0;
}

void raiga_state::screen_vblank_raiga(int state)
{
	if (state)
	{
		const rectangle visarea = m_screen->visible_area();
		// raiga sprite has 2 frame lags
		m_sprite_bitmap.fill(0, visarea);
		m_sprgen->gaiden_draw_sprites(*m_screen, m_sprite_bitmap, visarea, m_spriteram->buffer(), m_sprite_sizey, flip_screen() ? -m_spr_offset_y : m_spr_offset_y, flip_screen());

		m_spriteram->copy();
	}
}
