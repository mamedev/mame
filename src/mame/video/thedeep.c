/***************************************************************************

                            -= Run Deep / The Deep =-

                    driver by   Luca Elia (l.elia@tin.it)

    [ 1 Horizontally Scrolling Layer ]

        Size :  512 x 512
        Tiles:  16 x 16 x 4.

        In addition to a global x & y scroll register each tile-wide column
        has its own y scroll register.

    [ 1 Fixed Layer ]

        Size :  256 x 256
        Tiles:  8 x 8 x 2.

    [ 128? sprites ]

        Sprites tiles are 16 x 16 x 4. Each sprite has a height and width
        specified (1,2,4, or 8 tiles).

        A sprite of width N uses N consecutive sprites: the first one specifies
        all the data (position,flip), the following ones only the tile code and
        color for that column (tile codes in each column are consecutive).

***************************************************************************/

#include "driver.h"
#include "thedeep.h"

/* Variables only used here: */

static tilemap *tilemap_0,*tilemap_1;

/* Variables & functions needed by drivers: */

UINT8 *thedeep_vram_0, *thedeep_vram_1;
UINT8 *thedeep_scroll, *thedeep_scroll2;


/***************************************************************************

                        Callbacks for the TileMap code

***************************************************************************/

static TILEMAP_MAPPER( tilemap_scan_rows_back )
{
	return (col & 0x0f) + ((col & 0x10) << 5) + (row << 4);
}

static TILE_GET_INFO( get_tile_info_0 )
{
	UINT8 code	=	thedeep_vram_0[ tile_index * 2 + 0 ];
	UINT8 color	=	thedeep_vram_0[ tile_index * 2 + 1 ];
	SET_TILE_INFO(
			1,
			code + (color << 8),
			(color & 0xf0) >> 4,
			TILE_FLIPX	);	// why?
}

static TILE_GET_INFO( get_tile_info_1 )
{
	UINT8 code	=	thedeep_vram_1[ tile_index * 2 + 0 ];
	UINT8 color	=	thedeep_vram_1[ tile_index * 2 + 1 ];
	SET_TILE_INFO(
			2,
			code + (color << 8),
			(color & 0xf0) >> 4,
			0);
}

WRITE8_HANDLER( thedeep_vram_0_w )
{
	thedeep_vram_0[offset] = data;
	tilemap_mark_tile_dirty(tilemap_0, offset / 2);
}

WRITE8_HANDLER( thedeep_vram_1_w )
{
	thedeep_vram_1[offset] = data;
	tilemap_mark_tile_dirty(tilemap_1, offset / 2);
}


/***************************************************************************

                                Palette Init

***************************************************************************/

PALETTE_INIT( thedeep )
{
	int i;
	for (i = 0;i < 512;i++)
		palette_set_color_rgb(machine,i,pal4bit(color_prom[0x400 + i] >> 0),pal4bit(color_prom[0x400 + i] >> 4),pal4bit(color_prom[0x200 + i] >> 0));
}

/***************************************************************************

                                Video Init

***************************************************************************/

VIDEO_START( thedeep )
{
	tilemap_0  = tilemap_create(get_tile_info_0,tilemap_scan_rows_back,TILEMAP_TYPE_PEN,16,16,0x20,0x20);
	tilemap_1  = tilemap_create(get_tile_info_1,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,0x20,0x20);

	tilemap_set_transparent_pen( tilemap_0,  0 );
	tilemap_set_transparent_pen( tilemap_1,  0 );

	tilemap_set_scroll_cols(tilemap_0, 0x20);	// column scroll for the background
}

/***************************************************************************

                                Sprites Drawing

Offset:     Bits:       Value:

    0                   Y (low bits, 0 is bottom)

    1       7-------    Enable
            -6------    Flip Y
            --5-----    Flip X ? (unused)
            ---43---    Height: 1,2,4 or 8 tiles
            -----21-    Width: 1,2,4 or 8 tiles*
            -------0    Y (High bit)

    2                   Code (low bits)

    3                   Code (high bits)

    4                   X (low bits, 0 is right)

    5       7654----    Color
            ----321-
            -------0    X (High bit)

    6                   Unused

    7                   Unused

* a sprite of width N uses N consecutive sprites. The first one specifies
  all the data, the following ones only the tile code and color.

***************************************************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	UINT8 *s = spriteram, *end = s + spriteram_size;

	while (s < end)
	{
		int code,color,sx,sy,flipx,flipy,nx,ny,x,y,attr;

		attr	=	 s[1];
		if (!(attr & 0x80))	{	s+=8;	continue;	}

		sx		=	 s[4];
		sy		=	 s[0];

		color	=	 s[5];

		flipx	=	attr & 0x00;	// ?
		flipy	=	attr & 0x40;

		nx = 1 << ((attr & 0x06) >> 1);
		ny = 1 << ((attr & 0x18) >> 3);

		if (color & 1)	sx -= 256;
		if (attr  & 1)	sy -= 256;

		if (flip_screen)
		{
			flipx = !flipx;
			flipy = !flipy;
			sy = sy - 8;
		}
		else
		{
			sx = 240 - sx;
			sy = 240 - sy - ny * 16 + 16;
		}

		for (x = 0; (x < nx) && (s < end);  x++,s+=8)
		{
			code	=	 s[2] + (s[3] << 8);
			color	=	 s[5] >> 4;

			for (y = 0; y < ny; y++)
			{
				drawgfx(bitmap,machine->gfx[0],
						code + (flipy ? (ny - y - 1) :  y),
						color,
						flipx,flipy,
						sx + x * (flipx ? 16 : -16), sy + y * 16,
						cliprect,TRANSPARENCY_PEN,0 );
			}
		}
	}
}


/***************************************************************************

                                Screen Drawing

***************************************************************************/

VIDEO_UPDATE( thedeep )
{
	int scrollx = thedeep_scroll[0] + (thedeep_scroll[1]<<8);
	int scrolly = thedeep_scroll[2] + (thedeep_scroll[3]<<8);
	int x;

	tilemap_set_scrollx(tilemap_0, 0, scrollx);

	for (x = 0; x < 0x20; x++)
	{
		int y = thedeep_scroll2[x*2+0] + (thedeep_scroll2[x*2+1]<<8);
		tilemap_set_scrolly(tilemap_0, x, y + scrolly);
	}

	fillbitmap(bitmap,get_black_pen(machine),cliprect);

	tilemap_draw(bitmap,cliprect,tilemap_0,0,0);
	draw_sprites(machine, bitmap,cliprect);
	tilemap_draw(bitmap,cliprect,tilemap_1,0,0);
	return 0;
}
