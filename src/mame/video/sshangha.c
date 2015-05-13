// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Charles MacDonald, David Haywood
/***************************************************************************

    Uses Data East custom chip 55 for backgrounds, with a special 8bpp mode
    2 times custom chips 52/71 for sprites.

***************************************************************************/

#include "emu.h"
#include "includes/sshangha.h"


/******************************************************************************/

WRITE16_MEMBER(sshangha_state::sshangha_video_w)
{
	/* 0x4: Special video mode, other bits unknown */
	m_video_control=data;
//  popmessage("%04x",data);
}

/******************************************************************************/

void sshangha_state::video_start()
{
	m_sprgen1->alloc_sprite_bitmap();
	m_sprgen2->alloc_sprite_bitmap();
}

/******************************************************************************/

UINT32 sshangha_state::screen_update_sshangha(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_sprgen1->draw_sprites(bitmap, cliprect, m_spriteram, 0x800, true);

	// I'm pretty sure only the original has the 2nd spriteram, used for the Japanese text on the 2nd scene (non-scrolling text) in the intro of the quest (3rd in JPN) mode
	if (m_spriteram2 != NULL)
		m_sprgen2->draw_sprites(bitmap, cliprect, m_spriteram2, 0x800, true);

	// flip screen
	address_space &space = machine().driver_data()->generic_space();
	UINT16 flip = m_deco_tilegen1->pf_control_r(space, 0, 0xffff);
	flip_screen_set(BIT(flip, 7));

	bitmap.fill(m_palette->black_pen(), cliprect);

	m_deco_tilegen1->pf_update(m_pf1_rowscroll, m_pf2_rowscroll);

	/* the tilemap 4bpp + 4bpp = 8bpp mixing actually seems external to the tilemap, note video_control is not part of the tilemap chip */
	if ((m_video_control&4)==0) {
		m_deco_tilegen1->tilemap_12_combine_draw(screen, bitmap, cliprect, 0, 0, 1);
		m_sprgen1->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0200, 0x0200, 0x100, 0x1ff);
	}
	else {
		m_deco_tilegen1->tilemap_2_draw(screen, bitmap, cliprect, 0, 0);
		m_sprgen1->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0200, 0x0200, 0x100, 0x1ff);
		m_deco_tilegen1->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
	}

	if (m_spriteram2 != NULL)
		m_sprgen2->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0000, 0x0000, 0, 0x1ff);

	m_sprgen1->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0000, 0x0200, 0, 0x1ff);
	return 0;
}
