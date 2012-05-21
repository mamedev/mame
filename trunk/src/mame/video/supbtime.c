/***************************************************************************

   Super Burger Time Video emulation - Bryan McPhail, mish@tendril.co.uk

*********************************************************************

Uses Data East custom chip 55 for backgrounds, custom chip 52 for sprites.

See Dark Seal & Caveman Ninja drivers for info on these chips.

End sequence uses rowscroll '98 c0' on pf1 (jmp to 1d61a on supbtimj)

***************************************************************************/

#include "emu.h"
#include "video/deco16ic.h"
#include "includes/supbtime.h"
#include "video/decospr.h"

/******************************************************************************/

/******************************************************************************/

SCREEN_UPDATE_IND16(supbtime)
{
	supbtime_state *state = screen.machine().driver_data<supbtime_state>();
	UINT16 flip = deco16ic_pf_control_r(state->m_deco_tilegen1, 0, 0xffff);

	state->flip_screen_set(BIT(flip, 7));
	deco16ic_pf_update(state->m_deco_tilegen1, state->m_pf1_rowscroll, state->m_pf2_rowscroll);

	bitmap.fill(768, cliprect);

	deco16ic_tilemap_2_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 0);
	screen.machine().device<decospr_device>("spritegen")->draw_sprites(bitmap, cliprect, state->m_spriteram, 0x400);
	deco16ic_tilemap_1_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 0);
	return 0;
}
