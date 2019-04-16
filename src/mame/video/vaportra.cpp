// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

   Vapour Trail Video emulation - Bryan McPhail, mish@tendril.co.uk

****************************************************************************

    2 Data East 55 chips for playfields (same as Dark Seal, etc)
    1 Data East MXC-06 chip for sprites (same as Bad Dudes, etc)

***************************************************************************/

#include "emu.h"
#include "includes/vaportra.h"

/******************************************************************************/

WRITE16_MEMBER(vaportra_state::priority_w)
{
	COMBINE_DATA(&m_priority[offset]);
}

/******************************************************************************/

void vaportra_state::update_palette( int offset )
{
	uint8_t r, g, b;

	// TODO : Values aren't write in game when higher than 0xf0,
	// It's related from hardware colour resistors?
	r = (m_paletteram[offset] >> 0) & 0xff;
	g = (m_paletteram[offset] >> 8) & 0xff;
	b = (m_paletteram_ext[offset] >> 0) & 0xff;

	m_palette->set_pen_color(offset, rgb_t(r,g,b));
}

WRITE16_MEMBER(vaportra_state::palette_w)
{
	COMBINE_DATA(&m_paletteram[offset]);
	update_palette(offset);
}

WRITE16_MEMBER(vaportra_state::palette_ext_w)
{
	COMBINE_DATA(&m_paletteram_ext[offset]);
	update_palette(offset);
}

/******************************************************************************/


uint32_t vaportra_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t flip = m_deco_tilegen[0]->pf_control_r(0);
	int pri = m_priority[0] & 0x03;

	flip_screen_set(!BIT(flip, 7));
	m_deco_tilegen[0]->pf_update(nullptr, nullptr);
	m_deco_tilegen[1]->pf_update(nullptr, nullptr);

	m_spritegen->set_flip_screen(!BIT(flip, 7));
	m_spritegen->set_pri_type(1); // force priorities to be handled in a different way for this driver for now

	/* Draw playfields */
	if (pri == 0)
	{
		m_deco_tilegen[1]->tilemap_2_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
		m_spritegen->draw_sprites(bitmap, cliprect, m_spriteram->buffer(), 0, m_priority[1], 0x0f);
		m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 0);
	}
	else if (pri == 1)
	{
		m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_deco_tilegen[1]->tilemap_2_draw(screen, bitmap, cliprect, 0, 0);
		m_spritegen->draw_sprites(bitmap, cliprect, m_spriteram->buffer(), 0, m_priority[1], 0x0f);
		m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 0);
	}
	else if (pri == 2)
	{
		m_deco_tilegen[1]->tilemap_2_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 0);
		m_spritegen->draw_sprites(bitmap, cliprect, m_spriteram->buffer(), 0, m_priority[1], 0x0f);
		m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
	}
	else
	{
		m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 0);
		m_spritegen->draw_sprites(bitmap, cliprect, m_spriteram->buffer(), 0, m_priority[1], 0x0f);
		m_deco_tilegen[1]->tilemap_2_draw(screen, bitmap, cliprect, 0, 0);
	}

	m_spritegen->draw_sprites(bitmap, cliprect, m_spriteram->buffer(), 1, m_priority[1], 0x0f);
	m_deco_tilegen[0]->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
