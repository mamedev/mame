/* Simple 156 based board

*/

#include "emu.h"
#include "includes/simpl156.h"
#include "video/deco16ic.h"
#include "video/decospr.h"


VIDEO_START( simpl156 )
{
	simpl156_state *state = machine.driver_data<simpl156_state>();

	/* allocate the ram as 16-bit (we do it here because the CPU is 32-bit) */
	state->m_pf1_rowscroll = auto_alloc_array_clear(machine, UINT16, 0x800/2);
	state->m_pf2_rowscroll = auto_alloc_array_clear(machine, UINT16, 0x800/2);
	state->m_spriteram = auto_alloc_array_clear(machine, UINT16, 0x2000/2);
	state->m_generic_paletteram_16.allocate(0x1000/2);

	memset(state->m_spriteram, 0xff, 0x2000);

	/* and register the allocated ram so that save states still work */
	state->save_pointer(NAME(state->m_pf1_rowscroll), 0x800/2);
	state->save_pointer(NAME(state->m_pf2_rowscroll), 0x800/2);
	state->save_pointer(NAME(state->m_spriteram), 0x2000/2);
}

SCREEN_UPDATE_IND16( simpl156 )
{
	simpl156_state *state = screen.machine().driver_data<simpl156_state>();

	screen.machine().priority_bitmap.fill(0);

	deco16ic_pf_update(state->m_deco_tilegen1, state->m_pf1_rowscroll, state->m_pf2_rowscroll);

	bitmap.fill(256, cliprect);

	deco16ic_tilemap_2_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 2);
	deco16ic_tilemap_1_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 4);

	//FIXME: flip_screen_x should not be written!
	state->flip_screen_set_no_update(1);

	screen.machine().device<decospr_device>("spritegen")->draw_sprites(bitmap, cliprect, state->m_spriteram, 0x1400/4); // 0x1400/4 seems right for charlien (doesn't initialize any more RAM, so will draw a garbage 0 with more)
	return 0;
}
