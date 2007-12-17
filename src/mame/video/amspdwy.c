/***************************************************************************

                            -= American Speedway =-

                    driver by   Luca Elia (l.elia@tin.it)


- 8x8 4 Color Tiles (with 8 palettes) used for both:

    - 1 256x256 non scrolling layer
    - 64 (32?) Sprites

***************************************************************************/
#include "driver.h"

/* Variables only used here: */

static tilemap *bg_tilemap;


WRITE8_HANDLER( amspdwy_paletteram_w )
{
	data ^= 0xff;
	paletteram_BBGGGRRR_w(offset,data);
//  paletteram_RRRGGGBB_w(offset,data);
}

WRITE8_HANDLER( amspdwy_flipscreen_w )
{
	static int flip = 0;
	flip ^= 1;
	flip_screen_set( flip );
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
	UINT8 code	=	videoram[ tile_index ];
	UINT8 color	=	colorram[ tile_index ];
	SET_TILE_INFO(
			0,
			code + ((color & 0x18)<<5),
			color & 0x07,
			0);
}

WRITE8_HANDLER( amspdwy_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( amspdwy_colorram_w )
{
	colorram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}


/* logical (col,row) -> memory offset */
static TILEMAP_MAPPER( tilemap_scan_cols_back )
{
	return col*num_rows + (num_rows - row - 1);
}


VIDEO_START( amspdwy )
{
	bg_tilemap	=	tilemap_create(	get_tile_info,	tilemap_scan_cols_back,
								TILEMAP_TYPE_PEN,	8,8,	0x20, 0x20 );
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

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect)
{
	int i;
	int max_x = machine->screen[0].width  - 1;
	int max_y = machine->screen[0].height - 1;

	for (i = 0; i < spriteram_size ; i += 4)
	{
		int y		=	spriteram[i+0];
		int x		=	spriteram[i+1];
		int code	=	spriteram[i+2];
		int attr	=	spriteram[i+3];
		int flipx	=	attr & 0x80;
		int flipy	=	attr & 0x40;

		if (flip_screen)
		{
			x = max_x - x - 8;	y = max_y - y - 8;
			flipx = !flipx;	flipy = !flipy;
		}

		drawgfx(bitmap,machine->gfx[0],
//              code + ((attr & 0x18)<<5),
				code + ((attr & 0x08)<<5),
				attr,
				flipx, flipy,
				x,y,
				cliprect,TRANSPARENCY_PEN,0 );
	}
}




/***************************************************************************

                                Screen Drawing

***************************************************************************/

VIDEO_UPDATE( amspdwy )
{
	tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);
	draw_sprites(machine, bitmap,cliprect);
	return 0;
}
