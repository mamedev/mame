/***************************************************************************
  Great Swordsman

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"

size_t gsword_spritexy_size;

//UINT8 *gsword_scrolly_ram;
UINT8 *gsword_spritexy_ram;
UINT8 *gsword_spritetile_ram;
UINT8 *gsword_spriteattrib_ram;

static int charbank, charpalbank, flipscreen;

static tilemap *bg_tilemap;

PALETTE_INIT( josvolly )
{
	int i;

	#define TOTAL_COLORS(gfxn) (machine->gfx[gfxn]->total_colors * machine->gfx[gfxn]->color_granularity)
	#define COLOR(gfxn,offs) (colortable[machine->drv->gfxdecodeinfo[gfxn].color_codes_start + offs])

	for (i = 0;i < machine->drv->total_colors;i++)
	{
		int bit0,bit1,bit2,bit3,r,g,b;


		/* red component */
		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		bit3 = (color_prom[0] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		/* green component */
		bit0 = (color_prom[machine->drv->total_colors] >> 0) & 0x01;
		bit1 = (color_prom[machine->drv->total_colors] >> 1) & 0x01;
		bit2 = (color_prom[machine->drv->total_colors] >> 2) & 0x01;
		bit3 = (color_prom[machine->drv->total_colors] >> 3) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		/* blue component */
		bit0 = (color_prom[2*machine->drv->total_colors] >> 0) & 0x01;
		bit1 = (color_prom[2*machine->drv->total_colors] >> 1) & 0x01;
		bit2 = (color_prom[2*machine->drv->total_colors] >> 2) & 0x01;
		bit3 = (color_prom[2*machine->drv->total_colors] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
		color_prom++;
	}

	color_prom += 2*machine->drv->total_colors;
	/* color_prom now points to the beginning of the sprite lookup table */

	/* characters */
	for (i = 0;i < TOTAL_COLORS(0);i++)
		COLOR(0,i) = i;

	/* sprites */
	for (i = 0;i < TOTAL_COLORS(1);i++)
		COLOR(1,i) = 0x80 + (BITSWAP8( color_prom[i], 7,6,5,4, 0,1,2,3) & 0xf);
}

PALETTE_INIT( gsword )
{
	int i;

	#define TOTAL_COLORS(gfxn) (machine->gfx[gfxn]->total_colors * machine->gfx[gfxn]->color_granularity)
	#define COLOR(gfxn,offs) (colortable[machine->drv->gfxdecodeinfo[gfxn].color_codes_start + offs])

	for (i = 0;i < 256;i++)
	{
		int bit0,bit1,bit2,r,g,b;

		/* red component */
		bit0 = (color_prom[256+i] >> 0) & 1;
		bit1 = (color_prom[256+i] >> 1) & 1;
		bit2 = (color_prom[256+i] >> 2) & 1;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (color_prom[256+i] >> 3) & 1;
		bit1 = (color_prom[i] >> 0) & 1;
		bit2 = (color_prom[i] >> 1) & 1;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = (color_prom[i] >> 2) & 1;
		bit2 = (color_prom[i] >> 3) & 1;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}

	color_prom += 2*256;
	/* color_prom now points to the beginning of the sprite lookup table */

	/* characters */
	for (i = 0;i < TOTAL_COLORS(0);i++)
		COLOR(0,i) = i;

	/* sprites */
	for (i = 0;i < TOTAL_COLORS(1);i++)
		COLOR(1,i) = 0x80 + (BITSWAP8( color_prom[i], 7,6,5,4, 0,1,2,3) & 0xf);
}

WRITE8_HANDLER( gsword_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( gsword_charbank_w )
{
	if (charbank != data)
	{
		charbank = data;
		tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
	}
}

WRITE8_HANDLER( gsword_videoctrl_w )
{
	if (data & 0x8f)
	{
		popmessage("videoctrl %02x",data);
	}

	/* bits 5-6 are char palette bank */

	if (charpalbank != ((data & 0x60) >> 5))
	{
		charpalbank = (data & 0x60) >> 5;
		tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
	}

	/* bit 4 is flip screen */

	if (flipscreen != (data & 0x10))
	{
		flipscreen = data & 0x10;
	    tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
	}

	/* bit 0 could be used but unknown */

	/* other bits unused */
}

WRITE8_HANDLER( gsword_scroll_w )
{
	tilemap_set_scrolly(bg_tilemap, 0, data);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int code = videoram[tile_index] + ((charbank & 0x03) << 8);
	int color = ((code & 0x3c0) >> 6) + 16 * charpalbank;
	int flags = flipscreen ? (TILE_FLIPX | TILE_FLIPY) : 0;

	SET_TILE_INFO(0, code, color, flags);
}

VIDEO_START( gsword )
{
	bg_tilemap = tilemap_create(get_bg_tile_info, tilemap_scan_rows,
		 8, 8, 32, 64);
}

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs;

	for (offs = 0; offs < gsword_spritexy_size - 1; offs+=2)
	{
		int sx,sy,flipx,flipy,spritebank,tile;

		if (gsword_spritexy_ram[offs]!=0xf1)
		{
			spritebank = 0;
			tile = gsword_spritetile_ram[offs];
			sy = 241-gsword_spritexy_ram[offs];
			sx = gsword_spritexy_ram[offs+1]-56;
			flipx = gsword_spriteattrib_ram[offs] & 0x02;
			flipy = gsword_spriteattrib_ram[offs] & 0x01;

			// Adjust sprites that should be far far right!
			if (sx<0) sx+=256;

			// Adjuste for 32x32 tiles(#128-256)
			if (tile > 127)
			{
				spritebank = 1;
				tile -= 128;
				sy-=16;
			}
			if (flipscreen)
			{
				flipx = !flipx;
				flipy = !flipy;
			}
			drawgfx(bitmap,machine->gfx[1+spritebank],
					tile,
					gsword_spritetile_ram[offs+1] & 0x3f,
					flipx,flipy,
					sx,sy,
					cliprect,TRANSPARENCY_COLOR, 0x8f);
		}
	}
}

VIDEO_UPDATE( gsword )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	draw_sprites(machine, bitmap, cliprect);
	return 0;
}
