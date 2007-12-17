/***************************************************************************

 Lasso and similar hardware

    driver by Phil Stroffolino, Nicola Salmoria, Luca Elia


    Every game has 1 256 x 256 tilemap (non scrollable) made of 8 x 8
    tiles, and 16 x 16 sprites (some games use 32, some more).

    The graphics for tiles and sprites are held inside the same ROMs,
    but aren't shared between the two:

    the first $100 tiles are for the tilemap, the following $100 are
    for sprites. This constitutes the first graphics bank. There can
    be several.

    Lasso has an additional pixel layer (256 x 256 x 1) and a third
    CPU devoted to drawing into it (the lasso!)

    Wwjgtin has an additional $800 x $400 scrolling tilemap in ROM
    and $100 more 16 x 16 x 4 tiles for it.

    The colors are static ($40 colors, 2 PROMs) but the background
    color can be changed at runtime. Wwjgtin can change the last
    4 colors (= last palette) too.

***************************************************************************/

#include "driver.h"
#include "lasso.h"


UINT8 *lasso_videoram;
UINT8 *lasso_colorram;
UINT8 *lasso_spriteram;
size_t lasso_spriteram_size;
UINT8 *lasso_bitmap_ram; 	/* 0x2000 bytes for a 256 x 256 x 1 bitmap */
UINT8 *wwjgtin_track_scroll;

/* variables only used here: */
static tilemap *bg_tilemap, *track_tilemap;
static UINT8 gfxbank;				/* used by lasso, chameleo, wwjgtin and pinbo */
static UINT8 wwjgtin_track_enable;	/* used by wwjgtin */


/***************************************************************************


                            Colors (BBGGGRRR)


***************************************************************************/

static void lasso_set_color(int i, int data)
{
	int bit0,bit1,bit2,r,g,b;

	/* red component */
	bit0 = (data >> 0) & 0x01;
	bit1 = (data >> 1) & 0x01;
	bit2 = (data >> 2) & 0x01;
	r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
	/* green component */
	bit0 = (data >> 3) & 0x01;
	bit1 = (data >> 4) & 0x01;
	bit2 = (data >> 5) & 0x01;
	g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
	/* blue component */
	bit0 = (data >> 6) & 0x01;
	bit1 = (data >> 7) & 0x01;
	b = 0x4f * bit0 + 0xa8 * bit1;

	palette_set_color( Machine,i,MAKE_RGB(r,g,b) );
}

PALETTE_INIT( lasso )
{
	int i;

	for (i = 0;i < 0x40;i++)
	{
		lasso_set_color(i,*color_prom);
		color_prom++;
	}
}

/* 16 color tiles with a 4 color step for the palettes */
PALETTE_INIT( wwjgtin )
{
	int color, pen;

	palette_init_lasso(machine,colortable,color_prom);

	for( color = 0; color < 0x10; color++ )
		for( pen = 0; pen < 16; pen++ )
			colortable[color * 16 + pen + 4*16] = (color * 4 + pen) % 0x40;
}


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( lasso_get_bg_tile_info )
{
	int code  = lasso_videoram[tile_index];
	int color = lasso_colorram[tile_index];
	SET_TILE_INFO(0,
				  code + ((UINT16)gfxbank << 8),
				  color & 0x0f,
				  0);
}

static TILE_GET_INFO( wwjgtin_get_track_tile_info )
{
	UINT8 *ROM = memory_region(REGION_USER1);
	int code  = ROM[tile_index];
	int color = ROM[tile_index + 0x2000];
	SET_TILE_INFO(2,
				  code,
				  color & 0x0f,
				  0);
}

static TILE_GET_INFO( pinbo_get_bg_tile_info )
{
	int code  = lasso_videoram[tile_index];
	int color = lasso_colorram[tile_index];
	SET_TILE_INFO(0,
				  code + ((color & 0x30) << 4),
				  color & 0x0f,
				  0);
}


/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( lasso )
{
	/* create tilemap */
	bg_tilemap = tilemap_create(lasso_get_bg_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8,8, 32,32);

	/* register for saving */
	state_save_register_global(gfxbank);
}

VIDEO_START( wwjgtin )
{
	/* create tilemaps */
	bg_tilemap =    tilemap_create(lasso_get_bg_tile_info,      tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8,  32,  32);
	track_tilemap = tilemap_create(wwjgtin_get_track_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN,      16,16, 0x80,0x40);

	tilemap_set_transparent_pen(bg_tilemap,0);

	/* register for saving */
	state_save_register_global(gfxbank);
	state_save_register_global(wwjgtin_track_enable);
}

VIDEO_START( pinbo )
{
	/* create tilemap */
	bg_tilemap = tilemap_create(pinbo_get_bg_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8,8, 32,32);

	/* register for saving */
	state_save_register_global(gfxbank);
}


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

WRITE8_HANDLER( lasso_videoram_w )
{
	lasso_videoram[offset] = data;
	tilemap_mark_tile_dirty( bg_tilemap, offset );
}

WRITE8_HANDLER( lasso_colorram_w )
{
	lasso_colorram[offset] = data;
	tilemap_mark_tile_dirty( bg_tilemap, offset );
}


static WRITE8_HANDLER( lasso_flip_screen_w )
{
	/* don't know which is which, but they are always set together */
	flip_screen_x = data & 0x01;
	flip_screen_y = data & 0x02;

	tilemap_set_flip(ALL_TILEMAPS, (flip_screen_x ? TILEMAP_FLIPX : 0) |
								   (flip_screen_y ? TILEMAP_FLIPY : 0));
}


WRITE8_HANDLER( lasso_video_control_w )
{
	int bank = (data & 0x04) >> 2;

	if (gfxbank != bank)
	{
		gfxbank = bank;
		tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
	}

	lasso_flip_screen_w(offset, data);
}

WRITE8_HANDLER( wwjgtin_video_control_w )
{
	int bank = ((data & 0x04) ? 0 : 1) + ((data & 0x10) ? 2 : 0);
	wwjgtin_track_enable = data & 0x08;

	if (gfxbank != bank)
	{
		gfxbank = bank;
		tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
	}

	lasso_flip_screen_w(offset, data);
}

WRITE8_HANDLER( pinbo_video_control_w )
{
	/* no need to dirty the tilemap -- only the sprites use the global bank */
	gfxbank = (data & 0x0c) >> 2;

	lasso_flip_screen_w(offset, data);
}


/* The bg_tilemap color can be changed */
WRITE8_HANDLER( lasso_backcolor_w )
{
	int i;
	for( i=0; i<0x40; i+=4 ) /* stuff into color#0 of each palette */
		lasso_set_color(i,data);
}


/* The last 4 color (= last palette) entries can be changed */
WRITE8_HANDLER( wwjgtin_lastcolor_w )
{
	lasso_set_color(0x3f - offset,data);
}


/*************************************
 *
 *  Video update
 *
 *************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int reverse )
{
	const UINT8 *finish, *source;
	int inc;

	if (reverse)
	{
		source = lasso_spriteram;
		finish = lasso_spriteram + lasso_spriteram_size;
		inc	   = 4;
	}
	else
	{
		source = lasso_spriteram + lasso_spriteram_size - 4;
		finish = lasso_spriteram - 4;
		inc	   = -4;
	}

	while( source != finish )
	{
		int sx,sy,flipx,flipy;
		int code,color;

		sx = source[3];
		sy = source[0];
		flipx = source[1] & 0x40;
		flipy = source[1] & 0x80;

		if (flip_screen_x)
		{
			sx = 240 - sx;
			flipx = !flipx;
		}

		if (flip_screen_y)
		{
			flipy = !flipy;
		}
		else
		{
			sy = 240 - sy;
		}

		code = source[1] & 0x3f;
		color = source[2] & 0x0f;


        drawgfx(bitmap, machine->gfx[1],
				code | ((UINT16)gfxbank << 6),
				color,
				flipx, flipy,
				sx,sy,
				cliprect, TRANSPARENCY_PEN,0);

		source += inc;
    }
}


static void draw_lasso(running_machine *machine, mame_bitmap *bitmap)
{
	const UINT8 *source = lasso_bitmap_ram;
	int x,y;
	pen_t pen = machine->pens[0x3f];


	for (y = 0; y < 256; y++)
	{
		for (x = 0; x < 256; )
		{
			UINT8 data = *source++;

			if (data)
			{
				int bit;

				for (bit = 0; bit < 8; bit++)
				{
					if (data & 0x80)
					{
						if (flip_screen_x)
						{
							if (flip_screen_y)
								*BITMAP_ADDR16(bitmap, 255-y, 255-x) = pen;
							else
								*BITMAP_ADDR16(bitmap, y, 255-x) = pen;
						}
						else
						{
							if (flip_screen_y)
								*BITMAP_ADDR16(bitmap, 255-y, x) = pen;
							else
								*BITMAP_ADDR16(bitmap, y, x) = pen;
						}
					}

					x++;
					data <<= 1;
				}
			}
			else
			{
				x += 8;
			}
		}
	}
}


VIDEO_UPDATE( lasso )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	draw_lasso(machine, bitmap);
	draw_sprites(machine, bitmap, cliprect, 0);
	return 0;
}

VIDEO_UPDATE( chameleo )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	draw_sprites(machine, bitmap, cliprect, 0);
	return 0;
}


VIDEO_UPDATE( wwjgtin )
{
	tilemap_set_scrollx(track_tilemap,0,wwjgtin_track_scroll[0] + wwjgtin_track_scroll[1]*256);
	tilemap_set_scrolly(track_tilemap,0,wwjgtin_track_scroll[2] + wwjgtin_track_scroll[3]*256);

	if (wwjgtin_track_enable)
		tilemap_draw(bitmap, cliprect, track_tilemap, 0, 0);
	else
		fillbitmap(bitmap, machine->pens[0x40], cliprect);	// black

	draw_sprites(machine, bitmap, cliprect, 1);	// reverse order
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	return 0;
}
