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

void galpanic_state::galpanic_palette(palette_device &palette) const
{
	// first 1024 colors are dynamic

	// initialize 555 GRB lookup
	for (int i = 0; i < 32768; i++)
		palette.set_pen_color(i + 1024, pal5bit(i >> 5), pal5bit(i >> 10), pal5bit(i >> 0));
}

void galpanic_state::bgvideoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	data = COMBINE_DATA(&m_bgvideoram[offset]);

	int sy = offset / 256;
	int sx = offset % 256;

	m_bitmap.pix(sy, sx) = 1024 + (data >> 1);
}

void galpanic_state::draw_fgbitmap(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = 0;offs < m_fgvideoram.bytes()/2;offs++)
	{
		int const sx = offs % 256;
		int const sy = offs / 256;
		int const color = m_fgvideoram[offs];
		if (color)
			bitmap.pix(sy, sx) = color;
	}
}

uint32_t galpanic_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* copy the temporary bitmap to the screen */
	copybitmap(bitmap,m_bitmap,0,0,0,0,cliprect);

	draw_fgbitmap(bitmap, cliprect);

	m_pandora->update(bitmap, cliprect);

	return 0;
}
