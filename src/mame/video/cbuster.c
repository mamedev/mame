/***************************************************************************

   Crude Buster Video emulation - Bryan McPhail, mish@tendril.co.uk

***************************************************************************/

#include "emu.h"
#include "includes/cbuster.h"

/******************************************************************************/

/* maybe the game should just use generic palette handling, and have a darker palette by design... */

void cbuster_state::update_24bitcol( int offset )
{
	UINT8 r, g, b; /* The highest palette value seems to be 0x8e */

	r = (UINT8)((float)((m_generic_paletteram_16[offset]  >> 0) & 0xff) * 1.75);
	g = (UINT8)((float)((m_generic_paletteram_16[offset]  >> 8) & 0xff) * 1.75);
	b = (UINT8)((float)((m_generic_paletteram2_16[offset] >> 0) & 0xff) * 1.75);

	m_palette->set_pen_color(offset, rgb_t(r, g, b));
}

WRITE16_MEMBER(cbuster_state::twocrude_palette_24bit_rg_w)
{
	COMBINE_DATA(&m_generic_paletteram_16[offset]);
	update_24bitcol(offset);
}

WRITE16_MEMBER(cbuster_state::twocrude_palette_24bit_b_w)
{
	COMBINE_DATA(&m_generic_paletteram2_16[offset]);
	update_24bitcol(offset);
}


/******************************************************************************/


/******************************************************************************/

void cbuster_state::video_start()
{
	m_sprgen->alloc_sprite_bitmap();
}

UINT32 cbuster_state::screen_update_twocrude(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	address_space &space = machine().driver_data()->generic_space();
	UINT16 flip = m_deco_tilegen1->pf_control_r(space, 0, 0xffff);

	flip_screen_set(!BIT(flip, 7));

	m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram16_buffer, 0x400);


	m_deco_tilegen1->pf_update(m_pf1_rowscroll, m_pf2_rowscroll);
	m_deco_tilegen2->pf_update(m_pf3_rowscroll, m_pf4_rowscroll);

	/* Draw playfields & sprites */
	m_deco_tilegen2->tilemap_2_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	m_sprgen->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0800, 0x0900, 0x100, 0x0ff);
	m_sprgen->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0900, 0x0900, 0x500, 0x0ff);

	if (m_pri)
	{
		m_deco_tilegen1->tilemap_2_draw(screen, bitmap, cliprect, 0, 0);
		m_deco_tilegen2->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
	}
	else
	{
		m_deco_tilegen2->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
		m_deco_tilegen1->tilemap_2_draw(screen, bitmap, cliprect, 0, 0);
	}

	m_sprgen->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0000, 0x0900, 0x100, 0x0ff);
	m_sprgen->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0100, 0x0900, 0x500, 0x0ff);
	m_deco_tilegen1->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
