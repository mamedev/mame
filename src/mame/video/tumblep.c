/***************************************************************************

   Tumblepop Video emulation - Bryan McPhail, mish@tendril.co.uk

*********************************************************************

Uses Data East custom chip 55 for backgrounds, custom chip 52 for sprites.

See Dark Seal & Caveman Ninja drivers for info on these chips.

Tumblepop is one of few games to take advantage of the playfields ability
to switch between 8*8 tiles and 16*16 tiles.

***************************************************************************/

#include "emu.h"
#include "video/deco16ic.h"
#include "includes/tumblep.h"
#include "video/decospr.h"


UINT32 tumblep_state::screen_update_tumblep(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	address_space &space = screen.machine().driver_data()->generic_space();
	UINT16 flip = deco16ic_pf_control_r(m_deco_tilegen1, space, 0, 0xffff);

	flip_screen_set(BIT(flip, 7));
	deco16ic_pf_update(m_deco_tilegen1, m_pf1_rowscroll, m_pf2_rowscroll);

	bitmap.fill(256, cliprect); /* not verified */

	deco16ic_tilemap_2_draw(m_deco_tilegen1, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	deco16ic_tilemap_1_draw(m_deco_tilegen1, bitmap, cliprect, 0, 0);

	screen.machine().device<decospr_device>("spritegen")->draw_sprites(bitmap, cliprect, m_spriteram, 0x400);
	return 0;
}
