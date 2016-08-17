// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, Carlos A. Lozano, Rob Rosenbrock
/***************************************************************************

    Renegade Video Hardware

***************************************************************************/

#include "emu.h"
#include "includes/renegade.h"


WRITE8_MEMBER(renegade_state::bg_videoram_w)
{
	m_bg_videoram[offset] = data;
	offset = offset % (64 * 16);
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(renegade_state::fg_videoram_w)
{
	m_fg_videoram[offset] = data;
	offset = offset % (32 * 32);
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(renegade_state::flipscreen_w)
{
	flip_screen_set(~data & 0x01);
}

WRITE8_MEMBER(renegade_state::scroll_lsb_w)
{
	m_scrollx = (m_scrollx & 0xff00) | data;
}

WRITE8_MEMBER(renegade_state::scroll_msb_w)
{
	m_scrollx = (m_scrollx & 0xff) | (data << 8);
}

TILE_GET_INFO_MEMBER(renegade_state::get_bg_tilemap_info)
{
	const UINT8 *source = &m_bg_videoram[tile_index];
	UINT8 attributes = source[0x400]; /* CCC??BBB */
	SET_TILE_INFO_MEMBER(1 + (attributes & 0x7),
		source[0],
		attributes >> 5,
		0);
}

TILE_GET_INFO_MEMBER(renegade_state::get_fg_tilemap_info)
{
	const UINT8 *source = &m_fg_videoram[tile_index];
	UINT8 attributes = source[0x400];
	SET_TILE_INFO_MEMBER(0,
		(attributes & 3) * 256 + source[0],
		attributes >> 6,
		0);
}

void renegade_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(renegade_state::get_bg_tilemap_info),this), TILEMAP_SCAN_ROWS,      16, 16, 64, 16);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(renegade_state::get_fg_tilemap_info),this), TILEMAP_SCAN_ROWS,   8, 8, 32, 32);

	m_fg_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_scrolldx(256, 0);

	save_item(NAME(m_scrollx));
}

void renegade_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 *source = m_spriteram;
	UINT8 *finish = source + 96 * 4;

	while (source < finish)
	{
		int sy = 240 - source[0];

		if (sy >= 16)
		{
			int attributes = source[1]; /* SFCCBBBB */
			int sx = source[3];
			int sprite_number = source[2];
			int sprite_bank = 9 + (attributes & 0xf);
			int color = (attributes >> 4) & 0x3;
			int xflip = attributes & 0x40;

			if (sx > 248)
				sx -= 256;

			if (flip_screen())
			{
				sx = 240 - sx;
				sy = 240 - sy;
				xflip = !xflip;
			}

			if (attributes & 0x80) /* big sprite */
			{
				sprite_number &= ~1;
				m_gfxdecode->gfx(sprite_bank)->transpen(bitmap,cliprect,
					sprite_number + 1,
					color,
					xflip, flip_screen(),
					sx, sy + (flip_screen() ? -16 : 16), 0);
			}
			else
			{
				sy += (flip_screen() ? -16 : 16);
			}
			m_gfxdecode->gfx(sprite_bank)->transpen(bitmap,cliprect,
				sprite_number,
				color,
				xflip, flip_screen(),
				sx, sy, 0);
		}
		source += 4;
	}
}

UINT32 renegade_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0, m_scrollx);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0 , 0);
	draw_sprites(bitmap, cliprect);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0 , 0);
	return 0;
}
