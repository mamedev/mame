// license:BSD-3-Clause
// copyright-holders:Uki
/******************************************************************************

Himeshikibu (C) 1989 Hi-Soft

Video hardware
    driver by Uki

******************************************************************************/

#include "emu.h"
#include "includes/himesiki.h"

TILE_GET_INFO_MEMBER(himesiki_state::get_bg_tile_info)
{
	int code = m_bg_ram[tile_index * 2] + m_bg_ram[tile_index * 2 + 1] * 0x100 ;
	int col = code >> 12;

	code &= 0xfff;

	SET_TILE_INFO_MEMBER(0, code, col, 0);
}

void himesiki_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(himesiki_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
}

WRITE8_MEMBER(himesiki_state::himesiki_bg_ram_w)
{
	m_bg_ram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

WRITE8_MEMBER(himesiki_state::himesiki_scrollx_w)
{
	m_scrollx[offset] = data;
}

WRITE8_MEMBER(himesiki_state::himesiki_flip_w)
{
	m_flipscreen = data & 0xc0;
	flip_screen_set(m_flipscreen);

	if (data & 0x3f)
		logerror("p08_w %02x\n",data);
}

void himesiki_state::himesiki_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	UINT8 *spriteram = m_spriteram;
	int offs;

	for (offs = 0x100; offs < 0x160; offs += 4)
	{
		int attr = spriteram[offs + 1];
		int code = spriteram[offs + 0] | (attr & 3) << 8;
		int x = spriteram[offs + 3] | (attr & 8) << 5;
		int y = spriteram[offs + 2];

		int col = (attr & 0xf0) >> 4;
		int fx = attr & 4;
		int fy = 0;

		if (x > 0x1e0)
			x -= 0x200;

		if (m_flipscreen)
		{
			y = (y + 33) & 0xff;
			x = 224 - x;
			fx ^= 4;
			fy = 1;
		}
		else
		{
			y = 257 - y;
			if (y > 0xc0)
				y -= 0x100;
		}

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect, code, col, fx, fy, x, y, 15);
	}

	for (offs = 0; offs < 0x100; offs += 4)
	{
		int attr = spriteram[offs + 1];
		int code = spriteram[offs + 0] | (attr & 7) << 8;
		int x = spriteram[offs + 3] | (attr & 8) << 5;
		int y = spriteram[offs + 2];

		int col = (attr & 0xf0) >> 4;
		int f = 0;

		if (x > 0x1e0)
			x -= 0x200;

		if (m_flipscreen)
		{
			y += 49;
			x = 240 - x;
			f = 1;
		}
		else
			y = 257 - y;

		y &= 0xff;
		if (y > 0xf0)
			y -= 0x100;

		m_gfxdecode->gfx(2)->transpen(bitmap,cliprect, code, col, f, f, x, y, 15);
	}
}

UINT32 himesiki_state::screen_update_himesiki(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x = -(m_scrollx[0] << 8 | m_scrollx[1]) & 0x1ff;
	m_bg_tilemap->set_scrolldx(x, x);

	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	himesiki_draw_sprites(bitmap, cliprect);

	return 0;
}
