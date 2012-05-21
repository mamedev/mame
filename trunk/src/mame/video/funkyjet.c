/***************************************************************************

   Funky Jet Video emulation - Bryan McPhail, mish@tendril.co.uk

***************************************************************************/

#include "emu.h"
#include "includes/funkyjet.h"
#include "video/deco16ic.h"
#include "video/decospr.h"

/******************************************************************************/

SCREEN_UPDATE_IND16( funkyjet )
{
	funkyjet_state *state = screen.machine().driver_data<funkyjet_state>();
	UINT16 flip = deco16ic_pf_control_r(state->m_deco_tilegen1, 0, 0xffff);

	state->flip_screen_set(BIT(flip, 7));
	deco16ic_pf_update(state->m_deco_tilegen1, state->m_pf1_rowscroll, state->m_pf2_rowscroll);

	bitmap.fill(768, cliprect);
	deco16ic_tilemap_2_draw(state->m_deco_tilegen1, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	deco16ic_tilemap_1_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 0);
	screen.machine().device<decospr_device>("spritegen")->draw_sprites(bitmap, cliprect, state->m_spriteram, 0x400);
	return 0;
}
