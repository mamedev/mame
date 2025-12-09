// license:BSD-3-Clause
// copyright-holders:Mike Coates
/*******************************************************************************

  circus_v.cpp

  Functions to emulate the video hardware of the machine.

*******************************************************************************/

#include "emu.h"
#include "circus.h"



/*******************************************************************************
    Shared
*******************************************************************************/

void circus_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(circus_state::get_bg_tile_info)
{
	tileinfo.set(0, m_videoram[tile_index], 0, 0);
}

void circus_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(circus_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

void circus_state::draw_line(bitmap_ind16 &bitmap, const rectangle &cliprect, int x1, int y1, int x2, int y2, int dotted)
{
	// Draws horizontal and Vertical lines only!
	int skip = dotted ? 2 : 1;

	if (x1 == x2)
	{
		for (int count = y2; count >= y1; count -= skip)
			bitmap.pix(count, x1) = 1;
	}
	else
	{
		for (int count = x2; count >= x1; count -= skip)
			bitmap.pix(y1, count) = 1;
	}
}

void circus_state::draw_sprite_collision(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	gfx_element *sprite_gfx = m_gfxdecode->gfx(1);
	const uint8_t *sprite_data = sprite_gfx->get_data(m_clown_z & 0xf);
	int collision = 0;

	// draw sprite and check collision on a pixel basis
	for (int sy = 0; sy < 16; sy++)
	{
		int dy = m_clown_x + sy - 1;
		if (dy >= 0 && dy < bitmap.height())
		{
			for (int sx = 0; sx < 16; sx++)
			{
				int dx = m_clown_y + sx;
				if (dx >= 0 && dx < bitmap.width())
				{
					int pixel = sprite_data[sy * sprite_gfx->rowbytes() + sx];
					if (pixel)
					{
						collision |= bitmap.pix(dy, dx);
						bitmap.pix(dy, dx) = m_palette->pen(pixel);
					}
				}
			}
		}
	}

	if (collision)
		m_maincpu->set_input_line(0, ASSERT_LINE);
}



/*******************************************************************************
    Circus
*******************************************************************************/

void circus_state::draw_fg(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// The PROMs are used to draw the border and diving boards
	draw_line(bitmap, cliprect, 0, 18, 255, 18, 0);
	draw_line(bitmap, cliprect, 0, 249, 255, 249, 1);
	draw_line(bitmap, cliprect, 0, 18, 0, 248, 0);
	draw_line(bitmap, cliprect, 247, 18, 247, 248, 0);

	draw_line(bitmap, cliprect, 0, 136, 17, 136, 0);
	draw_line(bitmap, cliprect, 231, 136, 248, 136, 0);
	draw_line(bitmap, cliprect, 0, 192, 17, 192, 0);
	draw_line(bitmap, cliprect, 231, 192, 248, 192, 0);
}

uint32_t circus_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_fg(bitmap, cliprect);
	draw_sprite_collision(bitmap, cliprect);
	return 0;
}



/*******************************************************************************
    Robot Bowl
*******************************************************************************/

void robotbwl_state::draw_box(bitmap_ind16 &bitmap, const rectangle &cliprect, int x, int y)
{
	// Box
	int ex = x + 24;
	int ey = y + 26;

	draw_line(bitmap, cliprect, x, y, ex, y, 0);    // Top
	draw_line(bitmap, cliprect, x, ey, ex, ey, 0);  // Bottom
	draw_line(bitmap, cliprect, x, y, x, ey, 0);    // Left
	draw_line(bitmap, cliprect, ex, y, ex, ey, 0);  // Right

	// Score Grid
	ey = y + 10;
	draw_line(bitmap, cliprect, x + 8, ey, ex, ey, 0); // Horizontal Divide Line
	draw_line(bitmap, cliprect, x + 8, y, x + 8, ey, 0);
	draw_line(bitmap, cliprect, x + 16, y, x + 16, ey, 0);
}

void robotbwl_state::draw_scoreboard(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// The sync generator hardware is used to draw the bowling alley & scorecards
	for (int offs = 15; offs <= 63; offs += 24)
	{
		draw_box(bitmap, cliprect, offs, 31);
		draw_box(bitmap, cliprect, offs, 63);
		draw_box(bitmap, cliprect, offs, 95);

		draw_box(bitmap, cliprect, offs + 152, 31);
		draw_box(bitmap, cliprect, offs + 152, 63);
		draw_box(bitmap, cliprect, offs + 152, 95);
	}

	draw_box(bitmap, cliprect, 39, 127);       // 10th Frame
	draw_line(bitmap, cliprect, 39, 137, 47, 137, 0);   // Extra digit box

	draw_box(bitmap, cliprect, 39 + 152, 127);
	draw_line(bitmap, cliprect, 39 + 152, 137, 47 + 152, 137, 0);
}

void robotbwl_state::draw_bowling_alley(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	draw_line(bitmap, cliprect, 103, 17, 103, 205, 0);
	draw_line(bitmap, cliprect, 111, 17, 111, 203, 1);
	draw_line(bitmap, cliprect, 152, 17, 152, 205, 0);
	draw_line(bitmap, cliprect, 144, 17, 144, 203, 1);
}

void robotbwl_state::draw_ball(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_gfxdecode->gfx(1)->transpen(bitmap,
			cliprect,
			m_clown_z,
			0,
			0,0,
			m_clown_y + 8, // Y is horizontal position
			m_clown_x + 8,
			0);
}

uint32_t robotbwl_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_scoreboard(bitmap, cliprect);
	draw_bowling_alley(bitmap, cliprect);
	draw_ball(bitmap, cliprect);
	return 0;
}



/*******************************************************************************
    Crash
*******************************************************************************/

void crash_state::draw_car(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_gfxdecode->gfx(1)->transpen(bitmap,
			cliprect,
			m_clown_z,
			0,
			0,0,
			m_clown_y, // Y is horizontal position
			m_clown_x - 1,
			0);
}

uint32_t crash_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_car(bitmap, cliprect);
	return 0;
}



/*******************************************************************************
    Rip Cord
*******************************************************************************/

uint32_t ripcord_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprite_collision(bitmap, cliprect);
	return 0;
}
