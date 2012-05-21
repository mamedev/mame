/***************************************************************************

  stadhero video emulation - Bryan McPhail, mish@tendril.co.uk

*********************************************************************

    MXC-06 chip to produce sprites, see dec0.c
    BAC-06 chip for background
    ??? for text layer

***************************************************************************/

#include "emu.h"
#include "includes/stadhero.h"
#include "video/decbac06.h"
#include "video/decmxc06.h"

/******************************************************************************/

/******************************************************************************/

SCREEN_UPDATE_IND16( stadhero )
{
	stadhero_state *state = screen.machine().driver_data<stadhero_state>();
//  screen.machine().tilemap().set_flip_all(state->m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	screen.machine().device<deco_bac06_device>("tilegen1")->set_bppmultmask(0x8, 0x7);
	screen.machine().device<deco_bac06_device>("tilegen1")->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);
	screen.machine().device<deco_mxc06_device>("spritegen")->draw_sprites(screen.machine(), bitmap, cliprect, state->m_spriteram, 0x00, 0x00, 0x0f);
	state->m_pf1_tilemap->draw(bitmap, cliprect, 0,0);
	return 0;
}

/******************************************************************************/

WRITE16_MEMBER(stadhero_state::stadhero_pf1_data_w)
{
	COMBINE_DATA(&m_pf1_data[offset]);
	m_pf1_tilemap->mark_tile_dirty(offset);
}


/******************************************************************************/

static TILE_GET_INFO( get_pf1_tile_info )
{
	stadhero_state *state = machine.driver_data<stadhero_state>();
	int tile=state->m_pf1_data[tile_index];
	int color=tile >> 12;

	tile=tile&0xfff;
	SET_TILE_INFO(
			0,
			tile,
			color,
			0);
}

VIDEO_START( stadhero )
{
	stadhero_state *state = machine.driver_data<stadhero_state>();
	state->m_pf1_tilemap =     tilemap_create(machine, get_pf1_tile_info,tilemap_scan_rows, 8, 8,32,32);
	state->m_pf1_tilemap->set_transparent_pen(0);
}

/******************************************************************************/
