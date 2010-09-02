/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/spcforce.h"


WRITE8_HANDLER( spcforce_flip_screen_w )
{
	flip_screen_set(space->machine, ~data & 0x01);
}


VIDEO_UPDATE( spcforce )
{
	spcforce_state *state = screen->machine->driver_data<spcforce_state>();
	int offs;
	int flip = flip_screen_get(screen->machine);

	/* draw the characters as sprites because they could be overlapping */
	bitmap_fill(bitmap,cliprect,0);
	for (offs = 0; offs < 0x400; offs++)
	{
		int code,sx,sy,col;

		sy = 8 * (offs / 32) -  (state->scrollram[offs]       & 0x0f);
		sx = 8 * (offs % 32) + ((state->scrollram[offs] >> 4) & 0x0f);

		code = state->videoram[offs] + ((state->colorram[offs] & 0x01) << 8);
		col  = (~state->colorram[offs] >> 4) & 0x07;

		if (flip)
		{
			sx = 248 - sx;
			sy = 248 - sy;
		}

		drawgfx_transpen(bitmap,cliprect,screen->machine->gfx[0],
				code, col,
				flip, flip,
				sx, sy,0);
	}

	return 0;
}
