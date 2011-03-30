/* Data East 'Karnov style' sprites */
/* Custom Chip ??? */

#include "emu.h"
#include "deckarn.h"

deco_karnovsprites_device_config::deco_karnovsprites_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
	: device_config(mconfig, static_alloc_device_config, "karnovsprites_device", tag, owner, clock)
{
	m_gfxregion = 0;
}

device_config *deco_karnovsprites_device_config::static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
{
	return global_alloc(deco_karnovsprites_device_config(mconfig, tag, owner, clock));
}

device_t *deco_karnovsprites_device_config::alloc_device(running_machine &machine) const
{
	return auto_alloc(machine, deco_karnovsprites_device(machine, *this));
}

void deco_karnovsprites_device_config::set_gfx_region(device_config *device, int region)
{
	deco_karnovsprites_device_config *dev = downcast<deco_karnovsprites_device_config *>(device);
	dev->m_gfxregion = region;
}


deco_karnovsprites_device::deco_karnovsprites_device(running_machine &_machine, const deco_karnovsprites_device_config &config)
	: device_t(_machine, config),
	  m_config(config),
	  m_gfxregion(m_config.m_gfxregion)
{
}

void deco_karnovsprites_device::device_start()
{
	
}

void deco_karnovsprites_device::device_reset()
{

}

void deco_karnovsprites_device::draw_sprites( running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect, UINT16* spriteram, int size, int priority )
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

		// the 8-bit implementation had this, why?
		//if ((fx & 0x1) == 0) continue;

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
		if (flip_screen_get(machine))
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

		drawgfx_transpen(bitmap,cliprect,machine.gfx[m_gfxregion],
				sprite,
				colour,fx,fy,x,y,0);

    	/* 1 more sprite drawn underneath */
    	if (extra)
    		drawgfx_transpen(bitmap,cliprect,machine.gfx[m_gfxregion],
				sprite2,
				colour,fx,fy,x,y+16,0);
	}
}
