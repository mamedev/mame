// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

    VIC Dual Game board

****************************************************************************/

#include "emu.h"
#include "vicdual.h"


static const pen_t pens_from_color_prom[] =
{
	rgb_t::black(),
	rgb_t(0x00, 0xff, 0x00),
	rgb_t(0x00, 0x00, 0xff),
	rgb_t(0x00, 0xff, 0xff),
	rgb_t(0xff, 0x00, 0x00),
	rgb_t(0xff, 0xff, 0x00),
	rgb_t(0xff, 0x00, 0xff),
	rgb_t::white()
};


void vicdual_state::palette_bank_w(uint8_t data)
{
	m_screen->update_partial(m_screen->vpos());
	m_palette_bank = data & 3;
}


uint32_t vicdual_state::screen_update_bw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint8_t x = 0;
	uint8_t y = cliprect.min_y;
	uint8_t video_data = 0;

	while (1)
	{
		if ((x & 0x07) == 0)
		{
			offs_t offs;
			uint8_t char_code;

			/* read the character code */
			offs = (y >> 3 << 5) | (x >> 3);
			char_code = m_videoram[offs];

			/* read the appropriate line of the character ram */
			offs = (char_code << 3) | (y & 0x07);
			video_data = m_characterram[offs];
		}

		/* plot the current pixel */
		pen_t pen = (video_data & 0x80) ? rgb_t::white() : rgb_t::black();
		bitmap.pix(y, x) = pen;

		/* next pixel */
		video_data <<= 1;
		x++;

		/* end of line? */
		if (x == 0)
		{
			/* end of region to update? */
			if (y == cliprect.max_y)
			{
				break;
			}

			/* next row */
			y++;
		}
	}

	return 0;
}


uint32_t vicdual_state::screen_update_color(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint8_t const *const color_prom = (uint8_t *)m_proms->base();
	uint8_t x = 0;
	uint8_t y = cliprect.min_y;
	uint8_t video_data = 0;
	pen_t back_pen = 0;
	pen_t fore_pen = 0;

	while (1)
	{
		if ((x & 0x07) == 0)
		{
			offs_t offs;

			/* read the character code */
			offs = (y >> 3 << 5) | (x >> 3);
			uint8_t char_code = m_videoram[offs];

			/* read the appropriate line of the character ram */
			offs = (char_code << 3) | (y & 0x07);
			video_data = m_characterram[offs];

			/* get the foreground and background colors from the PROM */
			offs = (char_code >> 5) | (m_palette_bank << 3);
			back_pen = pens_from_color_prom[(color_prom[offs] >> 1) & 0x07];
			fore_pen = pens_from_color_prom[(color_prom[offs] >> 5) & 0x07];
		}

		// this does nothing by default, but is used to enable overrides
		back_pen = choose_pen(x, y, back_pen);

		/* plot the current pixel */
		pen_t pen = (video_data & 0x80) ? fore_pen : back_pen;
		bitmap.pix(y, x) = pen;

		/* next pixel */
		video_data <<= 1;
		x++;

		/* end of line? */
		if (x == 0)
		{
			/* end of region to update? */
			if (y == cliprect.max_y)
			{
				break;
			}

			/* next row */
			y = y + 1;
		}
	}

	return 0;
}


uint32_t vicdual_state::screen_update_bw_or_color(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (is_cabinet_color())
		screen_update_color(screen, bitmap, cliprect);
	else
		screen_update_bw(screen, bitmap, cliprect);

	return 0;
}


pen_t vicdual_state::choose_pen(uint8_t x, uint8_t y, pen_t back_pen)
{
	return back_pen;
}


pen_t nsub_state::choose_pen(uint8_t x, uint8_t y, pen_t back_pen)
{
	return m_s97269pb->choose_pen(x, y, back_pen);
}
