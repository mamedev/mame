// license:BSD-3-Clause
// copyright-holders:Bryan McPhail,David Haywood
/* Data East 'Karnov style' sprites */
/* Custom Chip ??? */

#include "emu.h"
#include "deckarn.h"

DEFINE_DEVICE_TYPE(DECO_KARNOVSPRITES, deco_karnovsprites_device, "deco_karnovsprites", "DECO Karnov Sprites")

deco_karnovsprites_device::deco_karnovsprites_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, DECO_KARNOVSPRITES, tag, owner, clock)
{
}

void deco_karnovsprites_device::device_start()
{
	m_flip_screen = false;
	m_colpri_cb.bind_relative_to(*owner());

	save_item(NAME(m_flip_screen));
}

void deco_karnovsprites_device::device_reset()
{
}

void deco_karnovsprites_device::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx, u16* spriteram, int size)
{
	const bool priority = !m_colpri_cb.isnull();
	int start, end, inc;
	if (priority)             { start = size - 4; end =   -4; inc = -4; }
	else                      { start =        0; end = size; inc = +4; }

	for (int offs = start; offs != end; offs += inc)
	{
		int sprite2;
		u32 pri_mask = 0;

		const u16 data0 = spriteram[offs];
		if (!(data0 & 0x8000))
			continue;

		int y = data0 & 0x1ff;
		const u16 data3 = spriteram[offs + 3];
		u32 colour = data3 >> 12;
		if (priority)
			m_colpri_cb(colour, pri_mask);

		u32 sprite = data3 & 0xfff;
		int x = spriteram[offs + 2] & 0x1ff;

		const u16 data1 = spriteram[offs + 1];

		/* the 8-bit implementation had this.
		           illustrated by enemy projectile explosions in Shackled being left on screen. */
		if ((data1 & 0x1) == 0) continue;

		const bool extra = (data1 & 0x10) ? 1 : 0;
		int fy = data1 & 0x2;
		int fx = data1 & 0x4;

		if (extra)
		{
			y = y + 16;
			sprite &= 0xffe; // taken from 8-bit version
		}

		/* Convert the co-ords..*/
		x = (x + 16) % 0x200;
		y = (y + 16) % 0x200;
		x = 256 - x;
		y = 256 - y;
		if (m_flip_screen)
		{
			y = 240 - y;
			x = 240 - x;
			if (fx) fx = 0; else fx = 1;
			if (fy) fy = 0; else fy = 1;
			if (extra) y = y - 16;
		}

		/* Y Flip determines order of multi-sprite */
		if (extra && fy)
		{
			sprite2 = sprite;
			sprite++;
		}
		else
			sprite2 = sprite + 1;

		if (priority)
		{
			gfx->prio_transpen(bitmap,cliprect,
					sprite,
					colour,fx,fy,x,y,screen.priority(),pri_mask,0);

			/* 1 more sprite drawn underneath */
			if (extra)
				gfx->prio_transpen(bitmap,cliprect,
					sprite2,
					colour,fx,fy,x,y+16,screen.priority(),pri_mask,0);
		}
		else
		{
			gfx->transpen(bitmap,cliprect,
					sprite,
					colour,fx,fy,x,y,0);

			/* 1 more sprite drawn underneath */
			if (extra)
				gfx->transpen(bitmap,cliprect,
					sprite2,
					colour,fx,fy,x,y+16,0);
		}
	}
}
