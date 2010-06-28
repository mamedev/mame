/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/firetrap.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Fire Trap has one 256x8 and one 256x4 palette PROMs.
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

PALETTE_INIT( firetrap )
{
	int i;


	for (i = 0; i < machine->total_colors(); i++)
	{
		int bit0, bit1, bit2, bit3, r, g, b;


		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		bit3 = (color_prom[i] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[i] >> 4) & 0x01;
		bit1 = (color_prom[i] >> 5) & 0x01;
		bit2 = (color_prom[i] >> 6) & 0x01;
		bit3 = (color_prom[i] >> 7) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[i + machine->total_colors()] >> 0) & 0x01;
		bit1 = (color_prom[i + machine->total_colors()] >> 1) & 0x01;
		bit2 = (color_prom[i + machine->total_colors()] >> 2) & 0x01;
		bit3 = (color_prom[i + machine->total_colors()] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette_set_color(machine, i, MAKE_RGB(r,g,b));
	}
}


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILEMAP_MAPPER( get_fg_memory_offset )
{
	return (row ^ 0x1f) + (col << 5);
}

static TILEMAP_MAPPER( get_bg_memory_offset )
{
	return ((row & 0x0f) ^ 0x0f) | ((col & 0x0f) << 4) |
			/* hole at bit 8 */
			((row & 0x10) << 5) | ((col & 0x10) << 6);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	firetrap_state *state = (firetrap_state *)machine->driver_data;
	int code = state->fgvideoram[tile_index];
	int color = state->fgvideoram[tile_index + 0x400];
	SET_TILE_INFO(
			0,
			code | ((color & 0x01) << 8),
			color >> 4,
			0);
}

INLINE void get_bg_tile_info(running_machine *machine, tile_data *tileinfo, int tile_index, UINT8 *bgvideoram, int gfx_region)
{
	int code = bgvideoram[tile_index];
	int color = bgvideoram[tile_index + 0x100];
	SET_TILE_INFO(
			gfx_region,
			code + ((color & 0x03) << 8),
			(color & 0x30) >> 4,
			TILE_FLIPXY((color & 0x0c) >> 2));
}

static TILE_GET_INFO( get_bg1_tile_info )
{
	firetrap_state *state = (firetrap_state *)machine->driver_data;
	get_bg_tile_info(machine, tileinfo, tile_index, state->bg1videoram, 1);
}

static TILE_GET_INFO( get_bg2_tile_info )
{
	firetrap_state *state = (firetrap_state *)machine->driver_data;
	get_bg_tile_info(machine, tileinfo, tile_index, state->bg2videoram, 2);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( firetrap )
{
	firetrap_state *state = (firetrap_state *)machine->driver_data;
	state->fg_tilemap  = tilemap_create(machine, get_fg_tile_info, get_fg_memory_offset, 8, 8, 32, 32);
	state->bg1_tilemap = tilemap_create(machine, get_bg1_tile_info, get_bg_memory_offset, 16, 16, 32, 32);
	state->bg2_tilemap = tilemap_create(machine, get_bg2_tile_info, get_bg_memory_offset, 16, 16, 32, 32);

	tilemap_set_transparent_pen(state->fg_tilemap, 0);
	tilemap_set_transparent_pen(state->bg1_tilemap, 0);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( firetrap_fgvideoram_w )
{
	firetrap_state *state = (firetrap_state *)space->machine->driver_data;
	state->fgvideoram[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap, offset & 0x3ff);
}

WRITE8_HANDLER( firetrap_bg1videoram_w )
{
	firetrap_state *state = (firetrap_state *)space->machine->driver_data;
	state->bg1videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg1_tilemap, offset & 0x6ff);
}

WRITE8_HANDLER( firetrap_bg2videoram_w )
{
	firetrap_state *state = (firetrap_state *)space->machine->driver_data;
	state->bg2videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg2_tilemap, offset & 0x6ff);
}


WRITE8_HANDLER( firetrap_bg1_scrollx_w )
{
	firetrap_state *state = (firetrap_state *)space->machine->driver_data;
	state->scroll1_x[offset] = data;
	tilemap_set_scrollx(state->bg1_tilemap, 0, state->scroll1_x[0] | (state->scroll1_x[1] << 8));
}

WRITE8_HANDLER( firetrap_bg1_scrolly_w )
{
	firetrap_state *state = (firetrap_state *)space->machine->driver_data;
	state->scroll1_y[offset] = data;
	tilemap_set_scrolly(state->bg1_tilemap, 0, -(state->scroll1_y[0] | (state->scroll1_y[1] << 8)));
}

WRITE8_HANDLER( firetrap_bg2_scrollx_w )
{
	firetrap_state *state = (firetrap_state *)space->machine->driver_data;
	state->scroll2_x[offset] = data;
	tilemap_set_scrollx(state->bg2_tilemap, 0, state->scroll2_x[0] | (state->scroll2_x[1] << 8));
}

WRITE8_HANDLER( firetrap_bg2_scrolly_w )
{
	firetrap_state *state = (firetrap_state *)space->machine->driver_data;
	state->scroll2_y[offset] = data;
	tilemap_set_scrolly(state->bg2_tilemap, 0, -(state->scroll2_y[0] | (state->scroll2_y[1] << 8)));
}


/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	firetrap_state *state = (firetrap_state *)machine->driver_data;
	int offs;

	for (offs = 0; offs < state->spriteram_size; offs += 4)
	{
		int sx, sy, flipx, flipy, code, color;


		/* the meaning of bit 3 of [offs] is unknown */

		sy = state->spriteram[offs];
		sx = state->spriteram[offs + 2];
		code = state->spriteram[offs + 3] + 4 * (state->spriteram[offs + 1] & 0xc0);
		color = ((state->spriteram[offs + 1] & 0x08) >> 2) | (state->spriteram[offs + 1] & 0x01);
		flipx = state->spriteram[offs + 1] & 0x04;
		flipy = state->spriteram[offs + 1] & 0x02;
		if (flip_screen_get(machine))
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		if (state->spriteram[offs + 1] & 0x10)	/* double width */
		{
			if (flip_screen_get(machine)) sy -= 16;

			drawgfx_transpen(bitmap,cliprect,machine->gfx[3],
					code & ~1,
					color,
					flipx,flipy,
					sx,flipy ? sy : sy + 16,0);
			drawgfx_transpen(bitmap,cliprect,machine->gfx[3],
					code | 1,
					color,
					flipx,flipy,
					sx,flipy ? sy + 16 : sy,0);

			/* redraw with wraparound */
			drawgfx_transpen(bitmap,cliprect,machine->gfx[3],
					code & ~1,
					color,
					flipx,flipy,
					sx - 256,flipy ? sy : sy + 16,0);
			drawgfx_transpen(bitmap,cliprect,machine->gfx[3],
					code | 1,
					color,
					flipx,flipy,
					sx - 256,flipy ? sy + 16 : sy,0);
		}
		else
		{
			drawgfx_transpen(bitmap,cliprect,machine->gfx[3],
					code,
					color,
					flipx,flipy,
					sx,sy,0);

			/* redraw with wraparound */
			drawgfx_transpen(bitmap,cliprect,machine->gfx[3],
					code,
					color,
					flipx,flipy,
					sx - 256,sy,0);
		}
	}
}

VIDEO_UPDATE( firetrap )
{
	firetrap_state *state = (firetrap_state *)screen->machine->driver_data;
	tilemap_draw(bitmap, cliprect, state->bg2_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, state->bg1_tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);
	return 0;
}
