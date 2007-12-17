#include "driver.h"

UINT8 *timeplt_videoram,*timeplt_colorram;
static tilemap *bg_tilemap;

/*
sprites are multiplexed, so we have to buffer the spriteram
scanline by scanline.
*/
static UINT8 *sprite_mux_buffer,*sprite_mux_buffer_2;
static int scanline;


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Time Pilot has two 32x8 palette PROMs and two 256x4 lookup table PROMs
  (one for characters, one for sprites).
  The palette PROMs are connected to the RGB output this way:

  bit 7 -- 390 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 560 ohm resistor  -- BLUE
        -- 820 ohm resistor  -- BLUE
        -- 1.2kohm resistor  -- BLUE
        -- 390 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
  bit 0 -- 560 ohm resistor  -- GREEN

  bit 7 -- 820 ohm resistor  -- GREEN
        -- 1.2kohm resistor  -- GREEN
        -- 390 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
        -- 560 ohm resistor  -- RED
        -- 820 ohm resistor  -- RED
        -- 1.2kohm resistor  -- RED
  bit 0 -- not connected

***************************************************************************/
PALETTE_INIT( timeplt )
{
	int i;
	#define TOTAL_COLORS(gfxn) (machine->gfx[gfxn]->total_colors * machine->gfx[gfxn]->color_granularity)
	#define COLOR(gfxn,offs) (colortable[machine->drv->gfxdecodeinfo[gfxn].color_codes_start + offs])


	for (i = 0;i < machine->drv->total_colors;i++)
	{
		int bit0,bit1,bit2,bit3,bit4,r,g,b;


		bit0 = (color_prom[i + machine->drv->total_colors] >> 1) & 0x01;
		bit1 = (color_prom[i + machine->drv->total_colors] >> 2) & 0x01;
		bit2 = (color_prom[i + machine->drv->total_colors] >> 3) & 0x01;
		bit3 = (color_prom[i + machine->drv->total_colors] >> 4) & 0x01;
		bit4 = (color_prom[i + machine->drv->total_colors] >> 5) & 0x01;
		r = 0x19 * bit0 + 0x24 * bit1 + 0x35 * bit2 + 0x40 * bit3 + 0x4d * bit4;
		bit0 = (color_prom[i + machine->drv->total_colors] >> 6) & 0x01;
		bit1 = (color_prom[i + machine->drv->total_colors] >> 7) & 0x01;
		bit2 = (color_prom[i] >> 0) & 0x01;
		bit3 = (color_prom[i] >> 1) & 0x01;
		bit4 = (color_prom[i] >> 2) & 0x01;
		g = 0x19 * bit0 + 0x24 * bit1 + 0x35 * bit2 + 0x40 * bit3 + 0x4d * bit4;
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		bit3 = (color_prom[i] >> 6) & 0x01;
		bit4 = (color_prom[i] >> 7) & 0x01;
		b = 0x19 * bit0 + 0x24 * bit1 + 0x35 * bit2 + 0x40 * bit3 + 0x4d * bit4;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}

	color_prom += 2*machine->drv->total_colors;
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

static TILE_GET_INFO( get_tile_info )
{
	UINT8 attr = timeplt_colorram[tile_index];
	tileinfo->category = (attr & 0x10) >> 4;
	SET_TILE_INFO(
			0,
			timeplt_videoram[tile_index] + ((attr & 0x20) << 3),
			attr & 0x1f,
			TILE_FLIPYX((attr & 0xc0) >> 6));
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( timeplt )
{
	bg_tilemap = tilemap_create(get_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,32,32);

	sprite_mux_buffer = auto_malloc(256 * spriteram_size);
	sprite_mux_buffer_2 = auto_malloc(256 * spriteram_size);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( timeplt_videoram_w )
{
	timeplt_videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,offset);
}

WRITE8_HANDLER( timeplt_colorram_w )
{
	timeplt_colorram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,offset);
}

WRITE8_HANDLER( timeplt_flipscreen_w )
{
	flip_screen_set(~data & 1);
}

/* Return the current video scan line */
READ8_HANDLER( timeplt_scanline_r )
{
	return scanline;
}



/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect)
{
	const gfx_element *gfx = machine->gfx[1];
	rectangle clip = *cliprect;
	int offs;
	int line;


	for (line = 0;line < 256;line++)
	{
		if (line >= cliprect->min_y && line <= cliprect->max_y)
		{
			UINT8 *sr,*sr2;

			sr = sprite_mux_buffer + line * spriteram_size;
			sr2 = sprite_mux_buffer_2 + line * spriteram_size;
			clip.min_y = clip.max_y = line;

			for (offs = spriteram_size - 2;offs >= 0;offs -= 2)
			{
				int code,color,sx,sy,flipx,flipy;

				sx = sr[offs];
				sy = 241 - sr2[offs + 1];

				if (sy > line-16 && sy <= line)
				{
					code = sr[offs + 1];
					color = sr2[offs] & 0x3f;
					flipx = ~sr2[offs] & 0x40;
					flipy = sr2[offs] & 0x80;

					drawgfx(bitmap,gfx,
							code,
							color,
							flipx,flipy,
							sx,sy,
							&clip,TRANSPARENCY_PEN,0);
				}
			}
		}
	}
}

VIDEO_UPDATE( timeplt )
{
	tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);
	draw_sprites(machine, bitmap,cliprect);
	tilemap_draw(bitmap,cliprect,bg_tilemap,1,0);
	return 0;
}


INTERRUPT_GEN( timeplt_interrupt )
{
	scanline = 255 - cpu_getiloops();

	memcpy(sprite_mux_buffer + scanline * spriteram_size,spriteram,spriteram_size);
	memcpy(sprite_mux_buffer_2 + scanline * spriteram_size,spriteram_2,spriteram_size);

	if (scanline == 255)
		nmi_line_pulse();
}
