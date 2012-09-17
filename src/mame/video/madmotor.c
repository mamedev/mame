/***************************************************************************

  Mad Motor video emulation - Bryan McPhail, mish@tendril.co.uk

  Notes:  Playfield 3 can change size between 512x1024 and 2048x256

***************************************************************************/

#include "emu.h"
#include "includes/madmotor.h"
#include "video/decbac06.h"
#include "video/decmxc06.h"

/******************************************************************************/

void madmotor_state::video_start()
{
}

/******************************************************************************/


/******************************************************************************/

UINT32 madmotor_state::screen_update_madmotor(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	flip_screen_set(screen.machine().device<deco_bac06_device>("tilegen1")->get_flip_state());

//  screen.machine().tilemap().set_flip_all(screen.machine().device<deco_bac06_device>("tilegen1")->get_flip_state() ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	screen.machine().device<deco_bac06_device>("tilegen3")->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);
	screen.machine().device<deco_bac06_device>("tilegen2")->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	screen.machine().device<deco_mxc06_device>("spritegen")->draw_sprites(screen.machine(), bitmap, cliprect, m_spriteram, 0x00, 0x00, 0x0f);
	screen.machine().device<deco_bac06_device>("tilegen1")->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	return 0;
}
