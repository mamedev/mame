/*

  Cross Pang
  video hardware emulation

 -- this seems to be the same as the tumblepop bootleg based hardware
    in tumbleb.c


*/

#include "driver.h"
#include "crospang.h"

WRITE16_HANDLER( bestri_tilebank_w)
{
	crospang_state *state = (crospang_state *)space->machine->driver_data;

	state->bestri_tilebank = (data>>10) & 0xf;
	//printf("bestri %04x\n", data);

	tilemap_mark_all_tiles_dirty(state->fg_layer);
	tilemap_mark_all_tiles_dirty(state->bg_layer);
}


WRITE16_HANDLER ( bestri_bg_scrolly_w )
{
	crospang_state *state = (crospang_state *)space->machine->driver_data;

	/* Very Strange */
	int scroll =  (data & 0x3ff) ^ 0x0155;
	tilemap_set_scrolly(state->bg_layer, 0, -scroll + 7);
}

WRITE16_HANDLER ( bestri_fg_scrolly_w )
{
	crospang_state *state = (crospang_state *)space->machine->driver_data;

	/* Very Strange */
	int scroll = (data & 0x3ff) ^ 0x00ab;
	tilemap_set_scrolly(state->fg_layer, 0, -scroll + 7);
}

WRITE16_HANDLER ( bestri_fg_scrollx_w )
{
	crospang_state *state = (crospang_state *)space->machine->driver_data;

	// printf("fg_layer x %04x\n",data);
	tilemap_set_scrollx(state->fg_layer, 0, data + 32);
}

WRITE16_HANDLER ( bestri_bg_scrollx_w )
{
	crospang_state *state = (crospang_state *)space->machine->driver_data;

	// printf("bg_layer x %04x\n",data);
	tilemap_set_scrollx(state->bg_layer, 0, data - 60);
}


WRITE16_HANDLER ( crospang_fg_scrolly_w )
{
	crospang_state *state = (crospang_state *)space->machine->driver_data;
	tilemap_set_scrolly(state->fg_layer, 0, data + 8);
}

WRITE16_HANDLER ( crospang_bg_scrolly_w )
{
	crospang_state *state = (crospang_state *)space->machine->driver_data;
	tilemap_set_scrolly(state->bg_layer, 0, data + 8);
}

WRITE16_HANDLER ( crospang_fg_scrollx_w )
{
	crospang_state *state = (crospang_state *)space->machine->driver_data;
	tilemap_set_scrollx(state->fg_layer, 0, data);
}

WRITE16_HANDLER ( crospang_bg_scrollx_w )
{
	crospang_state *state = (crospang_state *)space->machine->driver_data;
	tilemap_set_scrollx(state->bg_layer, 0, data + 4);
}


WRITE16_HANDLER ( crospang_fg_videoram_w )
{
	crospang_state *state = (crospang_state *)space->machine->driver_data;
	COMBINE_DATA(&state->fg_videoram[offset]);
	tilemap_mark_tile_dirty(state->fg_layer, offset);
}

WRITE16_HANDLER ( crospang_bg_videoram_w )
{
	crospang_state *state = (crospang_state *)space->machine->driver_data;
	COMBINE_DATA(&state->bg_videoram[offset]);
	tilemap_mark_tile_dirty(state->bg_layer, offset);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	crospang_state *state = (crospang_state *)machine->driver_data;
	int data  = state->bg_videoram[tile_index];
	int tile  = data & 0xfff;
	int color = (data >> 12) & 0x0f;

	SET_TILE_INFO(1, tile + state->bestri_tilebank * 0x1000, color + 0x20, 0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	crospang_state *state = (crospang_state *)machine->driver_data;
	int data  = state->fg_videoram[tile_index];
	int tile  = data & 0xfff;
	int color = (data >> 12) & 0x0f;

	SET_TILE_INFO(1, tile + state->bestri_tilebank * 0x1000, color + 0x10, 0);
}

/*

 offset

      0     -------yyyyyyyyy  y offset
            -----hh---------  sprite height
            ---a------------  alpha blending enable
            f---------------  flip x
            -??-?-----------  unused

      1     --ssssssssssssss  sprite code
            ??--------------  unused

      2     -------xxxxxxxxx  x offset
            ---cccc---------  colors
            ???-------------  unused

      3     ----------------  unused

*/

/* jumpkids / tumbleb.c! */
static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	crospang_state *state = (crospang_state *)machine->driver_data;
	int offs;
	int flipscreen = 0;

	for (offs = 0; offs < state->spriteram_size / 2; offs += 4)
	{
		int x, y, sprite, colour, multi, fx, fy, inc, flash, mult;

		sprite = state->spriteram[offs + 1] & 0x7fff;
		if (!sprite)
			continue;

		y = state->spriteram[offs];
		flash = y & 0x1000;
		if (flash && (video_screen_get_frame_number(machine->primary_screen) & 1))
			continue;

		x = state->spriteram[offs + 2];
		colour = (x >>9) & 0xf;

		fx = y & 0x2000;
		fy = y & 0x4000;
		multi = (1 << ((y & 0x0600) >> 9)) - 1;	/* 1x, 2x, 4x, 8x height */

		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= 320) x -= 512;
		if (y >= 256) y -= 512;
		y = 240 - y;
		x = 304 - x;

		// sprite &= ~multi; /* Todo:  I bet TumblePop bootleg doesn't do this either */
		if (fy)
			inc = -1;
		else
		{
			sprite += multi;
			inc = 1;
		}

		if (flipscreen)
		{
			y = 240 - y;
			x = 304 - x;
			if (fx) fx = 0; else fx = 1;
			if (fy) fy = 0; else fy = 1;
			mult = 16;
		}
		else
			mult = -16;

		while (multi >= 0)
		{
			drawgfx_transpen(bitmap,cliprect,machine->gfx[0],
					sprite - multi * inc,
					colour,
					fx, fy,
					x - state->xsproff, y - state->ysproff + mult * multi,0);

			multi--;
		}
	}
}

VIDEO_START( crospang )
{
	crospang_state *state = (crospang_state *)machine->driver_data;
	state->bg_layer = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 16, 16, 32, 32);
	state->fg_layer = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 16, 16, 32, 32);

	tilemap_set_transparent_pen(state->fg_layer, 0);
}

VIDEO_UPDATE( crospang )
{
	crospang_state *state = (crospang_state *)screen->machine->driver_data;
	tilemap_draw(bitmap, cliprect, state->bg_layer, 0, 0);
	tilemap_draw(bitmap, cliprect, state->fg_layer, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}
