/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/tnzs.h"


/***************************************************************************

  The New Zealand Story doesn't have a color PROM. It uses 1024 bytes of RAM
  to dynamically create the palette. Each couple of bytes defines one
  color (15 bits per pixel; the top bit of the second byte is unused).
  Since the graphics use 4 bitplanes, hence 16 colors, this makes for 32
  different color codes.

***************************************************************************/


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Arkanoid has a two 512x8 palette PROMs. The two bytes joined together
  form 512 xRRRRRGGGGGBBBBB color values.

***************************************************************************/

PALETTE_INIT( arknoid2 )
{
	int i, col;

	for (i = 0; i < machine->total_colors(); i++)
	{
		col = (color_prom[i] << 8) + color_prom[i + 512];
		palette_set_color_rgb(machine, i, pal5bit(col >> 10), pal5bit(col >> 5), pal5bit(col >> 0));
	}
}



static void draw_background( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, UINT8 *m )
{
	tnzs_state *state = (tnzs_state *)machine->driver_data;
	int x, y, column, tot, transpen;
	int scrollx, scrolly;
	UINT32 upperbits;
	int ctrl2 = state->objctrl[1];


	if ((ctrl2 ^ (~ctrl2 << 1)) & 0x40)
		m += 0x800;

	if (state->bg_flag[0] & 0x80)
		transpen = -1;
	else
		transpen = 0;


	/* The byte at f200 is the y-scroll value for the first column.
       The byte at f204 is the LSB of x-scroll value for the first column.

       The other columns follow at 16-byte intervals.

       The 9th bit of each x-scroll value is combined into 2 bytes
       at f302-f303 */

	/* f301 controls how many columns are drawn. */
	tot = state->objctrl[1] & 0x1f;
	if (tot == 1)
		tot = 16;

	upperbits = state->objctrl[2] + state->objctrl[3] * 256;

	for (column = 0; column < tot; column++)
	{
		scrollx = state->scrollram[column * 16 + 4] - ((upperbits & 0x01) * 256);
		if (state->screenflip)
			scrolly = state->scrollram[column * 16] + 1 - 256;
		else
			scrolly = -state->scrollram[column * 16] + 1;

		for (y = 0; y < 16; y++)
		{
			for (x = 0; x < 2; x++)
			{
				int code, color, flipx, flipy, sx, sy;
				int i = 32 * (column ^ 8) + 2 * y + x;

				code = m[i] + ((m[i + 0x1000] & 0x3f) << 8);
				color = (m[i + 0x1200] & 0xf8) >> 3; /* colours at d600-d7ff */
				sx = x * 16;
				sy = y * 16;
				flipx = m[i + 0x1000] & 0x80;
				flipy = m[i + 0x1000] & 0x40;
				if (state->screenflip)
				{
					sy = 240 - sy;
					flipx = !flipx;
					flipy = !flipy;
				}

				drawgfx_transpen(bitmap,cliprect,machine->gfx[0],
						code,
						color,
						flipx,flipy,
						sx + scrollx,(sy + scrolly) & 0xff,
						transpen);

				/* wrap around x */
				drawgfx_transpen(bitmap,cliprect,machine->gfx[0],
						code,
						color,
						flipx,flipy,
						sx + 512 + scrollx,(sy + scrolly) & 0xff,
						transpen);
			}
		}

		upperbits >>= 1;
	}
}


static void draw_foreground( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect,
							 UINT8 *char_pointer, UINT8 *x_pointer, UINT8 *y_pointer, UINT8 *ctrl_pointer, UINT8 *color_pointer)
{
	tnzs_state *state = (tnzs_state *)machine->driver_data;
	int i;
	int ctrl2 = state->objctrl[1];


	if ((ctrl2 ^ (~ctrl2 << 1)) & 0x40)
	{
		char_pointer += 0x800;
		x_pointer += 0x800;
		ctrl_pointer += 0x800;
		color_pointer += 0x800;
	}


	/* Draw all 512 sprites */
	for (i = 0x1ff; i >= 0; i--)
	{
		int code, color, sx, sy, flipx, flipy;

		code = char_pointer[i] + ((ctrl_pointer[i] & 0x3f) << 8);
		color = (color_pointer[i] & 0xf8) >> 3;
		sx = x_pointer[i] - ((color_pointer[i] & 1) << 8);
		sy = 240 - y_pointer[i];
		flipx = ctrl_pointer[i] & 0x80;
		flipy = ctrl_pointer[i] & 0x40;
		if (state->screenflip)
		{
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
			/* hack to hide Chuka Taisens grey line, top left corner */
			if ((sy == 0) && (code == 0)) sy += 240;
		}

		drawgfx_transpen(bitmap,cliprect,machine->gfx[0],
				code,
				color,
				flipx,flipy,
				sx,sy+2,0);

		/* wrap around x */
		drawgfx_transpen(bitmap,cliprect,machine->gfx[0],
				code,
				color,
				flipx,flipy,
				sx + 512,sy+2,0);
	}
}

VIDEO_UPDATE( tnzs )
{
	tnzs_state *state = (tnzs_state *)screen->machine->driver_data;
	/* If the byte at f300 has bit 6 set, flip the screen
       (I'm not 100% sure about this) */
	state->screenflip = (state->objctrl[0] & 0x40) >> 6;


	/* Fill the background */
	bitmap_fill(bitmap, cliprect, 0x1f0);

	/* Redraw the background tiles (c400-c5ff) */
	draw_background(screen->machine, bitmap, cliprect, state->objram + 0x400);

	/* Draw the sprites on top */
	draw_foreground(screen->machine, bitmap, cliprect,
					state->objram + 0x0000, /*  chars : c000 */
					state->objram + 0x0200, /*      x : c200 */
					state->vdcram + 0x0000, /*      y : f000 */
					state->objram + 0x1000, /*   ctrl : d000 */
					state->objram + 0x1200); /* color : d200 */
	return 0;
}

VIDEO_EOF( tnzs )
{
	tnzs_state *state = (tnzs_state *)machine->driver_data;
	int ctrl2 =	state->objctrl[1];
	if (~ctrl2 & 0x20)
	{
		// note I copy sprites only. seta.c also copies the "floating tilemap"
		if (ctrl2 & 0x40)
		{
			memcpy(&state->objram[0x0000], &state->objram[0x0800], 0x0400);
			memcpy(&state->objram[0x1000], &state->objram[0x1800], 0x0400);
		}
		else
		{
			memcpy(&state->objram[0x0800], &state->objram[0x0000], 0x0400);
			memcpy(&state->objram[0x1800], &state->objram[0x1000], 0x0400);
		}

		// and I copy the "floating tilemap" BACKWARDS - this fixes kabukiz
		memcpy(&state->objram[0x0400], &state->objram[0x0c00], 0x0400);
		memcpy(&state->objram[0x1400], &state->objram[0x1c00], 0x0400);
	}
}

