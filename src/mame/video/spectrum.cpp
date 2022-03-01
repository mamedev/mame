// license:GPL-2.0+
// copyright-holders:Kevin Thacker
/***************************************************************************

  spectrum.cpp

  Functions to emulate the video hardware of the ZX Spectrum.

  Changes:

  DJR 08/02/00 - Added support for FLASH 1.
  DJR 16/05/00 - Support for TS2068/TC2048 hires and 64 column modes.
  DJR 19/05/00 - Speeded up Spectrum 128 screen refresh.
  DJR 23/05/00 - Preliminary support for border colour emulation.

***************************************************************************/

#include "emu.h"
#include "includes/spectrum.h"
#include "includes/spec128.h"

/***************************************************************************
  Start the video hardware emulation.
***************************************************************************/
void spectrum_state::video_start()
{
	m_frame_invert_count = 16;
	m_frame_number = 0;
	m_flash_invert = 0;

	m_previous_screen_x = m_previous_screen_y = 0;
	m_screen_location = m_video_ram;

	m_scanline_timer = timer_alloc(TIMER_SCANLINE);
}

void spectrum_128_state::video_start()
{
	m_frame_invert_count = 16;
	m_frame_number = 0;
	m_flash_invert = 0;

	m_previous_screen_x = m_previous_screen_y = 0;
	m_screen_location = m_ram->pointer() + (5 << 14);

	m_scanline_timer = timer_alloc(TIMER_SCANLINE);
}

/* Code to change the FLASH status every 25 frames. Note this must be
   independent of frame skip etc. */
WRITE_LINE_MEMBER(spectrum_state::screen_vblank_spectrum)
{
	// rising edge
	if (state)
	{
		m_frame_number++;
		if (m_frame_number >= m_frame_invert_count)
		{
			m_frame_number = 0;
			m_flash_invert = !m_flash_invert;
		}
	}
}

/***************************************************************************
  Update the spectrum screen display.

  The screen consists of 312 scanlines as follows:
  64  border lines (the last 48 are actual border lines; the others may be
					border lines or vertical retrace)
  192 screen lines
  56  border lines

  Each screen line has 48 left border pixels, 256 screen pixels and 48 right
  border pixels.

  Each scanline takes 224 T-states divided as follows:
  128 Screen (reads a screen and ATTR byte [8 pixels] every 4 T states)
  24  Right border
  48  Horizontal retrace
  24  Left border

  The 128K Spectrums have only 63 scanlines before the TV picture (311 total)
  and take 228 T-states per scanline.

***************************************************************************/

uint32_t spectrum_state::screen_update_spectrum(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// Mostly for support attributes flashing.
	// Only requires force full refresh if updates are not performed per scanline.
	spectrum_update_screen(m_scanline_timer == nullptr);
	return 0;
}

static constexpr rgb_t spectrum_pens[16] = {
	{0x00, 0x00, 0x00},
	{0x00, 0x00, 0xbf},
	{0xbf, 0x00, 0x00},
	{0xbf, 0x00, 0xbf},
	{0x00, 0xbf, 0x00},
	{0x00, 0xbf, 0xbf},
	{0xbf, 0xbf, 0x00},
	{0xbf, 0xbf, 0xbf},
	{0x00, 0x00, 0x00},
	{0x00, 0x00, 0xff},
	{0xff, 0x00, 0x00},
	{0xff, 0x00, 0xff},
	{0x00, 0xff, 0x00},
	{0x00, 0xff, 0xff},
	{0xff, 0xff, 0x00},
	{0xff, 0xff, 0xff}};

// Initialise the palette
void spectrum_state::spectrum_palette(palette_device &palette) const
{
	palette.set_pen_colors(0, spectrum_pens);
}

rectangle spectrum_state::get_screen_area()
{
	// 256x192 screen position without border
	return rectangle{48, 48 + 255, 64, 64 + 191};
}

void spectrum_state::to_area(rectangle area, unsigned int &x, unsigned int &y)
{
	if (y < area.top() || y > area.bottom())
	{
		x = area.left();
		y = area.top();
	}
	else if (x > area.right())
	{
		x = area.left();
		to_area(area, x, ++y);
	}
	else if (x < area.left())
	{
		x = area.left();
	}
}

void spectrum_state::spectrum_update_screen(bool eof, bool border_only)
{
	unsigned int to_x = m_screen->hpos();
	unsigned int to_y = m_screen->vpos();
	to_area(m_screen->visible_area(), to_x, to_y);

	if ((m_previous_screen_x == to_x) && (m_previous_screen_y == to_y))
	{
		if (eof)
		{
			m_previous_screen_x = m_screen->visible_area().left();
			m_previous_screen_y++;
		}
		else
			return;
	}

	bitmap_ind16 *bm = &m_screen->curbitmap().as_ind16();
	if (bm->valid())
	{
		rectangle screen = get_screen_area();
		u8 *attrs_location = m_screen_location + 0x1800;
		bool stop = false;
		do
		{
			u16 *pix = &bm->pix(m_previous_screen_y, m_previous_screen_x);
			s32 x_max = m_screen->visible_area().right() + 1;
			if (m_previous_screen_y == to_y)
				x_max = std::min(s32(to_x), x_max);

			u16 border_color = get_border_color();
			if (m_previous_screen_y >= screen.top() && m_previous_screen_y <= screen.bottom())
			{
				for (; m_previous_screen_x < std::min(screen.left(), x_max); m_previous_screen_x++)
					*pix++ = border_color;

				// Some clones e.g. timex, tsconf use custom drawing and only inherit border updates.
				if (border_only)
				{
					s16 width = std::min(screen.right() + 1, x_max) - m_previous_screen_x;
					if (width > 0)
					{
						pix += width;
						m_previous_screen_x += width;
					}
				}
				else
				{
					u16 x = m_previous_screen_x - screen.left();
					u16 y = m_previous_screen_y - screen.top();
					u8 *scr = &m_screen_location[((y & 7) << 8) | ((y & 0x38) << 2) | ((y & 0xc0) << 5) | (x >> 3)];
					u8 *attr = &attrs_location[((y & 0xf8) << 2) | (x >> 3)];
					while (m_previous_screen_x < std::min(screen.right() + 1, x_max))
					{
						u16 ink = ((*attr >> 3) & 0x08) | (*attr & 0x07);
						u16 pap = (*attr >> 3) & 0x0f;
						u8 pix8 = (m_flash_invert && (*attr & 0x80)) ? ~*scr : *scr;

						for (u8 b = 0x80 >> (x & 0x07); b != 0 && m_previous_screen_x < std::min(screen.right() + 1, x_max); b >>= 1)
						{
							*pix++ = (pix8 & b) ? ink : pap;
							x++;
							m_previous_screen_x++;
						}
						scr++;
						attr++;
					}
				}
			}
			for (; m_previous_screen_x < x_max; m_previous_screen_x++)
				*pix++ = border_color;

			stop = m_previous_screen_y == to_y;
			to_area(m_screen->visible_area(), m_previous_screen_x, m_previous_screen_y);
		} while (!stop);
	}
}

u16 spectrum_state::get_border_color()
{
	return m_port_fe_data & 0x07;
}
