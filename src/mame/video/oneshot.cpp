// license:BSD-3-Clause
// copyright-holders:David Haywood, Paul Priest
/* One Shot One Kill Video Hardware */

#include "emu.h"
#include "includes/oneshot.h"


/* bg tilemap */
TILE_GET_INFO_MEMBER(oneshot_state::get_oneshot_bg_tile_info)
{
	int tileno = m_bg_videoram[tile_index * 2 + 1];

	SET_TILE_INFO_MEMBER(0, tileno, 0, 0);
}

WRITE16_MEMBER(oneshot_state::oneshot_bg_videoram_w)
{
	COMBINE_DATA(&m_bg_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

/* mid tilemap */
TILE_GET_INFO_MEMBER(oneshot_state::get_oneshot_mid_tile_info)
{
	int tileno = m_mid_videoram[tile_index * 2 + 1];

	SET_TILE_INFO_MEMBER(0, tileno, 2, 0);
}

WRITE16_MEMBER(oneshot_state::oneshot_mid_videoram_w)
{
	COMBINE_DATA(&m_mid_videoram[offset]);
	m_mid_tilemap->mark_tile_dirty(offset / 2);
}


/* fg tilemap */
TILE_GET_INFO_MEMBER(oneshot_state::get_oneshot_fg_tile_info)
{
	int tileno = m_fg_videoram[tile_index * 2 + 1];

	SET_TILE_INFO_MEMBER(0, tileno, 3, 0);
}

WRITE16_MEMBER(oneshot_state::oneshot_fg_videoram_w)
{
	COMBINE_DATA(&m_fg_videoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset / 2);
}

void oneshot_state::video_start()
{
	m_bg_tilemap =  &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(oneshot_state::get_oneshot_bg_tile_info),this),  TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_mid_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(oneshot_state::get_oneshot_mid_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_fg_tilemap =  &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(oneshot_state::get_oneshot_fg_tile_info),this),  TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	m_bg_tilemap->set_transparent_pen(0);
	m_mid_tilemap->set_transparent_pen(0);
	m_fg_tilemap->set_transparent_pen(0);
}

void oneshot_state::draw_crosshairs( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	//int xpos,ypos;

	/* get gun raw coordinates (player 1) */
	m_gun_x_p1 = (ioport("LIGHT0_X")->read() & 0xff) * 320 / 256;
	m_gun_y_p1 = (ioport("LIGHT0_Y")->read() & 0xff) * 240 / 256;

	/* compute the coordinates for drawing (from routine at 0x009ab0) */
	//xpos = m_gun_x_p1;
	//ypos = m_gun_y_p1;

	m_gun_x_p1 += m_gun_x_shift;

	m_gun_y_p1 -= 0x0a;
	if (m_gun_y_p1 < 0)
		m_gun_y_p1 = 0;


	/* get gun raw coordinates (player 2) */
	m_gun_x_p2 = (ioport("LIGHT1_X")->read() & 0xff) * 320 / 256;
	m_gun_y_p2 = (ioport("LIGHT1_Y")->read() & 0xff) * 240 / 256;

	/* compute the coordinates for drawing (from routine at 0x009b6e) */
	//xpos = m_gun_x_p2;
	//ypos = m_gun_y_p2;

	m_gun_x_p2 += m_gun_x_shift - 0x0a;
	if (m_gun_x_p2 < 0)
		m_gun_x_p2 = 0;
}

void oneshot_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	const UINT16 *source = m_sprites;
	const UINT16 *finish = source + (0x1000 / 2);
	gfx_element *gfx = m_gfxdecode->gfx(1);

	int xpos, ypos;

	while (source < finish)
	{
		int blockx, blocky;
		int num = source[1] & 0xffff;
		int xsize = (source[2] & 0x000f) + 1;
		int ysize = (source[3] & 0x000f) + 1;

		ypos = source[3] & 0xff80;
		xpos = source[2] & 0xff80;

		ypos = ypos >> 7;
		xpos = xpos >> 7;


		if (source[0] == 0x0001)
			break;

		xpos -= 8;
		ypos -= 6;

		for (blockx = 0; blockx < xsize; blockx++)
		{
			for (blocky = 0; blocky < ysize; blocky++)
			{
						gfx->transpen(
						bitmap,
						cliprect,
						num + (blocky * xsize) + blockx,
						1,
						0,0,
						xpos + blockx * 8, ypos + blocky * 8, 0);


						gfx->transpen(
						bitmap,
						cliprect,
						num + (blocky * xsize) + blockx,
						1,
						0,0,
						xpos + blockx * 8 - 0x200, ypos + blocky * 8, 0);
			}
		}
		source += 0x4;
	}

}

UINT32 oneshot_state::screen_update_oneshot(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	m_mid_tilemap->set_scrollx(0, m_scroll[0] - 0x1f5);
	m_mid_tilemap->set_scrolly(0, m_scroll[1]);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_mid_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_crosshairs(bitmap, cliprect);
	return 0;
}

UINT32 oneshot_state::screen_update_maddonna(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	m_mid_tilemap->set_scrolly(0, m_scroll[1]); // other registers aren't used so we don't know which layers they relate to

	m_mid_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);

//  popmessage ("%04x %04x %04x %04x %04x %04x %04x %04x", m_scroll[0], m_scroll[1], m_scroll[2], m_scroll[3], m_scroll[4], m_scroll[5], m_scroll[6], m_scroll[7]);
	return 0;
}
