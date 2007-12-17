/******************************************************************************

    Video Hardware for Video System Mahjong series.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 2000/06/10 -
    Driver by Uki 2001/12/10 -

******************************************************************************/

#include "driver.h"
#include "cpu/z80/z80.h"


static UINT8 *ojankohs_videoram;
static UINT8 *ojankohs_colorram;
static UINT8 *ojankohs_paletteram;
static int ojankohs_gfxreg;
static int ojankohs_flipscreen;
static int ojankohs_scrollx, ojankohs_scrolly;
static tilemap *ojankohs_tilemap;
static int ojankoc_screen_refresh;
static mame_bitmap *ojankoc_tmpbitmap;

WRITE8_HANDLER( ojankoc_videoram_w );


/******************************************************************************

    Palette system

******************************************************************************/

PALETTE_INIT( ojankoy )
{
	int i;
	int bit0, bit1, bit2, bit3, bit4, r, g, b;

	for (i = 0; i < machine->drv->total_colors; i++) {
		bit0 = (color_prom[0] >> 2) & 0x01;
		bit1 = (color_prom[0] >> 3) & 0x01;
		bit2 = (color_prom[0] >> 4) & 0x01;
		bit3 = (color_prom[0] >> 5) & 0x01;
		bit4 = (color_prom[0] >> 6) & 0x01;
		r = 0x08 * bit0 + 0x11 * bit1 + 0x21 * bit2 + 0x43 * bit3 + 0x82 * bit4;
		bit0 = (color_prom[machine->drv->total_colors] >> 5) & 0x01;
		bit1 = (color_prom[machine->drv->total_colors] >> 6) & 0x01;
		bit2 = (color_prom[machine->drv->total_colors] >> 7) & 0x01;
		bit3 = (color_prom[0] >> 0) & 0x01;
		bit4 = (color_prom[0] >> 1) & 0x01;
		g = 0x08 * bit0 + 0x11 * bit1 + 0x21 * bit2 + 0x43 * bit3 + 0x82 * bit4;
		bit0 = (color_prom[machine->drv->total_colors] >> 0) & 0x01;
		bit1 = (color_prom[machine->drv->total_colors] >> 1) & 0x01;
		bit2 = (color_prom[machine->drv->total_colors] >> 2) & 0x01;
		bit3 = (color_prom[machine->drv->total_colors] >> 3) & 0x01;
		bit4 = (color_prom[machine->drv->total_colors] >> 4) & 0x01;
		b = 0x08 * bit0 + 0x11 * bit1 + 0x21 * bit2 + 0x43 * bit3 + 0x82 * bit4;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
		color_prom++;
	}
}

READ8_HANDLER( ojankohs_palette_r )
{
	return ojankohs_paletteram[offset];
}

WRITE8_HANDLER( ojankohs_palette_w )
{
	int r, g, b;

	ojankohs_paletteram[offset] = data;

	offset &= 0x7fe;

	r = (ojankohs_paletteram[offset + 0] & 0x7c) >> 2;
	g = ((ojankohs_paletteram[offset + 0] & 0x03) << 3) |
			((ojankohs_paletteram[offset + 1] & 0xe0) >> 5);
	b = (ojankohs_paletteram[offset + 1] & 0x1f) >> 0;

	palette_set_color_rgb(Machine,offset >> 1, pal5bit(r), pal5bit(g), pal5bit(b));
}

WRITE8_HANDLER( ccasino_palette_w )
{
	int r, g, b;

	/* get top 8 bits of the I/O port address */
	offset = (offset << 8) | (activecpu_get_reg(Z80_BC) >> 8);

	ojankohs_paletteram[offset] = data;

	offset &= 0x7fe;

	r = (ojankohs_paletteram[offset + 0] & 0x7c) >> 2;
	g = ((ojankohs_paletteram[offset + 0] & 0x03) << 3) |
			((ojankohs_paletteram[offset + 1] & 0xe0) >> 5);
	b = (ojankohs_paletteram[offset + 1] & 0x1f) >> 0;

	palette_set_color_rgb(Machine,offset >> 1, pal5bit(r), pal5bit(g), pal5bit(b));
}

WRITE8_HANDLER( ojankoc_palette_w )
{
	int r, g, b, color;

	if (ojankohs_paletteram[offset] == data) return;

	ojankohs_paletteram[offset] = data;
	ojankoc_screen_refresh = 1;

	color = (ojankohs_paletteram[offset & 0x1e] << 8) | ojankohs_paletteram[offset | 0x01];

	r = (color >> 10) & 0x1f;
	g = (color >>  5) & 0x1f;
	b = (color >>  0) & 0x1f;

	palette_set_color_rgb(Machine,offset >> 1, pal5bit(r), pal5bit(g), pal5bit(b));
}


/******************************************************************************

    Tilemap system

******************************************************************************/

READ8_HANDLER( ojankohs_videoram_r )
{
	return ojankohs_videoram[offset];
}

WRITE8_HANDLER( ojankohs_videoram_w )
{
	ojankohs_videoram[offset] = data;
	tilemap_mark_tile_dirty(ojankohs_tilemap, offset);
}

READ8_HANDLER( ojankohs_colorram_r )
{
	return ojankohs_colorram[offset];
}

WRITE8_HANDLER( ojankohs_colorram_w )
{
	ojankohs_colorram[offset] = data;
	tilemap_mark_tile_dirty(ojankohs_tilemap, offset);
}

WRITE8_HANDLER( ojankohs_gfxreg_w )
{
	if (ojankohs_gfxreg != data) {
		ojankohs_gfxreg = data;
		tilemap_mark_all_tiles_dirty(ojankohs_tilemap);
	}
}

WRITE8_HANDLER( ojankohs_flipscreen_w )
{
	if (ojankohs_flipscreen != (data & 0x01)) {

		ojankohs_flipscreen = data & 0x01;

		tilemap_set_flip(ALL_TILEMAPS, ojankohs_flipscreen ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0);

		if (ojankohs_flipscreen) {
			ojankohs_scrollx = -0xe0;
			ojankohs_scrolly = -0x20;
		}
		else {
			ojankohs_scrollx = 0;
			ojankohs_scrolly = 0;
		}
	}
}

static TILE_GET_INFO( ojankohs_get_tile_info )
{
	int tile, color;

	tile = ojankohs_videoram[tile_index] | ((ojankohs_colorram[tile_index] & 0x0f) << 8);
	color = (ojankohs_colorram[tile_index] & 0xe0) >> 5;

	if (ojankohs_colorram[tile_index] & 0x10) {
		tile |= (ojankohs_gfxreg & 0x07) << 12;
		color |= (ojankohs_gfxreg & 0xe0) >> 2;
	}

	SET_TILE_INFO(0, tile, color, 0);
}

static TILE_GET_INFO( ojankoy_get_tile_info )
{
	int tile, color, flipx, flipy;

	tile = ojankohs_videoram[tile_index] | (ojankohs_videoram[tile_index + 0x1000] << 8);
	color = ojankohs_colorram[tile_index] & 0x3f;
	flipx = ((ojankohs_colorram[tile_index] & 0x40) >> 6) ? TILEMAP_FLIPX : 0;
	flipy = ((ojankohs_colorram[tile_index] & 0x80) >> 7) ? TILEMAP_FLIPY : 0;

	SET_TILE_INFO(0, tile, color, (flipx | flipy));
}


/******************************************************************************

    Pixel system

******************************************************************************/

void ojankoc_flipscreen(int data)
{
	static int ojankoc_flipscreen_old = 0;
	int x, y;
	UINT8 color1, color2;

	ojankohs_flipscreen = (data & 0x80) >> 7;

	if (ojankohs_flipscreen == ojankoc_flipscreen_old) return;

	for (y = 0; y < 0x40; y++) {
		for (x = 0; x < 0x100; x++) {
			color1 = ojankohs_videoram[0x0000 + ((y * 256) + x)];
			color2 = ojankohs_videoram[0x3fff - ((y * 256) + x)];
			ojankoc_videoram_w(0x0000 + ((y * 256) + x), color2);
			ojankoc_videoram_w(0x3fff - ((y * 256) + x), color1);

			color1 = ojankohs_videoram[0x4000 + ((y * 256) + x)];
			color2 = ojankohs_videoram[0x7fff - ((y * 256) + x)];
			ojankoc_videoram_w(0x4000 + ((y * 256) + x), color2);
			ojankoc_videoram_w(0x7fff - ((y * 256) + x), color1);
		}
	}

	ojankoc_flipscreen_old = ojankohs_flipscreen;
}

WRITE8_HANDLER( ojankoc_videoram_w )
{
	int i;
	UINT8 x, y, xx, px, py ;
	UINT8 color, color1, color2;

	ojankohs_videoram[offset] = data;

	color1 = ojankohs_videoram[offset & 0x3fff];
	color2 = ojankohs_videoram[offset | 0x4000];

	y = offset >> 6;
	x = (offset & 0x3f) << 2;
	xx = 0;

	if (ojankohs_flipscreen) {
		x = 0xfc - x;
		y = 0xff - y;
		xx = 3;
	}

	for (i = 0; i < 4; i++) {
		color = ((color1 & 0x01) >> 0) | ((color1 & 0x10) >> 3) | ((color2 & 0x01) << 2) | ((color2 & 0x10) >> 1);

		px = x + (i ^ xx);
		py = y;

		*BITMAP_ADDR16(ojankoc_tmpbitmap, py, px) = Machine->pens[color];

		color1 >>= 1;
		color2 >>= 1;
	}
}


/******************************************************************************

    Start the video hardware emulation

******************************************************************************/

VIDEO_START( ojankohs )
{
	ojankohs_tilemap = tilemap_create(ojankohs_get_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 4, 64, 64);
	ojankohs_videoram = auto_malloc(0x2000);
	ojankohs_colorram = auto_malloc(0x1000);
	ojankohs_paletteram = auto_malloc(0x800);
}

VIDEO_START( ojankoy )
{
	ojankohs_tilemap = tilemap_create(ojankoy_get_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 4, 64, 64);
	ojankohs_videoram = auto_malloc(0x2000);
	ojankohs_colorram = auto_malloc(0x1000);
	ojankohs_paletteram = auto_malloc(0x800);
}

VIDEO_START( ojankoc )
{
	ojankoc_tmpbitmap = auto_bitmap_alloc(machine->screen[0].width, machine->screen[0].height, machine->screen[0].format);
	ojankohs_videoram = auto_malloc(0x8000);
	ojankohs_paletteram = auto_malloc(0x20);
}


/******************************************************************************

    Display refresh

******************************************************************************/

VIDEO_UPDATE( ojankohs )
{
	tilemap_set_scrollx(ojankohs_tilemap, 0, ojankohs_scrollx);
	tilemap_set_scrolly(ojankohs_tilemap, 0, ojankohs_scrolly);

	tilemap_draw(bitmap, cliprect, ojankohs_tilemap, 0, 0);
	return 0;
}

VIDEO_UPDATE( ojankoc )
{
	int offs;

	if (ojankoc_screen_refresh)
	{
		/* redraw bitmap */
		for (offs = 0; offs < 0x8000; offs++) {
			ojankoc_videoram_w(offs, ojankohs_videoram[offs]);
		}
		ojankoc_screen_refresh = 0;
	}

	copybitmap(bitmap, ojankoc_tmpbitmap, 0, 0, 0, 0, cliprect, TRANSPARENCY_NONE, 0);
	return 0;
}
