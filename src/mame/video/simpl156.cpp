// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Simple 156 based board

*/

#include "emu.h"
#include "includes/simpl156.h"



void simpl156_state::video_start()
{
	/* allocate the ram as 16-bit (we do it here because the CPU is 32-bit) */
	m_pf1_rowscroll = make_unique_clear<UINT16[]>(0x800/2);
	m_pf2_rowscroll = make_unique_clear<UINT16[]>(0x800/2);
	m_spriteram = make_unique_clear<UINT16[]>(0x2000/2);

	memset(m_spriteram.get(), 0xff, 0x2000);

	/* and register the allocated ram so that save states still work */
	save_pointer(NAME(m_pf1_rowscroll.get()), 0x800/2);
	save_pointer(NAME(m_pf2_rowscroll.get()), 0x800/2);
	save_pointer(NAME(m_spriteram.get()), 0x2000/2);
}

UINT32 simpl156_state::screen_update_simpl156(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0);

	m_deco_tilegen1->pf_update(m_pf1_rowscroll.get(), m_pf2_rowscroll.get());

	bitmap.fill(256, cliprect);

	m_deco_tilegen1->tilemap_2_draw(screen, bitmap, cliprect, 0, 2);
	m_deco_tilegen1->tilemap_1_draw(screen, bitmap, cliprect, 0, 4);

	//FIXME: flip_screen_x should not be written!
	flip_screen_set_no_update(1);

	m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram.get(), 0x1400/4); // 0x1400/4 seems right for charlien (doesn't initialize any more RAM, so will draw a garbage 0 with more)
	return 0;
}
