// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Haywood
#include "emu.h"
#include "includes/dietgo.h"

uint32_t dietgo_state::screen_update_dietgo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	address_space &space = machine().dummy_space();
	uint16_t flip = m_deco_tilegen1->pf_control_r(space, 0, 0xffff);

	flip_screen_set(BIT(flip, 7));
	m_sprgen->set_flip_screen(BIT(flip, 7));
	m_deco_tilegen1->pf_update(m_pf1_rowscroll, m_pf2_rowscroll);

	bitmap.fill(256, cliprect); /* not verified */

	m_deco_tilegen1->tilemap_2_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	m_deco_tilegen1->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);

	m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram, 0x400);
	return 0;
}
