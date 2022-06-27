// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

  Dec0 Video emulation - Bryan McPhail, mish@tendril.co.uk

*********************************************************************/

#include "emu.h"
#include "dec0.h"


/******************************************************************************/

/******************************************************************************/

void dec0_state::hbarrel_colpri_cb(u32 &colour, u32 &pri_mask)
{
	pri_mask = GFX_PMASK_4; // above background, foreground
	if (colour & 8)
	{
		pri_mask |= GFX_PMASK_2; // behind foreground
	}
}

void dec0_state::bandit_colpri_cb(u32 &colour, u32 &pri_mask)
{
	pri_mask = 0; // above all
	if (m_pri == 0)
	{
		pri_mask |= GFX_PMASK_4; // behind foreground
	}
}

/* HB always keeps pf2 on top of pf3, no need explicitly support priority register */
uint32_t dec0_state::screen_update_hbarrel(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0,cliprect);
	bool flip = m_tilegen[0]->get_flip_state();
	m_tilegen[0]->set_flip_screen(flip);
	m_tilegen[1]->set_flip_screen(flip);
	m_tilegen[2]->set_flip_screen(flip);
	m_spritegen->set_flip_screen(flip);

	m_tilegen[2]->deco_bac06_pf_draw(screen,bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 1);
	m_tilegen[1]->deco_bac06_pf_draw(screen,bitmap,cliprect,0, 2);
	m_tilegen[0]->deco_bac06_pf_draw(screen,bitmap,cliprect,0, 4);
	m_spritegen->draw_sprites(screen, bitmap, cliprect, m_gfxdecode->gfx(3), m_buffered_spriteram, 0x800/2);
	return 0;
}

/******************************************************************************/

void dec0_state::baddudes_tile_cb(tile_data &tileinfo, u32 &tile, u32 &colour, u32 &flags)
{
	tileinfo.group = BIT(colour, 3) ? 1 : 0;
}

uint32_t dec0_state::screen_update_baddudes(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bool flip = m_tilegen[0]->get_flip_state();
	m_tilegen[0]->set_flip_screen(flip);
	m_tilegen[1]->set_flip_screen(flip);
	m_tilegen[2]->set_flip_screen(flip);
	m_spritegen->set_flip_screen(flip);

	/* WARNING: inverted wrt Midnight Resistance */
	const u8 fg = (m_pri & 0x01) ? 1 : 2;
	const u8 bg = (m_pri & 0x01) ? 2 : 1;
	m_tilegen[bg]->deco_bac06_pf_draw(screen,bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0);
	m_tilegen[fg]->deco_bac06_pf_draw(screen,bitmap,cliprect,TILEMAP_DRAW_LAYER0 | TILEMAP_DRAW_LAYER1, 0);

	if (m_pri & 2)
		m_tilegen[bg]->deco_bac06_pf_draw(screen,bitmap,cliprect,TILEMAP_DRAW_LAYER0, 0); // upper 8 pens of upper 8 priority marked tiles /* Foreground pens only */

	m_spritegen->draw_sprites(screen, bitmap, cliprect, m_gfxdecode->gfx(3), m_buffered_spriteram, 0x800/2);

	if (m_pri & 4)
		m_tilegen[fg]->deco_bac06_pf_draw(screen,bitmap,cliprect,TILEMAP_DRAW_LAYER0, 0); // upper 8 pens of upper 8 priority marked tiles /* Foreground pens only */

	m_tilegen[0]->deco_bac06_pf_draw(screen,bitmap,cliprect,0, 0);
	return 0;
}

/******************************************************************************/

/* WARNING: inverted wrt Midnight Resistance */
/* Robocop uses it only for the title screen, so this might be just */
/* completely wrong. The top 8 bits of the register might mean */
/* something (they are 0x80 in midres, 0x00 here) */
void dec0_state::robocop_colpri_cb(u32 &colour, u32 &pri_mask)
{
	pri_mask = 0; // above background, foreground
	if (m_pri & 0x02)
	{
		const u32 trans = (m_pri & 0x04) ? 0x08 : 0x00;
		if ((colour & 0x08) == trans)
			pri_mask |= GFX_PMASK_2; // behind foreground
	}
}

void dec0_state::midres_colpri_cb(u32 &colour, u32 &pri_mask)
{
	pri_mask = 0; // above background, foreground
	if (m_pri & 0x02)
	{
		const u32 trans = (m_pri & 0x04) ? 0x00 : 0x08;
		if ((colour & 0x08) == trans)
			pri_mask |= GFX_PMASK_2; // behind foreground
	}
}

uint32_t dec0_state::screen_update_robocop(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0,cliprect);

	bool flip = m_tilegen[0]->get_flip_state();
	m_tilegen[0]->set_flip_screen(flip);
	m_tilegen[1]->set_flip_screen(flip);
	m_tilegen[2]->set_flip_screen(flip);
	m_spritegen->set_flip_screen(flip);

	const u8 fg = (m_pri & 0x01) ? 2 : 1;
	const u8 bg = (m_pri & 0x01) ? 1 : 2;
	m_tilegen[bg]->deco_bac06_pf_draw(screen,bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 1);
	m_tilegen[fg]->deco_bac06_pf_draw(screen,bitmap,cliprect,0, 2);
	m_spritegen->draw_sprites(screen, bitmap, cliprect, m_gfxdecode->gfx(3), m_buffered_spriteram, 0x800/2);
	m_tilegen[0]->deco_bac06_pf_draw(screen,bitmap,cliprect,0, 0);
	return 0;
}

uint32_t dec0_automat_state::screen_update_automat(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0,cliprect);

	// layer enables seem different... where are they?

	// the bootleg doesn't write these registers, I think they're hardcoded?, so fake them for compatibility with our implementation..
	m_tilegen[0]->pf_control_0_w(0,0x0003, 0x00ff); // 8x8
	m_tilegen[0]->pf_control_0_w(1,0x0003, 0x00ff);
	m_tilegen[0]->pf_control_0_w(2,0x0000, 0x00ff);
	m_tilegen[0]->pf_control_0_w(3,0x0001, 0x00ff); // dimensions

	m_tilegen[1]->pf_control_0_w(0,0x0082, 0x00ff); // 16x16
	m_tilegen[1]->pf_control_0_w(1,0x0000, 0x00ff);
	m_tilegen[1]->pf_control_0_w(2,0x0000, 0x00ff);
	m_tilegen[1]->pf_control_0_w(3,0x0001, 0x00ff); // dimensions

	m_tilegen[2]->pf_control_0_w(0,0x0082, 0x00ff); // 16x16
	m_tilegen[2]->pf_control_0_w(1,0x0003, 0x00ff);
	m_tilegen[2]->pf_control_0_w(2,0x0000, 0x00ff);
	m_tilegen[2]->pf_control_0_w(3,0x0001, 0x00ff); // dimensions

	// scroll registers got written elsewhere, copy them across
	m_tilegen[0]->pf_control_1_w(0,0x0000, 0xffff); // no scroll?
	m_tilegen[0]->pf_control_1_w(1,0x0000, 0xffff); // no scroll?

	m_tilegen[1]->pf_control_1_w(0,m_automat_scroll_regs[3] - 0x010a, 0xffff);
	m_tilegen[1]->pf_control_1_w(1,m_automat_scroll_regs[2], 0xffff);

	m_tilegen[2]->pf_control_1_w(0,m_automat_scroll_regs[1] - 0x0108, 0xffff);
	m_tilegen[2]->pf_control_1_w(1,m_automat_scroll_regs[0], 0xffff);


	bool flip = m_tilegen[0]->get_flip_state();
	m_tilegen[0]->set_flip_screen(flip);
	m_tilegen[1]->set_flip_screen(flip);
	m_tilegen[2]->set_flip_screen(flip);
	m_spritegen->set_flip_screen(flip);

	const u8 fg = (m_pri & 0x01) ? 2 : 1;
	const u8 bg = (m_pri & 0x01) ? 1 : 2;
	m_tilegen[bg]->deco_bac06_pf_draw(screen,bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 1);
	m_tilegen[fg]->deco_bac06_pf_draw(screen,bitmap,cliprect,0, 2);
	m_spritegen->draw_sprites_bootleg(screen, bitmap, cliprect, m_gfxdecode->gfx(3), m_buffered_spriteram, 0x800/2); // TODO : RAM size
	m_tilegen[0]->deco_bac06_pf_draw(screen,bitmap,cliprect,0, 0);
	return 0;
}

/******************************************************************************/

uint32_t dec0_state::screen_update_birdtry(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bool flip = m_tilegen[0]->get_flip_state();
	m_tilegen[0]->set_flip_screen(flip);
	m_tilegen[1]->set_flip_screen(flip);
	m_spritegen->set_flip_screen(flip);

	/* This game doesn't have the extra playfield chip on the game board, but
	the palette does show through. */
	bitmap.fill(m_palette->pen(768), cliprect);
	m_tilegen[1]->deco_bac06_pf_draw(screen,bitmap,cliprect,0, 0);
	m_spritegen->draw_sprites(screen, bitmap, cliprect, m_gfxdecode->gfx(3), m_buffered_spriteram, 0x800/2);
	m_tilegen[0]->deco_bac06_pf_draw(screen,bitmap,cliprect,0, 0);
	return 0;
}

/******************************************************************************/

uint32_t dec0_state::screen_update_slyspy(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bool flip = m_tilegen[0]->get_flip_state();
	m_tilegen[0]->set_flip_screen(flip);
	m_tilegen[1]->set_flip_screen(flip);
	m_tilegen[2]->set_flip_screen(flip);
	m_spritegen->set_flip_screen(flip);

	m_tilegen[2]->deco_bac06_pf_draw(screen,bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0);
	m_tilegen[1]->deco_bac06_pf_draw(screen,bitmap,cliprect,TILEMAP_DRAW_LAYER0 | TILEMAP_DRAW_LAYER1, 0);

	m_spritegen->draw_sprites(screen, bitmap, cliprect, m_gfxdecode->gfx(3), m_buffered_spriteram, 0x800/2);

	/* Redraw top 8 pens of top 8 palettes over sprites */
	if (m_pri & 0x80)
		m_tilegen[1]->deco_bac06_pf_draw(screen,bitmap,cliprect,TILEMAP_DRAW_LAYER0, 0); // upper 8 pens of upper 8 priority marked tiles

	m_tilegen[0]->deco_bac06_pf_draw(screen,bitmap,cliprect,0,0);
	return 0;
}

uint32_t dec0_automat_state::screen_update_secretab(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// layer enables seem different... where are they?

	// the bootleg doesn't write these registers, I think they're hardcoded?, so fake them for compatibility with our implementation..
	m_tilegen[0]->pf_control_0_w(0,0x0003, 0x00ff); // 8x8
	m_tilegen[0]->pf_control_0_w(1,0x0003, 0x00ff);
	m_tilegen[0]->pf_control_0_w(2,0x0000, 0x00ff);
	m_tilegen[0]->pf_control_0_w(3,0x0001, 0x00ff); // dimensions

	m_tilegen[1]->pf_control_0_w(0,0x0082, 0x00ff); // 16x16
	m_tilegen[1]->pf_control_0_w(1,0x0000, 0x00ff);
	m_tilegen[1]->pf_control_0_w(2,0x0000, 0x00ff);
	m_tilegen[1]->pf_control_0_w(3,0x0001, 0x00ff); // dimensions

	m_tilegen[2]->pf_control_0_w(0,0x0082, 0x00ff); // 16x16
	m_tilegen[2]->pf_control_0_w(1,0x0003, 0x00ff);
	m_tilegen[2]->pf_control_0_w(2,0x0000, 0x00ff);
	m_tilegen[2]->pf_control_0_w(3,0x0001, 0x00ff); // dimensions

	// scroll registers got written elsewhere, copy them across
	m_tilegen[0]->pf_control_1_w(0,0x0000, 0xffff); // no scroll?
	m_tilegen[0]->pf_control_1_w(1,0x0000, 0xffff); // no scroll?

	m_tilegen[1]->pf_control_1_w(0,m_automat_scroll_regs[3] - 0x010a, 0xffff);
	m_tilegen[1]->pf_control_1_w(1,m_automat_scroll_regs[2], 0xffff);

	m_tilegen[2]->pf_control_1_w(0,m_automat_scroll_regs[1] - 0x0108, 0xffff);
	m_tilegen[2]->pf_control_1_w(1,m_automat_scroll_regs[0], 0xffff);

	bool flip = m_tilegen[0]->get_flip_state();
	m_tilegen[0]->set_flip_screen(flip);
	m_tilegen[1]->set_flip_screen(flip);
	m_tilegen[2]->set_flip_screen(flip);
	m_spritegen->set_flip_screen(flip);

	m_tilegen[2]->deco_bac06_pf_draw(screen,bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0);
	m_tilegen[1]->deco_bac06_pf_draw(screen,bitmap,cliprect,TILEMAP_DRAW_LAYER0 | TILEMAP_DRAW_LAYER1, 0);

	m_spritegen->draw_sprites_bootleg(screen, bitmap, cliprect, m_gfxdecode->gfx(3), m_buffered_spriteram, 0x800/2); // TODO : RAM size

	/* Redraw top 8 pens of top 8 palettes over sprites */
	if (m_pri & 0x80)
		m_tilegen[1]->deco_bac06_pf_draw(screen,bitmap,cliprect,TILEMAP_DRAW_LAYER0, 0); // upper 8 pens of upper 8 priority marked tiles

	m_tilegen[0]->deco_bac06_pf_draw(screen,bitmap,cliprect,0,0);
	return 0;
}

/******************************************************************************/

void dec0_state::priority_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_pri);
}

VIDEO_START_MEMBER(dec0_state,dec0_nodma)
{
	save_item(NAME(m_pri));
	m_buffered_spriteram = m_spriteram->live();
	save_pointer(NAME(m_buffered_spriteram), 0x800/2);

	m_pri = 0;
}

VIDEO_START_MEMBER(dec0_state,dec0)
{
	save_item(NAME(m_pri));
	m_buffered_spriteram = m_spriteram->buffer();
	save_pointer(NAME(m_buffered_spriteram), 0x800/2);

	m_pri = 0;
}

VIDEO_START_MEMBER(dec0_state,baddudes)
{
	VIDEO_START_CALL_MEMBER(dec0);
	m_tilegen[1]->set_transmask(0, 0xffff, 0x0001);
	m_tilegen[1]->set_transmask(1, 0x00ff, 0xff01);
	m_tilegen[2]->set_transmask(0, 0xffff, 0x0001);
	m_tilegen[2]->set_transmask(1, 0x00ff, 0xff01);
}

VIDEO_START_MEMBER(dec0_state,slyspy)
{
	VIDEO_START_CALL_MEMBER(dec0_nodma);
	m_tilegen[1]->set_transmask(0, 0xffff, 0x0001);
	m_tilegen[1]->set_transmask(1, 0x00ff, 0xff01);
}

/******************************************************************************/
