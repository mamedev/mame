/***************************************************************************

                          -= Yun Sung 16 Bit Games =-

                    driver by   Luca Elia (l.elia@tin.it)


    [ 2 Scrolling Layers ]

    Tiles are 16 x 16 x 8. The layout of the tilemap is a bit weird:
    16 consecutive tile codes define a vertical column.
    16 columns form a page (256 x 256).
    The tilemap is made of 4 x 4 pages (1024 x 1024)

    [ 512? Sprites ]

    Sprites are 16 x 16 x 4 in size. There's RAM for 512, but
    the game just copies 384 entries.


***************************************************************************/

#include "driver.h"

/* Variables that driver has access to: */

UINT16 *yunsun16_vram_0,   *yunsun16_vram_1;
UINT16 *yunsun16_scroll_0, *yunsun16_scroll_1;
UINT16 *yunsun16_priority;


/***************************************************************************


                                    Tilemaps


***************************************************************************/

static tilemap *tilemap_0, *tilemap_1;

#define TMAP_GFX			(0)
#define TILES_PER_PAGE_X	(0x10)
#define TILES_PER_PAGE_Y	(0x10)
#define PAGES_PER_TMAP_X	(0x4)
#define PAGES_PER_TMAP_Y	(0x4)

static TILEMAP_MAPPER( yunsun16_tilemap_scan_pages )
{
	return	(row / TILES_PER_PAGE_Y) * TILES_PER_PAGE_X * TILES_PER_PAGE_Y * PAGES_PER_TMAP_X +
			(row % TILES_PER_PAGE_Y) +

			(col / TILES_PER_PAGE_X) * TILES_PER_PAGE_X * TILES_PER_PAGE_Y +
			(col % TILES_PER_PAGE_X) * TILES_PER_PAGE_Y;
}

static TILE_GET_INFO( get_tile_info_0 )
{
	UINT16 code = yunsun16_vram_0[ 2 * tile_index + 0 ];
	UINT16 attr = yunsun16_vram_0[ 2 * tile_index + 1 ];
	SET_TILE_INFO(
			TMAP_GFX,
			code,
			attr & 0xf,
			(attr & 0x20) ? TILE_FLIPX : 0);
}

static TILE_GET_INFO( get_tile_info_1 )
{
	UINT16 code = yunsun16_vram_1[ 2 * tile_index + 0 ];
	UINT16 attr = yunsun16_vram_1[ 2 * tile_index + 1 ];
	SET_TILE_INFO(
			TMAP_GFX,
			code,
			attr & 0xf,
			(attr & 0x20) ? TILE_FLIPX : 0);
}

WRITE16_HANDLER( yunsun16_vram_0_w )
{
	COMBINE_DATA(&yunsun16_vram_0[offset]);
	tilemap_mark_tile_dirty(tilemap_0,offset/2);
}

WRITE16_HANDLER( yunsun16_vram_1_w )
{
	COMBINE_DATA(&yunsun16_vram_1[offset]);
	tilemap_mark_tile_dirty(tilemap_1,offset/2);
}


/***************************************************************************


                            Video Hardware Init


***************************************************************************/

static int sprites_scrolldx, sprites_scrolldy;

VIDEO_START( yunsun16 )
{
	tilemap_0 = tilemap_create(	get_tile_info_0,yunsun16_tilemap_scan_pages,
								TILEMAP_TYPE_PEN,
								16,16,
								TILES_PER_PAGE_X*PAGES_PER_TMAP_X,TILES_PER_PAGE_Y*PAGES_PER_TMAP_Y);

	tilemap_1 = tilemap_create(	get_tile_info_1,yunsun16_tilemap_scan_pages,
								TILEMAP_TYPE_PEN,
								16,16,
								TILES_PER_PAGE_X*PAGES_PER_TMAP_X,TILES_PER_PAGE_Y*PAGES_PER_TMAP_Y);

	sprites_scrolldx = -0x40;
	sprites_scrolldy = -0x0f;
	tilemap_set_scrolldx(tilemap_0,-0x34,0);
	tilemap_set_scrolldx(tilemap_1,-0x38,0);

	tilemap_set_scrolldy(tilemap_0,-0x10,0);
	tilemap_set_scrolldy(tilemap_1,-0x10,0);

	tilemap_set_transparent_pen(tilemap_0,0xff);
	tilemap_set_transparent_pen(tilemap_1,0xff);
}


/***************************************************************************


                                Sprites Drawing


        0.w                             X

        2.w                             Y

        4.w                             Code

        6.w     fedc ba98 7--- ----
                ---- ---- -6-- ----     Flip Y
                ---- ---- --5- ----     Flip X
                ---- ---- ---4 3210     Color


***************************************************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect)
{
	int offs;

	int max_x		=	machine->screen[0].visarea.max_x+1;
	int max_y		=	machine->screen[0].visarea.max_y+1;

	int pri			=	*yunsun16_priority & 3;
	int pri_mask;

	switch( pri )
	{
		case 1:		pri_mask = (1<<1)|(1<<2)|(1<<3);	break;
		case 2:		pri_mask = (1<<2)|(1<<3);			break;
		case 3:
		default:	pri_mask = 0;
	}

	for ( offs = (spriteram_size-8)/2 ; offs >= 0; offs -= 8/2 )
	{
		int x		=	spriteram16[offs + 0];
		int y		=	spriteram16[offs + 1];
		int code	=	spriteram16[offs + 2];
		int attr	=	spriteram16[offs + 3];
		int flipx	=	attr & 0x20;
		int flipy	=	attr & 0x40;


		x	+=	sprites_scrolldx;
		y	+=	sprites_scrolldy;

		if (flip_screen)	// not used?
		{
			flipx = !flipx;		x = max_x - x - 16;
			flipy = !flipy;		y = max_y - y - 16;
		}

		pdrawgfx(	bitmap,machine->gfx[1],
					code,
					attr & 0x1f,
					flipx, flipy,
					x,y,
					cliprect,TRANSPARENCY_PEN,15,
					pri_mask	);
	}
}


/***************************************************************************


                                Screen Drawing


***************************************************************************/


VIDEO_UPDATE( yunsun16 )
{
	tilemap_set_scrollx(tilemap_0, 0, yunsun16_scroll_0[ 0 ]);
	tilemap_set_scrolly(tilemap_0, 0, yunsun16_scroll_0[ 1 ]);

	tilemap_set_scrollx(tilemap_1, 0, yunsun16_scroll_1[ 0 ]);
	tilemap_set_scrolly(tilemap_1, 0, yunsun16_scroll_1[ 1 ]);

//  popmessage("%04X", *yunsun16_priority);

	fillbitmap(priority_bitmap,0,cliprect);

	if((*yunsun16_priority & 0x0c) == 4)
	{
		/* The color of the this layer's transparent pen goes below everything */
		tilemap_draw(bitmap,cliprect,tilemap_0, TILEMAP_DRAW_OPAQUE, 0);

		tilemap_draw(bitmap,cliprect,tilemap_0, 0, 1);

		tilemap_draw(bitmap,cliprect,tilemap_1, 0, 2);
	}
	else if((*yunsun16_priority & 0x0c) == 8)
	{
		/* The color of the this layer's transparent pen goes below everything */
		tilemap_draw(bitmap,cliprect,tilemap_1, TILEMAP_DRAW_OPAQUE, 0);

		tilemap_draw(bitmap,cliprect,tilemap_1, 0, 1);

		tilemap_draw(bitmap,cliprect,tilemap_0, 0, 2);
	}

	draw_sprites(machine, bitmap,cliprect);
	return 0;
}
