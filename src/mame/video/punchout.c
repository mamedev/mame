/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"


static tilemap_t *bg_top_tilemap;
static tilemap_t *bg_bot_tilemap;
static tilemap_t *fg_tilemap;	// bottom only

static tilemap_t *spr1_tilemap;
static tilemap_t *spr1_tilemap_flipx;
static tilemap_t *spr2_tilemap;

UINT8 *punchout_bg_top_videoram;
UINT8 *punchout_bg_bot_videoram;
UINT8 *armwrest_fg_videoram;
UINT8 *punchout_spr1_videoram;
UINT8 *punchout_spr2_videoram;
UINT8 *punchout_spr1_ctrlram;
UINT8 *punchout_spr2_ctrlram;
UINT8 *punchout_palettebank;



/* these depend on jumpers(?) on the board and change from game to game */
static int palette_reverse_top, palette_reverse_bot;


DRIVER_INIT( punchout )
{
	palette_reverse_top = 0x00;
	palette_reverse_bot = 0xff;
}

DRIVER_INIT( spnchout )
{
	palette_reverse_top = 0x00;
	palette_reverse_bot = 0xff;
}

DRIVER_INIT( spnchotj )
{
	palette_reverse_top = 0xff;
	palette_reverse_bot = 0xff;
}

DRIVER_INIT( armwrest )
{
	palette_reverse_top = 0x00;
	palette_reverse_bot = 0x00;
}




/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

static TILE_GET_INFO( top_get_info )
{
	int attr = punchout_bg_top_videoram[tile_index*2 + 1];
	int code = punchout_bg_top_videoram[tile_index*2] + ((attr & 0x03) << 8);
	int color = ((attr & 0x7c) >> 2);
	int flipx = attr & 0x80;
	SET_TILE_INFO(0, code, color, flipx ? TILE_FLIPX : 0);
}

static TILE_GET_INFO( armwrest_top_get_info )
{
	int attr = punchout_bg_top_videoram[tile_index*2 + 1];
	int code = punchout_bg_top_videoram[tile_index*2] + ((attr & 0x03) << 8) + ((attr & 0x80) << 3);
	int color = ((attr & 0x7c) >> 2);
	SET_TILE_INFO(0, code, color, 0);
}

static TILE_GET_INFO( bot_get_info )
{
	int attr = punchout_bg_bot_videoram[tile_index*2 + 1];
	int code = punchout_bg_bot_videoram[tile_index*2] + ((attr & 0x03) << 8);
	int color = ((attr & 0x7c) >> 2);
	int flipx = attr & 0x80;
	SET_TILE_INFO(1, code, color, flipx ? TILE_FLIPX : 0);
}

static TILE_GET_INFO( armwrest_bot_get_info )
{
	int attr = punchout_bg_bot_videoram[tile_index*2 + 1];
	int code = punchout_bg_bot_videoram[tile_index*2] + ((attr & 0x03) << 8);
	int color = ((attr & 0x7c) >> 2) + 0x40;
	int flipx = attr & 0x80;
	SET_TILE_INFO(0, code, color, flipx ? TILE_FLIPX : 0);
}

static TILE_GET_INFO( bs1_get_info )
{
	int attr = punchout_spr1_videoram[tile_index*4 + 3];
	int code = punchout_spr1_videoram[tile_index*4] + ((punchout_spr1_videoram[tile_index*4 + 1] & 0x1f) << 8);
	int color = attr & 0x1f;
	int flipx = attr & 0x80;
	SET_TILE_INFO(2, code, color, flipx ? TILE_FLIPX : 0);
}

static TILE_GET_INFO( bs2_get_info )
{
	int attr = punchout_spr2_videoram[tile_index*4 + 3];
	int code = punchout_spr2_videoram[tile_index*4] + ((punchout_spr2_videoram[tile_index*4 + 1] & 0x0f) << 8);
	int color = attr & 0x3f;
	int flipx = attr & 0x80;
	SET_TILE_INFO(3, code, color, flipx ? TILE_FLIPX : 0);
}

static TILE_GET_INFO( armwrest_fg_get_info )
{
	int attr = armwrest_fg_videoram[tile_index*2 + 1];
	int code = armwrest_fg_videoram[tile_index*2] + 256 * (attr & 0x07);
	int color = ((attr & 0xf8) >> 3);
	int flipx = attr & 0x80;
	SET_TILE_INFO(1, code, color, flipx ? TILE_FLIPX : 0);
}

static TILEMAP_MAPPER( armwrest_bs1_scan )
{
	int halfcols = num_cols/2;
	return (col/halfcols)*(halfcols*num_rows) + row*halfcols + col%halfcols;
}

static TILEMAP_MAPPER( armwrest_bs1_scan_flipx )
{
	int halfcols = num_cols/2;
	col ^=0x10;
	return (col/halfcols)*(halfcols*num_rows) + row*halfcols + col%halfcols;
}


VIDEO_START( punchout )
{
	bg_top_tilemap = tilemap_create(machine, top_get_info, tilemap_scan_rows,  8,8, 32,32);
	bg_bot_tilemap = tilemap_create(machine, bot_get_info, tilemap_scan_rows,  8,8, 64,32);
	tilemap_set_scroll_rows(bg_bot_tilemap, 32);

	spr1_tilemap = tilemap_create(machine, bs1_get_info, tilemap_scan_rows,  8,8, 16,32);
	spr2_tilemap = tilemap_create(machine, bs2_get_info, tilemap_scan_rows,  8,8, 16,32);

	fg_tilemap = NULL;

	tilemap_set_transparent_pen(spr1_tilemap, 0x07);
	tilemap_set_transparent_pen(spr2_tilemap, 0x03);
}


VIDEO_START( armwrest )
{
	bg_top_tilemap = tilemap_create(machine, armwrest_top_get_info, tilemap_scan_rows,  8,8, 32,32);
	bg_bot_tilemap = tilemap_create(machine, armwrest_bot_get_info, tilemap_scan_rows,  8,8, 32,32);

	spr1_tilemap =       tilemap_create(machine, bs1_get_info, armwrest_bs1_scan,  8,8, 32,16);
	spr1_tilemap_flipx = tilemap_create(machine, bs1_get_info, armwrest_bs1_scan_flipx,  8,8, 32,16);
	spr2_tilemap = tilemap_create(machine, bs2_get_info, tilemap_scan_rows,  8,8, 16,32);
	fg_tilemap = tilemap_create(machine, armwrest_fg_get_info, tilemap_scan_rows,  8,8, 32,32);

	tilemap_set_transparent_pen(spr1_tilemap, 0x07);
	tilemap_set_transparent_pen(spr1_tilemap_flipx, 0x07);
	tilemap_set_transparent_pen(spr2_tilemap, 0x03);
	tilemap_set_transparent_pen(fg_tilemap, 0x07);
}



WRITE8_HANDLER( punchout_bg_top_videoram_w )
{
	punchout_bg_top_videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_top_tilemap, offset/2);
}

WRITE8_HANDLER( punchout_bg_bot_videoram_w )
{
	punchout_bg_bot_videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_bot_tilemap, offset/2);
}

WRITE8_HANDLER( armwrest_fg_videoram_w )
{
	armwrest_fg_videoram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap, offset/2);
}

WRITE8_HANDLER( punchout_spr1_videoram_w )
{
	punchout_spr1_videoram[offset] = data;
	tilemap_mark_tile_dirty(spr1_tilemap, offset/4);
	if (spr1_tilemap_flipx)
		tilemap_mark_tile_dirty(spr1_tilemap_flipx, offset/4);
}

WRITE8_HANDLER( punchout_spr2_videoram_w )
{
	punchout_spr2_videoram[offset] = data;
	tilemap_mark_tile_dirty(spr2_tilemap, offset/4);
}



static void draw_big_sprite(bitmap_t *bitmap, const rectangle *cliprect, int palette)
{
	int zoom;

	zoom = punchout_spr1_ctrlram[0] + 256 * (punchout_spr1_ctrlram[1] & 0x0f);
	if (zoom)
	{
		int sx,sy;
		UINT32 startx,starty;
		int incxx,incyy;

		sx = 4096 - (punchout_spr1_ctrlram[2] + 256 * (punchout_spr1_ctrlram[3] & 0x0f));
		if (sx > 4096-4*127) sx -= 4096;

		sy = -(punchout_spr1_ctrlram[4] + 256 * (punchout_spr1_ctrlram[5] & 1));
		if (sy <= -256 + zoom/0x40) sy += 512;
		sy += 12;

		incxx = zoom << 6;
		incyy = zoom << 6;

		startx = -sx * 0x4000;
		starty = -sy * 0x10000;
		startx += 3740 * zoom;	/* adjustment to match the screen shots */
		starty -= 178 * zoom;	/* and make the hall of fame picture nice */

		if (punchout_spr1_ctrlram[6] & 1)	/* flip x */
		{
			startx = ((16 * 8) << 16) - startx - 1;
			incxx = -incxx;
		}

		tilemap_set_palette_offset(spr1_tilemap, 0x100 * palette);

		tilemap_draw_roz(bitmap,cliprect,spr1_tilemap,
			startx,starty + 0x200*(2) * zoom,
			incxx,0,0,incyy,	/* zoom, no rotation */
			0,	/* no wraparound */
			0,0);
	}
}


static void armwrest_draw_big_sprite(bitmap_t *bitmap, const rectangle *cliprect, int palette)
{
	int zoom;

	zoom = punchout_spr1_ctrlram[0] + 256 * (punchout_spr1_ctrlram[1] & 0x0f);
	if (zoom)
	{
		int sx,sy;
		UINT32 startx,starty;
		int incxx,incyy;
		tilemap_t *_tilemap;

		sx = 4096 - (punchout_spr1_ctrlram[2] + 256 * (punchout_spr1_ctrlram[3] & 0x0f));
		if (sx > 2048) sx -= 4096;

		sy = -(punchout_spr1_ctrlram[4] + 256 * (punchout_spr1_ctrlram[5] & 1));
		if (sy <= -256 + zoom/0x40) sy += 512;
		sy += 12;

		incxx = zoom << 6;
		incyy = zoom << 6;

		startx = -sx * 0x4000;
		starty = -sy * 0x10000;
		startx += 3740 * zoom;	/* adjustment to match the screen shots */
		starty -= 178 * zoom;	/* and make the hall of fame picture nice */

		if (punchout_spr1_ctrlram[6] & 1)	/* flip x */
		{
			_tilemap = spr1_tilemap_flipx;
			startx = ((32 * 8) << 16) - startx - 1;
			incxx = -incxx;
		}
		else
			_tilemap = spr1_tilemap;

		tilemap_set_palette_offset(_tilemap, 0x100 * palette);

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

	sx = 512 - (punchout_spr2_ctrlram[0] + 256 * (punchout_spr2_ctrlram[1] & 1));
	if (sx > 512-127) sx -= 512;
	sx -= 55;	/* adjustment to match the screen shots */

	sy = -punchout_spr2_ctrlram[2] + 256 * (punchout_spr2_ctrlram[3] & 1);
	sy += 3;	/* adjustment to match the screen shots */

	sx = -sx << 16;
	sy = -sy << 16;

	if (punchout_spr2_ctrlram[4] & 1)	/* flip x */
	{
		sx = ((16 * 8) << 16) - sx - 1;
		incxx = -1;
	}
	else
		incxx = 1;

	// this tilemap doesn't actually zoom, but draw_roz is the only way to draw it without wraparound
	tilemap_draw_roz(bitmap,cliprect,spr2_tilemap,
		sx, sy, incxx << 16, 0, 0, 1 << 16,
		0, 0, 0);
}



static void punchout_copy_top_palette(running_machine *machine, int bank)
{
	int i;
	const UINT8 *color_prom = memory_region(machine, "proms");

	// top monitor palette
	for (i = 0; i < 0x100; i++)
	{
		int base = 0x100 * bank;
		int r, g, b;

		r = 255 - pal4bit(color_prom[i + 0x000 + base]);
		g = 255 - pal4bit(color_prom[i + 0x200 + base]);
		b = 255 - pal4bit(color_prom[i + 0x400 + base]);

		palette_set_color(machine, i ^ palette_reverse_top, MAKE_RGB(r, g, b));
	}
}

static void punchout_copy_bot_palette(running_machine *machine, int bank)
{
	int i;
	const UINT8 *color_prom = memory_region(machine, "proms") + 0x600;

	// bottom monitor palette
	for (i = 0; i < 0x100; i++)
	{
		int base = 0x100 * bank;
		int r, g, b;

		r = 255 - pal4bit(color_prom[i + 0x000 + base]);
		g = 255 - pal4bit(color_prom[i + 0x200 + base]);
		b = 255 - pal4bit(color_prom[i + 0x400 + base]);

		palette_set_color(machine, (i ^ palette_reverse_bot) + 0x100, MAKE_RGB(r, g, b));
	}
}


VIDEO_UPDATE( punchout )
{
	const device_config *top_screen    = devtag_get_device(screen->machine, "top");
	const device_config *bottom_screen = devtag_get_device(screen->machine, "bottom");

	if (screen == top_screen)
	{
		punchout_copy_top_palette(screen->machine, BIT(*punchout_palettebank,1));

		tilemap_draw(bitmap, cliprect, bg_top_tilemap, 0, 0);

		if (punchout_spr1_ctrlram[7] & 1)	/* display in top monitor */
			draw_big_sprite(bitmap, cliprect, 0);
	}
	else if (screen == bottom_screen)
	{
		int offs;

		punchout_copy_bot_palette(screen->machine, BIT(*punchout_palettebank,0));

		/* copy the character mapped graphics */
		for (offs = 0;offs < 32;offs++)
			tilemap_set_scrollx(bg_bot_tilemap, offs, 58 + punchout_bg_bot_videoram[2*offs] + 256 * (punchout_bg_bot_videoram[2*offs + 1] & 0x01));

		tilemap_draw(bitmap, cliprect, bg_bot_tilemap, 0, 0);

		if (punchout_spr1_ctrlram[7] & 2)	/* display in bottom monitor */
			draw_big_sprite(bitmap, cliprect, 1);
		drawbs2(screen->machine, bitmap, cliprect);
	}
	return 0;
}


VIDEO_UPDATE( armwrest )
{
	const device_config *top_screen    = devtag_get_device(screen->machine, "top");
	const device_config *bottom_screen = devtag_get_device(screen->machine, "bottom");

	if (screen == top_screen)
	{
		punchout_copy_top_palette(screen->machine, BIT(*punchout_palettebank,1));

		tilemap_draw(bitmap, cliprect, bg_top_tilemap, 0, 0);

		if (punchout_spr1_ctrlram[7] & 1)	/* display in top monitor */
			armwrest_draw_big_sprite(bitmap, cliprect, 0);
	}
	else if (screen == bottom_screen)
	{
		punchout_copy_bot_palette(screen->machine, BIT(*punchout_palettebank,0));

		tilemap_draw(bitmap, cliprect, bg_bot_tilemap, 0, 0);

		if (punchout_spr1_ctrlram[7] & 2)	/* display in bottom monitor */
			armwrest_draw_big_sprite(bitmap, cliprect, 1);
		drawbs2(screen->machine, bitmap, cliprect);

		tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 0);
	}
	return 0;
}
