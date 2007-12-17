/*
 *  Chack'n Pop (C) 1983 TAITO Corp.
 *  emulate video hardware
 */

#include "driver.h"

#define GFX_FLIP_X	0x01
#define GFX_FLIP_Y	0x02
#define GFX_VRAM_BANK	0x04
#define GFX_UNKNOWN1	0x08
#define GFX_TX_BANK1	0x20
#define GFX_UNKNOWN2	0x40
#define GFX_TX_BANK2	0x80

#define TX_COLOR1	0x0b
#define TX_COLOR2	0x01

UINT8 *chaknpop_txram;
UINT8 *chaknpop_sprram;
size_t chaknpop_sprram_size;
UINT8 *chaknpop_attrram;

static UINT8 *vram1;
static UINT8 *vram2;
static UINT8 *vram3;
static UINT8 *vram4;

static tilemap *tx_tilemap;

static UINT8 gfxmode;
static UINT8 flip_x, flip_y;


/***************************************************************************
  palette decode
***************************************************************************/

PALETTE_INIT( chaknpop )
{
	int i;

	for (i = 0; i < 1024; i++)
	{
		int col, r, g, b;
		int bit0, bit1, bit2;

		col = (color_prom[i]&0x0f)+((color_prom[i+1024]&0x0f)<<4);

		/* red component */
		bit0 = (col >> 0) & 0x01;
		bit1 = (col >> 1) & 0x01;
		bit2 = (col >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = (col >> 3) & 0x01;
		bit1 = (col >> 4) & 0x01;
		bit2 = (col >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = 0;
		bit1 = (col >> 6) & 0x01;
		bit2 = (col >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}
}

/***************************************************************************
  Memory handlers
***************************************************************************/

static void set_vram_bank(void)
{
	UINT8 *RAM = memory_region(REGION_CPU1);
	int bankaddress;

	if (gfxmode & GFX_VRAM_BANK)
		bankaddress = 0x14000;
	else
		bankaddress = 0x10000;

	memory_set_bankptr(STATIC_BANK1, &RAM[bankaddress]);	 /* Select 2 banks of 16k */
}


static void tx_tilemap_mark_all_dirty(void)
{
	tilemap_mark_all_tiles_dirty(tx_tilemap);
	tilemap_set_flip(tx_tilemap, flip_x | flip_y);
}

READ8_HANDLER( chaknpop_gfxmode_r )
{
	return gfxmode;
}

WRITE8_HANDLER( chaknpop_gfxmode_w )
{
	if (gfxmode != data)
	{
		int all_dirty = 0;

		gfxmode = data;
		set_vram_bank();

		if (flip_x != (gfxmode & GFX_FLIP_X))
		{
			flip_x = gfxmode & GFX_FLIP_X;
			all_dirty = 1;
		}

		if (flip_y != (gfxmode & GFX_FLIP_Y))
		{
			flip_y = gfxmode & GFX_FLIP_Y;
			all_dirty = 1;
		}

		if (all_dirty)
			tx_tilemap_mark_all_dirty();
	}
}

WRITE8_HANDLER( chaknpop_txram_w )
{
	chaknpop_txram[offset] = data;
	tilemap_mark_tile_dirty(tx_tilemap, offset);
}

WRITE8_HANDLER( chaknpop_attrram_w )
{
	if (chaknpop_attrram[offset] != data)
	{
		chaknpop_attrram[offset] = data;

		if (offset == TX_COLOR1 || offset == TX_COLOR2)
			tx_tilemap_mark_all_dirty();
	}
}


/***************************************************************************
  Callback for the tilemap code
***************************************************************************/

/*
 *  I'm not sure how to handle attributes about color
 */

static TILE_GET_INFO( chaknpop_get_tx_tile_info )
{
	int tile = chaknpop_txram[tile_index];
	int tile_h_bank = (gfxmode & GFX_TX_BANK2) << 2;	/* 0x00-0xff -> 0x200-0x2ff */
	int color = chaknpop_attrram[TX_COLOR2];

	if (tile == 0x74)
		color = chaknpop_attrram[TX_COLOR1];

	if (gfxmode & GFX_TX_BANK1 && tile >= 0xc0)
		tile += 0xc0;					/* 0xc0-0xff -> 0x180-0x1bf */
	tile |= tile_h_bank;

	SET_TILE_INFO(1, tile, color, 0);
}


/***************************************************************************
  Initialize video hardware emulation
***************************************************************************/

VIDEO_START( chaknpop )
{
	UINT8 *RAM = memory_region(REGION_CPU1);

	/*                          info                       offset             type             w   h  col row */
	tx_tilemap = tilemap_create(chaknpop_get_tx_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN,  8,  8, 32, 32);

	vram1 = &RAM[0x10000];
	vram2 = &RAM[0x12000];
	vram3 = &RAM[0x14000];
	vram4 = &RAM[0x16000];

	set_vram_bank();
	tx_tilemap_mark_all_dirty();

	state_save_register_global(gfxmode);
	state_save_register_global_pointer(vram1, 0x2000);
	state_save_register_global_pointer(vram2, 0x2000);
	state_save_register_global_pointer(vram3, 0x2000);
	state_save_register_global_pointer(vram4, 0x2000);
	state_save_register_global(flip_x);
	state_save_register_global(flip_y);

	state_save_register_func_postload(set_vram_bank);
	state_save_register_func_postload(tx_tilemap_mark_all_dirty);
}


/***************************************************************************
  Screen refresh
***************************************************************************/

static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect)
{
	int offs;

	/* Draw the sprites */
	for (offs = 0; offs < chaknpop_sprram_size; offs += 4)
	{
		int sx = chaknpop_sprram[offs + 3];
		int sy = 256 - 15 - chaknpop_sprram[offs];
		int flipx = chaknpop_sprram[offs+1] & 0x40;
		int flipy = chaknpop_sprram[offs+1] & 0x80;
		int color = (chaknpop_sprram[offs + 2] & 7);
		int tile = (chaknpop_sprram[offs + 1] & 0x3f) | ((chaknpop_sprram[offs + 2] & 0x38) << 3);

		if (flip_x)
		{
			sx = 240 - sx;
			flipx = !flipx;
		}
		if (flip_y)
		{
			sy = 242 - sy;
			flipy = !flipy;
		}

		drawgfx(bitmap,machine->gfx[0],
				tile,
				color,
				flipx, flipy,
				sx, sy,
				cliprect,
				TRANSPARENCY_PEN, 0);
	}
}

static void draw_bitmap(mame_bitmap *bitmap, const rectangle *cliprect)
{
	int dx = flip_x ? -1 : 1;
	int offs, i;

	for (offs = 0; offs < 0x2000; offs++)
	{
		int x = ((offs & 0x1f) << 3) + 7;
		int y = offs >> 5;

		if (!flip_x)
			x = 255 - x;

		if (!flip_y)
			y = 255 - y;

		for (i = 0x80; i > 0; i >>= 1, x += dx)
		{
			pen_t color = 0;

			if (vram1[offs] & i)
				color |= 0x200;	// green lower cage
			if (vram2[offs] & i)
				color |= 0x080;
			if (vram3[offs] & i)
				color |= 0x100;	// green upper cage
			if (vram4[offs] & i)
				color |= 0x040;	// tx mask

			if (color)
			{
				pen_t pen = *BITMAP_ADDR16(bitmap, y, x);
				pen |= color;
				*BITMAP_ADDR16(bitmap, y, x) = pen;
			}
		}
	}
}

VIDEO_UPDATE( chaknpop )
{
	tilemap_draw(bitmap, cliprect, tx_tilemap, 0, 0);
	draw_sprites(machine,bitmap,cliprect);
	draw_bitmap(bitmap,cliprect);
	return 0;
}
