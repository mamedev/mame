// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Haywood
/*
 Deco MXC06 sprite generator:

 used by:
 madmotor.c

Notes (dec0.c)

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


todo:
    Implement sprite/tilemap orthogonality (not strictly needed as no
    games make deliberate use of it). (pdrawgfx, or rendering to bitmap for manual mixing)

*/


#include "emu.h"
#include "decmxc06.h"

void deco_mxc06_device::set_gfx_region(device_t &device, int region)
{
	deco_mxc06_device &dev = downcast<deco_mxc06_device &>(device);
	dev.m_gfxregion = region;
}


const device_type DECO_MXC06 = &device_creator<deco_mxc06_device>;

deco_mxc06_device::deco_mxc06_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, DECO_MXC06, "DECO MXC06 Sprite", tag, owner, clock, "deco_mxc06", __FILE__),
		device_video_interface(mconfig, *this),
		m_gfxregion(0),
		m_ramsize(0x800),
		m_gfxdecode(*this)
{
}

//-------------------------------------------------
//  static_set_gfxdecode_tag: Set the tag of the
//  gfx decoder
//-------------------------------------------------

void deco_mxc06_device::static_set_gfxdecode_tag(device_t &device, const char *tag)
{
	downcast<deco_mxc06_device &>(device).m_gfxdecode.set_tag(tag);
}


/* this implementation was originally from Mad Motor */
void deco_mxc06_device::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, UINT16* spriteram, int pri_mask, int pri_val, int col_mask )
{
	int offs;

	offs = 0;
	while (offs < m_ramsize / 2)
	{
		int sx, sy, code, color, w, h, flipx, flipy, incy, flash, mult, x, y;

		sy = spriteram[offs];
		sx = spriteram[offs + 2];
		color = sx >> 12;

		flash = sx & 0x800;

		flipx = sy & 0x2000;
		flipy = sy & 0x4000;
		h = (1 << ((sy & 0x1800) >> 11));   /* 1x, 2x, 4x, 8x height */
		w = (1 << ((sy & 0x0600) >>  9));   /* 1x, 2x, 4x, 8x width */
		/* multi width used only on the title screen? */



		sx = sx & 0x01ff;
		sy = sy & 0x01ff;
		if (sx >= 256) sx -= 512;
		if (sy >= 256) sy -= 512;
		sx = 240 - sx;
		sy = 240 - sy;

		if (machine().driver_data()->flip_screen())
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


		for (x = 0; x < w; x++)
		{
			// maybe, birdie try appears to specify the base code for each part..
			code = spriteram[offs + 1] & 0x1fff;

			code &= ~(h - 1);

			if (flipy)
				incy = -1;
			else
			{
				code += h - 1;
				incy = 1;
			}

			for (y = 0; y < h; y++)
			{
				{
					int draw = 0;
					if (!flash || (m_screen->frame_number() & 1))
					{
						if (m_priority_type == 0) // most cases
						{
							if ((color & pri_mask) == pri_val)
							{
								draw = 1;
							}
						}
						else if (m_priority_type == 1) // vaportra
						{
							if (pri_mask && (color >= pri_val))
								continue;

							if (!pri_mask && !(color >= pri_val))
								continue;

							draw = 1;
						}
					}

					if (draw)
					{
						m_gfxdecode->gfx(m_gfxregion)->transpen(bitmap, cliprect,
							code - y * incy,
							color & col_mask,
							flipx, flipy,
							sx + (mult * x), sy + (mult * y), 0);
					}
				}
			}

			offs += 4;
			if (offs >= m_ramsize / 2)
				return;


		}
	}
}

/* this is used by the automat bootleg, it seems to have greatly simplified sprites compared to the real chip */
/* spriteram is twice the size tho! */
void deco_mxc06_device::draw_sprites_bootleg( bitmap_ind16 &bitmap, const rectangle &cliprect, UINT16* spriteram, int pri_mask, int pri_val, int col_mask )
{
	int offs;

	offs = 0;
	while (offs < m_ramsize / 2)
	{
		int sx, sy, code, color, flipx, flipy;

		code =  spriteram[offs];
		sy = 240-spriteram[offs + 1]; // 241- will align robocop with the ground but causes other issues too
		sx = spriteram[offs + 2];
		code |= (spriteram[offs + 3] &0x0f)<<8;
		flipx = !(spriteram[offs + 3] &0x20);
		flipy = (spriteram[offs + 3] &0x40);
		color = (spriteram[offs + 0x400]&0xf0)>>4;
		sx |= (spriteram[offs + 0x400]&0x01)<<8;
		sx -= 16;
		sx &=0x1ff;

		sx -= 0x100;

		m_gfxdecode->gfx(m_gfxregion)->transpen(bitmap,cliprect,
			code,
			color & col_mask,
			flipx,flipy,
			sx,sy,0);

		offs += 4;
	}
}

void deco_mxc06_device::device_start()
{
	m_priority_type = 0;
}

void deco_mxc06_device::device_reset()
{
}
