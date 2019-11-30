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
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(sbugger_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 16, 64, 16);
}

uint32_t sbugger_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}

// not right but so we can see things OK
void sbugger_state::sbugger_palette(palette_device &palette) const
{
	// just some random colours for now
	for (int i = 0; i < 256; i++)
	{
		int const r = i ? (machine().rand() | 0x80) : 0;
		int const g = i ? (machine().rand() | 0x80) : 0;
		int const b = i ? (machine().rand() | 0x80) : 0;

		palette.set_pen_color(i*2+1, rgb_t(r, g, b));
		palette.set_pen_color(i*2, rgb_t(0, 0, 0));
	}
}
