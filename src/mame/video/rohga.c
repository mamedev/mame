// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

    Rohga Video emulation - Bryan McPhail, mish@tendril.co.uk

***************************************************************************/

#include "emu.h"
#include "includes/rohga.h"


WRITE16_MEMBER(rohga_state::rohga_buffer_spriteram16_w)
{
	// Spriteram seems to be triple buffered (no sprite lag on real pcb, but there
	// is on driver with only double buffering)
	m_spriteram->copy();
}

/******************************************************************************/

UINT32 rohga_state::screen_update_rohga(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	address_space &space = machine().driver_data()->generic_space();
	UINT16 flip = m_deco_tilegen1->pf_control_r(space, 0, 0xffff);
	UINT16 priority = m_decocomn->priority_r(space, 0, 0xffff);

	/* Update playfields */
	flip_screen_set(BIT(flip, 7));
	m_deco_tilegen1->pf_update(m_pf1_rowscroll, m_pf2_rowscroll);
	m_deco_tilegen2->pf_update(m_pf3_rowscroll, m_pf4_rowscroll);

	/* Draw playfields */
	screen.priority().fill(0, cliprect);
	bitmap.fill(m_palette->pen(768), cliprect);

	switch (priority & 3)
	{
	case 0:
		if (priority & 4)
		{
			// Draw as 1 8BPP layer
			m_deco_tilegen2->tilemap_12_combine_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 3);
		}
		else
		{
			// Draw as 2 4BPP layers
			m_deco_tilegen2->tilemap_2_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 1);
			m_deco_tilegen2->tilemap_1_draw(screen, bitmap, cliprect, 0, 2);
		}
		m_deco_tilegen1->tilemap_2_draw(screen, bitmap, cliprect, 0, 4);
		break;
	case 1:
		m_deco_tilegen2->tilemap_2_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 1);
		m_deco_tilegen1->tilemap_2_draw(screen, bitmap, cliprect, 0, 2);
		m_deco_tilegen2->tilemap_1_draw(screen, bitmap, cliprect, 0, 4);
		break;
	case 2:
		m_deco_tilegen1->tilemap_2_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 1);
		m_deco_tilegen2->tilemap_2_draw(screen, bitmap, cliprect, 0, 2);
		m_deco_tilegen2->tilemap_1_draw(screen, bitmap, cliprect, 0, 4);
		break;
	}

	m_sprgen1->draw_sprites(bitmap, cliprect, m_spriteram->buffer(), 0x400, true);
	m_deco_tilegen1->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}



VIDEO_START_MEMBER(rohga_state,wizdfire)
{
	m_sprgen1->alloc_sprite_bitmap();
	m_sprgen2->alloc_sprite_bitmap();
}

// not amazingly efficient, called multiple times to pull a layer out of the sprite bitmaps, but keeps correct sprite<->sprite priorities
void rohga_state::mixwizdfirelayer(bitmap_rgb32 &bitmap, const rectangle &cliprect, int gfxregion, UINT16 pri, UINT16 primask)
{
	int y, x;
	const pen_t *paldata = m_palette->pens();
	bitmap_ind16* sprite_bitmap;
	int penbase;

	sprite_bitmap = &m_sprgen2->get_sprite_temp_bitmap();
	penbase = 0x600;

	UINT16* srcline;
	UINT32* dstline;


	for (y=cliprect.min_y;y<=cliprect.max_y;y++)
	{
		srcline=&sprite_bitmap->pix16(y,0);
		dstline=&bitmap.pix32(y,0);

		for (x=cliprect.min_x;x<=cliprect.max_x;x++)
		{
			UINT16 pix = srcline[x];

			if ((pix & primask) != pri)
				continue;

			if (pix&0xf)
			{
				UINT16 pen = pix&0x1ff;

				if (pen&0x100)
				{
					UINT32 base = dstline[x];
					pen &=0xff;
					dstline[x] = alpha_blend_r32(base, paldata[pen+penbase], 0x80);
				}
				else
				{
					dstline[x] = paldata[pen+penbase];
				}
			}
		}
	}
}

UINT32 rohga_state::screen_update_wizdfire(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	address_space &space = machine().driver_data()->generic_space();
	UINT16 flip = m_deco_tilegen1->pf_control_r(space, 0, 0xffff);
	UINT16 priority = m_decocomn->priority_r(space, 0, 0xffff);

	/* draw sprite gfx to temp bitmaps */
	m_sprgen2->draw_sprites(bitmap, cliprect, m_spriteram2->buffer(), 0x400, true);
	m_sprgen1->draw_sprites(bitmap, cliprect, m_spriteram->buffer(), 0x400, true);

	/* Update playfields */
	flip_screen_set(BIT(flip, 7));
	m_deco_tilegen1->pf_update(0, 0);
	m_deco_tilegen2->pf_update(m_pf3_rowscroll, m_pf4_rowscroll);

	/* Draw playfields - Palette of 2nd playfield chip visible if playfields turned off */
	bitmap.fill(m_palette->pen(512), cliprect);

	m_deco_tilegen2->tilemap_2_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	m_sprgen1->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0600, 0x0600, 0x400, 0x1ff);
	m_deco_tilegen1->tilemap_2_draw(screen, bitmap, cliprect, 0, 0);
	m_sprgen1->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0400, 0x0600, 0x400, 0x1ff);

	if ((priority & 0x1f) == 0x1f) /* Wizdfire has bit 0x40 always set, Dark Seal 2 doesn't?! */
		m_deco_tilegen2->tilemap_1_draw(screen, bitmap, cliprect, TILEMAP_DRAW_ALPHA(0x80), 0);
	else
		m_deco_tilegen2->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);

	m_sprgen1->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0000, 0x0400, 0x400, 0x1ff); // 0x000 and 0x200 of 0x600

	mixwizdfirelayer(bitmap, cliprect, 4, 0x000, 0x000);

	m_deco_tilegen1->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

UINT32 rohga_state::screen_update_nitrobal(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	address_space &space = machine().driver_data()->generic_space();
	UINT16 flip = m_deco_tilegen1->pf_control_r(space, 0, 0xffff);

	/* draw sprite gfx to temp bitmaps */
	m_sprgen1->set_alt_format(true);
	m_sprgen2->set_alt_format(true);
	m_sprgen2->draw_sprites(bitmap, cliprect, m_spriteram2->buffer(), 0x400, false);
	m_sprgen1->draw_sprites(bitmap, cliprect, m_spriteram->buffer(), 0x400, false);

	/* Update playfields */
	flip_screen_set(BIT(flip, 7));
	m_deco_tilegen1->pf_update(m_pf1_rowscroll, m_pf2_rowscroll);
	m_deco_tilegen2->pf_update(m_pf3_rowscroll, m_pf4_rowscroll);

	/* Draw playfields - Palette of 2nd playfield chip visible if playfields turned off */
	bitmap.fill(m_palette->pen(512), cliprect);
	screen.priority().fill(0);

	/* pf3 and pf4 are combined into a single 8bpp bitmap */
	m_deco_tilegen2->tilemap_12_combine_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);

	m_deco_tilegen1->tilemap_2_draw(screen, bitmap, cliprect, 0, 16);

	/* ToDo reimplement priorities + mixing / alpha, it was busted worse than this before anyway, so no big loss that we don't do it for now ;-) */
	m_sprgen2->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0000, 0x0000, 0x600, 0xff);
	m_sprgen1->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0000, 0x0000, 0x400, 0x1ff);


	m_deco_tilegen1->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
