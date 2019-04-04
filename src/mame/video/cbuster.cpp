// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

   Crude Buster Video emulation - Bryan McPhail, mish@tendril.co.uk

***************************************************************************/

#include "emu.h"
#include "includes/cbuster.h"

/******************************************************************************/

/******************************************************************************/

void cbuster_state::video_start()
{
	m_sprgen->alloc_sprite_bitmap();
}

/*
    Crude Buster palette is a little strange compared to other Data East games
    of this period.  Although the digital palette is 8 bits per channel, the
    analog 'white' level is set at 0x8e.  In hardware this is done at the
    final resistors before the JAMMA connector.  It also suggests that if the
    game were to use any values above 0x8e (it doesn't) then the final output
    voltage would be out of spec.

    I suspect this setup is actually software compensating for a hardware
    design problem.
*/

void cbuster_state::update_palette(int offset)
{
	int r = m_paletteram[offset]&0xff;
	int g = m_paletteram[offset]>>8;
	int b = m_paletteram_ext[offset]&0xff;

	if (r>0x8e) r=0x8e;
	if (g>0x8e) g=0x8e;
	if (b>0x8e) b=0x8e;

	r = (r * 255) / 0x8e;
	g = (g * 255) / 0x8e;
	b = (b * 255) / 0x8e;

	m_palette->set_pen_color(offset,rgb_t(r,g,b));
}

WRITE16_MEMBER(cbuster_state::palette_w)
{
	COMBINE_DATA(&m_paletteram[offset]);
	update_palette(offset);
}

WRITE16_MEMBER(cbuster_state::palette_ext_w)
{
	COMBINE_DATA(&m_paletteram_ext[offset]);
	update_palette(offset);
}

uint32_t cbuster_state::screen_update_twocrude(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint16_t flip = m_deco_tilegen[0]->pf_control_r(0);

	flip_screen_set(!BIT(flip, 7));
	m_sprgen->set_flip_screen(!BIT(flip, 7));

	m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram->buffer(), 0x400);

	m_deco_tilegen[0]->pf_update(m_pf_rowscroll[0], m_pf_rowscroll[1]);
	m_deco_tilegen[1]->pf_update(m_pf_rowscroll[2], m_pf_rowscroll[3]);

	/* Draw playfields & sprites */
	m_deco_tilegen[1]->tilemap_2_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	m_sprgen->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0800, 0x0900, 0x100, 0x0ff);
	m_sprgen->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0900, 0x0900, 0x500, 0x0ff);

	if (m_pri)
	{
		m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 0);
		m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
	}
	else
	{
		m_deco_tilegen[1]->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
		m_deco_tilegen[0]->tilemap_2_draw(screen, bitmap, cliprect, 0, 0);
	}

	m_sprgen->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0000, 0x0900, 0x100, 0x0ff);
	m_sprgen->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0100, 0x0900, 0x500, 0x0ff);
	m_deco_tilegen[0]->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
