// license:BSD-3-Clause
// copyright-holders:David Graves, Angelo Salese, David Haywood, Tomasz Slanina, Carlos A. Lozano, Bryan McPhail, Pierpaolo Prazzoli
/*
    Seibu Kaihatsu SEI0210/SEI0211/SEI0220(BP) Sprite generator emulation

    Used by Seibu Kaihatsu at 1990 to 1994, SEI0210/SEI0211 is paired with
    SEI0220(BP).

    SEI0210 and SEI0211 is similar, but coordinate format is different.
    Another difference between these chips are still unknown.

    Used in:
        banprestoms.cpp
        bloodbro.cpp*
        dcon.cpp
        goodejan.cpp
        legionna.cpp
        sengokmj.cpp

    * Using alternative sprite format.

    TODO:
    - flip screen support
*/

#include "emu.h"
#include "sei021x_sei0220_spr.h"
#include "screen.h"


DEFINE_DEVICE_TYPE(SEI0210, sei0210_device, "sei0210", "Seibu Kaihatsu SEI0210 Sprite generator")
DEFINE_DEVICE_TYPE(SEI0211, sei0211_device, "sei0211", "Seibu Kaihatsu SEI0211 Sprite generator")

sei0210_device::sei0210_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_gfx_interface(mconfig, *this)
	, m_pri_cb(*this)
	, m_gfxbank_cb(*this)
	, m_alt_format(false)
	, m_xoffset(0)
	, m_yoffset(0)
{
}

sei0210_device::sei0210_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: sei0210_device(mconfig, SEI0210, tag, owner, clock)
{
}

sei0211_device::sei0211_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: sei0210_device(mconfig, SEI0211, tag, owner, clock)
{
}

void sei0210_device::device_start()
{
	m_pri_cb.resolve();
	m_gfxbank_cb.resolve();
}

void sei0210_device::device_reset()
{
}

/*

===============================================================

Common sprite format (8 byte per sprites)

Offset Bit                 Description
       fedc ba98 7654 3210
00     x--- ---- ---- ---- Enable this sprite
       -x-- ---- ---- ---- Flip X
       --x- ---- ---- ---- Flip Y
       ---x xx-- ---- ---- Sprite width
       ---- --xx x--- ---- Sprite height
       ---- ---- -x-- ---- (Optional) Extra bit
       ---- ---- --xx xxxx Color index
02     xx-- ---- ---- ---- Priority
       --xx xxxx xxxx xxxx Tile index
04     x--- ---- ---- ---- (Optional) X sign bit
       ---- ---x xxxx xxxx X position
06     x--- ---- ---- ---- (Optional) Y sign bit or Extra bit
       ---- ---x xxxx xxxx Y position

Unmarked bits are unused/unknown.

===============================================================

Blood Bros. sprite format (8 byte per sprites)

Offset Bit                 Description
       fedc ba98 7654 3210
00     x--- ---- ---- ---- Disable this sprite
       -x-- ---- ---- ---- Flip Y
       --x- ---- ---- ---- Flip X
       ---- x--- ---- ---- Priority
       ---- --xx x--- ---- Sprite width
       ---- ---- -xxx ---- Sprite height
       ---- ---- ---- xxxx Color index
02     ---x xxxx xxxx xxxx Tile index
04     x--- ---- ---- ---- (Optional) X sign bit
       ---- ---x xxxx xxxx X position
06     x--- ---- ---- ---- (Optional) Y sign bit
       ---- ---x xxxx xxxx Y position

Unmarked bits are unused/unknown.

===============================================================

*/
template<class T>
void sei0210_device::draw(screen_device &screen, T &bitmap, const rectangle cliprect, u16* spriteram, u16 size)
{
	for (int i = 0; i < (size / 2); i += 4)
	{
		bool flipx, flipy;
		u8 pri = 0, ext = 0;
		u8 sizex, sizey;
		u32 color, code;
		s32 x, y;
		if (m_alt_format)
		{
			if (BIT(spriteram[i], 15))
				continue;

			flipy = BIT(spriteram[i + 0], 14);
			flipx = BIT(spriteram[i + 0], 13);
			pri   = BIT(spriteram[i + 0], 11);
			sizex = BIT(spriteram[i + 0],  7,  3) + 1;
			sizey = BIT(spriteram[i + 0],  4,  3) + 1;
			color = BIT(spriteram[i + 0],  0,  4);
			code  = BIT(spriteram[i + 1],  0, 13);
			x     = BIT(spriteram[i + 2],  0,  9);
			y     = BIT(spriteram[i + 3],  0,  9);

			if (x >= 0x180) x -= 0x200;
			if (y >= 0x180) y -= 0x200;
		}
		else
		{
			if (BIT(~spriteram[i], 15))
				continue;

			flipx = BIT(spriteram[i + 0], 14);
			flipy = BIT(spriteram[i + 0], 13);
			sizex = BIT(spriteram[i + 0], 10,  3) + 1;
			sizey = BIT(spriteram[i + 0],  7,  3) + 1;
			ext   = BIT(spriteram[i + 0],  6);
			color = BIT(spriteram[i + 0],  0,  6);
			pri   = BIT(spriteram[i + 1], 14,  2);
			code  = BIT(spriteram[i + 1],  0, 14);
			x = get_coordinate(spriteram[i + 2]);
			y = get_coordinate(spriteram[i + 3]);
		}

		x += m_xoffset;
		y += m_yoffset;

		u32 pri_mask = 0;

		if (!m_pri_cb.isnull())
			pri_mask = m_pri_cb(pri, ext);

		if (!m_gfxbank_cb.isnull())
			code = m_gfxbank_cb(code, ext, BIT(spriteram[i + 3], 15));

		for (int ax = 0; ax < sizex; ax++)
		{
			for (int ay = 0; ay < sizey; ay++)
			{
				gfx(0)->prio_transpen(bitmap, cliprect,
					code++,
					color,
					flipx, flipy,
					flipx ? (x + 16 * (sizex - ax - 1)) : (x + 16 * ax),
					flipy ? (y + 16 * (sizey - ay - 1)) : (y + 16 * ay),
					screen.priority(), pri_mask, 15);
			}
		}
	}
}

void sei0210_device::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle cliprect, u16* spriteram, u16 size)
{
	draw(screen, bitmap, cliprect, spriteram, size);
}

void sei0210_device::draw_sprites(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle cliprect, u16* spriteram, u16 size)
{
	draw(screen, bitmap, cliprect, spriteram, size);
}
