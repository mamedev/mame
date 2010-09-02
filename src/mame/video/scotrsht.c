#include "emu.h"
#include "includes/scotrsht.h"


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
	scotrsht_state *state = space->machine->driver_data<scotrsht_state>();

	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

WRITE8_HANDLER( scotrsht_colorram_w )
{
	scotrsht_state *state = space->machine->driver_data<scotrsht_state>();

	state->colorram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

WRITE8_HANDLER( scotrsht_charbank_w )
{
	scotrsht_state *state = space->machine->driver_data<scotrsht_state>();

	if (state->charbank != (data & 0x01))
	{
		state->charbank = data & 0x01;
		tilemap_mark_all_tiles_dirty(state->bg_tilemap);
	}

	/* other bits unknown */
}

WRITE8_HANDLER( scotrsht_palettebank_w )
{
	scotrsht_state *state = space->machine->driver_data<scotrsht_state>();

	if (state->palette_bank != ((data & 0x70) >> 4))
	{
		state->palette_bank = ((data & 0x70) >> 4);
		tilemap_mark_all_tiles_dirty(state->bg_tilemap);
	}

	coin_counter_w(space->machine, 0, data & 1);
	coin_counter_w(space->machine, 1, data & 2);

	// data & 4 unknown
}


static TILE_GET_INFO( scotrsht_get_bg_tile_info )
{
	scotrsht_state *state = machine->driver_data<scotrsht_state>();
	int attr = state->colorram[tile_index];
	int code = state->videoram[tile_index] + (state->charbank << 9) + ((attr & 0x40) << 2);
	int color = (attr & 0x0f) + state->palette_bank * 16;
	int flag = 0;

	if(attr & 0x10)	flag |= TILE_FLIPX;
	if(attr & 0x20)	flag |= TILE_FLIPY;

	// data & 0x80 -> tile priority?

	SET_TILE_INFO(0, code, color, flag);
}

/* Same as Jailbreak + palette bank */
static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	scotrsht_state *state = machine->driver_data<scotrsht_state>();
	UINT8 *spriteram = state->spriteram;
	int i;

	for (i = 0; i < state->spriteram_size; i += 4)
	{
		int attr = spriteram[i + 1];	// attributes = ?tyxcccc
		int code = spriteram[i] + ((attr & 0x40) << 2);
		int color = (attr & 0x0f) + state->palette_bank * 16;
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
			colortable_get_transpen_mask(machine->colortable, machine->gfx[1], color, state->palette_bank * 16));
	}
}

VIDEO_START( scotrsht )
{
	scotrsht_state *state = machine->driver_data<scotrsht_state>();

	state->bg_tilemap = tilemap_create(machine, scotrsht_get_bg_tile_info, tilemap_scan_rows,  8, 8, 64, 32);

	tilemap_set_scroll_cols(state->bg_tilemap, 64);
}

VIDEO_UPDATE( scotrsht )
{
	scotrsht_state *state = screen->machine->driver_data<scotrsht_state>();
	int col;

	for (col = 0; col < 32; col++)
		tilemap_set_scrolly(state->bg_tilemap, col, state->scroll[col]);

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}
