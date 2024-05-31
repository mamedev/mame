// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, Angelo Salese, David Haywood, Tomasz Slanina, Nicola Salmoria, Ville Linde, hap
/*
    Seibu Kaihatsu SEI251/SEI252/RISE10/RISE11 Sprite generator emulation

    Used by Seibu Kaihatsu at 1993 onward, these are has encryption function.

    SEI25x and RISE1x has compatible sprite format, but RISE1x has
	expanded per-line sprite limit and different encryption method.
    Other chip differences are still unknown.

    Used in:
        seibu/raiden2.cpp
        seibu/seibuspi.cpp
        seibu/seibucats.cpp
        seibu/feversoc.cpp

    TODO:
    - flip screen support
    - Wraparound in raiden2/xsedae is correct?
    - Encryption
*/

#include "emu.h"
#include "sei25x_rise1x_spr.h"
#include "screen.h"


DEFINE_DEVICE_TYPE(SEI25X_RISE1X, sei25x_rise1x_device, "sei25x_rise1x", "Seibu Kaihatsu SEI251/SEI252/RISE10/RISE11 Sprite generator")

sei25x_rise1x_device::sei25x_rise1x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, device_gfx_interface(mconfig, *this)
	, m_pri_cb(*this)
	, m_gfxbank_cb(*this)
	, m_xoffset(0)
	, m_yoffset(0)
	, m_transpen(0)
	, m_pix_raw_shift(4)
{
}

sei25x_rise1x_device::sei25x_rise1x_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: sei25x_rise1x_device(mconfig, SEI25X_RISE1X, tag, owner, clock)
{
}

void sei25x_rise1x_device::device_start()
{
	m_pri_cb.resolve();
	m_gfxbank_cb.resolve();
}

void sei25x_rise1x_device::device_reset()
{
}

void sei25x_rise1x_device::alloc_sprite_bitmap()
{
	screen().register_screen_bitmap(m_sprite_bitmap);
}

/*

===============================================================

Common sprite format (8 byte per sprites)

Offset Bit                 Description
       fedc ba98 7654 3210
00     x--- ---- ---- ---- Flip Y
       -xxx ---- ---- ---- Sprite height
       ---- x--- ---- ---- Flip X
       ---- -xxx ---- ---- Sprite width
       ---- ---- xx-- ---- Priority
       ---- ---- --xx xxxx Color index
02     xxxx xxxx xxxx xxxx Tile index
04     ---x ---- ---- ---- (Optional) Extra tile bank bit
       ---- ---x xxxx xxxx X position
06     ---- ---x xxxx xxxx Y position

Unmarked bits are unused/unknown.

===============================================================

*/
template<class T>
void sei25x_rise1x_device::draw(screen_device &screen, T &bitmap, const rectangle cliprect, u16* spriteram, u16 size)
{
	if (m_sprite_bitmap.valid() && !m_pri_cb.isnull())
		fatalerror("m_sprite_bitmap && m_pri_cb is invalid\n");

	int start, end, inc;
	if (m_sprite_bitmap.valid())
		m_sprite_bitmap.fill(0xffff, cliprect);

	if (!m_pri_cb.isnull())
	{
		start = 0;
		end = (size / 2);
		inc = 4;
	}
	else
	{
		start = (size / 2) - 4;
		end = -4;
		inc = -4;
	}

	for (int i = start; i != end; i += inc)
	{
		u32 code         = spriteram[i + 1];
		// TODO: it needs at spi?
		if (code == 0)
			continue;

		const bool flipy = BIT(spriteram[i + 0], 15);
		const u8 sizey   = BIT(spriteram[i + 0], 12,  3) + 1;
		const bool flipx = BIT(spriteram[i + 0], 11);
		const u8 sizex   = BIT(spriteram[i + 0],  8,  3) + 1;
		const u8 pri     = BIT(spriteram[i + 0],  6,  2);
		u32 color        = BIT(spriteram[i + 0],  0,  6);
		const u8 ext     = BIT(spriteram[i + 2], 12);
		s32 x            = BIT(spriteram[i + 2],  0,  9);
		s32 y            = BIT(spriteram[i + 3],  0,  9);

		if (x >= 0x180) x -= 0x200;
		if (y >= 0x180) y -= 0x200;

		x += m_xoffset;
		y += m_yoffset;

		u32 pri_mask = 0;

		if (!m_pri_cb.isnull())
			pri_mask = m_pri_cb(pri);
		else if (m_sprite_bitmap.valid())
			color |= pri << (m_pri_raw_shift - m_pix_raw_shift); // for manual mixing

		if (!m_gfxbank_cb.isnull())
			code = m_gfxbank_cb(code, ext);

		for (int ax = 0; ax < sizex; ax++)
		{
			const int sx = flipx ? (x + 16 * (sizex - ax - 1)) : (x + 16 * ax);
			for (int ay = 0; ay < sizey; ay++)
			{
				const int sy = flipy ? (y + 16 * (sizey - ay - 1)) : (y + 16 * ay);
				if (!m_sprite_bitmap.valid())
				{
					if (!m_pri_cb.isnull())
					{
						gfx(0)->prio_transpen(bitmap, cliprect,
							code++,
							color,
							flipx, flipy,
							sx, sy,
							screen.priority(), pri_mask, m_transpen);
					}
					else
					{
						gfx(0)->transpen(bitmap, cliprect,
							code++,
							color,
							flipx, flipy,
							sx, sy,
							m_transpen);
					}
				}
				else
				{
					gfx(0)->transpen_raw(m_sprite_bitmap, cliprect,
						code++,
						color << m_pix_raw_shift,
						flipx, flipy,
						sx, sy,
						m_transpen);
				}
			}
		}
	}
}

void sei25x_rise1x_device::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle cliprect, u16* spriteram, u16 size)
{
	draw(screen, bitmap, cliprect, spriteram, size);
}

void sei25x_rise1x_device::draw_sprites(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle cliprect, u16* spriteram, u16 size)
{
	draw(screen, bitmap, cliprect, spriteram, size);
}
