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
	m_irq_on_timer = timer_alloc(FUNC(spectrum_state::irq_on), this);
	m_irq_off_timer = timer_alloc(FUNC(spectrum_state::irq_off), this);

	m_frame_invert_count = 16;
	m_screen_location = m_video_ram;
	m_contention_pattern = {6, 5, 4, 3, 2, 1, 0, 0};
	m_contention_offset = -1;
	m_border4t_render_at = 2;
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

static constexpr rgb_t spectrum_pens[16] = {
	{ 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0xbf },
	{ 0xbf, 0x00, 0x00 },
	{ 0xbf, 0x00, 0xbf },
	{ 0x00, 0xbf, 0x00 },
	{ 0x00, 0xbf, 0xbf },
	{ 0xbf, 0xbf, 0x00 },
	{ 0xbf, 0xbf, 0xbf },
	{ 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0xff },
	{ 0xff, 0x00, 0x00 },
	{ 0xff, 0x00, 0xff },
	{ 0x00, 0xff, 0x00 },
	{ 0x00, 0xff, 0xff },
	{ 0xff, 0xff, 0x00 },
	{ 0xff, 0xff, 0xff }
};

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

u8 spectrum_state::get_border_color(u16 hpos, u16 vpos)
{
	return m_port_fe_data & 0x07;
}

u32 spectrum_state::screen_update_spectrum(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	rectangle scr = get_screen_area();
	rectangle vis = screen.visible_area();
	if (vis != scr) {
		rectangle bsides[4] = {
			rectangle(vis.left(),      vis.right(),    vis.top(),        scr.top() - 1),
			rectangle(vis.left(),      scr.left() - 1, scr.top(),        scr.bottom()),
			rectangle(scr.right() + 1, vis.right(),    scr.top(),        scr.bottom()),
			rectangle(vis.left(),      vis.right(),    scr.bottom() + 1, vis.bottom())
		};
		for (auto i = 0; i < 4; i++)
		{
			rectangle border = bsides[i] & cliprect;
			if (!border.empty())
				spectrum_update_border(screen, bitmap, border);
		}
	}

	scr &= cliprect;
	if (!scr.empty())
		spectrum_update_screen(screen, bitmap, scr);

	return 0;
}

void spectrum_state::spectrum_update_border(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &border)
{
	u8 mod = m_contention_pattern.empty() ? 1 : 8;
	u8 at = m_contention_pattern.empty() ? 0 : m_border4t_render_at;
	for (auto y = border.top(); y <= border.bottom(); y++)
	{
		u16 *pix = &(bitmap.pix(y, border.left()));
		for (auto x = border.left(); x <= border.right(); )
		{
			if (x % mod == at)
			{
				pix -= at;
				x -= at;
				for (auto m = 0; m < mod; m++, x++)
					*pix++ = get_border_color(y, x);
			}
			else
			{
				pix++;
				x++;
			}
		}
	}
}

/* ULA reads screen data in 16px (8T) chunks as following:
 T: |   0   |   1   |   2   |   3   |   4   |   5   |   6   |   7   |
px: | 0 | 1 | 2 | 3 |*4*| 5 | 6 | 7 |*0*| 1 | 2 | 3 | 4 | 5 | 6 | 7 |
    | << !right <<  | char1 | attr1 | char2 | attr2 |   >> right >> |

TODO Curren implementation only tracks char switch position. In order to track both (char and attr) we need to share
     some state between screen->update() events.
*/
void spectrum_state::spectrum_update_screen(screen_device &screen_d, bitmap_ind16 &bitmap, const rectangle &screen)
{
	u8 *attrs_location = m_screen_location + 0x1800;
	bool invert_attrs = u64(screen_d.frame_number() / m_frame_invert_count) & 1;
	for (u16 vpos = screen.top(); vpos <= screen.bottom(); vpos++)
	{
		u16 hpos = screen.left();
		u16 x = hpos - get_screen_area().left();
		bool chunk_right = x & 8;
		if (x % 8 <= (chunk_right ? 0 : 4))
		{
			u8 shift = x % 8;
			x -= shift;
			hpos -= shift;
		}
		else
		{
			u8 shift = 8 - (x % 8);
			x += shift;
			hpos += shift;
			chunk_right = !chunk_right;
		}
		u16 y = vpos - get_screen_area().top();
		u8 *scr = &m_screen_location[((y & 7) << 8) | ((y & 0x38) << 2) | ((y & 0xc0) << 5) | (x >> 3)];
		u8 *attr = &attrs_location[((y & 0xf8) << 2) | (x >> 3)];
		u16 *pix = &(bitmap.pix(vpos, hpos));

		while ((hpos + (chunk_right ? 0 : 4)) <= screen.right())
		{
			u16 ink = ((*attr >> 3) & 0x08) | (*attr & 0x07);
			u16 pap = (*attr >> 3) & 0x0f;
			u8 pix8 = (invert_attrs && (*attr & 0x80)) ? ~*scr : *scr;

			for (u8 b = 0x80; b; b >>= 1, x++, hpos++)
				*pix++ = (pix8 & b) ? ink : pap;
			scr++;
			attr++;
			chunk_right = !chunk_right;
		}
	}
}

bool spectrum_state::is_vram_write(offs_t offset) {
	return offset >= 0x4000 && offset < 0x5b00;
}

bool spectrum_state::is_contended(offs_t offset) {
	return offset >= 0x4000 && offset < 0x8000;
}

void spectrum_state::content_early(s8 shift)
{
	u64 vpos = m_screen->vpos();
	if (m_contention_pattern.empty() || vpos < get_screen_area().top() || vpos > get_screen_area().bottom())
		return;

	u64 now = m_maincpu->total_cycles() - m_int_at + shift;
	u64 cf = vpos * m_screen->width() * m_maincpu->clock() / m_screen->clock() + m_contention_offset;
	u64 ct = cf + get_screen_area().width() * m_maincpu->clock() / m_screen->clock();

	if(cf <= now && now < ct)
	{
		u64 clocks = now - cf;
		u8 c = m_contention_pattern[clocks % m_contention_pattern.size()];
		m_maincpu->adjust_icount(-c);
	}
}

void spectrum_state::content_late()
{
	u64 vpos = m_screen->vpos();
	if (m_contention_pattern.empty() || vpos < get_screen_area().top() || vpos > get_screen_area().bottom())
		return;

	u64 now = m_maincpu->total_cycles() - m_int_at + 1;
	u64 cf = vpos * m_screen->width() * m_maincpu->clock() / m_screen->clock() + m_contention_offset;
	u64 ct = cf + get_screen_area().width() * m_maincpu->clock() / m_screen->clock();
	for(auto i = 0x04; i; i >>= 1)
	{
		if(cf <= now && now < ct)
		{
			u64 clocks = now - cf;
			u8 c = m_contention_pattern[clocks % m_contention_pattern.size()];
			m_maincpu->adjust_icount(-c);
			now += c;
		}
		now++;
	}
}

void spectrum_state::spectrum_nomreq(offs_t offset, uint8_t data)
{
	if (is_contended(offset)) content_early();
}
