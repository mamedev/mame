// license:BSD-3-Clause
// copyright-holders:Curt Coder
// thanks-to:Kenneth Lin (original driver author)
/***************************************************************************

  jackal.cpp

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/jackal.h"


void jackal_state::palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	for (int i = 0; i < 0x100; i++)
	{
		uint16_t const ctabentry = i | 0x100;
		palette.set_pen_indirect(i, ctabentry);
	}

	for (int i = 0x100; i < 0x200; i++)
	{
		uint16_t const ctabentry = color_prom[i - 0x100] & 0x0f;
		palette.set_pen_indirect(i, ctabentry);
	}

	for (int i = 0x200; i < 0x300; i++)
	{
		uint16_t const ctabentry = (color_prom[i - 0x100] & 0x0f) | 0x10;
		palette.set_pen_indirect(i, ctabentry);
	}
}

TILE_GET_INFO_MEMBER(jackal_state::get_bg_tile_info)
{
	int attr = m_videoram[0][tile_index];
	int code = m_videoram[0][0x400 + tile_index] + ((attr & 0xc0) << 2) + ((attr & 0x30) << 6);
	int color = 0;//attr & 0x0f;
	int flags = ((attr & 0x10) ? TILE_FLIPX : 0) | ((attr & 0x20) ? TILE_FLIPY : 0);

	tileinfo.set(0, code, color, flags);
}

void jackal_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(jackal_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_spritebank->configure_entry(0, m_spriteram[0]);
	m_spritebank->configure_entry(1, m_spriteram[1]);

	m_scrollbank->configure_entry(0, m_scrollram[0]);
	m_scrollbank->configure_entry(1, m_scrollram[1]);
}

void jackal_state::draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scroll_rows(1);
	m_bg_tilemap->set_scroll_cols(1);

	m_bg_tilemap->set_scrolly(0, m_videoctrl[0]);
	m_bg_tilemap->set_scrollx(0, m_videoctrl[1]);

	if (m_videoctrl[2] & 0x02)
	{
		if (m_videoctrl[2] & 0x08)
		{
			m_bg_tilemap->set_scroll_rows(32);

			for (int i = 0; i < 32; i++)
				m_bg_tilemap->set_scrollx(i, m_scrollram[0][i]);
		}

		if (m_videoctrl[2] & 0x04)
		{
			m_bg_tilemap->set_scroll_cols(32);

			for (int i = 0; i < 32; i++)
				m_bg_tilemap->set_scrolly(i, m_scrollram[0][i]);
		}
	}

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
}

#define DRAW_SPRITE(bank, code, sx, sy)  m_gfxdecode->gfx(bank)->transpen(bitmap,cliprect, code, color, flipx, flipy, sx, sy, 0);

void jackal_state::draw_sprites_region(bitmap_ind16 &bitmap, const rectangle &cliprect, const uint8_t *sram, int length, int bank)
{
	for (int offs = 0; offs < length; offs += 5)
	{
		int sn1 = sram[offs];
		int sn2 = sram[offs + 1];
		int sy  = sram[offs + 2];
		int sx  = sram[offs + 3];
		int attr = sram[offs + 4];
		int flipx = attr & 0x20;
		int flipy = attr & 0x40;
		int color = ((sn2 & 0xf0) >> 4);

		if (attr & 0x01)
			sx = sx - 256;
		if (sy > 0xf0)
			sy = sy - 256;

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		if (attr & 0xc)    // half-size sprite
		{
			int spritenum = sn1 * 4 + ((sn2 & (8 + 4)) >> 2) + ((sn2 & (2 + 1)) << 10);
			int mod = -8;

			if (flip_screen())
			{
				sx += 8;
				sy -= 8;
				mod = 8;
			}

			if ((attr & 0x0c) == 0x0c)
			{
				if (flip_screen()) sy += 16;
				DRAW_SPRITE(bank + 1, spritenum, sx, sy)
			}

			if ((attr & 0x0c) == 0x08)
			{
				sy += 8;
				DRAW_SPRITE(bank + 1, spritenum,     sx, sy)
				DRAW_SPRITE(bank + 1, spritenum - 2, sx, sy + mod)
			}

			if ((attr & 0x0c) == 0x04)
			{
				DRAW_SPRITE(bank + 1, spritenum,     sx,       sy)
				DRAW_SPRITE(bank + 1, spritenum + 1, sx + mod, sy)
			}
		}
		else
		{
			int spritenum = sn1 + ((sn2 & 0x03) << 8);

			if (attr & 0x10)
			{
				if (flip_screen())
				{
					sx -= 16;
					sy -= 16;
				}

				DRAW_SPRITE(bank, spritenum,     flipx ? sx+16 : sx, flipy ? sy+16 : sy)
				DRAW_SPRITE(bank, spritenum + 1, flipx ? sx : sx+16, flipy ? sy+16 : sy)
				DRAW_SPRITE(bank, spritenum + 2, flipx ? sx+16 : sx, flipy ? sy : sy+16)
				DRAW_SPRITE(bank, spritenum + 3, flipx ? sx : sx+16, flipy ? sy : sy+16)
			}
			else
			{
				DRAW_SPRITE(bank, spritenum, sx, sy)
			}
		}
	}
}

void jackal_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	draw_sprites_region(bitmap, cliprect, &m_spriteram[1][BIT(m_videoctrl[0x03], 3) * 0x800], 0x0f5, 3);
	draw_sprites_region(bitmap, cliprect, &m_spriteram[0][BIT(m_videoctrl[0x03], 3) * 0x800], 0x500, 1);
}

uint32_t jackal_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	draw_background(screen, bitmap, cliprect);
	draw_sprites(bitmap, cliprect);
	return 0;
}
