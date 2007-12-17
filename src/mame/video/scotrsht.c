#include "driver.h"

UINT8 *scotrsht_scroll;

static tilemap *bg_tilemap;
static int scotrsht_charbank = 0;
static int scotrsht_palette_bank = 0;

/* Similar as Iron Horse */
PALETTE_INIT( scotrsht )
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
	/* color_prom now points to the beginning of the character lookup table */


	/* there are eight 16 colors palette banks; sprites use colors 0x00-0x7f and */
	/* characters 0x80-0xff. */
	for (i = 0;i < TOTAL_COLORS(0)/8;i++)
	{
		int j;


		for (j = 0;j < 8;j++)
			COLOR(0,i + j * TOTAL_COLORS(0)/8) = (*color_prom & 0x0f) + 16 * j + 0x80;

		color_prom++;
	}

	for (i = 0;i < TOTAL_COLORS(1)/8;i++)
	{
		int j;


		for (j = 0;j < 8;j++)
			COLOR(1,i + j * TOTAL_COLORS(1)/8) = (*color_prom & 0x0f) + 16 * j;

		color_prom++;
	}
}

WRITE8_HANDLER( scotrsht_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( scotrsht_colorram_w )
{
	colorram[offset] = data;
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

	coin_counter_w(0, data & 1);
	coin_counter_w(1, data & 2);

	// data & 4 unknown
}


static TILE_GET_INFO( scotrsht_get_bg_tile_info )
{
	int attr = colorram[tile_index];
	int code = videoram[tile_index] + (scotrsht_charbank << 9) + ((attr & 0x40) << 2);
	int color = (attr & 0x0f) + scotrsht_palette_bank * 16;
	int flag = 0;

	if(attr & 0x10)	flag |= TILE_FLIPX;
	if(attr & 0x20)	flag |= TILE_FLIPY;

	// data & 0x80 -> tile priority?

	SET_TILE_INFO(0, code, color, flag);
}

/* Same as Jailbreak + palette bank */
static void draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect )
{
	int i;

	for (i = 0; i < spriteram_size; i += 4)
	{
		int attr = spriteram[i + 1];	// attributes = ?tyxcccc
		int code = spriteram[i] + ((attr & 0x40) << 2);
		int color = (attr & 0x0f) + scotrsht_palette_bank * 16;
		int flipx = attr & 0x10;
		int flipy = attr & 0x20;
		int sx = spriteram[i + 2] - ((attr & 0x80) << 1);
		int sy = spriteram[i + 3];

		if (flip_screen)
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx(bitmap, machine->gfx[1], code, color, flipx, flipy,
			sx, sy, cliprect, TRANSPARENCY_COLOR, scotrsht_palette_bank * 16);
	}
}

VIDEO_START( scotrsht )
{
	bg_tilemap = tilemap_create(scotrsht_get_bg_tile_info, tilemap_scan_rows, TILEMAP_TYPE_PEN, 8, 8, 64, 32);

	tilemap_set_scroll_cols(bg_tilemap, 64);
}

VIDEO_UPDATE( scotrsht )
{
	int col;

	for (col = 0; col < 32; col++)
		tilemap_set_scrolly(bg_tilemap, col, scotrsht_scroll[col]);

	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	draw_sprites(machine, bitmap, cliprect);
	return 0;
}
