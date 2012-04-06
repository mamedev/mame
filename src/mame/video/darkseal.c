/***************************************************************************

   Dark Seal Video emulation - Bryan McPhail, mish@tendril.co.uk

****************************************************************************

 uses 2x DECO55 tilemaps

**************************************************************************

 Sprite/Tilemap Priority Note (is this implemented?)

    Word 4:
        Mask 0x8000 - ?
        Mask 0x4000 - Sprite is drawn beneath top 8 pens of playfield 4
        Mask 0x3e00 - Colour (32 palettes, most games only use 16)
        Mask 0x01ff - X coordinate

***************************************************************************/

#include "emu.h"
#include "includes/darkseal.h"
#include "video/decospr.h"
#include "video/deco16ic.h"

/***************************************************************************/

/******************************************************************************/

static void update_24bitcol(running_machine &machine, int offset)
{
	darkseal_state *state = machine.driver_data<darkseal_state>();
	int r,g,b;

	r = (state->m_generic_paletteram_16[offset] >> 0) & 0xff;
	g = (state->m_generic_paletteram_16[offset] >> 8) & 0xff;
	b = (state->m_generic_paletteram2_16[offset] >> 0) & 0xff;

	palette_set_color(machine,offset,MAKE_RGB(r,g,b));
}

WRITE16_MEMBER(darkseal_state::darkseal_palette_24bit_rg_w)
{
	COMBINE_DATA(&m_generic_paletteram_16[offset]);
	update_24bitcol(machine(), offset);
}

WRITE16_MEMBER(darkseal_state::darkseal_palette_24bit_b_w)
{
	COMBINE_DATA(&m_generic_paletteram2_16[offset]);
	update_24bitcol(machine(), offset);
}

/******************************************************************************/

VIDEO_START( darkseal )
{

}

/******************************************************************************/

SCREEN_UPDATE_IND16( darkseal )
{
	darkseal_state *state = screen.machine().driver_data<darkseal_state>();
	screen.machine().tilemap().set_flip_all(state->m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	bitmap.fill(get_black_pen(screen.machine()), cliprect);

	deco16ic_pf_update(state->m_deco_tilegen1, state->m_pf1_rowscroll, state->m_pf1_rowscroll);
	deco16ic_pf_update(state->m_deco_tilegen2, state->m_pf3_rowscroll, state->m_pf3_rowscroll);

	deco16ic_tilemap_1_draw(state->m_deco_tilegen2, bitmap, cliprect, 0, 0);
	deco16ic_tilemap_2_draw(state->m_deco_tilegen2, bitmap, cliprect, 0, 0);

	deco16ic_tilemap_1_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 0);
	screen.machine().device<decospr_device>("spritegen")->draw_sprites(bitmap, cliprect, state->m_spriteram->buffer(), 0x400);
	deco16ic_tilemap_2_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 0);

	return 0;
}

/******************************************************************************/
