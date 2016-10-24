// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Space Bugger - Video Hardware */

#include "emu.h"
#include "includes/sbugger.h"

void sbugger_state::get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index)
{
	int tileno, color;

	tileno = m_videoram[tile_index];
	color = m_videoram_attr[tile_index];

	SET_TILE_INFO_MEMBER(0,tileno,color,0);
}

void sbugger_state::videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_videoram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}

void sbugger_state::videoram_attr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_videoram_attr[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}

void sbugger_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(sbugger_state::get_tile_info),this), TILEMAP_SCAN_ROWS, 8, 16, 64, 16);
}

uint32_t sbugger_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}

/* not right but so we can see things ok */
void sbugger_state::palette_init_sbugger(palette_device &palette)
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
