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

UINT32 supbtime_state::screen_update_supbtime(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	address_space &space = machine().driver_data()->generic_space();
	UINT16 flip = deco16ic_pf_control_r(m_deco_tilegen1, space, 0, 0xffff);

	flip_screen_set(BIT(flip, 7));
	deco16ic_pf_update(m_deco_tilegen1, m_pf1_rowscroll, m_pf2_rowscroll);

	bitmap.fill(768, cliprect);

	deco16ic_tilemap_2_draw(m_deco_tilegen1, bitmap, cliprect, 0, 0);
	machine().device<decospr_device>("spritegen")->draw_sprites(bitmap, cliprect, m_spriteram, 0x400);
	deco16ic_tilemap_1_draw(m_deco_tilegen1, bitmap, cliprect, 0, 0);
	return 0;
}
