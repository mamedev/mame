// license: BSD-3-Clause
// copyright-holders: Bryan McPhail, David Haywood, Dirk Best
/***************************************************************************

    Super Burger Time

    Video mixing

***************************************************************************/

#include "emu.h"
#include "includes/supbtime.h"

// End sequence uses rowscroll '98 c0' on pf1 (jmp to 1d61a on supbtimej)
uint32_t supbtime_state::screen_update_supbtime(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	address_space &space = machine().dummy_space();
	uint16_t flip = m_deco_tilegen->pf_control_r(space, 0, 0xffff);

	flip_screen_set(BIT(flip, 7));
	m_sprgen->set_flip_screen(BIT(flip, 7));
	m_deco_tilegen->pf_update(m_pf_rowscroll[0], m_pf_rowscroll[1]);

	bitmap.fill(768, cliprect);

	m_deco_tilegen->tilemap_2_draw(screen, bitmap, cliprect, 0, 0);
	m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram, 0x400);
	m_deco_tilegen->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

// Tumblepop is one of few games to take advantage of the playfields ability
// to switch between 8*8 tiles and 16*16 tiles.
uint32_t supbtime_state::screen_update_tumblep(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	address_space &space = machine().dummy_space();
	uint16_t flip = m_deco_tilegen->pf_control_r(space, 0, 0xffff);

	flip_screen_set(BIT(flip, 7));
	m_sprgen->set_flip_screen(BIT(flip, 7));
	m_deco_tilegen->pf_update(m_pf_rowscroll[0], m_pf_rowscroll[1]);

	bitmap.fill(256+512, cliprect); // not verified

	m_deco_tilegen->tilemap_2_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	m_deco_tilegen->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
	m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram, 0x400);

	return 0;
}

