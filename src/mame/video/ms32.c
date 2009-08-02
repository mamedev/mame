/* Jaleco MegaSystem 32 Video Hardware */

/* The Video Hardware is Similar to the Non-MS32 Version of Tetris Plus 2 */

/* Plenty to do, see list in drivers/ms32.c */

/*

priority should be given to
(a) dekluding the priorities, the kludge for kirarast made it easier to emulate the rest of it until then
(b) working out the background registers correctly ...
*/


#include "driver.h"
#include "includes/ms32.h"


//UINT32 *ms32_fce00000;
UINT32 *ms32_roz_ctrl;
UINT32 *ms32_tx_scroll;
UINT32 *ms32_bg_scroll;
UINT32 *ms32_priram;
UINT32 *ms32_palram;
UINT32 *ms32_bgram;
UINT32 *ms32_rozram;
UINT32 *ms32_lineram;
UINT32 *ms32_spram;
UINT32 *ms32_txram;
UINT32 *ms32_mainram;

// kirarast, tp2m32, and 47pie2 require the sprites in a different order
static int ms32_reverse_sprite_order;

/********** Tilemaps **********/

static tilemap *ms32_tx_tilemap, *ms32_roz_tilemap, *ms32_bg_tilemap;
static int flipscreen;


static TILE_GET_INFO( get_ms32_tx_tile_info )
{
	int tileno, colour;

	tileno = ms32_txram[tile_index *2] & 0x0000ffff;
	colour = ms32_txram[tile_index *2+1] & 0x000000f;

	SET_TILE_INFO(3,tileno,colour,0);
}

static TILE_GET_INFO( get_ms32_roz_tile_info )
{
	int tileno,colour;

	tileno = ms32_rozram[tile_index *2] & 0x0000ffff;
	colour = ms32_rozram[tile_index *2+1] & 0x000000f;

	SET_TILE_INFO(1,tileno,colour,0);
}

static TILE_GET_INFO( get_ms32_bg_tile_info )
{
	int tileno,colour;

	tileno = ms32_bgram[tile_index *2] & 0x0000ffff;
	colour = ms32_bgram[tile_index *2+1] & 0x000000f;

	SET_TILE_INFO(2,tileno,colour,0);
}

static UINT32 brt[4];
static int brt_r,brt_g,brt_b;

VIDEO_START( ms32 )
{
	ms32_tx_tilemap = tilemap_create(machine, get_ms32_tx_tile_info,tilemap_scan_rows,8, 8,64,64);
	ms32_bg_tilemap = tilemap_create(machine, get_ms32_bg_tile_info,tilemap_scan_rows,16,16,64,64);
	ms32_roz_tilemap = tilemap_create(machine, get_ms32_roz_tile_info,tilemap_scan_rows,16,16,128,128);

	tilemap_set_transparent_pen(ms32_tx_tilemap,0);
	tilemap_set_transparent_pen(ms32_bg_tilemap,0);
	tilemap_set_transparent_pen(ms32_roz_tilemap,0);

	ms32_reverse_sprite_order = 1;

	/* i hate per game patches...how should priority really work? tetrisp2.c ? i can't follow it */
	if (!strcmp(machine->gamedrv->name,"kirarast"))	ms32_reverse_sprite_order = 0;
	if (!strcmp(machine->gamedrv->name,"tp2m32"))	ms32_reverse_sprite_order = 0;
	if (!strcmp(machine->gamedrv->name,"47pie2"))	ms32_reverse_sprite_order = 0;
	if (!strcmp(machine->gamedrv->name,"47pie2o"))	ms32_reverse_sprite_order = 0;
	if (!strcmp(machine->gamedrv->name,"hayaosi3"))	ms32_reverse_sprite_order = 0;
	if (!strcmp(machine->gamedrv->name,"bnstars"))	ms32_reverse_sprite_order = 0;
	if (!strcmp(machine->gamedrv->name,"wpksocv2"))	ms32_reverse_sprite_order = 0;

	// tp2m32 doesn't set the brightness registers so we need sensible defaults
	brt[0] = brt[1] = 0xffff;
}

/********** PALETTE WRITES **********/


static void update_color(running_machine *machine, int color)
{
	int r,g,b;

	/* I'm not sure how the brightness should be applied, currently I'm only
       affecting bg & sprites, not fg.
       The second brightness control might apply to shadows, see gametngk.
     */
	if (~color & 0x4000)
	{
		r = ((ms32_palram[color*2] & 0xff00) >>8 ) * brt_r / 0x100;
		g = ((ms32_palram[color*2] & 0x00ff) >>0 ) * brt_g / 0x100;
		b = ((ms32_palram[color*2+1] & 0x00ff) >>0 ) * brt_b / 0x100;
	}
	else
	{
		r = ((ms32_palram[color*2] & 0xff00) >>8 );
		g = ((ms32_palram[color*2] & 0x00ff) >>0 );
		b = ((ms32_palram[color*2+1] & 0x00ff) >>0 );
	}

	palette_set_color(machine,color,MAKE_RGB(r,g,b));
}

WRITE32_HANDLER( ms32_brightness_w )
{
	int oldword = brt[offset];
	COMBINE_DATA(&brt[offset]);

	if (brt[offset] != oldword)
	{
		int bank = ((offset & 2) >> 1) * 0x4000;
		int i;

		if (bank == 0)
		{
			brt_r = 0x100 - ((brt[0] & 0xff00) >> 8);
			brt_g = 0x100 - ((brt[0] & 0x00ff) >> 0);
			brt_b = 0x100 - ((brt[1] & 0x00ff) >> 0);

			for (i = 0;i < 0x3000;i++)	// colors 0x3000-0x3fff are not used
				update_color(space->machine, i);
		}
	}

//popmessage("%04x %04x %04x %04x",brt[0],brt[1],brt[2],brt[3]);
}

WRITE32_HANDLER( ms32_palram_w )
{
	COMBINE_DATA(&ms32_palram[offset]);

	update_color(space->machine, offset/2);
}



READ32_HANDLER( ms32_txram_r )
{
	return ms32_txram[offset];
}

WRITE32_HANDLER( ms32_txram_w )
{
	COMBINE_DATA(&ms32_txram[offset]);
	tilemap_mark_tile_dirty(ms32_tx_tilemap,offset/2);
}

READ32_HANDLER( ms32_rozram_r )
{
	return ms32_rozram[offset];
}

WRITE32_HANDLER( ms32_rozram_w )
{
	COMBINE_DATA(&ms32_rozram[offset]);
	tilemap_mark_tile_dirty(ms32_roz_tilemap,offset/2);
}

READ32_HANDLER( ms32_lineram_r )
{
	return ms32_lineram[offset];
}

WRITE32_HANDLER( ms32_lineram_w )
{
	COMBINE_DATA(&ms32_lineram[offset]);
}

READ32_HANDLER( ms32_bgram_r )
{
	return ms32_bgram[offset];
}

WRITE32_HANDLER( ms32_bgram_w )
{
	COMBINE_DATA(&ms32_bgram[offset]);
	tilemap_mark_tile_dirty(ms32_bg_tilemap,offset/2);
}

READ32_HANDLER( ms32_spram_r )
{
	return ms32_spram[offset];
}

WRITE32_HANDLER( ms32_spram_w )
{
	COMBINE_DATA(&ms32_spram[offset]);
}

READ32_HANDLER( ms32_priram_r )
{
	return ms32_priram[offset];
}

WRITE32_HANDLER( ms32_priram_w )
{
	COMBINE_DATA(&ms32_priram[offset]);
}

WRITE32_HANDLER( ms32_gfxctrl_w )
{
	if (ACCESSING_BITS_0_7)
	{
		/* bit 1 = flip screen */
		flipscreen = data & 0x02;
		tilemap_set_flip(ms32_tx_tilemap,flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
		tilemap_set_flip(ms32_bg_tilemap,flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

		/* bit 2 used by f1superb, unknown */

		/* bit 3 used by several games, unknown */

//popmessage("%08x",data);
	}
}



/* SPRITES based on tetrisp2 for now, readd priority bits later */

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, UINT32 *sprram_top, size_t sprram_size)
{
/***************************************************************************


                                Sprites Drawing

    Offset:     Bits:                   Meaning:

    0.w         fedc ba98 ---- ----
                ---- ---- 7654 ----     Priority
                ---- ---- ---- 3---
                ---- ---- ---- -2--     Draw this sprite
                ---- ---- ---- --1-     Flip Y
                ---- ---- ---- ---0     Flip X

    1.w         fedc ba98 ---- ----     Tile's Y position in the tile page (*)
                ---- ---- 7654 3210     Tile's X position in the tile page (*)

    2.w         fedc ---- ---- ----     Color
                ---- ba98 7654 3210     Tile Page (32x32 tiles = 256x256 pixels each)

    3.w         fedc ba98 ---- ----     Y Size - 1 (*)
                ---- ---- 7654 3210     X Size - 1 (*)

    4.w         fedc ba-- ---- ----
                ---- --98 7654 3210     Y (Signed)

    5.w         fedc b--- ---- ----
                ---- -a98 7654 3210     X (Signed)

    6.w         fedc ba98 7654 3210     Zoom Y

    7.w         fedc ba98 7654 3210     Zoom X

(*) 1 pixel granularity

***************************************************************************/

	int tx, ty, sx, sy, flipx, flipy;
	int xsize, ysize, xzoom, yzoom;
	int code, attr, color, size, pri, pri_mask;
	gfx_element *gfx = machine->gfx[0];

	UINT32		*source	= sprram_top;
	const UINT32	*finish	= sprram_top + (sprram_size - 0x20) / 4;


	if (ms32_reverse_sprite_order == 1)
	{
		source	= sprram_top + (sprram_size - 0x20) / 4;
		finish	= sprram_top;
	}


	for (;ms32_reverse_sprite_order ? (source>=finish) : (source<finish); ms32_reverse_sprite_order ? (source-=8) : (source+=8))
	{
		attr	=	source[ 0 ];

		if ((attr & 0x0004) == 0)			continue;

		flipx	=	attr & 1;
		flipy	=	attr & 2;

		pri = (attr >> 4)&0xf;

		code	=	source[ 1 ];
		color	=	source[ 2 ];

		tx		=	(code >> 0) & 0xff;
		ty		=	(code >> 8) & 0xff;

		code	=	(color & 0x0fff);

		color	=	(color >> 12) & 0xf;

		size	=	source[ 3 ];

		xsize	=	((size >> 0) & 0xff) + 1;
		ysize	=	((size >> 8) & 0xff) + 1;

		sy		=	source[ 4 ];
		sx		=	source[ 5 ];

		sx		=	(sx & 0x3ff) - (sx & 0x400);
		sy		=	(sy & 0x1ff) - (sy & 0x200);

		xzoom	=	(source[ 6 ]&0xffff);
		yzoom	=	(source[ 7 ]&0xffff);

		if (!yzoom || !xzoom)				continue;

		yzoom = 0x1000000/yzoom;
		xzoom = 0x1000000/xzoom;

		// there are surely also shadows (see gametngk) but how they're enabled we don't know

		if (flipscreen)
		{
			sx = 320 - ((xsize*xzoom)>>16) - sx;
			sy = 224 - ((ysize*yzoom)>>16) - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

#if 0
if (input_code_pressed(KEYCODE_A) && (pri & 8)) color = rand();
if (input_code_pressed(KEYCODE_S) && (pri & 4)) color = rand();
if (input_code_pressed(KEYCODE_D) && (pri & 2)) color = rand();
if (input_code_pressed(KEYCODE_F) && (pri & 1)) color = rand();
#endif

		/* TODO: priority handling is completely wrong, but better than nothing */
		if (pri == 0x0)
			pri_mask = 0x00;
		else if (pri <= 0xd)
			pri_mask = 0xf0;
		else if (pri <= 0xe)
			pri_mask = 0xfc;
		else
			pri_mask = 0xfe;

		gfx_element_set_source_clip(gfx, tx, xsize, ty, ysize);
		pdrawgfxzoom_transpen(bitmap, cliprect, gfx,
				code,
				color,
				flipx, flipy,
				sx,sy,
				xzoom, yzoom, machine->priority_bitmap,pri_mask, 0);
	}	/* end sprite loop */
}



static void draw_roz(bitmap_t *bitmap, const rectangle *cliprect,int priority)
{
	/* TODO: registers 0x40/4 / 0x44/4 and 0x50/4 / 0x54/4 are used, meaning unknown */

	if (ms32_roz_ctrl[0x5c/4] & 1)	/* "super" mode */
	{
		rectangle my_clip;
		int y,maxy;

		my_clip.min_x = cliprect->min_x;
		my_clip.max_x = cliprect->max_x;

		y = cliprect->min_y;
		maxy = cliprect->max_y;

		while (y <= maxy)
		{
			UINT32 *lineaddr = ms32_lineram + 8 * (y & 0xff);

			int start2x = (lineaddr[0x00/4] & 0xffff) | ((lineaddr[0x04/4] & 3) << 16);
			int start2y = (lineaddr[0x08/4] & 0xffff) | ((lineaddr[0x0c/4] & 3) << 16);
			int incxx  = (lineaddr[0x10/4] & 0xffff) | ((lineaddr[0x14/4] & 1) << 16);
			int incxy  = (lineaddr[0x18/4] & 0xffff) | ((lineaddr[0x1c/4] & 1) << 16);
			int startx = (ms32_roz_ctrl[0x00/4] & 0xffff) | ((ms32_roz_ctrl[0x04/4] & 3) << 16);
			int starty = (ms32_roz_ctrl[0x08/4] & 0xffff) | ((ms32_roz_ctrl[0x0c/4] & 3) << 16);
			int offsx  = ms32_roz_ctrl[0x30/4];
			int offsy  = ms32_roz_ctrl[0x34/4];

			my_clip.min_y = my_clip.max_y = y;

			offsx += (ms32_roz_ctrl[0x38/4] & 1) * 0x400;	// ??? gratia, hayaosi1...
			offsy += (ms32_roz_ctrl[0x3c/4] & 1) * 0x400;	// ??? gratia, hayaosi1...

			/* extend sign */
			if (start2x & 0x20000) start2x |= ~0x3ffff;
			if (start2y & 0x20000) start2y |= ~0x3ffff;
			if (startx & 0x20000) startx |= ~0x3ffff;
			if (starty & 0x20000) starty |= ~0x3ffff;
			if (incxx & 0x10000) incxx |= ~0x1ffff;
			if (incxy & 0x10000) incxy |= ~0x1ffff;

			tilemap_draw_roz(bitmap, &my_clip, ms32_roz_tilemap,
					(start2x+startx+offsx)<<16, (start2y+starty+offsy)<<16,
					incxx<<8, incxy<<8, 0, 0,
					1, // Wrap
					0, priority);

			y++;
		}
	}
	else	/* "simple" mode */
	{
		int startx = (ms32_roz_ctrl[0x00/4] & 0xffff) | ((ms32_roz_ctrl[0x04/4] & 3) << 16);
		int starty = (ms32_roz_ctrl[0x08/4] & 0xffff) | ((ms32_roz_ctrl[0x0c/4] & 3) << 16);
		int incxx  = (ms32_roz_ctrl[0x10/4] & 0xffff) | ((ms32_roz_ctrl[0x14/4] & 1) << 16);
		int incxy  = (ms32_roz_ctrl[0x18/4] & 0xffff) | ((ms32_roz_ctrl[0x1c/4] & 1) << 16);
		int incyy  = (ms32_roz_ctrl[0x20/4] & 0xffff) | ((ms32_roz_ctrl[0x24/4] & 1) << 16);
		int incyx  = (ms32_roz_ctrl[0x28/4] & 0xffff) | ((ms32_roz_ctrl[0x2c/4] & 1) << 16);
		int offsx  = ms32_roz_ctrl[0x30/4];
		int offsy  = ms32_roz_ctrl[0x34/4];

		offsx += (ms32_roz_ctrl[0x38/4] & 1) * 0x400;	// ??? gratia, hayaosi1...
		offsy += (ms32_roz_ctrl[0x3c/4] & 1) * 0x400;	// ??? gratia, hayaosi1...

		/* extend sign */
		if (startx & 0x20000) startx |= ~0x3ffff;
		if (starty & 0x20000) starty |= ~0x3ffff;
		if (incxx & 0x10000) incxx |= ~0x1ffff;
		if (incxy & 0x10000) incxy |= ~0x1ffff;
		if (incyy & 0x10000) incyy |= ~0x1ffff;
		if (incyx & 0x10000) incyx |= ~0x1ffff;

		tilemap_draw_roz(bitmap, cliprect, ms32_roz_tilemap,
				(startx+offsx)<<16, (starty+offsy)<<16,
				incxx<<8, incxy<<8, incyx<<8, incyy<<8,
				1, // Wrap
				0, priority);
	}
}



VIDEO_UPDATE( ms32 )
{
	int scrollx,scrolly;

	/* TODO: registers 0x04/4 and 0x10/4 are used too; the most interesting case
       is gametngk, where they are *usually*, but not always, copies of 0x00/4
       and 0x0c/4 (used for scrolling).
       0x10/4 is 0xdf in most games (apart from gametngk's special case), but
       it's 0x00 in hayaosi1 and kirarast, and 0xe2 (!) in gratia's tx layer.
       The two registers might be somewhat related to the width and height of the
       tilemaps, but there's something that just doesn't fit.
     */
	scrollx = ms32_tx_scroll[0x00/4] + ms32_tx_scroll[0x08/4] + 0x18;
	scrolly = ms32_tx_scroll[0x0c/4] + ms32_tx_scroll[0x14/4];
	tilemap_set_scrollx(ms32_tx_tilemap, 0, scrollx);
	tilemap_set_scrolly(ms32_tx_tilemap, 0, scrolly);

	scrollx = ms32_bg_scroll[0x00/4] + ms32_bg_scroll[0x08/4] + 0x10;
	scrolly = ms32_bg_scroll[0x0c/4] + ms32_bg_scroll[0x14/4];
	tilemap_set_scrollx(ms32_bg_tilemap, 0, scrollx);
	tilemap_set_scrolly(ms32_bg_tilemap, 0, scrolly);


	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);

	/* TODO: 0 is correct for gametngk, but break f1superb scrolling grid (text at
       top and bottom of the screen becomes black on black) */
	bitmap_fill(bitmap,cliprect,0);	/* bg color */


	/* priority hack, we really need to figure out what priority ram is I think */
	if (!strcmp(screen->machine->gamedrv->name,"hayaosi3"))
	{
		tilemap_draw(bitmap,cliprect,ms32_bg_tilemap,0,1);
		tilemap_draw(bitmap,cliprect,ms32_tx_tilemap,0,4);
		draw_roz(bitmap,cliprect,4); // this question text needs to appear over the sprites
		draw_sprites(screen->machine,bitmap,cliprect, ms32_spram, 0x40000);

	}
	else
	{
		tilemap_draw(bitmap,cliprect,ms32_bg_tilemap,0,1);
		draw_roz(bitmap,cliprect,2);
		tilemap_draw(bitmap,cliprect,ms32_tx_tilemap,0,4);
		draw_sprites(screen->machine,bitmap,cliprect, ms32_spram, 0x40000);
	}


	return 0;
}
