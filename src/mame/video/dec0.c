/***************************************************************************

  Dec0 Video emulation - Bryan McPhail, mish@tendril.co.uk

*********************************************************************/

#include "emu.h"
#include "includes/dec0.h"


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

	state->flip_screen_set(state->m_tilegen1->get_flip_state());

	state->m_tilegen3->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);
	state->m_spritegen->draw_sprites(screen.machine(), bitmap, cliprect, state->m_buffered_spriteram, 0x08, 0x08, 0x0f);
	state->m_tilegen2->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);

	/* HB always keeps pf2 on top of pf3, no need explicitly support priority register */

	state->m_spritegen->draw_sprites(screen.machine(), bitmap, cliprect, state->m_buffered_spriteram, 0x08, 0x00, 0x0f);
	state->m_tilegen1->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	return 0;
}

/******************************************************************************/

SCREEN_UPDATE_IND16( baddudes )
{
	dec0_state *state = screen.machine().driver_data<dec0_state>();
	state->flip_screen_set(state->m_tilegen1->get_flip_state());

	/* WARNING: inverted wrt Midnight Resistance */
	if ((state->m_pri & 0x01) == 0)
	{
		state->m_tilegen2->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);
		state->m_tilegen3->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);

		if (state->m_pri & 2)
			state->m_tilegen2->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0,0x08,0x08,0x08,0x08); // upper 8 pens of upper 8 priority marked tiles /* Foreground pens only */

		state->m_spritegen->draw_sprites(screen.machine(), bitmap, cliprect, state->m_buffered_spriteram, 0x00, 0x00, 0x0f);

		if (state->m_pri & 4)
			state->m_tilegen3->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0,0x08,0x08,0x08,0x08); // upper 8 pens of upper 8 priority marked tiles /* Foreground pens only */
	}
	else
	{
		state->m_tilegen3->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);
		state->m_tilegen2->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);

		if (state->m_pri & 2)
			state->m_tilegen3->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0,0x08,0x08,0x08,0x08); // upper 8 pens of upper 8 priority marked tiles /* Foreground pens only */

		state->m_spritegen->draw_sprites(screen.machine(), bitmap, cliprect, state->m_buffered_spriteram, 0x00, 0x00, 0x0f);

		if (state->m_pri & 4)
			state->m_tilegen2->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0,0x08,0x08,0x08,0x08); // upper 8 pens of upper 8 priority marked tiles /* Foreground pens only */
	}

	state->m_tilegen1->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	return 0;
}

/******************************************************************************/

SCREEN_UPDATE_IND16( robocop )
{
	dec0_state *state = screen.machine().driver_data<dec0_state>();
	int trans;

	state->flip_screen_set(state->m_tilegen1->get_flip_state());

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
		state->m_tilegen2->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);

		if (state->m_pri & 0x02)
			state->m_spritegen->draw_sprites(screen.machine(), bitmap, cliprect, state->m_buffered_spriteram, 0x08, trans, 0x0f);

		state->m_tilegen3->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	}
	else
	{
		state->m_tilegen3->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);

		if (state->m_pri & 0x02)
			state->m_spritegen->draw_sprites(screen.machine(), bitmap, cliprect, state->m_buffered_spriteram, 0x08, trans, 0x0f);

		state->m_tilegen2->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	}

	if (state->m_pri & 0x02)
		state->m_spritegen->draw_sprites(screen.machine(), bitmap, cliprect, state->m_buffered_spriteram, 0x08, trans^0x08, 0x0f);
	else
		state->m_spritegen->draw_sprites(screen.machine(), bitmap, cliprect, state->m_buffered_spriteram, 0x00, 0x00, 0x0f);

	state->m_tilegen1->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	return 0;
}


SCREEN_UPDATE_IND16( automat )
{
	dec0_automat_state *state = screen.machine().driver_data<dec0_automat_state>();
	int trans;

	// layer enables seem different... where are they?

	// the bootleg doesn't write these registers, I think they're hardcoded?, so fake them for compatibility with our implementation..
	deco_bac06_pf_control_0_w(state->m_tilegen1,0,0x0003, 0x00ff); // 8x8
	deco_bac06_pf_control_0_w(state->m_tilegen1,1,0x0003, 0x00ff);
	deco_bac06_pf_control_0_w(state->m_tilegen1,2,0x0000, 0x00ff);
	deco_bac06_pf_control_0_w(state->m_tilegen1,3,0x0001, 0x00ff); // dimensions

	deco_bac06_pf_control_0_w(state->m_tilegen2,0,0x0082, 0x00ff); // 16x16
	deco_bac06_pf_control_0_w(state->m_tilegen2,1,0x0000, 0x00ff);
	deco_bac06_pf_control_0_w(state->m_tilegen2,2,0x0000, 0x00ff);
	deco_bac06_pf_control_0_w(state->m_tilegen2,3,0x0001, 0x00ff); // dimensions

	deco_bac06_pf_control_0_w(state->m_tilegen3,0,0x0082, 0x00ff); // 16x16
	deco_bac06_pf_control_0_w(state->m_tilegen3,1,0x0003, 0x00ff);
	deco_bac06_pf_control_0_w(state->m_tilegen3,2,0x0000, 0x00ff);
	deco_bac06_pf_control_0_w(state->m_tilegen3,3,0x0001, 0x00ff); // dimensions

	// scroll registers got written elsewhere, copy them across
	deco_bac06_pf_control_1_w(state->m_tilegen1,0,0x0000, 0xffff); // no scroll?
	deco_bac06_pf_control_1_w(state->m_tilegen1,1,0x0000, 0xffff); // no scroll?

	deco_bac06_pf_control_1_w(state->m_tilegen2,0,state->m_automat_scroll_regs[3] - 0x010a, 0xffff);
	deco_bac06_pf_control_1_w(state->m_tilegen2,1,state->m_automat_scroll_regs[2], 0xffff);

	deco_bac06_pf_control_1_w(state->m_tilegen3,0,state->m_automat_scroll_regs[1] - 0x0108, 0xffff);
	deco_bac06_pf_control_1_w(state->m_tilegen3,1,state->m_automat_scroll_regs[0], 0xffff);


	state->flip_screen_set(state->m_tilegen1->get_flip_state());

	if (state->m_pri & 0x04)
		trans = 0x08;
	else
		trans = 0x00;

	if (state->m_pri & 0x01)
	{
		state->m_tilegen2->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);

		if (state->m_pri & 0x02)
			state->m_spritegen->draw_sprites(screen.machine(), bitmap, cliprect, state->m_buffered_spriteram, 0x08, trans, 0x0f);

		state->m_tilegen3->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	}
	else
	{
		state->m_tilegen3->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);

		if (state->m_pri & 0x02)
			state->m_spritegen->draw_sprites(screen.machine(), bitmap, cliprect, state->m_buffered_spriteram, 0x08, trans, 0x0f);

		state->m_tilegen2->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	}

	if (state->m_pri & 0x02)
		state->m_spritegen->draw_sprites_bootleg(screen.machine(), bitmap, cliprect, state->m_buffered_spriteram, 0x08, trans^0x08, 0x0f);
	else
		state->m_spritegen->draw_sprites_bootleg(screen.machine(), bitmap, cliprect, state->m_buffered_spriteram, 0x00, 0x00, 0x0f);

	state->m_tilegen1->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	return 0;
}

SCREEN_UPDATE_IND16( secretab )
{
	dec0_automat_state *state = screen.machine().driver_data<dec0_automat_state>();

	// layer enables seem different... where are they?

	// the bootleg doesn't write these registers, I think they're hardcoded?, so fake them for compatibility with our implementation..
	deco_bac06_pf_control_0_w(state->m_tilegen1,0,0x0003, 0x00ff); // 8x8
	deco_bac06_pf_control_0_w(state->m_tilegen1,1,0x0003, 0x00ff);
	deco_bac06_pf_control_0_w(state->m_tilegen1,2,0x0000, 0x00ff);
	deco_bac06_pf_control_0_w(state->m_tilegen1,3,0x0001, 0x00ff); // dimensions

	deco_bac06_pf_control_0_w(state->m_tilegen2,0,0x0082, 0x00ff); // 16x16
	deco_bac06_pf_control_0_w(state->m_tilegen2,1,0x0000, 0x00ff);
	deco_bac06_pf_control_0_w(state->m_tilegen2,2,0x0000, 0x00ff);
	deco_bac06_pf_control_0_w(state->m_tilegen2,3,0x0001, 0x00ff); // dimensions

	deco_bac06_pf_control_0_w(state->m_tilegen3,0,0x0082, 0x00ff); // 16x16
	deco_bac06_pf_control_0_w(state->m_tilegen3,1,0x0003, 0x00ff);
	deco_bac06_pf_control_0_w(state->m_tilegen3,2,0x0000, 0x00ff);
	deco_bac06_pf_control_0_w(state->m_tilegen3,3,0x0001, 0x00ff); // dimensions

	// scroll registers got written elsewhere, copy them across
	deco_bac06_pf_control_1_w(state->m_tilegen1,0,0x0000, 0xffff); // no scroll?
	deco_bac06_pf_control_1_w(state->m_tilegen1,1,0x0000, 0xffff); // no scroll?

	deco_bac06_pf_control_1_w(state->m_tilegen2,0,state->m_automat_scroll_regs[3] - 0x010a, 0xffff);
	deco_bac06_pf_control_1_w(state->m_tilegen2,1,state->m_automat_scroll_regs[2], 0xffff);

	deco_bac06_pf_control_1_w(state->m_tilegen3,0,state->m_automat_scroll_regs[1] - 0x0108, 0xffff);
	deco_bac06_pf_control_1_w(state->m_tilegen3,1,state->m_automat_scroll_regs[0], 0xffff);

	state->flip_screen_set(state->m_tilegen1->get_flip_state());

	state->m_tilegen3->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);
	state->m_tilegen2->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);

	state->m_spritegen->draw_sprites_bootleg(screen.machine(), bitmap, cliprect, state->m_buffered_spriteram, 0x00, 0x00, 0x0f);

	/* Redraw top 8 pens of top 8 palettes over sprites */
	if (state->m_pri&0x80)
		state->m_tilegen2->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0,0x08,0x08,0x08,0x08); // upper 8 pens of upper 8 priority marked tiles

	state->m_tilegen1->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	return 0;
}


/******************************************************************************/

SCREEN_UPDATE_IND16( birdtry )
{
	dec0_state *state = screen.machine().driver_data<dec0_state>();

	state->flip_screen_set(state->m_tilegen1->get_flip_state());

	/* This game doesn't have the extra playfield chip on the game board, but
    the palette does show through. */
	bitmap.fill(screen.machine().pens[768], cliprect);
	state->m_tilegen2->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	state->m_spritegen->draw_sprites(screen.machine(), bitmap, cliprect, state->m_buffered_spriteram, 0x00, 0x00, 0x0f);
	state->m_tilegen1->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	return 0;
}

/******************************************************************************/

SCREEN_UPDATE_IND16( hippodrm )
{
	dec0_state *state = screen.machine().driver_data<dec0_state>();
	state->flip_screen_set(state->m_tilegen1->get_flip_state());

	if (state->m_pri & 0x01)
	{
		state->m_tilegen2->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);
		state->m_tilegen3->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	}
	else
	{
		state->m_tilegen3->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);
		state->m_tilegen2->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	}

	state->m_spritegen->draw_sprites(screen.machine(), bitmap, cliprect, state->m_buffered_spriteram, 0x00, 0x00, 0x0f);
	state->m_tilegen1->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	return 0;
}

/******************************************************************************/

SCREEN_UPDATE_IND16( slyspy )
{
	dec0_state *state = screen.machine().driver_data<dec0_state>();
	state->flip_screen_set(state->m_tilegen1->get_flip_state());

	state->m_tilegen3->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);
	state->m_tilegen2->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);

	state->m_spritegen->draw_sprites(screen.machine(), bitmap, cliprect, state->m_buffered_spriteram, 0x00, 0x00, 0x0f);

	/* Redraw top 8 pens of top 8 palettes over sprites */
	if (state->m_pri&0x80)
		state->m_tilegen2->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0,0x08,0x08,0x08,0x08); // upper 8 pens of upper 8 priority marked tiles

	state->m_tilegen1->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	return 0;
}

/******************************************************************************/

SCREEN_UPDATE_IND16( midres )
{
	dec0_state *state = screen.machine().driver_data<dec0_state>();
	int trans;

	state->flip_screen_set(state->m_tilegen1->get_flip_state());

	if (state->m_pri & 0x04)
		trans = 0x00;
	else trans = 0x08;

	if (state->m_pri & 0x01)
	{
		state->m_tilegen2->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);

		if (state->m_pri & 0x02)
			state->m_spritegen->draw_sprites(screen.machine(), bitmap, cliprect, state->m_buffered_spriteram, 0x08, trans, 0x0f);

		state->m_tilegen3->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	}
	else
	{
		state->m_tilegen3->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0x00, 0x00, 0x00, 0x00);

		if (state->m_pri & 0x02)
			state->m_spritegen->draw_sprites(screen.machine(), bitmap, cliprect, state->m_buffered_spriteram, 0x08, trans, 0x0f);

		state->m_tilegen2->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
	}

	if (state->m_pri & 0x02)
		state->m_spritegen->draw_sprites(screen.machine(), bitmap, cliprect, state->m_buffered_spriteram, 0x08, trans ^ 0x08, 0x0f);
	else
		state->m_spritegen->draw_sprites(screen.machine(), bitmap, cliprect, state->m_buffered_spriteram, 0x00, 0x00, 0x0f);

	state->m_tilegen1->deco_bac06_pf_draw(screen.machine(),bitmap,cliprect,0, 0x00, 0x00, 0x00, 0x00);
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

VIDEO_START( automat )
{
//	dec0_state *state = machine.driver_data<dec0_state>();
}

/******************************************************************************/
