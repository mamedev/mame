/***************************************************************************

                              -= Afega Games =-

                    driver by   Luca Elia (l.elia@tin.it)


Note:   if MAME_DEBUG is defined, pressing Z with:

        Q / W           Shows Layer 0 / 1
        A               Shows Sprites

        Keys can be used together!

    [ 2 Layers ]
                                [ Layer 0 ]     [ Layer 1 ]

        Tile Size:              16 x 16 x 4/8   8 x 8 x 4
        Layer Size (pixels):    1024 x 1024     256 x 256
        Layer Size (tiles):     64 x 64         32 x 32
        Scrolling:              Yes             No

        The layout is a bit weird. 16 consecutive tile codes define a
        vertical column. 16 columns form a page (256 x 256).
        Layer 0 is made of 4 x 4 pages. Layer 1 of just 1 page.

    [ 256 Sprites ]

        Sprites are made of 16 x 16 x 4 tiles. Size can vary from 1 to 16
        tiles both horizontally and vertically.
        Is there zooming ?

    [ Priorities ]

        The game only uses this scheme:

        Back -> Front:  Layer 0, Sprites, Layer 1

***************************************************************************/

#include "driver.h"
#ifdef MAME_DEBUG
#include "ui.h"
#endif

/* Variables needed by drivers: */

UINT16 *afega_vram_0, *afega_scroll_0;
UINT16 *afega_vram_1, *afega_scroll_1;


/***************************************************************************


                        Palette - RRRRGGGGBBBB????


***************************************************************************/

WRITE16_HANDLER( afega_palette_w )
{
	int r,g,b;
	data = COMBINE_DATA(&paletteram16[offset]);
	b = ((data & 0x00F0) >> 3 ) + ((data & 0x0002) >> 1);
	g = ((data & 0x0F00) >> 7 ) + ((data & 0x0004) >> 2);
	r = ((data & 0xF000) >> 11) + ((data & 0x0008) >> 3);
	palette_set_color_rgb( Machine, offset, pal5bit(r) , pal5bit(g) , pal5bit(b) );
}

/* This game uses 8 bit tiles, so it ignores the color codes and just
   uses the same 256 colors for every tile */
PALETTE_INIT( grdnstrm )
{
	int color, pen;
	for( color = 0; color < 16; color++ )
		for( pen = 0; pen < 256; pen++ )
			colortable[color * 256 + pen + 256*3] = 256*0 + pen;
}


/***************************************************************************

                                Tilemaps

    Offset:     Bits:                   Value:

        2.w     fedc ---- ---- ----     Color
                ---- ba98 7654 3210     Code

***************************************************************************/

#define TILES_PER_PAGE_X	(0x10)
#define TILES_PER_PAGE_Y	(0x10)
#define PAGES_PER_TMAP_X	(0x4)
#define PAGES_PER_TMAP_Y	(0x4)
#define FIREHAWK_PAGES_PER_TMAP_X	(0x1)
#define FIREHAWK_PAGES_PER_TMAP_Y	(0x1)
#define TWINACTN_TILES_PER_PAGE_X	(0x100)
#define TWINACTN_TILES_PER_PAGE_Y	(0x10)
#define TWINACTN_PAGES_PER_TMAP_X	(0x1)
#define TWINACTN_PAGES_PER_TMAP_Y	(0x1)


static TILEMAP_MAPPER( afega_tilemap_scan_pages )
{
	return	(row / TILES_PER_PAGE_Y) * TILES_PER_PAGE_X * TILES_PER_PAGE_Y * PAGES_PER_TMAP_X +
			(row % TILES_PER_PAGE_Y) +

			(col / TILES_PER_PAGE_X) * TILES_PER_PAGE_X * TILES_PER_PAGE_Y +
			(col % TILES_PER_PAGE_X) * TILES_PER_PAGE_Y;
}

static TILEMAP_MAPPER( twinactn_tilemap_scan_pages )
{
	return	(row / TWINACTN_TILES_PER_PAGE_Y) * TWINACTN_TILES_PER_PAGE_X * TWINACTN_TILES_PER_PAGE_Y * TWINACTN_PAGES_PER_TMAP_X +
			(row % TWINACTN_TILES_PER_PAGE_Y) +

			(col / TWINACTN_TILES_PER_PAGE_X) * TWINACTN_TILES_PER_PAGE_X * TWINACTN_TILES_PER_PAGE_Y +
			(col % TWINACTN_TILES_PER_PAGE_X) * TWINACTN_TILES_PER_PAGE_Y;
}

static TILEMAP_MAPPER( firehawk_tilemap_scan_pages )
{
	return	(row / TILES_PER_PAGE_Y) * TILES_PER_PAGE_X * TILES_PER_PAGE_Y * FIREHAWK_PAGES_PER_TMAP_X +
			(row % TILES_PER_PAGE_Y) +

			(col / TILES_PER_PAGE_X) * TILES_PER_PAGE_X * TILES_PER_PAGE_Y +
			(col % TILES_PER_PAGE_X) * TILES_PER_PAGE_Y;
}

static tilemap *tilemap_0, *tilemap_1;

static TILE_GET_INFO( get_tile_info_0 )
{
	UINT16 code = afega_vram_0[tile_index];
	SET_TILE_INFO(
			1,
			code,
			(code & 0xf000) >> 12,
			0);
}
static TILE_GET_INFO( get_tile_info_1 )
{
	UINT16 code = afega_vram_1[tile_index];
	SET_TILE_INFO(
			2,
			code,
			(code & 0xf000) >> 12,
			0);
}

WRITE16_HANDLER( afega_vram_0_w )
{
	COMBINE_DATA(&afega_vram_0[offset]);
	tilemap_mark_tile_dirty(tilemap_0,offset);
}
WRITE16_HANDLER( afega_vram_1_w )
{
	COMBINE_DATA(&afega_vram_1[offset]);
	tilemap_mark_tile_dirty(tilemap_1,offset);
}


/***************************************************************************


                            Video Hardware Init


***************************************************************************/

VIDEO_START( afega )
{
	tilemap_0 = tilemap_create(	get_tile_info_0, afega_tilemap_scan_pages,
								TILEMAP_TYPE_PEN,
								16,16,
								TILES_PER_PAGE_X*PAGES_PER_TMAP_X,TILES_PER_PAGE_Y*PAGES_PER_TMAP_Y);

	tilemap_1 = tilemap_create(	get_tile_info_1, tilemap_scan_cols,
								TILEMAP_TYPE_PEN,
								8,8,
								32,32);

	tilemap_set_transparent_pen(tilemap_1,0xf);
}

VIDEO_START( twinactn )
{
	tilemap_0 = tilemap_create(	get_tile_info_0, twinactn_tilemap_scan_pages,
								TILEMAP_TYPE_PEN,
								16,16,
								TWINACTN_TILES_PER_PAGE_X*TWINACTN_PAGES_PER_TMAP_X,TWINACTN_TILES_PER_PAGE_Y*TWINACTN_PAGES_PER_TMAP_Y);

	tilemap_1 = tilemap_create(	get_tile_info_1, tilemap_scan_cols,
								TILEMAP_TYPE_PEN,
								8,8,
								32,32);

	tilemap_set_transparent_pen(tilemap_1,0xf);
}

VIDEO_START( firehawk )
{
	tilemap_0 = tilemap_create(	get_tile_info_0, firehawk_tilemap_scan_pages,
								TILEMAP_TYPE_PEN,
								16,16,
								TILES_PER_PAGE_X*FIREHAWK_PAGES_PER_TMAP_X,TILES_PER_PAGE_Y*FIREHAWK_PAGES_PER_TMAP_Y);

	tilemap_1 = tilemap_create(	get_tile_info_1, tilemap_scan_cols,
								TILEMAP_TYPE_PEN,
								8,8,
								32,32);

	tilemap_set_transparent_pen(tilemap_1,0xf);
}


/***************************************************************************

                                Sprites Drawing

    Offset:     Bits:                   Value:

        0.w     fedc ba-- ---- ----
                ---- --9- ---- ----     Flip Y?
                ---- ---8 7654 3---
                ---- --------- -21-     Priority?
                ---- ---- ---- ---0     1 = Draw This Sprite

        2.w     fedc ba98 ---- ----
                ---- ---- 7654 ----     Number Of Tiles Along Y - 1
                ---- ---- ---- 3210     Number Of Tiles Along X - 1

        4.w

        6.w                             Code

        8.w     fedc ba98 ---- ----
                ---- ---- 7654 3210     X (Signed)

        A.w

        C.w     fedc ba98 ---- ----
                ---- ---- 7654 3210     Y (Signed)

        E.w     fedc ba98 7654 ----
                ---- ---- ---- 3210     Color


***************************************************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect, UINT16 attr_mask)
{
	int offs;

	int max_x		=	machine->screen[0].width;
	int max_y		=	machine->screen[0].height;

	for ( offs = 0; offs < spriteram_size/2; offs += 16/2 )
	{
		int attr, dim, code, sx, sy, color, flipx, flipy;

		int x, xnum, xstart, xend, xinc;
		int y, ynum, ystart, yend, yinc;

		attr	=		spriteram16[offs + 0x0/2];
		if (!(attr & attr_mask))	continue;
		dim		=		spriteram16[offs + 0x2/2];
		code	=		spriteram16[offs + 0x6/2];
		sx		=		spriteram16[offs + 0x8/2];
		sy		=		spriteram16[offs + 0xc/2];
		color	=		spriteram16[offs + 0xe/2];

		flipx	=		attr & 0x000;	// ?
		flipy	=		attr & 0x000;	// ?

		xnum		=		((dim >> 0) & 0xf) + 1;
		ynum		=		((dim >> 4) & 0xf) + 1;

		sx = (sx & 0xff) - (sx & 0x100);
		sy = (sy & 0xff) - (sy & 0x100);

		if (flip_screen_x)	{	flipx = !flipx;		sx = max_x - sx - xnum * 16;	}
		if (flip_screen_y)	{	flipy = !flipy;		sy = max_y - sy - ynum * 16;	}

		if (flipx)	{ xstart = xnum-1;  xend = -1;    xinc = -1; }
		else		{ xstart = 0;       xend = xnum;  xinc = +1; }

		if (flipy)	{ ystart = ynum-1;  yend = -1;    yinc = -1; }
		else		{ ystart = 0;       yend = ynum;  yinc = +1; }

		for (y = ystart; y != yend; y += yinc)
		{
			for (x = xstart; x != xend; x += xinc)
			{
				drawgfx( bitmap,machine->gfx[0],
								code++,
								color,
								flipx, flipy,
								sx + x * 16, sy + y * 16,
								cliprect,TRANSPARENCY_PEN,15 );
			}
		}

#ifdef MAME_DEBUG
#if 1
if (input_code_pressed(KEYCODE_X))
{	/* Display some info on each sprite */
	char buf[10];
	sprintf(buf, "%X",(spriteram16[offs + 0x0/2]&6)/2);
	ui_draw_text(buf, sy, sx);
}
#endif
#endif
	}
}



/***************************************************************************


                                Screen Drawing


***************************************************************************/

static void video_update(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect,
	int dsw_flipscreen,			// 1 = Horizontal and vertical screen flip are hardwired to 2 dip switches
	int xoffset, int yoffset,	// tilemap_0 offsets
	int attr_mask				// "sprite active" mask
	)
{
	int layers_ctrl = -1;

	if (dsw_flipscreen)
	{

		flip_screen_x_set(~readinputport(2) & 0x0100);
		flip_screen_y_set(~readinputport(2) & 0x0200);
	}

	tilemap_set_scrollx(tilemap_0, 0, afega_scroll_0[1] + xoffset);
	tilemap_set_scrolly(tilemap_0, 0, afega_scroll_0[0] + yoffset);

	tilemap_set_scrollx(tilemap_1, 0, afega_scroll_1[1]);
	tilemap_set_scrolly(tilemap_1, 0, afega_scroll_1[0]);

#ifdef MAME_DEBUG
if ( input_code_pressed(KEYCODE_Z) )
{	int msk = 0;
	if (input_code_pressed(KEYCODE_Q))	msk |= 1;
	if (input_code_pressed(KEYCODE_W))	msk |= 2;
	if (input_code_pressed(KEYCODE_A))	msk |= 4;
	if (msk != 0) layers_ctrl &= msk;	}
#endif

	if (layers_ctrl & 1)	tilemap_draw(bitmap,cliprect,tilemap_0,0,0);
	else					fillbitmap(bitmap,get_black_pen(machine),cliprect);

	if (layers_ctrl & 4) 	draw_sprites(machine,bitmap,cliprect, attr_mask);

	if (layers_ctrl & 2)	tilemap_draw(bitmap,cliprect,tilemap_1,0,0);
}

VIDEO_UPDATE( afega )		{	video_update(machine,bitmap,cliprect, 1, -0x100,+0x000, 0x0001);	return 0; }
VIDEO_UPDATE( bubl2000 )	{	video_update(machine,bitmap,cliprect, 0, -0x100,+0x000, 0x0001);	return 0; }	// no flipscreen support, I really would confirmation from the schematics
VIDEO_UPDATE( redhawkb )	{	video_update(machine,bitmap,cliprect, 0, +0x000,+0x100, 0x0001);	return 0; }
VIDEO_UPDATE( twinactn )	{	video_update(machine,bitmap,cliprect, 0, +0x000,+0x000, 0x0100);	return 0; }

VIDEO_UPDATE( firehawk )
{
	tilemap_set_scrolly(tilemap_0, 0, afega_scroll_1[1] + 0x100);
	tilemap_set_scrollx(tilemap_0, 0, afega_scroll_1[0]);

	tilemap_draw(bitmap,cliprect,tilemap_0,0,0);
	draw_sprites(machine,bitmap,cliprect,1);

	tilemap_draw(bitmap,cliprect,tilemap_1,0,0);
	return 0;
}
