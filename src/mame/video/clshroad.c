/***************************************************************************

                            -= Clash Road =-

                    driver by   Luca Elia (l.elia@tin.it)

    [ 2 Horizontally Scrolling Layers ]

        Size :  512 x 256
        Tiles:  16 x 16 x 4.

        These 2 layers share the same graphics and X scroll value.
        The tile codes are stuffed together in memory too: first one
        layer's row, then the other's (and so on for all the rows).

    [ 1 Fixed Layer ]

        Size :  (256 + 32) x 256
        Tiles:  8 x 8 x 4.

        This is like a 32x32 tilemap, but the top and bottom rows (that
        fall outside the visible area) are used to widen the tilemap
        horizontally, adding 2 vertical columns both sides.

        The result is a 36x28 visible tilemap.

    [ 64? sprites ]

        Sprites are 16 x 16 x 4.

***************************************************************************/

#include "driver.h"

/* Variables only used here: */

static tilemap *tilemap_0a, *tilemap_0b, *tilemap_1;

/* Variables & functions needed by drivers: */

UINT8 *clshroad_vram_0, *clshroad_vram_1;
UINT8 *clshroad_vregs;

WRITE8_HANDLER( clshroad_vram_0_w );
WRITE8_HANDLER( clshroad_vram_1_w );
WRITE8_HANDLER( clshroad_flipscreen_w );


WRITE8_HANDLER( clshroad_flipscreen_w )
{
	flip_screen_set( data & 1 );
}


PALETTE_INIT( clshroad )
{
	int i;
	for (i = 0;i < 256;i++)
		palette_set_color_rgb(machine,i,	pal4bit(color_prom[i + 256 * 0]),
								        pal4bit(color_prom[i + 256 * 1]),
								        pal4bit(color_prom[i + 256 * 2]));
}

PALETTE_INIT( firebatl )
{
	int i;
	#define TOTAL_COLORS(gfxn) (machine->gfx[gfxn]->total_colors * machine->gfx[gfxn]->color_granularity)
	#define COLOR(gfxn,offs) (colortable[machine->drv->gfxdecodeinfo[gfxn].color_codes_start + offs])


#if 1
	for (i = 0;i < 256;i++)
	{
		int bit0,bit1,bit2,bit3,r,g,b;


		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		bit3 = (color_prom[i] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		/* green component */
		bit0 = (color_prom[i + 256] >> 0) & 0x01;
		bit1 = (color_prom[i + 256] >> 1) & 0x01;
		bit2 = (color_prom[i + 256] >> 2) & 0x01;
		bit3 = (color_prom[i + 256] >> 3) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		/* blue component */
		bit0 = (color_prom[i + 2*256] >> 0) & 0x01;
		bit1 = (color_prom[i + 2*256] >> 1) & 0x01;
		bit2 = (color_prom[i + 2*256] >> 2) & 0x01;
		bit3 = (color_prom[i + 2*256] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}
#endif

	color_prom += 3*256;
	/* color_prom now points to the beginning of the lookup table */


	for (i = 0;i < TOTAL_COLORS(2);i++)
		COLOR(2,i) = ((color_prom[i] & 0x0f) << 4) + (color_prom[i+256] & 0x0f);
}



/***************************************************************************

                        Callbacks for the TileMap code

***************************************************************************/

/***************************************************************************

                          Layers 0 Tiles Format

Offset:

    00-3f:  Even bytes: Codes   Odd bytes: Colors   <- Layer B First Row
    40-7f:  Even bytes: Codes   Odd bytes: Colors   <- Layer A First Row
    ..                                      <- 2nd Row
    ..                                      <- 3rd Row
    etc.

***************************************************************************/

static TILE_GET_INFO( get_tile_info_0a )
{
	UINT8 code;
	tile_index = (tile_index & 0x1f) + (tile_index & ~0x1f)*2;
	code	=	clshroad_vram_0[ tile_index * 2 + 0x40 ];
//  color   =   clshroad_vram_0[ tile_index * 2 + 0x41 ];
	SET_TILE_INFO(
			1,
			code,
			0,
			0);
}

static TILE_GET_INFO( get_tile_info_0b )
{
	UINT8 code;
	tile_index = (tile_index & 0x1f) + (tile_index & ~0x1f)*2;
	code	=	clshroad_vram_0[ tile_index * 2 + 0x00 ];
//  color   =   clshroad_vram_0[ tile_index * 2 + 0x01 ];
	SET_TILE_INFO(
			1,
			code,
			0,
			0);
}

WRITE8_HANDLER( clshroad_vram_0_w )
{
	int tile_index = offset / 2;
	int tile = (tile_index & 0x1f) + (tile_index & ~0x3f)/2;
	clshroad_vram_0[offset] = data;
	if (tile_index & 0x20)	tilemap_mark_tile_dirty(tilemap_0a, tile);
	else					tilemap_mark_tile_dirty(tilemap_0b, tile);
}

/***************************************************************************

                          Layer 1 Tiles Format

Offset:

    000-3ff     Code
    400-7ff     7654----    Code (High bits)
                ----3210    Color

    This is like a 32x32 tilemap, but the top and bottom rows (that
    fall outside the visible area) are used to widen the tilemap
    horizontally, adding 2 vertical columns both sides.

    The result is a 36x28 visible tilemap.

***************************************************************************/

/* logical (col,row) -> memory offset */
static TILEMAP_MAPPER( tilemap_scan_rows_extra )
{
	// The leftmost columns come from the bottom rows
	if (col <= 0x01)	return row + (col + 0x1e) * 0x20;
	// The rightmost columns come from the top rows
	if (col >= 0x22)	return row + (col - 0x22) * 0x20;

	// These are not visible, but *must* be mapped to other tiles than
	// those used by the leftmost and rightmost columns (tilemap "bug"?)
	if (row <= 0x01)	return 0;
	if (row >= 0x1e)	return 0;

	// "normal" layout for the rest.
	return (col-2) + row * 0x20;
}

static TILE_GET_INFO( get_tile_info_fb1 )
{
	UINT8 code	=	clshroad_vram_1[ tile_index + 0x000 ];
	UINT8 color	=	clshroad_vram_1[ tile_index + 0x400 ];
	SET_TILE_INFO(
			2,
			code,
			color & 0x3f,
			0);
}

static TILE_GET_INFO( get_tile_info_1 )
{
	UINT8 code	=	clshroad_vram_1[ tile_index + 0x000 ];
	UINT8 color	=	clshroad_vram_1[ tile_index + 0x400 ];
	SET_TILE_INFO(
			2,
			code + ((color & 0xf0)<<4),
			color & 0x0f,
			0);
}

WRITE8_HANDLER( clshroad_vram_1_w )
{
	clshroad_vram_1[offset] = data;
	tilemap_mark_tile_dirty(tilemap_1, offset % 0x400);
}


VIDEO_START( firebatl )
{
	/* These 2 use the graphics and scroll value */
	tilemap_0a = tilemap_create(get_tile_info_0a,tilemap_scan_rows,TILEMAP_TYPE_PEN,     16,16,0x20,0x10);
	tilemap_0b = tilemap_create(get_tile_info_0b,tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,0x20,0x10);
	/* Text (No scrolling) */
	tilemap_1  = tilemap_create(get_tile_info_fb1,tilemap_scan_rows_extra,TILEMAP_TYPE_COLORTABLE,8,8,0x24,0x20);

	tilemap_set_scroll_rows( tilemap_0a, 1);
	tilemap_set_scroll_rows( tilemap_0b, 1);
	tilemap_set_scroll_rows( tilemap_1,  1);

	tilemap_set_scroll_cols( tilemap_0a, 1);
	tilemap_set_scroll_cols( tilemap_0b, 1);
	tilemap_set_scroll_cols( tilemap_1,  1);

	tilemap_set_scrolldx( tilemap_0a, -0x30, -0xb5);
	tilemap_set_scrolldx( tilemap_0b, -0x30, -0xb5);

	tilemap_set_transparent_pen( tilemap_0b, 0 );
	tilemap_set_transparent_pen( tilemap_1,  0x0f );
}

VIDEO_START( clshroad )
{
	/* These 2 use the graphics and scroll value */
	tilemap_0a = tilemap_create(get_tile_info_0a,tilemap_scan_rows,TILEMAP_TYPE_PEN,     16,16,0x20,0x10);
	tilemap_0b = tilemap_create(get_tile_info_0b,tilemap_scan_rows,TILEMAP_TYPE_PEN,16,16,0x20,0x10);
	/* Text (No scrolling) */
	tilemap_1  = tilemap_create(get_tile_info_1,tilemap_scan_rows_extra,TILEMAP_TYPE_PEN,8,8,0x24,0x20);

	tilemap_set_scroll_rows( tilemap_0a, 1);
	tilemap_set_scroll_rows( tilemap_0b, 1);
	tilemap_set_scroll_rows( tilemap_1,  1);

	tilemap_set_scroll_cols( tilemap_0a, 1);
	tilemap_set_scroll_cols( tilemap_0b, 1);
	tilemap_set_scroll_cols( tilemap_1,  1);

	tilemap_set_scrolldx( tilemap_0a, -0x30, -0xb5);
	tilemap_set_scrolldx( tilemap_0b, -0x30, -0xb5);

	tilemap_set_transparent_pen( tilemap_0b, 0x0f );
	tilemap_set_transparent_pen( tilemap_1,  0x0f );
}


/***************************************************************************

                                Sprites Drawing

Offset:     Format:     Value:

    0

    1                   Y (Bottom-up)

    2       765432--
            ------10    Code (high bits)

    3       76------
            --543210    Code (low bits)

    4

    5                   X (low bits)

    6                   X (High bits)

    7       7654----
            ----3210    Color

- Sprite flipping ?

***************************************************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int i;

	for (i = 0; i < spriteram_size ; i += 8)
	{
		int y		=	 240 - spriteram[i+1];
		int code	=	(spriteram[i+3] & 0x3f) + (spriteram[i+2] << 6);
		int x		=	 spriteram[i+5]         + (spriteram[i+6] << 8);
		int attr	=	 spriteram[i+7];

		int flipx	=	0;
		int flipy	=	0;

		x -= 0x4a/2;
		if (flip_screen)
		{
			y = 240 - y;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx(bitmap,machine->gfx[0],
				code,
				attr & 0x0f,
				flipx,flipy,
				x,y,
				cliprect,TRANSPARENCY_PEN,15 );
	}
}


/***************************************************************************


                                Screen Drawing


***************************************************************************/

VIDEO_UPDATE( clshroad )
{
	int scrollx  = clshroad_vregs[ 0 ] + (clshroad_vregs[ 1 ] << 8);
//  int priority = clshroad_vregs[ 2 ];

	/* Only horizontal scrolling (these 2 layers use the same value) */
	tilemap_set_scrollx(tilemap_0a, 0, scrollx);
	tilemap_set_scrollx(tilemap_0b, 0, scrollx);

	tilemap_draw(bitmap,cliprect,tilemap_0a,0,0);	// Opaque
	tilemap_draw(bitmap,cliprect,tilemap_0b,0,0);
	draw_sprites(machine,bitmap,cliprect);
	tilemap_draw(bitmap,cliprect,tilemap_1,0,0);
	return 0;
}
