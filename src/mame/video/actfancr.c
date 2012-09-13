/*******************************************************************************

    actfancr - Bryan McPhail, mish@tendril.co.uk

*******************************************************************************/

#include "emu.h"
#include "includes/actfancr.h"
#include "video/decbac06.h"
#include "video/decmxc06.h"

/******************************************************************************/

static void register_savestate( running_machine &machine )
{
	actfancr_state *state = machine.driver_data<actfancr_state>();
	state->save_item(NAME(state->m_flipscreen));
}

void actfancr_state::video_start()
{
	register_savestate(machine());
}

/******************************************************************************/

SCREEN_UPDATE_IND16( actfancr )
{
	actfancr_state *state = screen.machine().driver_data<actfancr_state>();

	/* Draw playfield */
	//state->m_flipscreen = state->m_control_2[0] & 0x80;
	//screen.machine().tilemap().set_flip_all(state->m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	screen.machine().device<deco_bac06_device>("tilegen1")->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);
	screen.machine().device<deco_mxc06_device>("spritegen")->draw_sprites(screen.machine(), bitmap, cliprect, state->m_spriteram16, 0x00, 0x00, 0x0f);
	screen.machine().device<deco_bac06_device>("tilegen2")->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);

	return 0;
}

