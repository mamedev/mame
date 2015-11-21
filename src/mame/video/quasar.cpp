// license:BSD-3-Clause
// copyright-holders:Mike Coates, Pierpaolo Prazzoli
/***************************************************************************

  video\quasar.c

  Functions to emulate the video hardware of the machine.

  Zaccaria S2650 games share various levels of design with the Century Video
  System (CVS) games, and hence some routines are shared from there.

  Shooting seems to mix custom boards from Zaccaria and sound boards from CVS
  hinting at a strong link between the two companies.

  Zaccaria are an italian company, Century were based in Manchester UK

***************************************************************************/

#include "emu.h"
#include "cpu/s2650/s2650.h"
#include "includes/quasar.h"

PALETTE_INIT_MEMBER(quasar_state,quasar)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	/* standard 1 bit per color palette (background and sprites) */
	for (i = 0; i < 8; i++)
	{
		rgb_t color = rgb_t(pal1bit(i >> 0), pal1bit(i >> 1), pal1bit(i >> 2));
		palette.set_indirect_color(i, color);
	}

	/* effects color map */
	for (i = 0; i < 0x100; i++)
	{
		rgb_t color;
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = BIT(i, 0);
		bit1 = BIT(i, 1);
		bit2 = BIT(i, 2);
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = BIT(i, 3);
		bit1 = BIT(i, 4);
		bit2 = BIT(i, 5);
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = BIT(i, 6);
		bit1 = BIT(i, 7);
		b = 0x4f * bit0 + 0xa8 * bit1;

		/* intensity 0 */
		palette.set_indirect_color(0x100 + i, rgb_t::black);

		/* intensity 1 */
		color = rgb_t(r >> 2, g >> 2, b >> 2);
		palette.set_indirect_color(0x200 + i, color);

		/* intensity 2 */
		color = rgb_t((r >> 2) + (r >> 3), (g >> 2) + (g >> 3), (b >> 2) + (b >> 2));
		palette.set_indirect_color(0x300 + i, color);

		/* intensity 3 */
		color = rgb_t(r >> 1, g >> 1, b >> 1);
		palette.set_indirect_color(0x400 + i, color);
	}

	// Address 0-2 from graphic rom
	//         3-5 from color ram
	//         6-8 from sprite chips (Used for priority)
	for (i = 0; i < 0x200; i++)
		palette.set_pen_indirect(i, color_prom[i] & 0x07);

	/* background for collision */
	for (i = 1; i < 8; i++)
		palette.set_pen_indirect(0x200 + i, 7);
	palette.set_pen_indirect(0x200, 0);

	/* effects */
	for (i = 0; i < 0x400; i++)
		palette.set_pen_indirect(0x208 + i, 0x100 + i);
}


VIDEO_START_MEMBER(quasar_state,quasar)
{
	m_effectram = auto_alloc_array(machine(), UINT8, 0x400);

	/* create helper bitmap */
	m_screen->register_screen_bitmap(m_collision_background);

	/* register save */
	save_item(NAME(m_collision_background));
	save_pointer(NAME(m_effectram), 0x400);
}

UINT32 quasar_state::screen_update_quasar(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs;

	/* for every character in the video RAM */
	for (offs = 0; offs < 0x0400; offs++)
	{
		int ox, oy;
		UINT8 code = m_video_ram[offs];
		UINT8 x = (offs & 0x1f) << 3;
		UINT8 y = (offs >> 5) << 3;

		// While we have the current character code, draw the effects layer
		// intensity / on and off controlled by latch

		int forecolor = 0x208 + m_effectram[offs] + (256 * (((m_effectcontrol >> 4) ^ 3) & 3));

		for (ox = 0; ox < 8; ox++)
			for (oy = 0; oy < 8; oy++)
				bitmap.pix16(y + oy, x + ox) = forecolor;

		/* Main Screen */
		m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
				code,
				m_color_ram[offs] & 0x3f,
				0,0,
				x,y,0);


		/* background for Collision Detection (it can only hit certain items) */
		if((m_color_ram[offs] & 7) == 0)
		{
			m_gfxdecode->gfx(0)->opaque(m_collision_background,cliprect,
					code,
					64,
					0,0,
					x,y);
		}
	}

	/* update the S2636 chips */
	bitmap_ind16 &s2636_0_bitmap = m_s2636_0->update(cliprect);
	bitmap_ind16 &s2636_1_bitmap = m_s2636_1->update(cliprect);
	bitmap_ind16 &s2636_2_bitmap = m_s2636_2->update(cliprect);

	/* Bullet Hardware */
	for (offs = 8; offs < 256; offs++ )
	{
		if(m_bullet_ram[offs] != 0)
		{
			int ct;
			for (ct = 0; ct < 1; ct++)
			{
				int bx = 255 - 9 - m_bullet_ram[offs] - ct;

				/* bullet/object Collision */
				if (s2636_0_bitmap.pix16(offs, bx) != 0) m_collision_register |= 0x04;
				if (s2636_2_bitmap.pix16(offs, bx) != 0) m_collision_register |= 0x08;

				bitmap.pix16(offs, bx) = 7;
			}
		}
	}


	/* mix and copy the S2636 images into the main bitmap, also check for collision */
	{
		int y;

		for (y = cliprect.min_y; y <= cliprect.max_y; y++)
		{
			int x;

			for (x = cliprect.min_x; x <= cliprect.max_x; x++)
			{
				int pixel0 = s2636_0_bitmap.pix16(y, x);
				int pixel1 = s2636_1_bitmap.pix16(y, x);
				int pixel2 = s2636_2_bitmap.pix16(y, x);

				int pixel = pixel0 | pixel1 | pixel2;

				if (S2636_IS_PIXEL_DRAWN(pixel))
				{
					bitmap.pix16(y, x) = S2636_PIXEL_COLOR(pixel);

					/* S2636 vs. background collision detection */
					if (m_palette->pen_indirect(m_collision_background.pix16(y, x)))
					{
						if (S2636_IS_PIXEL_DRAWN(pixel0)) m_collision_register |= 0x01;
						if (S2636_IS_PIXEL_DRAWN(pixel2)) m_collision_register |= 0x02;
					}
				}
			}
		}
	}

	return 0;
}
