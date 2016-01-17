// license:BSD-3-Clause
// copyright-holders:Bryan McPhail,David Haywood
/* Data East 'Karnov style' sprites */
/* Custom Chip ??? */

#include "emu.h"
#include "deckarn.h"

void deco_karnovsprites_device::set_gfx_region(device_t &device, int region)
{
	deco_karnovsprites_device &dev = downcast<deco_karnovsprites_device &>(device);
	dev.m_gfxregion = region;
}

const device_type DECO_KARNOVSPRITES = &device_creator<deco_karnovsprites_device>;

deco_karnovsprites_device::deco_karnovsprites_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, DECO_KARNOVSPRITES, "DECO Karnov Sprites", tag, owner, clock, "deco_karnovsprites", __FILE__),
		m_gfxregion(0),
		m_gfxdecode(*this),
		m_palette(*this)
{
}

//-------------------------------------------------
//  static_set_gfxdecode_tag: Set the tag of the
//  gfx decoder
//-------------------------------------------------

void deco_karnovsprites_device::static_set_gfxdecode_tag(device_t &device, std::string tag)
{
	downcast<deco_karnovsprites_device &>(device).m_gfxdecode.set_tag(tag);
}

void deco_karnovsprites_device::device_start()
{
}

void deco_karnovsprites_device::device_reset()
{
}

void deco_karnovsprites_device::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, UINT16* spriteram, int size, int priority )
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
		if (machine().driver_data()->flip_screen())
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

//-------------------------------------------------
//  static_set_palette_tag: Set the tag of the
//  palette device
//-------------------------------------------------

void deco_karnovsprites_device::static_set_palette_tag(device_t &device, std::string tag)
{
	downcast<deco_karnovsprites_device &>(device).m_palette.set_tag(tag);
}
