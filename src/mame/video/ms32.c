/* Jaleco MegaSystem 32 Video Hardware */

/* The Video Hardware is Similar to the Non-MS32 Version of Tetris Plus 2 */

/* Plenty to do, see list in drivers/ms32.c */

/*

priority should be given to
(a) dekluding the priorities, the kludge for kirarast made it easier to emulate the rest of it until then
(b) working out the background registers correctly ...
*/


#include "emu.h"
#include "includes/tetrisp2.h"
#include "includes/ms32.h"

static bitmap_t* temp_bitmap_tilemaps;
static bitmap_t* temp_bitmap_sprites;
static bitmap_t* temp_bitmap_sprites_pri;


//UINT32 *ms32_fce00000;
UINT32 *ms32_roz_ctrl;
UINT32 *ms32_tx_scroll;
UINT32 *ms32_bg_scroll;
//UINT32 *ms32_priram;
//UINT32 *ms32_palram;
//UINT32 *ms32_rozram;
//UINT32 *ms32_lineram;
//UINT32 *ms32_spram;
//UINT32 *ms32_bgram;
//UINT32 *ms32_txram;
UINT32 *ms32_mainram;


UINT8* ms32_priram_8;
UINT16* ms32_palram_16;
UINT16* ms32_rozram_16;
UINT16 *ms32_lineram_16;
UINT16 *ms32_sprram_16;
UINT16 *ms32_bgram_16;
UINT16 *ms32_txram_16;
UINT32 ms32_tilemaplayoutcontrol;

UINT16* f1superb_extraram_16;

// kirarast, tp2m32, and 47pie2 require the sprites in a different order
static int ms32_reverse_sprite_order;

/********** Tilemaps **********/

tilemap_t *ms32_tx_tilemap, *ms32_roz_tilemap, *ms32_bg_tilemap, *ms32_bg_tilemap_alt;
tilemap_t *ms32_extra_tilemap;
static int flipscreen;


static TILE_GET_INFO( get_ms32_tx_tile_info )
{
	int tileno, colour;

	tileno = ms32_txram_16[tile_index *2]   & 0xffff;
	colour = ms32_txram_16[tile_index *2+1] & 0x000f;

	SET_TILE_INFO(3,tileno,colour,0);
}

static TILE_GET_INFO( get_ms32_roz_tile_info )
{
	int tileno,colour;

	tileno = ms32_rozram_16[tile_index *2]   & 0xffff;
	colour = ms32_rozram_16[tile_index *2+1] & 0x000f;

	SET_TILE_INFO(1,tileno,colour,0);
}

static TILE_GET_INFO( get_ms32_bg_tile_info )
{
	int tileno,colour;

	tileno = ms32_bgram_16[tile_index *2]   & 0xffff;
	colour = ms32_bgram_16[tile_index *2+1] & 0x000f;

	SET_TILE_INFO(2,tileno,colour,0);
}

static TILE_GET_INFO( get_ms32_extra_tile_info )
{
	int tileno,colour;

	tileno = f1superb_extraram_16[tile_index *2]   & 0xffff;
	colour = f1superb_extraram_16[tile_index *2+1] & 0x000f;

	SET_TILE_INFO(4,tileno,colour+0x50,0);
}


static UINT32 brt[4];
static int brt_r,brt_g,brt_b;

VIDEO_START( ms32 )
{
	int width = machine->primary_screen->width();
	int height = machine->primary_screen->height();

	ms32_priram_8   = auto_alloc_array_clear(machine, UINT8, 0x2000);
	ms32_palram_16  = auto_alloc_array_clear(machine, UINT16, 0x20000);
	ms32_rozram_16  = auto_alloc_array_clear(machine, UINT16, 0x10000);
	ms32_lineram_16 = auto_alloc_array_clear(machine, UINT16, 0x1000);
	ms32_sprram_16  = auto_alloc_array_clear(machine, UINT16, 0x20000);
	ms32_bgram_16   = auto_alloc_array_clear(machine, UINT16, 0x4000);
	ms32_txram_16   = auto_alloc_array_clear(machine, UINT16, 0x4000);

	ms32_tx_tilemap = tilemap_create(machine, get_ms32_tx_tile_info,tilemap_scan_rows,8, 8,64,64);
	ms32_bg_tilemap = tilemap_create(machine, get_ms32_bg_tile_info,tilemap_scan_rows,16,16,64,64);
	ms32_bg_tilemap_alt = tilemap_create(machine, get_ms32_bg_tile_info,tilemap_scan_rows,16,16,256,16); // alt layout, controller by register?
	ms32_roz_tilemap = tilemap_create(machine, get_ms32_roz_tile_info,tilemap_scan_rows,16,16,128,128);


	/* set up tile layers */
	temp_bitmap_tilemaps = auto_bitmap_alloc(machine, width, height, BITMAP_FORMAT_INDEXED16);
	temp_bitmap_sprites  = auto_bitmap_alloc(machine, width, height, BITMAP_FORMAT_INDEXED16);
	temp_bitmap_sprites_pri = auto_bitmap_alloc(machine, width, height, BITMAP_FORMAT_INDEXED16); // not actually being used for rendering, we embed pri info in the raw colour bitmap

	bitmap_fill(temp_bitmap_tilemaps,0,0);
	bitmap_fill(temp_bitmap_sprites,0,0);
	bitmap_fill(temp_bitmap_sprites_pri,0,0);

	tilemap_set_transparent_pen(ms32_tx_tilemap,0);
	tilemap_set_transparent_pen(ms32_bg_tilemap,0);
	tilemap_set_transparent_pen(ms32_bg_tilemap_alt,0);
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

VIDEO_START( f1superb )
{
	VIDEO_START_CALL( ms32 );

	f1superb_extraram_16  = auto_alloc_array_clear(machine, UINT16, 0x10000);
	ms32_extra_tilemap = tilemap_create(machine, get_ms32_extra_tile_info,tilemap_scan_rows,2048,1,1,0x400);

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
		r = ((ms32_palram_16[color*2] & 0xff00) >>8 ) * brt_r / 0x100;
		g = ((ms32_palram_16[color*2] & 0x00ff) >>0 ) * brt_g / 0x100;
		b = ((ms32_palram_16[color*2+1] & 0x00ff) >>0 ) * brt_b / 0x100;
	}
	else
	{
		r = ((ms32_palram_16[color*2] & 0xff00) >>8 );
		g = ((ms32_palram_16[color*2] & 0x00ff) >>0 );
		b = ((ms32_palram_16[color*2+1] & 0x00ff) >>0 );
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
		//int i;

		if (bank == 0)
		{
			brt_r = 0x100 - ((brt[0] & 0xff00) >> 8);
			brt_g = 0x100 - ((brt[0] & 0x00ff) >> 0);
			brt_b = 0x100 - ((brt[1] & 0x00ff) >> 0);

		//  for (i = 0;i < 0x3000;i++)  // colors 0x3000-0x3fff are not used
		//      update_color(space->machine, i);
		}
	}

//popmessage("%04x %04x %04x %04x",brt[0],brt[1],brt[2],brt[3]);
}






WRITE32_HANDLER( ms32_gfxctrl_w )
{
	if (ACCESSING_BITS_0_7)
	{
		/* bit 1 = flip screen */
		flipscreen = data & 0x02;
		tilemap_set_flip(ms32_tx_tilemap,flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
		tilemap_set_flip(ms32_bg_tilemap,flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
		tilemap_set_flip(ms32_bg_tilemap_alt,flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

		/* bit 2 used by f1superb, unknown */

		/* bit 3 used by several games, unknown */

//popmessage("%08x",data);
	}
}



/* SPRITES based on tetrisp2 for now, readd priority bits later */
/* now using function in tetrisp2.c */





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
			UINT16 *lineaddr = ms32_lineram_16 + 8 * (y & 0xff);

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
	int asc_pri;
	int scr_pri;
	int rot_pri;

	/* TODO: registers 0x04/4 and 0x10/4 are used too; the most interesting case
       is gametngk, where they are *usually*, but not always, copies of 0x00/4
       and 0x0c/4 (used for scrolling).
       0x10/4 is 0xdf in most games (apart from gametngk's special case), but
       it's 0x00 in hayaosi1 and kirarast, and 0xe2 (!) in gratia's tx layer.
       The two registers might be somewhat related to the width and height of the
       tilemaps, but there's something that just doesn't fit.
     */
	int i;

	for (i = 0;i < 0x10000;i++)	// colors 0x3000-0x3fff are not used
		update_color(screen->machine, i);

	scrollx = ms32_tx_scroll[0x00/4] + ms32_tx_scroll[0x08/4] + 0x18;
	scrolly = ms32_tx_scroll[0x0c/4] + ms32_tx_scroll[0x14/4];
	tilemap_set_scrollx(ms32_tx_tilemap, 0, scrollx);
	tilemap_set_scrolly(ms32_tx_tilemap, 0, scrolly);

	scrollx = ms32_bg_scroll[0x00/4] + ms32_bg_scroll[0x08/4] + 0x10;
	scrolly = ms32_bg_scroll[0x0c/4] + ms32_bg_scroll[0x14/4];
	tilemap_set_scrollx(ms32_bg_tilemap, 0, scrollx);
	tilemap_set_scrolly(ms32_bg_tilemap, 0, scrolly);
	tilemap_set_scrollx(ms32_bg_tilemap_alt, 0, scrollx);
	tilemap_set_scrolly(ms32_bg_tilemap_alt, 0, scrolly);


	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);



	/* TODO: 0 is correct for gametngk, but break f1superb scrolling grid (text at
       top and bottom of the screen becomes black on black) */
	bitmap_fill(temp_bitmap_tilemaps,cliprect,0);	/* bg color */

	/* clear our sprite bitmaps */
	bitmap_fill(temp_bitmap_sprites,cliprect,0);
	bitmap_fill(temp_bitmap_sprites_pri,cliprect,0);

	tetrisp2_draw_sprites(screen->machine, temp_bitmap_sprites, temp_bitmap_sprites_pri, cliprect, NULL, ms32_sprram_16, 0x20000, 0, ms32_reverse_sprite_order, 0, 1 );




	asc_pri = scr_pri = rot_pri = 0;

	if((ms32_priram_8[0x2b00 / 2] & 0x00ff) == 0x0034)
		asc_pri++;
	else
		rot_pri++;

	if((ms32_priram_8[0x2e00 / 2] & 0x00ff) == 0x0034)
		asc_pri++;
	else
		scr_pri++;

	if((ms32_priram_8[0x3a00 / 2] & 0x00ff) == 0x000c)
		scr_pri++;
	else
		rot_pri++;

	if (rot_pri == 0)
		draw_roz(temp_bitmap_tilemaps,cliprect, 1 << 1);
	else if (scr_pri == 0)
		if (ms32_tilemaplayoutcontrol&1)
		{
			tilemap_draw(temp_bitmap_tilemaps,cliprect, ms32_bg_tilemap_alt,  0, 1 << 0);
		}
		else
		{
			tilemap_draw(temp_bitmap_tilemaps,cliprect, ms32_bg_tilemap,  0, 1 << 0);
		}
	else if (asc_pri == 0)
		tilemap_draw(temp_bitmap_tilemaps,cliprect, ms32_tx_tilemap,  0, 1 << 2);

	if (rot_pri == 1)
		draw_roz(temp_bitmap_tilemaps,cliprect, 1 << 1);
	else if (scr_pri == 1)
		if (ms32_tilemaplayoutcontrol&1)
		{
			tilemap_draw(temp_bitmap_tilemaps,cliprect, ms32_bg_tilemap_alt,  0, 1 << 0);
		}
		else
		{
			tilemap_draw(temp_bitmap_tilemaps,cliprect, ms32_bg_tilemap,  0, 1 << 0);
		}
	else if (asc_pri == 1)
		tilemap_draw(temp_bitmap_tilemaps,cliprect, ms32_tx_tilemap,  0, 1 << 2);

	if (rot_pri == 2)
		draw_roz(temp_bitmap_tilemaps,cliprect, 1 << 1);
	else if (scr_pri == 2)
		if (ms32_tilemaplayoutcontrol&1)
		{
			tilemap_draw(temp_bitmap_tilemaps,cliprect, ms32_bg_tilemap_alt,  0, 1 << 0);
		}
		else
		{
			tilemap_draw(temp_bitmap_tilemaps,cliprect, ms32_bg_tilemap,  0, 1 << 0);
		}
	else if (asc_pri == 2)
		tilemap_draw(temp_bitmap_tilemaps,cliprect, ms32_tx_tilemap,  0, 1 << 2);

	/* MIX it! */
	/* this mixing isn't 100% accurate, it should be using ALL the data in
       the priority ram, probably for per-pixel / pen mixing, or more levels
       than are supported here..  I don't know, it will need hw tests I think */
	{
		int xx, yy;
		int width = screen->width();
		int height = screen->height();
		const pen_t *paldata = screen->machine->pens;

		UINT16* srcptr_tile;
		UINT8* srcptr_tilepri;
		UINT16* srcptr_spri;
		//UINT8* srcptr_spripri;

		UINT32* dstptr_bitmap;

		bitmap_fill(bitmap,cliprect,0);

		for (yy=0;yy<height;yy++)
		{
			srcptr_tile =     BITMAP_ADDR16(temp_bitmap_tilemaps, yy, 0);
			srcptr_tilepri =  BITMAP_ADDR8(screen->machine->priority_bitmap, yy, 0);
			srcptr_spri =     BITMAP_ADDR16(temp_bitmap_sprites, yy, 0);
			//srcptr_spripri =  BITMAP_ADDR8(temp_bitmap_sprites_pri, yy, 0);
			dstptr_bitmap  =  BITMAP_ADDR32(bitmap, yy, 0);
			for (xx=0;xx<width;xx++)
			{
				UINT16 src_tile  = srcptr_tile[xx];
				UINT8 src_tilepri = srcptr_tilepri[xx];
				UINT16 src_spri = srcptr_spri[xx];
				//UINT8 src_spripri;// = srcptr_spripri[xx];
				UINT16 spridat = ((src_spri&0x0fff));
				UINT8  spritepri =     ((src_spri&0xf000) >> 8);
				int primask = 0;

				// get sprite priority value back out of bitmap/colour data (this is done in draw_sprite for standalone hw)
				if (ms32_priram_8[(spritepri | 0x0a00 | 0x1500) / 2] & 0x38) primask |= 1 << 0;
				if (ms32_priram_8[(spritepri | 0x0a00 | 0x1400) / 2] & 0x38) primask |= 1 << 1;
				if (ms32_priram_8[(spritepri | 0x0a00 | 0x1100) / 2] & 0x38) primask |= 1 << 2;
				if (ms32_priram_8[(spritepri | 0x0a00 | 0x1000) / 2] & 0x38) primask |= 1 << 3;
				if (ms32_priram_8[(spritepri | 0x0a00 | 0x0500) / 2] & 0x38) primask |= 1 << 4;
				if (ms32_priram_8[(spritepri | 0x0a00 | 0x0400) / 2] & 0x38) primask |= 1 << 5;
				if (ms32_priram_8[(spritepri | 0x0a00 | 0x0100) / 2] & 0x38) primask |= 1 << 6;
				if (ms32_priram_8[(spritepri | 0x0a00 | 0x0000) / 2] & 0x38) primask |= 1 << 7;


				if (primask == 0x00)
				{

					if (src_tilepri==0x00)
					{
						if (spridat & 0xff)
							dstptr_bitmap[xx] = paldata[spridat]; // best bout boxing title
						else
							dstptr_bitmap[xx] = paldata[src_tile];
					}
					else if (src_tilepri==0x01)
					{
						if (spridat & 0xff)
							dstptr_bitmap[xx] = paldata[spridat]; // best bout boxing title
						else
							dstptr_bitmap[xx] = paldata[src_tile];
					}
					else if (src_tilepri==0x02)
					{
						if (spridat & 0xff)
							dstptr_bitmap[xx] = paldata[spridat]; // best bout boxing
						else
							dstptr_bitmap[xx] = paldata[src_tile];
					}
					else if (src_tilepri==0x03)
					{
						if (spridat & 0xff)
							dstptr_bitmap[xx] = paldata[spridat]; // best bout boxing
						else
							dstptr_bitmap[xx] = paldata[src_tile];
					}
					else if (src_tilepri==0x04)
					{
						if (spridat & 0xff)
							dstptr_bitmap[xx] = paldata[spridat];
						else
							dstptr_bitmap[xx] = paldata[src_tile];
					}
					else if (src_tilepri==0x05)
					{
						if (spridat & 0xff)
							dstptr_bitmap[xx] = paldata[spridat];
						else
							dstptr_bitmap[xx] = paldata[src_tile];
					}
					else if (src_tilepri==0x06)
					{
						if (spridat & 0xff)
							dstptr_bitmap[xx] = paldata[spridat];
						else
							dstptr_bitmap[xx] = paldata[src_tile];
					}
					else if (src_tilepri==0x07)
					{
						if (spridat & 0xff)
							dstptr_bitmap[xx] = paldata[spridat]; // desert war radar?
						else
							dstptr_bitmap[xx] = paldata[src_tile];
					}


				}
				else if (primask == 0xc0)
				{
					dstptr_bitmap[xx] = paldata[screen->machine->rand()&0xfff];
				}
				else if (primask == 0xf0)
				{
//                  dstptr_bitmap[xx] = paldata[spridat];
					if (src_tilepri==0x00)
					{
						if (spridat & 0xff)
							dstptr_bitmap[xx] = paldata[spridat]; // clouds at top gametngk intro
						else
							dstptr_bitmap[xx] = paldata[src_tile];
					}
					else if (src_tilepri==0x01)
					{
						if (spridat & 0xff)
							dstptr_bitmap[xx] = paldata[spridat]; // clouds gametngk intro
						else
							dstptr_bitmap[xx] = paldata[src_tile];
					}
					else if (src_tilepri==0x02)
					{
						if (spridat & 0xff)
							dstptr_bitmap[xx] = paldata[spridat]; // mode select gametngk
						else
							dstptr_bitmap[xx] = paldata[src_tile];
					}
					else if (src_tilepri==0x03)
					{
						if (spridat & 0xff)
							dstptr_bitmap[xx] = paldata[spridat]; // title gametngk
						else
							dstptr_bitmap[xx] = paldata[src_tile];
					}
					else if (src_tilepri==0x04)
					{
						dstptr_bitmap[xx] = paldata[src_tile]; // insert coin text on girl gametngk intro
					}
					else if (src_tilepri==0x05)
					{
						dstptr_bitmap[xx] = paldata[src_tile]; // insert coin gametngk intro
					}
					else if (src_tilepri==0x06)
					{
						dstptr_bitmap[xx] = paldata[src_tile]; // insert coin gametngk intro
					}
					else if (src_tilepri==0x07)
					{
						dstptr_bitmap[xx] = paldata[src_tile]; // insert coin gametngk intro
					}
				}
				else if (primask == 0xfc)
				{
					if (src_tilepri==0x00)
					{
						if (spridat & 0xff)
							dstptr_bitmap[xx] = paldata[spridat]; // tetrisp intro text
						else
							dstptr_bitmap[xx] = paldata[src_tile];
					}
					else if (src_tilepri==0x01)
					{
						if (spridat & 0xff)
							dstptr_bitmap[xx] = paldata[spridat]; // tetrisp intro text
						else
							dstptr_bitmap[xx] = paldata[src_tile];
					}
					else if (src_tilepri==0x02)
					{
						dstptr_bitmap[xx] = paldata[src_tile]; // tetrisp story
					}
					else if (src_tilepri==0x03)
					{
						dstptr_bitmap[xx] = paldata[src_tile]; // tetrisp fader to game after story
					}
					else if (src_tilepri==0x04)
					{
						dstptr_bitmap[xx] = paldata[src_tile]; // credit text tetrisp mode select
					}
					else if (src_tilepri==0x05)
					{
						dstptr_bitmap[xx] = paldata[src_tile]; // credit text tetrisp intro
					}
					else if (src_tilepri==0x06)
					{
						//dstptr_bitmap[xx] = paldata[screen->machine->rand()&0xfff];
						dstptr_bitmap[xx] = paldata[src_tile]; // assumed
					}
					else if (src_tilepri==0x07)
					{
						//dstptr_bitmap[xx] = paldata[screen->machine->rand()&0xfff];
						dstptr_bitmap[xx] = paldata[src_tile]; // assumed
					}
				}
				else if (primask == 0xfe)
				{
					if (src_tilepri==0x00)
					{
						if (spridat & 0xff)
							dstptr_bitmap[xx] = paldata[spridat]; // screens in gametngk intro
						else
							dstptr_bitmap[xx] = paldata[src_tile];
					}
					else if (src_tilepri==0x01)
					{
						dstptr_bitmap[xx] = alpha_blend_r32( paldata[src_tile], 0x00000000, 128); // shadow, gametngk title
					}
					else if (src_tilepri==0x02)
					{
						dstptr_bitmap[xx] = alpha_blend_r32( paldata[src_tile], 0x00000000, 128); // shadow, gametngk mode select
					}
					else if (src_tilepri==0x03)
					{
						dstptr_bitmap[xx] = alpha_blend_r32( paldata[src_tile], 0x00000000, 128); // shadow, gametngk title
					}
					else if (src_tilepri==0x04)
					{
						dstptr_bitmap[xx] = paldata[src_tile]; // credit text gametngk intro
					}
					else if (src_tilepri==0x05)
					{
						dstptr_bitmap[xx] = paldata[src_tile]; // credit text near shadow, gametngk title
					}
					else if (src_tilepri==0x06)
					{
						dstptr_bitmap[xx] = paldata[src_tile]; // credit gametngk highscores
					}
					else if (src_tilepri==0x07)
					{
						dstptr_bitmap[xx] = paldata[src_tile]; // assumed
					}
				}

				else
				{
					fatalerror("unhandled priority type %02x\n",primask);
				}



			}

		}

	}


	return 0;
}
