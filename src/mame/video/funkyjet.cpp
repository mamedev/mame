// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

   Funky Jet Video emulation - Bryan McPhail, mish@tendril.co.uk

***************************************************************************/

#include "emu.h"
#include "includes/funkyjet.h"

/******************************************************************************/

uint32_t funkyjet_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t flip = m_deco_tilegen->pf_control_r(0);

	flip_screen_set(BIT(flip, 7));
	m_sprgen->set_flip_screen(BIT(flip, 7));
	m_deco_tilegen->pf_update(m_pf_rowscroll[0], m_pf_rowscroll[1]);

	bitmap.fill(768, cliprect);
	m_deco_tilegen->tilemap_2_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	m_deco_tilegen->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
	m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram, 0x400);
	return 0;
}
