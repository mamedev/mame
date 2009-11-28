/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"
#include "video/resnet.h"


UINT8 *sraider_grid_data;
static UINT8 sraider_grid_color;

/* Use the Zero Hour star generator board */
void redclash_set_stars_enable( UINT8 on );
void redclash_update_stars_state(void);
void redclash_set_stars_speed( UINT8 speed );
void redclash_draw_stars(bitmap_t *bitmap, const rectangle *cliprect, UINT8 palette_offset, UINT8 sraider, UINT8 firstx, UINT8 lastx);

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

static void palette_init_common(running_machine *machine, const UINT8 *color_prom, int colortable_size,
								int r_bit0, int r_bit1, int g_bit0, int g_bit1, int b_bit0, int b_bit1)
{
	static const int resistances[2] = { 470, 220 };
	double rweights[2], gweights[2], bweights[2];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0,	255, -1.0,
			2, resistances, rweights, 470, 0,
			2, resistances, gweights, 470, 0,
			2, resistances, bweights, 470, 0);

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, colortable_size);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x20; i++)
	{
		int bit0, bit1;
		int r, g, b;

		/* red component */
		bit0 = (~color_prom[i] >> r_bit0) & 0x01;
		bit1 = (~color_prom[i] >> r_bit1) & 0x01;
		r = combine_2_weights(rweights, bit0, bit1);

		/* green component */
		bit0 = (~color_prom[i] >> g_bit0) & 0x01;
		bit1 = (~color_prom[i] >> g_bit1) & 0x01;
		g = combine_2_weights(gweights, bit0, bit1);

		/* blue component */
		bit0 = (~color_prom[i] >> b_bit0) & 0x01;
		bit1 = (~color_prom[i] >> b_bit1) & 0x01;
		b = combine_2_weights(bweights, bit0, bit1);

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x20;

	/* characters */
	for (i = 0; i < 0x20; i++)
	{
		UINT8 ctabentry = ((i << 3) & 0x18) | ((i >> 2) & 0x07);
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}

	/* sprites */
	for (i = 0x20; i < 0x40; i++)
	{
		UINT8 ctabentry = color_prom[(i - 0x20) >> 1];

		ctabentry = BITSWAP8((color_prom[i - 0x20] >> 0) & 0x0f, 7,6,5,4,0,1,2,3);
		colortable_entry_set_value(machine->colortable, i + 0x00, ctabentry);

		ctabentry = BITSWAP8((color_prom[i - 0x20] >> 4) & 0x0f, 7,6,5,4,0,1,2,3);
		colortable_entry_set_value(machine->colortable, i + 0x20, ctabentry);
	}
}


PALETTE_INIT( ladybug )
{
	palette_init_common(machine, color_prom, 0x20, 0, 5, 2, 6, 4, 7);
}

PALETTE_INIT( sraider )
{
	int i;

	/* the resistor net may be probably different than Lady Bug */
	palette_init_common(machine, color_prom, 0x41, 3, 0, 5, 4, 7, 6);

	/* star colors */
	for (i = 0x20; i < 0x40; i++)
	{
		int bit0, bit1;
		int r, g, b;

		/* red component */
		bit0 = ((i - 0x20) >> 3) & 0x01;
		bit1 = ((i - 0x20) >> 4) & 0x01;
		b = 0x47 * bit0 + 0x97 * bit1;

		/* green component */
		bit0 = ((i - 0x20) >> 1) & 0x01;
		bit1 = ((i - 0x20) >> 2) & 0x01;
		g = 0x47 * bit0 + 0x97 * bit1;

		/* blue component */
		bit0 = ((i - 0x20) >> 0) & 0x01;
		r = 0x47 * bit0;

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r, g, b));
	}

	for (i = 0x60; i < 0x80; i++)
		colortable_entry_set_value(machine->colortable, i, (i - 0x60) + 0x20);

	/* stationary part of grid */
	colortable_entry_set_value(machine->colortable, 0x81, 0x40);
}

WRITE8_HANDLER( ladybug_videoram_w )
{
	space->machine->generic.videoram.u8[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( ladybug_colorram_w )
{
	space->machine->generic.colorram.u8[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( ladybug_flipscreen_w )
{
	if (flip_screen_get(space->machine) != (data & 0x01))
	{
		flip_screen_set(space->machine, data & 0x01);
		tilemap_mark_all_tiles_dirty_all(space->machine);
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
			mame_printf_debug("(%04X) write to %02X\n",cpu_get_pc(space->cpu),offset);
			break;
	}
}

////////////////////////////////////////////////////

WRITE8_HANDLER( sraider_io_w )
{
	// bit7 = flip
	// bit6 = grid red
	// bit5 = grid green
	// bit4 = grid blue
	// bit3 = enable stars
	// bit210 = stars speed/dir

	if (flip_screen_get(space->machine) != (data & 0x80))
	{
		flip_screen_set(space->machine, data & 0x80);
		tilemap_mark_all_tiles_dirty_all(space->machine);
	}

	sraider_grid_color = data & 0x70;

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
	int code = machine->generic.videoram.u8[tile_index] + 32 * (machine->generic.colorram.u8[tile_index] & 0x08);
	int color = machine->generic.colorram.u8[tile_index] & 0x07;

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
	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	tilemap_set_scroll_rows(bg_tilemap, 32);
	tilemap_set_transparent_pen(bg_tilemap, 0);
}

VIDEO_START( sraider )
{
	grid_tilemap = tilemap_create(machine, get_grid_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	tilemap_set_scroll_rows(grid_tilemap, 32);
	tilemap_set_transparent_pen(grid_tilemap, 0);

	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	tilemap_set_scroll_rows(bg_tilemap, 32);
	tilemap_set_transparent_pen(bg_tilemap, 0);
}

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	UINT8 *spriteram = machine->generic.spriteram.u8;
	int offs;

	for (offs = machine->generic.spriteram_size - 2*0x40;offs >= 2*0x40;offs -= 0x40)
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
					drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
							(spriteram[offs + i + 1] >> 2) + 4 * (spriteram[offs + i + 2] & 0x10),
							spriteram[offs + i + 2] & 0x0f,
							spriteram[offs + i] & 0x20,spriteram[offs + i] & 0x10,
							spriteram[offs + i + 3],
							offs / 4 - 8 + (spriteram[offs + i] & 0x0f),0);
				else	/* 8x8 */
					drawgfx_transpen(bitmap,cliprect,machine->gfx[2],
							spriteram[offs + i + 1] + 16 * (spriteram[offs + i + 2] & 0x10),
							spriteram[offs + i + 2] & 0x0f,
							spriteram[offs + i] & 0x20,spriteram[offs + i] & 0x10,
							spriteram[offs + i + 3],
							offs / 4 + (spriteram[offs + i] & 0x0f),0);
			}
		}
	}
}

VIDEO_UPDATE( ladybug )
{
	int offs;

	// clear the bg bitmap
	bitmap_fill(bitmap,cliprect,0);

	for (offs = 0; offs < 32; offs++)
	{
		int sx = offs % 4;
		int sy = offs / 4;

		if (flip_screen_get(screen->machine))
			tilemap_set_scrollx(bg_tilemap, offs, -screen->machine->generic.videoram.u8[32 * sx + sy]);
		else
			tilemap_set_scrollx(bg_tilemap, offs, screen->machine->generic.videoram.u8[32 * sx + sy]);
	}

	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
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

		if (flip_screen_get(screen->machine))
			tilemap_set_scrollx(bg_tilemap, offs, -screen->machine->generic.videoram.u8[32 * sx + sy]);
		else
			tilemap_set_scrollx(bg_tilemap, offs, screen->machine->generic.videoram.u8[32 * sx + sy]);
	}

	// clear the bg bitmap
	bitmap_fill(bitmap,cliprect,0);

	// draw the stars
	if (flip_screen_get(screen->machine))
		redclash_draw_stars(bitmap,cliprect,0x60,1,0x27,0xff);
	else
		redclash_draw_stars(bitmap,cliprect,0x60,1,0x00,0xd8);

	// draw the gridlines
	colortable_palette_set_color(screen->machine->colortable, 0x40, MAKE_RGB(sraider_grid_color & 0x40 ? 0xff : 0,
		              					  									 sraider_grid_color & 0x20 ? 0xff : 0,
		              					  									 sraider_grid_color & 0x10 ? 0xff : 0));
	tilemap_draw(bitmap, cliprect, grid_tilemap, 0, flip_screen_get(screen->machine));

	for (i = 0; i < 0x100; i++)
	{
		if (sraider_grid_data[i] != 0)
		{
			UINT8 x = i;

			int height = cliprect->max_y - cliprect->min_y + 1;

			if (flip_screen_get(screen->machine))
				x = ~x;

			plot_box(bitmap, x, cliprect->min_y, 1, height, 0x81);
		}
	}

	// now the chars
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, flip_screen_get(screen->machine));

	// now the sprites
	draw_sprites(screen->machine, bitmap, cliprect);

	return 0;
}

