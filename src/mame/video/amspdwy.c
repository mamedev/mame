/***************************************************************************

                            -= American Speedway =-

                    driver by   Luca Elia (l.elia@tin.it)


- 8x8 4 Color Tiles (with 8 palettes) used for both:

    - 1 256x256 non scrolling layer
    - 64 (32?) Sprites

***************************************************************************/

#include "emu.h"
#include "includes/amspdwy.h"


WRITE8_HANDLER( amspdwy_paletteram_w )
{
	data ^= 0xff;
	paletteram_BBGGGRRR_w(space, offset, data);
//  paletteram_RRRGGGBB_w(offset, data);
}

WRITE8_HANDLER( amspdwy_flipscreen_w )
{
	amspdwy_state *state = (amspdwy_state *)space->machine->driver_data;
	state->flipscreen ^= 1;
	flip_screen_set(space->machine, state->flipscreen);
}

/***************************************************************************

                        Callbacks for the TileMap code

                              [ Tiles Format ]

    Videoram:   76543210    Code Low Bits
    Colorram:   765-----
                ---43---    Code High Bits
                -----210    Color

***************************************************************************/

static TILE_GET_INFO( get_tile_info )
{
	amspdwy_state *state = (amspdwy_state *)machine->driver_data;
	UINT8 code = state->videoram[tile_index];
	UINT8 color = state->colorram[tile_index];
	SET_TILE_INFO(
			0,
			code + ((color & 0x18)<<5),
			color & 0x07,
			0);
}

WRITE8_HANDLER( amspdwy_videoram_w )
{
	amspdwy_state *state = (amspdwy_state *)space->machine->driver_data;
	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

WRITE8_HANDLER( amspdwy_colorram_w )
{
	amspdwy_state *state = (amspdwy_state *)space->machine->driver_data;
	state->colorram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}


/* logical (col,row) -> memory offset */
static TILEMAP_MAPPER( tilemap_scan_cols_back )
{
	return col * num_rows + (num_rows - row - 1);
}


VIDEO_START( amspdwy )
{
	amspdwy_state *state = (amspdwy_state *)machine->driver_data;
	state->bg_tilemap = tilemap_create(machine, get_tile_info, tilemap_scan_cols_back, 8, 8, 0x20, 0x20);
}



/***************************************************************************

                                Sprites Drawing

Offset:     Format:     Value:

0                       Y
1                       X
2                       Code Low Bits
3           7-------    Flip X
            -6------    Flip Y
            --5-----
            ---4----    ?
            ----3---    Code High Bit?
            -----210    Color

***************************************************************************/

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	amspdwy_state *state = (amspdwy_state *)machine->driver_data;
	UINT8 *spriteram = state->spriteram;
	int i;
	int max_x = video_screen_get_width(machine->primary_screen)  - 1;
	int max_y = video_screen_get_height(machine->primary_screen) - 1;

	for (i = 0; i < state->spriteram_size ; i += 4)
	{
		int y = spriteram[i + 0];
		int x = spriteram[i + 1];
		int code = spriteram[i + 2];
		int attr = spriteram[i + 3];
		int flipx = attr & 0x80;
		int flipy = attr & 0x40;

		if (flip_screen_get(machine))
		{
			x = max_x - x - 8;
			y = max_y - y - 8;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap,cliprect,machine->gfx[0],
//              code + ((attr & 0x18)<<5),
				code + ((attr & 0x08)<<5),
				attr,
				flipx, flipy,
				x,y,0 );
	}
}




/***************************************************************************

                                Screen Drawing

***************************************************************************/

VIDEO_UPDATE( amspdwy )
{
	amspdwy_state *state = (amspdwy_state *)screen->machine->driver_data;
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}
