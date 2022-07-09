// license:BSD-3-Clause
// copyright-holders:Paul Leaman
/***************************************************************************

    Battle Lane Vol. 5

***************************************************************************/

#include "emu.h"
#include "battlane.h"


void battlane_state::battlane_palette_w(offs_t offset, uint8_t data)
{
	int r, g, b;
	int bit0, bit1, bit2;

	// red component
	bit0 = (~data >> 0) & 0x01;
	bit1 = (~data >> 1) & 0x01;
	bit2 = (~data >> 2) & 0x01;
	r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

	// green component
	bit0 = (~data >> 3) & 0x01;
	bit1 = (~data >> 4) & 0x01;
	bit2 = (~data >> 5) & 0x01;
	g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

	// blue component
	bit0 = (~m_video_ctrl >> 7) & 0x01;
	bit1 = (~data >> 6) & 0x01;
	bit2 = (~data >> 7) & 0x01;
	b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

	m_palette->set_pen_color(offset, rgb_t(r, g, b));
}

void battlane_state::battlane_scrollx_w(uint8_t data)
{
	m_bg_tilemap->set_scrollx(0, ((m_video_ctrl & 0x01) << 8) + data);
}

void battlane_state::battlane_scrolly_w(uint8_t data)
{
	m_bg_tilemap->set_scrolly(0, ((m_cpu_control & 0x01) << 8) + data);
}

void battlane_state::battlane_tileram_w(offs_t offset, uint8_t data)
{
	m_tileram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

void battlane_state::battlane_spriteram_w(offs_t offset, uint8_t data)
{
	m_spriteram[offset] = data;
}

void battlane_state::battlane_bitmap_w(offs_t offset, uint8_t data)
{
	int orval = (~m_video_ctrl >> 1) & 0x07;
	if (!orval)
		orval = 7;

	for (int i = 0; i < 8; i++)
	{
		if (data & 1 << i)
		{
			m_screen_bitmap.pix(offset & 0xff, (offset >> 8 ) * 8 + i) |= orval;
		}
		else
		{
			m_screen_bitmap.pix(offset & 0xff, (offset >> 8) * 8 + i) &= ~orval;
		}
	}
}

void battlane_state::battlane_video_ctrl_w(uint8_t data)
{
	/*
	    Video control register

	        0x80    = low bit of blue component (taken when writing to palette)
	        0x0e    = Bitmap plane (bank?) select  (0-7)
	        0x01    = Scroll MSB
	*/

	m_video_ctrl = data;
}

TILE_GET_INFO_MEMBER(battlane_state::get_tile_info_bg)
{
	int code = m_tileram[tile_index];
	int attr = m_tileram[tile_index + 0x400];
	int gfxn = (attr & 0x01) + 1;
	int color = (attr >> 1) & 0x03;

	tileinfo.set(gfxn, code, color, 0);
}

TILEMAP_MAPPER_MEMBER(battlane_state::battlane_tilemap_scan_rows_2x2)
{
	/*
	        Tilemap Memory Organization

	     0              15 16            31
	    +-----------------+----------------+
	    |0              15|256             |0
	    |                 |                |
	    |     screen 0    |    screen 1    |
	    |                 |                |
	    |240           255|             511|15
	    +-----------------+----------------+
	    |512              |768             |16
	    |                 |                |
	    |     screen 2    |    screen 3    |
	    |                 |                |
	    |              767|            1023|31
	    +-----------------+-----------------

	*/

	return (row & 0xf) * 16 + (col & 0xf) + (row & 0x10) * 32 + (col & 0x10) * 16;
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void battlane_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(battlane_state::get_tile_info_bg)), tilemap_mapper_delegate(*this, FUNC(battlane_state::battlane_tilemap_scan_rows_2x2)), 16, 16, 32, 32);
	m_screen_bitmap.allocate(32 * 8, 32 * 8);
	save_item(NAME(m_screen_bitmap));
}

void battlane_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs, attr, code, color, sx, sy, flipx, flipy;

	for (offs = 0; offs < 0x100; offs += 4)
	{
		/*
		    0x80 = Bank 2
		    0x40 =
		    0x20 = Bank 1
		    0x10 = Y Double
		    0x08 = Color
		    0x04 = X Flip
		    0x02 = Y Flip
		    0x01 = Sprite Enable
		*/

		attr = m_spriteram[offs + 1];
		code = m_spriteram[offs + 3];

		code += 256 * ((attr >> 6) & 0x02);
		code += 256 * ((attr >> 5) & 0x01);

		if (attr & 0x01)
		{
			color = BIT(attr, 3);

			sx = m_spriteram[offs + 2];
			sy = m_spriteram[offs];

			flipx = BIT(attr, 2);
			flipy = BIT(attr, 1);

			if (!flip_screen())
			{
				sx = 240 - sx;
				sy = 240 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			if (attr & 0x10) // Y Double
			{
				for (int i = 0; i < 2; i++)
					m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
						code + i,
						color,
						flipx, flipy,
						sx, sy - (16 * (i ^ flipy)), 0);
			}
			else
			{
				m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
					code,
					color,
					flipx, flipy,
					sx, sy, 0);
			}
		}
	}
}

void battlane_state::draw_fg_bitmap(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int y = 0; y < 32 * 8; y++)
	{
		for (int x = 0; x < 32 * 8; x++)
		{
			int const data = m_screen_bitmap.pix(y, x);

			if (data)
			{
				int px = x, py = y;
				if (flip_screen())
				{
					px = 255 - px;
					py = 255 - py;
				}

				if (cliprect.contains(px, py))
					bitmap.pix(y, x) = data;
			}
		}
	}
}

uint32_t battlane_state::screen_update_battlane(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect);
	draw_sprites(bitmap, cliprect);
	draw_fg_bitmap(bitmap, cliprect);
	return 0;
}
