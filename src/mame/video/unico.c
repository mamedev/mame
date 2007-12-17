/***************************************************************************

                              -= Unico Games =-

                    driver by   Luca Elia (l.elia@tin.it)


Note:   if MAME_DEBUG is defined, pressing Z with:

        Q / W / E       Shows Layer 0 / 1 / 2
        A               Shows Sprites

        Keys can be used together!


    [ 3 Scrolling Layers ]

        Tile Size:              16 x 16 x 8
        Layer Size (tiles):     64 x 64

    [ 512 Sprites ]

        Sprites are made of 16 x 16 x 8 tiles. Size can vary from 1 to
        16 tiles horizontally, while their height is always 1 tile.
        There seems to be 4 levels of priority (wrt layers) for each
        sprite, following this simple scheme:

        [if we denote the three layers with 0-3 (0 being the backmost)
         and the sprite with S]

        Sprite Priority         Order (back -> front)
                0                   S 0 1 2
                1                   0 S 1 2
                2                   0 1 S 2
                3                   0 1 2 S

***************************************************************************/

#include "driver.h"
#include "unico.h"

/* Variables needed by drivers: */

UINT16 *unico_vram_0,   *unico_scrollx_0, *unico_scrolly_0;
UINT16 *unico_vram_1,   *unico_scrollx_1, *unico_scrolly_1;
UINT16 *unico_vram_2,   *unico_scrollx_2, *unico_scrolly_2;
UINT32 *unico_vram32_0, *unico_vram32_1, *unico_vram32_2, *unico_scroll32;


/***************************************************************************

                                    Palette

    Byte:   0   1   2   3
    Gun:    R   G   B   0

    6 Bits x Gun

***************************************************************************/

WRITE16_HANDLER( unico_palette_w )
{
	UINT16 data1, data2;
	COMBINE_DATA(&paletteram16[offset]);
	data1 = paletteram16[offset & ~1];
	data2 = paletteram16[offset |  1];
	palette_set_color_rgb( Machine,offset/2,
		 (data1 >> 8) & 0xFC,
		 (data1 >> 0) & 0xFC,
		 (data2 >> 8) & 0xFC	);
}

WRITE32_HANDLER( unico_palette32_w )
{
	UINT32 rgb0 = COMBINE_DATA(&paletteram32[offset]);
	palette_set_color_rgb( Machine,offset,
		 (rgb0 >> 24) & 0xFC,
		 (rgb0 >> 16) & 0xFC,
		 (rgb0 >>  8) & 0xFC	);
}


/***************************************************************************

                                Tilemaps

    Offset:     Bits:                   Value:

        0.w                             Code
        2.w     fedc ba98 7--- ----
                ---- ---- -6-- ----     Flip Y
                ---- ---- --5- ----     Flip X
                ---- ---- ---4 3210     Color

***************************************************************************/

#define LAYER( _N_ ) \
static tilemap *tilemap_##_N_; \
\
static TILE_GET_INFO( get_tile_info_##_N_ ) \
{ \
	UINT16 code = unico_vram_##_N_[ 2 * tile_index + 0 ]; \
	UINT16 attr = unico_vram_##_N_[ 2 * tile_index + 1 ]; \
	SET_TILE_INFO(1, code, attr & 0x1f, TILE_FLIPYX( attr >> 5 )); \
} \
\
static TILE_GET_INFO( get_tile_info32_##_N_ ) \
{ \
	UINT32 code = unico_vram32_##_N_[tile_index]; \
	SET_TILE_INFO(1, code >> 16, code & 0x1f, TILE_FLIPYX( code >> 5 )); \
} \
\
WRITE16_HANDLER( unico_vram_##_N_##_w ) \
{ \
	COMBINE_DATA(&unico_vram_##_N_[offset]); \
	tilemap_mark_tile_dirty(tilemap_##_N_,offset/2); \
} \
\
WRITE32_HANDLER( unico_vram32_##_N_##_w ) \
{ \
	COMBINE_DATA(&unico_vram32_##_N_[offset]); \
	tilemap_mark_tile_dirty(tilemap_##_N_,offset); \
}

LAYER( 0 )
LAYER( 1 )
LAYER( 2 )



/***************************************************************************


                            Video Hardware Init


***************************************************************************/

static int sprites_scrolldx, sprites_scrolldy;

VIDEO_START( unico )
{
	tilemap_0 = tilemap_create(	get_tile_info_0,tilemap_scan_rows,
								TILEMAP_TYPE_PEN,	16,16,	0x40, 0x40);

	tilemap_1 = tilemap_create(	get_tile_info_1,tilemap_scan_rows,
								TILEMAP_TYPE_PEN,	16,16,	0x40, 0x40);

	tilemap_2 = tilemap_create(	get_tile_info_2,tilemap_scan_rows,
								TILEMAP_TYPE_PEN,	16,16,	0x40, 0x40);

	sprites_scrolldx = -0x3f;
	sprites_scrolldy = -0x0e;

	tilemap_set_scrolldx(tilemap_0,-0x32,0);
	tilemap_set_scrolldx(tilemap_1,-0x30,0);
	tilemap_set_scrolldx(tilemap_2,-0x2e,0);

	tilemap_set_scrolldy(tilemap_0,-0x0f,0);
	tilemap_set_scrolldy(tilemap_1,-0x0f,0);
	tilemap_set_scrolldy(tilemap_2,-0x0f,0);

	tilemap_set_transparent_pen(tilemap_0,0x00);
	tilemap_set_transparent_pen(tilemap_1,0x00);
	tilemap_set_transparent_pen(tilemap_2,0x00);
}

VIDEO_START( zeropnt2 )
{
	tilemap_0 = tilemap_create(	get_tile_info32_0,tilemap_scan_rows,
								TILEMAP_TYPE_PEN,	16,16,	0x40, 0x40);

	tilemap_1 = tilemap_create(	get_tile_info32_1,tilemap_scan_rows,
								TILEMAP_TYPE_PEN,	16,16,	0x40, 0x40);

	tilemap_2 = tilemap_create(	get_tile_info32_2,tilemap_scan_rows,
								TILEMAP_TYPE_PEN,	16,16,	0x40, 0x40);

	sprites_scrolldx = -0x3f;
	sprites_scrolldy = -0x0e;

	tilemap_set_scrolldx(tilemap_0,-0x32,0);
	tilemap_set_scrolldx(tilemap_1,-0x30,0);
	tilemap_set_scrolldx(tilemap_2,-0x2e,0);

	tilemap_set_scrolldy(tilemap_0,-0x0f,0);
	tilemap_set_scrolldy(tilemap_1,-0x0f,0);
	tilemap_set_scrolldy(tilemap_2,-0x0f,0);

	tilemap_set_transparent_pen(tilemap_0,0x00);
	tilemap_set_transparent_pen(tilemap_1,0x00);
	tilemap_set_transparent_pen(tilemap_2,0x00);
}



/***************************************************************************

                                Sprites Drawing


        0.w                             X

        2.w                             Y

        4.w                             Code

        6.w     fe-- ---- ---- ----
                --dc ---- ---- ----     Priority
                ---- ba98 ---- ----     Number of tiles along X, minus 1
                ---- ---- 7--- ----
                ---- ---- -6-- ----     Flip Y?
                ---- ---- --5- ----     Flip X
                ---- ---- ---4 3210     Color


***************************************************************************/

static void unico_draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect)
{
	int offs;

	/* Draw them backwards, for pdrawgfx */
	for ( offs = (spriteram_size-8)/2; offs >= 0 ; offs -= 8/2 )
	{
		int x, startx, endx, incx;

		int	sx			=	spriteram16[ offs + 0 ];
		int	sy			=	spriteram16[ offs + 1 ];
		int	code		=	spriteram16[ offs + 2 ];
		int	attr		=	spriteram16[ offs + 3 ];

		int	flipx		=	attr & 0x020;
		int	flipy		=	attr & 0x040;	// not sure

		int dimx		=	((attr >> 8) & 0xf) + 1;

		int priority	=	((attr >> 12) & 0x3);
		int pri_mask;

		switch( priority )
		{
			case 0:		pri_mask = 0xfe;	break;	// below all
			case 1:		pri_mask = 0xf0;	break;	// above layer 0
			case 2:		pri_mask = 0xfc;	break;	// above layer 1
			default:
			case 3:		pri_mask = 0x00;			// above all
		}

		sx	+=	sprites_scrolldx;
		sy	+=	sprites_scrolldy;

		sx	=	(sx & 0x1ff) - (sx & 0x200);
		sy	=	(sy & 0x1ff) - (sy & 0x200);

		if (flipx)	{	startx = sx+(dimx-1)*16;	endx = sx-16;		incx = -16;	}
		else		{	startx = sx;				endx = sx+dimx*16;	incx = +16;	}

		for (x = startx ; x != endx ; x += incx)
		{
			pdrawgfx(	bitmap, machine->gfx[0],
						code++,
						attr & 0x1f,
						flipx, flipy,
						x, sy,
						cliprect, TRANSPARENCY_PEN,0x00,
						pri_mask	);
		}
	}
}

static void zeropnt2_draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect)
{
	int offs;

	/* Draw them backwards, for pdrawgfx */
	for ( offs = (spriteram_size-8)/4; offs >= 0 ; offs -= 8/4 )
	{
		int x, startx, endx, incx;

		int	sx			=	spriteram32[ offs + 0 ] >> 16;
		int	sy			=	spriteram32[ offs + 0 ] & 0xffff;
		int	code		=	spriteram32[ offs + 1 ] >> 16;
		int	attr		=	spriteram32[ offs + 1 ] & 0xffff;

		int	flipx		=	attr & 0x020;
		int	flipy		=	attr & 0x040;	// not sure

		int dimx		=	((attr >> 8) & 0xf) + 1;

		int priority	=	((attr >> 12) & 0x3);
		int pri_mask;

		switch( priority )
		{
			case 0:		pri_mask = 0xfe;	break;	// below all
			case 1:		pri_mask = 0xf0;	break;	// above layer 0
			case 2:		pri_mask = 0xfc;	break;	// above layer 1
			default:
			case 3:		pri_mask = 0x00;			// above all
		}

		sx	+=	sprites_scrolldx;
		sy	+=	sprites_scrolldy;

		sx	=	(sx & 0x1ff) - (sx & 0x200);
		sy	=	(sy & 0x1ff) - (sy & 0x200);

		if (flipx)	{	startx = sx+(dimx-1)*16;	endx = sx-16;		incx = -16;	}
		else		{	startx = sx;				endx = sx+dimx*16;	incx = +16;	}

		for (x = startx ; x != endx ; x += incx)
		{
			pdrawgfx(	bitmap, machine->gfx[0],
						code++,
						attr & 0x1f,
						flipx, flipy,
						x, sy,
						cliprect, TRANSPARENCY_PEN,0x00,
						pri_mask	);
		}
	}
}



/***************************************************************************


                                Screen Drawing


***************************************************************************/

VIDEO_UPDATE( unico )
{
	int layers_ctrl = -1;

	tilemap_set_scrollx(tilemap_0, 0, *unico_scrollx_0);
	tilemap_set_scrolly(tilemap_0, 0, *unico_scrolly_0);

	tilemap_set_scrollx(tilemap_1, 0, *unico_scrollx_1);
	tilemap_set_scrolly(tilemap_1, 0, *unico_scrolly_1);

	tilemap_set_scrolly(tilemap_2, 0, *unico_scrolly_2);
	tilemap_set_scrollx(tilemap_2, 0, *unico_scrollx_2);

#ifdef MAME_DEBUG
if ( input_code_pressed(KEYCODE_Z) || input_code_pressed(KEYCODE_X) )
{
	int msk = 0;
	if (input_code_pressed(KEYCODE_Q))	msk |= 1;
	if (input_code_pressed(KEYCODE_W))	msk |= 2;
	if (input_code_pressed(KEYCODE_E))	msk |= 4;
	if (input_code_pressed(KEYCODE_A))	msk |= 8;
	if (msk != 0) layers_ctrl &= msk;
}
#endif

	/* The background color is the first of the last palette */
	fillbitmap(bitmap,machine->pens[0x1f00],cliprect);
	fillbitmap(priority_bitmap,0,cliprect);

	if (layers_ctrl & 1)	tilemap_draw(bitmap,cliprect,tilemap_0,0,1);
	if (layers_ctrl & 2)	tilemap_draw(bitmap,cliprect,tilemap_1,0,2);
	if (layers_ctrl & 4)	tilemap_draw(bitmap,cliprect,tilemap_2,0,4);

	/* Sprites are drawn last, using pdrawgfx */
	if (layers_ctrl & 8)	unico_draw_sprites(machine, bitmap,cliprect);

	return 0;
}

VIDEO_UPDATE( zeropnt2 )
{
	int layers_ctrl = -1;

	tilemap_set_scrollx(tilemap_0, 0, unico_scroll32[0] >> 16);
	tilemap_set_scrolly(tilemap_0, 0, unico_scroll32[0] & 0xffff);

	tilemap_set_scrollx(tilemap_1, 0, unico_scroll32[2] & 0xffff);
	tilemap_set_scrolly(tilemap_1, 0, unico_scroll32[5] >> 16);

	tilemap_set_scrollx(tilemap_2, 0, unico_scroll32[2] >> 16);
	tilemap_set_scrolly(tilemap_2, 0, unico_scroll32[1] >> 16);

#ifdef MAME_DEBUG
if ( input_code_pressed(KEYCODE_Z) || input_code_pressed(KEYCODE_X) )
{
	int msk = 0;
	if (input_code_pressed(KEYCODE_Q))	msk |= 1;
	if (input_code_pressed(KEYCODE_W))	msk |= 2;
	if (input_code_pressed(KEYCODE_E))	msk |= 4;
	if (input_code_pressed(KEYCODE_A))	msk |= 8;
	if (msk != 0) layers_ctrl &= msk;
}
#endif

	/* The background color is the first of the last palette */
	fillbitmap(bitmap,machine->pens[0x1f00],cliprect);
	fillbitmap(priority_bitmap,0,cliprect);

	if (layers_ctrl & 1)	tilemap_draw(bitmap,cliprect,tilemap_0,0,1);
	if (layers_ctrl & 2)	tilemap_draw(bitmap,cliprect,tilemap_1,0,2);
	if (layers_ctrl & 4)	tilemap_draw(bitmap,cliprect,tilemap_2,0,4);

	/* Sprites are drawn last, using pdrawgfx */
	if (layers_ctrl & 8)	zeropnt2_draw_sprites(machine, bitmap,cliprect);

	return 0;
}

