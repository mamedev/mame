/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"
#include "phoenix.h"

static UINT8 *videoram_pg1;
static UINT8 *videoram_pg2;
static UINT8 *current_videoram_pg;
static int current_videoram_pg_index;
static int palette_bank;
static int cocktail_mode;
static int pleiads_protection_question;
static int survival_protection_value;
static tilemap *fg_tilemap, *bg_tilemap;


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Phoenix has two 256x4 palette PROMs, one containing the high bits and the
  other the low bits (2x2x2 color space).
  The palette PROMs are connected to the RGB output this way:

  bit 3 --
        -- 270 ohm resistor  -- GREEN
        -- 270 ohm resistor  -- BLUE
  bit 0 -- 270 ohm resistor  -- RED

  bit 3 --
        -- GREEN
        -- BLUE
  bit 0 -- RED

  plus 270 ohm pullup and pulldown resistors on all lines

***************************************************************************/

PALETTE_INIT( phoenix )
{
	int i;
	#define COLOR(gfxn,offs) (colortable[machine->drv->gfxdecodeinfo[gfxn].color_codes_start + offs])


	for (i = 0;i < machine->drv->total_colors;i++)
	{
		int bit0,bit1,r,g,b;


		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[machine->drv->total_colors] >> 0) & 0x01;
		r = 0x55 * bit0 + 0xaa * bit1;
		bit0 = (color_prom[0] >> 2) & 0x01;
		bit1 = (color_prom[machine->drv->total_colors] >> 2) & 0x01;
		g = 0x55 * bit0 + 0xaa * bit1;
		bit0 = (color_prom[0] >> 1) & 0x01;
		bit1 = (color_prom[machine->drv->total_colors] >> 1) & 0x01;
		b = 0x55 * bit0 + 0xaa * bit1;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
		color_prom++;
	}

	/* first bank of characters use colors 0x00-0x1f and 0x40-0x5f */
	/* second bank of characters use colors 0x20-0x3f and 0x60-0x7f */
	for (i = 0;i < 0x40;i++)
	{
		int col;


		col = ((i & 0x1c) >> 2) | ((i & 0x03) << 3) | ((i & 0x20) << 1);

		COLOR(0,i) = col;
		COLOR(1,i) = col | 0x20;
	}
}

PALETTE_INIT( pleiads )
{
	int i;
	#define COLOR(gfxn,offs) (colortable[machine->drv->gfxdecodeinfo[gfxn].color_codes_start + offs])


	for (i = 0;i < machine->drv->total_colors;i++)
	{
		int bit0,bit1,r,g,b;


		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[machine->drv->total_colors] >> 0) & 0x01;
		r = 0x55 * bit0 + 0xaa * bit1;
		bit0 = (color_prom[0] >> 2) & 0x01;
		bit1 = (color_prom[machine->drv->total_colors] >> 2) & 0x01;
		g = 0x55 * bit0 + 0xaa * bit1;
		bit0 = (color_prom[0] >> 1) & 0x01;
		bit1 = (color_prom[machine->drv->total_colors] >> 1) & 0x01;
		b = 0x55 * bit0 + 0xaa * bit1;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
		color_prom++;
	}

	/* first bank of characters use colors 0x00-0x1f, 0x40-0x5f, 0x80-0x9f and 0xc0-0xdf */
	/* second bank of characters use colors 0x20-0x3f, 0x60-0x7f, 0xa0-0xbf and 0xe0-0xff */
	for (i = 0;i < 0x80;i++)
	{
		int col;


		col = ((i & 0x1c) >> 2) | ((i & 0x03) << 3) | ((i & 0x60) << 1);

		COLOR(0,i) = col;
		COLOR(1,i) = col | 0x20;
	}
}

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_fg_tile_info )
{
	int code;

	code = current_videoram_pg[tile_index];
	SET_TILE_INFO(
			1,
			code,
			(code >> 5) | (palette_bank << 3),
			(tile_index & 0x1f) ? 0 : TILE_FORCE_LAYER0);	/* first row (column) is opaque */
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int code;

	code = current_videoram_pg[tile_index + 0x800];
	SET_TILE_INFO(
			0,
			code,
			(code >> 5) | (palette_bank << 3),
			0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( phoenix )
{
	videoram_pg1 = auto_malloc(0x1000);
	videoram_pg2 = auto_malloc(0x1000);

	memory_configure_bank(1, 0, 1, videoram_pg1, 0);
	memory_configure_bank(1, 1, 1, videoram_pg2, 0);
	memory_set_bank(1, 0);

    current_videoram_pg_index = -1;
	current_videoram_pg = videoram_pg1;		/* otherwise, hiscore loading crashes */


	fg_tilemap = tilemap_create(get_fg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,8,8,32,32);
	bg_tilemap = tilemap_create(get_bg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,     8,8,32,32);

	tilemap_set_transparent_pen(fg_tilemap,0);
	tilemap_set_scrolldx(fg_tilemap,0,8);
	tilemap_set_scrolldy(fg_tilemap,0,48);
	tilemap_set_scrolldx(bg_tilemap,0,8);
	tilemap_set_scrolldy(bg_tilemap,0,48);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( phoenix_videoram_w )
{
	UINT8 *rom = memory_region(REGION_CPU1);

	current_videoram_pg[offset] = data;

	if ((offset & 0x7ff) < 0x340)
	{
		if (offset & 0x800)
			tilemap_mark_tile_dirty(bg_tilemap,offset & 0x3ff);
		else
			tilemap_mark_tile_dirty(fg_tilemap,offset & 0x3ff);
	}

	/* as part of the protecion, Survival executes code from $43a4 */
	rom[offset + 0x4000] = data;
}


WRITE8_HANDLER( phoenix_videoreg_w )
{
    if (current_videoram_pg_index != (data & 1))
	{
		/* set memory bank */
		current_videoram_pg_index = data & 1;
		memory_set_bank(1, current_videoram_pg_index);
		current_videoram_pg = current_videoram_pg_index ? videoram_pg2 : videoram_pg1;

		cocktail_mode = current_videoram_pg_index && (input_port_3_r(0) & 0x01);

		tilemap_set_flip(ALL_TILEMAPS, cocktail_mode ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0);
		tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
	}

	/* Phoenix has only one palette select effecting both layers */
	if (palette_bank != ((data >> 1) & 1))
	{
		palette_bank = (data >> 1) & 1;

		tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
	}
}

WRITE8_HANDLER( pleiads_videoreg_w )
{
    if (current_videoram_pg_index != (data & 1))
	{
		/* set memory bank */
		current_videoram_pg_index = data & 1;
		current_videoram_pg = current_videoram_pg_index ? videoram_pg2 : videoram_pg1;

		cocktail_mode = current_videoram_pg_index && (input_port_3_r(0) & 0x01);

		tilemap_set_flip(ALL_TILEMAPS, cocktail_mode ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0);
		tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
	}


	/* the palette table is at $0420-$042f and is set by $06bc.
       Four palette changes by level.  The palette selection is
       wrong, but the same paletter is used for both layers. */

    if (palette_bank != ((data >> 1) & 3))
	{
		palette_bank = ((data >> 1) & 3);

		tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);

		logerror("Palette: %02X\n", (data & 0x06) >> 1);
	}

	pleiads_protection_question = data & 0xfc;

	/* send two bits to sound control C (not sure if they are there) */
	pleiads_sound_control_c_w(offset, data);
}


WRITE8_HANDLER( phoenix_scroll_w )
{
	tilemap_set_scrollx(bg_tilemap,0,data);
}


READ8_HANDLER( phoenix_input_port_0_r )
{
	if (cocktail_mode)
		return (input_port_0_r(0) & 0x07) | (input_port_1_r(0) & 0xf8);
	else
		return input_port_0_r(0);
}

READ8_HANDLER( pleiads_input_port_0_r )
{
	int ret = phoenix_input_port_0_r(0) & 0xf7;

	/* handle Pleiads protection */
	switch (pleiads_protection_question)
	{
	case 0x00:
	case 0x20:
		/* Bit 3 is 0 */
		break;
	case 0x0c:
	case 0x30:
		/* Bit 3 is 1 */
		ret	|= 0x08;
		break;
	default:
		logerror("Unknown protection question %02X at %04X\n", pleiads_protection_question, activecpu_get_pc());
	}

	return ret;
}

READ8_HANDLER( survival_input_port_0_r )
{
	int ret = phoenix_input_port_0_r(0);

	if (survival_protection_value)
	{
		ret ^= 0xf0;
	}

	return ret;
}

READ8_HANDLER( survival_protection_r )
{
	if (activecpu_get_pc() == 0x2017)
	{
		survival_protection_value ^= 1;
	}

	return survival_protection_value;
}

/***************************************************************************

  Display refresh

***************************************************************************/

VIDEO_UPDATE( phoenix )
{
	tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,fg_tilemap,0,0);
	return 0;
}
