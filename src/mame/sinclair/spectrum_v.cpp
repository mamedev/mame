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
#include "spectrum.h"
#include "spec128.h"

TIMER_CALLBACK_MEMBER(spectrum_state::finish_screen_update)
{
	/* TMP screen.cpp at the current state behaves incorrectly if layout is configured as: visarea.bottom() == m_screen.bottom()
	This leads video_manager::finish_screen_updates() to ignore final update_now(). As a result partially rendered scanline of
	the screen is redrawn entirely.
	Consider this timer a tmp solution till screen.cpp provides better support for such case.
	Validation:
	    https://mametesters.org/view.php?id=8264
	    https://mametesters.org/view.php?id=8265
	    https://github.com/mamedev/mame/pull/9670#issuecomment-1118576555
	    https://github.com/mamedev/mame/pull/9750
	*/
	if (started()) m_screen->update_now();
	m_finish_screen_update_timer->adjust(m_screen->time_until_pos(m_screen->visible_area().bottom(), m_screen->visible_area().right() + 1));
}

/***************************************************************************
  Start the video hardware emulation.
***************************************************************************/
void spectrum_state::video_start()
{
	m_irq_on_timer = timer_alloc(FUNC(spectrum_state::irq_on), this);
	m_irq_off_timer = timer_alloc(FUNC(spectrum_state::irq_off), this);
	m_finish_screen_update_timer = timer_alloc(FUNC(spectrum_state::finish_screen_update), this);
	finish_screen_update(0);

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
	return { SPEC_LEFT_BORDER, SPEC_LEFT_BORDER + SPEC_DISPLAY_XSIZE - 1,
			SPEC_UNSEEN_LINES + SPEC_TOP_BORDER, SPEC_UNSEEN_LINES + SPEC_TOP_BORDER + SPEC_DISPLAY_YSIZE - 1 };
}

u8 spectrum_state::get_border_color(u16 hpos, u16 vpos)
{
	return m_port_fe_data & 0x07;
}

u32 spectrum_state::screen_update_spectrum(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	rectangle scr = get_screen_area();
	rectangle vis = screen.visible_area();
	if (vis != scr)
	{
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
             T: |       |   0   |   1   |   2   |   3   |   4   |   5   |   6   |   7   |
ULA contention: |       |   6   |   5   |   4   |   3   |   2   |   1   |   0   |   0   |
 opcode read T: |   0   |   1   | 2 RSH | 3 RSH |       |       |       |       |       | ULA reads lower address for char1/attr1 from R6:0
 opcode read T: |       |       |   0   |   1   | 2 RSH | 3 RSH |       |       |       | ULA skip reading char2/attr2 and keep them from char1/attr1
            px: |       | 0 | 1 | 2 | 3 |*4*| 5 | 6 | 7 |*0*| 1 | 2 | 3 | 4 | 5 | 6 | 7 |
                |       | << !right <<  | char1 | attr1 | char2 | attr2 |   >> right >> |

TODO Curren implementation only tracks char switch position. In order to track both (char and attr) we need to share
     some state between screen->update() events.
*/
void spectrum_state::spectrum_update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bool invert_attrs = u64(screen.frame_number() / m_frame_invert_count) & 1;
	for (u16 vpos = cliprect.top(); vpos <= cliprect.bottom(); vpos++)
	{
		u16 hpos = cliprect.left();
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
		u8 *scr = &m_screen_location[((y & 0xc0) << 5) | ((y & 7) << 8) | ((y & 0x38) << 2) | (x >> 3)];
		u8 *attr = &m_screen_location[0x1800 | (((y & 0xf8) << 2) | (x >> 3))];
		u16 *pix = &(bitmap.pix(vpos, hpos));

		while ((hpos + (chunk_right ? 0 : 4)) <= cliprect.right())
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
		m_is_m1_rd_contended = true; // make sure M1 sets it to false before
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

// see: spectrum_state::spectrum_update_screen()
void spectrum_state::spectrum_refresh_w(offs_t offset, uint8_t data)
{
	const u8 i = offset >> 8;
	bool is_snow_possible = !m_contention_pattern.empty() && is_contended(i << 8);
	if (!is_snow_possible || m_is_m1_rd_contended)
		return;

	const u64 hpos = m_screen->hpos();
	const u64 vpos = m_screen->vpos();
	if (hpos < get_screen_area().left() || hpos > get_screen_area().right() || vpos < get_screen_area().top() || vpos > get_screen_area().bottom())
		return;

	u8 x = hpos - get_screen_area().left();
	const u16 y = vpos - get_screen_area().top();

	// icount is not adjusted yet, everything below is happening during
	// the last REFRESH cycle: +2px(1t of 3.5MHz)
	u8 snow_pattern = 0;
	u8 t_to_chunk_end = 0;
	if (x % 16 == 0x00)
	{
		snow_pattern = 1;
		t_to_chunk_end = 4;
	}
	else if (x % 16 == 0x04)
	{
		snow_pattern = 2;
		x += 8;
		t_to_chunk_end = 6;
	}
	else
	{
		return;
	}

	const u16 px_addr_hi = ((y & 0xc0) << 5) | ((y & 7) << 8) | ((y & 0x20) << 2);
	const u16 attr_addr_hi = 0x1800 | ((y & 0xe0) << 2);
	const u8 addr_lo = ((y & 0x18) << 2) | (x >> 3);

	const u8 px_tmp = m_screen_location[px_addr_hi | addr_lo];
	const u8 attr_tmp = m_screen_location[attr_addr_hi | addr_lo];

	if (snow_pattern == 1)
	{
		const u8 r = (offset + 1) & 0x7f; // R must be already incremented during refresh. Consider z80 update.
		const u8* base = snow_pattern1_base(i);
		m_screen_location[px_addr_hi | addr_lo] = base[px_addr_hi | r];
		m_screen_location[attr_addr_hi | addr_lo] = base[attr_addr_hi | r];
	}
	else if (snow_pattern == 2)
	{
		m_screen_location[px_addr_hi | addr_lo] = m_screen_location[(px_addr_hi | addr_lo) - 1];
		m_screen_location[attr_addr_hi | addr_lo] = m_screen_location[(attr_addr_hi | addr_lo) - 1];
	}

	m_maincpu->adjust_icount(-t_to_chunk_end);
	m_screen->update_now();
	m_maincpu->adjust_icount(t_to_chunk_end);
	m_screen_location[px_addr_hi | addr_lo] = px_tmp;
	m_screen_location[attr_addr_hi | addr_lo] = attr_tmp;
}

u8* spectrum_state::snow_pattern1_base(u8 i_reg)
{
	return m_screen_location;
}
