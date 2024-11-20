// license:BSD-3-Clause
// copyright-holders: Luca Elia, David Haywood

/*
	Fuuki Sprite hardware

	Used by:
	fuukifg2.cpp
	fuukifg3.cpp (with sprite tile bankswitching, triple buffered)
*/


#include "emu.h"
#include "fuukispr.h"
#include "screen.h"

DEFINE_DEVICE_TYPE(FUUKI_SPRITE, fuukispr_device, "fuukispr", "Fuuki Sprite hardware")

fuukispr_device::fuukispr_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, FUUKI_SPRITE, tag, owner, clock)
	, device_gfx_interface(mconfig, *this)
	, device_video_interface(mconfig, *this)
	, m_tile_cb(*this)
	, m_colpri_cb(*this)
	, m_colbase(0)
	, m_colnum(0x100)
{
}

GFXDECODE_MEMBER(fuukispr_device::gfxinfo)
	GFXDECODE_DEVICE(DEVICE_SELF, 0, gfx_16x16x4_packed_msb, 0, 16)
GFXDECODE_END

void fuukispr_device::device_start()
{
	decode_gfx(gfxinfo);
	gfx(0)->set_colorbase(m_colbase);
	gfx(0)->set_colors(m_colnum);

	m_tile_cb.resolve();
	m_colpri_cb.resolve();
}


/***************************************************************************


                                Sprites Drawing

    Offset:     Bits:                   Value:

        0.w     fedc ---- ---- ----     Number Of Tiles Along X - 1
                ---- b--- ---- ----     Flip X
                ---- -a-- ---- ----     1 = Don't Draw This Sprite
                ---- --98 7654 3210     X (Signed)

        2.w     fedc ---- ---- ----     Number Of Tiles Along Y - 1
                ---- b--- ---- ----     Flip Y
                ---- -a-- ---- ----
                ---- --98 7654 3210     Y (Signed)

        4.w     fedc ---- ---- ----     Zoom X ($0 = Full Size, $F = Half Size)
                ---- ba98 ---- ----     Zoom Y ""
                ---- ---- 76-- ----     Priority
                ---- ---- --54 3210     Color

        6.w                             Tile Code

        for FG3 hardware

        6.w     fe-- ---- ---- ----     Tile Bank
                --dc ba98 7654 3210     Tile Code

***************************************************************************/

void fuukispr_device::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, bool flip_screen, u16 *spriteram, u32 size)
{
	// as we're likely framebuffered (sprites are delayed by 2-3 frames, at least on FG3, and doing rasters on sprites causes glitches) we
	// only draw the sprites when MAME wants to draw the final screen line.  Ideally we should framebuffer them instead.
	if (cliprect.max_y != screen.visible_area().max_y)
		return;

	const bool tilebank = !m_tile_cb.isnull();
	const bool priority = !m_colpri_cb.isnull();
	const rectangle spriteclip = screen.visible_area();

	const int max_x = spriteclip.max_x + 1;
	const int max_y = spriteclip.max_y + 1;

	int start, end, inc;
	if (priority)             { start = size - 4; end =   -4; inc = -4; }
	else                      { start =        0; end = size; inc = +4; }

	// Draw them backwards, for pdrawgfx
	for (int offs = start; offs != end; offs += inc)
	{
		const u16 data0 = spriteram[offs + 0];
		if (BIT(data0, 10))
			continue;

		const u16 data1 = spriteram[offs + 1];
		const u16 attr = spriteram[offs + 2];
		u32 code = spriteram[offs + 3];
		if (tilebank)
			m_tile_cb(code);

		bool flipx = BIT(data0, 11);
		bool flipy = BIT(data1, 11);

		const int xnum = ((data0 >> 12) & 0xf) + 1;
		const int ynum = ((data1 >> 12) & 0xf) + 1;

		const int xzoom = 16 * 8 - (8 * ((attr >> 12) & 0xf)) / 2;
		const int yzoom = 16 * 8 - (8 * ((attr >>  8) & 0xf)) / 2;

		u32 pri_mask = 0;
		u32 colour = attr & 0xff;
		if (priority)
			m_colpri_cb(colour, pri_mask);

		int sx = (data0 & 0x1ff) - (data0 & 0x200);
		int sy = (data1 & 0x1ff) - (data1 & 0x200);

		if (flip_screen)
		{
			flipx = !flipx;     sx = max_x - sx - xnum * 16;
			flipy = !flipy;     sy = max_y - sy - ynum * 16;
		}

		int xstart, ystart, xend, yend, xinc, yinc;
		if (flipx)  { xstart = xnum-1;  xend = -1;    xinc = -1; }
		else        { xstart = 0;       xend = xnum;  xinc = +1; }

		if (flipy)  { ystart = ynum-1;  yend = -1;    yinc = -1; }
		else        { ystart = 0;       yend = ynum;  yinc = +1; }

		for (int y = ystart; y != yend; y += yinc)
		{
			for (int x = xstart; x != xend; x += xinc)
			{
				if (priority)
				{
					if (xzoom == (16*8) && yzoom == (16*8))
						gfx(0)->prio_transpen(bitmap, spriteclip,
										code++,
										colour,
										flipx, flipy,
										sx + x * 16, sy + y * 16,
										screen.priority(), pri_mask, 15);
					else
						gfx(0)->prio_zoom_transpen(bitmap, spriteclip,
										code++,
										colour,
										flipx, flipy,
										sx + (x * xzoom) / 8, sy + (y * yzoom) / 8,
										(0x10000/0x10/8) * (xzoom + 8), (0x10000/0x10/8) * (yzoom + 8),// nearest greater integer value to avoid holes
										screen.priority(), pri_mask, 15);
				}
				else
				{
					if (xzoom == (16*8) && yzoom == (16*8))
						gfx(0)->transpen(bitmap, spriteclip,
										code++,
										colour,
										flipx, flipy,
										sx + x * 16, sy + y * 16,
										15);
					else
						gfx(0)->zoom_transpen(bitmap, spriteclip,
										code++,
										colour,
										flipx, flipy,
										sx + (x * xzoom) / 8, sy + (y * yzoom) / 8,
										(0x10000/0x10/8) * (xzoom + 8), (0x10000/0x10/8) * (yzoom + 8),// nearest greater integer value to avoid holes
										15);
				}
			}
		}
	}
}
