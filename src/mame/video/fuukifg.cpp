// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood
/*
*/

#include "emu.h"
#include "fuukifg.h"

const device_type FUUKI_VIDEO = &device_creator<fuukivid_device>;

fuukivid_device::fuukivid_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, FUUKI_VIDEO, "Fuuki Video", tag, owner, clock, "fuukivid", __FILE__),
		device_video_interface(mconfig, *this),
		m_gfxdecode(*this)
{
}

void fuukivid_device::static_set_gfxdecode_tag(device_t &device, const char *tag)
{
	downcast<fuukivid_device &>(device).m_gfxdecode.set_tag(tag);
}


void fuukivid_device::device_start()
{
	m_sprram = make_unique_clear<UINT16[]>(0x2000 / 2);

	// fuukifg3 clearly has buffered ram, it is unclear if fuukifg2 has
	// it is likely these render to a framebuffer as the tile bank (which is probably external hw) also needs to be banked
	// suggesting that the sprites are rendered earlier, then displayed from a buffer

	m_sprram_old = make_unique_clear<UINT16[]>(0x2000 / 2);
	m_sprram_old2 = make_unique_clear<UINT16[]>(0x2000 / 2);

	save_pointer(NAME(m_sprram.get()), 0x2000 / 2);
	save_pointer(NAME(m_sprram_old.get()), 0x2000 / 2);
	save_pointer(NAME(m_sprram_old2.get()), 0x2000 / 2);

}

void fuukivid_device::device_reset()
{
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

void fuukivid_device::draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int flip_screen , UINT32* tilebank)
{
	// as we're likely framebuffered (sprites are delayed by 2-3 frames, at least on FG3, and doing rasters on sprites causes glitches) we
	// only draw the sprites when MAME wants to draw the final screen line.  Ideally we should framebuffer them instead.
	if (cliprect.max_y != m_screen->visible_area().max_y)
		return;

	rectangle spriteclip = m_screen->visible_area();

	int offs;
	gfx_element *gfx = m_gfxdecode->gfx(0);
	bitmap_ind8 &priority_bitmap = screen.priority();
	const rectangle &visarea = screen.visible_area();

	UINT16 *spriteram16 = m_sprram.get();

	if (tilebank) spriteram16 = m_sprram_old2.get(); // so that FG3 uses the buffered RAM

	int max_x = visarea.max_x + 1;
	int max_y = visarea.max_y + 1;

	/* Draw them backwards, for pdrawgfx */
	for ( offs = (0x2000 - 8) / 2; offs >=0; offs -= 8 / 2 )
	{
		int x, y, xstart, ystart, xend, yend, xinc, yinc;
		int xnum, ynum, xzoom, yzoom, flipx, flipy;
		int pri_mask;

		int sx = spriteram16[offs + 0];
		int sy = spriteram16[offs + 1];
		int attr = spriteram16[offs + 2];
		int code = spriteram16[offs + 3];

		if (tilebank)
		{
			int bank = (code & 0xc000) >> 14;
			int bank_lookedup;

			bank_lookedup = ((tilebank[1] & 0xffff0000) >> (16 + bank * 4)) & 0xf;
			code &= 0x3fff;
			code += bank_lookedup * 0x4000;
		}

		if (sx & 0x400)
			continue;

		flipx = sx & 0x0800;
		flipy = sy & 0x0800;

		xnum = ((sx >> 12) & 0xf) + 1;
		ynum = ((sy >> 12) & 0xf) + 1;

		xzoom = 16 * 8 - (8 * ((attr >> 12) & 0xf)) / 2;
		yzoom = 16 * 8 - (8 * ((attr >>  8) & 0xf)) / 2;

		switch ((attr >> 6) & 3)
		{
			case 3: pri_mask = 0xf0 | 0xcc | 0xaa;  break;  // behind all layers
			case 2: pri_mask = 0xf0 | 0xcc;         break;  // behind fg + middle layer
			case 1: pri_mask = 0xf0;                break;  // behind fg layer
			case 0:
			default:    pri_mask = 0;                       // above all
		}

		sx = (sx & 0x1ff) - (sx & 0x200);
		sy = (sy & 0x1ff) - (sy & 0x200);

		if (flip_screen)
		{
			flipx = !flipx;     sx = max_x - sx - xnum * 16;
			flipy = !flipy;     sy = max_y - sy - ynum * 16;
		}

		if (flipx)  { xstart = xnum-1;  xend = -1;    xinc = -1; }
		else        { xstart = 0;       xend = xnum;  xinc = +1; }

		if (flipy)  { ystart = ynum-1;  yend = -1;    yinc = -1; }
		else        { ystart = 0;       yend = ynum;  yinc = +1; }

		for (y = ystart; y != yend; y += yinc)
		{
			for (x = xstart; x != xend; x += xinc)
			{
				if (xzoom == (16*8) && yzoom == (16*8))
					gfx->prio_transpen(bitmap,spriteclip,
									code++,
									attr & 0x3f,
									flipx, flipy,
									sx + x * 16, sy + y * 16,
									priority_bitmap,
									pri_mask,15 );
				else
					gfx->prio_zoom_transpen(bitmap,spriteclip,
									code++,
									attr & 0x3f,
									flipx, flipy,
									sx + (x * xzoom) / 8, sy + (y * yzoom) / 8,
									(0x10000/0x10/8) * (xzoom + 8),(0x10000/0x10/8) * (yzoom + 8),  priority_bitmap,// nearest greater integer value to avoid holes
									pri_mask,15 );
			}
		}
	}
}

void fuukivid_device::buffer_sprites(void)
{
	memcpy(m_sprram_old2.get(), m_sprram_old.get(), 0x2000);
	memcpy(m_sprram_old.get(), m_sprram.get(), 0x2000);
}
