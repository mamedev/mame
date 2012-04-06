/***************************************************************************

  Dec0 Video emulation - Bryan McPhail, mish@tendril.co.uk

*********************************************************************/

#include "emu.h"
#include "includes/dec0.h"
#include "video/decbac06.h"
#include "video/decmxc06.h"

/******************************************************************************/

WRITE16_MEMBER(dec0_state::dec0_update_sprites_w)
{
	memcpy(m_buffered_spriteram,m_spriteram,0x800);
}

/******************************************************************************/

static void update_24bitcol(running_machine &machine, int offset)
{
	dec0_state *state = machine.driver_data<dec0_state>();
	int r,g,b;

	r = (state->m_generic_paletteram_16[offset] >> 0) & 0xff;
	g = (state->m_generic_paletteram_16[offset] >> 8) & 0xff;
	b = (state->m_generic_paletteram2_16[offset] >> 0) & 0xff;

	palette_set_color(machine,offset,MAKE_RGB(r,g,b));
}

WRITE16_MEMBER(dec0_state::dec0_paletteram_rg_w)
{
	COMBINE_DATA(&m_generic_paletteram_16[offset]);
	update_24bitcol(machine(), offset);
}

WRITE16_MEMBER(dec0_state::dec0_paletteram_b_w)
{
	COMBINE_DATA(&m_generic_paletteram2_16[offset]);
	update_24bitcol(machine(), offset);
}

/******************************************************************************/


SCREEN_UPDATE_IND16( hbarrel )
{
	dec0_state *state = screen.machine().driver_data<dec0_state>();

	flip_screen_set(screen.machine(), screen.machine().device<deco_bac06_device>("tilegen1")->get_flip_state());

	screen.machine().device<deco_bac06_device>("tilegen3")->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);
	screen.machine().device<deco_mxc06_device>("spritegen")->draw_sprites(screen.machine(), bitmap, cliprect, state->m_buffered_spriteram, 0x08, 0x08, 0x0f);
	screen.machine().device<deco_bac06_device>("tilegen2")->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);

	/* HB always keeps pf2 on top of pf3, no need explicitly support priority register */

	screen.machine().device<deco_mxc06_device>("spritegen")->draw_sprites(screen.machine(), bitmap, cliprect, state->m_buffered_spriteram, 0x08, 0x00, 0x0f);
	screen.machine().device<deco_bac06_device>("tilegen1")->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	return 0;
}

/******************************************************************************/

SCREEN_UPDATE_IND16( baddudes )
{
	dec0_state *state = screen.machine().driver_data<dec0_state>();
	flip_screen_set(screen.machine(), screen.machine().device<deco_bac06_device>("tilegen1")->get_flip_state());

	/* WARNING: inverted wrt Midnight Resistance */
	if ((state->m_pri & 0x01) == 0)
	{
		screen.machine().device<deco_bac06_device>("tilegen2")->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);
		screen.machine().device<deco_bac06_device>("tilegen3")->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);

		if (state->m_pri & 2)
			screen.machine().device<deco_bac06_device>("tilegen2")->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0,0x08,0x08,0x08,0x08); // upper 8 pens of upper 8 priority marked tiles /* Foreground pens only */

		screen.machine().device<deco_mxc06_device>("spritegen")->draw_sprites(screen.machine(), bitmap, cliprect, state->m_buffered_spriteram, 0x00, 0x00, 0x0f);

		if (state->m_pri & 4)
			screen.machine().device<deco_bac06_device>("tilegen3")->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0,0x08,0x08,0x08,0x08); // upper 8 pens of upper 8 priority marked tiles /* Foreground pens only */
	}
	else
	{
		screen.machine().device<deco_bac06_device>("tilegen3")->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);
		screen.machine().device<deco_bac06_device>("tilegen2")->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);

		if (state->m_pri & 2)
			screen.machine().device<deco_bac06_device>("tilegen3")->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0,0x08,0x08,0x08,0x08); // upper 8 pens of upper 8 priority marked tiles /* Foreground pens only */

		screen.machine().device<deco_mxc06_device>("spritegen")->draw_sprites(screen.machine(), bitmap, cliprect, state->m_buffered_spriteram, 0x00, 0x00, 0x0f);

		if (state->m_pri & 4)
			screen.machine().device<deco_bac06_device>("tilegen2")->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0,0x08,0x08,0x08,0x08); // upper 8 pens of upper 8 priority marked tiles /* Foreground pens only */
	}

	screen.machine().device<deco_bac06_device>("tilegen1")->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	return 0;
}

/******************************************************************************/

SCREEN_UPDATE_IND16( robocop )
{
	dec0_state *state = screen.machine().driver_data<dec0_state>();
	int trans;

	flip_screen_set(screen.machine(), screen.machine().device<deco_bac06_device>("tilegen1")->get_flip_state());

	if (state->m_pri & 0x04)
		trans = 0x08;
	else
		trans = 0x00;

	if (state->m_pri & 0x01)
	{
		/* WARNING: inverted wrt Midnight Resistance */
		/* Robocop uses it only for the title screen, so this might be just */
		/* completely wrong. The top 8 bits of the register might mean */
		/* something (they are 0x80 in midres, 0x00 here) */
		screen.machine().device<deco_bac06_device>("tilegen2")->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);

		if (state->m_pri & 0x02)
			screen.machine().device<deco_mxc06_device>("spritegen")->draw_sprites(screen.machine(), bitmap, cliprect, state->m_buffered_spriteram, 0x08, trans, 0x0f);

		screen.machine().device<deco_bac06_device>("tilegen3")->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	}
	else
	{
		screen.machine().device<deco_bac06_device>("tilegen3")->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);

		if (state->m_pri & 0x02)
			screen.machine().device<deco_mxc06_device>("spritegen")->draw_sprites(screen.machine(), bitmap, cliprect, state->m_buffered_spriteram, 0x08, trans, 0x0f);

		screen.machine().device<deco_bac06_device>("tilegen2")->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	}

	if (state->m_pri & 0x02)
		screen.machine().device<deco_mxc06_device>("spritegen")->draw_sprites(screen.machine(), bitmap, cliprect, state->m_buffered_spriteram, 0x08, trans^0x08, 0x0f);
	else
		screen.machine().device<deco_mxc06_device>("spritegen")->draw_sprites(screen.machine(), bitmap, cliprect, state->m_buffered_spriteram, 0x00, 0x00, 0x0f);

	screen.machine().device<deco_bac06_device>("tilegen1")->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	return 0;
}

/******************************************************************************/

SCREEN_UPDATE_IND16( birdtry )
{
	dec0_state *state = screen.machine().driver_data<dec0_state>();

	flip_screen_set(screen.machine(), screen.machine().device<deco_bac06_device>("tilegen1")->get_flip_state());

	/* This game doesn't have the extra playfield chip on the game board, but
    the palette does show through. */
	bitmap.fill(screen.machine().pens[768], cliprect);
	screen.machine().device<deco_bac06_device>("tilegen2")->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	screen.machine().device<deco_mxc06_device>("spritegen")->draw_sprites(screen.machine(), bitmap, cliprect, state->m_buffered_spriteram, 0x00, 0x00, 0x0f);
	screen.machine().device<deco_bac06_device>("tilegen1")->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	return 0;
}

/******************************************************************************/

SCREEN_UPDATE_IND16( hippodrm )
{
	dec0_state *state = screen.machine().driver_data<dec0_state>();
	flip_screen_set(screen.machine(), screen.machine().device<deco_bac06_device>("tilegen1")->get_flip_state());

	if (state->m_pri & 0x01)
	{
		screen.machine().device<deco_bac06_device>("tilegen2")->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);
		screen.machine().device<deco_bac06_device>("tilegen3")->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	}
	else
	{
		screen.machine().device<deco_bac06_device>("tilegen3")->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);
		screen.machine().device<deco_bac06_device>("tilegen2")->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	}

	screen.machine().device<deco_mxc06_device>("spritegen")->draw_sprites(screen.machine(), bitmap, cliprect, state->m_buffered_spriteram, 0x00, 0x00, 0x0f);
	screen.machine().device<deco_bac06_device>("tilegen1")->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	return 0;
}

/******************************************************************************/

SCREEN_UPDATE_IND16( slyspy )
{
	dec0_state *state = screen.machine().driver_data<dec0_state>();
	flip_screen_set(screen.machine(), screen.machine().device<deco_bac06_device>("tilegen1")->get_flip_state());

	screen.machine().device<deco_bac06_device>("tilegen3")->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);
	screen.machine().device<deco_bac06_device>("tilegen2")->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);

	screen.machine().device<deco_mxc06_device>("spritegen")->draw_sprites(screen.machine(), bitmap, cliprect, state->m_buffered_spriteram, 0x00, 0x00, 0x0f);

	/* Redraw top 8 pens of top 8 palettes over sprites */
	if (state->m_pri&0x80)
		screen.machine().device<deco_bac06_device>("tilegen2")->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0,0x08,0x08,0x08,0x08); // upper 8 pens of upper 8 priority marked tiles

	screen.machine().device<deco_bac06_device>("tilegen1")->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	return 0;
}

/******************************************************************************/

SCREEN_UPDATE_IND16( midres )
{
	dec0_state *state = screen.machine().driver_data<dec0_state>();
	int trans;

	flip_screen_set(screen.machine(), screen.machine().device<deco_bac06_device>("tilegen1")->get_flip_state());

	if (state->m_pri & 0x04)
		trans = 0x00;
	else trans = 0x08;

	if (state->m_pri & 0x01)
	{
		screen.machine().device<deco_bac06_device>("tilegen2")->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);

		if (state->m_pri & 0x02)
			screen.machine().device<deco_mxc06_device>("spritegen")->draw_sprites(screen.machine(), bitmap, cliprect, state->m_buffered_spriteram, 0x08, trans, 0x0f);

		screen.machine().device<deco_bac06_device>("tilegen3")->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	}
	else
	{
		screen.machine().device<deco_bac06_device>("tilegen3")->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);

		if (state->m_pri & 0x02)
			screen.machine().device<deco_mxc06_device>("spritegen")->draw_sprites(screen.machine(), bitmap, cliprect, state->m_buffered_spriteram, 0x08, trans, 0x0f);

		screen.machine().device<deco_bac06_device>("tilegen2")->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	}

	if (state->m_pri & 0x02)
		screen.machine().device<deco_mxc06_device>("spritegen")->draw_sprites(screen.machine(), bitmap, cliprect, state->m_buffered_spriteram, 0x08, trans ^ 0x08, 0x0f);
	else
		screen.machine().device<deco_mxc06_device>("spritegen")->draw_sprites(screen.machine(), bitmap, cliprect, state->m_buffered_spriteram, 0x00, 0x00, 0x0f);

	screen.machine().device<deco_bac06_device>("tilegen1")->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	return 0;
}


WRITE16_MEMBER(dec0_state::dec0_priority_w)
{
	COMBINE_DATA(&m_pri);
}

VIDEO_START( dec0_nodma )
{
	dec0_state *state = machine.driver_data<dec0_state>();
	state->m_buffered_spriteram = state->m_spriteram;
}

VIDEO_START( dec0 )
{
	dec0_state *state = machine.driver_data<dec0_state>();
	VIDEO_START_CALL(dec0_nodma);
	state->m_buffered_spriteram = auto_alloc_array(machine, UINT16, 0x800/2);
}

/******************************************************************************/
