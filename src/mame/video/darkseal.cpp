// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
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

/***************************************************************************/

/******************************************************************************/

void darkseal_state::update_palette(int offset)
{
	int r,g,b;

	// TODO : Values aren't write in game when higher than 0xf0,
	// It's related from hardware colour resistors?
	r = (m_paletteram[offset] >> 0) & 0xff;
	g = (m_paletteram[offset] >> 8) & 0xff;
	b = (m_paletteram_ext[offset] >> 0) & 0xff;

	m_palette->set_pen_color(offset,rgb_t(r,g,b));
}

WRITE16_MEMBER(darkseal_state::palette_w)
{
	COMBINE_DATA(&m_paletteram[offset]);
	update_palette(offset);
}

WRITE16_MEMBER(darkseal_state::palette_ext_w)
{
	COMBINE_DATA(&m_paletteram_ext[offset]);
	update_palette(offset);
}

/******************************************************************************/

uint32_t darkseal_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t flip = m_deco_tilegen[1]->pf_control_r(0);
	flip_screen_set(!BIT(flip, 7));
	m_sprgen->set_flip_screen(!BIT(flip, 7));

	bitmap.fill(m_palette->black_pen(), cliprect);

	m_deco_tilegen[0]->pf_update(m_pf1_rowscroll, m_pf1_rowscroll);
	m_deco_tilegen[1]->pf_update(m_pf3_rowscroll, m_pf3_rowscroll);

	m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
	m_deco_tilegen[1]->tilemap_2_draw(screen, bitmap, cliprect, 0, 0);

	m_deco_tilegen[0]->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
	m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram->buffer(), 0x400);
	m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

/******************************************************************************/
