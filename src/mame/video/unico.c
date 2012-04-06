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

#include "emu.h"
#include "includes/unico.h"


/***************************************************************************

                                    Palette

    Byte:   0   1   2   3
    Gun:    R   G   B   0

    6 Bits x Gun

***************************************************************************/

WRITE16_MEMBER(unico_state::unico_palette_w)
{
	UINT16 data1, data2;
	COMBINE_DATA(&m_generic_paletteram_16[offset]);
	data1 = m_generic_paletteram_16[offset & ~1];
	data2 = m_generic_paletteram_16[offset |  1];
	palette_set_color_rgb( machine(),offset/2,
		 (data1 >> 8) & 0xFC,
		 (data1 >> 0) & 0xFC,
		 (data2 >> 8) & 0xFC	);
}

WRITE32_MEMBER(unico_state::unico_palette32_w)
{
	UINT32 rgb0 = COMBINE_DATA(&m_generic_paletteram_32[offset]);
	palette_set_color_rgb( machine(),offset,
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


static TILE_GET_INFO( get_tile_info )
{
	UINT16 *vram = (UINT16 *)param;
	UINT16 code = vram[2 * tile_index + 0 ];
	UINT16 attr = vram[2 * tile_index + 1 ];
	SET_TILE_INFO(1, code, attr & 0x1f, TILE_FLIPYX( attr >> 5 ));
}

static TILE_GET_INFO( get_tile_info32 )
{
	UINT32 *vram = (UINT32 *)param;
	UINT16 code = vram[tile_index] >> 16;
	UINT16 attr = vram[tile_index] & 0xff;
	SET_TILE_INFO(1, code, attr & 0x1f, TILE_FLIPYX( attr >> 5 ));
}

WRITE16_MEMBER(unico_state::unico_vram_w)
{
	UINT16 *vram = m_vram;
	int tile = ((offset / 0x2000) + 1) % 3;
	COMBINE_DATA(&vram[offset]);
	m_tilemap[tile]->mark_tile_dirty((offset & 0x3fff)/2);
}

WRITE32_MEMBER(unico_state::unico_vram32_w)
{
	UINT32 *vram = m_vram32;
	int tile = ((offset / 0x1000) + 1) % 3;
	COMBINE_DATA(&vram[offset]);
	m_tilemap[tile]->mark_tile_dirty((offset & 0x3fff));
}



/***************************************************************************


                            Video Hardware Init


***************************************************************************/


VIDEO_START( unico )
{
	unico_state *state = machine.driver_data<unico_state>();
	state->m_tilemap[0] = tilemap_create(	machine, get_tile_info,tilemap_scan_rows,
									16,16,	0x40, 0x40);

	state->m_tilemap[1] = tilemap_create(	machine, get_tile_info,tilemap_scan_rows,
									16,16,	0x40, 0x40);

	state->m_tilemap[2] = tilemap_create(	machine, get_tile_info,tilemap_scan_rows,
									16,16,	0x40, 0x40);

	state->m_tilemap[0]->set_user_data(&state->m_vram[0x8000/2]);
	state->m_tilemap[1]->set_user_data(&state->m_vram[0x0000/2]);
	state->m_tilemap[2]->set_user_data(&state->m_vram[0x4000/2]);

	state->m_sprites_scrolldx = -0x3f;
	state->m_sprites_scrolldy = -0x0e;

	state->m_tilemap[0]->set_scrolldx(-0x32,0);
	state->m_tilemap[1]->set_scrolldx(-0x30,0);
	state->m_tilemap[2]->set_scrolldx(-0x2e,0);

	state->m_tilemap[0]->set_scrolldy(-0x0f,0);
	state->m_tilemap[1]->set_scrolldy(-0x0f,0);
	state->m_tilemap[2]->set_scrolldy(-0x0f,0);

	state->m_tilemap[0]->set_transparent_pen(0x00);
	state->m_tilemap[1]->set_transparent_pen(0x00);
	state->m_tilemap[2]->set_transparent_pen(0x00);
}

VIDEO_START( zeropnt2 )
{
	unico_state *state = machine.driver_data<unico_state>();
	state->m_tilemap[0] = tilemap_create(	machine, get_tile_info32,tilemap_scan_rows,
									16,16,	0x40, 0x40);

	state->m_tilemap[1] = tilemap_create(	machine, get_tile_info32,tilemap_scan_rows,
									16,16,	0x40, 0x40);

	state->m_tilemap[2] = tilemap_create(	machine, get_tile_info32,tilemap_scan_rows,
									16,16,	0x40, 0x40);

	state->m_tilemap[0]->set_user_data(&state->m_vram32[0x8000/4]);
	state->m_tilemap[1]->set_user_data(&state->m_vram32[0x0000/4]);
	state->m_tilemap[2]->set_user_data(&state->m_vram32[0x4000/4]);

	state->m_sprites_scrolldx = -0x3f;
	state->m_sprites_scrolldy = -0x0e;

	state->m_tilemap[0]->set_scrolldx(-0x32,0);
	state->m_tilemap[1]->set_scrolldx(-0x30,0);
	state->m_tilemap[2]->set_scrolldx(-0x2e,0);

	state->m_tilemap[0]->set_scrolldy(-0x0f,0);
	state->m_tilemap[1]->set_scrolldy(-0x0f,0);
	state->m_tilemap[2]->set_scrolldy(-0x0f,0);

	state->m_tilemap[0]->set_transparent_pen(0x00);
	state->m_tilemap[1]->set_transparent_pen(0x00);
	state->m_tilemap[2]->set_transparent_pen(0x00);
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

static void unico_draw_sprites(running_machine &machine, bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	unico_state *state = machine.driver_data<unico_state>();
	UINT16 *spriteram16 = state->m_spriteram;
	int offs;

	/* Draw them backwards, for pdrawgfx */
	for ( offs = (state->m_spriteram_size-8)/2; offs >= 0 ; offs -= 8/2 )
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

		sx	+=	state->m_sprites_scrolldx;
		sy	+=	state->m_sprites_scrolldy;

		sx	=	(sx & 0x1ff) - (sx & 0x200);
		sy	=	(sy & 0x1ff) - (sy & 0x200);

		if (flipx)	{	startx = sx+(dimx-1)*16;	endx = sx-16;		incx = -16;	}
		else		{	startx = sx;				endx = sx+dimx*16;	incx = +16;	}

		for (x = startx ; x != endx ; x += incx)
		{
			pdrawgfx_transpen(	bitmap, cliprect, machine.gfx[0],
						code++,
						attr & 0x1f,
						flipx, flipy,
						x, sy,
						machine.priority_bitmap,
						pri_mask,0x00	);
		}
	}
}

static void zeropnt2_draw_sprites(running_machine &machine, bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	unico_state *state = machine.driver_data<unico_state>();
	UINT32 *spriteram32 = (UINT32 *)state->m_spriteram;
	int offs;

	/* Draw them backwards, for pdrawgfx */
	for ( offs = (state->m_spriteram_size-8)/4; offs >= 0 ; offs -= 8/4 )
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

		sx	+=	state->m_sprites_scrolldx;
		sy	+=	state->m_sprites_scrolldy;

		sx	=	(sx & 0x1ff) - (sx & 0x200);
		sy	=	(sy & 0x1ff) - (sy & 0x200);

		if (flipx)	{	startx = sx+(dimx-1)*16;	endx = sx-16;		incx = -16;	}
		else		{	startx = sx;				endx = sx+dimx*16;	incx = +16;	}

		for (x = startx ; x != endx ; x += incx)
		{
			pdrawgfx_transpen(	bitmap, cliprect, machine.gfx[0],
						code++,
						attr & 0x1f,
						flipx, flipy,
						x, sy,
						machine.priority_bitmap,
						pri_mask,0x00	);
		}
	}
}



/***************************************************************************


                                Screen Drawing


***************************************************************************/

SCREEN_UPDATE_IND16( unico )
{
	unico_state *state = screen.machine().driver_data<unico_state>();
	int layers_ctrl = -1;

	state->m_tilemap[0]->set_scrollx(0, state->m_scroll[0x00]);
	state->m_tilemap[0]->set_scrolly(0, state->m_scroll[0x01]);

	state->m_tilemap[1]->set_scrollx(0, state->m_scroll[0x05]);
	state->m_tilemap[1]->set_scrolly(0, state->m_scroll[0x0a]);

	state->m_tilemap[2]->set_scrollx(0, state->m_scroll[0x04]);
	state->m_tilemap[2]->set_scrolly(0, state->m_scroll[0x02]);

#ifdef MAME_DEBUG
if ( screen.machine().input().code_pressed(KEYCODE_Z) || screen.machine().input().code_pressed(KEYCODE_X) )
{
	int msk = 0;
	if (screen.machine().input().code_pressed(KEYCODE_Q))	msk |= 1;
	if (screen.machine().input().code_pressed(KEYCODE_W))	msk |= 2;
	if (screen.machine().input().code_pressed(KEYCODE_E))	msk |= 4;
	if (screen.machine().input().code_pressed(KEYCODE_A))	msk |= 8;
	if (msk != 0) layers_ctrl &= msk;
}
#endif

	/* The background color is the first of the last palette */
	bitmap.fill(0x1f00, cliprect);
	screen.machine().priority_bitmap.fill(0, cliprect);

	if (layers_ctrl & 1)	state->m_tilemap[0]->draw(bitmap, cliprect, 0,1);
	if (layers_ctrl & 2)	state->m_tilemap[1]->draw(bitmap, cliprect, 0,2);
	if (layers_ctrl & 4)	state->m_tilemap[2]->draw(bitmap, cliprect, 0,4);

	/* Sprites are drawn last, using pdrawgfx */
	if (layers_ctrl & 8)	unico_draw_sprites(screen.machine(), bitmap,cliprect);

	return 0;
}

SCREEN_UPDATE_IND16( zeropnt2 )
{
	unico_state *state = screen.machine().driver_data<unico_state>();
	int layers_ctrl = -1;

	state->m_tilemap[0]->set_scrollx(0, state->m_scroll32[0] >> 16);
	state->m_tilemap[0]->set_scrolly(0, state->m_scroll32[0] & 0xffff);

	state->m_tilemap[1]->set_scrollx(0, state->m_scroll32[2] & 0xffff);
	state->m_tilemap[1]->set_scrolly(0, state->m_scroll32[5] >> 16);

	state->m_tilemap[2]->set_scrollx(0, state->m_scroll32[2] >> 16);
	state->m_tilemap[2]->set_scrolly(0, state->m_scroll32[1] >> 16);

#ifdef MAME_DEBUG
if ( screen.machine().input().code_pressed(KEYCODE_Z) || screen.machine().input().code_pressed(KEYCODE_X) )
{
	int msk = 0;
	if (screen.machine().input().code_pressed(KEYCODE_Q))	msk |= 1;
	if (screen.machine().input().code_pressed(KEYCODE_W))	msk |= 2;
	if (screen.machine().input().code_pressed(KEYCODE_E))	msk |= 4;
	if (screen.machine().input().code_pressed(KEYCODE_A))	msk |= 8;
	if (msk != 0) layers_ctrl &= msk;
}
#endif

	/* The background color is the first of the last palette */
	bitmap.fill(0x1f00, cliprect);
	screen.machine().priority_bitmap.fill(0, cliprect);

	if (layers_ctrl & 1)	state->m_tilemap[0]->draw(bitmap, cliprect, 0,1);
	if (layers_ctrl & 2)	state->m_tilemap[1]->draw(bitmap, cliprect, 0,2);
	if (layers_ctrl & 4)	state->m_tilemap[2]->draw(bitmap, cliprect, 0,4);

	/* Sprites are drawn last, using pdrawgfx */
	if (layers_ctrl & 8)	zeropnt2_draw_sprites(screen.machine(), bitmap,cliprect);

	return 0;
}

