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

SCREEN_UPDATE( stadhero )
{
	stadhero_state *state = screen->machine().driver_data<stadhero_state>();
//  tilemap_set_flip_all(screen->machine(),state->m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	screen->machine().device<deco_bac06_device>("tilegen1")->set_bppmultmask(0x8, 0x7);
	screen->machine().device<deco_bac06_device>("tilegen1")->deco_bac06_pf_draw(screen->machine(),bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);
	screen->machine().device<deco_mxc06_device>("spritegen")->draw_sprites(screen->machine(), bitmap, cliprect, state->m_spriteram, 0x00, 0x00, 0x0f);
	tilemap_draw(bitmap,cliprect,state->m_pf1_tilemap,0,0);
	return 0;
}

/******************************************************************************/

WRITE16_HANDLER( stadhero_pf1_data_w )
{
	stadhero_state *state = space->machine().driver_data<stadhero_state>();
	COMBINE_DATA(&state->m_pf1_data[offset]);
	tilemap_mark_tile_dirty(state->m_pf1_tilemap,offset);
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
	tilemap_set_transparent_pen(state->m_pf1_tilemap,0);
}

/******************************************************************************/
