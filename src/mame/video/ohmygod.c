#include "emu.h"
#include "includes/ohmygod.h"

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_tile_info )
{
	ohmygod_state *state = (ohmygod_state *)machine->driver_data;
	UINT16 code = state->videoram[2 * tile_index + 1];
	UINT16 attr = state->videoram[2 * tile_index];
	SET_TILE_INFO(
			0,
			code,
			(attr & 0x0f00) >> 8,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( ohmygod )
{
	ohmygod_state *state = (ohmygod_state *)machine->driver_data;
	state->bg_tilemap = tilemap_create(machine, get_tile_info, tilemap_scan_rows, 8, 8, 64, 64);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE16_HANDLER( ohmygod_videoram_w )
{
	ohmygod_state *state = (ohmygod_state *)space->machine->driver_data;
	COMBINE_DATA(&state->videoram[offset]);
	tilemap_mark_tile_dirty(state->bg_tilemap, offset / 2);
}

WRITE16_HANDLER( ohmygod_spritebank_w )
{
	ohmygod_state *state = (ohmygod_state *)space->machine->driver_data;
	if (ACCESSING_BITS_8_15)
		state->spritebank = data & 0x8000;
}

WRITE16_HANDLER( ohmygod_scrollx_w )
{
	ohmygod_state *state = (ohmygod_state *)space->machine->driver_data;
	COMBINE_DATA(&state->scrollx);
	tilemap_set_scrollx(state->bg_tilemap, 0, state->scrollx - 0x81ec);
}

WRITE16_HANDLER( ohmygod_scrolly_w )
{
	ohmygod_state *state = (ohmygod_state *)space->machine->driver_data;
	COMBINE_DATA(&state->scrolly);
	tilemap_set_scrolly(state->bg_tilemap, 0, state->scrolly - 0x81ef);
}


/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	ohmygod_state *state = (ohmygod_state *)machine->driver_data;
	UINT16 *spriteram = state->spriteram;
	int offs;

	for (offs = 0; offs < state->spriteram_size / 4; offs += 4)
	{
		int sx, sy, code, color, flipx;
		UINT16 *sr;

		sr = state->spritebank ? (spriteram + state->spriteram_size / 4) : spriteram;

		code = sr[offs + 3] & 0x0fff;
		color = sr[offs + 2] & 0x000f;
		sx = sr[offs + 0] - 29;
		sy = sr[offs + 1];
		if (sy >= 32768)
			sy -= 65536;
		flipx = sr[offs + 3] & 0x8000;

		drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
				code,
				color,
				flipx,0,
				sx,sy,0);
	}
}

VIDEO_UPDATE( ohmygod )
{
	ohmygod_state *state = (ohmygod_state *)screen->machine->driver_data;

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}
