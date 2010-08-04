#include "emu.h"
#include "includes/cbasebal.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_bg_tile_info )
{
	cbasebal_state *state = machine->driver_data<cbasebal_state>();
	UINT8 attr = state->scrollram[2 * tile_index + 1];
	SET_TILE_INFO(
			1,
			state->scrollram[2 * tile_index] + ((attr & 0x07) << 8) + 0x800 * state->tilebank,
			(attr & 0xf0) >> 4,
			(attr & 0x08) ? TILE_FLIPX : 0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	cbasebal_state *state = machine->driver_data<cbasebal_state>();
	UINT8 attr = state->textram[tile_index + 0x800];
	SET_TILE_INFO(
			0,
			state->textram[tile_index] + ((attr & 0xf0) << 4),
			attr & 0x07,
			(attr & 0x08) ? TILE_FLIPX : 0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( cbasebal )
{
	cbasebal_state *state = machine->driver_data<cbasebal_state>();

	state->textram = auto_alloc_array(machine, UINT8, 0x1000);
	state->scrollram = auto_alloc_array(machine, UINT8, 0x1000);

	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 16, 16, 64, 32);
	state->fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 64, 32);

	tilemap_set_transparent_pen(state->fg_tilemap, 3);

	state_save_register_global_pointer(machine, state->textram, 0x1000);
	state_save_register_global_pointer(machine, state->scrollram, 0x1000);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( cbasebal_textram_w )
{
	cbasebal_state *state = space->machine->driver_data<cbasebal_state>();

	state->textram[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap, offset & 0x7ff);
}

READ8_HANDLER( cbasebal_textram_r )
{
	cbasebal_state *state = space->machine->driver_data<cbasebal_state>();
	return state->textram[offset];
}

WRITE8_HANDLER( cbasebal_scrollram_w )
{
	cbasebal_state *state = space->machine->driver_data<cbasebal_state>();

	state->scrollram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset / 2);
}

READ8_HANDLER( cbasebal_scrollram_r )
{
	cbasebal_state *state = space->machine->driver_data<cbasebal_state>();
	return state->scrollram[offset];
}

WRITE8_HANDLER( cbasebal_gfxctrl_w )
{
	cbasebal_state *state = space->machine->driver_data<cbasebal_state>();

	/* bit 0 is unknown - toggles continuously */

	/* bit 1 is flip screen */
	state->flipscreen = data & 0x02;
	tilemap_set_flip_all(space->machine, state->flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	/* bit 2 is unknown - unused? */

	/* bit 3 is tile bank */
	if (state->tilebank != ((data & 0x08) >> 3))
	{
		state->tilebank = (data & 0x08) >> 3;
		tilemap_mark_all_tiles_dirty(state->bg_tilemap);
	}

	/* bit 4 is sprite bank */
	state->spritebank = (data & 0x10) >> 4;

	/* bits 5 is text enable */
	state->text_on = ~data & 0x20;

	/* bits 6-7 are bg/sprite enable (don't know which is which) */
	state->bg_on = ~data & 0x40;
	state->obj_on = ~data & 0x80;

	/* other bits unknown, but used */
}

WRITE8_HANDLER( cbasebal_scrollx_w )
{
	cbasebal_state *state = space->machine->driver_data<cbasebal_state>();
	state->scroll_x[offset] = data;
	tilemap_set_scrollx(state->bg_tilemap, 0, state->scroll_x[0] + 256 * state->scroll_x[1]);
}

WRITE8_HANDLER( cbasebal_scrolly_w )
{
	cbasebal_state *state = space->machine->driver_data<cbasebal_state>();
	state->scroll_y[offset] = data;
	tilemap_set_scrolly(state->bg_tilemap, 0, state->scroll_y[0] + 256 * state->scroll_y[1]);
}



/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	cbasebal_state *state = machine->driver_data<cbasebal_state>();
	UINT8 *spriteram = state->spriteram;
	int offs, sx, sy;

	/* the last entry is not a sprite, we skip it otherwise spang shows a bubble */
	/* moving diagonally across the screen */
	for (offs = state->spriteram_size - 8; offs >= 0; offs -= 4)
	{
		int code = spriteram[offs];
		int attr = spriteram[offs + 1];
		int color = attr & 0x07;
		int flipx = attr & 0x08;
		sx = spriteram[offs + 3] + ((attr & 0x10) << 4);
		sy = ((spriteram[offs + 2] + 8) & 0xff) - 8;
		code += (attr & 0xe0) << 3;
		code += state->spritebank * 0x800;

		if (state->flipscreen)
		{
			sx = 496 - sx;
			sy = 240 - sy;
			flipx = !flipx;
		}

		drawgfx_transpen(bitmap,cliprect,machine->gfx[2],
				code,
				color,
				flipx,state->flipscreen,
				sx,sy,15);
	}
}

VIDEO_UPDATE( cbasebal )
{
	cbasebal_state *state = screen->machine->driver_data<cbasebal_state>();

	if (state->bg_on)
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	else
		bitmap_fill(bitmap, cliprect, 768);

	if (state->obj_on)
		draw_sprites(screen->machine, bitmap, cliprect);

	if (state->text_on)
		tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);
	return 0;
}
