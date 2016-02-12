// license:BSD-3-Clause
// copyright-holders:Martin Buchholz
/***************************************************************************

  Poly-Play
  (c) 1985 by VEB Polytechnik Karl-Marx-Stadt

  video hardware

  driver written by Martin Buchholz (buchholz@mail.uni-greifswald.de)

***************************************************************************/

#include "emu.h"
#include "includes/polyplay.h"


PALETTE_INIT_MEMBER(polyplay_state, polyplay)
{
	palette.set_pen_color(0,rgb_t(0x00,0x00,0x00));
	palette.set_pen_color(1,rgb_t(0xff,0xff,0xff));

	palette.set_pen_color(2,rgb_t(0x00,0x00,0x00));
	palette.set_pen_color(3,rgb_t(0xff,0x00,0x00));
	palette.set_pen_color(4,rgb_t(0x00,0xff,0x00));
	palette.set_pen_color(5,rgb_t(0xff,0xff,0x00));
	palette.set_pen_color(6,rgb_t(0x00,0x00,0xff));
	palette.set_pen_color(7,rgb_t(0xff,0x00,0xff));
	palette.set_pen_color(8,rgb_t(0x00,0xff,0xff));
	palette.set_pen_color(9,rgb_t(0xff,0xff,0xff));
}


WRITE8_MEMBER(polyplay_state::polyplay_characterram_w)
{
	if (m_characterram[offset] != data)
	{
		m_gfxdecode->gfx(1)->mark_dirty((offset >> 3) & 0x7f);

		m_characterram[offset] = data;
	}
}

void polyplay_state::video_start()
{
	m_gfxdecode->gfx(1)->set_source(m_characterram);
}


UINT32 polyplay_state::screen_update_polyplay(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 *videoram = m_videoram;
	offs_t offs;


	for (offs = 0; offs < 0x800; offs++)
	{
		int sx = (offs & 0x3f) << 3;
		int sy = offs >> 6 << 3;
		UINT8 code = videoram[offs];

		m_gfxdecode->gfx((code >> 7) & 0x01)->opaque(bitmap,cliprect,
				code, 0, 0, 0, sx, sy);
	}

	return 0;
}
