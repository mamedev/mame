#include "emu.h"
#include "video/konicdev.h"
#include "includes/tail2nos.h"


#define TOTAL_CHARS 0x400

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_tile_info )
{
	tail2nos_state *state = machine->driver_data<tail2nos_state>();
	UINT16 code = state->bgvideoram[tile_index];
	SET_TILE_INFO(
			0,
			(code & 0x1fff) + (state->charbank << 13),
			((code & 0xe000) >> 13) + state->charpalette * 16,
			0);
}


/***************************************************************************

  Callbacks for the K051316

***************************************************************************/

void tail2nos_zoom_callback( running_machine *machine, int *code, int *color, int *flags )
{
	*code |= ((*color & 0x03) << 8);
	*color = 32 + ((*color & 0x38) >> 3);
}

/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

static STATE_POSTLOAD( tail2nos_postload )
{
	tail2nos_state *state = machine->driver_data<tail2nos_state>();
	int i;

	tilemap_mark_all_tiles_dirty(state->bg_tilemap);

	for (i = 0; i < 0x20000; i += 64)
	{
		gfx_element_mark_dirty(machine->gfx[2], i / 64);
	}
}

VIDEO_START( tail2nos )
{
	tail2nos_state *state = machine->driver_data<tail2nos_state>();

	state->bg_tilemap = tilemap_create(machine, get_tile_info, tilemap_scan_rows, 8, 8, 64, 32);

	tilemap_set_transparent_pen(state->bg_tilemap, 15);

	state->zoomdata = (UINT16 *)machine->region("gfx3")->base();

	state_save_register_global_pointer(machine, state->zoomdata, 0x20000 / 2);
	state_save_register_postload(machine, tail2nos_postload, NULL);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE16_HANDLER( tail2nos_bgvideoram_w )
{
	tail2nos_state *state = space->machine->driver_data<tail2nos_state>();

	COMBINE_DATA(&state->bgvideoram[offset]);
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

READ16_HANDLER( tail2nos_zoomdata_r )
{
	tail2nos_state *state = space->machine->driver_data<tail2nos_state>();
	return state->zoomdata[offset];
}

WRITE16_HANDLER( tail2nos_zoomdata_w )
{
	tail2nos_state *state = space->machine->driver_data<tail2nos_state>();
	int oldword = state->zoomdata[offset];

	COMBINE_DATA(&state->zoomdata[offset]);
	if (oldword != state->zoomdata[offset])
		gfx_element_mark_dirty(space->machine->gfx[2], offset / 64);
}

WRITE16_HANDLER( tail2nos_gfxbank_w )
{
	tail2nos_state *state = space->machine->driver_data<tail2nos_state>();

	if (ACCESSING_BITS_0_7)
	{
		int bank;

		/* bits 0 and 2 select char bank */
		if (data & 0x04)
			bank = 2;
		else if (data & 0x01)
			bank = 1;
		else
			bank = 0;

		if (state->charbank != bank)
		{
			state->charbank = bank;
			tilemap_mark_all_tiles_dirty(state->bg_tilemap);
		}

		/* bit 5 seems to select palette bank (used on startup) */
		if (data & 0x20)
			bank = 7;
		else
			bank = 3;

		if (state->charpalette != bank)
		{
			state->charpalette = bank;
			tilemap_mark_all_tiles_dirty(state->bg_tilemap);
		}

		/* bit 4 seems to be video enable */
		state->video_enable = data & 0x10;
	}
}


/***************************************************************************

    Display Refresh

***************************************************************************/

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	tail2nos_state *state = machine->driver_data<tail2nos_state>();
	UINT16 *spriteram = state->spriteram;
	int offs;


	for (offs = 0; offs < state->spriteram_size / 2; offs += 4)
	{
		int sx, sy, flipx, flipy, code, color;

		sx = spriteram[offs + 1];
		if (sx >= 0x8000)
			sx -= 0x10000;
		sy = 0x10000 - spriteram[offs + 0];
		if (sy >= 0x8000)
			sy -= 0x10000;
		code = spriteram[offs + 2] & 0x07ff;
		color = (spriteram[offs + 2] & 0xe000) >> 13;
		flipx = spriteram[offs + 2] & 0x1000;
		flipy = spriteram[offs + 2] & 0x0800;

		drawgfx_transpen(bitmap,/* placement relative to zoom layer verified on the real thing */
				cliprect,machine->gfx[1],
				code,
				40 + color,
				flipx,flipy,
				sx+3,sy+1,15);
	}
}

VIDEO_UPDATE( tail2nos )
{
	tail2nos_state *state = screen->machine->driver_data<tail2nos_state>();

	if (state->video_enable)
	{
		k051316_zoom_draw(state->k051316, bitmap, cliprect, 0, 0);
		draw_sprites(screen->machine, bitmap, cliprect);
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	}
	else
		bitmap_fill(bitmap, cliprect, 0);

	return 0;
}
