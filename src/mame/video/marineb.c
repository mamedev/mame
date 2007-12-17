/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"
#include "includes/espial.h"


UINT8 *marineb_videoram;
UINT8 *marineb_colorram;
UINT8 marineb_active_low_flipscreen;

static UINT8 column_scroll;
static UINT8 palette_bank;
static UINT8 flipscreen_x;
static UINT8 flipscreen_y;
static tilemap *bg_tilemap;



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_tile_info )
{
	UINT8 code = marineb_videoram[tile_index];
	UINT8 col = marineb_colorram[tile_index];

	SET_TILE_INFO(0,
				  code | ((col & 0xc0) << 2),
				  (col & 0x0f) | (palette_bank << 4),
				  TILE_FLIPXY((col >> 4) & 0x03));
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( marineb )
{
	bg_tilemap = tilemap_create(get_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,32,32);

	tilemap_set_scroll_cols(bg_tilemap, 32);
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

WRITE8_HANDLER( marineb_videoram_w )
{
	marineb_videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}


WRITE8_HANDLER( marineb_colorram_w )
{
	marineb_colorram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}


WRITE8_HANDLER( marineb_column_scroll_w )
{
	column_scroll = data;
}


WRITE8_HANDLER( marineb_palette_bank_0_w )
{
	UINT8 old = palette_bank;

	palette_bank = (palette_bank & 0x02) | ((data & 0x01) << 0);

	if (old != palette_bank)
	{
		tilemap_mark_all_tiles_dirty(bg_tilemap);
	}
}


WRITE8_HANDLER( marineb_palette_bank_1_w )
{
	UINT8 old = palette_bank;

	palette_bank = (palette_bank & 0x01) | ((data & 0x01) << 1);

	if (old != palette_bank)
	{
		tilemap_mark_all_tiles_dirty(bg_tilemap);
	}
}


WRITE8_HANDLER( marineb_flipscreen_x_w )
{
	flipscreen_x = data ^ marineb_active_low_flipscreen;

	tilemap_set_flip(bg_tilemap, (flipscreen_x ? TILEMAP_FLIPX : 0) | (flipscreen_y ? TILEMAP_FLIPY : 0));
}


WRITE8_HANDLER( marineb_flipscreen_y_w )
{
	flipscreen_y = data ^ marineb_active_low_flipscreen;

	tilemap_set_flip(bg_tilemap, (flipscreen_x ? TILEMAP_FLIPX : 0) | (flipscreen_y ? TILEMAP_FLIPY : 0));
}



/*************************************
 *
 *  Video update
 *
 *************************************/

static void set_tilemap_scrolly(int cols)
{
	int col;

	for (col = 0; col < cols; col++)
		tilemap_set_scrolly(bg_tilemap, col, column_scroll);

	for (; col < 32; col++)
		tilemap_set_scrolly(bg_tilemap, col, 0);
}


VIDEO_UPDATE( marineb )
{
	int offs;


	set_tilemap_scrolly(24);
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);


	/* draw the sprites */
	for (offs = 0x0f; offs >= 0; offs--)
	{
		int gfx,sx,sy,code,col,flipx,flipy,offs2;


		if ((offs == 0) || (offs == 2))  continue;  /* no sprites here */


		if (offs < 8)
		{
			offs2 = 0x0018 + offs;
		}
		else
		{
			offs2 = 0x03d8 - 8 + offs;
		}


		code  = marineb_videoram[offs2];
		sx    = marineb_videoram[offs2 + 0x20];
		sy    = marineb_colorram[offs2];
		col   = (marineb_colorram[offs2 + 0x20] & 0x0f) + 16 * palette_bank;
		flipx =   code & 0x02;
		flipy = !(code & 0x01);

		if (offs < 4)
		{
			/* big sprite */
			gfx = 2;
			code = (code >> 4) | ((code & 0x0c) << 2);
		}
		else
		{
			/* small sprite */
			gfx = 1;
			code >>= 2;
		}

		if (!flipscreen_y)
		{
			sy = 256 - machine->gfx[gfx]->width - sy;
			flipy = !flipy;
		}

		if (flipscreen_x)
		{
			sx++;
		}

		drawgfx(bitmap,machine->gfx[gfx],
				code,
				col,
				flipx,flipy,
				sx,sy,
				cliprect,TRANSPARENCY_PEN,0);
	}
	return 0;
}


VIDEO_UPDATE( changes )
{
	int offs,sx,sy,code,col,flipx,flipy;


	set_tilemap_scrolly(26);
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);


	/* draw the small sprites */
	for (offs = 0x05; offs >= 0; offs--)
	{
		int offs2;


		offs2 = 0x001a + offs;

		code  = marineb_videoram[offs2];
		sx    = marineb_videoram[offs2 + 0x20];
		sy    = marineb_colorram[offs2];
		col   = (marineb_colorram[offs2 + 0x20] & 0x0f) + 16 * palette_bank;
		flipx =   code & 0x02;
		flipy = !(code & 0x01);

		if (!flipscreen_y)
		{
			sy = 256 - machine->gfx[1]->width - sy;
			flipy = !flipy;
		}

		if (flipscreen_x)
		{
			sx++;
		}

		drawgfx(bitmap,machine->gfx[1],
				code >> 2,
				col,
				flipx,flipy,
				sx,sy,
				cliprect,TRANSPARENCY_PEN,0);
	}

	/* draw the big sprite */

	code  = marineb_videoram[0x3df];
	sx    = marineb_videoram[0x3ff];
	sy    = marineb_colorram[0x3df];
	col   = marineb_colorram[0x3ff];
	flipx =   code & 0x02;
	flipy = !(code & 0x01);

	if (!flipscreen_y)
	{
		sy = 256 - machine->gfx[2]->width - sy;
		flipy = !flipy;
	}

	if (flipscreen_x)
	{
		sx++;
	}

	code >>= 4;

	drawgfx(bitmap,machine->gfx[2],
			code,
			col,
			flipx,flipy,
			sx,sy,
			cliprect,TRANSPARENCY_PEN,0);

	/* draw again for wrap around */

	drawgfx(bitmap,machine->gfx[2],
			code,
			col,
			flipx,flipy,
			sx-256,sy,
			cliprect,TRANSPARENCY_PEN,0);
	return 0;
}


VIDEO_UPDATE( springer )
{
	int offs;


	set_tilemap_scrolly(0);
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);


	/* draw the sprites */
	for (offs = 0x0f; offs >= 0; offs--)
	{
		int gfx,sx,sy,code,col,flipx,flipy,offs2;


		if ((offs == 0) || (offs == 2))  continue;  /* no sprites here */


		offs2 = 0x0010 + offs;


		code  = marineb_videoram[offs2];
		sx    = 240 - marineb_videoram[offs2 + 0x20];
		sy    = marineb_colorram[offs2];
		col   = (marineb_colorram[offs2 + 0x20] & 0x0f) + 16 * palette_bank;
		flipx = !(code & 0x02);
		flipy = !(code & 0x01);

		if (offs < 4)
		{
			/* big sprite */
			sx -= 0x10;
			gfx = 2;
			code = (code >> 4) | ((code & 0x0c) << 2);
		}
		else
		{
			/* small sprite */
			gfx = 1;
			code >>= 2;
		}

		if (!flipscreen_y)
		{
			sy = 256 - machine->gfx[gfx]->width - sy;
			flipy = !flipy;
		}

		if (!flipscreen_x)
		{
			sx--;
		}

		drawgfx(bitmap,machine->gfx[gfx],
				code,
				col,
				flipx,flipy,
				sx,sy,
				cliprect,TRANSPARENCY_PEN,0);
	}
	return 0;
}


VIDEO_UPDATE( hoccer )
{
	int offs;


	set_tilemap_scrolly(0);
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);


	/* draw the sprites */
	for (offs = 0x07; offs >= 0; offs--)
	{
		int sx,sy,code,col,flipx,flipy,offs2;


		offs2 = 0x0018 + offs;


		code  = spriteram[offs2];
		sx    = spriteram[offs2 + 0x20];
		sy    = marineb_colorram[offs2];
		col   = marineb_colorram[offs2 + 0x20];
		flipx =   code & 0x02;
		flipy = !(code & 0x01);

		if (!flipscreen_y)
		{
			sy = 256 - machine->gfx[1]->width - sy;
			flipy = !flipy;
		}

		if (flipscreen_x)
		{
			sx = 256 - machine->gfx[1]->width - sx;
			flipx = !flipx;
		}

		drawgfx(bitmap,machine->gfx[1],
				code >> 2,
				col,
				flipx,flipy,
				sx,sy,
				cliprect,TRANSPARENCY_PEN,0);
	}
	return 0;
}


VIDEO_UPDATE( hopprobo )
{
	int offs;


	set_tilemap_scrolly(0);
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);


	/* draw the sprites */
	for (offs = 0x0f; offs >= 0; offs--)
	{
		int gfx,sx,sy,code,col,flipx,flipy,offs2;


		if ((offs == 0) || (offs == 2))  continue;  /* no sprites here */


		offs2 = 0x0010 + offs;


		code  = marineb_videoram[offs2];
		sx    = marineb_videoram[offs2 + 0x20];
		sy    = marineb_colorram[offs2];
		col   = (marineb_colorram[offs2 + 0x20] & 0x0f) + 16 * palette_bank;
		flipx =   code & 0x02;
		flipy = !(code & 0x01);

		if (offs < 4)
		{
			/* big sprite */
			gfx = 2;
			code = (code >> 4) | ((code & 0x0c) << 2);
		}
		else
		{
			/* small sprite */
			gfx = 1;
			code >>= 2;
		}

		if (!flipscreen_y)
		{
			sy = 256 - machine->gfx[gfx]->width - sy;
			flipy = !flipy;
		}

		if (!flipscreen_x)
		{
			sx--;
		}

		drawgfx(bitmap,machine->gfx[gfx],
				code,
				col,
				flipx,flipy,
				sx,sy,
				cliprect,TRANSPARENCY_PEN,0);
	}
	return 0;
}
