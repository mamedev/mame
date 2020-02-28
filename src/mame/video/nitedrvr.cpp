// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/***************************************************************************

    Atari Night Driver hardware

***************************************************************************/

#include "emu.h"
#include "includes/nitedrvr.h"


void nitedrvr_state::draw_box(bitmap_ind16 &bitmap, const rectangle &cliprect, int bx, int by, int ex, int ey)
{
	for (int y = by; y < ey; y++)
	{
		for (int x = bx; x < ex; x++)
			if (cliprect.contains(x, y))
				bitmap.pix16(y, x) = 1;
	}
}

void nitedrvr_state::draw_roadway(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int roadway = 0; roadway < 16; roadway++)
	{
		int bx = m_hvc[roadway];
		int by = m_hvc[roadway + 16];
		int ex = bx + ((m_hvc[roadway + 32] & 0xf0) >> 4);
		int ey = by + (16 - (m_hvc[roadway + 32] & 0x0f));

		draw_box(bitmap, cliprect, bx, by, ex, ey);
	}
}

void nitedrvr_state::draw_tiles(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// draw tiles manually, note that tile rows are ignored on V&8, V&64, V&128
	for (int offs = 0; offs < 0x80; offs++)
	{
		int code = m_videoram[offs];
		int sx = (offs & 0x1f) * 8;
		int sy = (offs >> 5) * 2 * 8;

		m_gfxdecode->gfx(0)->opaque(bitmap, cliprect, code, 0, 0, 0, sx, sy);
	}
}

uint32_t nitedrvr_state::screen_update_nitedrvr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	draw_tiles(bitmap, cliprect);
	draw_roadway(bitmap, cliprect);

	return 0;
}
