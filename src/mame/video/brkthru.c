/***************************************************************************

    video/brkthru.c

***************************************************************************/

#include "driver.h"


//UINT8 *brkthru_scroll;
UINT8 *brkthru_videoram;
size_t brkthru_videoram_size;
static int bgscroll;
static int bgbasecolor;
static int flipscreen;

static tilemap *fg_tilemap;
static tilemap *bg_tilemap;


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Break Thru has one 256x8 and one 256x4 palette PROMs.
  I don't know for sure how the palette PROMs are connected to the RGB
  output, but it's probably the usual:

  bit 7 -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 2.2kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
        -- 1  kohm resistor  -- RED
  bit 0 -- 2.2kohm resistor  -- RED

  bit 3 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 1  kohm resistor  -- BLUE
  bit 0 -- 2.2kohm resistor  -- BLUE

***************************************************************************/
PALETTE_INIT( brkthru )
{
	int i;


	for (i = 0;i < machine->config->total_colors;i++)
	{
		int bit0,bit1,bit2,bit3,r,g,b;


		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		bit3 = (color_prom[0] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[0] >> 4) & 0x01;
		bit1 = (color_prom[0] >> 5) & 0x01;
		bit2 = (color_prom[0] >> 6) & 0x01;
		bit3 = (color_prom[0] >> 7) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[machine->config->total_colors] >> 0) & 0x01;
		bit1 = (color_prom[machine->config->total_colors] >> 1) & 0x01;
		bit2 = (color_prom[machine->config->total_colors] >> 2) & 0x01;
		bit3 = (color_prom[machine->config->total_colors] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));

		color_prom++;
	}
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

static TILE_GET_INFO( get_bg_tile_info )
{
	/* BG RAM format
        0         1
        ---- -c-- ---- ---- = Color
        ---- --xx xxxx xxxx = Code
    */

	int code = (videoram[tile_index*2] | ((videoram[tile_index*2+1]) << 8)) & 0x3ff;
	int region = 1 + (code >> 7);
	int colour = bgbasecolor + ((videoram[tile_index*2+1] & 0x04) >> 2);

	SET_TILE_INFO(region, code & 0x7f,colour,0);
}

WRITE8_HANDLER( brkthru_bgram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,offset/2);
}


static TILE_GET_INFO( get_fg_tile_info )
{
	UINT8 code = brkthru_videoram[tile_index];
	SET_TILE_INFO(0, code, 0, 0);
}

WRITE8_HANDLER( brkthru_fgram_w )
{
	brkthru_videoram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap,offset);
}

VIDEO_START( brkthru )
{
	fg_tilemap = tilemap_create(machine, get_fg_tile_info,tilemap_scan_rows,8,8,32,32);
	bg_tilemap = tilemap_create(machine, get_bg_tile_info,tilemap_scan_cols,16,16,32,16);

	tilemap_set_transparent_pen( fg_tilemap, 0 );
	tilemap_set_transparent_pen( bg_tilemap, 0 );
}



WRITE8_HANDLER( brkthru_1800_w )
{
	if (offset == 0)	/* low 8 bits of scroll */
		bgscroll = (bgscroll & 0x100) | data;
	else if (offset == 1)
	{
		int bankaddress;
		UINT8 *RAM = memory_region(space->machine, "maincpu");


		/* bit 0-2 = ROM bank select */
		bankaddress = 0x10000 + (data & 0x07) * 0x2000;
		memory_set_bankptr(space->machine, 1,&RAM[bankaddress]);

		/* bit 3-5 = background tiles color code */
		if (((data & 0x38) >> 2) != bgbasecolor)
		{
			bgbasecolor = (data & 0x38) >> 2;
			tilemap_mark_all_tiles_dirty (bg_tilemap);
		}

		/* bit 6 = screen flip */
		if (flipscreen != (data & 0x40))
		{
			flipscreen = data & 0x40;
			tilemap_set_flip(bg_tilemap,flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
			tilemap_set_flip(fg_tilemap,flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

		}

		/* bit 7 = high bit of scroll */
		bgscroll = (bgscroll & 0xff) | ((data & 0x80) << 1);
	}
}


#if 0
static void show_register( bitmap_t *bitmap, int x, int y, UINT32 data )
{
	char buf[5];

	sprintf(buf, "%04X", data);
	ui_draw_text(y, x, buf);
}
#endif


static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int prio )
{
	int offs;
	/* Draw the sprites. Note that it is important to draw them exactly in this */
	/* order, to have the correct priorities. */

	/* Sprite RAM format
        0         1         2         3
        ccc- ---- ---- ---- ---- ---- ---- ---- = Color
        ---d ---- ---- ---- ---- ---- ---- ---- = Double Size
        ---- p--- ---- ---- ---- ---- ---- ---- = Priority
        ---- -bb- ---- ---- ---- ---- ---- ---- = Bank
        ---- ---e ---- ---- ---- ---- ---- ---- = Enable/Disable
        ---- ---- ssss ssss ---- ---- ---- ---- = Sprite code
        ---- ---- ---- ---- yyyy yyyy ---- ---- = Y position
        ---- ---- ---- ---- ---- ---- xxxx xxxx = X position
    */

	for (offs = 0;offs < spriteram_size; offs += 4)
	{
		if ((spriteram[offs] & 0x09) == prio)	/* Enable && Low Priority */
		{
			int sx,sy,code,color;

			sx = 240 - spriteram[offs+3];
			if (sx < -7) sx += 256;
			sy = 240 - spriteram[offs+2];
			code = spriteram[offs+1] + 128 * (spriteram[offs] & 0x06);
			color = (spriteram[offs] & 0xe0) >> 5;
			if (flipscreen)
			{
				sx = 240 - sx;
				sy = 240 - sy;
			}

			if (spriteram[offs] & 0x10)	/* double height */
			{
				drawgfx_transpen(bitmap,cliprect,machine->gfx[9],
						code & ~1,
						color,
						flipscreen,flipscreen,
						sx,flipscreen? sy + 16 : sy - 16,0);
				drawgfx_transpen(bitmap,cliprect,machine->gfx[9],
						code | 1,
						color,
						flipscreen,flipscreen,
						sx,sy,0);

				/* redraw with wraparound */
				drawgfx_transpen(bitmap,cliprect,machine->gfx[9],
						code & ~1,
						color,
						flipscreen,flipscreen,
						sx,(flipscreen? sy + 16 : sy - 16) + 256,0);
				drawgfx_transpen(bitmap,cliprect,machine->gfx[9],
						code | 1,
						color,
						flipscreen,flipscreen,
						sx,sy + 256,0);

			}
			else
			{
				drawgfx_transpen(bitmap,cliprect,machine->gfx[9],
						code,
						color,
						flipscreen,flipscreen,
						sx,sy,0);

				/* redraw with wraparound */
				drawgfx_transpen(bitmap,cliprect,machine->gfx[9],
						code,
						color,
						flipscreen,flipscreen,
						sx,sy + 256,0);

			}
			}
		}
	}

VIDEO_UPDATE( brkthru )
	{
	tilemap_set_scrollx(bg_tilemap,0, bgscroll);
	tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_OPAQUE,0);

	/* low priority sprites */
	draw_sprites(screen->machine, bitmap, cliprect, 0x01 );

	/* draw background over low priority sprites */
	tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);

	/* high priority sprites */
	draw_sprites(screen->machine, bitmap, cliprect, 0x09 );

	/* fg layer */
	tilemap_draw(bitmap,cliprect,fg_tilemap,0,0);

/*  show_register(bitmap,8,8,(UINT32)flipscreen); */

	return 0;
}
