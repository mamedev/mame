/***************************************************************************

                              -= Blomby Car =-

                    driver by   Luca Elia (l.elia@tin.it)


Note:   if MAME_DEBUG is defined, pressing Z with:

        Q       shows the background
        W       shows the foreground
        A       shows the sprites

        Keys can be used together!


    [ 2 Scrolling Layers ]

    The Tilemaps are 64 x 32 tiles in size (1024 x 512).
    Tiles are 16 x 16 x 4, with 32 color codes and 2 priority
    leves (wrt sprites). Each tile needs 4 bytes.

    [ 1024? Sprites ]

    They use the same graphics the tilemaps use (16 x 16 x 4 tiles)
    with 16 color codes and 2 levels of priority


***************************************************************************/

#include "driver.h"

/* Variables needed by driver: */

UINT16 *blmbycar_vram_0, *blmbycar_scroll_0;
UINT16 *blmbycar_vram_1, *blmbycar_scroll_1;


/***************************************************************************


                                Palette


***************************************************************************/

/* xxxxBBBBGGGGRRRR */

WRITE16_HANDLER( blmbycar_palette_w )
{
	data = COMBINE_DATA(&paletteram16[offset]);
	palette_set_color_rgb( Machine, offset, pal4bit(data >> 4), pal4bit(data >> 0), pal4bit(data >> 8));
}



/***************************************************************************


                                Tilemaps

    Offset:     Bits:                   Value:

        0.w                             Code
        2.w     fedc ba98 ---- ----
                ---- ---- 7--- ----     Flip Y
                ---- ---- -6-- ----     Flip X
                ---- ---- --5- ----     Priority (0 = Low)
                ---- ---- ---4 3210     Color

***************************************************************************/

static tilemap *tilemap_0, *tilemap_1;

#define DIM_NX		(0x40)
#define DIM_NY		(0x20)

static TILE_GET_INFO( get_tile_info_0 )
{
	UINT16 code = blmbycar_vram_0[ tile_index * 2 + 0 ];
	UINT16 attr = blmbycar_vram_0[ tile_index * 2 + 1 ];
	SET_TILE_INFO(
			0,
			code,
			attr & 0x1f,
			TILE_FLIPYX((attr >> 6) & 3));

	tileinfo->category = (attr >> 5) & 1;
}

static TILE_GET_INFO( get_tile_info_1 )
{
	UINT16 code = blmbycar_vram_1[ tile_index * 2 + 0 ];
	UINT16 attr = blmbycar_vram_1[ tile_index * 2 + 1 ];
	SET_TILE_INFO(
			0,
			code,
			attr & 0x1f,
			TILE_FLIPYX((attr >> 6) & 3));

	tileinfo->category = (attr >> 5) & 1;
}


WRITE16_HANDLER( blmbycar_vram_0_w )
{
	COMBINE_DATA(&blmbycar_vram_0[offset]);
	tilemap_mark_tile_dirty(tilemap_0, offset/2);
}

WRITE16_HANDLER( blmbycar_vram_1_w )
{
	COMBINE_DATA(&blmbycar_vram_1[offset]);
	tilemap_mark_tile_dirty(tilemap_1, offset/2);
}


/***************************************************************************


                                Video Init


***************************************************************************/

VIDEO_START( blmbycar )
{
	tilemap_0 = tilemap_create(	get_tile_info_0, tilemap_scan_rows,
								TILEMAP_TYPE_PEN, 16,16, DIM_NX, DIM_NY );

	tilemap_1 = tilemap_create(	get_tile_info_1, tilemap_scan_rows,
								TILEMAP_TYPE_PEN, 16,16, DIM_NX, DIM_NY );

		tilemap_set_scroll_rows(tilemap_0,1);
		tilemap_set_scroll_cols(tilemap_0,1);

		tilemap_set_scroll_rows(tilemap_1,1);
		tilemap_set_scroll_cols(tilemap_1,1);
		tilemap_set_transparent_pen(tilemap_1,0);
}


/***************************************************************************


                                Sprites Drawing

    Offset:     Bits:                   Value:

        0.w     f--- ---- ---- ----     End Of Sprites
                -edc ba9- ---- ----
                ---- ---8 7654 3210     Y (Signed)

        2.w                             Code

        4.w     f--- ---- ---- ----     Flip Y
                -e-- ---- ---- ----     Flip X
                --dc ba98 7654 ----
                ---- ---- ---- 3210     Color (Bit 3 = Priority)

        6.w     f--- ---- ---- ----     ? Is this ever used ?
                -e-- ---- ---- ----     ? 1 = Don't Draw ?
                --dc ba9- ---- ----
                ---- ---8 7654 3210     X (Signed)


***************************************************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	UINT16 *source, *finish;

	source = spriteram16 + 0x6/2;				// !
	finish = spriteram16 + spriteram_size/2 - 8/2;

	/* Find "the end of sprites" marker */

	for ( ; source < finish; source += 8/2 )
		if (source[0] & 0x8000)	break;

	/* Draw sprites in reverse order for pdrawfgfx */

	source -= 8/2;
	finish = spriteram16;

	for ( ; source >= finish; source -= 8/2 )
	{
		int	y			=	source[0];
		int	code		=	source[1];
		int	attr		=	source[2];
		int	x			=	source[3];

		int	flipx		=	attr & 0x4000;
		int	flipy		=	attr & 0x8000;
		int	pri			=	(~attr >> 3) & 0x1;		// Priority (1 = Low)
		int pri_mask	=	~((1 << (pri+1)) - 1);	// Above the first "pri" levels

		if (x & 0x4000)	continue;	// ? To get rid of the "shadow" blocks

		x	=	(x & 0x1ff) - 0x10;
		y	=	0xf0 - ((y & 0xff)  - (y & 0x100));

		pdrawgfx(	bitmap, machine->gfx[0],
					code,
					0x20 + (attr & 0xf),
					flipx, flipy,
					x, y,
					cliprect, TRANSPARENCY_PEN,0,
					pri_mask	);
	}
}


/***************************************************************************


                                Screen Drawing


***************************************************************************/

VIDEO_UPDATE( blmbycar )
{
	int i,layers_ctrl = -1;

	tilemap_set_scrolly( tilemap_0, 0, blmbycar_scroll_0[ 0 ]);
	tilemap_set_scrollx( tilemap_0, 0, blmbycar_scroll_0[ 1 ]);

	tilemap_set_scrolly( tilemap_1, 0, blmbycar_scroll_1[ 0 ]+1);
	tilemap_set_scrollx( tilemap_1, 0, blmbycar_scroll_1[ 1 ]+5);

#ifdef MAME_DEBUG
if (input_code_pressed(KEYCODE_Z))
{
	int msk = 0;

	if (input_code_pressed(KEYCODE_Q))	msk |= 1;
	if (input_code_pressed(KEYCODE_W))	msk |= 2;
//  if (input_code_pressed(KEYCODE_E))    msk |= 4;
	if (input_code_pressed(KEYCODE_A))	msk |= 8;
	if (msk != 0) layers_ctrl &= msk;
}
#endif

	fillbitmap(priority_bitmap,0,cliprect);

	if (layers_ctrl&1)
		for (i = 0; i <= 1; i++)
			tilemap_draw(bitmap, cliprect, tilemap_0, i, i);
	else	fillbitmap(bitmap,machine->pens[0],cliprect);

	if (layers_ctrl&2)
		for (i = 0; i <= 1; i++)
			tilemap_draw(bitmap, cliprect, tilemap_1, i, i);

	if (layers_ctrl&8)
		draw_sprites(machine, bitmap, cliprect);
	return 0;
}
