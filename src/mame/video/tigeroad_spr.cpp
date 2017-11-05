// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

    very simple sprite scheme, used by some Capcom games and hardware cloned from them

    bionicc.c
    tigeroad.c
    supduck.c

    it is unknown if this is handled by a custom chip, or simple logic.
    y positions are inverted in Bionic Commando, but it seems otherwise the same as
    Tiger Road

*/


#include "emu.h"
#include "tigeroad_spr.h"


DEFINE_DEVICE_TYPE(TIGEROAD_SPRITE, tigeroad_spr_device, "tigeroad_spr", "Simple Capcom (Tiger Road) Sprite")

tigeroad_spr_device::tigeroad_spr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TIGEROAD_SPRITE, tag, owner, clock)
{
}


void tigeroad_spr_device::device_start()
{
}



void tigeroad_spr_device::device_reset()
{
}

/*
   4  words per sprite

   0  ---- ---t tttt tttt = tile number

   1  ---- ---- --cc cc-- = colour
   1  ---- ---- ---- --x- = flip x
   1  ---- ---- ---- ---y = flip y

   2  ---- ---x xxxx xxxx = x pos (signed)

   3  ---- ---y yyyy yyyy = y pos (signed)

*/


void tigeroad_spr_device::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, gfxdecode_device *gfxdecode, int region, uint16_t* ram, uint32_t size, int flip_screen, int rev_y )
{
	uint16_t *source = &ram[size/2] - 4;
	uint16_t *finish = ram;

	while (source >= finish)
	{
		int tile_number = source[0];

		int attr = source[1];
		int sy = source[2] & 0x1ff;
		int sx = source[3] & 0x1ff;

		int flipx = attr & 0x02;
		int flipy = attr & 0x01;
		int color = (attr >> 2) & 0x0f;

		if (sx > 0x100) sx -= 0x200;
		if (sy > 0x100) sy -= 0x200;

		if (flip_screen)
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		if (rev_y)
			sy = 240 - sy;


		gfxdecode->gfx(region)->transpen(bitmap,cliprect,
		tile_number,
		color,
		flipx, flipy,
		sx, sy, 15);

		source -= 4;
	}
}
