/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"
#include "video/resnet.h"
#include "phoenix.h"

static UINT8 *videoram_pg[2];
static UINT8 videoram_pg_index;
static UINT8 palette_bank;
static UINT8 cocktail_mode;
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

static const res_net_decode_info phoenix_decode_info =
{
	2,		// there may be two proms needed to construct color
	0,		// start at 0
	255,	// end at 255
	//  R,   G,   B,   R,   G,   B
	{   0,   0,   0, 256, 256, 256},		// offsets
	{   0,   2,   1,  -1,   1,   0},		// shifts
	{0x01,0x01,0x01,0x02,0x02,0x02}		    // masks
};

static const res_net_info phoenix_net_info =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_5V | RES_NET_VIN_OPEN_COL,
	{
		{ RES_NET_AMP_NONE, 270, 270, 2, { 270, 1 } },
		{ RES_NET_AMP_NONE, 270, 270, 2, { 270, 1 } },
		{ RES_NET_AMP_NONE, 270, 270, 2, { 270, 1 } }
	}
};

static const res_net_info pleiades_net_info =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_5V | RES_NET_VIN_OPEN_COL,
	{
		{ RES_NET_AMP_NONE, 150, 270, 2, { 270, 1 } },
		{ RES_NET_AMP_NONE, 150, 270, 2, { 270, 1 } },
		{ RES_NET_AMP_NONE, 150, 270, 2, { 270, 1 } }
	}
};

static const res_net_info survival_net_info =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_5V | RES_NET_VIN_OPEN_COL,
	{
		{ RES_NET_AMP_NONE, 270, 270, 2, { 180, 1 } },
		{ RES_NET_AMP_NONE, 270, 270, 2, { 180, 1 } },
		{ RES_NET_AMP_NONE, 270, 270, 2, { 180, 1 } }
	}
};

PALETTE_INIT( phoenix )
{
	int i;
	rgb_t	*rgb;

	rgb = compute_res_net_all(color_prom, &phoenix_decode_info, &phoenix_net_info);
	/* native order */
	for (i=0;i<256;i++)
	{
		int col;
		col = ((i << 3 ) & 0x18) | ((i>>2) & 0x07) | (i & 0x60);
		palette_set_color(machine,i,rgb[col]);
	}
	palette_normalize_range(machine->palette, 0, 255, 0, 255);
	free(rgb);
}

PALETTE_INIT( survival )
{
	int i;
	rgb_t	*rgb;

	rgb = compute_res_net_all(color_prom, &phoenix_decode_info, &survival_net_info);
	/* native order */
	for (i=0;i<256;i++)
	{
		int col;
		col = ((i << 3 ) & 0x18) | ((i>>2) & 0x07) | (i & 0x60);
		palette_set_color(machine,i,rgb[col]);
	}
	palette_normalize_range(machine->palette, 0, 255, 0, 255);
	free(rgb);
}

PALETTE_INIT( pleiads )
{
	int i;
	rgb_t	*rgb;

	rgb = compute_res_net_all(color_prom, &phoenix_decode_info, &pleiades_net_info);
	/* native order */
	for (i=0;i<256;i++)
	{
		int col;
		col = ((i << 3 ) & 0x18) | ((i>>2) & 0x07) | (i & 0xE0);
		palette_set_color(machine,i,rgb[col]);
	}
	palette_normalize_range(machine->palette, 0, 255, 0, 255);
	free(rgb);
}

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_fg_tile_info )
{
	int code, col;

	code = videoram_pg[videoram_pg_index][tile_index];
	col = (code >> 5);
	col = col | 0x08 | (palette_bank << 4);
	SET_TILE_INFO(
			1,
			code,
			col,
			0);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int code, col;

	code = videoram_pg[videoram_pg_index][tile_index + 0x800];
	col = (code >> 5);
	col = col | 0x00 | (palette_bank << 4);
	SET_TILE_INFO(
			0,
			code,
			col,
			0);
}

/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( phoenix )
{
	videoram_pg[0] = auto_malloc(0x1000);
	videoram_pg[1] = auto_malloc(0x1000);

	memory_configure_bank(1, 0, 1, videoram_pg[0], 0);
	memory_configure_bank(1, 1, 1, videoram_pg[1], 0);
	memory_set_bank(1, 0);

    videoram_pg_index = 0;

	fg_tilemap = tilemap_create(get_fg_tile_info,tilemap_scan_rows,8,8,32,32);
	bg_tilemap = tilemap_create(get_bg_tile_info,tilemap_scan_rows,     8,8,32,32);

	tilemap_set_transparent_pen(fg_tilemap,0);

	tilemap_set_scrolldx(fg_tilemap, 0, (HTOTAL - HBSTART));
	tilemap_set_scrolldx(bg_tilemap, 0, (HTOTAL - HBSTART));
	tilemap_set_scrolldy(fg_tilemap, 0, (VTOTAL - VBSTART));
	tilemap_set_scrolldy(bg_tilemap, 0, (VTOTAL - VBSTART));

	state_save_register_global_pointer(videoram_pg[0], 0x1000);
	state_save_register_global_pointer(videoram_pg[1], 0x1000);
	state_save_register_global(videoram_pg_index);
	state_save_register_global(palette_bank);
	state_save_register_global(cocktail_mode);
}

/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( phoenix_videoram_w )
{
	UINT8 *rom = memory_region(REGION_CPU1);

	videoram_pg[videoram_pg_index][offset] = data;

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
    if (videoram_pg_index != (data & 1))
	{
		/* set memory bank */
		videoram_pg_index = data & 1;
		memory_set_bank(1, videoram_pg_index);

		cocktail_mode = videoram_pg_index && (input_port_read(machine, "CAB") & 0x01);

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
	if (videoram_pg_index != (data & 1))
	{
		/* set memory bank */
		videoram_pg_index = data & 1;
		memory_set_bank(1, videoram_pg_index);

		cocktail_mode = videoram_pg_index && (input_port_read(machine, "CAB") & 0x01);

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
	pleiads_sound_control_c_w(machine, offset, data);
}


WRITE8_HANDLER( phoenix_scroll_w )
{
	tilemap_set_scrollx(bg_tilemap,0,data);
}


CUSTOM_INPUT( player_input_r )
{
	if (cocktail_mode)
		return (input_port_read(field->port->machine, "CTRL") & 0xf0) >> 4;
	else
		return (input_port_read(field->port->machine, "CTRL") & 0x0f) >> 0;
}

CUSTOM_INPUT( pleiads_protection_r )
{
	/* handle Pleiads protection */
	switch (pleiads_protection_question)
	{
	case 0x00:
	case 0x20:
		/* Bit 3 is 0 */
		return 0;
		break;
	case 0x0c:
	case 0x30:
		/* Bit 3 is 1 */
		return 1;
		break;
	default:
		logerror("Unknown protection question %02X at %04X\n", pleiads_protection_question, activecpu_get_pc());
		return 0;
	}
}

READ8_HANDLER( survival_input_port_0_r )
{
	int ret = input_port_read(machine, "IN0");

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
