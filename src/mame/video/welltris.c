#include "driver.h"



extern UINT16 *welltris_spriteram;
//extern size_t welltris_spriteram_size;
extern UINT16 *welltris_pixelram;
extern UINT16 *welltris_charvideoram;

static tilemap *char_tilemap;
static UINT8 gfxbank[8];
static UINT16 charpalettebank;
static UINT16 spritepalettebank;
static UINT16 pixelpalettebank;
static int welltris_scrollx, welltris_scrolly;



#ifdef UNUSED_FUNCTION
READ16_HANDLER( welltris_spriteram_r )
{
	return welltris_spriteram[offset];
}
#endif

WRITE16_HANDLER( welltris_spriteram_w )
{
	int offs;

	COMBINE_DATA(&welltris_spriteram[offset]);

	/* hack... sprite doesn't work otherwise (quiz18kn) */
	if ((offset == 0x1fe) &&
		(welltris_spriteram[0x01fc] == 0x0000) &&
		(welltris_spriteram[0x01fd] == 0x0000) &&
		(welltris_spriteram[0x01ff] == 0x0000)) {
		for (offs = 0; offs < 0x1fc; offs++) welltris_spriteram[offs] = 0x0000;
	}
}


/* Sprite Drawing is pretty much the same as fromance.c */
static void draw_sprites(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect)
{
	static const UINT8 zoomtable[16] = { 0,7,14,20,25,30,34,38,42,46,49,52,54,57,59,61 };
	int offs;
	const rectangle *visarea = video_screen_get_visible_area(machine->primary_screen);

	/* draw the sprites */
	for (offs = 0; offs < 0x200 - 4; offs += 4) {
		int data0 = welltris_spriteram[offs + 0];
		int data1 = welltris_spriteram[offs + 1];
		int data2 = welltris_spriteram[offs + 2];
		int data3 = welltris_spriteram[offs + 3];
		int code = data3 & 0x1fff;
		int color = (data2 & 0x0f) + (0x10 * spritepalettebank);
		int y = (data0 & 0x1ff) + 1;
		int x = (data1 & 0x1ff) + 6;
		int yzoom = (data0 >> 12) & 15;
		int xzoom = (data1 >> 12) & 15;
		int zoomed = (xzoom | yzoom);
		int ytiles = ((data2 >> 12) & 7) + 1;
		int xtiles = ((data2 >>  8) & 7) + 1;
		int yflip = (data2 >> 15) & 1;
		int xflip = (data2 >> 11) & 1;
		int xt, yt;

		if (!(welltris_spriteram[offs + 2] & 0x0080)) continue;

		/* compute the zoom factor -- stolen from aerofgt.c */
		xzoom = 16 - zoomtable[xzoom] / 8;
		yzoom = 16 - zoomtable[yzoom] / 8;

		/* wrap around */
		if (x > visarea->max_x) x -= 0x200;
		if (y > visarea->max_y) y -= 0x200;

		/* normal case */
		if (!xflip && !yflip) {
			for (yt = 0; yt < ytiles; yt++) {
				for (xt = 0; xt < xtiles; xt++, code++) {
					if (!zoomed)
						drawgfx_transpen(bitmap, cliprect, machine->gfx[1], code, color, 0, 0,
								x + xt * 16, y + yt * 16, 15);
					else
						drawgfxzoom_transpen(bitmap, cliprect, machine->gfx[1], code, color, 0, 0,
								x + xt * xzoom, y + yt * yzoom,
								0x1000 * xzoom, 0x1000 * yzoom, 15);
				}
				if (xtiles == 3) code += 1;
				if (xtiles == 5) code += 3;
				if (xtiles == 6) code += 2;
				if (xtiles == 7) code += 1;
			}
		}

		/* xflipped case */
		else if (xflip && !yflip) {
			for (yt = 0; yt < ytiles; yt++) {
				for (xt = 0; xt < xtiles; xt++, code++) {
					if (!zoomed)
						drawgfx_transpen(bitmap, cliprect, machine->gfx[1], code, color, 1, 0,
								x + (xtiles - 1 - xt) * 16, y + yt * 16, 15);
					else
						drawgfxzoom_transpen(bitmap, cliprect, machine->gfx[1], code, color, 1, 0,
								x + (xtiles - 1 - xt) * xzoom, y + yt * yzoom,
								0x1000 * xzoom, 0x1000 * yzoom, 15);
				}
				if (xtiles == 3) code += 1;
				if (xtiles == 5) code += 3;
				if (xtiles == 6) code += 2;
				if (xtiles == 7) code += 1;
			}
		}

		/* yflipped case */
		else if (!xflip && yflip) {
			for (yt = 0; yt < ytiles; yt++) {
				for (xt = 0; xt < xtiles; xt++, code++) {
					if (!zoomed)
						drawgfx_transpen(bitmap, cliprect, machine->gfx[1], code, color, 0, 1,
								x + xt * 16, y + (ytiles - 1 - yt) * 16, 15);
					else
						drawgfxzoom_transpen(bitmap, cliprect, machine->gfx[1], code, color, 0, 1,
								x + xt * xzoom, y + (ytiles - 1 - yt) * yzoom,
								0x1000 * xzoom, 0x1000 * yzoom, 15);
				}
				if (xtiles == 3) code += 1;
				if (xtiles == 5) code += 3;
				if (xtiles == 6) code += 2;
				if (xtiles == 7) code += 1;
			}
		}

		/* x & yflipped case */
		else {
			for (yt = 0; yt < ytiles; yt++) {
				for (xt = 0; xt < xtiles; xt++, code++) {
					if (!zoomed)
						drawgfx_transpen(bitmap, cliprect, machine->gfx[1], code, color, 1, 1,
								x + (xtiles - 1 - xt) * 16, y + (ytiles - 1 - yt) * 16, 15);
					else
						drawgfxzoom_transpen(bitmap, cliprect, machine->gfx[1], code, color, 1, 1,
								x + (xtiles - 1 - xt) * xzoom, y + (ytiles - 1 - yt) * yzoom,
								0x1000 * xzoom, 0x1000 * yzoom, 15);
				}
				if (xtiles == 3) code += 1;
				if (xtiles == 5) code += 3;
				if (xtiles == 6) code += 2;
				if (xtiles == 7) code += 1;
			}
		}
	}
}

static void setbank(tilemap *tmap, int num, int bank)
{
	if (gfxbank[num] != bank)
	{
		gfxbank[num] = bank;
		tilemap_mark_all_tiles_dirty(tmap);
	}
}


/* Not really enough evidence here */

WRITE16_HANDLER( welltris_palette_bank_w )
{
	if (ACCESSING_BITS_0_7)
	{
		if (charpalettebank != (data & 0x03))
		{
			charpalettebank = (data & 0x03);
			tilemap_mark_all_tiles_dirty(char_tilemap);
		}

		flip_screen_set(space->machine, data & 0x80);

		spritepalettebank = (data & 0x20) >> 5;
		pixelpalettebank = (data & 0x08) >> 3;
	}
}

WRITE16_HANDLER( welltris_gfxbank_w )
{
	if (ACCESSING_BITS_0_7)
	{
		setbank(char_tilemap, 0, (data & 0xf0) >> 4);
		setbank(char_tilemap, 1, data & 0x0f);
	}
}

WRITE16_HANDLER( welltris_scrollreg_w )
{
	switch (offset) {
		case 0: welltris_scrollx = data - 14; break;
		case 1: welltris_scrolly = data +  0; break;
	}
}

static TILE_GET_INFO( get_welltris_tile_info )
{
	UINT16 code = welltris_charvideoram[tile_index];
	int bank = (code & 0x1000) >> 12;

	SET_TILE_INFO(
			0,
			(code & 0x0fff) + (gfxbank[bank] << 12),
			((code & 0xe000) >> 13) + (8 * charpalettebank),
			0);
}

WRITE16_HANDLER( welltris_charvideoram_w )
{
	COMBINE_DATA(&welltris_charvideoram[offset]);
	tilemap_mark_tile_dirty(char_tilemap, offset);
}

VIDEO_START( welltris )
{
	char_tilemap = tilemap_create(machine, get_welltris_tile_info, tilemap_scan_rows,  8, 8, 64, 32);

	tilemap_set_transparent_pen(char_tilemap, 15);
}

static void draw_background(bitmap_t *bitmap, const rectangle *cliprect)
{
	int x, y;
	int pixdata;

	for (y = 0; y < 256; y++) {
		for (x = 0; x < 512 / 2; x++) {
			pixdata = welltris_pixelram[(x & 0xff) + (y & 0xff) * 256];

			*BITMAP_ADDR16(bitmap, y, (x * 2) + 0) = (pixdata >> 8) + (0x100 * pixelpalettebank) + 0x400;
			*BITMAP_ADDR16(bitmap, y, (x * 2) + 1) = (pixdata & 0xff) + (0x100 * pixelpalettebank) + 0x400;
		}
	}
}

VIDEO_UPDATE( welltris )
{
	tilemap_set_scrollx(char_tilemap, 0, welltris_scrollx);
	tilemap_set_scrolly(char_tilemap, 0, welltris_scrolly);

	draw_background(bitmap, cliprect);
	tilemap_draw(bitmap, cliprect, char_tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}
