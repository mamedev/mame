// license:BSD-3-Clause
// copyright-holders:David Haywood, Paul Priest
/* One Shot One Kill Video Hardware */

#include "emu.h"
#include "oneshot.h"


/* bg tilemap */
TILE_GET_INFO_MEMBER(oneshot_state::get_bg_tile_info)
{
	const u32 tileno = m_bg_videoram[tile_index * 2 + 1];

	tileinfo.set(0, tileno, 0, 0);
}

void oneshot_state::bg_videoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_bg_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

/* mid tilemap */
TILE_GET_INFO_MEMBER(oneshot_state::get_mid_tile_info)
{
	const u32 tileno = m_mid_videoram[tile_index * 2 + 1];

	tileinfo.set(0, tileno, 2, 0);
}

void oneshot_state::mid_videoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_mid_videoram[offset]);
	m_mid_tilemap->mark_tile_dirty(offset / 2);
}


/* fg tilemap */
TILE_GET_INFO_MEMBER(oneshot_state::get_fg_tile_info)
{
	const u32 tileno = m_fg_videoram[tile_index * 2 + 1];

	tileinfo.set(0, tileno, 3, 0);
}

void oneshot_state::fg_videoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_fg_videoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset / 2);
}

void oneshot_state::video_start()
{
	m_bg_tilemap =  &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(oneshot_state::get_bg_tile_info)),  TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_mid_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(oneshot_state::get_mid_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_fg_tilemap =  &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(oneshot_state::get_fg_tile_info)),  TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	m_bg_tilemap->set_transparent_pen(0);
	m_mid_tilemap->set_transparent_pen(0);
	m_fg_tilemap->set_transparent_pen(0);
}

void oneshot_state::draw_crosshairs()
{
	//int xpos,ypos;

	/* get gun raw coordinates (player 1) */
	m_gun_x_p1 = (m_io_lightgun_x[0]->read() & 0xff) * 320 / 256;
	m_gun_y_p1 = (m_io_lightgun_y[0]->read() & 0xff) * 240 / 256;

	/* compute the coordinates for drawing (from routine at 0x009ab0) */
	//xpos = m_gun_x_p1;
	//ypos = m_gun_y_p1;

	m_gun_x_p1 += m_gun_x_shift;

	m_gun_y_p1 -= 0x0a;
	if (m_gun_y_p1 < 0)
		m_gun_y_p1 = 0;


	/* get gun raw coordinates (player 2) */
	m_gun_x_p2 = (m_io_lightgun_x[1]->read() & 0xff) * 320 / 256;
	m_gun_y_p2 = (m_io_lightgun_y[1]->read() & 0xff) * 240 / 256;

	/* compute the coordinates for drawing (from routine at 0x009b6e) */
	//xpos = m_gun_x_p2;
	//ypos = m_gun_y_p2;

	m_gun_x_p2 += m_gun_x_shift - 0x0a;
	if (m_gun_x_p2 < 0)
		m_gun_x_p2 = 0;
}

void oneshot_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const u16 *source = m_spriteram;
	const u16 *finish = source + (0x1000 / 2);
	gfx_element *gfx = m_gfxdecode->gfx(1);

	while (source < finish)
	{
		const u16 attr = source[0];

		if (attr & 0x0001) // end of sprites
			break;

		if (!(attr & 0x8000)) // visible bit
		{
			source += 0x4;
			continue;
		}

		const u32 num = source[1] & 0xffff;
		const int xsize = (source[2] & 0x000f) + 1;
		const int ysize = (source[3] & 0x000f) + 1;

		int ypos = source[3] & 0xff80;
		int xpos = source[2] & 0xff80;

		ypos = ypos >> 7;
		xpos = xpos >> 7;

		xpos -= 8;
		ypos -= 6;

		for (int blockx = 0; blockx < xsize; blockx++)
		{
			for (int blocky = 0; blocky < ysize; blocky++)
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

u32 oneshot_state::screen_update_oneshot(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	m_mid_tilemap->set_scrollx(0, m_scroll[0] - 0x1f5);
	m_mid_tilemap->set_scrolly(0, m_scroll[1]);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_mid_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_crosshairs();
	return 0;
}

u32 oneshot_state::screen_update_maddonna(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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

// why are the layers in a different order?
u32 oneshot_state::screen_update_komocomo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	m_mid_tilemap->set_scrolly(0, m_scroll[1]); // other registers aren't used so we don't know which layers they relate to

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_mid_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);

//  popmessage ("%04x %04x %04x %04x %04x %04x %04x %04x", m_scroll[0], m_scroll[1], m_scroll[2], m_scroll[3], m_scroll[4], m_scroll[5], m_scroll[6], m_scroll[7]);
	return 0;
}
