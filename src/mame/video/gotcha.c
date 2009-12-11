#include "driver.h"
#include "includes/gotcha.h"

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILEMAP_MAPPER( gotcha_tilemap_scan )
{
	return (col & 0x1f) | (row << 5) | ((col & 0x20) << 5);
}

INLINE void get_tile_info( running_machine *machine, tile_data *tileinfo, int tile_index ,UINT16 *vram, int color_offs)
{
	gotcha_state *state = (gotcha_state *)machine->driver_data;
	UINT16 data = vram[tile_index];
	int code = (data & 0x3ff) | (state->gfxbank[(data & 0x0c00) >> 10] << 10);

	SET_TILE_INFO(0, code, (data >> 12) + color_offs, 0);
}

static TILE_GET_INFO( fg_get_tile_info )
{
	gotcha_state *state = (gotcha_state *)machine->driver_data;
	get_tile_info(machine, tileinfo, tile_index, state->fgvideoram, 0);
}

static TILE_GET_INFO( bg_get_tile_info )
{
	gotcha_state *state = (gotcha_state *)machine->driver_data;
	get_tile_info(machine, tileinfo, tile_index, state->bgvideoram, 16);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( gotcha )
{
	gotcha_state *state = (gotcha_state *)machine->driver_data;
	state->fg_tilemap = tilemap_create(machine, fg_get_tile_info, gotcha_tilemap_scan, 16, 16, 64, 32);
	state->bg_tilemap = tilemap_create(machine, bg_get_tile_info, gotcha_tilemap_scan, 16, 16, 64, 32);

	tilemap_set_transparent_pen(state->fg_tilemap, 0);

	tilemap_set_scrolldx(state->fg_tilemap, -1, 0);
	tilemap_set_scrolldx(state->bg_tilemap, -5, 0);
}


WRITE16_HANDLER( gotcha_fgvideoram_w )
{
	gotcha_state *state = (gotcha_state *)space->machine->driver_data;
	COMBINE_DATA(&state->fgvideoram[offset]);
	tilemap_mark_tile_dirty(state->fg_tilemap, offset);
}

WRITE16_HANDLER( gotcha_bgvideoram_w )
{
	gotcha_state *state = (gotcha_state *)space->machine->driver_data;
	COMBINE_DATA(&state->bgvideoram[offset]);
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

WRITE16_HANDLER( gotcha_gfxbank_select_w )
{
	gotcha_state *state = (gotcha_state *)space->machine->driver_data;
	if (ACCESSING_BITS_8_15)
		state->banksel = (data & 0x0300) >> 8;
}

WRITE16_HANDLER( gotcha_gfxbank_w )
{
	gotcha_state *state = (gotcha_state *)space->machine->driver_data;
	if (ACCESSING_BITS_8_15)
	{
		if (state->gfxbank[state->banksel] != ((data & 0x0f00) >> 8))
		{
			state->gfxbank[state->banksel] = (data & 0x0f00) >> 8;
			tilemap_mark_all_tiles_dirty_all(space->machine);
		}
	}
}

WRITE16_HANDLER( gotcha_scroll_w )
{
	gotcha_state *state = (gotcha_state *)space->machine->driver_data;
	COMBINE_DATA(&state->scroll[offset]);

	switch (offset)
	{
		case 0: tilemap_set_scrollx(state->fg_tilemap, 0, state->scroll[0]); break;
		case 1: tilemap_set_scrolly(state->fg_tilemap, 0, state->scroll[1]); break;
		case 2: tilemap_set_scrollx(state->bg_tilemap, 0, state->scroll[2]); break;
		case 3: tilemap_set_scrolly(state->bg_tilemap, 0, state->scroll[3]); break;
	}
}




static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	gotcha_state *state = (gotcha_state *)machine->driver_data;
	UINT16 *spriteram = state->spriteram;
	int offs;

	for (offs = 0; offs < state->spriteram_size / 2; offs += 4)
	{
		int sx, sy, code, color, flipx, flipy, height, y;

		sx = spriteram[offs + 2];
		sy = spriteram[offs + 0];
		code = spriteram[offs + 1];
		color = spriteram[offs + 2] >> 9;
		height = 1 << ((spriteram[offs + 0] & 0x0600) >> 9);
		flipx = spriteram[offs + 0] & 0x2000;
		flipy = spriteram[offs + 0] & 0x4000;

		for (y = 0; y < height; y++)
		{
			drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
					code + (flipy ? height-1 - y : y),
					color,
					flipx,flipy,
					0x140-5 - ((sx + 0x10) & 0x1ff),0x100+1 - ((sy + 0x10 * (height - y)) & 0x1ff),0);
		}
	}
}


VIDEO_UPDATE( gotcha )
{
	gotcha_state *state = (gotcha_state *)screen->machine->driver_data;
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}
