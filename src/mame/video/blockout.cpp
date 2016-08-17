// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Block Out

***************************************************************************/

#include "emu.h"
#include "includes/blockout.h"


void blockout_state::setcolor( int color, int rgb )
{
	int bit0, bit1, bit2, bit3;
	int r, g, b;


	/* red component */
	bit0 = (rgb >> 0) & 0x01;
	bit1 = (rgb >> 1) & 0x01;
	bit2 = (rgb >> 2) & 0x01;
	bit3 = (rgb >> 3) & 0x01;
	r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

	/* green component */
	bit0 = (rgb >> 4) & 0x01;
	bit1 = (rgb >> 5) & 0x01;
	bit2 = (rgb >> 6) & 0x01;
	bit3 = (rgb >> 7) & 0x01;
	g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

	/* blue component */
	bit0 = (rgb >> 8) & 0x01;
	bit1 = (rgb >> 9) & 0x01;
	bit2 = (rgb >> 10) & 0x01;
	bit3 = (rgb >> 11) & 0x01;
	b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

	m_palette->set_pen_color(color, rgb_t(r,g,b));
}

WRITE16_MEMBER(blockout_state::blockout_paletteram_w)
{
	COMBINE_DATA(&m_paletteram[offset]);
	setcolor(offset, m_paletteram[offset]);
}

WRITE16_MEMBER(blockout_state::blockout_frontcolor_w)
{
	COMBINE_DATA(&m_color);
	setcolor(512, m_color);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/
void blockout_state::video_start()
{
	/* Allocate temporary bitmaps */
	m_screen->register_screen_bitmap(m_tmpbitmap);
	save_item(NAME(m_tmpbitmap));
}

void blockout_state::update_pixels( int x, int y )
{
	UINT16 front, back;
	int color;
	const rectangle &visarea = m_screen->visible_area();

	if (!visarea.contains(x, y))
		return;

	front = m_videoram[y * 256 + x / 2];
	back = m_videoram[0x10000 + y * 256 + x / 2];

	if (front >> 8)
		color = front >> 8;
	else
		color = (back >> 8) + 256;

	m_tmpbitmap.pix16(y, x) = color;

	if (front & 0xff)
		color = front & 0xff;
	else
		color = (back & 0xff) + 256;

	m_tmpbitmap.pix16(y, x + 1) = color;
}



WRITE16_MEMBER(blockout_state::blockout_videoram_w)
{
	COMBINE_DATA(&m_videoram[offset]);
	update_pixels((offset % 256) * 2, (offset / 256) % 256);
}



UINT32 blockout_state::screen_update_blockout(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x, y;
	pen_t color = 512;

	copybitmap(bitmap, m_tmpbitmap, 0, 0, 0, 0, cliprect);

	for (y = 0; y < 256; y++)
	{
		for (x = 0; x < 320; x += 8)
		{
			int d = m_frontvideoram[y * 64 + (x / 8)];

			if (d)
			{
				if (d & 0x80) bitmap.pix16(y, x + 0) = color;
				if (d & 0x40) bitmap.pix16(y, x + 1) = color;
				if (d & 0x20) bitmap.pix16(y, x + 2) = color;
				if (d & 0x10) bitmap.pix16(y, x + 3) = color;
				if (d & 0x08) bitmap.pix16(y, x + 4) = color;
				if (d & 0x04) bitmap.pix16(y, x + 5) = color;
				if (d & 0x02) bitmap.pix16(y, x + 6) = color;
				if (d & 0x01) bitmap.pix16(y, x + 7) = color;
			}
		}
	}

	return 0;
}
