/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/cop01.h"


PALETTE_INIT( cop01 )
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

	/* characters use colors 0x00-0x0f (or 0x00-0x7f, but the eight rows are identical) */
	for (i = 0; i < 0x10; i++)
		colortable_entry_set_value(machine->colortable, i, i);

	/* background tiles use colors 0xc0-0xff */
	/* I don't know how much of the lookup table PROM is hooked up, */
	/* I'm only using the first 32 bytes because the rest is empty. */
	for (i = 0x10; i < 0x90; i++)
	{
		UINT8 ctabentry = 0xc0 | ((i - 0x10) & 0x30) |
						  (color_prom[(((i - 0x10) & 0x40) >> 2) | ((i - 0x10) & 0x0f)] & 0x0f);
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}

	/* sprites use colors 0x80-0x8f (or 0x80-0xbf, but the four rows are identical) */
	for (i = 0x90; i < 0x190; i++)
	{
		UINT8 ctabentry = 0x80 | (color_prom[i - 0x90 + 0x100] & 0x0f);
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_bg_tile_info )
{
	cop01_state *state = (cop01_state *)machine->driver_data;
	int tile = state->bgvideoram[tile_index];
	int attr = state->bgvideoram[tile_index + 0x800];
	int pri = (attr & 0x80) >> 7;

	/* kludge: priority is not actually pen based, but color based. Since the
     * game uses a lookup table, the two are not the same thing.
     * Palette entries with bits 2&3 set have priority over sprites.
     * tilemap.c can't handle that yet, so I'm cheating, because I know that
     * color codes using the second row of the lookup table don't use palette
     * entries 12-15.
     * The only place where this has any effect is the beach at the bottom of
     * the screen right at the beginning of mightguy. cop01 doesn't seem to
     * use priority at all.
     */
	if (attr & 0x10)
		pri = 0;

	SET_TILE_INFO(1, tile + ((attr & 0x03) << 8), (attr & 0x1c) >> 2, 0);
	tileinfo->group = pri;
}

static TILE_GET_INFO( get_fg_tile_info )
{
	cop01_state *state = (cop01_state *)machine->driver_data;
	int tile = state->fgvideoram[tile_index];
	SET_TILE_INFO(0, tile, 0, 0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( cop01 )
{
	cop01_state *state = (cop01_state *)machine->driver_data;
	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info,tilemap_scan_rows, 8, 8, 64, 32);
	state->fg_tilemap = tilemap_create(machine, get_fg_tile_info,tilemap_scan_rows, 8, 8, 32, 32);

	tilemap_set_transparent_pen(state->fg_tilemap, 15);

	/* priority doesn't exactly work this way, see above */
	tilemap_set_transmask(state->bg_tilemap, 0, 0xffff, 0x0000); /* split type 0 is totally transparent in front half */
	tilemap_set_transmask(state->bg_tilemap, 1, 0x0fff, 0xf000); /* split type 1 has pens 0-11 transparent in front half */
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( cop01_background_w )
{
	cop01_state *state = (cop01_state *)space->machine->driver_data;
	state->bgvideoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset & 0x7ff);
}

WRITE8_HANDLER( cop01_foreground_w )
{
	cop01_state *state = (cop01_state *)space->machine->driver_data;
	state->fgvideoram[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap, offset);
}

WRITE8_HANDLER( cop01_vreg_w )
{
	/*  0x40: --xx---- sprite bank, coin counters, flip screen
     *        -----x-- flip screen
     *        ------xx coin counters
     *  0x41: xxxxxxxx xscroll
     *  0x42: ---xx--- ? matches the bg tile color most of the time, but not
     *                 during level transitions. Maybe sprite palette bank?
     *                 (the four banks in the PROM are identical)
     *        ------x- unused (xscroll overflow)
     *        -------x msb xscroll
     *  0x43: xxxxxxxx yscroll
     */
	cop01_state *state = (cop01_state *)space->machine->driver_data;
	state->vreg[offset] = data;

	if (offset == 0)
	{
		coin_counter_w(space->machine, 0, data & 1);
		coin_counter_w(space->machine, 1, data & 2);
		flip_screen_set(space->machine, data & 4);
	}
}



/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	cop01_state *state = (cop01_state *)machine->driver_data;
	int offs, code, attr, sx, sy, flipx, flipy, color;

	for (offs = 0; offs < state->spriteram_size; offs += 4)
	{
		code = state->spriteram[offs + 1];
		attr = state->spriteram[offs + 2];
		/* xxxx---- color
         * ----xx-- flipy,flipx
         * -------x msbx
         */
		color = attr>>4;
		flipx = attr & 0x04;
		flipy = attr & 0x08;

		sx = (state->spriteram[offs + 3] - 0x80) + 256 * (attr & 0x01);
		sy = 240 - state->spriteram[offs];

		if (flip_screen_get(machine))
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		if (code & 0x80)
			code += (state->vreg[0] & 0x30) << 3;

		drawgfx_transpen(bitmap,cliprect,machine->gfx[2],
			code,
			color,
			flipx,flipy,
			sx,sy,0 );
	}
}


VIDEO_UPDATE( cop01 )
{
	cop01_state *state = (cop01_state *)screen->machine->driver_data;
	tilemap_set_scrollx(state->bg_tilemap, 0, state->vreg[1] + 256 * (state->vreg[2] & 1));
	tilemap_set_scrolly(state->bg_tilemap, 0, state->vreg[3]);

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_LAYER1, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_LAYER0, 0);
	tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0 );
	return 0;
}
