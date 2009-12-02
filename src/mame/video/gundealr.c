/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"
#include "includes/gundealr.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_bg_tile_info )
{
	gundealr_state *state = (gundealr_state *)machine->driver_data;
	UINT8 attr = state->bg_videoram[2 * tile_index + 1];
	SET_TILE_INFO(
			0,
			state->bg_videoram[2 * tile_index] + ((attr & 0x07) << 8),
			(attr & 0xf0) >> 4,
			0);
}

static TILEMAP_MAPPER( gundealr_scan )
{
	/* logical (col,row) -> memory offset */
	return (row & 0x0f) + ((col & 0x3f) << 4) + ((row & 0x10) << 6);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	gundealr_state *state = (gundealr_state *)machine->driver_data;
	UINT8 attr = state->fg_videoram[2 * tile_index + 1];
	SET_TILE_INFO(
			1,
			state->fg_videoram[2 * tile_index] + ((attr & 0x03) << 8),
			(attr & 0xf0) >> 4,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( gundealr )
{
	gundealr_state *state = (gundealr_state *)machine->driver_data;
	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_cols, 8, 8, 32, 32);
	state->fg_tilemap = tilemap_create(machine, get_fg_tile_info, gundealr_scan, 16, 16, 64, 32);

	tilemap_set_transparent_pen(state->fg_tilemap, 15);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( gundealr_bg_videoram_w )
{
	gundealr_state *state = (gundealr_state *)space->machine->driver_data;
	state->bg_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset / 2);
}

WRITE8_HANDLER( gundealr_fg_videoram_w )
{
	gundealr_state *state = (gundealr_state *)space->machine->driver_data;
	state->fg_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap, offset / 2);
}

WRITE8_HANDLER( gundealr_paletteram_w )
{
	gundealr_state *state = (gundealr_state *)space->machine->driver_data;
	int r,g,b,val;

	state->paletteram[offset] = data;

	val = state->paletteram[offset & ~1];
	r = (val >> 4) & 0x0f;
	g = (val >> 0) & 0x0f;

	val = state->paletteram[offset | 1];
	b = (val >> 4) & 0x0f;
	/* TODO: the bottom 4 bits are used as well, but I'm not sure about the meaning */

	palette_set_color_rgb(space->machine, offset / 2, pal4bit(r), pal4bit(g), pal4bit(b));
}

WRITE8_HANDLER( gundealr_fg_scroll_w )
{
	gundealr_state *state = (gundealr_state *)space->machine->driver_data;
	state->scroll[offset] = data;
	tilemap_set_scrollx(state->fg_tilemap, 0, state->scroll[1] | ((state->scroll[0] & 0x03) << 8));
	tilemap_set_scrolly(state->fg_tilemap, 0, state->scroll[3] | ((state->scroll[2] & 0x03) << 8));
}

WRITE8_HANDLER( yamyam_fg_scroll_w )
{
	gundealr_state *state = (gundealr_state *)space->machine->driver_data;
	state->scroll[offset] = data;
	tilemap_set_scrollx(state->fg_tilemap, 0, state->scroll[0] | ((state->scroll[1] & 0x03) << 8));
	tilemap_set_scrolly(state->fg_tilemap, 0, state->scroll[2] | ((state->scroll[3] & 0x03) << 8));
}

WRITE8_HANDLER( gundealr_flipscreen_w )
{
	gundealr_state *state = (gundealr_state *)space->machine->driver_data;
	state->flipscreen = data;
	tilemap_set_flip_all(space->machine, state->flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
}



/***************************************************************************

  Display refresh

***************************************************************************/

VIDEO_UPDATE( gundealr )
{
	gundealr_state *state = (gundealr_state *)screen->machine->driver_data;
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);
	return 0;
}
