// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

    VIC Dual Game board

****************************************************************************/

#include "emu.h"
#include "includes/vicdual.h"


static const pen_t pens_from_color_prom[] =
{
	rgb_t::black,
	rgb_t(0x00, 0xff, 0x00),
	rgb_t(0x00, 0x00, 0xff),
	rgb_t(0x00, 0xff, 0xff),
	rgb_t(0xff, 0x00, 0x00),
	rgb_t(0xff, 0xff, 0x00),
	rgb_t(0xff, 0x00, 0xff),
	rgb_t::white
};


WRITE8_MEMBER(vicdual_state::palette_bank_w)
{
	m_screen->update_partial(m_screen->vpos());
	m_palette_bank = data & 3;
}


UINT32 vicdual_state::screen_update_bw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT8 x = 0;
	UINT8 y = cliprect.min_y;
	UINT8 video_data = 0;

	while (1)
	{
		pen_t pen;

		if ((x & 0x07) == 0)
		{
			offs_t offs;
			UINT8 char_code;

			/* read the character code */
			offs = (y >> 3 << 5) | (x >> 3);
			char_code = m_videoram[offs];

			/* read the appropriate line of the character ram */
			offs = (char_code << 3) | (y & 0x07);
			video_data = m_characterram[offs];
		}

		/* plot the current pixel */
		pen = (video_data & 0x80) ? rgb_t::white : rgb_t::black;
		bitmap.pix32(y, x) = pen;

		/* next pixel */
		video_data = video_data << 1;
		x = x + 1;

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


UINT32 vicdual_state::screen_update_color(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT8 *color_prom = (UINT8 *)m_proms->base();
	UINT8 x = 0;
	UINT8 y = cliprect.min_y;
	UINT8 video_data = 0;
	pen_t back_pen = 0;
	pen_t fore_pen = 0;

	while (1)
	{
		pen_t pen;

		if ((x & 0x07) == 0)
		{
			offs_t offs;
			UINT8 char_code;

			/* read the character code */
			offs = (y >> 3 << 5) | (x >> 3);
			char_code = m_videoram[offs];

			/* read the appropriate line of the character ram */
			offs = (char_code << 3) | (y & 0x07);
			video_data = m_characterram[offs];

			/* get the foreground and background colors from the PROM */
			offs = (char_code >> 5) | (m_palette_bank << 3);
			back_pen = pens_from_color_prom[(color_prom[offs] >> 1) & 0x07];
			fore_pen = pens_from_color_prom[(color_prom[offs] >> 5) & 0x07];
		}

		/* plot the current pixel */
		pen = (video_data & 0x80) ? fore_pen : back_pen;
		bitmap.pix32(y, x) = pen;

		/* next pixel */
		video_data = video_data << 1;
		x = x + 1;

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


UINT32 vicdual_state::screen_update_bw_or_color(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (is_cabinet_color())
		screen_update_color(screen, bitmap, cliprect);
	else
		screen_update_bw(screen, bitmap, cliprect);

	return 0;
}
