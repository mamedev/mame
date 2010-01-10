#include "emu.h"

UINT8 *scotrsht_scroll;

UINT8 *scotrsht_videoram;
UINT8 *scotrsht_colorram;
static tilemap_t *bg_tilemap;
static int scotrsht_charbank = 0;
static int scotrsht_palette_bank = 0;

/* Similar as Iron Horse */
PALETTE_INIT( scotrsht )
{
	int i;

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x100);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x100; i++)
	{
		int r = pal4bit(color_prom[i + 0x000]);
		int g = pal4bit(color_prom[i + 0x100]);
		int b = pal4bit(color_prom[i + 0x200]);

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x300;

	/* characters use colors 0x80-0xff, sprites use colors 0-0x7f */
	for (i = 0; i < 0x200; i++)
	{
		int j;

		for (j = 0; j < 8; j++)
		{
			UINT8 ctabentry = ((~i & 0x100) >> 1) | (j << 4) | (color_prom[i] & 0x0f);
			colortable_entry_set_value(machine->colortable, ((i & 0x100) << 3) | (j << 8) | (i & 0xff), ctabentry);
		}
	}
}

WRITE8_HANDLER( scotrsht_videoram_w )
{
	scotrsht_videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( scotrsht_colorram_w )
{
	scotrsht_colorram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( scotrsht_charbank_w )
{
	if (scotrsht_charbank != (data & 0x01))
	{
		scotrsht_charbank = data & 0x01;
		tilemap_mark_all_tiles_dirty(bg_tilemap);
	}

	/* other bits unknown */
}

WRITE8_HANDLER( scotrsht_palettebank_w )
{
	if(scotrsht_palette_bank != ((data & 0x70) >> 4))
	{
		scotrsht_palette_bank = ((data & 0x70) >> 4);
		tilemap_mark_all_tiles_dirty(bg_tilemap);
	}

	coin_counter_w(space->machine, 0, data & 1);
	coin_counter_w(space->machine, 1, data & 2);

	// data & 4 unknown
}


static TILE_GET_INFO( scotrsht_get_bg_tile_info )
{
	int attr = scotrsht_colorram[tile_index];
	int code = scotrsht_videoram[tile_index] + (scotrsht_charbank << 9) + ((attr & 0x40) << 2);
	int color = (attr & 0x0f) + scotrsht_palette_bank * 16;
	int flag = 0;

	if(attr & 0x10)	flag |= TILE_FLIPX;
	if(attr & 0x20)	flag |= TILE_FLIPY;

	// data & 0x80 -> tile priority?

	SET_TILE_INFO(0, code, color, flag);
}

/* Same as Jailbreak + palette bank */
static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	UINT8 *spriteram = machine->generic.spriteram.u8;
	int i;

	for (i = 0; i < machine->generic.spriteram_size; i += 4)
	{
		int attr = spriteram[i + 1];	// attributes = ?tyxcccc
		int code = spriteram[i] + ((attr & 0x40) << 2);
		int color = (attr & 0x0f) + scotrsht_palette_bank * 16;
		int flipx = attr & 0x10;
		int flipy = attr & 0x20;
		int sx = spriteram[i + 2] - ((attr & 0x80) << 1);
		int sy = spriteram[i + 3];

		if (flip_screen_get(machine))
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_transmask(bitmap, cliprect, machine->gfx[1], code, color, flipx, flipy,
			sx, sy,
			colortable_get_transpen_mask(machine->colortable, machine->gfx[1], color, scotrsht_palette_bank * 16));
	}
}

VIDEO_START( scotrsht )
{
	bg_tilemap = tilemap_create(machine, scotrsht_get_bg_tile_info, tilemap_scan_rows,  8, 8, 64, 32);

	tilemap_set_scroll_cols(bg_tilemap, 64);
}

VIDEO_UPDATE( scotrsht )
{
	int col;

	for (col = 0; col < 32; col++)
		tilemap_set_scrolly(bg_tilemap, col, scotrsht_scroll[col]);

	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}
