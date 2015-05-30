// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Space Bugger - Video Hardware */

#include "emu.h"
#include "includes/sbugger.h"

TILE_GET_INFO_MEMBER(sbugger_state::get_tile_info)
{
	int tileno, color;

	tileno = m_videoram[tile_index];
	color = m_videoram_attr[tile_index];

	SET_TILE_INFO_MEMBER(0,tileno,color,0);
}

WRITE8_MEMBER(sbugger_state::videoram_w)
{
	m_videoram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(sbugger_state::videoram_attr_w)
{
	m_videoram_attr[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}

void sbugger_state::video_start()
{
	m_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(sbugger_state::get_tile_info),this), TILEMAP_SCAN_ROWS, 8, 16, 64, 16);
}

UINT32 sbugger_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}

/* not right but so we can see things ok */
PALETTE_INIT_MEMBER(sbugger_state, sbugger)
{
	/* just some random colours for now */
	int i;

	for (i = 0;i < 256;i++)
	{
		int r = machine().rand()|0x80;
		int g = machine().rand()|0x80;
		int b = machine().rand()|0x80;
		if (i == 0) r = g = b = 0;

		palette.set_pen_color(i*2+1,rgb_t(r,g,b));
		palette.set_pen_color(i*2,rgb_t(0,0,0));

	}

}
