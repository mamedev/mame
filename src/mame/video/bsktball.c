// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/***************************************************************************

    Atari Basketball hardware

***************************************************************************/

#include "emu.h"
#include "includes/bsktball.h"


WRITE8_MEMBER(bsktball_state::bsktball_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(bsktball_state::get_bg_tile_info)
{
	int attr = m_videoram[tile_index];
	int code = ((attr & 0x0f) << 2) | ((attr & 0x30) >> 4);
	int color = (attr & 0x40) >> 6;
	int flags = (attr & 0x80) ? TILE_FLIPX : 0;

	SET_TILE_INFO_MEMBER(0, code, color, flags);
}

void bsktball_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(bsktball_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

void bsktball_state::draw_sprites(  bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int mot;

	for (mot = 0; mot < 16; mot++)
	{
		int pic = m_motion[mot * 4];
		int sy = 28 * 8 - m_motion[mot * 4 + 1];
		int sx = m_motion[mot * 4 + 2];
		int color = m_motion[mot * 4 + 3];
		int flipx = (pic & 0x80) >> 7;

		pic = (pic & 0x3f);
		color = (color & 0x3f);

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect, pic, color, flipx, 0, sx, sy, 0);
	}
}

UINT32 bsktball_state::screen_update_bsktball(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}
