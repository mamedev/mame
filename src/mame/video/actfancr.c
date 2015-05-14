// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*******************************************************************************

    actfancr - Bryan McPhail, mish@tendril.co.uk

*******************************************************************************/

#include "emu.h"
#include "includes/actfancr.h"

/******************************************************************************/

UINT32 actfancr_state::screen_update_actfancr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* Draw playfield */
	flip_screen_set(m_tilegen2->get_flip_state());

	m_tilegen1->deco_bac06_pf_draw(bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);
	m_spritegen->draw_sprites(bitmap, cliprect, m_spriteram16, 0x00, 0x00, 0x0f);
	m_tilegen2->deco_bac06_pf_draw(bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);

	return 0;
}
