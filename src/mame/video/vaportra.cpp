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

WRITE16_MEMBER(vaportra_state::vaportra_priority_w)
{
	COMBINE_DATA(&m_priority[offset]);
}

/******************************************************************************/

void vaportra_state::update_24bitcol( int offset )
{
	UINT8 r, g, b;

	r = (m_generic_paletteram_16[offset] >> 0) & 0xff;
	g = (m_generic_paletteram_16[offset] >> 8) & 0xff;
	b = (m_generic_paletteram2_16[offset] >> 0) & 0xff;

	m_palette->set_pen_color(offset, rgb_t(r,g,b));
}

WRITE16_MEMBER(vaportra_state::vaportra_palette_24bit_rg_w)
{
	COMBINE_DATA(&m_generic_paletteram_16[offset]);
	update_24bitcol(offset);
}

WRITE16_MEMBER(vaportra_state::vaportra_palette_24bit_b_w)
{
	COMBINE_DATA(&m_generic_paletteram2_16[offset]);
	update_24bitcol(offset);
}

/******************************************************************************/


UINT32 vaportra_state::screen_update_vaportra(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	address_space &space = machine().driver_data()->generic_space();
	UINT16 flip = m_deco_tilegen1->pf_control_r(space, 0, 0xffff);
	int pri = m_priority[0] & 0x03;

	flip_screen_set(!BIT(flip, 7));
	m_deco_tilegen1->pf_update(nullptr, nullptr);
	m_deco_tilegen2->pf_update(nullptr, nullptr);

	m_spritegen->set_pri_type(1); // force priorities to be handled in a different way for this driver for now

	/* Draw playfields */
	if (pri == 0)
	{
		m_deco_tilegen2->tilemap_2_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_deco_tilegen2->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
		m_spritegen->draw_sprites(bitmap, cliprect, m_spriteram->buffer(), 0, m_priority[1], 0x0f);
		m_deco_tilegen1->tilemap_2_draw(screen, bitmap, cliprect, 0, 0);
	}
	else if (pri == 1)
	{
		m_deco_tilegen2->tilemap_1_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_deco_tilegen2->tilemap_2_draw(screen, bitmap, cliprect, 0, 0);
		m_spritegen->draw_sprites(bitmap, cliprect, m_spriteram->buffer(), 0, m_priority[1], 0x0f);
		m_deco_tilegen1->tilemap_2_draw(screen, bitmap, cliprect, 0, 0);
	}
	else if (pri == 2)
	{
		m_deco_tilegen2->tilemap_2_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_deco_tilegen1->tilemap_2_draw(screen, bitmap, cliprect, 0, 0);
		m_spritegen->draw_sprites(bitmap, cliprect, m_spriteram->buffer(), 0, m_priority[1], 0x0f);
		m_deco_tilegen2->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
	}
	else
	{
		m_deco_tilegen2->tilemap_1_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
		m_deco_tilegen1->tilemap_2_draw(screen, bitmap, cliprect, 0, 0);
		m_spritegen->draw_sprites(bitmap, cliprect, m_spriteram->buffer(), 0, m_priority[1], 0x0f);
		m_deco_tilegen2->tilemap_2_draw(screen, bitmap, cliprect, 0, 0);
	}

	m_spritegen->draw_sprites(bitmap, cliprect, m_spriteram->buffer(), 1, m_priority[1], 0x0f);
	m_deco_tilegen1->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
