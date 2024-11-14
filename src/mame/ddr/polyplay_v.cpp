// license:BSD-3-Clause
// copyright-holders:Martin Buchholz
// thanks-to:James Wallace, Martin Buchholz, Juergen Oppermann, Volker Hann, Jan-Ole Christian
/***************************************************************************

  Poly-Play
  (c) 1985 by VEB Polytechnik Karl-Marx-Stadt

  video hardware

  driver written by Martin Buchholz (buchholz@mail.uni-greifswald.de)

***************************************************************************/

#include "emu.h"
#include "polyplay.h"

void polyplay_state::polyplay_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(0x00, 0x00, 0x00));
	palette.set_pen_color(1, rgb_t(0xff, 0xff, 0xff));
	palette.set_pen_color(2, rgb_t(0x00, 0x00, 0x00));
	palette.set_pen_color(3, rgb_t(0xff, 0x00, 0x00));
	palette.set_pen_color(4, rgb_t(0x00, 0xff, 0x00));
	palette.set_pen_color(5, rgb_t(0xff, 0xff, 0x00));
	palette.set_pen_color(6, rgb_t(0x00, 0x00, 0xff));
	palette.set_pen_color(7, rgb_t(0xff, 0x00, 0xff));
	palette.set_pen_color(8, rgb_t(0x00, 0xff, 0xff));
	palette.set_pen_color(9, rgb_t(0xff, 0xff, 0xff));
}

void polyplay_state::polyplay_characterram_w(offs_t offset, uint8_t data)
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

uint32_t polyplay_state::screen_update_polyplay(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t const *const videoram = m_videoram;

	for (offs_t offs = 0; offs < 0x800; offs++)
	{
		int const sx = (offs & 0x3f) << 3;
		int const sy = offs >> 6 << 3;
		uint8_t const code = videoram[offs];

		m_gfxdecode->gfx(BIT(code, 7))->opaque(bitmap, cliprect, code, 0, 0, 0, sx, sy);
	}

	return 0;
}
