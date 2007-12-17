/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

d800-dbff   foreground:         char low bits (1 screen * 1024 chars/screen)
dc00-dfff   foreground attribute:   7       ?
                         6543       color
                             21 ?
                               0    char hi bit


e000-e0ff   spriteram:  64 sprites (4 bytes/sprite)
        offset :    0       1       2       3
        meaning:    ypos(lo)    sprite(lo)  attribute   xpos(lo)
                                7   flipy
                                6   flipx
                                5-2 color
                                1   sprite(hi)
                                0   xpos(hi)


background: 0x4000 bytes of ROM:    76543210    tile code low bits
        0x4000 bytes of ROM:    7       ?
                         6543       color
                             2  ?
                              10    tile code high bits

***************************************************************************/

#include "driver.h"

static UINT8 scrollx[2], scrolly[2];

/* Layers has only bits 5-6 active.
   6 selects background off/on
   5 controls sprite priority (active only on title screen,
     not for scores or push start nor game)
*/

static UINT8 flipscreen;
static UINT8 write_layers, layers;
static UINT8 ninjemak_dispdisable;

static tilemap *bg_tilemap, *tx_tilemap;

static const UINT8 *spritepalettebank;

/* Notes:
     write_layers and layers are used in galivan/dangar but not ninjemak
     ninjemak_dispdisable is used in ninjemak but not galivan/dangar
     spritepalettebank is set at palette init and doesn't need to be saved
*/



/***************************************************************************

  Convert the color PROMs into a more useable format.

***************************************************************************/

PALETTE_INIT( galivan )
{
	int i;
	#define TOTAL_COLORS(gfxn) (machine->gfx[gfxn]->total_colors * machine->gfx[gfxn]->color_granularity)
	#define COLOR(gfxn,offs) (colortable[machine->drv->gfxdecodeinfo[gfxn].color_codes_start + offs])

	for (i = 0;i < machine->drv->total_colors;i++)
	{
		int bit0,bit1,bit2,bit3,r,g,b;

		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		bit3 = (color_prom[0] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[machine->drv->total_colors] >> 0) & 0x01;
		bit1 = (color_prom[machine->drv->total_colors] >> 1) & 0x01;
		bit2 = (color_prom[machine->drv->total_colors] >> 2) & 0x01;
		bit3 = (color_prom[machine->drv->total_colors] >> 3) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[2*machine->drv->total_colors] >> 0) & 0x01;
		bit1 = (color_prom[2*machine->drv->total_colors] >> 1) & 0x01;
		bit2 = (color_prom[2*machine->drv->total_colors] >> 2) & 0x01;
		bit3 = (color_prom[2*machine->drv->total_colors] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
		color_prom++;
	}

	color_prom += 2*machine->drv->total_colors;
	/* color_prom now points to the beginning of the lookup tables */


	/* characters use colors 0-127 */
	for (i = 0;i < TOTAL_COLORS(0);i++)
		COLOR(0,i) = i;

	/* I think that */
	/* background tiles use colors 192-255 in four banks */
	/* the bottom two bits of the color code select the palette bank for */
	/* pens 0-7; the top two bits for pens 8-15. */
	for (i = 0;i < TOTAL_COLORS(1);i++)
	{
		if (i & 8) COLOR(1,i) = 192 + (i & 0x0f) + ((i & 0xc0) >> 2);
		else COLOR(1,i) = 192 + (i & 0x0f) + ((i & 0x30) >> 0);
	}


	/* sprites use colors 128-191 in four banks */
	/* The lookup table tells which colors to pick from the selected bank */
	/* the bank is selected by another PROM and depends on the top 7 bits of */
	/* the sprite code. The PROM selects the bank *separately* for pens 0-7 and */
	/* 8-15 (like for tiles). */
	for (i = 0;i < TOTAL_COLORS(2)/16;i++)
	{
		int j;

		for (j = 0;j < 16;j++)
		{
			if (i & 8)
				COLOR(2,i + j * (TOTAL_COLORS(2)/16)) = 128 + ((j & 0x0c) << 2) + (*color_prom & 0x0f);
			else
				COLOR(2,i + j * (TOTAL_COLORS(2)/16)) = 128 + ((j & 0x03) << 4) + (*color_prom & 0x0f);
		}

		color_prom++;
	}

	/* color_prom now points to the beginning of the sprite palette bank table */
	spritepalettebank = color_prom;	/* we'll need it at run time */
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_bg_tile_info )
{
	UINT8 *BGROM = memory_region(REGION_GFX4);
	int attr = BGROM[tile_index + 0x4000];
	int code = BGROM[tile_index] | ((attr & 0x03) << 8);
	SET_TILE_INFO(
			1,
			code,
			(attr & 0x78) >> 3,		/* seems correct */
			0);
}

static TILE_GET_INFO( get_tx_tile_info )
{
	int attr = colorram[tile_index];
	int code = videoram[tile_index] | ((attr & 0x01) << 8);
	SET_TILE_INFO(
			0,
			code,
			(attr & 0xe0) >> 5,		/* not sure */
			0);
	tileinfo->category = attr & 8 ? 0 : 1;	/* seems correct */
}

static TILE_GET_INFO( ninjemak_get_bg_tile_info )
{
	UINT8 *BGROM = memory_region(REGION_GFX4);
	int attr = BGROM[tile_index + 0x4000];
	int code = BGROM[tile_index] | ((attr & 0x03) << 8);
	SET_TILE_INFO(
			1,
			code,
			((attr & 0x60) >> 3) | ((attr & 0x0c) >> 2),	/* seems correct */
			0);
}

static TILE_GET_INFO( ninjemak_get_tx_tile_info )
{
	int attr = colorram[tile_index];
	int code = videoram[tile_index] | ((attr & 0x03) << 8);
	SET_TILE_INFO(
			0,
			code,
			(attr & 0x1c) >> 2,		/* seems correct ? */
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( galivan )
{
	/* configure ROM banking */
	UINT8 *rombase = memory_region(REGION_CPU1);
	memory_configure_bank(1, 0, 2, &rombase[0x10000], 0x2000);

	bg_tilemap = tilemap_create(get_bg_tile_info,tilemap_scan_rows,TILEMAP_TYPE_PEN,   16,16,128,128);
	tx_tilemap = tilemap_create(get_tx_tile_info,tilemap_scan_cols,TILEMAP_TYPE_PEN,8,8,32,32);

	tilemap_set_transparent_pen(tx_tilemap,15);

	/* register for saving */
	state_save_register_global_array(scrollx);
	state_save_register_global_array(scrolly);
	state_save_register_global(flipscreen);
	state_save_register_global(write_layers);
	state_save_register_global(layers);
}

VIDEO_START( ninjemak )
{
	/* configure ROM banking */
	UINT8 *rombase = memory_region(REGION_CPU1);
	memory_configure_bank(1, 0, 4, &rombase[0x10000], 0x2000);

	bg_tilemap = tilemap_create(ninjemak_get_bg_tile_info,tilemap_scan_cols,TILEMAP_TYPE_PEN,   16,16,512,32);
	tx_tilemap = tilemap_create(ninjemak_get_tx_tile_info,tilemap_scan_cols,TILEMAP_TYPE_PEN,8,8,32,32);

	tilemap_set_transparent_pen(tx_tilemap,15);

	/* register for saving */
	state_save_register_global_array(scrollx);
	state_save_register_global_array(scrolly);
	state_save_register_global(flipscreen);
	state_save_register_global(ninjemak_dispdisable);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( galivan_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(tx_tilemap,offset);
}

WRITE8_HANDLER( galivan_colorram_w )
{
	colorram[offset] = data;
	tilemap_mark_tile_dirty(tx_tilemap,offset);
}

/* Written through port 40 */
WRITE8_HANDLER( galivan_gfxbank_w )
{
	/* bits 0 and 1 coin counters */
	coin_counter_w(0,data & 1);
	coin_counter_w(1,data & 2);

	/* bit 2 flip screen */
	flipscreen = data & 0x04;
	tilemap_set_flip (bg_tilemap, flipscreen ? TILEMAP_FLIPX|TILEMAP_FLIPY : 0);
	tilemap_set_flip (tx_tilemap, flipscreen ? TILEMAP_FLIPX|TILEMAP_FLIPY : 0);

	/* bit 7 selects one of two ROM banks for c000-dfff */
	memory_set_bank(1, (data & 0x80) >> 7);

/*  logerror("Address: %04X - port 40 = %02x\n",activecpu_get_pc(),data); */
}

WRITE8_HANDLER( ninjemak_gfxbank_w )
{
	/* bits 0 and 1 coin counters */
	coin_counter_w(0,data & 1);
	coin_counter_w(1,data & 2);

	/* bit 2 flip screen */
	flipscreen = data & 0x04;
	tilemap_set_flip (bg_tilemap, flipscreen ? TILEMAP_FLIPX|TILEMAP_FLIPY : 0);
	tilemap_set_flip (tx_tilemap, flipscreen ? TILEMAP_FLIPX|TILEMAP_FLIPY : 0);

	/* bit 3 text bank flag ??? */
	if (data & 0x08)
	{
		/* This is a temporary condition specification. */

		int offs;

logerror("%04x: write %02x to port 80\n",activecpu_get_pc(),data);

		for (offs = 0; offs < videoram_size; offs++)
		{
			galivan_videoram_w(offs, 0x20);
		}
		for (offs = 0; offs < videoram_size; offs++)
		{
			galivan_colorram_w(offs, 0x03);
		}
	}

	/* bit 4 background disable flag */
	ninjemak_dispdisable = data & 0x10;

	/* bit 5 sprite flag ??? */

	/* bit 6, 7 ROM bank select */
	memory_set_bank(1, (data & 0xc0) >> 6);

#if 0
	{
		char mess[80];
		int btz[8];
		int offs;

		for (offs = 0; offs < 8; offs++) btz[offs] = (((data >> offs) & 0x01) ? 1 : 0);

		sprintf(mess, "BK:%01X%01X S:%01X B:%01X T:%01X FF:%01X C2:%01X C1:%01X", btz[7], btz[6], btz[5], btz[4], btz[3], btz[2], btz[1], btz[0]);
		popmessage(mess);
	}
#endif
}



/* Written through port 41-42 */
WRITE8_HANDLER( galivan_scrollx_w )
{
	if (offset == 1) {
		if (data & 0x80)
			write_layers = 1;
		else if (write_layers) {
			layers = data & 0x60;
			write_layers = 0;
		}
	}
	scrollx[offset] = data;
}

/* Written through port 43-44 */
WRITE8_HANDLER( galivan_scrolly_w )
{
	scrolly[offset] = data;
}


WRITE8_HANDLER( ninjemak_scrollx_w )
{
	scrollx[offset] = data;
}

WRITE8_HANDLER( ninjemak_scrolly_w )
{
	scrolly[offset] = data;
}



/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs;

	/* draw the sprites */
	for (offs = 0;offs < spriteram_size;offs += 4)
	{
		int code;
		int attr = spriteram[offs+2];
		int color = (attr & 0x3c) >> 2;
		int flipx = attr & 0x40;
		int flipy = attr & 0x80;
		int sx,sy;

		sx = (spriteram[offs+3] - 0x80) + 256 * (attr & 0x01);
		sy = 240 - spriteram[offs];
		if (flipscreen)
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

//      code = spriteram[offs+1] + ((attr & 0x02) << 7);
		code = spriteram[offs+1] + ((attr & 0x06) << 7);	// for ninjemak, not sure ?

		drawgfx(bitmap,machine->gfx[2],
				code,
				color + 16 * (spritepalettebank[code >> 2] & 0x0f),
				flipx,flipy,
				sx,sy,
				cliprect,TRANSPARENCY_PEN,15);
	}
}


VIDEO_UPDATE( galivan )
{
	tilemap_set_scrollx(bg_tilemap,0,scrollx[0] + 256 * (scrollx[1] & 0x07));
	tilemap_set_scrolly(bg_tilemap,0,scrolly[0] + 256 * (scrolly[1] & 0x07));

	if (layers & 0x40)
		fillbitmap(bitmap,machine->pens[0],cliprect);
	else
		tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);

	if (layers & 0x20) {
		tilemap_draw(bitmap,cliprect,tx_tilemap,0,0);
		tilemap_draw(bitmap,cliprect,tx_tilemap,1,0);
		draw_sprites(machine,bitmap,cliprect);
	} else {
		draw_sprites(machine,bitmap,cliprect);
		tilemap_draw(bitmap,cliprect,tx_tilemap,0,0);
		tilemap_draw(bitmap,cliprect,tx_tilemap,1,0);
	}

	return 0;
}

VIDEO_UPDATE( ninjemak )
{
	/* (scrollx[1] & 0x40) does something */
	tilemap_set_scrollx(bg_tilemap,0,scrollx[0] + 256 * (scrollx[1] & 0x1f));
	tilemap_set_scrolly(bg_tilemap,0,scrolly[0] + 256 * (scrolly[1] & 0xff));

	if (ninjemak_dispdisable)
		fillbitmap(bitmap,machine->pens[0],cliprect);
	else
		tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);

	draw_sprites(machine,bitmap,cliprect);

	tilemap_draw(bitmap,cliprect,tx_tilemap,0,0);
	return 0;
}
