// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Poke Champ */

#include "emu.h"
#include "includes/pokechmp.h"


WRITE8_MEMBER(pokechmp_state::pokechmp_videoram_w)
{
	UINT8 *videoram = m_videoram;
	videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

WRITE8_MEMBER(pokechmp_state::pokechmp_flipscreen_w)
{
	if (flip_screen() != (data & 0x80))
	{
		flip_screen_set(data & 0x80);
		machine().tilemap().mark_all_dirty();
	}
}

TILE_GET_INFO_MEMBER(pokechmp_state::get_bg_tile_info)
{
	UINT8 *videoram = m_videoram;
	int code = videoram[tile_index*2+1] + ((videoram[tile_index*2] & 0x3f) << 8);
	int color = videoram[tile_index*2] >> 6;

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

void pokechmp_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(pokechmp_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS,
			8, 8, 32, 32);
}

void pokechmp_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 *spriteram = m_spriteram;
	int offs;

	for (offs = 0;offs < m_spriteram.bytes();offs += 4)
	{
		if (spriteram[offs] != 0xf8)
		{
			int sx,sy,flipx,flipy;


			sx = 240 - spriteram[offs+2];
			sy = 240 - spriteram[offs];

			flipx = spriteram[offs+1] & 0x04;
			flipy = spriteram[offs+1] & 0x02;
			if (flip_screen()) {
				sx=240-sx;
				sy=240-sy;
				if (flipx) flipx=0; else flipx=1;
				if (flipy) flipy=0; else flipy=1;
			}
			int tileno = spriteram[offs+3];
			if (spriteram[offs+1] & 0x01) tileno += 0x100;
			if (spriteram[offs+1] & 0x08) tileno += 0x200;

			m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
					tileno,
					(spriteram[offs+1] & 0xf0) >> 4,
					flipx,flipy,
					sx,sy,0);
		}
	}
}

UINT32 pokechmp_state::screen_update_pokechmp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}
