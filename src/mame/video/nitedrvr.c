// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/***************************************************************************

    Atari Night Driver hardware

***************************************************************************/

#include "emu.h"
#include "includes/nitedrvr.h"

WRITE8_MEMBER(nitedrvr_state::nitedrvr_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


TILE_GET_INFO_MEMBER(nitedrvr_state::get_bg_tile_info)
{
	int code = m_videoram[tile_index] & 0x3f;

	SET_TILE_INFO_MEMBER(0, code, 0, 0);
}

void nitedrvr_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(nitedrvr_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 4);
}

void nitedrvr_state::draw_box(bitmap_ind16 &bitmap, const rectangle &cliprect, int bx, int by, int ex, int ey)
{
	for (int y = by; y < ey; y++)
	{
		for (int x = bx; x < ex; x++)
			if (cliprect.contains(x, y))
				bitmap.pix16(y, x) = 1;
	}
}

void nitedrvr_state::draw_roadway(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int roadway = 0; roadway < 16; roadway++)
	{
		int bx = m_hvc[roadway];
		int by = m_hvc[roadway + 16];
		int ex = bx + ((m_hvc[roadway + 32] & 0xf0) >> 4);
		int ey = by + (16 - (m_hvc[roadway + 32] & 0x0f));

		draw_box(bitmap, cliprect, bx, by, ex, ey);
	}
}

UINT32 nitedrvr_state::screen_update_nitedrvr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	// don't wrap playfield
	rectangle clip = cliprect;
	if (clip.max_y > 31) clip.max_y = 31;

	m_bg_tilemap->draw(screen, bitmap, clip, 0, 0);
	draw_roadway(bitmap, cliprect);

	return 0;
}
