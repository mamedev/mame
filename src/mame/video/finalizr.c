// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

  Konami Finalizer

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/finalizr.h"


/***************************************************************************

  The palette PROMs are connected to the RGB output this way:

  bit 7 -- 220  ohm resistor  -- \
        -- 470  ohm resistor  -- | -- 470 ohm pulldown resistor -- GREEN
        -- 1   kohm resistor  -- |
        -- 2.2 kohm resistor  -- /
        -- 220  ohm resistor  -- \
        -- 470  ohm resistor  -- | -- 470 ohm pulldown resistor -- RED
        -- 1   kohm resistor  -- |
  bit 0 -- 2.2 kohm resistor  -- /


  bit 3 -- 220  ohm resistor  -- \
        -- 470  ohm resistor  -- | -- 470 ohm pulldown resistor -- BLUE
        -- 1   kohm resistor  -- |
  bit 0 -- 2.2 kohm resistor  -- /

***************************************************************************/

PALETTE_INIT_MEMBER(finalizr_state, finalizr)
{
	const UINT8 *color_prom = memregion("proms")->base();
	static const int resistances[4] = { 2200, 1000, 470, 220 };
	double rweights[4], gweights[4], bweights[4];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0, 255, -1.0,
			4, &resistances[0], rweights, 470, 0,
			4, &resistances[0], gweights, 470, 0,
			4, &resistances[0], bweights, 470, 0);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2, bit3;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		bit3 = (color_prom[i] >> 3) & 0x01;
		r = combine_4_weights(rweights, bit0, bit1, bit2, bit3);

		/* green component */
		bit0 = (color_prom[i] >> 4) & 0x01;
		bit1 = (color_prom[i] >> 5) & 0x01;
		bit2 = (color_prom[i] >> 6) & 0x01;
		bit3 = (color_prom[i] >> 7) & 0x01;
		g = combine_4_weights(gweights, bit0, bit1, bit2, bit3);

		/* blue component */
		bit0 = (color_prom[i + 0x20] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x20] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x20] >> 2) & 0x01;
		bit3 = (color_prom[i + 0x20] >> 3) & 0x01;
		b = combine_4_weights(bweights, bit0, bit1, bit2, bit3);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x40;

	for (i = 0; i < 0x100; i++)
	{
		UINT8 ctabentry = (color_prom[i] & 0x0f) | 0x10;
		palette.set_pen_indirect(i, ctabentry);
	}

	for (i = 0x100; i < 0x200; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		palette.set_pen_indirect(i, ctabentry);
	}
}

TILE_GET_INFO_MEMBER(finalizr_state::get_bg_tile_info)
{
	int attr = m_colorram[tile_index];
	int code = m_videoram[tile_index] + ((attr & 0xc0) << 2) + (m_charbank << 10);
	int color = attr & 0x0f;
	int flags = TILE_FLIPYX((attr & 0x30) >> 4);

	SET_TILE_INFO_MEMBER(0, code, color, flags);
}

TILE_GET_INFO_MEMBER(finalizr_state::get_fg_tile_info)
{
	int attr = m_colorram2[tile_index];
	int code = m_videoram2[tile_index] + ((attr & 0xc0) << 2);
	int color = attr & 0x0f;
	int flags = TILE_FLIPYX((attr & 0x30) >> 4);

	SET_TILE_INFO_MEMBER(0, code, color, flags);
}

void finalizr_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(finalizr_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(finalizr_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}



/**************************************************************************/

void finalizr_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx1 = m_gfxdecode->gfx(1);
	gfx_element *gfx2 = m_gfxdecode->gfx(2);

	UINT8 *sr = m_spriterambank ? m_spriteram_2 : m_spriteram;

	for (int offs = 0; offs <= m_spriteram.bytes() - 5; offs += 5)
	{
		int sx, sy, flipx, flipy, code, color, size;

		sx = 32 + 1 + sr[offs + 3] - ((sr[offs + 4] & 0x01) << 8);
		sy = sr[offs + 2];
		flipx = sr[offs + 4] & 0x20;
		flipy = sr[offs + 4] & 0x40;
		code = sr[offs] + ((sr[offs + 1] & 0x0f) << 8);
		color = ((sr[offs + 1] & 0xf0) >> 4);

//      (sr[offs + 4] & 0x02) is used, meaning unknown

		size = sr[offs + 4] & 0x1c;

		if (size >= 0x10)
		{
			// 32x32
			if (flip_screen())
			{
				sx = 256 - sx;
				sy = 224 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			gfx1->transpen(bitmap, cliprect, code + 0, color, flipx, flipy, flipx ? sx + 16 : sx, flipy ? sy + 16 : sy, 0);
			gfx1->transpen(bitmap, cliprect, code + 1, color, flipx, flipy, flipx ? sx : sx + 16, flipy ? sy + 16 : sy, 0);
			gfx1->transpen(bitmap, cliprect, code + 2, color, flipx, flipy, flipx ? sx + 16: sx , flipy ? sy : sy + 16, 0);
			gfx1->transpen(bitmap, cliprect, code + 3, color, flipx, flipy, flipx ? sx : sx + 16, flipy ? sy : sy + 16, 0);
		}
		else
		{
			if (flip_screen())
			{
				sx = ((size & 0x08) ? 280: 272) - sx;
				sy = ((size & 0x04) ? 248: 240) - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			if (size == 0x00)
			{
				// 16x16
				gfx1->transpen(bitmap, cliprect, code, color, flipx, flipy, sx, sy, 0);
			}
			else
			{
				code = ((code & 0x3ff) << 2) | ((code & 0xc00) >> 10);

				if (size == 0x04)
				{
					// 16x8
					gfx2->transpen(bitmap, cliprect, code &~1, color, flipx, flipy, flipx ? sx + 8 : sx, sy, 0);
					gfx2->transpen(bitmap, cliprect, code | 1, color, flipx, flipy, flipx ? sx : sx + 8, sy, 0);
				}
				else if (size == 0x08)
				{
					// 8x16
					gfx2->transpen(bitmap, cliprect, code &~2, color, flipx, flipy, sx, flipy ? sy + 8 : sy, 0);
					gfx2->transpen(bitmap, cliprect, code | 2, color, flipx, flipy, sx, flipy ? sy : sy + 8, 0);
				}
				else if (size == 0x0c)
				{
					// 8x8
					gfx2->transpen(bitmap, cliprect, code, color, flipx, flipy, sx, sy, 0);
				}
			}
		}
	}
}


UINT32 finalizr_state::screen_update_finalizr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->mark_all_dirty();
	m_fg_tilemap->mark_all_dirty();

	m_bg_tilemap->set_scrollx(0, *m_scroll - 32);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	draw_sprites(bitmap, cliprect);

	// draw top status region
	const rectangle &visarea = screen.visible_area();
	rectangle clip = cliprect;

	clip.min_x = visarea.min_x;
	clip.max_x = visarea.min_x + 31;
	m_fg_tilemap->set_scrolldx(0,-32);
	m_fg_tilemap->draw(screen, bitmap, clip, 0, 0);

	return 0;
}
