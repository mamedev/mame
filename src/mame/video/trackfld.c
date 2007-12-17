/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"

UINT8 *trackfld_scroll;
UINT8 *trackfld_scroll2;

static tilemap *bg_tilemap;
static int bg_bank = 0, sprite_bank1 = 0, sprite_bank2 = 0;

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Track 'n Field has one 32x8 palette PROM and two 256x4 lookup table PROMs
  (one for characters, one for sprites).
  The palette PROM is connected to the RGB output this way:

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

***************************************************************************/
PALETTE_INIT( trackfld )
{
	int i;
	#define TOTAL_COLORS(gfxn) (machine->gfx[gfxn]->total_colors * machine->gfx[gfxn]->color_granularity)
	#define COLOR(gfxn,offs) (colortable[machine->drv->gfxdecodeinfo[gfxn].color_codes_start + offs])


	for (i = 0;i < machine->drv->total_colors;i++)
	{
		int bit0,bit1,bit2,r,g,b;


		/* red component */
		bit0 = (*color_prom >> 0) & 0x01;
		bit1 = (*color_prom >> 1) & 0x01;
		bit2 = (*color_prom >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (*color_prom >> 3) & 0x01;
		bit1 = (*color_prom >> 4) & 0x01;
		bit2 = (*color_prom >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = (*color_prom >> 6) & 0x01;
		bit2 = (*color_prom >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));

		color_prom++;
	}

	/* color_prom now points to the beginning of the lookup table */


	/* sprites */
	for (i = 0;i < TOTAL_COLORS(1);i++)
		COLOR(1,i) = *(color_prom++) & 0x0f;

	/* characters */
	for (i = 0;i < TOTAL_COLORS(0);i++)
		COLOR(0,i) = (*(color_prom++) & 0x0f) + 0x10;
}

WRITE8_HANDLER( trackfld_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( trackfld_colorram_w )
{
	colorram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( trackfld_flipscreen_w )
{
	if (flip_screen != data)
	{
		flip_screen_set(data);
		tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
	}
}

WRITE8_HANDLER( atlantol_gfxbank_w )
{
	static int old = 0;

	if( data & 1 )
	{
		/* male / female sprites switch */
		if( (old == 1 && (data & 1) == 1) ||
			(old == 0 && (data & 1) == 1) )
			sprite_bank2 = 0x200;
		else
			sprite_bank2 = 0;

		sprite_bank1 = 0;

		old = data & 1;
	}
	else
	{
		/* male / female sprites switch */
		if( (old == 0 && (data & 1) == 0) ||
			(old == 1 && (data & 1) == 0) )
			sprite_bank2 = 0;
		else
			sprite_bank2 = 0x200;

		sprite_bank1 = 0;

		old = data & 1;
	}

	if( (data & 3) == 3 )
	{
		if( sprite_bank2 )
			sprite_bank1 = 0x500;
		else
			sprite_bank1 = 0x300;
	}
	else if( (data & 3) == 2 )
	{
		if( sprite_bank2 )
			sprite_bank1 = 0x300;
		else
			sprite_bank1 = 0x100;
	}

	if( bg_bank != (data & 0x8) )
	{
		bg_bank = data & 0x8;
		tilemap_mark_all_tiles_dirty(bg_tilemap);
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int attr = colorram[tile_index];
	int code = videoram[tile_index] + 4 * (attr & 0xc0);
	int color = attr & 0x0f;
	int flags = ((attr & 0x10) ? TILE_FLIPX : 0) | ((attr & 0x20) ? TILE_FLIPY : 0);

	if( bg_bank )
		code |= 0x400;

	SET_TILE_INFO(0, code, color, flags);
}

VIDEO_START( trackfld )
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows,
		TILEMAP_TYPE_PEN, 8, 8, 64, 32);

	tilemap_set_scroll_rows(bg_tilemap, 32);
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs;

	for (offs = spriteram_size - 2; offs >= 0; offs -= 2)
	{
		int attr = spriteram_2[offs];
		int code = spriteram[offs + 1];
		int color = attr & 0x0f;
		int flipx = ~attr & 0x40;
		int flipy = attr & 0x80;
		int sx = spriteram[offs] - 1;
		int sy = 240 - spriteram_2[offs + 1];

		if (flip_screen)
		{
			sy = 240 - sy;
			flipy = !flipy;
		}

		/* Note that this adjustement must be done AFTER handling flip screen, thus */
		/* proving that this is a hardware related "feature" */
		sy += 1;

		drawgfx(bitmap, machine->gfx[1],
			code + sprite_bank1 + sprite_bank2, color,
			flipx, flipy,
			sx, sy,
			cliprect,
			TRANSPARENCY_COLOR, 0);

		/* redraw with wraparound */
		drawgfx(bitmap,machine->gfx[1],
			code + sprite_bank1 + sprite_bank2, color,
			flipx, flipy,
			sx - 256, sy,
			cliprect,
			TRANSPARENCY_COLOR, 0);
	}
}

VIDEO_UPDATE( trackfld )
{
	int row, scrollx;

	for (row = 0; row < 32; row++)
	{
		scrollx = trackfld_scroll[row] + 256 * (trackfld_scroll2[row] & 0x01);
		if (flip_screen) scrollx = -scrollx;
		tilemap_set_scrollx(bg_tilemap, row, scrollx);
	}

	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	draw_sprites(machine, bitmap, cliprect);
	return 0;
}
