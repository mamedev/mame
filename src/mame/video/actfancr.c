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

UINT32 actfancr_state::screen_update_actfancr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* Draw playfield */
	//m_flipscreen = m_control_2[0] & 0x80;
	//machine().tilemap().set_flip_all(m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	machine().device<deco_bac06_device>("tilegen1")->deco_bac06_pf_draw(machine(),bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);
	machine().device<deco_mxc06_device>("spritegen")->draw_sprites(machine(), bitmap, cliprect, m_spriteram16, 0x00, 0x00, 0x0f);
	machine().device<deco_bac06_device>("tilegen2")->deco_bac06_pf_draw(machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);

	return 0;
}
