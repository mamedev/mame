/***************************************************************************

    video/brkthru.c

***************************************************************************/

#include "emu.h"
#include "includes/brkthru.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Break Thru has one 256x8 and one 256x4 palette PROMs.
  I don't know for sure how the palette PROMs are connected to the RGB
  output, but it's probably the usual:

  bit 7 -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 2.2kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
        -- 1  kohm resistor  -- RED
  bit 0 -- 2.2kohm resistor  -- RED

  bit 3 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 1  kohm resistor  -- BLUE
  bit 0 -- 2.2kohm resistor  -- BLUE

***************************************************************************/

PALETTE_INIT( brkthru )
{
	int i;

	for (i = 0; i < machine->total_colors(); i++)
	{
		int bit0, bit1, bit2, bit3, r, g, b;

		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		bit3 = (color_prom[0] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[0] >> 4) & 0x01;
		bit1 = (color_prom[0] >> 5) & 0x01;
		bit2 = (color_prom[0] >> 6) & 0x01;
		bit3 = (color_prom[0] >> 7) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[machine->total_colors()] >> 0) & 0x01;
		bit1 = (color_prom[machine->total_colors()] >> 1) & 0x01;
		bit2 = (color_prom[machine->total_colors()] >> 2) & 0x01;
		bit3 = (color_prom[machine->total_colors()] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette_set_color(machine, i, MAKE_RGB(r,g,b));

		color_prom++;
	}
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

static TILE_GET_INFO( get_bg_tile_info )
{
	brkthru_state *state = (brkthru_state *)machine->driver_data;
	/* BG RAM format
        0         1
        ---- -c-- ---- ---- = Color
        ---- --xx xxxx xxxx = Code
    */

	int code = (state->videoram[tile_index * 2] | ((state->videoram[tile_index * 2 + 1]) << 8)) & 0x3ff;
	int region = 1 + (code >> 7);
	int colour = state->bgbasecolor + ((state->videoram[tile_index * 2 + 1] & 0x04) >> 2);

	SET_TILE_INFO(region, code & 0x7f, colour,0);
}

WRITE8_HANDLER( brkthru_bgram_w )
{
	brkthru_state *state = (brkthru_state *)space->machine->driver_data;

	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset / 2);
}


static TILE_GET_INFO( get_fg_tile_info )
{
	brkthru_state *state = (brkthru_state *)machine->driver_data;
	UINT8 code = state->fg_videoram[tile_index];
	SET_TILE_INFO(0, code, 0, 0);
}

WRITE8_HANDLER( brkthru_fgram_w )
{
	brkthru_state *state = (brkthru_state *)space->machine->driver_data;

	state->fg_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap,offset);
}

VIDEO_START( brkthru )
{
	brkthru_state *state = (brkthru_state *)machine->driver_data;

	state->fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_cols, 16, 16, 32, 16);

	tilemap_set_transparent_pen(state->fg_tilemap, 0);
	tilemap_set_transparent_pen(state->bg_tilemap, 0);
}


WRITE8_HANDLER( brkthru_1800_w )
{
	brkthru_state *state = (brkthru_state *)space->machine->driver_data;

	if (offset == 0)	/* low 8 bits of scroll */
		state->bgscroll = (state->bgscroll & 0x100) | data;
	else if (offset == 1)
	{
		/* bit 0-2 = ROM bank select */
		memory_set_bank(space->machine, "bank1", data & 0x07);

		/* bit 3-5 = background tiles color code */
		if (((data & 0x38) >> 2) != state->bgbasecolor)
		{
			state->bgbasecolor = (data & 0x38) >> 2;
			tilemap_mark_all_tiles_dirty(state->bg_tilemap);
		}

		/* bit 6 = screen flip */
		if (state->flipscreen != (data & 0x40))
		{
			state->flipscreen = data & 0x40;
			tilemap_set_flip(state->bg_tilemap, state->flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
			tilemap_set_flip(state->fg_tilemap, state->flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

		}

		/* bit 7 = high bit of scroll */
		state->bgscroll = (state->bgscroll & 0xff) | ((data & 0x80) << 1);
	}
}


#if 0
static void show_register( bitmap_t *bitmap, int x, int y, UINT32 data )
{
	char buf[5];

	sprintf(buf, "%04X", data);
	ui_draw_text(y, x, buf);
}
#endif


static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int prio )
{
	brkthru_state *state = (brkthru_state *)machine->driver_data;
	int offs;
	/* Draw the sprites. Note that it is important to draw them exactly in this */
	/* order, to have the correct priorities. */

	/* Sprite RAM format
        0         1         2         3
        ccc- ---- ---- ---- ---- ---- ---- ---- = Color
        ---d ---- ---- ---- ---- ---- ---- ---- = Double Size
        ---- p--- ---- ---- ---- ---- ---- ---- = Priority
        ---- -bb- ---- ---- ---- ---- ---- ---- = Bank
        ---- ---e ---- ---- ---- ---- ---- ---- = Enable/Disable
        ---- ---- ssss ssss ---- ---- ---- ---- = Sprite code
        ---- ---- ---- ---- yyyy yyyy ---- ---- = Y position
        ---- ---- ---- ---- ---- ---- xxxx xxxx = X position
    */

	for (offs = 0;offs < state->spriteram_size; offs += 4)
	{
		if ((state->spriteram[offs] & 0x09) == prio)	/* Enable && Low Priority */
		{
			int sx, sy, code, color;

			sx = 240 - state->spriteram[offs + 3];
			if (sx < -7)
				sx += 256;

			sy = 240 - state->spriteram[offs + 2];
			code = state->spriteram[offs + 1] + 128 * (state->spriteram[offs] & 0x06);
			color = (state->spriteram[offs] & 0xe0) >> 5;
			if (state->flipscreen)
			{
				sx = 240 - sx;
				sy = 240 - sy;
			}

			if (state->spriteram[offs] & 0x10)	/* double height */
			{
				drawgfx_transpen(bitmap,cliprect,machine->gfx[9],
						code & ~1,
						color,
						state->flipscreen, state->flipscreen,
						sx, state->flipscreen ? sy + 16 : sy - 16,0);
				drawgfx_transpen(bitmap,cliprect,machine->gfx[9],
						code | 1,
						color,
						state->flipscreen, state->flipscreen,
						sx,sy,0);

				/* redraw with wraparound */
				drawgfx_transpen(bitmap,cliprect,machine->gfx[9],
						code & ~1,
						color,
						state->flipscreen, state->flipscreen,
						sx,(state->flipscreen ? sy + 16 : sy - 16) + 256,0);
				drawgfx_transpen(bitmap,cliprect,machine->gfx[9],
						code | 1,
						color,
						state->flipscreen, state->flipscreen,
						sx,sy + 256,0);

			}
			else
			{
				drawgfx_transpen(bitmap,cliprect,machine->gfx[9],
						code,
						color,
						state->flipscreen, state->flipscreen,
						sx,sy,0);

				/* redraw with wraparound */
				drawgfx_transpen(bitmap,cliprect,machine->gfx[9],
						code,
						color,
						state->flipscreen, state->flipscreen,
						sx,sy + 256,0);

			}
		}
	}
}

VIDEO_UPDATE( brkthru )
{
	brkthru_state *state = (brkthru_state *)screen->machine->driver_data;

	tilemap_set_scrollx(state->bg_tilemap, 0, state->bgscroll);
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_OPAQUE, 0);

	/* low priority sprites */
	draw_sprites(screen->machine, bitmap, cliprect, 0x01);

	/* draw background over low priority sprites */
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);

	/* high priority sprites */
	draw_sprites(screen->machine, bitmap, cliprect, 0x09);

	/* fg layer */
	tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);

/*  show_register(bitmap, 8, 8, (UINT32)state->flipscreen); */

	return 0;
}
