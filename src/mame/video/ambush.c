/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/ambush.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

  I'm not sure about the resistor value, I'm using the Galaxian ones.

***************************************************************************/

PALETTE_INIT( ambush )
{
	int i;

	for (i = 0; i < machine->total_colors(); i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine, i, MAKE_RGB(r,g,b));
	}
}


static void draw_chars( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int priority )
{
	ambush_state *state = (ambush_state *)machine->driver_data;
	int offs, transpen;

	transpen = (priority == 0) ? -1 : 0;

	for (offs = 0; offs < state->videoram_size; offs++)
	{
		int code, sx, sy, col;
		UINT8 scroll;

		sy = (offs / 32);
		sx = (offs % 32);

		col = state->colorram[((sy & 0x1c) << 3) + sx];

		if ((col & 0x10) != priority)
			continue;

		scroll = ~state->scrollram[sx];

		code = state->videoram[offs] | ((col & 0x60) << 3);

		if (flip_screen_get(machine))
		{
			sx = 31 - sx;
			sy = 31 - sy;
			scroll = ~scroll - 1;
		}

		drawgfx_transpen(bitmap,cliprect,machine->gfx[0],
				code,
				(col & 0x0f) | ((*state->colorbank & 0x03) << 4),
				flip_screen_get(machine), flip_screen_get(machine),
				8 * sx, (8 * sy + scroll) & 0xff,
				transpen);
	}
}


VIDEO_UPDATE( ambush )
{
	ambush_state *state = (ambush_state *)screen->machine->driver_data;
	int offs;

	bitmap_fill(bitmap, cliprect, 0);

	/* Draw the background priority characters */
	draw_chars(screen->machine, bitmap, cliprect, 0x00);

	/* Draw the sprites. */
	for (offs = state->spriteram_size - 4; offs >= 0; offs -= 4)
	{
		int code, col, sx, sy, flipx, flipy, gfx;

		sy = state->spriteram[offs + 0];
		sx = state->spriteram[offs + 3];

		if ( (sy == 0) ||
			 (sy == 0xff) ||
			((sx <  0x40) && (  state->spriteram[offs + 2] & 0x10)) ||
			((sx >= 0xc0) && (!(state->spriteram[offs + 2] & 0x10))))
			continue;  /* prevent wraparound */


		code = (state->spriteram[offs + 1] & 0x3f) | ((state->spriteram[offs + 2] & 0x60) << 1);

		if (state->spriteram[offs + 2] & 0x80)
		{
			/* 16x16 sprites */
			gfx = 1;

			if (!flip_screen_get(screen->machine))
				sy = 240 - sy;
			else
				sx = 240 - sx;
		}
		else
		{
			/* 8x8 sprites */
			gfx = 0;
			code <<= 2;

			if (!flip_screen_get(screen->machine))
				sy = 248 - sy;
			else
				sx = 248 - sx;
		}

		col   = state->spriteram[offs + 2] & 0x0f;
		flipx = state->spriteram[offs + 1] & 0x40;
		flipy = state->spriteram[offs + 1] & 0x80;

		if (flip_screen_get(screen->machine))
		{
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap,cliprect,screen->machine->gfx[gfx],
				code, col | ((*state->colorbank & 0x03) << 4),
				flipx, flipy,
				sx,sy,0);
	}


	/* Draw the foreground priority characters */
	draw_chars(screen->machine, bitmap, cliprect, 0x10);
	return 0;
}
