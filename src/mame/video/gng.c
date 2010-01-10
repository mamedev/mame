/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/gng.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_fg_tile_info )
{
	gng_state *state = (gng_state *)machine->driver_data;
	UINT8 attr = state->fgvideoram[tile_index + 0x400];
	SET_TILE_INFO(
			0,
			state->fgvideoram[tile_index] + ((attr & 0xc0) << 2),
			attr & 0x0f,
			TILE_FLIPYX((attr & 0x30) >> 4));
}

static TILE_GET_INFO( get_bg_tile_info )
{
	gng_state *state = (gng_state *)machine->driver_data;
	UINT8 attr = state->bgvideoram[tile_index + 0x400];
	SET_TILE_INFO(
			1,
			state->bgvideoram[tile_index] + ((attr & 0xc0) << 2),
			attr & 0x07,
			TILE_FLIPYX((attr & 0x30) >> 4));
	tileinfo->group = (attr & 0x08) >> 3;
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( gng )
{
	gng_state *state = (gng_state *)machine->driver_data;
	state->fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_cols, 16, 16, 32, 32);

	tilemap_set_transparent_pen(state->fg_tilemap, 3);
	tilemap_set_transmask(state->bg_tilemap, 0, 0xff, 0x00); /* split type 0 is totally transparent in front half */
	tilemap_set_transmask(state->bg_tilemap, 1, 0x41, 0xbe); /* split type 1 has pens 0 and 6 transparent in front half */
}


/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( gng_fgvideoram_w )
{
	gng_state *state = (gng_state *)space->machine->driver_data;
	state->fgvideoram[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap, offset & 0x3ff);
}

WRITE8_HANDLER( gng_bgvideoram_w )
{
	gng_state *state = (gng_state *)space->machine->driver_data;
	state->bgvideoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset & 0x3ff);
}


WRITE8_HANDLER( gng_bgscrollx_w )
{
	gng_state *state = (gng_state *)space->machine->driver_data;
	state->scrollx[offset] = data;
	tilemap_set_scrollx(state->bg_tilemap, 0, state->scrollx[0] + 256 * state->scrollx[1]);
}

WRITE8_HANDLER( gng_bgscrolly_w )
{
	gng_state *state = (gng_state *)space->machine->driver_data;
	state->scrolly[offset] = data;
	tilemap_set_scrolly(state->bg_tilemap, 0, state->scrolly[0] + 256 * state->scrolly[1]);
}


WRITE8_HANDLER( gng_flipscreen_w )
{
	flip_screen_set(space->machine, ~data & 1);
}



/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	UINT8 *buffered_spriteram = machine->generic.buffered_spriteram.u8;
	const gfx_element *gfx = machine->gfx[2];
	int offs;


	for (offs = machine->generic.spriteram_size - 4; offs >= 0; offs -= 4)
	{
		UINT8 attributes = buffered_spriteram[offs + 1];
		int sx = buffered_spriteram[offs + 3] - 0x100 * (attributes & 0x01);
		int sy = buffered_spriteram[offs + 2];
		int flipx = attributes & 0x04;
		int flipy = attributes & 0x08;

		if (flip_screen_get(machine))
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap,cliprect,gfx,
				buffered_spriteram[offs] + ((attributes << 2) & 0x300),
				(attributes >> 4) & 3,
				flipx,flipy,
				sx,sy,15);
	}
}

VIDEO_UPDATE( gng )
{
	gng_state *state = (gng_state *)screen->machine->driver_data;

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_LAYER1, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_LAYER0, 0);
	tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);
	return 0;
}

VIDEO_EOF( gng )
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);

	buffer_spriteram_w(space, 0, 0);
}
