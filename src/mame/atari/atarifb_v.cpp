// license:BSD-3-Clause
// copyright-holders:Mike Balfour, Patrick Lawrence, Brad Oliver
/*************************************************************************

    Atari Football hardware

*************************************************************************/

#include "emu.h"
#include "atarifb.h"


/*************************************
 *
 *  Palette generation
 *
 *************************************/

void atarifb_state::atarifb_palette(palette_device &palette) const
{
	// chars
	palette.set_pen_color(0, rgb_t(0xff,0xff,0xff));
	palette.set_pen_color(1, rgb_t(0x00,0x00,0x00));

	// field
	palette.set_pen_color(2, rgb_t(0x40,0x40,0x40));
	palette.set_pen_color(3, rgb_t(0xc0,0xc0,0xc0));

	// sprites
	palette.set_pen_color(4, rgb_t(0x40,0x40,0x40));
	palette.set_pen_color(5, rgb_t(0xff,0xff,0xff));
	palette.set_pen_color(6, rgb_t(0x40,0x40,0x40));
	palette.set_pen_color(7, rgb_t(0x00,0x00,0x00));

	// sprite masks
	palette.set_pen_color(8, rgb_t(0x40,0x40,0x40));
	palette.set_pen_color(9, rgb_t(0x80,0x80,0x80));
	palette.set_pen_color(10, rgb_t(0x40,0x40,0x40));
	palette.set_pen_color(11, rgb_t(0x00,0x00,0x00));
	palette.set_pen_color(12, rgb_t(0x40,0x40,0x40));
	palette.set_pen_color(13, rgb_t(0xff,0xff,0xff));
}



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

void atarifb_state::get_tile_info_common( tile_data &tileinfo, tilemap_memory_index tile_index, uint8_t *alpha_videoram )
{
	int code = alpha_videoram[tile_index] & 0x3f;
	int flip = alpha_videoram[tile_index] & 0x40;
	int disable = alpha_videoram[tile_index] & 0x80;

	if (disable)
		code = 0;   /* I *know* this is a space */

	tileinfo.set(0, code, 0, (flip ? TILE_FLIPX | TILE_FLIPY : 0));
}


TILE_GET_INFO_MEMBER(atarifb_state::alpha1_get_tile_info)
{
	get_tile_info_common(tileinfo, tile_index, m_alphap1_videoram);
}


TILE_GET_INFO_MEMBER(atarifb_state::alpha2_get_tile_info)
{
	get_tile_info_common(tileinfo, tile_index, m_alphap2_videoram);
}


TILE_GET_INFO_MEMBER(atarifb_state::field_get_tile_info)
{
	int code = m_field_videoram[tile_index] & 0x3f;
	int flipyx = m_field_videoram[tile_index] >> 6;

	tileinfo.set(1, code, 0, TILE_FLIPYX(flipyx));
}



/*************************************
 *
 *  Video RAM writes
 *
 *************************************/

void atarifb_state::atarifb_alpha1_videoram_w(offs_t offset, uint8_t data)
{
	m_alphap1_videoram[offset] = data;
	m_alpha1_tilemap->mark_tile_dirty(offset);
}


void atarifb_state::atarifb_alpha2_videoram_w(offs_t offset, uint8_t data)
{
	m_alphap2_videoram[offset] = data;
	m_alpha2_tilemap->mark_tile_dirty(offset);
}


void atarifb_state::atarifb_field_videoram_w(offs_t offset, uint8_t data)
{
	m_field_videoram[offset] = data;
	m_field_tilemap->mark_tile_dirty(offset);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

void atarifb_state::video_start()
{
	m_alpha1_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(atarifb_state::alpha1_get_tile_info)), TILEMAP_SCAN_COLS, 8, 8, 3, 32);
	m_alpha2_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(atarifb_state::alpha2_get_tile_info)), TILEMAP_SCAN_COLS, 8, 8, 3, 32);
	m_field_tilemap  = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(atarifb_state::field_get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}


void atarifb_state::draw_playfield_and_alpha( bitmap_ind16 &bitmap, const rectangle &cliprect, int playfield_x_offset, int playfield_y_offset )
{
	bitmap.fill(1, cliprect);
	rectangle clip(4 * 8, 34 * 8 - 1, 0 * 8, 32 * 8 - 1);
	clip &= cliprect;

	int scroll_x[1];
	int scroll_y[1];

	scroll_x[0] = - *m_scroll_register + 32 + playfield_x_offset;
	scroll_y[0] = 8 + playfield_y_offset;

	copybitmap(bitmap, m_alpha1_tilemap->pixmap(), 0, 0, 35*8, 1*8, cliprect);
	copybitmap(bitmap, m_alpha2_tilemap->pixmap(), 0, 0,  0*8, 1*8, cliprect);
	copyscrollbitmap(bitmap, m_field_tilemap->pixmap(),  1, scroll_x, 1, scroll_y, clip);
}


void atarifb_state::draw_sprites_atarifb( bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	rectangle clip(4 * 8, 34 * 8 - 1, 0 * 8, 32 * 8 - 1);
	clip &= cliprect;

	for (int obj = 0; obj < 16; obj++)
	{
		int charcode = m_spriteram[obj * 2] & 0x3f;
		int flipx = (m_spriteram[obj * 2] & 0x40);
		int flipy = (m_spriteram[obj * 2] & 0x80);
		int sx = m_spriteram[obj * 2 + 0x20] + 8 * 3;
		int sy = 256 - m_spriteram[obj * 2 + 1];

		m_gfxdecode->gfx(1)->transpen(bitmap, clip, charcode, 1, flipx, flipy, sx, sy, 0);
	}
}

void atarifb_state::draw_sprites_soccer( bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	rectangle clip(4 * 8, 34 * 8 - 1, 0 * 8, 32 * 8 - 1);
	clip &= cliprect;

	for (int obj = 0; obj < 16; obj++)
	{
		int charcode = m_spriteram[obj * 2] & 0x3f;
		int flipx = (m_spriteram[obj * 2] & 0x40);
		int flipy = (m_spriteram[obj * 2] & 0x80);
		int sx = m_spriteram[obj * 2 + 0x20] + 8 * 3;
		int sy = 256 - m_spriteram[obj * 2 + 1] - 8;

		/* There are 3 sets of 2 bits each, where the 2 bits represent */
		/* black, dk grey, grey and white. I think the 3 sets determine the */
		/* color of each bit in the sprite, but I haven't implemented it that way. */
		int shade = m_spriteram[obj * 2 + 1 + 0x20];

		m_gfxdecode->gfx(3)->transpen(bitmap, clip, charcode, shade & 7, flipx, flipy, sx, sy, 0);
		m_gfxdecode->gfx(2)->transpen(bitmap, clip, charcode, shade >> 3 & 1, flipx, flipy, sx, sy, 0);
	}
}


uint32_t atarifb_state::screen_update_atarifb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	draw_playfield_and_alpha(bitmap, cliprect, 0, 0);
	draw_sprites_atarifb(bitmap, cliprect);
	return 0;
}

uint32_t atarifb_state::screen_update_abaseb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	draw_playfield_and_alpha(bitmap, cliprect, -8, 0);
	draw_sprites_atarifb(bitmap, cliprect);
	return 0;
}

uint32_t atarifb_state::screen_update_soccer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	draw_playfield_and_alpha(bitmap, cliprect, 0, 0);
	draw_sprites_soccer(bitmap, cliprect);
	return 0;
}
