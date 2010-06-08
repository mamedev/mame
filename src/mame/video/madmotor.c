/***************************************************************************

  Mad Motor video emulation - Bryan McPhail, mish@tendril.co.uk

  Notes:  Playfield 3 can change size between 512x1024 and 2048x256

***************************************************************************/

#include "emu.h"
#include "includes/madmotor.h"


/* 512 by 512 playfield, 8 by 8 tiles */
static TILEMAP_MAPPER( pf1_scan )
{
	/* logical (col,row) -> memory offset */
	return (col & 0x1f) + ((row & 0x1f) << 5) + ((row & 0x20) << 5) + ((col & 0x20) << 6);
}

static TILE_GET_INFO( get_pf1_tile_info )
{
	madmotor_state *state = (madmotor_state *)machine->driver_data;
	int tile = state->pf1_data[tile_index];
	int color = tile >> 12;

	tile = tile & 0xfff;

	SET_TILE_INFO(
			0,
			tile,
			color,
			0);
}

/* 512 by 512 playfield, 16 by 16 tiles */
static TILEMAP_MAPPER( pf2_scan )
{
	/* logical (col,row) -> memory offset */
	return (col & 0x0f) + ((row & 0x0f) << 4) + ((row & 0x10) << 4) + ((col & 0x10) << 5);
}

static TILE_GET_INFO( get_pf2_tile_info )
{
	madmotor_state *state = (madmotor_state *)machine->driver_data;
	int tile = state->pf2_data[tile_index];
	int color = tile >> 12;

	tile = tile & 0xfff;

	SET_TILE_INFO(
			1,
			tile,
			color,
			0);
}

/* 512 by 1024 playfield, 16 by 16 tiles */
static TILEMAP_MAPPER( pf3_scan )
{
	/* logical (col,row) -> memory offset */
	return (col & 0x0f) + ((row & 0x0f) << 4) + ((row & 0x30) << 4) + ((col & 0x10) << 6);
}

static TILE_GET_INFO( get_pf3_tile_info )
{
	madmotor_state *state = (madmotor_state *)machine->driver_data;
	int tile = state->pf3_data[tile_index];
	int color = tile >> 12;

	tile = tile & 0xfff;

	SET_TILE_INFO(
			2,
			tile,
			color,
			0);
}

/* 2048 by 256 playfield, 16 by 16 tiles */
static TILEMAP_MAPPER( pf3a_scan )
{
	/* logical (col,row) -> memory offset */
	return (col & 0x0f) + ((row & 0x0f) << 4) + ((col & 0x70) << 4);
}

static TILE_GET_INFO( get_pf3a_tile_info )
{
	madmotor_state *state = (madmotor_state *)machine->driver_data;
	int tile = state->pf3_data[tile_index];
	int color = tile >> 12;

	tile = tile & 0xfff;

	SET_TILE_INFO(
			2,
			tile,
			color,
			0);
}

/******************************************************************************/

VIDEO_START( madmotor )
{
	madmotor_state *state = (madmotor_state *)machine->driver_data;

	state->pf1_tilemap = tilemap_create(machine, get_pf1_tile_info,  pf1_scan,   8,  8,  64, 64);
	state->pf2_tilemap = tilemap_create(machine, get_pf2_tile_info,  pf2_scan,  16, 16,  32, 32);
	state->pf3_tilemap = tilemap_create(machine, get_pf3_tile_info,  pf3_scan,  16, 16,  32, 64);
	state->pf3a_tilemap= tilemap_create(machine, get_pf3a_tile_info, pf3a_scan, 16, 16, 128, 16);

	tilemap_set_transparent_pen(state->pf1_tilemap, 0);
	tilemap_set_transparent_pen(state->pf2_tilemap, 0);
	tilemap_set_scroll_rows(state->pf1_tilemap, 512);
}

/******************************************************************************/

WRITE16_HANDLER( madmotor_pf1_data_w )
{
	madmotor_state *state = (madmotor_state *)space->machine->driver_data;

	COMBINE_DATA(&state->pf1_data[offset]);
	tilemap_mark_tile_dirty(state->pf1_tilemap, offset);
}

WRITE16_HANDLER( madmotor_pf2_data_w )
{
	madmotor_state *state = (madmotor_state *)space->machine->driver_data;

	COMBINE_DATA(&state->pf2_data[offset]);
	tilemap_mark_tile_dirty(state->pf2_tilemap, offset);
}

WRITE16_HANDLER( madmotor_pf3_data_w )
{
	madmotor_state *state = (madmotor_state *)space->machine->driver_data;

	COMBINE_DATA(&state->pf3_data[offset]);

	/* Mark the dirty position on the 512 x 1024 version */
	tilemap_mark_tile_dirty(state->pf3_tilemap, offset);

	/* Mark the dirty position on the 2048 x 256 version */
	tilemap_mark_tile_dirty(state->pf3a_tilemap, offset);
}

/******************************************************************************/

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int pri_mask, int pri_val )
{
	madmotor_state *state = (madmotor_state *)machine->driver_data;
	UINT16 *spriteram = state->spriteram;
	int offs;

	offs = 0;
	while (offs < state->spriteram_size / 2)
	{
		int sx, sy, code, color, w, h, flipx, flipy, incy, flash, mult, x, y;

		sy = spriteram[offs];
		sx = spriteram[offs + 2];
		color = sx >> 12;

		flash = sx & 0x800;

		flipx = sy & 0x2000;
		flipy = sy & 0x4000;
		h = (1 << ((sy & 0x1800) >> 11));	/* 1x, 2x, 4x, 8x height */
		w = (1 << ((sy & 0x0600) >>  9));	/* 1x, 2x, 4x, 8x width */
		/* multi width used only on the title screen? */

		code = spriteram[offs + 1] & 0x1fff;

		sx = sx & 0x01ff;
		sy = sy & 0x01ff;
		if (sx >= 256) sx -= 512;
		if (sy >= 256) sy -= 512;
		sx = 240 - sx;
		sy = 240 - sy;

		code &= ~(h-1);
		if (flipy)
			incy = -1;
		else
		{
			code += h-1;
			incy = 1;
		}

		if (state->flipscreen)
		{
			sy = 240 - sy;
			sx = 240 - sx;
			if (flipx) flipx = 0; else flipx = 1;
			if (flipy) flipy = 0; else flipy = 1;
			mult = 16;
		}
		else
			mult = -16;

		for (x = 0; x < w; x++)
		{
			for (y = 0; y < h; y++)
			{
				if ((color & pri_mask) == pri_val &&
							(!flash || (machine->primary_screen->frame_number() & 1)))
					drawgfx_transpen(bitmap,cliprect,machine->gfx[3],
							code - y * incy + h * x,
							color,
							flipx,flipy,
							sx + mult * x,sy + mult * y,0);
			}

			offs += 4;
			if (offs >= state->spriteram_size / 2 || spriteram[offs] & 0x8000)	// seems the expected behaviour on the title screen
				 break;
		}
	}
}


/******************************************************************************/

VIDEO_UPDATE( madmotor )
{
	madmotor_state *state = (madmotor_state *)screen->machine->driver_data;
	int offs;

	/* Update flipscreen */
	if (state->pf1_control[0] & 0x80)
		state->flipscreen = 1;
	else
		state->flipscreen = 0;

	tilemap_set_flip_all(screen->machine, state->flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	/* Setup scroll registers */
	for (offs = 0; offs < 512; offs++)
		tilemap_set_scrollx(state->pf1_tilemap, offs, state->pf1_control[0x08] + state->pf1_rowscroll[0x200 + offs]);

	tilemap_set_scrolly(state->pf1_tilemap,  0, state->pf1_control[0x09]);
	tilemap_set_scrollx(state->pf2_tilemap,  0, state->pf2_control[0x08]);
	tilemap_set_scrolly(state->pf2_tilemap,  0, state->pf2_control[0x09]);
	tilemap_set_scrollx(state->pf3_tilemap,  0, state->pf3_control[0x08]);
	tilemap_set_scrolly(state->pf3_tilemap,  0, state->pf3_control[0x09]);
	tilemap_set_scrollx(state->pf3a_tilemap, 0, state->pf3_control[0x08]);
	tilemap_set_scrolly(state->pf3a_tilemap, 0, state->pf3_control[0x09]);

	/* Draw playfields & sprites */
	if (state->pf3_control[0x03] == 2)
		tilemap_draw(bitmap, cliprect, state->pf3_tilemap, 0, 0);
	else
		tilemap_draw(bitmap, cliprect, state->pf3a_tilemap, 0, 0);

	tilemap_draw(bitmap, cliprect, state->pf2_tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect, 0x00, 0x00);
	tilemap_draw(bitmap, cliprect, state->pf1_tilemap, 0, 0);
	return 0;
}
