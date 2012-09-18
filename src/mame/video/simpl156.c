/* Simple 156 based board

*/

#include "emu.h"
#include "includes/simpl156.h"
#include "video/deco16ic.h"
#include "video/decospr.h"


void simpl156_state::video_start()
{

	/* allocate the ram as 16-bit (we do it here because the CPU is 32-bit) */
	m_pf1_rowscroll = auto_alloc_array_clear(machine(), UINT16, 0x800/2);
	m_pf2_rowscroll = auto_alloc_array_clear(machine(), UINT16, 0x800/2);
	m_spriteram = auto_alloc_array_clear(machine(), UINT16, 0x2000/2);
	m_generic_paletteram_16.allocate(0x1000/2);

	memset(m_spriteram, 0xff, 0x2000);

	/* and register the allocated ram so that save states still work */
	save_pointer(NAME(m_pf1_rowscroll), 0x800/2);
	save_pointer(NAME(m_pf2_rowscroll), 0x800/2);
	save_pointer(NAME(m_spriteram), 0x2000/2);
}

UINT32 simpl156_state::screen_update_simpl156(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{

	machine().priority_bitmap.fill(0);

	deco16ic_pf_update(m_deco_tilegen1, m_pf1_rowscroll, m_pf2_rowscroll);

	bitmap.fill(256, cliprect);

	deco16ic_tilemap_2_draw(m_deco_tilegen1, bitmap, cliprect, 0, 2);
	deco16ic_tilemap_1_draw(m_deco_tilegen1, bitmap, cliprect, 0, 4);

	//FIXME: flip_screen_x should not be written!
	flip_screen_set_no_update(1);

	machine().device<decospr_device>("spritegen")->draw_sprites(bitmap, cliprect, m_spriteram, 0x1400/4); // 0x1400/4 seems right for charlien (doesn't initialize any more RAM, so will draw a garbage 0 with more)
	return 0;
}
