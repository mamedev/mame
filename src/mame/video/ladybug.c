/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"

/* Use the Zero Hour star generator board */
extern void redclash_set_stars_enable( UINT8 on );
extern void redclash_update_stars_state(void);
extern void redclash_set_stars_speed( UINT8 speed );
extern void redclash_draw_stars(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, UINT8 palette_offset, UINT8 sraider, UINT8 firstx, UINT8 lastx);

static tilemap *bg_tilemap;
static tilemap *grid_tilemap;

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Lady Bug has a 32 bytes palette PROM and a 32 bytes sprite color lookup
  table PROM.
  The palette PROM is connected to the RGB output this way:

  bit 7 -- inverter -- 220 ohm resistor  -- BLUE
        -- inverter -- 220 ohm resistor  -- GREEN
        -- inverter -- 220 ohm resistor  -- RED
        -- inverter -- 470 ohm resistor  -- BLUE
        -- unused
        -- inverter -- 470 ohm resistor  -- GREEN
        -- unused
  bit 0 -- inverter -- 470 ohm resistor  -- RED

***************************************************************************/
PALETTE_INIT( ladybug )
{
	int i;

	for (i = 0;i < 32;i++)
	{
		int bit1,bit2,r,g,b;


		bit1 = (~color_prom[i] >> 0) & 0x01;
		bit2 = (~color_prom[i] >> 5) & 0x01;
		r = 0x47 * bit1 + 0x97 * bit2;
		bit1 = (~color_prom[i] >> 2) & 0x01;
		bit2 = (~color_prom[i] >> 6) & 0x01;
		g = 0x47 * bit1 + 0x97 * bit2;
		bit1 = (~color_prom[i] >> 4) & 0x01;
		bit2 = (~color_prom[i] >> 7) & 0x01;
		b = 0x47 * bit1 + 0x97 * bit2;
		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}

	/* characters */
	for (i = 0;i < 8;i++)
	{
		colortable[4 * i] = 0;
		colortable[4 * i + 1] = i + 0x08;
		colortable[4 * i + 2] = i + 0x10;
		colortable[4 * i + 3] = i + 0x18;
	}

	/* sprites */
	for (i = 0;i < 4 * 8;i++)
	{
		int bit0,bit1,bit2,bit3;


		/* low 4 bits are for sprite n */
		bit0 = (color_prom[i + 32] >> 3) & 0x01;
		bit1 = (color_prom[i + 32] >> 2) & 0x01;
		bit2 = (color_prom[i + 32] >> 1) & 0x01;
		bit3 = (color_prom[i + 32] >> 0) & 0x01;
		colortable[i + 4 * 8] = 1 * bit0 + 2 * bit1 + 4 * bit2 + 8 * bit3;

		/* high 4 bits are for sprite n + 8 */
		bit0 = (color_prom[i + 32] >> 7) & 0x01;
		bit1 = (color_prom[i + 32] >> 6) & 0x01;
		bit2 = (color_prom[i + 32] >> 5) & 0x01;
		bit3 = (color_prom[i + 32] >> 4) & 0x01;
		colortable[i + 4 * 16] = 1 * bit0 + 2 * bit1 + 4 * bit2 + 8 * bit3;
	}
}

PALETTE_INIT( sraider )
{
	int i;

	for (i = 0;i < 32;i++)
	{
		int bit1,bit2,r,g,b;


		bit1 = (~color_prom[i] >> 3) & 0x01;
		bit2 = (~color_prom[i] >> 0) & 0x01;
		r = 0x47 * bit1 + 0x97 * bit2;
		bit1 = (~color_prom[i] >> 5) & 0x01;
		bit2 = (~color_prom[i] >> 4) & 0x01;
		g = 0x47 * bit1 + 0x97 * bit2;
		bit1 = (~color_prom[i] >> 7) & 0x01;
		bit2 = (~color_prom[i] >> 6) & 0x01;
		b = 0x47 * bit1 + 0x97 * bit2;
		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}

	/* This is for the stars colors */
	for (i = 32;i < 64;i++)
	{
			int bit1,bit2,r,g,b;


			bit2 = (i >> 4) & 0x01;
			bit1 = (i >> 3) & 0x01;
			b = 0x47 * bit1 + 0x97 * bit2;
			bit2 = (i >> 2) & 0x01;
			bit1 = (i >> 1) & 0x01;
			g = 0x47 * bit1 + 0x97 * bit2;
			bit1 = i & 0x01;
			r = 0x47 * bit1;
			palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}

	/* This is reserved for the grid color */
	palette_set_color(machine,64,MAKE_RGB(0,0,0));

	/* characters */
	for (i = 0;i < 8;i++)
	{
		colortable[4 * i] = 0;
		colortable[4 * i + 1] = i + 0x08;
		colortable[4 * i + 2] = i + 0x10;
		colortable[4 * i + 3] = i + 0x18;
	}

	/* sprites */
	for (i = 0;i < 4 * 8;i++)
	{
		int bit0,bit1,bit2,bit3;


		/* low 4 bits are for sprite n */
		bit0 = (color_prom[i + 32] >> 3) & 0x01;
		bit1 = (color_prom[i + 32] >> 2) & 0x01;
		bit2 = (color_prom[i + 32] >> 1) & 0x01;
		bit3 = (color_prom[i + 32] >> 0) & 0x01;
		colortable[i + 4 * 8] = 1 * bit0 + 2 * bit1 + 4 * bit2 + 8 * bit3;

		/* high 4 bits are for sprite n + 8 */
		bit0 = (color_prom[i + 32] >> 7) & 0x01;
		bit1 = (color_prom[i + 32] >> 6) & 0x01;
		bit2 = (color_prom[i + 32] >> 5) & 0x01;
		bit3 = (color_prom[i + 32] >> 4) & 0x01;
		colortable[i + 4 * 16] = 1 * bit0 + 2 * bit1 + 4 * bit2 + 8 * bit3;
	}

	/* stationary part of grid */
	colortable[32 + 64] = 0;
	colortable[32 + 64 + 1] = 64;
}

WRITE8_HANDLER( ladybug_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( ladybug_colorram_w )
{
	colorram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( ladybug_flipscreen_w )
{
	if (flip_screen != (data & 0x01))
	{
		flip_screen_set(data & 0x01);
		tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
	}
}

////////////////////////////////////////////////
/* All this should probably go somewhere else */

/* Sound comm between CPU's */
static UINT8 sraider_sound_low;

READ8_HANDLER( sraider_sound_low_r )
{
	return sraider_sound_low;
}

static UINT8 sraider_sound_high;

READ8_HANDLER( sraider_sound_high_r )
{
	return sraider_sound_high;
}

WRITE8_HANDLER( sraider_sound_low_w )
{
	sraider_sound_low = data;
}

WRITE8_HANDLER( sraider_sound_high_w )
{
	sraider_sound_high = data;
}

/* Protection? */

READ8_HANDLER( sraider_8005_r )
{
	/* This must return X011111X or cpu #1 will hang */
	/* see code at rst $10 */
	return 0x3e;
}

/* Unknown IO */

static UINT8 sraider_wierd_value[8];
static UINT8 sraider_0x30, sraider_0x38;

WRITE8_HANDLER( sraider_misc_w )
{
	switch(offset)
	{
		/* These 8 bits are stored in the latch at A7 */
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
			sraider_wierd_value[offset&7] = data&1;
			break;
		/* These 6 bits are stored in the latch at N7 */
		case 0x08:
			sraider_0x30 = data&0x3f;
			break;
		/* These 6 bits are stored in the latch at N8 */
		case 0x10:
			sraider_0x38 = data&0x3f;
			break;
		default:
			mame_printf_debug("(%04X) write to %02X\n",activecpu_get_pc(),offset);
			break;
	}
}

////////////////////////////////////////////////////

static UINT8 gridline[256];

WRITE8_HANDLER( sraider_grid_data_w )
{
	static int x = 0;

	if (x == 0)
	{
		tilemap_mark_all_tiles_dirty(grid_tilemap);
		x = 1;
	}
	gridline[offset] = data;

}

WRITE8_HANDLER( sraider_io_w )
{
	// bit7 = flip
	// bit6 = grid red
	// bit5 = grid green
	// bit4 = grid blue
	// bit3 = enable stars
	// bit210 = stars speed/dir

	if (flip_screen != (data & 0x80))
	{
		flip_screen_set(data & 0x80);
		tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
	}

	palette_set_color_rgb(Machine,64,
		              ((data&0x40)>>6)*0xff,
		              ((data&0x20)>>5)*0xff,
		              ((data&0x10)>>4)*0xff);

	redclash_set_stars_enable((data&0x08)>>3);

	/*
     * There must be a subtle clocking difference between
     * Space Raider and the other games using this star generator,
     * hence the -1 here
     */

	redclash_set_stars_speed((data&0x07) - 1);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int code = videoram[tile_index] + 32 * (colorram[tile_index] & 0x08);
	int color = colorram[tile_index] & 0x07;

	SET_TILE_INFO(0, code, color, 0);
}

static TILE_GET_INFO( get_grid_tile_info )
{
	if (tile_index < 512)
		SET_TILE_INFO(3, tile_index, 0, 0);
	else
	{
		int temp = tile_index/32;
		tile_index = (31 - temp) * 32 + (tile_index % 32);
		SET_TILE_INFO(4, tile_index, 0, 0);
	}
}

VIDEO_START( ladybug )
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows,
		TILEMAP_TYPE_PEN, 8, 8, 32, 32);

	tilemap_set_scroll_rows(bg_tilemap, 32);
}

VIDEO_START( sraider )
{
	grid_tilemap = tilemap_create(get_grid_tile_info, tilemap_scan_rows,
		TILEMAP_TYPE_PEN, 8, 8, 32, 32);

	tilemap_set_scroll_rows(grid_tilemap, 32);

	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows,
		TILEMAP_TYPE_PEN, 8, 8, 32, 32);

	tilemap_set_scroll_rows(bg_tilemap, 32);

	tilemap_set_transparent_pen(grid_tilemap, 0);
	tilemap_set_transparent_pen(bg_tilemap, 0);
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs;

	for (offs = spriteram_size - 2*0x40;offs >= 2*0x40;offs -= 0x40)
	{
		int i = 0;

		while (i < 0x40 && spriteram[offs + i] != 0)
			i += 4;

		while (i > 0)
		{
/*
 abccdddd eeeeeeee fffghhhh iiiiiiii

 a enable?
 b size (0 = 8x8, 1 = 16x16)
 cc flip
 dddd y offset
 eeeeeeee sprite code (shift right 2 bits for 16x16 sprites)
 fff unknown
 g sprite bank
 hhhh color
 iiiiiiii x position
*/
			i -= 4;

			if (spriteram[offs + i] & 0x80)
			{
				if (spriteram[offs + i] & 0x40)	/* 16x16 */
					drawgfx(bitmap,machine->gfx[1],
							(spriteram[offs + i + 1] >> 2) + 4 * (spriteram[offs + i + 2] & 0x10),
							spriteram[offs + i + 2] & 0x0f,
							spriteram[offs + i] & 0x20,spriteram[offs + i] & 0x10,
							spriteram[offs + i + 3],
							offs / 4 - 8 + (spriteram[offs + i] & 0x0f),
							cliprect,TRANSPARENCY_PEN,0);
				else	/* 8x8 */
					drawgfx(bitmap,machine->gfx[2],
							spriteram[offs + i + 1] + 16 * (spriteram[offs + i + 2] & 0x10),
							spriteram[offs + i + 2] & 0x0f,
							spriteram[offs + i] & 0x20,spriteram[offs + i] & 0x10,
							spriteram[offs + i + 3],
							offs / 4 + (spriteram[offs + i] & 0x0f),
							cliprect,TRANSPARENCY_PEN,0);
			}
		}
	}
}

VIDEO_UPDATE( ladybug )
{
	int offs;

	for (offs = 0; offs < 32; offs++)
	{
		int sx = offs % 4;
		int sy = offs / 4;

		if (flip_screen)
			tilemap_set_scrollx(bg_tilemap, offs, -videoram[32 * sx + sy]);
		else
			tilemap_set_scrollx(bg_tilemap, offs, videoram[32 * sx + sy]);
	}

	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	draw_sprites(machine, bitmap, cliprect);
	return 0;
}

VIDEO_EOF( sraider )	/* update starfield position */
{
	redclash_update_stars_state();
}

VIDEO_UPDATE( sraider )
{
	// this part is boilerplate from ladybug, not sure if hardware does this,
	// since it's not used

	int offs;
	int i;

	for (offs = 0; offs < 32; offs++)
	{
		int sx = offs % 4;
		int sy = offs / 4;

		if (flip_screen)
			tilemap_set_scrollx(bg_tilemap, offs, -videoram[32 * sx + sy]);
		else
			tilemap_set_scrollx(bg_tilemap, offs, videoram[32 * sx + sy]);
	}

	// clear the bg bitmap
	fillbitmap(bitmap,machine->pens[0],cliprect);

	// draw the stars
	if (flip_screen)
		redclash_draw_stars(machine,bitmap,cliprect,32,1,0x27,0xff);
	else
		redclash_draw_stars(machine,bitmap,cliprect,32,1,0x00,0xd8);

	// draw the horizontal gridlines
	tilemap_draw(bitmap, cliprect, grid_tilemap, 0, flip_screen);
	for(i=0;i<256;i++)
	{
		if (gridline[i] != 0)
		{
			if (flip_screen)
				plot_box(bitmap,i^0xff,0,1,255,machine->pens[64]);
			else
				plot_box(bitmap,i,0,1,255,machine->pens[64]);
		}
	}

	// now the chars
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, flip_screen);

	// now the sprites
	draw_sprites(machine, bitmap, cliprect);

	return 0;
}

