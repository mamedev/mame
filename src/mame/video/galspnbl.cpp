// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "emu.h"
#include "includes/galspnbl.h"


PALETTE_INIT_MEMBER(galspnbl_state, galspnbl)
{
	int i;

	/* initialize 555 RGB lookup */
	for (i = 0; i < 32768; i++)
		palette.set_pen_color(i + 1024, pal5bit(i >> 5), pal5bit(i >> 10), pal5bit(i >> 0));
}



void galspnbl_state::draw_background( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	offs_t offs;

//  int screenscroll = 4 - (m_scroll[0] & 0xff);

	for (offs = 0; offs < 0x20000; offs++)
	{
		int y = offs >> 9;
		int x = offs & 0x1ff;

		bitmap.pix16(y, x) = 1024 + (m_bgvideoram[offs] >> 1);
	}
}

VIDEO_START_MEMBER(galspnbl_state,galspnbl)
{
	/* allocate bitmaps */
	m_screen->register_screen_bitmap(m_sprite_bitmap);
}

void galspnbl_state::mix_sprite_layer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri)
{
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		UINT16 *dd = &bitmap.pix16(y);
		UINT16 *sd2 = &m_sprite_bitmap.pix16(y);

		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			UINT16 sprpixel = (sd2[x]);
			//UINT16 sprpri = (sprpixel >> 8) & 3;
			UINT16 sprpri = (sprpixel >> 9) & 1; // only upper priority bit matters on the bootleg hw?

			sprpixel &= 0xff;

			if (sprpixel & 0xf)
			{
				if (sprpri == pri)
					dd[x] = sprpixel;
			}

			//  UINT16 sprbln = (sprpixel >> 10) & 1; // we handle 'blending' from the original as a simple on/off flicker in the bootleg sprite function, I don't think the bootleg hw can blend
		}
	}
}

UINT32 galspnbl_state::screen_update_galspnbl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs;
	m_sprite_bitmap.fill(0, cliprect);
	m_sprgen->gaiden_draw_sprites(screen, m_gfxdecode, cliprect, m_spriteram, 0, 0, flip_screen(), m_sprite_bitmap);


	draw_background(bitmap, cliprect);

	mix_sprite_layer(screen, bitmap, cliprect, 0);

	for (offs = 0; offs < 0x1000 / 2; offs++)
	{
		int sx, sy, code, attr, color;

		code = m_videoram[offs];
		attr = m_colorram[offs];
		color = (attr & 0x00f0) >> 4;
		sx = offs % 64;
		sy = offs / 64;

		/* What is this? A priority/half transparency marker? */ // leftover blend flags from original spbactn game
		if (!(attr & 0x0008))
		{
			m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
					code,
					color,
					0,0,
//                  16*sx + screenscroll,8*sy,
					16*sx,8*sy,0);
		}
	}

	mix_sprite_layer(screen, bitmap, cliprect, 1);


	return 0;
}
