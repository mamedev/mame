/*
*   Video Driver for Metal Soldier Isaac II (1985)
*/

#include "emu.h"
#include "includes/buggychl.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_fg_tile_info )
{
	buggychl_state *state = machine->driver_data<buggychl_state>();
	int tile_number = state->videoram[tile_index];
	SET_TILE_INFO( 0,
			tile_number,
			0x10,
			0);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	buggychl_state *state = machine->driver_data<buggychl_state>();
	int tile_number = state->videoram2[tile_index];
	SET_TILE_INFO( 1,
			0x100 + tile_number,
			0x30,
			0);
}

static TILE_GET_INFO( get_bg2_tile_info )
{
	buggychl_state *state = machine->driver_data<buggychl_state>();
	int tile_number = state->videoram3[tile_index];

	/* graphics 0 or 1 */
	int gfx_b = (state->bg2_textbank >> 3) & 1;

	SET_TILE_INFO( gfx_b,
			tile_number,
			0x20,
			0);
}

/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( msisaac )
{
	buggychl_state *state = machine->driver_data<buggychl_state>();
	state->bg_tilemap  = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->bg2_tilemap = tilemap_create(machine, get_bg2_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->fg_tilemap  = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);

	tilemap_set_transparent_pen(state->bg2_tilemap, 0);
	tilemap_set_transparent_pen(state->fg_tilemap, 0);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( msisaac_fg_scrolly_w )
{
	buggychl_state *state = space->machine->driver_data<buggychl_state>();
	tilemap_set_scrolly(state->fg_tilemap, 0, data);
}

WRITE8_HANDLER( msisaac_fg_scrollx_w )
{
	buggychl_state *state = space->machine->driver_data<buggychl_state>();
	tilemap_set_scrollx(state->fg_tilemap, 0, 9 + data);
}

WRITE8_HANDLER( msisaac_bg2_scrolly_w )
{
	buggychl_state *state = space->machine->driver_data<buggychl_state>();
	tilemap_set_scrolly(state->bg2_tilemap, 0, data);
}

WRITE8_HANDLER( msisaac_bg2_scrollx_w )
{
	buggychl_state *state = space->machine->driver_data<buggychl_state>();
	tilemap_set_scrollx(state->bg2_tilemap, 0, 9 + 2 + data);
}

WRITE8_HANDLER( msisaac_bg_scrolly_w )
{
	buggychl_state *state = space->machine->driver_data<buggychl_state>();
	tilemap_set_scrolly(state->bg_tilemap, 0, data);
}

WRITE8_HANDLER( msisaac_bg_scrollx_w )
{
	buggychl_state *state = space->machine->driver_data<buggychl_state>();
	tilemap_set_scrollx(state->bg_tilemap, 0, 9 + 4 + data);
}


#ifdef UNUSED_FUNCTION
WRITE8_HANDLER( msisaac_textbank1_w )
{
	if (textbank1!=data)
	{
		textbank1 = data;
		tilemap_mark_all_tiles_dirty(fg_tilemap);
	}
}
#endif

WRITE8_HANDLER( msisaac_bg2_textbank_w )
{
	buggychl_state *state = space->machine->driver_data<buggychl_state>();
	if (state->bg2_textbank != data )
	{
		state->bg2_textbank = data;
		tilemap_mark_all_tiles_dirty(state->bg2_tilemap);

		//check if we are correct on this one
		if ((data != 8) && (data != 0))
		{
			logerror("bg2 control=%2x\n", data);
		}
	}
}

WRITE8_HANDLER( msisaac_bg_videoram_w )
{
	buggychl_state *state = space->machine->driver_data<buggychl_state>();
	state->videoram2[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

WRITE8_HANDLER( msisaac_bg2_videoram_w )
{
	buggychl_state *state = space->machine->driver_data<buggychl_state>();
	state->videoram3[offset] = data;
	tilemap_mark_tile_dirty(state->bg2_tilemap, offset);
}

WRITE8_HANDLER( msisaac_fg_videoram_w )
{
	buggychl_state *state = space->machine->driver_data<buggychl_state>();
	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap, offset);
}


/***************************************************************************

  Display refresh

***************************************************************************/
static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	buggychl_state *state = machine->driver_data<buggychl_state>();
	const UINT8 *source = state->spriteram + 32 * 4 - 4;
	const UINT8 *finish = state->spriteram; /* ? */

	while (source >= finish)
	{
		int sx = source[0];
		int sy = 240 - source[1] - 1;
		int attributes = source[2];
		int sprite_number = source[3];

		int color = (attributes >> 4) & 0xf;
		int flipx = (attributes & 0x1);
		int flipy = (attributes & 0x2);

		gfx_element *gfx = machine->gfx[2];

		if (attributes & 4)
		{
			//color = rand() & 15;
			gfx = machine->gfx[3];
		}

		if (attributes & 8)	/* double size sprite */
		{
			switch (attributes & 3)
			{
			case 0: /* flipx==0 && flipy==0 */
				drawgfx_transpen(bitmap,cliprect,gfx,
					sprite_number+1,color,
					flipx,flipy,
					sx,sy-16,0 );
				drawgfx_transpen(bitmap,cliprect,gfx,
					sprite_number,color,
					flipx,flipy,
					sx,sy,0 );
				break;
			case 1: /* flipx==1 && flipy==0 */
				drawgfx_transpen(bitmap,cliprect,gfx,
					sprite_number+1,color,
					flipx,flipy,
					sx,sy-16,0 );
				drawgfx_transpen(bitmap,cliprect,gfx,
					sprite_number,color,
					flipx,flipy,
					sx,sy,0 );
				break;
			case 2: /* flipx==0 && flipy==1 */
				drawgfx_transpen(bitmap,cliprect,gfx,
					sprite_number,color,
					flipx,flipy,
					sx,sy-16,0 );
				drawgfx_transpen(bitmap,cliprect,gfx,
					sprite_number+1,color,
					flipx,flipy,
					sx,sy,0 );
				break;
			case 3: /* flipx==1 && flipy==1 */
				drawgfx_transpen(bitmap,cliprect,gfx,
					sprite_number,color,
					flipx,flipy,
					sx,sy-16,0 );
				drawgfx_transpen(bitmap,cliprect,gfx,
					sprite_number+1,color,
					flipx,flipy,
					sx,sy,0 );
				break;
			}
		}
		else
		{
			drawgfx_transpen(bitmap,cliprect,gfx,
				sprite_number,
				color,
				flipx,flipy,
				sx,sy,0 );
		}
		source -= 4;
	}
}

VIDEO_UPDATE( msisaac )
{
	buggychl_state *state = screen->machine->driver_data<buggychl_state>();
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, state->bg2_tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);
	return 0;
}
