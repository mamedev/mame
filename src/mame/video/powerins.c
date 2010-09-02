/***************************************************************************

                          -= Power Instinct =-
                            (C) 1993 Atlus

                driver by   Luca Elia (l.elia@tin.it)


Note:   if MAME_DEBUG is defined, pressing Z with:

        Q           shows layer 1
        W           shows layer 2
        A           shows the sprites

        Keys can be used togheter!

        [ 2 Scrolling Layers ]

        Each Layer is made of various pages of 256x256 pixels.

            [Layer 0]
                Pages:              16x2
                Tiles:              16x16x4
                Scroll:             X,Y

            [Layer 1]
                Pages:              2x1
                Tiles:              8x8x4
                Scroll:             No

        [ 256 Sprites ]

        Each sprite is made of a variable amount of 16x16 tiles.
        Size can therefore vary from 16x16 (1 tile) to 256x256
        (16x16 tiles)


**************************************************************************/

#include "emu.h"
#include "includes/powerins.h"

/* Variables that driver has access to: */
UINT16 *powerins_vram_0, *powerins_vctrl_0;
UINT16 *powerins_vram_1, *powerins_vctrl_1;
//UINT16 *powerins_vregs;

/* Variables only used here: */
static tilemap_t *tilemap_0, *tilemap_1;
static int tile_bank;



/***************************************************************************

                        Hardware registers access

***************************************************************************/


WRITE16_HANDLER( powerins_flipscreen_w )
{
	if (ACCESSING_BITS_0_7)	flip_screen_set(space->machine,  data & 1 );
}

WRITE16_HANDLER( powerins_tilebank_w )
{
	if (ACCESSING_BITS_0_7)
	{
		if (data != tile_bank)
		{
			tile_bank = data;		// Tiles Bank (VRAM 0)
			tilemap_mark_all_tiles_dirty(tilemap_0);
		}
	}
}



/***************************************************************************

                                    Palette

***************************************************************************/


WRITE16_HANDLER( powerins_paletteram16_w )
{
	/*  byte 0    byte 1    */
	/*  RRRR GGGG BBBB RGBx */
	/*  4321 4321 4321 000x */

	UINT16 newword = COMBINE_DATA(&space->machine->generic.paletteram.u16[offset]);

	int r = ((newword >> 11) & 0x1E ) | ((newword >> 3) & 0x01);
	int g = ((newword >>  7) & 0x1E ) | ((newword >> 2) & 0x01);
	int b = ((newword >>  3) & 0x1E ) | ((newword >> 1) & 0x01);

	palette_set_color_rgb( space->machine,offset, pal5bit(r),pal5bit(g),pal5bit(b) );
}


/***************************************************************************

                        Callbacks for the TileMap code

***************************************************************************/


/***************************************************************************
                          [ Tiles Format VRAM 0]

Offset:

0.w     fedc ---- ---- ----     Color Low  Bits
        ---- b--- ---- ----     Color High Bit
        ---- -a98 7654 3210     Code (Banked)


***************************************************************************/

/* Layers are made of 256x256 pixel pages */
#define TILES_PER_PAGE_X	(0x10)
#define TILES_PER_PAGE_Y	(0x10)
#define TILES_PER_PAGE		(TILES_PER_PAGE_X * TILES_PER_PAGE_Y)

#define DIM_NX_0			(0x100)
#define DIM_NY_0			(0x20)


static TILE_GET_INFO( get_tile_info_0 )
{
	UINT16 code = powerins_vram_0[tile_index];
	SET_TILE_INFO(
			0,
			(code & 0x07ff) + (tile_bank*0x800),
			((code & 0xf000) >> (16-4)) + ((code & 0x0800) >> (11-4)),
			0);
}

WRITE16_HANDLER( powerins_vram_0_w )
{
	COMBINE_DATA(&powerins_vram_0[offset]);
	tilemap_mark_tile_dirty(tilemap_0, offset);
}

static TILEMAP_MAPPER( powerins_get_memory_offset_0 )
{
	return	(col * TILES_PER_PAGE_Y) +

			(row % TILES_PER_PAGE_Y) +
			(row / TILES_PER_PAGE_Y) * (TILES_PER_PAGE * 16);
}



/***************************************************************************
                          [ Tiles Format VRAM 1]

Offset:

0.w     fedc ---- ---- ----     Color
        ---- ba98 7654 3210     Code


***************************************************************************/

#define DIM_NX_1	(0x40)
#define DIM_NY_1	(0x20)

static TILE_GET_INFO( get_tile_info_1 )
{
	UINT16 code = powerins_vram_1[tile_index];
	SET_TILE_INFO(
			1,
			code & 0x0fff,
			(code & 0xf000) >> (16-4),
			0);
}

WRITE16_HANDLER( powerins_vram_1_w )
{
	COMBINE_DATA(&powerins_vram_1[offset]);
	tilemap_mark_tile_dirty(tilemap_1, offset);
}





/***************************************************************************


                                Vh_Start


***************************************************************************/

VIDEO_START( powerins )
{
	tilemap_0 = tilemap_create(	machine, get_tile_info_0,
								powerins_get_memory_offset_0,

								16,16,
								DIM_NX_0, DIM_NY_0 );

	tilemap_1 = tilemap_create(	machine, get_tile_info_1,
								tilemap_scan_cols,

								8,8,
								DIM_NX_1, DIM_NY_1 );

		tilemap_set_scroll_rows(tilemap_0,1);
		tilemap_set_scroll_cols(tilemap_0,1);

		tilemap_set_scroll_rows(tilemap_1,1);
		tilemap_set_scroll_cols(tilemap_1,1);
		tilemap_set_transparent_pen(tilemap_1,15);
}






/***************************************************************************


                                Sprites Drawing


***************************************************************************/



/* --------------------------[ Sprites Format ]----------------------------

Offset:     Format:                 Value:

    00      fedc ba98 7654 321-     -
            ---- ---- ---- ---0     Display this sprite

    02      fed- ---- ---- ----     -
            ---c ---- ---- ----     Flip X
            ---- ba9- ---- ----     -
            ---- ---8 ---- ----     Code High Bit
            ---- ---- 7654 ----     Number of tiles along Y, minus 1 (1-16)
            ---- ---- ---- 3210     Number of tiles along X, minus 1 (1-16)

    04                              Unused?

    06      f--- ---- ---- ----     -
            -edc ba98 7654 3210     Code Low Bits

    08                              X

    0A                              Unused?

    0C                              Y

    0E      fedc ba98 76-- ----     -
            ---- ---- --54 3210     Color


------------------------------------------------------------------------ */

#define SIGN_EXTEND_POS(_var_)	{_var_ &= 0x3ff; if (_var_ > 0x1ff) _var_ -= 0x400;}


static void draw_sprites(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect)
{
	UINT16 *source = machine->generic.spriteram.u16 + 0x8000/2;
	UINT16 *finish = machine->generic.spriteram.u16 + 0x9000/2;

	int screen_w = machine->primary_screen->width();
	int screen_h = machine->primary_screen->height();

	for ( ; source < finish; source += 16/2 )
	{
		int x,y,inc;

		int	attr	=	source[ 0x0/2 ];
		int	size	=	source[ 0x2/2 ];
		int	code	=	source[ 0x6/2 ];
		int	sx		=	source[ 0x8/2 ];
		int	sy		=	source[ 0xc/2 ];
		int	color	=	source[ 0xe/2 ];

		int	flipx	=	size & 0x1000;
		int	flipy	=	0;	// ??

		int	dimx	=	((size >> 0) & 0xf ) + 1;
		int	dimy	=	((size >> 4) & 0xf ) + 1;

		if (!(attr&1)) continue;

		SIGN_EXTEND_POS(sx)
		SIGN_EXTEND_POS(sy)

		/* Handle flip_screen. Apply a global offset of 32 pixels along x too */

		if (flip_screen_get(machine))
		{
			sx = screen_w - sx - dimx*16 - 32;	flipx = !flipx;
			sy = screen_h - sy - dimy*16;		flipy = !flipy;
			code += dimx*dimy-1;			inc = -1;
		}
		else
		{
			sx += 32;						inc = +1;
		}

		code = (code & 0x7fff) + ( (size & 0x0100) << 7 );

		for (x = 0 ; x < dimx ; x++)
		{
			for (y = 0 ; y < dimy ; y++)
			{
				drawgfx_transpen(bitmap,cliprect,machine->gfx[2],
						code,
						color,
						flipx, flipy,
						sx + x*16,
						sy + y*16,15);

				code += inc;
			}
		}


	}
}




/***************************************************************************


                                Screen Drawing


***************************************************************************/


VIDEO_UPDATE( powerins )
{
	int layers_ctrl = -1;

	int scrollx = (powerins_vctrl_0[2/2]&0xff) + (powerins_vctrl_0[0/2]&0xff)*256;
	int scrolly = (powerins_vctrl_0[6/2]&0xff) + (powerins_vctrl_0[4/2]&0xff)*256;

	tilemap_set_scrollx( tilemap_0, 0, scrollx - 0x20);
	tilemap_set_scrolly( tilemap_0, 0, scrolly );

	tilemap_set_scrollx( tilemap_1, 0, -0x20);	// fixed offset
	tilemap_set_scrolly( tilemap_1, 0,  0x00);

#ifdef MAME_DEBUG
if (input_code_pressed(screen->machine, KEYCODE_Z))
{
	int msk = 0;

	if (input_code_pressed(screen->machine, KEYCODE_Q))	msk |= 1;
	if (input_code_pressed(screen->machine, KEYCODE_W))	msk |= 2;
//  if (input_code_pressed(screen->machine, KEYCODE_E))    msk |= 4;
	if (input_code_pressed(screen->machine, KEYCODE_A))	msk |= 8;
	if (msk != 0) layers_ctrl &= msk;
}
#endif

	if (layers_ctrl&1)		tilemap_draw(bitmap,cliprect, tilemap_0, 0, 0);
	else					bitmap_fill(bitmap,cliprect,0);
	if (layers_ctrl&8)		draw_sprites(screen->machine,bitmap,cliprect);
	if (layers_ctrl&2)		tilemap_draw(bitmap,cliprect, tilemap_1, 0, 0);
	return 0;
}
