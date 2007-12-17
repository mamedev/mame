/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"

// Real stuff
UINT8 *stfight_text_char_ram;
UINT8 *stfight_text_attr_ram;
UINT8 *stfight_vh_latch_ram;
UINT8 *stfight_sprite_ram;

static tilemap *fg_tilemap,*bg_tilemap,*tx_tilemap;
static int stfight_sprite_base = 0;

/*
        Graphics ROM Format
        ===================

        Each tile is 8x8 pixels
        Each composite tile is 2x2 tiles, 16x16 pixels
        Each screen is 32x32 composite tiles, 64x64 tiles, 256x256 pixels
        Each layer is a 4-plane bitmap 8x16 screens, 2048x4096 pixels

        There are 4x256=1024 composite tiles defined for each layer

        Each layer is mapped using 2 bytes/composite tile
        - one byte for the tile
        - one byte for the tile bank, attribute
            - b7,b5     tile bank (0-3)

        Each pixel is 4 bits = 16 colours.

 */

PALETTE_INIT( stfight )
{
	int i;
	#define TOTAL_COLORS(gfxn) (machine->gfx[gfxn]->total_colors * machine->gfx[gfxn]->color_granularity)
	#define COLOR(gfxn,offs) (colortable[machine->drv->gfxdecodeinfo[gfxn].color_codes_start + offs])


	/* unique color for transparency */
	palette_set_color(machine,256,MAKE_RGB(0x04,0x04,0x04));

	/* text uses colors 192-207 */
	for (i = 0;i < TOTAL_COLORS(0);i++)
	{
		if ((*color_prom & 0x0f) == 0x0f) COLOR(0,i) = 256;	/* transparent */
		else COLOR(0,i) = (*color_prom & 0x0f) + 0xc0;
		color_prom++;
	}
	color_prom += 256 - TOTAL_COLORS(0);	/* rest of the PROM is unused */

	/* fg uses colors 64-127 */
	for (i = 0;i < TOTAL_COLORS(1);i++)
	{
		COLOR(1,i) = (color_prom[256] & 0x0f) + 16 * (color_prom[0] & 0x03) + 0x40;
		color_prom++;
	}
	color_prom += 256;

	/* bg uses colors 0-63 */
	for (i = 0;i < TOTAL_COLORS(2);i++)
	{
		COLOR(2,i) = (color_prom[256] & 0x0f) + 16 * (color_prom[0] & 0x03) + 0x00;
		color_prom++;
	}
	color_prom += 256;

	/* sprites use colors 128-191 */
	for (i = 0;i < TOTAL_COLORS(4);i++)
	{
		COLOR(4,i) = (color_prom[256] & 0x0f) + 16 * (color_prom[0] & 0x03) + 0x80;
		color_prom++;
	}
	color_prom += 256;
}

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILEMAP_MAPPER( fg_scan )
{
	/* logical (col,row) -> memory offset */
	return (col & 0x0f) + ((row & 0x0f) << 4) + ((col & 0x70) << 4) + ((row & 0xf0) << 7);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	UINT8   *fgMap = memory_region(REGION_GFX5);
	int attr,tile_base;

	attr = fgMap[0x8000+tile_index];
	tile_base = ((attr & 0x80) << 2) | ((attr & 0x20) << 3);

	SET_TILE_INFO(
			1,
			tile_base + fgMap[tile_index],
			attr & 0x07,
			0);
}

static TILEMAP_MAPPER( bg_scan )
{
	/* logical (col,row) -> memory offset */
	return ((col & 0x0e) >> 1) + ((row & 0x0f) << 3) + ((col & 0x70) << 3) +
			((row & 0x80) << 3) + ((row & 0x10) << 7) + ((col & 0x01) << 12) +
			((row & 0x60) << 8);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	UINT8   *bgMap = memory_region(REGION_GFX6);
	int attr,tile_bank,tile_base;

	attr = bgMap[0x8000+tile_index];
	tile_bank = (attr & 0x20) >> 5;
	tile_base = (attr & 0x80) << 1;

	SET_TILE_INFO(
			2+tile_bank,
			tile_base + bgMap[tile_index],
			attr & 0x07,
			0);
}

static TILE_GET_INFO( get_tx_tile_info )
{
	UINT8 attr = stfight_text_attr_ram[tile_index];

	SET_TILE_INFO(
			0,
			stfight_text_char_ram[tile_index] + ((attr & 0x80) << 1),
			attr & 0x0f,
			TILE_FLIPYX((attr & 0x60) >> 5));
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( stfight )
{
	bg_tilemap = tilemap_create(get_bg_tile_info,bg_scan,TILEMAP_TYPE_PEN,     16,16,128,256);
	fg_tilemap = tilemap_create(get_fg_tile_info,fg_scan,TILEMAP_TYPE_PEN,16,16,128,256);
	tx_tilemap = tilemap_create(get_tx_tile_info,tilemap_scan_rows,
			TILEMAP_TYPE_COLORTABLE,8,8,32,32);

	tilemap_set_transparent_pen(fg_tilemap,0x0F);
	tilemap_set_transparent_pen(tx_tilemap,256);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( stfight_text_char_w )
{
	stfight_text_char_ram[offset] = data;
	tilemap_mark_tile_dirty(tx_tilemap,offset);
}

WRITE8_HANDLER( stfight_text_attr_w )
{
	stfight_text_attr_ram[offset] = data;
	tilemap_mark_tile_dirty(tx_tilemap,offset);
}

WRITE8_HANDLER( stfight_sprite_bank_w )
{
	stfight_sprite_base = ( ( data & 0x04 ) << 7 ) |
				          ( ( data & 0x01 ) << 8 );
}

WRITE8_HANDLER( stfight_vh_latch_w )
{
	int scroll;


	stfight_vh_latch_ram[offset] = data;

	switch( offset )
	{
		case 0x00:
		case 0x01:
			scroll = (stfight_vh_latch_ram[1] << 8) | stfight_vh_latch_ram[0];
			tilemap_set_scrollx(fg_tilemap,0,scroll);
			break;

		case 0x02:
		case 0x03:
			scroll = (stfight_vh_latch_ram[3] << 8) | stfight_vh_latch_ram[2];
			tilemap_set_scrolly(fg_tilemap,0,scroll);
			break;

		case 0x04:
		case 0x05:
			scroll = (stfight_vh_latch_ram[5] << 8) | stfight_vh_latch_ram[4];
			tilemap_set_scrollx(bg_tilemap,0,scroll);
			break;

		case 0x06:
		case 0x08:
			scroll = (stfight_vh_latch_ram[8] << 8) | stfight_vh_latch_ram[6];
			tilemap_set_scrolly(bg_tilemap,0,scroll);
			break;

		case 0x07:
			tilemap_set_enable(tx_tilemap,data & 0x80);
			/* 0x40 = sprites */
			tilemap_set_enable(bg_tilemap,data & 0x20);
			tilemap_set_enable(fg_tilemap,data & 0x10);
			flip_screen_set(data & 0x01);
			break;
	}
}

/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs,sx,sy;

	for (offs = 0;offs < 4096;offs += 32)
	{
		int code;
		int attr = stfight_sprite_ram[offs+1];
		int flipx = attr & 0x10;
		int color = attr & 0x0f;
		int pri = (attr & 0x20) >> 5;

		sy = stfight_sprite_ram[offs+2];
		sx = stfight_sprite_ram[offs+3];

		// non-active sprites have zero y coordinate value
		if( sy > 0 )
		{
			// sprites which wrap onto/off the screen have
			// a sign extension bit in the sprite attribute
			if( sx >= 0xf0 )
			{
				if (attr & 0x80)
				    sx -= 0x100;
			}

			if (flip_screen)
			{
				sx = 240 - sx;
				sy = 240 - sy;
				flipx = !flipx;
			}

			code = stfight_sprite_base + stfight_sprite_ram[offs];

			pdrawgfx(bitmap,machine->gfx[4],
				     code,
					 color,
					 flipx,flip_screen,
					 sx,sy,
				     cliprect,TRANSPARENCY_PEN,0x0f,
					 pri ? 0x02 : 0);
		}
	}
}


VIDEO_UPDATE( stfight )
{
	fillbitmap(priority_bitmap,0,cliprect);

	fillbitmap(bitmap,machine->pens[0],cliprect);	/* in case bg_tilemap is disabled */
    tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,fg_tilemap,0,1);

	/* Draw sprites (may be obscured by foreground layer) */
	if (stfight_vh_latch_ram[0x07] & 0x40)
		draw_sprites(machine, bitmap,cliprect);

	tilemap_draw(bitmap,cliprect,tx_tilemap,0,0);
	return 0;
}
