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


/***************************************************************************

                        Hardware registers access

***************************************************************************/


WRITE16_MEMBER(powerins_state::powerins_flipscreen_w)
{
	if (ACCESSING_BITS_0_7)	flip_screen_set(machine(),  data & 1 );
}

WRITE16_MEMBER(powerins_state::powerins_tilebank_w)
{
	if (ACCESSING_BITS_0_7)
	{
		if (data != m_tile_bank)
		{
			m_tile_bank = data;		// Tiles Bank (VRAM 0)
			m_tilemap_0->mark_all_dirty();
		}
	}
}



/***************************************************************************

                                    Palette

***************************************************************************/


WRITE16_MEMBER(powerins_state::powerins_paletteram16_w)
{
	/*  byte 0    byte 1    */
	/*  RRRR GGGG BBBB RGBx */
	/*  4321 4321 4321 000x */

	UINT16 newword = COMBINE_DATA(&m_generic_paletteram_16[offset]);

	int r = ((newword >> 11) & 0x1E ) | ((newword >> 3) & 0x01);
	int g = ((newword >>  7) & 0x1E ) | ((newword >> 2) & 0x01);
	int b = ((newword >>  3) & 0x1E ) | ((newword >> 1) & 0x01);

	palette_set_color_rgb( machine(),offset, pal5bit(r),pal5bit(g),pal5bit(b) );
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
	powerins_state *state = machine.driver_data<powerins_state>();
	UINT16 code = state->m_vram_0[tile_index];
	SET_TILE_INFO(
			0,
			(code & 0x07ff) + (state->m_tile_bank*0x800),
			((code & 0xf000) >> (16-4)) + ((code & 0x0800) >> (11-4)),
			0);
}

WRITE16_MEMBER(powerins_state::powerins_vram_0_w)
{
	COMBINE_DATA(&m_vram_0[offset]);
	m_tilemap_0->mark_tile_dirty(offset);
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
	powerins_state *state = machine.driver_data<powerins_state>();
	UINT16 code = state->m_vram_1[tile_index];
	SET_TILE_INFO(
			1,
			code & 0x0fff,
			(code & 0xf000) >> (16-4),
			0);
}

WRITE16_MEMBER(powerins_state::powerins_vram_1_w)
{
	COMBINE_DATA(&m_vram_1[offset]);
	m_tilemap_1->mark_tile_dirty(offset);
}





/***************************************************************************


                                Vh_Start


***************************************************************************/

VIDEO_START( powerins )
{
	powerins_state *state = machine.driver_data<powerins_state>();
	state->m_tilemap_0 = tilemap_create(	machine, get_tile_info_0,
								powerins_get_memory_offset_0,

								16,16,
								DIM_NX_0, DIM_NY_0 );

	state->m_tilemap_1 = tilemap_create(	machine, get_tile_info_1,
								tilemap_scan_cols,

								8,8,
								DIM_NX_1, DIM_NY_1 );

		state->m_tilemap_0->set_scroll_rows(1);
		state->m_tilemap_0->set_scroll_cols(1);

		state->m_tilemap_1->set_scroll_rows(1);
		state->m_tilemap_1->set_scroll_cols(1);
		state->m_tilemap_1->set_transparent_pen(15);
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


static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	powerins_state *state = machine.driver_data<powerins_state>();
	UINT16 *source = state->m_spriteram + 0x8000/2;
	UINT16 *finish = state->m_spriteram + 0x9000/2;

	int screen_w = machine.primary_screen->width();
	int screen_h = machine.primary_screen->height();

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
				drawgfx_transpen(bitmap,cliprect,machine.gfx[2],
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


SCREEN_UPDATE_IND16( powerins )
{
	powerins_state *state = screen.machine().driver_data<powerins_state>();
	int layers_ctrl = -1;

	int scrollx = (state->m_vctrl_0[2/2]&0xff) + (state->m_vctrl_0[0/2]&0xff)*256;
	int scrolly = (state->m_vctrl_0[6/2]&0xff) + (state->m_vctrl_0[4/2]&0xff)*256;

	state->m_tilemap_0->set_scrollx(0, scrollx - 0x20);
	state->m_tilemap_0->set_scrolly(0, scrolly );

	state->m_tilemap_1->set_scrollx(0, -0x20);	// fixed offset
	state->m_tilemap_1->set_scrolly(0,  0x00);

#ifdef MAME_DEBUG
if (screen.machine().input().code_pressed(KEYCODE_Z))
{
	int msk = 0;

	if (screen.machine().input().code_pressed(KEYCODE_Q))	msk |= 1;
	if (screen.machine().input().code_pressed(KEYCODE_W))	msk |= 2;
//  if (screen.machine().input().code_pressed(KEYCODE_E))    msk |= 4;
	if (screen.machine().input().code_pressed(KEYCODE_A))	msk |= 8;
	if (msk != 0) layers_ctrl &= msk;
}
#endif

	if (layers_ctrl&1)		state->m_tilemap_0->draw(bitmap, cliprect, 0, 0);
	else					bitmap.fill(0, cliprect);
	if (layers_ctrl&8)		draw_sprites(screen.machine(),bitmap,cliprect);
	if (layers_ctrl&2)		state->m_tilemap_1->draw(bitmap, cliprect, 0, 0);
	return 0;
}
