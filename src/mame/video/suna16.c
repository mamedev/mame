/***************************************************************************

                            -=  SunA 16 Bit Games =-

                    driver by   Luca Elia (l.elia@tin.it)


    These games have only sprites, of a peculiar type:

    there is a region of memory where 32 pages of 32x32 tile codes can
    be written like a tilemap made of 32 pages of 256x256 pixels. Each
    tile uses 2 words.

    Sprites are rectangular regions of *tiles* fetched from there and
    sent to the screen. Each sprite uses 4 words, held where the last
    page of tiles would be.


                                [ Sprites Format ]


    Offset:     Bits:                   Value:

        0.w     fedc ---- ---- ----     Source Page (Low Bits)
                ---- ba98 ---- ----     Source Column (Bit 8 = Sprite Flip X)
                ---- ---- 7654 3210     Screen Y Position

        2.w     fedc ---- ---- ----     Tile Bank
                ---- ba-- ---- ----
                ---- --9- ---- ----     Source Page (High Bit)
                ---- ---8 7654 3210     Screen X Position

    $10000.w    fedc ba98 ---- ----
                ---- ---- 76-- ----     Sprite Size:
                                        00 -> 16 x 16   (2x2  tiles)
                                        01 -> 32 x 32   (4x4  tiles)
                                        10 -> 16 x 256  (2x32 tiles)
                                        11 -> 32 x 256  (4x32 tiles)
                ---- ---- --54 ----     ? (Bit 4 used by uballoon)
                ---- ---- ---- 3210     Source Row

    $10002.w                            -


                            [ Sprite's Tiles Format ]


    Offset:     Bits:                   Value:

        0.w     f--- ---- ---- ----     Flip Y
                -e-- ---- ---- ----     Flip X
                --dc ba98 7654 3210     Code

    $10000.w    fedc ba98 7654 ----
                ---- ---- ---- 3210     Color


***************************************************************************/

#include "emu.h"
#include "includes/suna16.h"

WRITE16_MEMBER(suna16_state::suna16_flipscreen_w)
{
	if (ACCESSING_BITS_0_7)
	{
		flip_screen_set(machine(),  data & 1 );
		m_color_bank = ( data & 4 ) >> 2;
	}
	if (data & ~(1|4))	logerror("CPU#0 PC %06X - Flip screen unknown bits: %04X\n", cpu_get_pc(&space.device()), data);
}

WRITE16_MEMBER(suna16_state::bestbest_flipscreen_w)
{
	if (ACCESSING_BITS_0_7)
	{
		flip_screen_set(machine(),  data & 0x10 );
		//m_color_bank = ( data & 0x07 );
	}
	if (data & ~(0x10))	logerror("CPU#0 PC %06X - Flip screen unknown bits: %04X\n", cpu_get_pc(&space.device()), data);
}


/***************************************************************************


                Banked Palette RAM. Format is xBBBBBGGGGRRRRR


***************************************************************************/

VIDEO_START( suna16 )
{
	suna16_state *state = machine.driver_data<suna16_state>();

	state->m_paletteram = auto_alloc_array(machine, UINT16, machine.total_colors());
}

READ16_MEMBER(suna16_state::suna16_paletteram16_r)
{

	return m_paletteram[offset + m_color_bank * 256];
}

WRITE16_MEMBER(suna16_state::suna16_paletteram16_w)
{

	offset += m_color_bank * 256;
	data = COMBINE_DATA(&m_paletteram[offset]);
	palette_set_color_rgb( machine(), offset, pal5bit(data >> 0),pal5bit(data >> 5),pal5bit(data >> 10));
}


/***************************************************************************


                                Sprites Drawing


***************************************************************************/

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, UINT16 *sprites, int gfx)
{
	suna16_state *state = machine.driver_data<suna16_state>();
	int offs;
	int max_x = machine.primary_screen->width() - 8;
	int max_y = machine.primary_screen->height() - 8;

	for ( offs = 0xfc00/2; offs < 0x10000/2 ; offs += 4/2 )
	{
		int srcpg, srcx,srcy, dimx,dimy;
		int tile_x, tile_xinc, tile_xstart;
		int tile_y, tile_yinc;
		int dx, dy;
		int flipx, y0;

		int y		=	sprites[ offs + 0 + 0x00000 / 2 ];
		int x		=	sprites[ offs + 1 + 0x00000 / 2 ];
		int dim 	=	sprites[ offs + 0 + 0x10000 / 2 ];

		int bank	=	(x >> 12) & 0xf;

		srcpg	=	((y & 0xf000) >> 12) + ((x & 0x0200) >> 5); // src page
		srcx	=	((y   >> 8) & 0xf) * 2; 					// src col
		srcy	=	((dim >> 0) & 0xf) * 2; 					// src row

		switch ( (dim >> 4) & 0xc )
		{
			case 0x0:	dimx = 2;	dimy =	2;	y0 = 0x100; break;
			case 0x4:	dimx = 4;	dimy =	4;	y0 = 0x100; break;
			case 0x8:	dimx = 2;	dimy = 32;	y0 = 0x130; break;
			default:
			case 0xc:	dimx = 4;	dimy = 32;	y0 = 0x120; break;
		}

		if (dimx==4)	{ flipx = srcx & 2; 	srcx &= ~2; }
		else			{ flipx = 0; }

		x = (x & 0xff) - (x & 0x100);
		y = (y0 - (y & 0xff) - dimy*8 ) & 0xff;

		if (flipx)	{ tile_xstart = dimx-1; tile_xinc = -1; }
		else		{ tile_xstart = 0;		tile_xinc = +1; }

		tile_y = 0; 	tile_yinc = +1;

		for (dy = 0; dy < dimy * 8; dy += 8)
		{
			tile_x = tile_xstart;

			for (dx = 0; dx < dimx * 8; dx += 8)
			{
				int addr	=	(srcpg * 0x20 * 0x20) +
								((srcx + tile_x) & 0x1f) * 0x20 +
								((srcy + tile_y) & 0x1f);

				int tile	=	sprites[ addr + 0x00000 / 2 ];
				int attr	=	sprites[ addr + 0x10000 / 2 ];

				int sx		=	x + dx;
				int sy		=	(y + dy) & 0xff;

				int tile_flipx	=	tile & 0x4000;
				int tile_flipy	=	tile & 0x8000;

				if (flipx)	tile_flipx = !tile_flipx;

				if (flip_screen_get(machine))
				{
					sx = max_x - sx;
					sy = max_y - sy;
					tile_flipx = !tile_flipx;
					tile_flipy = !tile_flipy;
				}

				drawgfx_transpen(	bitmap, cliprect,machine.gfx[gfx],
							(tile & 0x3fff) + bank*0x4000,
							attr + (state->m_color_bank << 4),
							tile_flipx, tile_flipy,
							sx, sy,15	);

				tile_x += tile_xinc;
			}

			tile_y += tile_yinc;
		}

	}

}


/***************************************************************************


                                Screen Drawing


***************************************************************************/

SCREEN_UPDATE_IND16( suna16 )
{
	suna16_state *state = screen.machine().driver_data<suna16_state>();

	/* Suna Quiz indicates the background is the last pen */
	bitmap.fill(0xff, cliprect);
	draw_sprites(screen.machine(), bitmap, cliprect, state->m_spriteram, 0);
	return 0;
}

SCREEN_UPDATE_IND16( bestbest )
{
	suna16_state *state = screen.machine().driver_data<suna16_state>();
	int layers_ctrl = -1;

#ifdef MAME_DEBUG
if (screen.machine().input().code_pressed(KEYCODE_Z))
{	int msk = 0;
	if (screen.machine().input().code_pressed(KEYCODE_Q))	msk |= 1;
	if (screen.machine().input().code_pressed(KEYCODE_W))	msk |= 2;
	if (msk != 0) layers_ctrl &= msk;
}
#endif

	/* Suna Quiz indicates the background is the last pen */
	bitmap.fill(0xff, cliprect);
	if (layers_ctrl & 1)	draw_sprites(screen.machine(), bitmap, cliprect, state->m_spriteram,  0);
	if (layers_ctrl & 2)	draw_sprites(screen.machine(), bitmap, cliprect, state->m_spriteram2, 1);
	return 0;
}
