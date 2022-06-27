// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, Carlos A. Lozano, Rob Rosenbrock
/***************************************************************************

    Renegade Video Hardware

***************************************************************************/

#include "emu.h"
#include "renegade.h"


void renegade_state::bg_videoram_w(offs_t offset, uint8_t data)
{
	m_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

void renegade_state::fg_videoram_w(offs_t offset, uint8_t data)
{
	m_fg_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

void renegade_state::flipscreen_w(uint8_t data)
{
	flip_screen_set(~data & 0x01);
}

void renegade_state::scroll_lsb_w(uint8_t data)
{
	m_scrollx = (m_scrollx & 0xff00) | data;
}

void renegade_state::scroll_msb_w(uint8_t data)
{
	m_scrollx = (m_scrollx & 0xff) | (data << 8);
}

TILE_GET_INFO_MEMBER(renegade_state::get_bg_tilemap_info)
{
	const uint8_t *source = &m_bg_videoram[tile_index];
	uint8_t attributes = source[0x400]; /* CCC??BBB */
	tileinfo.set(1 + (attributes & 0x7),
		source[0],
		attributes >> 5,
		0);
}

TILE_GET_INFO_MEMBER(renegade_state::get_fg_tilemap_info)
{
	const uint8_t *source = &m_fg_videoram[tile_index];
	uint8_t attributes = source[0x400];
	tileinfo.set(0,
		((attributes & 3) << 8) | source[0],
		attributes >> 6,
		0);
}

void renegade_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(renegade_state::get_bg_tilemap_info)), TILEMAP_SCAN_ROWS, 16, 16, 64, 16);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(renegade_state::get_fg_tilemap_info)), TILEMAP_SCAN_ROWS,  8,  8, 32, 32);

	m_fg_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_scrolldx(256, 0);
	m_fg_tilemap->set_scrolldy(10, 10);
	m_bg_tilemap->set_scrolldy(10, 10);

	save_item(NAME(m_scrollx));
}

void renegade_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t *source = m_spriteram;
	uint8_t *finish = source + 96 * 4;

	while (source < finish)
	{
		// reference: stage 1 boss in kuniokun is aligned with the train door
		int sy = 234 - source[0];

		//if (sy >= 0)
		{
			int attributes = source[1]; /* SFCCBBBB */
			int sx = source[3];
			int sprite_number = source[2];
			int sprite_bank = 9 + (attributes & 0xf);
			int color = (attributes >> 4) & 0x3;
			int xflip = attributes & 0x40;

			if (sx > 248)
				sx -= 256;
			// wrap-around (stage 2 bike tires)
			if (sy < 0)
				sy += 256;

			if (flip_screen())
			{
				sx = 240 - sx;
				sy = 260 - sy;
				xflip = !xflip;
			}

			if (attributes & 0x80) /* big sprite */
			{
				m_gfxdecode->gfx(sprite_bank)->transpen(bitmap,cliprect,
					sprite_number | 1,
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

uint32_t renegade_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0, m_scrollx);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0 , 0);
	draw_sprites(bitmap, cliprect);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0 , 0);
	return 0;
}
