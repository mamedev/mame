// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

  stadhero video emulation - Bryan McPhail, mish@tendril.co.uk

*********************************************************************

    MXC-06 chip to produce sprites, see dec0.c
    BAC-06 chip for background
    ??? for text layer

***************************************************************************/

#include "emu.h"
#include "includes/stadhero.h"


/******************************************************************************/

/******************************************************************************/

UINT32 stadhero_state::screen_update_stadhero(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	flip_screen_set(m_tilegen1->get_flip_state());

	m_tilegen1->set_bppmultmask(0x8, 0x7);
	m_tilegen1->deco_bac06_pf_draw(bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);
	m_spritegen->draw_sprites(bitmap, cliprect, m_spriteram, 0x00, 0x00, 0x0f);
	m_pf1_tilemap->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}

/******************************************************************************/

WRITE16_MEMBER(stadhero_state::stadhero_pf1_data_w)
{
	COMBINE_DATA(&m_pf1_data[offset]);
	m_pf1_tilemap->mark_tile_dirty(offset);
}


/******************************************************************************/

TILE_GET_INFO_MEMBER(stadhero_state::get_pf1_tile_info)
{
	int tile=m_pf1_data[tile_index];
	int color=tile >> 12;

	tile=tile&0xfff;
	SET_TILE_INFO_MEMBER(0,
			tile,
			color,
			0);
}

void stadhero_state::video_start()
{
	m_pf1_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(stadhero_state::get_pf1_tile_info),this),TILEMAP_SCAN_ROWS, 8, 8,32,32);
	m_pf1_tilemap->set_transparent_pen(0);
}

/******************************************************************************/
