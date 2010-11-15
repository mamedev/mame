/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/marineb.h"


PALETTE_INIT( marineb )
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
		bit1 = (color_prom[i + machine->total_colors()] >> 0) & 0x01;
		bit2 = (color_prom[i + machine->total_colors()] >> 1) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = (color_prom[i + machine->total_colors()] >> 2) & 0x01;
		bit2 = (color_prom[i + machine->total_colors()] >> 3) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine, i, MAKE_RGB(r,g,b));
	}
}

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_tile_info )
{
	marineb_state *state = machine->driver_data<marineb_state>();

	UINT8 code = state->videoram[tile_index];
	UINT8 col = state->colorram[tile_index];

	SET_TILE_INFO(0,
				  code | ((col & 0xc0) << 2),
				  (col & 0x0f) | (state->palette_bank << 4),
				  TILE_FLIPXY((col >> 4) & 0x03));
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( marineb )
{
	marineb_state *state = machine->driver_data<marineb_state>();

	state->bg_tilemap = tilemap_create(machine, get_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	tilemap_set_scroll_cols(state->bg_tilemap, 32);

	state_save_register_global(machine, state->palette_bank);
	state_save_register_global(machine, state->column_scroll);
	state_save_register_global(machine, state->flipscreen_x);
	state_save_register_global(machine, state->flipscreen_y);
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

WRITE8_HANDLER( marineb_videoram_w )
{
	marineb_state *state = space->machine->driver_data<marineb_state>();

	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}


WRITE8_HANDLER( marineb_colorram_w )
{
	marineb_state *state = space->machine->driver_data<marineb_state>();

	state->colorram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}


WRITE8_HANDLER( marineb_column_scroll_w )
{
	marineb_state *state = space->machine->driver_data<marineb_state>();
	state->column_scroll = data;
}


WRITE8_HANDLER( marineb_palette_bank_0_w )
{
	marineb_state *state = space->machine->driver_data<marineb_state>();
	UINT8 old = state->palette_bank;

	state->palette_bank = (state->palette_bank & 0x02) | (data & 0x01);

	if (old != state->palette_bank)
	{
		tilemap_mark_all_tiles_dirty(state->bg_tilemap);
	}
}


WRITE8_HANDLER( marineb_palette_bank_1_w )
{
	marineb_state *state = space->machine->driver_data<marineb_state>();
	UINT8 old = state->palette_bank;

	state->palette_bank = (state->palette_bank & 0x01) | ((data & 0x01) << 1);

	if (old != state->palette_bank)
	{
		tilemap_mark_all_tiles_dirty(state->bg_tilemap);
	}
}


WRITE8_HANDLER( marineb_flipscreen_x_w )
{
	marineb_state *state = space->machine->driver_data<marineb_state>();

	state->flipscreen_x = data ^ state->marineb_active_low_flipscreen;
	tilemap_set_flip(state->bg_tilemap, (state->flipscreen_x ? TILEMAP_FLIPX : 0) | (state->flipscreen_y ? TILEMAP_FLIPY : 0));
}


WRITE8_HANDLER( marineb_flipscreen_y_w )
{
	marineb_state *state = space->machine->driver_data<marineb_state>();

	state->flipscreen_y = data ^ state->marineb_active_low_flipscreen;
	tilemap_set_flip(state->bg_tilemap, (state->flipscreen_x ? TILEMAP_FLIPX : 0) | (state->flipscreen_y ? TILEMAP_FLIPY : 0));
}



/*************************************
 *
 *  Video update
 *
 *************************************/

static void set_tilemap_scrolly( running_machine *machine, int cols )
{
	marineb_state *state = machine->driver_data<marineb_state>();
	int col;

	for (col = 0; col < cols; col++)
		tilemap_set_scrolly(state->bg_tilemap, col, state->column_scroll);

	for (; col < 32; col++)
		tilemap_set_scrolly(state->bg_tilemap, col, 0);
}


VIDEO_UPDATE( marineb )
{
	marineb_state *state = screen->machine->driver_data<marineb_state>();
	int offs;

	set_tilemap_scrolly(screen->machine, 24);
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);

	/* draw the sprites */
	for (offs = 0x0f; offs >= 0; offs--)
	{
		int gfx, sx, sy, code, col, flipx, flipy, offs2;

		if ((offs == 0) || (offs == 2))
			continue;  /* no sprites here */

		if (offs < 8)
			offs2 = 0x0018 + offs;
		else
			offs2 = 0x03d8 - 8 + offs;

		code = state->videoram[offs2];
		sx = state->videoram[offs2 + 0x20];
		sy = state->colorram[offs2];
		col = (state->colorram[offs2 + 0x20] & 0x0f) + 16 * state->palette_bank;
		flipx = code & 0x02;
		flipy = !(code & 0x01);

		if (offs < 4)
		{
			/* big sprite */
			gfx = 2;
			code = (code >> 4) | ((code & 0x0c) << 2);
		}
		else
		{
			/* small sprite */
			gfx = 1;
			code >>= 2;
		}

		if (!state->flipscreen_y)
		{
			sy = 256 - screen->machine->gfx[gfx]->width - sy;
			flipy = !flipy;
		}

		if (state->flipscreen_x)
		{
			sx++;
		}

		drawgfx_transpen(bitmap,cliprect,screen->machine->gfx[gfx],
				code,
				col,
				flipx,flipy,
				sx,sy,0);
	}
	return 0;
}


VIDEO_UPDATE( changes )
{
	marineb_state *state = screen->machine->driver_data<marineb_state>();
	int offs, sx, sy, code, col, flipx, flipy;

	set_tilemap_scrolly(screen->machine, 26);
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);

	/* draw the small sprites */
	for (offs = 0x05; offs >= 0; offs--)
	{
		int offs2;

		offs2 = 0x001a + offs;

		code = state->videoram[offs2];
		sx = state->videoram[offs2 + 0x20];
		sy = state->colorram[offs2];
		col = (state->colorram[offs2 + 0x20] & 0x0f) + 16 * state->palette_bank;
		flipx = code & 0x02;
		flipy = !(code & 0x01);

		if (!state->flipscreen_y)
		{
			sy = 256 - screen->machine->gfx[1]->width - sy;
			flipy = !flipy;
		}

		if (state->flipscreen_x)
		{
			sx++;
		}

		drawgfx_transpen(bitmap,cliprect,screen->machine->gfx[1],
				code >> 2,
				col,
				flipx,flipy,
				sx,sy,0);
	}

	/* draw the big sprite */

	code = state->videoram[0x3df];
	sx = state->videoram[0x3ff];
	sy = state->colorram[0x3df];
	col = state->colorram[0x3ff];
	flipx = code & 0x02;
	flipy = !(code & 0x01);

	if (!state->flipscreen_y)
	{
		sy = 256 - screen->machine->gfx[2]->width - sy;
		flipy = !flipy;
	}

	if (state->flipscreen_x)
	{
		sx++;
	}

	code >>= 4;

	drawgfx_transpen(bitmap,cliprect,screen->machine->gfx[2],
			code,
			col,
			flipx,flipy,
			sx,sy,0);

	/* draw again for wrap around */

	drawgfx_transpen(bitmap,cliprect,screen->machine->gfx[2],
			code,
			col,
			flipx,flipy,
			sx-256,sy,0);
	return 0;
}


VIDEO_UPDATE( springer )
{
	marineb_state *state = screen->machine->driver_data<marineb_state>();
	int offs;

	set_tilemap_scrolly(screen->machine, 0);
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);

	/* draw the sprites */
	for (offs = 0x0f; offs >= 0; offs--)
	{
		int gfx, sx, sy, code, col, flipx, flipy, offs2;

		if ((offs == 0) || (offs == 2))
			continue;  /* no sprites here */

		offs2 = 0x0010 + offs;

		code = state->videoram[offs2];
		sx = 240 - state->videoram[offs2 + 0x20];
		sy = state->colorram[offs2];
		col = (state->colorram[offs2 + 0x20] & 0x0f) + 16 * state->palette_bank;
		flipx = !(code & 0x02);
		flipy = !(code & 0x01);

		if (offs < 4)
		{
			/* big sprite */
			sx -= 0x10;
			gfx = 2;
			code = (code >> 4) | ((code & 0x0c) << 2);
		}
		else
		{
			/* small sprite */
			gfx = 1;
			code >>= 2;
		}

		if (!state->flipscreen_y)
		{
			sy = 256 - screen->machine->gfx[gfx]->width - sy;
			flipy = !flipy;
		}

		if (!state->flipscreen_x)
		{
			sx--;
		}

		drawgfx_transpen(bitmap,cliprect,screen->machine->gfx[gfx],
				code,
				col,
				flipx,flipy,
				sx,sy,0);
	}
	return 0;
}


VIDEO_UPDATE( hoccer )
{
	marineb_state *state = screen->machine->driver_data<marineb_state>();
	int offs;

	set_tilemap_scrolly(screen->machine, 0);
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);

	/* draw the sprites */
	for (offs = 0x07; offs >= 0; offs--)
	{
		int sx, sy, code, col, flipx, flipy, offs2;

		offs2 = 0x0018 + offs;

		code = state->spriteram[offs2];
		sx = state->spriteram[offs2 + 0x20];
		sy = state->colorram[offs2];
		col = state->colorram[offs2 + 0x20];
		flipx = code & 0x02;
		flipy = !(code & 0x01);

		if (!state->flipscreen_y)
		{
			sy = 256 - screen->machine->gfx[1]->width - sy;
			flipy = !flipy;
		}

		if (state->flipscreen_x)
		{
			sx = 256 - screen->machine->gfx[1]->width - sx;
			flipx = !flipx;
		}

		drawgfx_transpen(bitmap,cliprect,screen->machine->gfx[1],
				code >> 2,
				col,
				flipx,flipy,
				sx,sy,0);
	}
	return 0;
}


VIDEO_UPDATE( hopprobo )
{
	marineb_state *state = screen->machine->driver_data<marineb_state>();
	int offs;

	set_tilemap_scrolly(screen->machine, 0);
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);

	/* draw the sprites */
	for (offs = 0x0f; offs >= 0; offs--)
	{
		int gfx, sx, sy, code, col, flipx, flipy, offs2;

		if ((offs == 0) || (offs == 2))
			continue;  /* no sprites here */

		offs2 = 0x0010 + offs;

		code = state->videoram[offs2];
		sx = state->videoram[offs2 + 0x20];
		sy = state->colorram[offs2];
		col = (state->colorram[offs2 + 0x20] & 0x0f) + 16 * state->palette_bank;
		flipx = code & 0x02;
		flipy = !(code & 0x01);

		if (offs < 4)
		{
			/* big sprite */
			gfx = 2;
			code = (code >> 4) | ((code & 0x0c) << 2);
		}
		else
		{
			/* small sprite */
			gfx = 1;
			code >>= 2;
		}

		if (!state->flipscreen_y)
		{
			sy = 256 - screen->machine->gfx[gfx]->width - sy;
			flipy = !flipy;
		}

		if (!state->flipscreen_x)
		{
			sx--;
		}

		drawgfx_transpen(bitmap,cliprect,screen->machine->gfx[gfx],
				code,
				col,
				flipx,flipy,
				sx,sy,0);
	}
	return 0;
}
