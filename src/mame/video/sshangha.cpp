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

uint32_t sshangha_state::screen_update_sshangha(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const int combine_tilemaps = (m_video_control&4) ? 0 : 1;

	// sprites are flipped relative to tilemaps
	address_space &space = machine().dummy_space();
	uint16_t flip = m_deco_tilegen1->pf_control_r(space, 0, 0xffff);
	flip_screen_set(BIT(flip, 7));
	m_sprgen1->set_flip_screen(!BIT(flip, 7));
	m_sprgen2->set_flip_screen(!BIT(flip, 7));

	// render sprites to temp bitmaps
	m_sprgen1->draw_sprites(bitmap, cliprect, m_spriteram, 0x400);
	m_sprgen2->draw_sprites(bitmap, cliprect, m_spriteram2, 0x400);

	// draw / mix
	bitmap.fill(m_palette->black_pen(), cliprect);

	m_deco_tilegen1->pf_update(m_pf1_rowscroll, m_pf2_rowscroll);

	// TODO: fully verify draw order / priorities

	/* the tilemap 4bpp + 4bpp = 8bpp mixing actually seems external to the tilemap, note video_control is not part of the tilemap chip */
	if (combine_tilemaps) {
		m_deco_tilegen1->tilemap_12_combine_draw(screen, bitmap, cliprect, 0, 0, 1);
	}
	else {
		m_deco_tilegen1->tilemap_2_draw(screen, bitmap, cliprect, 0, 0);
	}
	//                                                          pri,   primask,palbase,palmask
	m_sprgen1->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x200, 0x200,  0x000,  0x0ff); // low pri spr1
	m_sprgen2->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x200, 0x200,  0x100,  0x0ff); // low pri spr2  (definitely needs to be below tilemap1 - 2nd level failure screen etc.)

	if (!combine_tilemaps)
		m_deco_tilegen1->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);

	//                                                          pri,   primask,palbase,palmask
	m_sprgen2->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x000, 0x200,  0x100,  0x0ff); // high pri spr2 (definitely needs to be above tilemap1 - title logo)
	m_sprgen1->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x000, 0x200,  0x000,  0x0ff); // high pri spr1

	return 0;
}
