// license:BSD-3-Clause
// copyright-holders:David Haywood, Nicola Salmoria
#include "emu.h"
#include "includes/usgames.h"


PALETTE_INIT_MEMBER(usgames_state, usgames)
{
	int j;

	for (j = 0; j < 0x200; j++)
	{
		int data;
		int r, g, b, i;

		if (j & 0x01)
			data = (j >> 5) & 0x0f;
		else
			data = (j >> 1) & 0x0f;

		r = (data & 1) >> 0;
		g = (data & 2) >> 1;
		b = (data & 4) >> 2;
		i = (data & 8) >> 3;

		r = 0xff * r;
		g = 0x7f * g * (i + 1);
		b = 0x7f * b * (i + 1);

		palette.set_pen_color(j,rgb_t(r, g, b));
	}
}



TILE_GET_INFO_MEMBER(usgames_state::get_tile_info)
{
	int tileno = m_videoram[tile_index*2];
	int colour = m_videoram[tile_index*2+1];

	SET_TILE_INFO_MEMBER(0,tileno,colour,0);
}

void usgames_state::video_start()
{
	m_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(usgames_state::get_tile_info),this),TILEMAP_SCAN_ROWS, 8, 8,64,32);
	m_gfxdecode->gfx(0)->set_source(m_charram);
}


WRITE8_MEMBER(usgames_state::videoram_w)
{
	m_videoram[offset] = data;
	m_tilemap->mark_tile_dirty(offset/2);
}

WRITE8_MEMBER(usgames_state::charram_w)
{
	m_charram[offset] = data;
	m_gfxdecode->gfx(0)->mark_dirty(offset/8);
}


UINT32 usgames_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}
