/***************************************************************************

   Crude Buster Video emulation - Bryan McPhail, mish@tendril.co.uk

***************************************************************************/

#include "emu.h"
#include "includes/cbuster.h"
#include "video/deco16ic.h"
#include "video/decospr.h"

/******************************************************************************/

/* maybe the game should just use generic palette handling, and have a darker palette by design... */

static void update_24bitcol( running_machine &machine, int offset )
{
	cbuster_state *state = machine.driver_data<cbuster_state>();
	UINT8 r, g, b; /* The highest palette value seems to be 0x8e */

	r = (UINT8)((float)((state->m_generic_paletteram_16[offset]  >> 0) & 0xff) * 1.75);
	g = (UINT8)((float)((state->m_generic_paletteram_16[offset]  >> 8) & 0xff) * 1.75);
	b = (UINT8)((float)((state->m_generic_paletteram2_16[offset] >> 0) & 0xff) * 1.75);

	palette_set_color(machine, offset, MAKE_RGB(r, g, b));
}

WRITE16_MEMBER(cbuster_state::twocrude_palette_24bit_rg_w)
{
	COMBINE_DATA(&m_generic_paletteram_16[offset]);
	update_24bitcol(machine(), offset);
}

WRITE16_MEMBER(cbuster_state::twocrude_palette_24bit_b_w)
{
	COMBINE_DATA(&m_generic_paletteram2_16[offset]);
	update_24bitcol(machine(), offset);
}


/******************************************************************************/


/******************************************************************************/

VIDEO_START( twocrude )
{
	machine.device<decospr_device>("spritegen")->alloc_sprite_bitmap();
}

SCREEN_UPDATE_RGB32( twocrude )
{
	cbuster_state *state = screen.machine().driver_data<cbuster_state>();
	UINT16 flip = deco16ic_pf_control_r(state->m_deco_tilegen1, 0, 0xffff);

	flip_screen_set(screen.machine(), !BIT(flip, 7));

	screen.machine().device<decospr_device>("spritegen")->draw_sprites(bitmap, cliprect, state->m_spriteram16_buffer, 0x400);


	deco16ic_pf_update(state->m_deco_tilegen1, state->m_pf1_rowscroll, state->m_pf2_rowscroll);
	deco16ic_pf_update(state->m_deco_tilegen2, state->m_pf3_rowscroll, state->m_pf4_rowscroll);

	/* Draw playfields & sprites */
	deco16ic_tilemap_2_draw(state->m_deco_tilegen2, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	screen.machine().device<decospr_device>("spritegen")->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0800, 0x0900, 0x100, 0x0ff);
	screen.machine().device<decospr_device>("spritegen")->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0900, 0x0900, 0x500, 0x0ff);

	if (state->m_pri)
	{
		deco16ic_tilemap_2_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 0);
		deco16ic_tilemap_1_draw(state->m_deco_tilegen2, bitmap, cliprect, 0, 0);
	}
	else
	{
		deco16ic_tilemap_1_draw(state->m_deco_tilegen2, bitmap, cliprect, 0, 0);
		deco16ic_tilemap_2_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 0);
	}

	screen.machine().device<decospr_device>("spritegen")->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0000, 0x0900, 0x100, 0x0ff);
	screen.machine().device<decospr_device>("spritegen")->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0100, 0x0900, 0x500, 0x0ff);
	deco16ic_tilemap_1_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 0);
	return 0;
}
