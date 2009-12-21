/* Funny Bubble Video hardware

todo - convert to tilemap

 */


#include "driver.h"
#include "includes/funybubl.h"


WRITE8_HANDLER ( funybubl_paldatawrite )
{
	funybubl_state *state = (funybubl_state *)space->machine->driver_data;
	int colchanged ;
	UINT32 coldat;

	state->paletteram[offset] = data;
	colchanged = offset >> 2;
	coldat = state->paletteram[colchanged * 4] | (state->paletteram[colchanged * 4 + 1] << 8) |
			(state->paletteram[colchanged * 4 + 2] << 16) | (state->paletteram[colchanged * 4 + 3] << 24);

	palette_set_color_rgb(space->machine, colchanged, pal6bit(coldat >> 12), pal6bit(coldat >> 0), pal6bit(coldat >> 6));
}


VIDEO_START(funybubl)
{
}

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	funybubl_state *state = (funybubl_state *)machine->driver_data;
	UINT8 *source = &state->banked_vram[0x2000 - 0x20];
	UINT8 *finish = source - 0x1000;

	while (source > finish)
	{
		int xpos, ypos, tile;

		/* the sprites are in the sprite list twice
         the first format (in comments) appears to be a buffer, if you use
         this list you get garbage sprites in 2 player mode
         the second format (used) seems correct

         */
/*
        ypos = 0xff - source[1 + 0x10];
        xpos = source[2 + 0x10];
        tile = source[0 + 0x10] | ( (source[3 + 0x10] & 0x0f) <<8);
        if (source[3 + 0x10] & 0x80) tile += 0x1000;
        if (source[3 + 0x10] & 0x20) xpos += 0x100;
        // bits 0x40 (not used?) and 0x10 (just set during transition period of x co-ord 0xff and 0x00) ...
        xpos -= 8;
        ypos -= 14;

*/
		ypos = source[2];
		xpos = source[3];
		tile = source[0] | ( (source[1] & 0x0f) << 8);
		if (source[1] & 0x80) tile += 0x1000;
		if (source[1] & 0x20)
		{
			if (xpos < 0xe0)
				xpos += 0x100;
		}

		// bits 0x40 and 0x10 not used?...

		drawgfx_transpen(bitmap, cliprect, machine->gfx[1], tile, 0, 0, 0, xpos, ypos, 255);
		source -= 0x20;
	}
}


VIDEO_UPDATE(funybubl)
{
	funybubl_state *state = (funybubl_state *)screen->machine->driver_data;
	int x, y, offs;
	offs = 0;

	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));

	/* tilemap .. convert it .. banking makes it slightly more annoying but still easy */
	for (y = 0; y < 32; y++)
	{
		for (x = 0; x< 64; x++)
		{
			int data;

			data = state->banked_vram[offs] | (state->banked_vram[offs + 1] << 8);
			drawgfx_transpen(bitmap, cliprect, screen->machine->gfx[0], data & 0x7fff, (data & 0x8000) ? 2 : 1, 0, 0, x*8, y*8, 0);
			offs += 2;
		}
	}

	draw_sprites(screen->machine, bitmap, cliprect);

#if 0
	if ( input_code_pressed_once(screen->machine, KEYCODE_W) )
	{
		FILE *fp;

		fp = fopen("funnybubsprites", "w+b");
		if (fp)
		{
			fwrite(&state->banked_vram[0x1000], 0x1000, 1, fp);
			fclose(fp);
		}
	}
#endif
	return 0;
}
