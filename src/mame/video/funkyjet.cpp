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
	// Similar to chinatwn and tumblep, see video/supbtime.cpp
	//
	// This causes a 2 pixel gap on the left side of the first stage of all worlds in funkyjet
	// but allows subsequent stages to be centered and avoids corruption on the world select
	// screen after each world.  It also correctly aligns the graphics in sotsugyo.
	//
	// The 2 pixel gap on the first stage of each world has been verified to occur on hardware.
	// (it can easily be seen by moving your player sprite to the far left)
	//
	// it is unclear where this offset comes from, but real hardware videos confirm it is needed

	m_deco_tilegen->set_scrolldx(0, 0, 1, 1);
	m_deco_tilegen->set_scrolldx(0, 1, 1, 1);
	m_deco_tilegen->set_scrolldx(1, 0, 1, 1);
	m_deco_tilegen->set_scrolldx(1, 1, 1, 1);

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
