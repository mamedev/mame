// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "emu.h"
#include "includes/lsasquad.h"

void lsasquad_state::draw_layer( bitmap_ind16 &bitmap, const rectangle &cliprect, UINT8 *scrollram )
{
	int offs, scrollx, scrolly;

	scrollx = scrollram[3];
	scrolly = -scrollram[0];

	for (offs = 0; offs < 0x080; offs += 4)
	{
		int base, y, sx, sy, code, color;

		base = 64 * scrollram[offs + 1];
		sx = 8 * (offs / 4) + scrollx;
		if (flip_screen())
			sx = 248 - sx;

		sx &= 0xff;

		for (y = 0; y < 32; y++)
		{
			int attr;

			sy = 8 * y + scrolly;
			if (flip_screen())
				sy = 248 - sy;
			sy &= 0xff;

			attr = m_videoram[base + 2 * y + 1];
			code = m_videoram[base + 2 * y] + ((attr & 0x0f) << 8);
			color = attr >> 4;

			m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
					code,
					color,
					flip_screen(),flip_screen(),
					sx,sy,15);
			if (sx > 248)   /* wraparound */
				m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
						code,
						color,
						flip_screen(),flip_screen(),
						sx-256,sy,15);
		}
	}
}

int lsasquad_state::draw_layer_daikaiju( bitmap_ind16 &bitmap, const rectangle &cliprect, int offs, int  * previd, int type )
{
	int id, scrollx, scrolly, initoffs, globalscrollx;
	int stepx = 0;

	initoffs = offs;
	globalscrollx = 0;

	id = m_scrollram[offs + 2];

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

		//skip empty (??) column, potential probs with 1st column in scrollram (scroll 0, tile 0, id 0)
		if ((m_scrollram[offs + 0] | m_scrollram[offs + 1] | m_scrollram[offs + 2] | m_scrollram[offs + 3]) == 0)
			continue;

		//local scroll x/y
		scrolly = -m_scrollram[offs + 0];
		scrollx =  m_scrollram[offs + 3];

		//check for global x scroll used in bg layer in game (starts at offset 0 in scrollram
		// and game name/logo on title screen (starts in the middle of scrollram, but with different
		// (NOT unique )id than prev coulmn(s)

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
			int attr;

			sy = 8 * y + scrolly;
			if (flip_screen())
				sy = 248 - sy;
			sy &= 0xff;

			attr = m_videoram[base + 2 * y + 1];
			code = m_videoram[base + 2 * y] + ((attr & 0x0f) << 8);
			color = attr >> 4;

			if ((type == 0 && color != 0x0d) || (type != 0 && color == 0x0d))
			{
				m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
					code,
					color,
					flip_screen(),flip_screen(),
					sx,sy,15);
				if (sx > 248)   /* wraparound */
					m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
						code,
						color,
						flip_screen(),flip_screen(),
						sx-256,sy,15);
			}
		}
	}
	return offs;
}

void lsasquad_state::drawbg( bitmap_ind16 &bitmap, const rectangle &cliprect, int type )
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

void lsasquad_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	UINT8 *spriteram = m_spriteram;
	int offs;

	for (offs = m_spriteram.bytes() - 4; offs >= 0; offs -= 4)
	{
		int sx, sy, attr, code, color, flipx, flipy;

		sx = spriteram[offs + 3];
		sy = 240 - spriteram[offs];
		attr = spriteram[offs + 1];
		code = spriteram[offs + 2] + ((attr & 0x30) << 4);
		color = attr & 0x0f;
		flipx = attr & 0x40;
		flipy = attr & 0x80;

		if (flip_screen())
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				code,
				color,
				flipx,flipy,
				sx,sy,15);
		/* wraparound */
		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
				code,
				color,
				flipx,flipy,
				sx-256,sy,15);
	}
}

UINT32 lsasquad_state::screen_update_lsasquad(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(511, cliprect);

	draw_layer(bitmap, cliprect, m_scrollram + 0x000);
	draw_layer(bitmap, cliprect, m_scrollram + 0x080);
	draw_sprites(bitmap, cliprect);
	draw_layer(bitmap, cliprect, m_scrollram + 0x100);
	return 0;
}


UINT32 lsasquad_state::screen_update_daikaiju(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(511, cliprect);
	drawbg(bitmap, cliprect, 0); // bottom
	draw_sprites(bitmap, cliprect);
	drawbg(bitmap, cliprect, 1); // top = palette $d ?
	return 0;
}
