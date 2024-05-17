// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Simple 156 based board

*/

#include "emu.h"

#include "simpl156.h"

#include "screen.h"

#include <algorithm>


void simpl156_state::video_start()
{
	std::fill_n(&m_spriteram[0], m_spriteram.length(), 0xffff);
}

u32 simpl156_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0);

	m_deco_tilegen->pf_update(m_rowscroll[0], m_rowscroll[1]);

	bitmap.fill(256, cliprect);

	m_deco_tilegen->tilemap_2_draw(screen, bitmap, cliprect, 0, 2);
	m_deco_tilegen->tilemap_1_draw(screen, bitmap, cliprect, 0, 4);

	// sprites are flipped relative to tilemaps
	m_sprgen->set_flip_screen(true);

	m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram, 0x1400/4); // 0x1400/4 seems right for charlien (doesn't initialize any more RAM, so will draw a garbage 0 with more)
	return 0;
}
