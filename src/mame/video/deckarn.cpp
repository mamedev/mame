// license:BSD-3-Clause
// copyright-holders:Bryan McPhail,David Haywood
/* Data East 'Karnov style' sprites */
/* Custom Chip ??? */

#include "emu.h"
#include "deckarn.h"

DEFINE_DEVICE_TYPE(DECO_KARNOVSPRITES, deco_karnovsprites_device, "deco_karnovsprites", "DECO Karnov Sprites")

deco_karnovsprites_device::deco_karnovsprites_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DECO_KARNOVSPRITES, tag, owner, clock)
	, m_gfxregion(0)
	, m_gfxdecode(*this, finder_base::DUMMY_TAG)
{
}

void deco_karnovsprites_device::device_start()
{
	m_flip_screen = false;

	save_item(NAME(m_flip_screen));
}

void deco_karnovsprites_device::device_reset()
{
}

void deco_karnovsprites_device::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, uint16_t* spriteram, int size, int priority )
{
	int offs;

	for (offs = 0; offs < size; offs += 4)
	{
		int x, y, sprite, sprite2, colour, fx, fy, extra;

		y = spriteram[offs];
		if (!(y & 0x8000))
			continue;

		y = y & 0x1ff;
		sprite = spriteram[offs + 3];
		colour = sprite >> 12;

		if (priority == 1 && (colour & 8)) continue;
		if (priority == 2 && !(colour & 8)) continue;

		sprite = sprite & 0xfff;
		x = spriteram[offs + 2] & 0x1ff;

		fx = spriteram[offs + 1];

		/* the 8-bit implementation had this.
		           illustrated by enemy projectile explosions in Shackled being left on screen. */
		if ((fx & 0x1) == 0) continue;

		extra = (fx & 0x10) ? 1 : 0;
		fy = fx & 0x2;
		fx = fx & 0x4;

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

		m_gfxdecode->gfx(m_gfxregion)->transpen(bitmap,cliprect,
				sprite,
				colour,fx,fy,x,y,0);

		/* 1 more sprite drawn underneath */
		if (extra)
			m_gfxdecode->gfx(m_gfxregion)->transpen(bitmap,cliprect,
				sprite2,
				colour,fx,fy,x,y+16,0);
	}
}
