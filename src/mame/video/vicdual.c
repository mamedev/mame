/***************************************************************************

    VIC Dual Game board

****************************************************************************/

#include "emu.h"
#include "includes/vicdual.h"


static const pen_t pens_from_color_prom[] =
{
	RGB_BLACK,
	MAKE_RGB(0x00, 0xff, 0x00),
	MAKE_RGB(0x00, 0x00, 0xff),
	MAKE_RGB(0x00, 0xff, 0xff),
	MAKE_RGB(0xff, 0x00, 0x00),
	MAKE_RGB(0xff, 0xff, 0x00),
	MAKE_RGB(0xff, 0x00, 0xff),
	RGB_WHITE
};


WRITE8_MEMBER(vicdual_state::vicdual_palette_bank_w)
{
	machine().primary_screen->update_partial(machine().primary_screen->vpos());
	m_palette_bank = data & 3;
}


SCREEN_UPDATE_RGB32( vicdual_bw )
{
	vicdual_state *state = screen.machine().driver_data<vicdual_state>();
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
			char_code = state->m_videoram[offs];

			/* read the appropriate line of the character ram */
			offs = (char_code << 3) | (y & 0x07);
			video_data = state->m_characterram[offs];
		}

		/* plot the current pixel */
		pen = (video_data & 0x80) ? RGB_WHITE : RGB_BLACK;
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


SCREEN_UPDATE_RGB32( vicdual_color )
{
	vicdual_state *state = screen.machine().driver_data<vicdual_state>();
	UINT8 *color_prom = (UINT8 *)screen.machine().region("proms")->base();
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
			char_code = state->m_videoram[offs];

			/* read the appropriate line of the character ram */
			offs = (char_code << 3) | (y & 0x07);
			video_data = state->m_characterram[offs];

			/* get the foreground and background colors from the PROM */
			offs = (char_code >> 5) | (state->m_palette_bank << 3);
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


SCREEN_UPDATE_RGB32( vicdual_bw_or_color )
{
	if (vicdual_is_cabinet_color(screen.machine()))
		SCREEN_UPDATE32_CALL(vicdual_color);
	else
		SCREEN_UPDATE32_CALL(vicdual_bw);

	return 0;
}
