// license:BSD-3-Clause
// copyright-holders:Luca Elia
/*

    TAXAN KY-3211 and KY-10510 Sprite generator

    These chips are sprite generator, with ROZ capability.
    Tile size: 16x16, 4bpp or 8bpp, Can be composed to
    32 tiles for both horizontal and vertical independently.

    Later revision, KY-10510 has extended color and slightly
    different sprite RAM format.

    used at:
    - sigma/sigmab98.cpp
    - sigma/sammymdl.cpp

    TODO:
    - Verify vregs usage other than background color,
      and vtable usage

    -- Original docs from sigma/sigmab98.cpp:

    Sprites (Older chip: TAXAN KY-3211. Newer chip: KY-10510)

    Offset:     Bits:         Value:

    0           7654 ----     Color (High, newer chip only?)
                ---- 3210     Color
    1           7--- ----
                -6-- ----     256 Color Sprite (older chip)
                --5- ----
                ---4 ----     Flip X (older chip)
                ---- 3---     Flip Y (older chip) / 256 Color Sprite (newer chip)
                ---- -2--     Draw Sprite
                ---- --10     Priority (0 = Front .. 3 = Back)
    2                         Tile Code (High)
    3                         Tile Code (Low)
    4           7654 3---     Number of X Tiles - 1
                ---- -2--     Flip X (newer chip)
                ---- --10     X (High)
    5                         X (Low)
    6           7654 3---     Number of Y Tiles - 1
                ---- -2--     Flip Y (newer chip)
                ---- --10     Y (High)
    7                         Y (Low)
    8                         Destination Delta X, Scaled by Shrink Factor << 8 (High)
    9                         Destination Delta X, Scaled by Shrink Factor << 8 (Low)
    a                         Destination Delta Y, Scaled by Shrink Factor << 8 (High)
    b                         Destination Delta Y, Scaled by Shrink Factor << 8 (Low)
    c           7654 3---
                ---- -210     Source X (High)
    d                         Source X (Low)
    e           7654 3---
                ---- -210     Source Y (High)
    f                         Source Y (Low)

    Sprites rotation examples:
    logo in dashhero, pepsiman https://youtu.be/p3cbZ67m4lo?t=1m24s, tdoboon https://youtu.be/loPP3jt0Ob0


    Video Regs

    Offset:     Bits:         Value:

    01                        Screen Width / 2 - 1
    03
    05
    07
    09                        Screen Height - 1
    0b
    0d
    0f
    11
    13           76-- ----
                 --5- ----    VBlank?
                 ---4 3---
                 ---- -2--    Sprites Buffered?
                 ---- --10
    15
    17
    19
    1b                        Background Color (Low)
    1d                        Background Color (High)
    1f
    21

*/

#include "emu.h"
#include "ky3211_ky10510.h"

#include "multibyte.h"

#include "screen.h"

constexpr int integer_part(int x)
{
	//return x >> 16;
	return (x + 0x8000) >> 16;
}

GFXDECODE_START( ky3211_device::gfx_ky3211 )
	GFXDECODE_DEVICE(DEVICE_SELF, 0, gfx_16x16x4_packed_lsb, 0, 0x100/16  )
	GFXDECODE_DEVICE(DEVICE_SELF, 0, gfx_16x16x8_raw,        0, 0x100/256 )
GFXDECODE_END

// Larger palette
GFXDECODE_START( ky10510_device::gfx_ky10510 )
	GFXDECODE_DEVICE(DEVICE_SELF, 0, gfx_16x16x4_packed_lsb, 0, 0x1000/16 )
	GFXDECODE_DEVICE(DEVICE_SELF, 0, gfx_16x16x8_raw,        0, 0x1000/16 )
GFXDECODE_END

DEFINE_DEVICE_TYPE(KY3211,  ky3211_device,  "ky3211",  "TAXAN KY-3211 Sprites")
DEFINE_DEVICE_TYPE(KY10510, ky10510_device, "ky10510", "TAXAN KY-10510 Sprites")

ky3211_device::ky3211_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_gfx_interface(mconfig, *this)
	, m_vregs(*this, "vregs", 0x22, ENDIANNESS_LITTLE)
	, m_vtable(*this, "vtable", 0x80, ENDIANNESS_LITTLE)
	, m_sprite_bitmap(512, 512)
{
}

ky3211_device::ky3211_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ky3211_device(mconfig, KY3211, tag, owner, clock)
{
}

ky10510_device::ky10510_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ky3211_device(mconfig, KY10510, tag, owner, clock)
{
}

void ky3211_device::save_states()
{
	save_item(NAME(m_sprite_bitmap));
}

void ky3211_device::device_start()
{
	decode_gfx(gfx_ky3211);

	save_states();
}

void ky10510_device::device_start()
{
	decode_gfx(gfx_ky10510);
	gfx(1)->set_granularity(16);

	save_states();
}

bool ky3211_device::get_attr(const u8 *src, u32 &color, u8 &gfx, bool &flipx, bool &flipy)
{
	if (BIT(~src[0x01], 2))
		return false;

	color = src[0x00] & 0xf;
	gfx   = BIT(src[0x01], 6);
	flipx = BIT(src[0x01], 4);
	flipy = BIT(src[0x01], 3);

	return true;
}

bool ky10510_device::get_attr(const u8 *src, u32 &color, u8 &gfx, bool &flipx, bool &flipy)
{
	if ((src[0x01] & 0x0c) == 0)
		return false;

	color = src[0x00] & 0xff;
	gfx   = BIT(src[0x01], 3);
	flipx = BIT(src[0x04], 2);
	flipy = BIT(src[0x06], 2);

	return true;
}

void ky3211_device::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, const u8 *spriteram, int pri_mask)
{
	const u8 *end = spriteram - 0x10;
	const u8 *src = end + 0x1000;

	for (; src != end; src -= 0x10)
	{
		u32 color;
		u8 gfx;
		bool flipx, flipy;
		if (!get_attr(src, color, gfx, flipx, flipy))
			continue;

		if (((1 << (src[0x01] & 0x03)) & pri_mask) == 0)
			continue;

		int code     = get_u16be(&src[0x02]);

		const int nx = ((src[0x04] & 0xf8) >> 3) + 1;
		int dstx     = get_u16be(&src[0x04]) & 0x3ff;

		const int ny = ((src[0x06] & 0xf8) >> 3) + 1;
		int dsty     = get_u16be(&src[0x06]) & 0x3ff;

		int dstdx    = get_u16be(&src[0x08]);   // 0x100 = no zoom, 0x200 = 50% zoom
		int dstdy    = get_u16be(&src[0x0a]);   // ""

		int srcx     = get_u16be(&src[0x0c]);
		int srcy     = get_u16be(&src[0x0e]);

		// Sign extend the position
		dstx = util::sext(dstx, 10); // or 11?
		dsty = util::sext(dsty, 10);

		// Flipping
		int x0, x1, dx;
		int y0, y1, dy;

		if (flipx) { x0 = nx - 1; x1 = -1; dx = -1; }
		else       { x0 = 0;      x1 = nx; dx = +1; }

		if (flipy) { y0 = ny - 1; y1 = -1; dy = -1; }
		else       { y0 = 0;      y1 = ny; dy = +1; }

		// Draw the sprite directly to screen if no zoom/rotation/offset is required
		if (dstdx == 0x100 && !dstdy && !srcx && !srcy)
		{
			for (int y = y0; y != y1; y += dy)
			{
				for (int x = x0; x != x1; x += dx)
				{
					this->gfx(gfx)->transpen(bitmap, cliprect,
						code++, color,
						flipx, flipy,
						dstx + x * 16, dsty + y * 16, 0);
				}
			}

			continue;
		}

		// First draw the sprite in a buffer without zoom/rotation/offset, nor transparency
		rectangle sprite_cliprect(0, nx * 16 - 1, 0, ny * 16 - 1);
		for (int y = y0; y != y1; y += dy)
		{
			for (int x = x0; x != x1; x += dx)
			{
				this->gfx(gfx)->opaque(m_sprite_bitmap, sprite_cliprect,
					code++, color,
					flipx, flipy,
					x * 16, y * 16);
			}
		}

		// Sign extend the transformation values
		dstdx = util::sext(dstdx, 16);
		dstdy = util::sext(dstdy, 16);
		srcx  = util::sext(srcx, 16);
		srcy  = util::sext(srcy, 16);
		dstdy = -dstdy;

		// Use fixed point values (16.16), for accuracy
		dstx <<= 16;
		dsty <<= 16;

		// Source delta (equal for x and y)
		int z = int(sqrt(dstdx * dstdx + dstdy * dstdy) + 0.5);   // dest delta vector is scaled by the source delta!?
		if (!z)
			z = 0x100;
		int srcdzz = z << 8;

		// Destination x and y deltas
		int dstdxx = (dstdx << 16) / z; // dest x delta for source x increments
		int dstdyx = (dstdy << 16) / z; // dest y delta for source x increments

		int dstdxy = -dstdyx;           // dest x delta for source y increments (orthogonal to the above vector)
		int dstdyy = dstdxx;            // dest y delta for source y increments

		// Transform the source offset in a destination offset (negate, scale and rotate it)
		srcx = (-srcx << 8) / z;
		srcy = (-srcy << 8) / z;

		dstx += srcx * dstdxx;
		dsty += srcx * dstdyx;

		dstx += srcy * dstdxy;
		dsty += srcy * dstdyy;

		// Supersampling (2x2) to avoid gaps in the destination
		srcdzz /= 2;
		dstdxx /= 2;
		dstdyx /= 2;
		dstdxy /= 2;
		dstdyy /= 2;

		// Transform the source image while drawing to the screen
		u16 const *const src = &m_sprite_bitmap.pix(0);
		u16 *const dst = &bitmap.pix(0);

		const int src_rowpixels = m_sprite_bitmap.rowpixels();
		const int dst_rowpixels = bitmap.rowpixels();

		const u16 penmask = gfx ? 0xff : 0x0f;

		// Scan source image top to bottom
		srcy = 0;
		for (;;)
		{
			const int dstx_prev = dstx;
			const int dsty_prev = dsty;

			const int fy = integer_part(srcy);
			if (fy > sprite_cliprect.max_y)
				break;
			if (fy >= sprite_cliprect.min_y)
			{
				// left to right
				srcx = 0;
				for (;;)
				{
					int fx = integer_part(srcx);
					if (fx > sprite_cliprect.max_x)
						break;
					if (fx >= sprite_cliprect.min_x)
					{
						const int px = integer_part(dstx);
						const int py = integer_part(dsty);

						if (px >= cliprect.min_x && px <= cliprect.max_x && py >= cliprect.min_y && py <= cliprect.max_y)
						{
							const u16 pen = src[fy * src_rowpixels + fx];
							if (pen & penmask)
								dst[py * dst_rowpixels + px] = pen;
						}
					}

					// increment source x and dest x,y
					srcx += srcdzz;

					dstx += dstdxx;
					dsty += dstdyx;
				}
			}

			// increment source y and dest x,y
			srcy += srcdzz;

			dstx = dstx_prev;
			dsty = dsty_prev;
			dstx += dstdxy;
			dsty += dstdyy;
		}
	}
}

void ky3211_device::vregs_w(offs_t offset, u8 data)
{
	m_vregs[offset] = data;

	switch (offset)
	{
		case 0x1b:  // background color
		case 0x1d:
		{
			const int x = (m_vregs[0x1d] << 8) + m_vregs[0x1b];
			const int r = (x >> 10) & 0x1f;
			const int g = (x >>  5) & 0x1f;
			const int b = (x >>  0) & 0x1f;
			palette().set_pen_color(0x1000, pal5bit(r), pal5bit(g), pal5bit(b));
			break;
		}
//      default:
//          logerror("%s: unknown video reg written: %02x = %02x\n", machine().describe_context(), offset, data);
	}
}

u8 ky3211_device::vregs_r(offs_t offset)
{
	switch (offset)
	{
		default:
			if (!machine().side_effects_disabled())
				logerror("%s: unknown video reg read: %02x\n", machine().describe_context(), offset);
			return m_vregs[offset];
	}
}

void ky3211_device::vtable_w(offs_t offset, u8 data)
{
	m_vtable[offset] = data;
}

u8 ky3211_device::vtable_r(offs_t offset)
{
	return m_vtable[offset];
}
