// license:BSD-3-Clause
// copyright-holders:Bryan McPhail,David Haywood
/* Data East 'Karnov style' sprites */
/* Custom Chip ??? */

#include "emu.h"
#include "deckarn.h"

DEFINE_DEVICE_TYPE(DECO_KARNOVSPRITES, deco_karnovsprites_device, "deco_karnovsprites", "DECO Karnov Sprites")

deco_karnovsprites_device::deco_karnovsprites_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, DECO_KARNOVSPRITES, tag, owner, clock)
	, device_gfx_interface(mconfig, *this)
	, m_colpri_cb(*this)
	, m_flip_screen(false)
{
}

void deco_karnovsprites_device::device_start()
{
	m_colpri_cb.resolve();

	save_item(NAME(m_flip_screen));
}


// Sprite format:
/*

  fedcba98 76543210
0 x....... ........ show?
0 ....x... ........ size 16x32
0 .......x xxxxxxxx y

1 ........ x....... must be 0 for 16x32 sprite to work right
1 ........ ..x..... y zoom (size is still 16x16 or 16x32)
1 ........ ...x.... must be 1 for 16x32 sprite to work right
1 ........ .....x.. flip x
1 ........ ......x. flip y
1 ........ .......x show?

2 .......x xxxxxxxx x

3 xxxx.... ........ colour
3 ....xxxx xxxxxxxx code

data1 bits 4 and 7 are weird sprite code masks, and can cause glitches with 16x32 sprites.
The y zoom flag isn't used by any games.

*/

void deco_karnovsprites_device::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, const u16 *spriteram, int size)
{
	const bool priority = !m_colpri_cb.isnull();
	const int start = priority ? (size - 4) : 0;
	const int end = priority ? -4 : size;
	const int inc = priority ? -4 : 4;

	for (int offs = start; offs != end; offs += inc)
	{
		const u16 data0 = spriteram[offs];
		if (!BIT(data0, 15))
			continue;

		int y = data0 & 0x1ff;
		const u16 data3 = spriteram[offs + 3];
		u32 colour = data3 >> 12;
		u32 pri_mask = 0;
		if (priority)
			m_colpri_cb(colour, pri_mask);

		u32 sprite = data3 & 0xfff;
		int x = spriteram[offs + 2] & 0x1ff;

		const u16 data1 = spriteram[offs + 1];

		// the 8-bit implementation had this, illustrated by enemy projectile explosions in Shackled being left on screen
		if (!BIT(data1, 0))
			continue;

		const bool extra = BIT(data0, 11);
		int fy = BIT(data1, 1);
		int fx = BIT(data1, 2);

		if (extra)
		{
			y += 16;
			sprite &= 0xffe; // taken from 8-bit version
		}

		// convert the coords
		x = (x + 16) & 0x1ff;
		y = (y + 16) & 0x1ff;
		x = 256 - x;
		y = 256 - y;
		if (m_flip_screen)
		{
			y = 240 - y;
			x = 240 - x;
			fx = !fx;
			fy = !fy;
			if (extra)
				y -= 16;
		}

		// y flip determines order of multi-sprite
		int sprite2;
		if (extra && fy)
		{
			sprite2 = sprite;
			sprite |= 1;
		}
		else
			sprite2 = sprite | 1;

		if (priority)
		{
			gfx(0)->prio_transpen(bitmap, cliprect,
					sprite, colour,
					fx, fy, x, y,
					screen.priority(), pri_mask, 0);

			// 1 more sprite drawn underneath
			if (extra)
			{
				gfx(0)->prio_transpen(bitmap, cliprect,
						sprite2, colour,
						fx, fy, x, y + 16,
						screen.priority(), pri_mask, 0);
			}
		}
		else
		{
			gfx(0)->transpen(bitmap,cliprect,
					sprite, colour,
					fx, fy, x, y, 0);

			// 1 more sprite drawn underneath
			if (extra)
			{
				gfx(0)->transpen(bitmap, cliprect,
						sprite2, colour,
						fx, fy, x, y + 16, 0);
			}
		}
	}
}
