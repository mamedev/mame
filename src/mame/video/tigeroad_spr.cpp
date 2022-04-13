// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

    very simple sprite scheme, used by some Capcom games and hardware cloned from them

    bionicc.cpp
    tigeroad.cpp
    supduck.cpp

    it is unknown if this is handled by a custom chip, or simple logic.
    y positions are inverted in Bionic Commando, but it seems otherwise the same as
    Tiger Road

*/


#include "emu.h"
#include "tigeroad_spr.h"


DEFINE_DEVICE_TYPE(TIGEROAD_SPRITE, tigeroad_spr_device, "tigeroad_spr", "Simple Capcom (Tiger Road) Sprite")

tigeroad_spr_device::tigeroad_spr_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, TIGEROAD_SPRITE, tag, owner, clock)
	, device_gfx_interface(mconfig, *this, nullptr)
	, m_gfx_region(*this, DEVICE_SELF)
	, m_colbase(0)
{
}


void tigeroad_spr_device::device_start()
{
	gfx_layout layout_16x16x4 =
	{
		16,16,
		0,
		4,
		{ STEP4(0,8) },
		{ STEP8(0,1), STEP8(8*4*16,1) },
		{ STEP16(0,8*4) },
		16*16*4
	};
	layout_16x16x4.total = m_gfx_region->bytes() / ((16*16*4) / 8);
	set_gfx(0, std::make_unique<gfx_element>(&palette(), layout_16x16x4, m_gfx_region->base(), 0, 0x10, m_colbase));
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


void tigeroad_spr_device::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, u16* ram, u32 size, bool flip_screen, bool rev_y)
{
	const u16 *source = &ram[size/2] - 4;
	const u16 *finish = ram;

	while (source >= finish)
	{
		const u32 tile_number = source[0];

		const u16 attr = source[1];
		int sy = source[2] & 0x1ff;
		int sx = source[3] & 0x1ff;

		int flipx = attr & 0x02;
		int flipy = attr & 0x01;
		const u32 color = (attr >> 2) & 0x0f;

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

		gfx(0)->transpen(bitmap,cliprect,
		tile_number,
		color,
		flipx, flipy,
		sx+128, sy+6, 15);

		source -= 4;
	}
}
