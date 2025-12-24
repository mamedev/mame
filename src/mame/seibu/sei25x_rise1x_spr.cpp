// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, Angelo Salese, David Haywood, Tomasz Slanina, Nicola Salmoria, Ville Linde, hap
/*
    Seibu Kaihatsu SEI251/SEI252/RISE10/RISE11 Sprite generator emulation

    Used by Seibu Kaihatsu at 1993 onward, these are has encryption function.

    SEI25x and RISE1x have compatible sprite format, but RISE1x has
    expanded per-line sprite limit and different encryption method.
    Other chip differences are still unknown.

    Used in:
        seibu/raiden2.cpp
		seibu/r2dx_v33.cpp
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


DEFINE_DEVICE_TYPE(SEI25X_RISE1X, sei25x_rise1x_device, "sei25x_rise1x", "Seibu Kaihatsu SEI251/SEI252/RISE10/RISE11 sprite generator")

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
template <typename T, typename U>
inline void sei25x_rise1x_device::draw_sprites(const u16 *spriteram, int start, int end, int inc, T &&set_pri_col, U &&plot)
{
	for (int i = start; i != end; i += inc)
	{
		u32 code         = spriteram[i + 1];
		// TODO: needed for spi and feversoc?
		if ((code % gfx(0)->elements()) == 0)
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

		set_pri_col(pri, color);

		if (!m_gfxbank_cb.isnull())
			code = m_gfxbank_cb(code, ext);

		for (int ax = 0; ax < sizex; ax++)
		{
			const int sx = flipx ? (x + 16 * (sizex - ax - 1)) : (x + 16 * ax);
			for (int ay = 0; ay < sizey; ay++)
			{
				const int sy = flipy ? (y + 16 * (sizey - ay - 1)) : (y + 16 * ay);
				plot(code++, color, flipx, flipy, sx, sy);
			}
		}
	}
}

void sei25x_rise1x_device::draw(bitmap_ind16 &bitmap, const rectangle cliprect, const u16 *spriteram, u16 size)
{
	draw_sprites(
			spriteram, (size / 2) - 4, -4, -4,
			[] (u8 pri, u32 &color) { },
			[this, &bitmap, &cliprect] (u32 code, u32 color, bool flipx, bool flipy, s32 sx, s32 sy)
			{
				gfx(0)->transpen(bitmap, cliprect,
						code,
						color,
						flipx, flipy,
						sx, sy,
						m_transpen);
			});
}

void sei25x_rise1x_device::draw(bitmap_rgb32 &bitmap, const rectangle cliprect, const u16 *spriteram, u16 size)
{
	draw_sprites(
			spriteram, (size / 2) - 4, -4, -4,
			[] (u8 pri, u32 &color) { },
			[this, &bitmap, &cliprect] (u32 code, u32 color, bool flipx, bool flipy, s32 sx, s32 sy)
			{
				gfx(0)->transpen(bitmap, cliprect,
						code,
						color,
						flipx, flipy,
						sx, sy,
						m_transpen);
			});
}

void sei25x_rise1x_device::draw_prio(screen_device &screen, bitmap_ind16 &bitmap, const rectangle cliprect, const u16 *spriteram, u16 size)
{
	assert(!m_pri_cb.isnull());

	u32 pri_mask;
	draw_sprites(
			spriteram, 0, size / 2, 4,
			[this, &pri_mask] (u8 pri, u32 &color) { pri_mask = m_pri_cb(pri); },
			[this, &screen, &bitmap, &cliprect, &pri_mask] (u32 code, u32 color, bool flipx, bool flipy, s32 sx, s32 sy)
			{
				gfx(0)->prio_transpen(bitmap, cliprect,
						code,
						color,
						flipx, flipy,
						sx, sy,
						screen.priority(), pri_mask, m_transpen);
			});
}

void sei25x_rise1x_device::draw_prio(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle cliprect, const u16 *spriteram, u16 size)
{
	assert(!m_pri_cb.isnull());

	u32 pri_mask;
	draw_sprites(
			spriteram, 0, size / 2, 4,
			[this, &pri_mask] (u8 pri, u32 &color) { pri_mask = m_pri_cb(pri); },
			[this, &screen, &bitmap, &cliprect, &pri_mask] (u32 code, u32 color, bool flipx, bool flipy, s32 sx, s32 sy)
			{
				gfx(0)->prio_transpen(bitmap, cliprect,
						code,
						color,
						flipx, flipy,
						sx, sy,
						screen.priority(), pri_mask, m_transpen);
			});
}

void sei25x_rise1x_device::draw_raw(bitmap_ind16 &bitmap, const rectangle cliprect, const u16 *spriteram, u16 size)
{
	// for manual mixing
	bitmap.fill(0xffff, cliprect);
	draw_sprites(
			spriteram, (size / 2) - 4, -4, -4,
			[this] (u8 pri, u32 &color) { color = (color << m_pix_raw_shift) | (u32(pri) << m_pri_raw_shift); },
			[this, &bitmap, &cliprect] (u32 code, u32 color, bool flipx, bool flipy, s32 sx, s32 sy)
			{
				gfx(0)->transpen_raw(bitmap, cliprect,
						code,
						color,
						flipx, flipy,
						sx, sy,
						m_transpen);
			});
}
