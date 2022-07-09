// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Simple 156 based board

*/

#include "emu.h"
#include "simpl156.h"
#include "screen.h"


void simpl156_state::video_start()
{
	/* allocate the ram as 16-bit (we do it here because the CPU is 32-bit) */
	m_rowscroll[0] = make_unique_clear<u16[]>(0x800/2);
	m_rowscroll[1] = make_unique_clear<u16[]>(0x800/2);
	m_spriteram = make_unique_clear<u16[]>(0x2000/2);

	memset(m_spriteram.get(), 0xff, 0x2000);

	/* and register the allocated ram so that save states still work */
	save_pointer(NAME(m_rowscroll[0]), 0x800/2);
	save_pointer(NAME(m_rowscroll[1]), 0x800/2);
	save_pointer(NAME(m_spriteram), 0x2000/2);
}

u32 simpl156_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0);

	m_deco_tilegen->pf_update(m_rowscroll[0].get(), m_rowscroll[1].get());

	bitmap.fill(256, cliprect);

	m_deco_tilegen->tilemap_2_draw(screen, bitmap, cliprect, 0, 2);
	m_deco_tilegen->tilemap_1_draw(screen, bitmap, cliprect, 0, 4);

	// sprites are flipped relative to tilemaps
	m_sprgen->set_flip_screen(true);

	m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram.get(), 0x1400/4); // 0x1400/4 seems right for charlien (doesn't initialize any more RAM, so will draw a garbage 0 with more)
	return 0;
}
