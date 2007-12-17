/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "driver.h"

UINT8 *toypop_videoram;

extern UINT8 *m68000_sharedram;

static tilemap *bg_tilemap;
UINT16 *toypop_bg_image;
static int bitmapflip,palettebank;

static colortable *toypop_colortable;


/***************************************************************************

  Convert the color PROMs into a more useable format.

  toypop has three 256x4 palette PROM and two 256x8 color lookup table PROMs
  (one for characters, one for sprites).

***************************************************************************/

PALETTE_INIT( toypop )
{
	int i;

	/* allocate the colortable */
	toypop_colortable = colortable_alloc(machine, 256);

	for (i = 0;i < 256;i++)
	{
		int bit0,bit1,bit2,bit3,r,g,b;

		// red component
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		bit3 = (color_prom[i] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		// green component
		bit0 = (color_prom[i+0x100] >> 0) & 0x01;
		bit1 = (color_prom[i+0x100] >> 1) & 0x01;
		bit2 = (color_prom[i+0x100] >> 2) & 0x01;
		bit3 = (color_prom[i+0x100] >> 3) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		// blue component
		bit0 = (color_prom[i+0x200] >> 0) & 0x01;
		bit1 = (color_prom[i+0x200] >> 1) & 0x01;
		bit2 = (color_prom[i+0x200] >> 2) & 0x01;
		bit3 = (color_prom[i+0x200] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		colortable_palette_set_color(toypop_colortable, i, MAKE_RGB(r,g,b));
	}

	for (i = 0;i < 256;i++)
	{
		UINT8 entry;

		// characters
		colortable_entry_set_value(toypop_colortable, i + 0*256, (color_prom[i + 0x300] & 0x0f) | 0x70);
		colortable_entry_set_value(toypop_colortable, i + 1*256, (color_prom[i + 0x300] & 0x0f) | 0xf0);
		// sprites
		entry = color_prom[i + 0x500];
		colortable_entry_set_value(toypop_colortable, i + 2*256, entry);
	}
	for (i = 0;i < 16;i++)
	{
		// background
		colortable_entry_set_value(toypop_colortable, i + 3*256 + 0*16, 0x60 + i);
		colortable_entry_set_value(toypop_colortable, i + 3*256 + 1*16, 0xe0 + i);
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

/* convert from 32x32 to 36x28 */
static TILEMAP_MAPPER( tilemap_scan )
{
	int offs;

	row += 2;
	col -= 2;
	if (col & 0x20)
		offs = row + ((col & 0x1f) << 5);
	else
		offs = col + (row << 5);

	return offs;
}

static TILE_GET_INFO( get_tile_info )
{
	UINT8 attr = toypop_videoram[tile_index + 0x400];
	SET_TILE_INFO(
			0,
			toypop_videoram[tile_index],
			(attr & 0x3f) + 0x40 * palettebank,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( toypop )
{
	bg_tilemap = tilemap_create(get_tile_info,tilemap_scan,TILEMAP_TYPE_PEN,8,8,36,28);

	tilemap_set_transparent_pen(bg_tilemap, 0);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( toypop_videoram_w )
{
	toypop_videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,offset & 0x3ff);
}

WRITE8_HANDLER( toypop_palettebank_w )
{
	if (palettebank != (offset & 1))
	{
		palettebank = offset & 1;
		tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
	}
}

WRITE16_HANDLER( toypop_flipscreen_w )
{
	bitmapflip = offset & 1;
}

READ16_HANDLER( toypop_merged_background_r )
{
	int data1, data2;

	// 0x0a0b0c0d is read as 0xabcd
	data1 = toypop_bg_image[2*offset];
	data2 = toypop_bg_image[2*offset + 1];
	return ((data1 & 0xf00) << 4) | ((data1 & 0xf) << 8) | ((data2 & 0xf00) >> 4) | (data2 & 0xf);
}

WRITE16_HANDLER( toypop_merged_background_w )
{
	// 0xabcd is written as 0x0a0b0c0d in the background image
	if (ACCESSING_MSB)
		toypop_bg_image[2*offset] = ((data & 0xf00) >> 8) | ((data & 0xf000) >> 4);

	if (ACCESSING_LSB)
		toypop_bg_image[2*offset+1] = (data & 0xf) | ((data & 0xf0) << 4);
}

static void draw_background(mame_bitmap *bitmap)
{
	register int offs, x, y;
	UINT8 scanline[288];

	// copy the background image from RAM (0x190200-0x19FDFF) to bitmap
	if (bitmapflip)
	{
		offs = 0xFDFE/2;
		for (y = 0; y < 224; y++)
		{
			for (x = 0; x < 288; x+=2)
			{
				UINT16 data = toypop_bg_image[offs];
				scanline[x]   = data;
				scanline[x+1] = data >> 8;
				offs--;
			}
			draw_scanline8(bitmap, 0, y, 288, scanline, &Machine->pens[0x300 + 0x10*palettebank], -1);
		}
	}
	else
	{
		offs = 0x200/2;
		for (y = 0; y < 224; y++)
		{
			for (x = 0; x < 288; x+=2)
			{
				UINT16 data = toypop_bg_image[offs];
				scanline[x]   = data >> 8;
				scanline[x+1] = data;
				offs++;
			}
			draw_scanline8(bitmap, 0, y, 288, scanline, &Machine->pens[0x300 + 0x10*palettebank], -1);
		}
	}
}



/***************************************************************************

  Display refresh

***************************************************************************/

/* from mappy.c */
void mappy_draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int xoffs, int yoffs, colortable *ctable, int transcolor);


VIDEO_UPDATE( toypop )
{
	draw_background(bitmap);
	tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);

	mappy_draw_sprites(machine, bitmap, cliprect, -31, -8, toypop_colortable, 0xff);
	return 0;
}
