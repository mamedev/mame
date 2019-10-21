// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Block Out

***************************************************************************/

#include "emu.h"
#include "includes/blockout.h"


rgb_t blockout_state::blockout_xBGR_444(u32 raw)
{
	/* red component */
	u8 bit0 = (raw >> 0) & 0x01;
	u8 bit1 = (raw >> 1) & 0x01;
	u8 bit2 = (raw >> 2) & 0x01;
	u8 bit3 = (raw >> 3) & 0x01;
	const u8 r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

	/* green component */
	bit0 = (raw >> 4) & 0x01;
	bit1 = (raw >> 5) & 0x01;
	bit2 = (raw >> 6) & 0x01;
	bit3 = (raw >> 7) & 0x01;
	const u8 g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

	/* blue component */
	bit0 = (raw >> 8) & 0x01;
	bit1 = (raw >> 9) & 0x01;
	bit2 = (raw >> 10) & 0x01;
	bit3 = (raw >> 11) & 0x01;
	const u8 b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

	return rgb_t(r, g, b);
}

void blockout_state::frontcolor_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_color);
	m_palette->set_pen_color(512, blockout_xBGR_444(m_color));
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/
void blockout_state::video_start()
{
	/* Allocate temporary bitmaps */
	m_tmpbitmap.allocate(512, 256);
	const rectangle clip(0, 511, 0, 255);
	m_tmpbitmap.fill(0x100, clip);
	save_item(NAME(m_tmpbitmap));
}

u8 blockout_state::videoram_r(offs_t offset)
{
	return m_videoram[offset];
}

void blockout_state::videoram_w(offs_t offset, u8 data)
{
	if (m_videoram[offset] != data)
	{
		m_videoram[offset] = data;
		const u16 x = offset & 0x1ff;
		const u16 y = (offset >> 9) & 0xff;
		u16 color;

		const u8 front = m_videoram[(y << 9) | x];
		const u8 back = m_videoram[0x20000 | (y << 9) | x];

		if (front)
			color = front;
		else
			color = back | 0x100;

		m_tmpbitmap.pix16(y, x) = color;
	}
}


u32 blockout_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	pen_t color = 512;

	copybitmap(bitmap, m_tmpbitmap, 0, 0, 0, 0, cliprect);

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			const u16 d = m_frontvideoram[((y & 0xff) << 6) + ((x & 0x1ff) >> 3)];
			if (d & (1 << (7 - (x & 7))))
				bitmap.pix16(y, x) = color;
		}
	}

	return 0;
}
