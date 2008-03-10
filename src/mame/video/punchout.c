/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"


static tilemap *punchout_topTilemap;
static tilemap *punchout_botTilemap;

static tilemap *spr1tilemap;
static tilemap *spr1alttilemap;
static tilemap *spr2tilemap;
static tilemap *fgtilemap;

UINT8 *punchout_topTilemap_ram;
UINT8 *punchout_botTilemap_ram;
UINT8 *punchout_botTilemap_scroll_ram;
UINT8 *armwrest_fgTilemap_ram;
UINT8 *punchout_bigsprite1ram;
UINT8 *punchout_bigsprite2ram;
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

/* these depend on jumpers on the board and change from game to game */
static int gfx0inv,gfx1inv,gfx2inv,gfx3inv;

PALETTE_INIT( punchout )
{
	int i;

	for (i = 0; i < 0x800; i++)
	{
		int pen;
		int r, g, b;

		if (i < 0x200)
			/* top monitor chars */
			pen = ((i - 0x000) ^ gfx0inv) | 0x000;
		else if (i < 0x400)
			/* bottom monitor chars */
			pen = ((i - 0x200) ^ gfx1inv) | 0x200;
		else if (i < 0x600)
			/* big sprite #1 */
			pen = ((i - 0x400) ^ gfx2inv) | 0x200;
		else
			/* big sprite #2*/
			pen = ((i - 0x600) ^ gfx3inv) | 0x200;

		r = 255 - pal4bit(color_prom[pen + 0x000]);
		g = 255 - pal4bit(color_prom[pen + 0x400]);
		b = 255 - pal4bit(color_prom[pen + 0x800]);

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}
}


PALETTE_INIT( armwrest )
{
	int i;

	for (i = 0; i < 0xa00; i++)
	{
		int pen;
		int r, g, b;

		if (i < 0x400)
			/* top monitor chars */
			pen = (i - 0x000) | 0x000;
		else if (i < 0x600)
			/* bottom monitor chars */
			pen = (i - 0x400) | 0x200;
		else if (i < 0x800)
			/* big sprite #1 */
			pen = (i - 0x600) | 0x200;
		else
			/* big sprite #2*/
			pen = ((i - 0x800) ^ 0x03) | 0x200;

		r = 255 - pal4bit(color_prom[pen + 0x000]);
		g = 255 - pal4bit(color_prom[pen + 0x400]);
		b = 255 - pal4bit(color_prom[pen + 0x800]);

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
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
	int code = punchout_topTilemap_ram[tile_index*2] + 256 * (punchout_topTilemap_ram[tile_index*2 + 1] & 0x03);
	int color = ((punchout_topTilemap_ram[tile_index*2 + 1] & 0x7c) >> 2) + 64 * top_palette_bank;
	int flipx = punchout_topTilemap_ram[tile_index*2 + 1] & 0x80;
	SET_TILE_INFO(0, code, color, flipx ? TILE_FLIPX : 0);
}

static TILE_GET_INFO( bot_get_info )
{
	int code = punchout_botTilemap_ram[tile_index*2] + 256 * (punchout_botTilemap_ram[tile_index*2 + 1] & 0x03);
	int color = ((punchout_botTilemap_ram[tile_index*2 + 1] & 0x7c) >> 2) + 64 * bottom_palette_bank;
	int flipx = punchout_botTilemap_ram[tile_index*2 + 1] & 0x80;
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
	punchout_topTilemap = tilemap_create(top_get_info, tilemap_scan_rows,  8,8, 32,32);
	punchout_botTilemap = tilemap_create(bot_get_info, tilemap_scan_rows,  8,8, 64,32);
	tilemap_set_scroll_rows(punchout_botTilemap, 32);

	spr1tilemap = tilemap_create(bs1_get_info, tilemap_scan_rows,  8,8, 16,32);
	spr2tilemap = tilemap_create(bs2_get_info, tilemap_scan_rows,  8,8, 16,32);

	fgtilemap = NULL;

	tilemap_set_transparent_pen(spr1tilemap, gfx2inv & 0x07);
	tilemap_set_transparent_pen(spr2tilemap, gfx3inv & 0x03);
}



static TILE_GET_INFO( armwrest_top_get_info )
{
	int code = punchout_topTilemap_ram[tile_index*2] + 256 * (punchout_topTilemap_ram[tile_index*2 + 1] & 0x03) +
								8 * (punchout_topTilemap_ram[tile_index*2 + 1] & 0x80);
	int color = ((punchout_topTilemap_ram[tile_index*2 + 1] & 0x7c) >> 2) + 64 * top_palette_bank;
	SET_TILE_INFO(0, code, color, 0);
}

static TILE_GET_INFO( armwrest_bot_get_info )
{
	int code = punchout_botTilemap_ram[tile_index*2] + 256 * (punchout_botTilemap_ram[tile_index*2 + 1] & 0x03);
	int color = 128 + ((punchout_botTilemap_ram[tile_index*2 + 1] & 0x7c) >> 2) + 64 * bottom_palette_bank;
	int flipx = punchout_botTilemap_ram[tile_index*2 + 1] & 0x80;
	SET_TILE_INFO(0, code, color, flipx ? TILE_FLIPX : 0);
}

static TILEMAP_MAPPER( armwrest_bs1_scan )
{
	int halfcols = num_cols/2;
	return (col/halfcols)*(halfcols*num_rows) + row*halfcols + col%halfcols;
}

static TILEMAP_MAPPER( armwrest_bs1alt_scan )
{
	int halfcols = num_cols/2;
	col ^=0x10;
	return (col/halfcols)*(halfcols*num_rows) + row*halfcols + col%halfcols;
}



static TILE_GET_INFO( armwrest_fg_get_info )
{
	int code = armwrest_fgTilemap_ram[tile_index*2] + 256 * (armwrest_fgTilemap_ram[tile_index*2 + 1] & 0x07);
	int color = ((armwrest_fgTilemap_ram[tile_index*2 + 1] & 0xf8) >> 3) + 32 * bottom_palette_bank;
	int flipx = armwrest_fgTilemap_ram[tile_index*2 + 1] & 0x80;
	SET_TILE_INFO(1, code, color, flipx ? TILE_FLIPX : 0);
}

VIDEO_START( armwrest )
{
	punchout_topTilemap = tilemap_create(armwrest_top_get_info, tilemap_scan_rows,  8,8, 32,32);
	punchout_botTilemap = tilemap_create(armwrest_bot_get_info, tilemap_scan_rows,  8,8, 32,32);

	spr1tilemap = tilemap_create(bs1_get_info, armwrest_bs1_scan,  8,8, 32,16);
	spr1alttilemap = tilemap_create(bs1_get_info, armwrest_bs1alt_scan,  8,8, 32,16);
	spr2tilemap = tilemap_create(bs2_get_info, tilemap_scan_rows,  8,8, 16,32);
	fgtilemap = tilemap_create(armwrest_fg_get_info, tilemap_scan_rows,  8,8, 32,32);

	tilemap_set_transparent_pen(spr1tilemap, 0x07);
	tilemap_set_transparent_pen(spr1alttilemap, 0x07);
	tilemap_set_transparent_pen(spr2tilemap, 0x00);
	tilemap_set_transparent_pen(fgtilemap, 0x07);
}



WRITE8_HANDLER( punchout_topTilemap_ram_w )
{
	punchout_topTilemap_ram[offset] = data;
	tilemap_mark_tile_dirty(punchout_topTilemap, offset/2);
}

WRITE8_HANDLER( punchout_botTilemap_ram_w )
{
	punchout_botTilemap_ram[offset] = data;
	tilemap_mark_tile_dirty(punchout_botTilemap, offset/2);
}

WRITE8_HANDLER( armwrest_fgTilemap_ram_w )
{
	armwrest_fgTilemap_ram[offset] = data;
	tilemap_mark_tile_dirty(fgtilemap, offset/2);
}

WRITE8_HANDLER( punchout_bigsprite1ram_w )
{
	punchout_bigsprite1ram[offset] = data;
	tilemap_mark_tile_dirty(spr1tilemap, offset/4);
	if (spr1alttilemap) tilemap_mark_tile_dirty(spr1alttilemap, offset/4);
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
		tilemap_mark_all_tiles_dirty(punchout_topTilemap);
	}
	if (bottom_palette_bank != ((data >> 0) & 0x01))
	{
		bottom_palette_bank = (data >> 0) & 0x01;
		if (fgtilemap != NULL)
			tilemap_mark_all_tiles_dirty(fgtilemap);
		tilemap_mark_all_tiles_dirty(punchout_botTilemap);
		tilemap_mark_all_tiles_dirty(spr1tilemap);
		tilemap_mark_all_tiles_dirty(spr2tilemap);
	}
}



static void draw_big_sprite(bitmap_t *bitmap, const rectangle *cliprect)
{
	int zoom;

	zoom = punchout_bigsprite1[0] + 256 * (punchout_bigsprite1[1] & 0x0f);
	if (zoom)
	{
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
			startx = ((16 * 8) << 16) - startx - 1;
			incxx = -incxx;
		}

		tilemap_draw_roz(bitmap,cliprect,spr1tilemap,
			startx,starty + 0x200*(2) * zoom,
			incxx,0,0,incyy,	/* zoom, no rotation */
			0,	/* no wraparound */
			0,0);
	}
}


static void armwrest_draw_big_sprite(bitmap_t *bitmap, const rectangle *cliprect)
{
	int zoom;

	zoom = punchout_bigsprite1[0] + 256 * (punchout_bigsprite1[1] & 0x0f);
	if (zoom)
	{
		int sx,sy;
		UINT32 startx,starty;
		int incxx,incyy;
		tilemap *_tilemap;

		sx = 4096 - (punchout_bigsprite1[2] + 256 * (punchout_bigsprite1[3] & 0x0f));
		if (sx > 2048) sx -= 4096;

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
			_tilemap = spr1alttilemap;
			startx = ((32 * 8) << 16) - startx - 1;
			incxx = -incxx;
		}
		else
			_tilemap = spr1tilemap;


		tilemap_draw_roz(bitmap,cliprect,_tilemap,
			startx,starty + 0x200*(2) * zoom,
			incxx,0,0,incyy,	/* zoom, no rotation */
			0,	/* no wraparound */
			0,0);
	}
}

static void drawbs2(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	int sx,sy;
	int incxx;

	sx = 512 - (punchout_bigsprite2[0] + 256 * (punchout_bigsprite2[1] & 1));
	if (sx > 512-127) sx -= 512;
	sx -= 55;	/* adjustment to match the screen shots */

	sy = -punchout_bigsprite2[2] + 256 * (punchout_bigsprite2[3] & 1);
	sy += 3;	/* adjustment to match the screen shots */

	sx = -sx << 16;
	sy = -sy << 16;

	if (punchout_bigsprite2[4] & 1)	/* flip x */
	{
		sx = ((16 * 8) << 16) - sx - 1;
		incxx = -1;
	}
	else
		incxx = 1;

	tilemap_draw_roz(bitmap,cliprect,spr2tilemap,
		sx, sy, incxx << 16, 0, 0, 1 << 16,
		0, 0, 0);
}

VIDEO_UPDATE( punchout )
{
	const device_config *top_screen    = device_list_find_by_tag(screen->machine->config->devicelist, VIDEO_SCREEN, "top");
	const device_config *bottom_screen = device_list_find_by_tag(screen->machine->config->devicelist, VIDEO_SCREEN, "bottom");

	if (screen == bottom_screen)
	{
		tilemap_draw(bitmap, cliprect, punchout_topTilemap, 0, 0);

		if (punchout_bigsprite1[7] & 1)	/* display in top monitor */
			draw_big_sprite(bitmap, cliprect);
	}
	else if (screen == top_screen)
	{
		int offs;

		/* copy the character mapped graphics */
		for (offs = 0;offs < 32;offs++)
			tilemap_set_scrollx(punchout_botTilemap, offs, 58 + punchout_botTilemap_scroll_ram[2*offs] + 256 * (punchout_botTilemap_scroll_ram[2*offs + 1] & 0x01));

		tilemap_draw(bitmap, cliprect, punchout_botTilemap, 0, 0);

		if (punchout_bigsprite1[7] & 2)	/* display in bottom monitor */
			draw_big_sprite(bitmap, cliprect);
		drawbs2(screen->machine, bitmap, cliprect);
	}
	return 0;
}


VIDEO_UPDATE( armwrest )
{
	const device_config *top_screen    = device_list_find_by_tag(screen->machine->config->devicelist, VIDEO_SCREEN, "top");
	const device_config *bottom_screen = device_list_find_by_tag(screen->machine->config->devicelist, VIDEO_SCREEN, "bottom");

	if (screen == bottom_screen)
	{
		tilemap_draw(bitmap, cliprect, punchout_topTilemap, 0, 0);

		if (punchout_bigsprite1[7] & 1)	/* display in top monitor */
			armwrest_draw_big_sprite(bitmap, cliprect);
	}
	else if (screen == top_screen)
	{
		tilemap_draw(bitmap, cliprect, punchout_botTilemap, 0, 0);

		if (punchout_bigsprite1[7] & 2)	/* display in bottom monitor */
			armwrest_draw_big_sprite(bitmap, cliprect);
		drawbs2(screen->machine, bitmap, cliprect);

		tilemap_draw(bitmap, cliprect, fgtilemap, 0, 0);
	}
	return 0;
}
