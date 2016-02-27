// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "emu.h"
#include "kan_pand.h"
#include "includes/galpanic.h"


void galpanic_state::video_start()
{
	m_screen->register_screen_bitmap(m_bitmap);

	save_item(NAME(m_bitmap));
}

PALETTE_INIT_MEMBER(galpanic_state,galpanic)
{
	/* first 1024 colors are dynamic */

	/* initialize 555 RGB lookup */
	for (int i = 0;i < 32768;i++)
		palette.set_pen_color(i+1024,pal5bit(i >> 5),pal5bit(i >> 10),pal5bit(i >> 0));
}

WRITE16_MEMBER(galpanic_state::bgvideoram_w)
{
	data = COMBINE_DATA(&m_bgvideoram[offset]);

	int sy = offset / 256;
	int sx = offset % 256;

	m_bitmap.pix16(sy, sx) = 1024 + (data >> 1);
}

void galpanic_state::draw_fgbitmap(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = 0;offs < m_fgvideoram.bytes()/2;offs++)
	{
		int sx = offs % 256;
		int sy = offs / 256;
		int color = m_fgvideoram[offs];
		if (color)
			bitmap.pix16(sy, sx) = color;
	}
}

UINT32 galpanic_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* copy the temporary bitmap to the screen */
	copybitmap(bitmap,m_bitmap,0,0,0,0,cliprect);

	draw_fgbitmap(bitmap, cliprect);

	m_pandora->update(bitmap, cliprect);

	return 0;
}
