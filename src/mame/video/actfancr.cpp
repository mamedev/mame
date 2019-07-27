// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*******************************************************************************

    actfancr - Bryan McPhail, mish@tendril.co.uk

*******************************************************************************/

#include "emu.h"
#include "includes/actfancr.h"

/******************************************************************************/

void actfancr_state::video_start()
{
	m_spriteram16 = make_unique_clear<uint16_t[]>(0x800/2);
	save_pointer(NAME(m_spriteram16),0x800/2);
}

uint32_t actfancr_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* Draw playfield */
	bool flip = m_tilegen[1]->get_flip_state();
	m_tilegen[0]->set_flip_screen(flip);
	m_tilegen[1]->set_flip_screen(flip);
	m_spritegen->set_flip_screen(flip);

	m_tilegen[0]->deco_bac06_pf_draw(screen,bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00, 0);
	m_spritegen->draw_sprites(screen, bitmap, cliprect, m_gfxdecode->gfx(1), m_spriteram16.get(), 0x800/2);
	m_tilegen[1]->deco_bac06_pf_draw(screen,bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00, 0);

	return 0;
}
