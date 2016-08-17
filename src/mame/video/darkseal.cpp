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

void darkseal_state::update_24bitcol(int offset)
{
	int r,g,b;

	r = (m_generic_paletteram_16[offset] >> 0) & 0xff;
	g = (m_generic_paletteram_16[offset] >> 8) & 0xff;
	b = (m_generic_paletteram2_16[offset] >> 0) & 0xff;

	m_palette->set_pen_color(offset,rgb_t(r,g,b));
}

WRITE16_MEMBER(darkseal_state::palette_24bit_rg_w)
{
	COMBINE_DATA(&m_generic_paletteram_16[offset]);
	update_24bitcol(offset);
}

WRITE16_MEMBER(darkseal_state::palette_24bit_b_w)
{
	COMBINE_DATA(&m_generic_paletteram2_16[offset]);
	update_24bitcol(offset);
}

/******************************************************************************/

void darkseal_state::video_start()
{
}

/******************************************************************************/

UINT32 darkseal_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	address_space &space = machine().driver_data()->generic_space();
	UINT16 flip = m_deco_tilegen2->pf_control_r(space, 0, 0xffff);
	flip_screen_set(!BIT(flip, 7));

	bitmap.fill(m_palette->black_pen(), cliprect);

	m_deco_tilegen1->pf_update(m_pf1_rowscroll, m_pf1_rowscroll);
	m_deco_tilegen2->pf_update(m_pf3_rowscroll, m_pf3_rowscroll);

	m_deco_tilegen2->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
	m_deco_tilegen2->tilemap_2_draw(screen, bitmap, cliprect, 0, 0);

	m_deco_tilegen1->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
	m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram->buffer(), 0x400);
	m_deco_tilegen1->tilemap_2_draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

/******************************************************************************/
