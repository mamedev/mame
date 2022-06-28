// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

   Vapour Trail Video emulation - Bryan McPhail, mish@tendril.co.uk

****************************************************************************

    2 Data East 55 chips for playfields (same as Dark Seal, etc)
    1 Data East MXC-06 chip for sprites (same as Bad Dudes, etc)

***************************************************************************/

#include "emu.h"
#include "vaportra.h"

/******************************************************************************/

void vaportra_state::priority_w(offs_t offset, uint16_t data, uint16_t mem_mask)
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

void vaportra_state::palette_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_paletteram[offset]);
	update_palette(offset);
}

void vaportra_state::palette_ext_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_paletteram_ext[offset]);
	update_palette(offset);
}

/******************************************************************************/

void vaportra_state::vaportra_colpri_cb(u32 &colour, u32 &pri_mask)
{
	pri_mask = 0; // above back, mid, foreground
	if (colour >= m_priority[1])
	{
		pri_mask |= GFX_PMASK_4; // behind foreground
	}
}

uint32_t vaportra_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t flip = m_deco_tilegen[0]->pf_control_r(0);
	int pri = m_priority[0] & 0x03;

	screen.priority().fill(0, cliprect);
	flip_screen_set(!BIT(flip, 7));
	m_deco_tilegen[0]->pf_update(nullptr, nullptr);
	m_deco_tilegen[1]->pf_update(nullptr, nullptr);

	m_spritegen->set_flip_screen(!BIT(flip, 7));

	/* Draw playfields */
	if (pri == 0)
	{
		m_deco_tilegen[1]->tilemap_2_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 1);
		m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, 0, 2);
		m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 4);
	}
	else if (pri == 1)
	{
		m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 1);
		m_deco_tilegen[1]->tilemap_2_draw(screen, bitmap, cliprect, 0, 2);
		m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 4);
	}
	else if (pri == 2)
	{
		m_deco_tilegen[1]->tilemap_2_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 1);
		m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 2);
		m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, 0, 4);
	}
	else
	{
		m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 1);
		m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 2);
		m_deco_tilegen[1]->tilemap_2_draw(screen, bitmap, cliprect, 0, 4);
	}

	m_spritegen->draw_sprites(screen, bitmap, cliprect, m_gfxdecode->gfx(4), m_spriteram->buffer(), 0x800 / 2);
	m_deco_tilegen[0]->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
