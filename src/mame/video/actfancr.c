/*******************************************************************************

    actfancr - Bryan McPhail, mish@tendril.co.uk

*******************************************************************************/

#include "emu.h"
#include "includes/actfancr.h"


static TILEMAP_MAPPER( actfancr_scan )
{
	/* logical (col,row) -> memory offset */
	return (col & 0x0f) + ((row & 0x0f) << 4) + ((col & 0xf0) << 4);
}

static TILEMAP_MAPPER( actfancr_scan2 )
{
	/* logical (col,row) -> memory offset */
	return (col & 0x0f) + ((row & 0x0f) << 4) + ((row & 0x10) << 4) + ((col & 0x70) << 5);
}

static TILE_GET_INFO( get_tile_info )
{
	actfancr_state *state = (actfancr_state *)machine->driver_data;
	int tile = state->pf1_data[2 * tile_index] + (state->pf1_data[2 * tile_index + 1] << 8);
	int color = tile >> 12;

	tile = tile & 0xfff;

	SET_TILE_INFO(
			2,
			tile,
			color,
			0);
}

static TILEMAP_MAPPER( triothep_scan )
{
	/* logical (col,row) -> memory offset */
	return (col & 0x0f) + ((row & 0x0f) << 4) + ((row & 0x10) << 4) + ((col & 0x10) << 5);
}

static TILE_GET_INFO( get_trio_tile_info )
{
	actfancr_state *state = (actfancr_state *)machine->driver_data;
	int tile = state->pf1_data[2 * tile_index] + (state->pf1_data[2 * tile_index + 1] << 8);
	int color = tile >> 12;

	tile = tile & 0xfff;

	SET_TILE_INFO(
			2,
			tile,
			color,
			0);
}

static TILE_GET_INFO( get_pf2_tile_info )
{
	actfancr_state *state = (actfancr_state *)machine->driver_data;
	int tile = state->pf2_data[2 * tile_index] + (state->pf2_data[2 * tile_index + 1] << 8);
	int color = tile >> 12;

	tile = tile & 0xfff;


	SET_TILE_INFO(
				0,
				tile,
				color,
				0);
}

/******************************************************************************/

static void register_savestate( running_machine *machine )
{
	actfancr_state *state = (actfancr_state *)machine->driver_data;
	state_save_register_global_array(machine, state->control_1);
	state_save_register_global_array(machine, state->control_2);
	state_save_register_global(machine, state->flipscreen);
}

VIDEO_START( actfancr )
{
	actfancr_state *state = (actfancr_state *)machine->driver_data;
	state->pf1_tilemap = tilemap_create(machine, get_tile_info, actfancr_scan, 16, 16, 256, 16);
	state->pf1_alt_tilemap = tilemap_create(machine, get_tile_info, actfancr_scan2, 16, 16, 128, 32);
	state->pf2_tilemap = tilemap_create(machine, get_pf2_tile_info, tilemap_scan_rows, 8, 8, 32, 32);

	tilemap_set_transparent_pen(state->pf2_tilemap, 0);

	register_savestate(machine);
}

VIDEO_START( triothep )
{
	actfancr_state *state = (actfancr_state *)machine->driver_data;
	state->pf1_tilemap = tilemap_create(machine, get_trio_tile_info, triothep_scan, 16, 16, 32, 32);
	state->pf2_tilemap = tilemap_create(machine, get_pf2_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->pf1_alt_tilemap = NULL;

	tilemap_set_transparent_pen(state->pf2_tilemap, 0);

	register_savestate(machine);
}

/******************************************************************************/

WRITE8_HANDLER( actfancr_pf1_control_w )
{
	actfancr_state *state = (actfancr_state *)space->machine->driver_data;
	state->control_1[offset] = data;
}

WRITE8_HANDLER( actfancr_pf2_control_w )
{
	actfancr_state *state = (actfancr_state *)space->machine->driver_data;
	state->control_2[offset] = data;
}

WRITE8_HANDLER( actfancr_pf1_data_w )
{
	actfancr_state *state = (actfancr_state *)space->machine->driver_data;
	state->pf1_data[offset] = data;
	tilemap_mark_tile_dirty(state->pf1_tilemap, offset / 2);
	if (state->pf1_alt_tilemap)
		tilemap_mark_tile_dirty(state->pf1_alt_tilemap, offset / 2);
}

READ8_HANDLER( actfancr_pf1_data_r )
{
	actfancr_state *state = (actfancr_state *)space->machine->driver_data;
	return state->pf1_data[offset];
}

WRITE8_HANDLER( actfancr_pf2_data_w )
{
	actfancr_state *state = (actfancr_state *)space->machine->driver_data;
	state->pf2_data[offset] = data;
	tilemap_mark_tile_dirty(state->pf2_tilemap, offset / 2);
}

READ8_HANDLER( actfancr_pf2_data_r )
{
	actfancr_state *state = (actfancr_state *)space->machine->driver_data;
	return state->pf2_data[offset];
}

/******************************************************************************/

VIDEO_UPDATE( actfancr )
{
	actfancr_state *state = (actfancr_state *)screen->machine->driver_data;
	UINT8 *buffered_spriteram = screen->machine->generic.buffered_spriteram.u8;
	int offs, mult;
	int scrollx = (state->control_1[0x10] + (state->control_1[0x11] << 8));
	int scrolly = (state->control_1[0x12] + (state->control_1[0x13] << 8));

	/* Draw playfield */
	state->flipscreen = state->control_2[0] & 0x80;
	tilemap_set_flip_all(screen->machine, state->flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	tilemap_set_scrollx(state->pf1_tilemap,0, scrollx );
	tilemap_set_scrolly(state->pf1_tilemap,0, scrolly );
	tilemap_set_scrollx(state->pf1_alt_tilemap, 0, scrollx );
	tilemap_set_scrolly(state->pf1_alt_tilemap, 0, scrolly );

	if (state->control_1[6] == 1)
		tilemap_draw(bitmap, cliprect, state->pf1_alt_tilemap, 0, 0);
	else
		tilemap_draw(bitmap, cliprect, state->pf1_tilemap, 0, 0);

	/* Sprites */
	for (offs = 0; offs < 0x800; offs += 8)
	{
		int x, y, sprite, colour, multi, fx, fy, inc, flash;

		y = buffered_spriteram[offs] + (buffered_spriteram[offs + 1] << 8);
		if ((y & 0x8000) == 0)
			continue;

		x = buffered_spriteram[offs + 4] + (buffered_spriteram[offs + 5] << 8);
		colour = ((x & 0xf000) >> 12);
		flash = x & 0x800;
		if (flash && (screen->frame_number() & 1))
			continue;

		fx = y & 0x2000;
		fy = y & 0x4000;
		multi = (1 << ((y & 0x1800) >> 11)) - 1;	/* 1x, 2x, 4x, 8x height */
										/* multi = 0   1   3   7 */

		sprite = buffered_spriteram[offs + 2] + (buffered_spriteram[offs + 3] << 8);
		sprite &= 0x0fff;

		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= 256) x -= 512;
		if (y >= 256) y -= 512;
		x = 240 - x;
		y = 240 - y;

		sprite &= ~multi;
		if (fy)
			inc = -1;
		else
		{
			sprite += multi;
			inc = 1;
		}

		if (state->flipscreen)
		{
			y = 240 - y;
			x = 240 - x;
			if (fx) fx = 0; else fx = 1;
			if (fy) fy = 0; else fy = 1;
			mult = 16;
		}
		else mult = -16;

		while (multi >= 0)
		{
			drawgfx_transpen(bitmap,cliprect,screen->machine->gfx[1],
					sprite - multi * inc,
					colour,
					fx,fy,
					x,y + mult * multi,0);
			multi--;
		}
	}

	tilemap_draw(bitmap, cliprect, state->pf2_tilemap, 0, 0);
	return 0;
}

VIDEO_UPDATE( triothep )
{
	actfancr_state *state = (actfancr_state *)screen->machine->driver_data;
	UINT8 *buffered_spriteram = screen->machine->generic.buffered_spriteram.u8;
	int offs, i, mult;
	int scrollx = (state->control_1[0x10] + (state->control_1[0x11] << 8));
	int scrolly = (state->control_1[0x12] + (state->control_1[0x13] << 8));

	/* Draw playfield */
	state->flipscreen = state->control_2[0] & 0x80;
	tilemap_set_flip_all(screen->machine, state->flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	if (state->control_2[0] & 0x4)
	{
		tilemap_set_scroll_rows(state->pf1_tilemap, 32);
		tilemap_set_scrolly(state->pf1_tilemap, 0, scrolly);
		for (i = 0; i < 32; i++)
			tilemap_set_scrollx(state->pf1_tilemap, i, scrollx + (state->pf1_rowscroll_data[i * 2] | state->pf1_rowscroll_data[i * 2 + 1] << 8) );
	}
	else
	{
		tilemap_set_scroll_rows(state->pf1_tilemap, 1);
		tilemap_set_scrollx(state->pf1_tilemap, 0, scrollx);
		tilemap_set_scrolly(state->pf1_tilemap, 0, scrolly);
	}

	tilemap_draw(bitmap, cliprect, state->pf1_tilemap, 0, 0);

	/* Sprites */
	for (offs = 0; offs < 0x800; offs += 8)
	{
		int x, y, sprite, colour, multi, fx, fy, inc, flash;

		y = buffered_spriteram[offs] + (buffered_spriteram[offs + 1] << 8);
		if ((y & 0x8000) == 0)
			continue;

		x = buffered_spriteram[offs + 4] + (buffered_spriteram[offs + 5] << 8);
		colour = ((x & 0xf000) >> 12);
		flash = x & 0x800;
		if (flash && (screen->frame_number() & 1))
			continue;

		fx = y & 0x2000;
		fy = y & 0x4000;
		multi = (1 << ((y & 0x1800) >> 11)) - 1;	/* 1x, 2x, 4x, 8x height */
											/* multi = 0   1   3   7 */

		sprite = buffered_spriteram[offs + 2] + (buffered_spriteram[offs + 3] << 8);
		sprite &= 0x0fff;

		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= 256) x -= 512;
		if (y >= 256) y -= 512;
		x = 240 - x;
		y = 240 - y;

		sprite &= ~multi;
		if (fy)
			inc = -1;
		else
		{
			sprite += multi;
			inc = 1;
		}

		if (state->flipscreen)
		{
			y = 240 - y;
			x = 240 - x;
			if (fx) fx = 0; else fx = 1;
			if (fy) fy = 0; else fy = 1;
			mult = 16;
		}
		else mult = -16;

		while (multi >= 0)
		{
			drawgfx_transpen(bitmap,cliprect,screen->machine->gfx[1],
					sprite - multi * inc,
					colour,
					fx,fy,
					x,y + mult * multi,0);
			multi--;
		}
	}

	tilemap_draw(bitmap, cliprect, state->pf2_tilemap, 0, 0);
	return 0;
}
