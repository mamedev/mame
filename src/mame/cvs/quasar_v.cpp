// license:BSD-3-Clause
// copyright-holders:Mike Coates, Pierpaolo Prazzoli
/***************************************************************************

  video\quasar.cpp

  Functions to emulate the video hardware of the machine.

  Zaccaria S2650 games share various levels of design with the Century Video
  System (CVS) games, and hence some routines are shared from there.

  Shooting seems to mix custom boards from Zaccaria and sound boards from CVS
  hinting at a strong link between the two companies.

  Zaccaria are an italian company, Century were based in Manchester UK

***************************************************************************/

#include "emu.h"
#include "cpu/s2650/s2650.h"
#include "quasar.h"

void quasar_state::quasar_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	// standard 1 bit per color palette (background and sprites)
	for (int i = 0; i < 8; i++)
		palette.set_indirect_color(i, rgb_t(pal1bit(i >> 0), pal1bit(i >> 1), pal1bit(i >> 2)));

	// effects color map
	for (int i = 0; i < 0x100; i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(i, 0);
		bit1 = BIT(i, 1);
		bit2 = BIT(i, 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// green component
		bit0 = BIT(i, 3);
		bit1 = BIT(i, 4);
		bit2 = BIT(i, 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// blue component
		bit0 = BIT(i, 6);
		bit1 = BIT(i, 7);
		int const b = 0x4f * bit0 + 0xa8 * bit1;

		// intensity 0
		palette.set_indirect_color(0x100 + i, rgb_t::black());

		// intensity 1
		palette.set_indirect_color(0x200 + i, rgb_t(r >> 2, g >> 2, b >> 2));

		/* intensity 2 */
		palette.set_indirect_color(0x300 + i, rgb_t((r >> 2) + (r >> 3), (g >> 2) + (g >> 3), (b >> 2) + (b >> 2)));

		/* intensity 3 */
		palette.set_indirect_color(0x400 + i, rgb_t(r >> 1, g >> 1, b >> 1));
	}

	// Address 0-2 from graphic rom
	//         3-5 from color ram
	//         6-8 from sprite chips (Used for priority)
	for (int i = 0; i < 0x200; i++)
		palette.set_pen_indirect(i, color_prom[i] & 0x07);

	// background for collision
	for (int i = 1; i < 8; i++)
		palette.set_pen_indirect(0x200 + i, 7);
	palette.set_pen_indirect(0x200, 0);

	// effects
	for (int i = 0; i < 0x400; i++)
		palette.set_pen_indirect(0x208 + i, 0x100 + i);
}


void quasar_state::video_start()
{
	m_effectram = std::make_unique<uint8_t[]>(0x400);

	/* create helper bitmap */
	m_screen->register_screen_bitmap(m_collision_background);

	/* register save */
	save_item(NAME(m_collision_background));
	save_pointer(NAME(m_effectram), 0x400);
}

uint32_t quasar_state::screen_update_quasar(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// for every character in the video RAM
	for (int offs = 0; offs < 0x0400; offs++)
	{
		uint8_t const code = m_video_ram[offs];
		uint8_t const x = (offs & 0x1f) << 3;
		uint8_t const y = (offs >> 5) << 3;

		// While we have the current character code, draw the effects layer
		// intensity / on and off controlled by latch

		int const forecolor = 0x208 + m_effectram[offs] + (256 * (((m_effectcontrol >> 4) ^ 3) & 3));

		for (int ox = 0; ox < 8; ox++)
			for (int oy = 0; oy < 8; oy++)
				bitmap.pix(y + oy, x + ox) = forecolor;

		// Main Screen
		m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
				code,
				m_color_ram[offs] & 0x3f,
				0,0,
				x,y,0);


		// background for Collision Detection (it can only hit certain items)
		if ((m_color_ram[offs] & 7) == 0)
		{
			m_gfxdecode->gfx(0)->opaque(m_collision_background,cliprect,
					code,
					64,
					0,0,
					x,y);
		}
	}

	/* update the S2636 chips */
	bitmap_ind16 const &s2636_0_bitmap = m_s2636[0]->update(cliprect);
	bitmap_ind16 const &s2636_1_bitmap = m_s2636[1]->update(cliprect);
	bitmap_ind16 const &s2636_2_bitmap = m_s2636[2]->update(cliprect);

	// Bullet Hardware
	for (int offs = 8; offs < 256; offs++)
	{
		if (m_bullet_ram[offs] != 0)
		{
			for (int ct = 0; ct < 1; ct++)
			{
				int const bx = 255 - 9 - m_bullet_ram[offs] - ct;

				// bullet/object Collision
				if (s2636_0_bitmap.pix(offs, bx) != 0) m_collision_register |= 0x04;
				if (s2636_2_bitmap.pix(offs, bx) != 0) m_collision_register |= 0x08;

				bitmap.pix(offs, bx) = 7;
			}
		}
	}


	// mix and copy the S2636 images into the main bitmap, also check for collision
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			int const pixel0 = s2636_0_bitmap.pix(y, x);
			int const pixel1 = s2636_1_bitmap.pix(y, x);
			int const pixel2 = s2636_2_bitmap.pix(y, x);

			int const pixel = pixel0 | pixel1 | pixel2;

			if (S2636_IS_PIXEL_DRAWN(pixel))
			{
				bitmap.pix(y, x) = S2636_PIXEL_COLOR(pixel);

				/* S2636 vs. background collision detection */
				if (m_palette->pen_indirect(m_collision_background.pix(y, x)))
				{
					if (S2636_IS_PIXEL_DRAWN(pixel0)) m_collision_register |= 0x01;
					if (S2636_IS_PIXEL_DRAWN(pixel2)) m_collision_register |= 0x02;
				}
			}
		}
	}

	return 0;
}
