/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/citycon.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILEMAP_MAPPER( citycon_scan )
{
	/* logical (col,row) -> memory offset */
	return (col & 0x1f) + ((row & 0x1f) << 5) + ((col & 0x60) << 5);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	citycon_state *state = (citycon_state *)machine->driver_data;
	SET_TILE_INFO(
			0,
			state->videoram[tile_index],
			(tile_index & 0x03e0) >> 5,	/* color depends on scanline only */
			0);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	citycon_state *state = (citycon_state *)machine->driver_data;
	UINT8 *rom = memory_region(machine, "gfx4");
	int code = rom[0x1000 * state->bg_image + tile_index];
	SET_TILE_INFO(
			3 + state->bg_image,
			code,
			rom[0xc000 + 0x100 * state->bg_image + code],
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( citycon )
{
	citycon_state *state = (citycon_state *)machine->driver_data;
	state->fg_tilemap = tilemap_create(machine, get_fg_tile_info, citycon_scan, 8, 8, 128, 32);
	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, citycon_scan, 8, 8, 128, 32);

	tilemap_set_transparent_pen(state->fg_tilemap, 0);
	tilemap_set_scroll_rows(state->fg_tilemap, 32);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( citycon_videoram_w )
{
	citycon_state *state = (citycon_state *)space->machine->driver_data;
	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap, offset);
}


WRITE8_HANDLER( citycon_linecolor_w )
{
	citycon_state *state = (citycon_state *)space->machine->driver_data;
	state->linecolor[offset] = data;
}


WRITE8_HANDLER( citycon_background_w )
{
	citycon_state *state = (citycon_state *)space->machine->driver_data;

	/* bits 4-7 control the background image */
	if (state->bg_image != (data >> 4))
	{
		state->bg_image = (data >> 4);
		tilemap_mark_all_tiles_dirty(state->bg_tilemap);
	}

	/* bit 0 flips screen */
	/* it is also used to multiplex player 1 and player 2 controls */
	flip_screen_set(space->machine, data & 0x01);

	/* bits 1-3 are unknown */
//  if ((data & 0x0e) != 0) logerror("background register = %02x\n", data);
}



static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	citycon_state *state = (citycon_state *)machine->driver_data;
	int offs;

	for (offs = state->spriteram_size - 4; offs >= 0; offs -= 4)
	{
		int sx, sy, flipx;

		sx = state->spriteram[offs + 3];
		sy = 239 - state->spriteram[offs];
		flipx = ~state->spriteram[offs + 2] & 0x10;
		if (flip_screen_get(machine))
		{
			sx = 240 - sx;
			sy = 238 - sy;
			flipx = !flipx;
		}

		drawgfx_transpen(bitmap, cliprect, machine->gfx[state->spriteram[offs + 1] & 0x80 ? 2 : 1],
				state->spriteram[offs + 1] & 0x7f,
				state->spriteram[offs + 2] & 0x0f,
				flipx,flip_screen_get(machine),
				sx, sy, 0);
	}
}


INLINE void changecolor_RRRRGGGGBBBBxxxx( running_machine *machine, int color, int indx )
{
	int data = machine->generic.paletteram.u8[2 * indx | 1] | (machine->generic.paletteram.u8[2 * indx] << 8);
	palette_set_color_rgb(machine, color, pal4bit(data >> 12), pal4bit(data >> 8), pal4bit(data >> 4));
}

VIDEO_UPDATE( citycon )
{
	citycon_state *state = (citycon_state *)screen->machine->driver_data;
	int offs, scroll;

	/* Update the virtual palette to support text color code changing on every scanline. */
	for (offs = 0; offs < 256; offs++)
	{
		int indx = state->linecolor[offs];
		int i;

		for (i = 0; i < 4; i++)
			changecolor_RRRRGGGGBBBBxxxx(screen->machine, 640 + 4 * offs + i, 512 + 4 * indx + i);
	}


	scroll = state->scroll[0] * 256 + state->scroll[1];
	tilemap_set_scrollx(state->bg_tilemap, 0, scroll >> 1);
	for (offs = 6; offs < 32; offs++)
		tilemap_set_scrollx(state->fg_tilemap, offs, scroll);

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}
