/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"


static tilemap *toptilemap;
static tilemap *bottilemap;
static tilemap *spr1tilemap;
static tilemap *spr2tilemap;
static tilemap *fgtilemap;

UINT8 *punchout_videoram2;
UINT8 *armwrest_videoram3;
UINT8 *punchout_bigsprite1ram;
UINT8 *punchout_bigsprite2ram;
UINT8 *punchout_scroll;
UINT8 *punchout_bigsprite1;
UINT8 *punchout_bigsprite2;
UINT8 *punchout_palettebank;

static UINT8 top_palette_bank,bottom_palette_bank;



/***************************************************************************

  Convert the color PROMs into a more useable format.

  Punch Out has a six 512x4 palette PROMs (one per gun; three for the top
  monitor chars, three for everything else).
  The PROMs are connected to the RGB output this way:

  bit 3 -- 240 ohm resistor -- inverter  -- RED/GREEN/BLUE
        -- 470 ohm resistor -- inverter  -- RED/GREEN/BLUE
        -- 1  kohm resistor -- inverter  -- RED/GREEN/BLUE
  bit 0 -- 2  kohm resistor -- inverter  -- RED/GREEN/BLUE

***************************************************************************/
static void convert_palette(running_machine *machine,const UINT8 *color_prom)
{
	int i;


	for (i = 0;i < 1024;i++)
	{
		int bit0,bit1,bit2,bit3,r,g,b;


		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		bit3 = (color_prom[0] >> 3) & 0x01;
		r = 255 - (0x10 * bit0 + 0x21 * bit1 + 0x46 * bit2 + 0x88 * bit3);
		bit0 = (color_prom[1024] >> 0) & 0x01;
		bit1 = (color_prom[1024] >> 1) & 0x01;
		bit2 = (color_prom[1024] >> 2) & 0x01;
		bit3 = (color_prom[1024] >> 3) & 0x01;
		g = 255 - (0x10 * bit0 + 0x21 * bit1 + 0x46 * bit2 + 0x88 * bit3);
		bit0 = (color_prom[2*1024] >> 0) & 0x01;
		bit1 = (color_prom[2*1024] >> 1) & 0x01;
		bit2 = (color_prom[2*1024] >> 2) & 0x01;
		bit3 = (color_prom[2*1024] >> 3) & 0x01;
		b = 255 - (0x10 * bit0 + 0x21 * bit1 + 0x46 * bit2 + 0x88 * bit3);

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
		color_prom++;
	}

	/* reserve the last color for the transparent pen (none of the game colors has */
	/* these RGB components) */
	palette_set_color(machine,1024,MAKE_RGB(240,240,240));
}


/* these depend on jumpers on the board and change from game to game */
static int gfx0inv,gfx1inv,gfx2inv,gfx3inv;

PALETTE_INIT( punchout )
{
	int i;
	#define TOTAL_COLORS(gfxn) (machine->gfx[gfxn]->total_colors * machine->gfx[gfxn]->color_granularity)
	#define COLOR(gfxn,offs) (colortable[machine->drv->gfxdecodeinfo[gfxn].color_codes_start + (offs)])


	convert_palette(machine,color_prom);


	/* top monitor chars */
	for (i = 0;i < TOTAL_COLORS(0);i++)
		COLOR(0,i ^ gfx0inv) = i;

	/* bottom monitor chars */
	for (i = 0;i < TOTAL_COLORS(1);i++)
		COLOR(1,i ^ gfx1inv) = i + 512;

	/* big sprite #1 */
	for (i = 0;i < TOTAL_COLORS(2);i++)
	{
		if (i % 8 == 0) COLOR(2,i ^ gfx2inv) = 1024;	/* transparent */
		else COLOR(2,i ^ gfx2inv) = i + 512;
	}

	/* big sprite #2 */
	for (i = 0;i < TOTAL_COLORS(3);i++)
	{
		if (i % 4 == 0) COLOR(3,i ^ gfx3inv) = 1024;	/* transparent */
		else COLOR(3,i ^ gfx3inv) = i + 512;
	}
}

PALETTE_INIT( armwrest )
{
	int i;
	#define TOTAL_COLORS(gfxn) (machine->gfx[gfxn]->total_colors * machine->gfx[gfxn]->color_granularity)
	#define COLOR(gfxn,offs) (colortable[machine->drv->gfxdecodeinfo[gfxn].color_codes_start + (offs)])


	convert_palette(machine,color_prom);


	/* top monitor / bottom monitor backround chars */
	for (i = 0;i < TOTAL_COLORS(0);i++)
		COLOR(0,i) = i;

	/* bottom monitor foreground chars */
	for (i = 0;i < TOTAL_COLORS(1);i++)
		COLOR(1,i) = i + 512;

	/* big sprite #1 */
	for (i = 0;i < TOTAL_COLORS(2);i++)
	{
		if (i % 8 == 7) COLOR(2,i) = 1024;	/* transparent */
		else COLOR(2,i) = i + 512;
	}

	/* big sprite #2 - pen order is inverted */
	for (i = 0;i < TOTAL_COLORS(3);i++)
	{
		if (i % 4 == 3) COLOR(3,i ^ 3) = 1024;	/* transparent */
		else COLOR(3,i ^ 3) = i + 512;
	}
}



DRIVER_INIT( punchout )
{
	gfx0inv = 0x03;
	gfx1inv = 0xfc;
	gfx2inv = 0xff;
	gfx3inv = 0xfc;
}

DRIVER_INIT( spnchout )
{
	gfx0inv = 0x00;
	gfx1inv = 0xff;
	gfx2inv = 0xff;
	gfx3inv = 0xff;
}

DRIVER_INIT( spnchotj )
{
	gfx0inv = 0xfc;
	gfx1inv = 0xff;
	gfx2inv = 0xff;
	gfx3inv = 0xff;
}

DRIVER_INIT( armwrest )
{
}




/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/
static TILE_GET_INFO( top_get_info )
{
	int code = videoram[tile_index*2] + 256 * (videoram[tile_index*2 + 1] & 0x03);
	int color = ((videoram[tile_index*2 + 1] & 0x7c) >> 2) + 64 * top_palette_bank;
	int flipx = videoram[tile_index*2 + 1] & 0x80;
	SET_TILE_INFO(0, code, color, flipx ? TILE_FLIPX : 0);
}

static TILE_GET_INFO( bot_get_info )
{
	int code = punchout_videoram2[tile_index*2] + 256 * (punchout_videoram2[tile_index*2 + 1] & 0x03);
	int color = ((punchout_videoram2[tile_index*2 + 1] & 0x7c) >> 2) + 64 * bottom_palette_bank;
	int flipx = punchout_videoram2[tile_index*2 + 1] & 0x80;
	SET_TILE_INFO(1, code, color, flipx ? TILE_FLIPX : 0);
}

static TILE_GET_INFO( bs1_get_info )
{
	int code = punchout_bigsprite1ram[tile_index*4] + 256 * (punchout_bigsprite1ram[tile_index*4 + 1] & 0x1f);
	int color = (punchout_bigsprite1ram[tile_index*4 + 3] & 0x1f) + 32 * bottom_palette_bank;
	int flipx = punchout_bigsprite1ram[tile_index*4 + 3] & 0x80;
	SET_TILE_INFO(2, code, color, flipx ? TILE_FLIPX : 0);
}

static TILE_GET_INFO( bs2_get_info )
{
	int code = punchout_bigsprite2ram[tile_index*4] + 256 * (punchout_bigsprite2ram[tile_index*4 + 1] & 0x0f);
	int color = (punchout_bigsprite2ram[tile_index*4 + 3] & 0x3f) + 64 * bottom_palette_bank;
	int flipx = punchout_bigsprite2ram[tile_index*4 + 3] & 0x80;
	SET_TILE_INFO(3, code, color, flipx ? TILE_FLIPX : 0);
}

VIDEO_START( punchout )
{
	toptilemap = tilemap_create(top_get_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8,8, 32,32);
	bottilemap = tilemap_create(bot_get_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8,8, 64,32);
	tilemap_set_scroll_rows(bottilemap, 32);

	spr1tilemap = tilemap_create(bs1_get_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8,8, 16,32);
	spr2tilemap = tilemap_create(bs2_get_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8,8, 16,32);

	fgtilemap = NULL;
}



static TILE_GET_INFO( armwrest_top_get_info )
{
	int code = videoram[tile_index*2] + 256 * (videoram[tile_index*2 + 1] & 0x03) +
								8 * (videoram[tile_index*2 + 1] & 0x80);
	int color = ((videoram[tile_index*2 + 1] & 0x7c) >> 2) + 64 * top_palette_bank;
	SET_TILE_INFO(0, code, color, 0);
}

static TILE_GET_INFO( armwrest_bot_get_info )
{
	int code = punchout_videoram2[tile_index*2] + 256 * (punchout_videoram2[tile_index*2 + 1] & 0x03);
	int color = 128 + ((punchout_videoram2[tile_index*2 + 1] & 0x7c) >> 2) + 64 * bottom_palette_bank;
	int flipx = punchout_videoram2[tile_index*2 + 1] & 0x80;
	SET_TILE_INFO(0, code, color, flipx ? TILE_FLIPX : 0);
}

static TILEMAP_MAPPER( armwrest_bs1_scan )
{
	int halfcols = num_cols/2;
	return (col/halfcols)*(halfcols*num_rows) + row*halfcols + col%halfcols;
}

static TILE_GET_INFO( armwrest_fg_get_info )
{
	int code = armwrest_videoram3[tile_index*2] + 256 * (armwrest_videoram3[tile_index*2 + 1] & 0x07);
	int color = ((armwrest_videoram3[tile_index*2 + 1] & 0xf8) >> 3) + 32 * bottom_palette_bank;
	int flipx = armwrest_videoram3[tile_index*2 + 1] & 0x80;
	SET_TILE_INFO(1, code, color, flipx ? TILE_FLIPX : 0);
}

VIDEO_START( armwrest )
{
	toptilemap = tilemap_create(armwrest_top_get_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8,8, 32,32);
	bottilemap = tilemap_create(armwrest_bot_get_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8,8, 32,32);

	spr1tilemap = tilemap_create(bs1_get_info, armwrest_bs1_scan, TILEMAP_TYPE_PEN, 8,8, 32,16);
	spr2tilemap = tilemap_create(bs2_get_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8,8, 16,32);

	fgtilemap = tilemap_create(armwrest_fg_get_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8,8, 32,32);
	tilemap_set_transparent_pen(fgtilemap, 7);
}



WRITE8_HANDLER( punchout_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(toptilemap, offset/2);
}

WRITE8_HANDLER( punchout_videoram2_w )
{
	punchout_videoram2[offset] = data;
	tilemap_mark_tile_dirty(bottilemap, offset/2);
}

WRITE8_HANDLER( armwrest_videoram3_w )
{
	armwrest_videoram3[offset] = data;
	tilemap_mark_tile_dirty(fgtilemap, offset/2);
}

WRITE8_HANDLER( punchout_bigsprite1ram_w )
{
	punchout_bigsprite1ram[offset] = data;
	tilemap_mark_tile_dirty(spr1tilemap, offset/4);
}

WRITE8_HANDLER( punchout_bigsprite2ram_w )
{
	punchout_bigsprite2ram[offset] = data;
	tilemap_mark_tile_dirty(spr2tilemap, offset/4);
}



WRITE8_HANDLER( punchout_palettebank_w )
{
	*punchout_palettebank = data;

	if (top_palette_bank != ((data >> 1) & 0x01))
	{
		top_palette_bank = (data >> 1) & 0x01;
		tilemap_mark_all_tiles_dirty(toptilemap);
	}
	if (bottom_palette_bank != ((data >> 0) & 0x01))
	{
		bottom_palette_bank = (data >> 0) & 0x01;
		if (fgtilemap != NULL)
			tilemap_mark_all_tiles_dirty(fgtilemap);
		tilemap_mark_all_tiles_dirty(bottilemap);
		tilemap_mark_all_tiles_dirty(spr1tilemap);
		tilemap_mark_all_tiles_dirty(spr2tilemap);
	}
}



static void draw_big_sprite(mame_bitmap *bitmap, const rectangle *cliprect)
{
	int zoom;

	zoom = punchout_bigsprite1[0] + 256 * (punchout_bigsprite1[1] & 0x0f);
	if (zoom)
	{
		mame_bitmap *sprbitmap = tilemap_get_pixmap(spr1tilemap);
		int sx,sy;
		UINT32 startx,starty;
		int incxx,incyy;

		sx = 4096 - (punchout_bigsprite1[2] + 256 * (punchout_bigsprite1[3] & 0x0f));
		if (sx > 4096-4*127) sx -= 4096;

		sy = -(punchout_bigsprite1[4] + 256 * (punchout_bigsprite1[5] & 1));
		if (sy <= -256 + zoom/0x40) sy += 512;
		sy += 12;

		incxx = zoom << 6;
		incyy = zoom << 6;

		startx = -sx * 0x4000;
		starty = -sy * 0x10000;
		startx += 3740 * zoom;	/* adjustment to match the screen shots */
		starty -= 178 * zoom;	/* and make the hall of fame picture nice */

		if (punchout_bigsprite1[6] & 1)	/* flip x */
		{
			startx = (sprbitmap->width << 16) - startx - 1;
			incxx = -incxx;
		}

		copyrozbitmap(bitmap,sprbitmap,
			startx,starty + 0x200*(2) * zoom,
			incxx,0,0,incyy,	/* zoom, no rotation */
			0,	/* no wraparound */
			cliprect,TRANSPARENCY_COLOR,1024,0);
	}
}

static void drawbs2(mame_bitmap *bitmap, const rectangle *cliprect)
{
	mame_bitmap *sprbitmap = tilemap_get_pixmap(spr2tilemap);
	int sx,sy;

	sx = 512 - (punchout_bigsprite2[0] + 256 * (punchout_bigsprite2[1] & 1));
	if (sx > 512-127) sx -= 512;
	sx -= 55;	/* adjustment to match the screen shots */

	sy = -punchout_bigsprite2[2] + 256 * (punchout_bigsprite2[3] & 1);
	sy += 3;	/* adjustment to match the screen shots */

	copybitmap(bitmap,sprbitmap,
			punchout_bigsprite2[4] & 1,0,
			sx,sy,
			cliprect,TRANSPARENCY_COLOR,1024);
}

VIDEO_UPDATE( punchout )
{
	if (screen == 1)
	{
		tilemap_draw(bitmap, cliprect, toptilemap, 0, 0);

		if (punchout_bigsprite1[7] & 1)	/* display in top monitor */
			draw_big_sprite(bitmap, cliprect);
	}
	else
	{
		int offs;

		/* copy the character mapped graphics */
		for (offs = 0;offs < 32;offs++)
			tilemap_set_scrollx(bottilemap, offs, 58 + punchout_scroll[2*offs] + 256 * (punchout_scroll[2*offs + 1] & 0x01));

		tilemap_draw(bitmap, cliprect, bottilemap, 0, 0);

		if (punchout_bigsprite1[7] & 2)	/* display in bottom monitor */
			draw_big_sprite(bitmap, cliprect);
		drawbs2(bitmap, cliprect);
	}
	return 0;
}


VIDEO_UPDATE( armwrest )
{
	if (screen == 1)
	{
		tilemap_draw(bitmap, cliprect, toptilemap, 0, 0);

		if (punchout_bigsprite1[7] & 1)	/* display in top monitor */
			draw_big_sprite(bitmap, cliprect);
	}
	else
	{
		tilemap_draw(bitmap, cliprect, bottilemap, 0, 0);

		if (punchout_bigsprite1[7] & 2)	/* display in bottom monitor */
			draw_big_sprite(bitmap, cliprect);
		drawbs2(bitmap, cliprect);

		tilemap_draw(bitmap, cliprect, fgtilemap, 0, 0);
	}
	return 0;
}
