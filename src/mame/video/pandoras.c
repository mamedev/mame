#include "driver.h"

static int flipscreen;
static tilemap *layer0;
extern UINT8 *pandoras_sharedram;

/***********************************************************************

  Convert the color PROMs into a more useable format.

  Pandora's Palace has one 32x8 palette PROM and two 256x4 lookup table
  PROMs (one for characters, one for sprites).
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
PALETTE_INIT( pandoras )
{
	int i;
	#define TOTAL_COLORS(gfxn) (machine->gfx[gfxn]->total_colors * machine->gfx[gfxn]->color_granularity)
	#define COLOR(gfxn,offs) (colortable[machine->drv->gfxdecodeinfo[gfxn].color_codes_start + offs])

	for (i = 0; i < machine->drv->total_colors; i++)
	{
		int bit0, bit1, bit2, r, g, b;

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

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_tile_info0 )
{
	UINT8 attr = colorram[tile_index];
	SET_TILE_INFO(
			0,
			videoram[tile_index] + ((attr & 0x10) << 4),
			attr & 0x0f,
			TILE_FLIPYX((attr & 0xc0) >> 6));
	tileinfo->category = (attr & 0x20) >> 5;
}

/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

VIDEO_START( pandoras )
{
	layer0 = tilemap_create(get_tile_info0,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,32,32);
}

/***************************************************************************

  Memory Handlers

***************************************************************************/

READ8_HANDLER( pandoras_vram_r )
{
	return videoram[offset];
}

READ8_HANDLER( pandoras_cram_r )
{
	return colorram[offset];
}

WRITE8_HANDLER( pandoras_vram_w )
{
	tilemap_mark_tile_dirty(layer0,offset);
	videoram[offset] = data;
}

WRITE8_HANDLER( pandoras_cram_w )
{
	tilemap_mark_tile_dirty(layer0,offset);
	colorram[offset] = data;
}

WRITE8_HANDLER( pandoras_scrolly_w )
{
	tilemap_set_scrolly(layer0,0,data);
}

WRITE8_HANDLER( pandoras_flipscreen_w )
{
	flipscreen = data;
	tilemap_set_flip(ALL_TILEMAPS, flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
}

/***************************************************************************

  Screen Refresh

***************************************************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, UINT8* sr)
{
	int offs;

	for (offs = 0; offs < 0x100; offs += 4)
	{
		int sx = sr[offs + 1];
		int sy = 240 - sr[offs];
		int nflipx = sr[offs + 3] & 0x40;
		int nflipy = sr[offs + 3] & 0x80;

		drawgfx(bitmap,machine->gfx[1],
			sr[offs + 2],
			sr[offs + 3] & 0x0f,
			!nflipx,!nflipy,
			sx,sy,
			cliprect,TRANSPARENCY_COLOR,0);
	}
}

VIDEO_UPDATE( pandoras )
{
	tilemap_draw( bitmap,cliprect, layer0, 1 ,0);
	draw_sprites( machine, bitmap,cliprect, &pandoras_sharedram[0x800] );
	tilemap_draw( bitmap,cliprect, layer0, 0 ,0);
	return 0;
}
