// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria

#include "emu.h"
#include "lsasquad.h"

void lsasquad_state::draw_layer(bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t *scrollram)
{
	int scrollx = scrollram[3];
	int scrolly = -scrollram[0];

	for (int offs = 0; offs < 0x080; offs += 4)
	{
		int base = 64 * scrollram[offs + 1];
		int sx = 8 * (offs / 4) + scrollx;
		if (flip_screen())
			sx = 248 - sx;

		sx &= 0xff;

		for (int y = 0; y < 32; y++)
		{
			int sy = 8 * y + scrolly;
			if (flip_screen())
				sy = 248 - sy;
			sy &= 0xff;

			int attr = m_videoram[(base + 2 * y + 1) & 0x1fff];
			int code = m_videoram[(base + 2 * y) & 0x1fff] + ((attr & 0x0f) << 8);
			int color = attr >> 4;

			m_gfxdecode->gfx(0)->transpen(bitmap, cliprect,
					code,
					color,
					flip_screen(), flip_screen(),
					sx, sy, 15);
			if (sx > 248)   // wraparound
				m_gfxdecode->gfx(0)->transpen(bitmap, cliprect,
						code,
						color,
						flip_screen(), flip_screen(),
						sx - 256, sy, 15);
		}
	}
}

int lsasquad_state::draw_layer_daikaiju(bitmap_ind16 &bitmap, const rectangle &cliprect, int offs, int *previd, int type)
{
	int stepx = 0;

	int initoffs = offs;
	int globalscrollx = 0;

	int id = m_scrollram[offs + 2];

	for( ; offs < 0x400; offs += 4)
	{
		int base, y, sx, sy, code, color;

			//id change
		if (id != m_scrollram[offs + 2])
		{
			*previd = id;
			return offs;
		}
		else
		{
			id = m_scrollram[offs + 2];
		}

		//skip empty (??) column, potential problems with 1st column in scrollram (scroll 0, tile 0, id 0)
		if ((m_scrollram[offs + 0] | m_scrollram[offs + 1] | m_scrollram[offs + 2] | m_scrollram[offs + 3]) == 0)
			continue;

		//local scroll x/y
		int scrolly = -m_scrollram[offs + 0];
		int scrollx =  m_scrollram[offs + 3];

		//check for global x scroll used in bg layer in game (starts at offset 0 in scrollram
		// and game name/logo on title screen (starts in the middle of scrollram, but with different
		// (NOT unique )id than previous column(s)

		if (*previd != 1)
		{
			if (offs != initoffs)
			{
				scrollx += globalscrollx;
			}
			else
			{
				//global scroll init
				globalscrollx = scrollx;
			}
		}

		base = 64 * m_scrollram[offs + 1];
		sx = scrollx + stepx;

		if (flip_screen())
			sx = 248 - sx;
		sx &= 0xff;

		for (y = 0; y < 32; y++)
		{
			sy = 8 * y + scrolly;
			if (flip_screen())
				sy = 248 - sy;
			sy &= 0xff;

			int attr = m_videoram[(base + 2 * y + 1) & 0x1fff];
			code = m_videoram[(base + 2 * y) & 0x1fff] + ((attr & 0x0f) << 8);
			color = attr >> 4;

			if ((type == 0 && color != 0x0d) || (type != 0 && color == 0x0d))
			{
				m_gfxdecode->gfx(0)->transpen(bitmap, cliprect,
					code,
					color,
					flip_screen(), flip_screen(),
					sx, sy, 15);
				if (sx > 248)   // wraparound
					m_gfxdecode->gfx(0)->transpen(bitmap, cliprect,
						code,
						color,
						flip_screen(), flip_screen(),
						sx - 256, sy, 15);
			}
		}
	}
	return offs;
}

void lsasquad_state::drawbg(bitmap_ind16 &bitmap, const rectangle &cliprect, int type)
{
	int i = 0;
	int id = -1;

	while (i < 0x400)
	{
		if (!(m_scrollram[i + 2] & 1))
		{
			i = draw_layer_daikaiju(bitmap, cliprect, i, &id, type);
		}
		else
		{
			id = m_scrollram[i + 2];
			i += 4;
		}
	}
}

void lsasquad_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t priority)
{
	for (int offs = m_spriteram.bytes() - 4; offs >= 0; offs -= 4)
	{
		int attr = m_spriteram[offs + 1];
		int code = m_spriteram[offs + 2] + ((attr & 0x30) << 4);
		int sx = m_spriteram[offs + 3];
		int sy = 240 - m_spriteram[offs];
		int color = attr & 0x0f;
		int flipx = attr & 0x40;
		int flipy = attr & 0x80;

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
				code,
				color,
				flipx, flipy,
				sx, sy, 15);
		// wraparound
		m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
				code,
				color,
				flipx, flipy,
				sx - 256, sy, 15);
	}
}

uint32_t lsasquad_state::screen_update_lsasquad(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(511, cliprect);

	draw_layer(bitmap, cliprect, m_scrollram + 0x000);
	draw_layer(bitmap, cliprect, m_scrollram + 0x080);
	draw_sprites(bitmap, cliprect, 0);
	draw_layer(bitmap, cliprect, m_scrollram + 0x100);
	return 0;
}


uint32_t lsasquad_state::screen_update_daikaiju(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(511, cliprect);
	drawbg(bitmap, cliprect, 0); // bottom
	draw_sprites(bitmap, cliprect, 0);
	drawbg(bitmap, cliprect, 1); // top = palette $d ?
	return 0;
}
