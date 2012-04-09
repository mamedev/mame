/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/spcforce.h"


WRITE8_MEMBER(spcforce_state::spcforce_flip_screen_w)
{
	flip_screen_set(~data & 0x01);
}


SCREEN_UPDATE_IND16( spcforce )
{
	spcforce_state *state = screen.machine().driver_data<spcforce_state>();
	int offs;
	int flip = state->flip_screen();

	/* draw the characters as sprites because they could be overlapping */
	bitmap.fill(0, cliprect);
	for (offs = 0; offs < 0x400; offs++)
	{
		int code,sx,sy,col;

		sy = 8 * (offs / 32) -  (state->m_scrollram[offs]       & 0x0f);
		sx = 8 * (offs % 32) + ((state->m_scrollram[offs] >> 4) & 0x0f);

		code = state->m_videoram[offs] + ((state->m_colorram[offs] & 0x01) << 8);
		col  = (~state->m_colorram[offs] >> 4) & 0x07;

		if (flip)
		{
			sx = 248 - sx;
			sy = 248 - sy;
		}

		drawgfx_transpen(bitmap,cliprect,screen.machine().gfx[0],
				code, col,
				flip, flip,
				sx, sy,0);
	}

	return 0;
}
