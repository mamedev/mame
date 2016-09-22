// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

  Mad Motor video emulation - Bryan McPhail, mish@tendril.co.uk

  Notes:  Playfield 3 can change size between 512x1024 and 2048x256

***************************************************************************/

#include "emu.h"
#include "includes/madmotor.h"

/******************************************************************************/

void madmotor_state::video_start()
{
}

/******************************************************************************/


/******************************************************************************/

UINT32 madmotor_state::screen_update_madmotor(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bool flip = m_tilegen1->get_flip_state();
	m_tilegen2->set_flip_state(flip);
	m_tilegen3->set_flip_state(flip);
	m_gfxdecode->set_flip_all(flip ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	m_tilegen3->deco_bac06_pf_draw(bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);
	m_tilegen2->deco_bac06_pf_draw(bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	m_spritegen->draw_sprites(bitmap, cliprect, m_spriteram, 0x00, 0x00, 0x0f, flip);
	m_tilegen1->deco_bac06_pf_draw(bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	return 0;
}
