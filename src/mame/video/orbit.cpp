// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
/***************************************************************************

Atari Orbit video emulation

***************************************************************************/

#include "emu.h"
#include "includes/orbit.h"

WRITE8_MEMBER(orbit_state::orbit_playfield_w)
{
	m_playfield_ram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


TILE_GET_INFO_MEMBER(orbit_state::get_tile_info)
{
	UINT8 code = m_playfield_ram[tile_index];
	int flags = 0;

	if (BIT(code, 6))
		flags |= TILE_FLIPX;
	if (m_flip_screen)
		flags |= TILE_FLIPY;

	SET_TILE_INFO_MEMBER(3, code & 0x3f, 0, flags);
}


void orbit_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(orbit_state::get_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 30);
}


void orbit_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	const UINT8* p = m_sprite_ram;

	int i;

	for (i = 0; i < 16; i++)
	{
		int code = *p++;
		int vpos = *p++;
		int hpos = *p++;
		int flag = *p++;

		int layout =
			((flag & 0xc0) == 0x80) ? 1 :
			((flag & 0xc0) == 0xc0) ? 2 : 0;

		int flip_x = BIT(code, 6);
		int flip_y = BIT(code, 7);

		int zoom_x = 0x10000;
		int zoom_y = 0x10000;

		code &= 0x3f;

		if (flag & 1)
			code |= 0x40;
		if (flag & 2)
			zoom_x *= 2;

		vpos = 240 - vpos;

		hpos <<= 1;
		vpos <<= 1;

		m_gfxdecode->gfx(layout)->zoom_transpen(bitmap,cliprect, code, 0, flip_x, flip_y,
			hpos, vpos, zoom_x, zoom_y, 0);
	}
}


UINT32 orbit_state::screen_update_orbit(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_flip_screen = ioport("DSW2")->read() & 8;

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	draw_sprites(bitmap, cliprect);
	return 0;
}
