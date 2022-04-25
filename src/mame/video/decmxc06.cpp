// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Haywood
/*
 Deco MXC06 sprite generator:

 used by:
 madmotor.cpp
 dec0.cpp
 stadhero.cpp
 thedeep.cpp
 actfancr.cpp
 vaportra.cpp
 dec8.cpp

Notes (dec0.cpp)

   Sprite data:  The unknown bits seem to be unused.

    Byte 0:
        Bit 0 : Y co-ord hi bit
        Bit 1,2 : Sprite width (1x, 2x, 4x, 8x)
        Bit 3,4 : Sprite height (1x, 2x, 4x, 8x)
        Bit 5  - X flip
        Bit 6  - Y flip
        Bit 7  - Only display Sprite if set
    Byte 1: Y-coords
    Byte 2:
        Bit 0,1,2,3: Hi bits of sprite number
        Bit 4,5,6,7: (Probably unused MSB's of sprite)
    Byte 3: Low bits of sprite number
    Byte 4:
        Bit 0 : X co-ords hi bit
        Bit 1,2: ??
        Bit 3: Sprite flash (sprite is displayed every other frame)
        Bit 4,5,6,7:  - Colour
    Byte 5: X-coords

*/


#include "emu.h"
#include "decmxc06.h"
#include "screen.h"


DEFINE_DEVICE_TYPE(DECO_MXC06, deco_mxc06_device, "deco_mxc06", "DECO MXC06 Sprite")

deco_mxc06_device::deco_mxc06_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DECO_MXC06, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, m_colpri_cb(*this)
{
}

/* this implementation was originally from Mad Motor */
void deco_mxc06_device::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx, u16* spriteram, int size)
{
	const bool priority = !m_colpri_cb.isnull();
	struct sprite_t *sprite_ptr = m_spritelist.get();

	for (int offs = 0; offs < size;)
	{
		u32 pri_mask = 0;
		int flipy, incy, mult, parentFlipY;

		const u16 data0 = spriteram[offs];
		const u16 data2 = spriteram[offs + 2];
		u32 colour = data2 >> 12;
		if (priority)
			m_colpri_cb(colour, pri_mask);

		const bool flash = data2 & 0x800;

		int flipx = data0 & 0x2000;
		parentFlipY = flipy = data0 & 0x4000;
		const int h = (1 << ((data0 & 0x1800) >> 11));   /* 1x, 2x, 4x, 8x height */
		const int w = (1 << ((data0 & 0x0600) >>  9));   /* 1x, 2x, 4x, 8x width */

		int sx = data2 & 0x01ff;
		int sy = data0 & 0x01ff;
		if (sx >= 256) sx -= 512;
		if (sy >= 256) sy -= 512;
		sx = 240 - sx;
		sy = 240 - sy;

		if (m_flip_screen)
		{
			sy = 240 - sy;
			sx = 240 - sx;
			if (flipx) flipx = 0; else flipx = 1;
			if (flipy) flipy = 0; else flipy = 1;
			mult = 16;
		}
		else
			mult = -16;

		// thedeep strongly suggests that this check goes here, otherwise the radar breaks
		if (!(spriteram[offs] & 0x8000))
		{
			offs += 4;
			continue;
		}

		for (int x = 0; x < w; x++)
		{
			if (offs < size)
			{
				// maybe, birdie try appears to specify the base code for each part..
				u16 code = spriteram[offs + 1] & 0x1fff;

				code &= ~(h - 1);

				// not affected by flipscreen
				if (parentFlipY) // in the case of multi-width sprites the y flip bit is set by the parent
					incy = -1;
				else
				{
					code += h - 1;
					incy = 1;
				}

				if (!flash || (screen.frame_number() & 1))
				{
					sprite_ptr->height = h;
					sprite_ptr->colour = colour;
					sprite_ptr->flipx = flipx;
					sprite_ptr->flipy = flipy;
					if (priority)
					{
						sprite_ptr->pri_mask = pri_mask;
						for (int y = 0; y < h; y++)
						{
							sprite_ptr->code[y] = code - y * incy;
							sprite_ptr->x[y] = sx + (mult * x);
							sprite_ptr->y[y] = sy + (mult * y);
						}
						sprite_ptr++;
					}
					else
					{
						for (int y = 0; y < h; y++)
						{
							sprite_ptr->code[y] = code - y * incy;
							sprite_ptr->x[y] = sx + (mult * x);
							sprite_ptr->y[y] = sy + (mult * y);
							gfx->transpen(bitmap, cliprect,
								sprite_ptr->code[y],
								sprite_ptr->colour,
								sprite_ptr->flipx, sprite_ptr->flipy,
								sprite_ptr->x[y], sprite_ptr->y[y], 0);
						}
					}
				}
			}
			offs += 4;
		}
	}

	if (priority)
	{
		while (sprite_ptr != m_spritelist.get())
		{
			sprite_ptr--;

			for (int y = 0; y < sprite_ptr->height; y++)
			{
				gfx->prio_transpen(bitmap, cliprect,
						sprite_ptr->code[y],
						sprite_ptr->colour,
						sprite_ptr->flipx, sprite_ptr->flipy,
						sprite_ptr->x[y], sprite_ptr->y[y], screen.priority(), sprite_ptr->pri_mask, 0);
			};
		}
	}
}

/* this is used by the automat bootleg, it seems to have greatly simplified sprites compared to the real chip */
/* spriteram is twice the size tho! */
void deco_mxc06_device::draw_sprites_bootleg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx, u16* spriteram, int size)
{
	const bool priority = !m_colpri_cb.isnull();
	struct sprite_t *sprite_ptr = m_spritelist.get();

	for (int offs = 0; offs < size; offs += 4)
	{
		u32 pri_mask = 0;
		u32 code =  spriteram[offs];
		int sy = 240-spriteram[offs + 1]; // 241- will align robocop with the ground but causes other issues too
		int sx = spriteram[offs + 2];
		code |= (spriteram[offs + 3] &0x0f)<<8;
		const bool flipx = !(spriteram[offs + 3] &0x20);
		const bool flipy = (spriteram[offs + 3] &0x40);
		u32 colour = (spriteram[offs + 0x400] & 0xf0) >> 4;
		if (priority)
			m_colpri_cb(colour, pri_mask);

		sx |= (spriteram[offs + 0x400] & 0x01) << 8;
		sx -= 16;
		sx &=0x1ff;

		sx -= 0x100;

		sprite_ptr->colour = colour;
		sprite_ptr->flipx = flipx;
		sprite_ptr->flipy = flipy;
		sprite_ptr->code[0] = code;
		sprite_ptr->x[0] = sx;
		sprite_ptr->y[0] = sy;
		if (priority)
		{
			sprite_ptr->pri_mask = pri_mask;
			sprite_ptr++;
		}
		else
		{
			gfx->transpen(bitmap,cliprect,
				sprite_ptr->code[0],
				sprite_ptr->colour,
				sprite_ptr->flipx,sprite_ptr->flipy,
				sprite_ptr->x[0],sprite_ptr->y[0],0);
		}
	}

	if (priority)
	{
		while (sprite_ptr != m_spritelist.get())
		{
			sprite_ptr--;

			gfx->prio_transpen(bitmap, cliprect,
					sprite_ptr->code[0],
					sprite_ptr->colour,
					sprite_ptr->flipx, sprite_ptr->flipy,
					sprite_ptr->x[0], sprite_ptr->y[0], screen.priority(), sprite_ptr->pri_mask, 0);
		}
	}
}

void deco_mxc06_device::device_start()
{
	m_colpri_cb.resolve();
	m_flip_screen = false;
	m_spritelist = std::make_unique<sprite_t[]>(0x400);

	save_item(NAME(m_flip_screen));
}

void deco_mxc06_device::device_reset()
{
}
