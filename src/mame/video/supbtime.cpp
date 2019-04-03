// license: BSD-3-Clause
// copyright-holders: Bryan McPhail, David Haywood, Dirk Best
/***************************************************************************

    Super Burger Time

    Video mixing

    - are there priority registers / bits in the sprites that would allow
      this to be collapsed further?

***************************************************************************/

#include "emu.h"
#include "includes/supbtime.h"


uint32_t supbtime_state::screen_update_common(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, bool use_offsets)
{
	uint16_t flip = m_deco_tilegen->pf_control_r(0);

	flip_screen_set(BIT(flip, 7));
	m_sprgen->set_flip_screen(BIT(flip, 7));
	m_deco_tilegen->pf_update(m_pf_rowscroll[0], m_pf_rowscroll[1]);

	bitmap.fill(768, cliprect);

	if (use_offsets)
	{
		// chinatwn and tumblep are verified as needing a 1 pixel offset on the tilemaps to match original hardware (supbtime appears to not want them)
		m_deco_tilegen->set_scrolldx(0, 0, 1, -1);
		m_deco_tilegen->set_scrolldx(0, 1, 1, -1);
		m_deco_tilegen->set_scrolldx(1, 0, 1, -1);
		m_deco_tilegen->set_scrolldx(1, 1, 1, -1);
	}

	return 0;
}

// End sequence uses rowscroll '98 c0' on pf1 (jmp to 1d61a on supbtimej)
uint32_t supbtime_state::screen_update_supbtime(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen_update_common(screen, bitmap, cliprect, false);

	m_deco_tilegen->tilemap_2_draw(screen, bitmap, cliprect, 0, 0);
	m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram, 0x400);
	m_deco_tilegen->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

uint32_t supbtime_state::screen_update_chinatwn(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen_update_common(screen, bitmap, cliprect, true);

	m_deco_tilegen->tilemap_2_draw(screen, bitmap, cliprect, 0, 0);
	m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram, 0x400);
	m_deco_tilegen->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

uint32_t supbtime_state::screen_update_tumblep(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen_update_common(screen, bitmap, cliprect, true);

	m_deco_tilegen->tilemap_2_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	m_deco_tilegen->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
	m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram, 0x400);

	return 0;
}

