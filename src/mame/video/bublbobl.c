/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"
#include "includes/bublbobl.h"


VIDEO_UPDATE( bublbobl )
{
	bublbobl_state *state = (bublbobl_state *)screen->machine->driver_data;
	int offs;
	int sx, sy, xc, yc;
	int gfx_num, gfx_attr, gfx_offs;
	const UINT8 *prom;
	const UINT8 *prom_line;


	/* Bubble Bobble doesn't have a real video RAM. All graphics (characters */
	/* and sprites) are stored in the same memory region, and information on */
	/* the background character columns is stored in the area dd00-dd3f */

	/* This clears & redraws the entire screen each pass */
	bitmap_fill(bitmap, cliprect, 255);

	if (!state->video_enable)
		return 0;

	sx = 0;

	prom = memory_region(screen->machine, "proms");
	for (offs = 0; offs < state->objectram_size; offs += 4)
	{
		/* skip empty sprites */
		/* this is dword aligned so the UINT32 * cast shouldn't give problems */
		/* on any architecture */
		if (*(UINT32 *)(&state->objectram[offs]) == 0)
			continue;

		gfx_num = state->objectram[offs + 1];
		gfx_attr = state->objectram[offs + 3];
		prom_line = prom + 0x80 + ((gfx_num & 0xe0) >> 1);

		gfx_offs = ((gfx_num & 0x1f) * 0x80);
		if ((gfx_num & 0xa0) == 0xa0)
			gfx_offs |= 0x1000;

		sy = -state->objectram[offs + 0];

		for (yc = 0; yc < 32; yc++)
		{
			if (prom_line[yc / 2] & 0x08)	continue;	/* NEXT */

			if (!(prom_line[yc / 2] & 0x04))	/* next column */
			{
				sx = state->objectram[offs + 2];
				if (gfx_attr & 0x40) sx -= 256;
			}

			for (xc = 0; xc < 2; xc++)
			{
				int goffs, code, color, flipx, flipy, x, y;

				goffs = gfx_offs + xc * 0x40 + (yc & 7) * 0x02 + (prom_line[yc/2] & 0x03) * 0x10;
				code = state->videoram[goffs] + 256 * (state->videoram[goffs + 1] & 0x03) + 1024 * (gfx_attr & 0x0f);
				color = (state->videoram[goffs + 1] & 0x3c) >> 2;
				flipx = state->videoram[goffs + 1] & 0x40;
				flipy = state->videoram[goffs + 1] & 0x80;
				x = sx + xc * 8;
				y = (sy + yc * 8) & 0xff;

				if (flip_screen_get(screen->machine))
				{
					x = 248 - x;
					y = 248 - y;
					flipx = !flipx;
					flipy = !flipy;
				}

				drawgfx_transpen(bitmap,cliprect,screen->machine->gfx[0],
						code,
						color,
						flipx,flipy,
						x,y,15);
			}
		}

		sx += 16;
	}
	return 0;
}
