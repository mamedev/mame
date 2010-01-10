/* Fit of Fighting Video Hardware */

#include "emu.h"
#include "includes/fitfight.h"


static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int layer )
{
	fitfight_state *state = (fitfight_state *)machine->driver_data;
	const gfx_element *gfx = machine->gfx[3];
	UINT16 *source = state->spriteram;
	UINT16 *finish = source + 0x800 / 2;

	while (source < finish)
	{
		int xpos, ypos, number, xflip, yflip, end, colr, prio;

		ypos = source[0];
		xpos = source[3];
		number = source[2];
		xflip = (source[1] & 0x0001) ^ 0x0001;
		yflip = (source[1] & 0x0002);
		prio = (source[1] & 0x0400) >> 10;
		colr = (source[1] & 0x00fc) >> 2;

		if (state->bbprot_kludge == 1)
			colr = (source[1] & 0x00f8) >> 3;

		end = source[0] & 0x8000;

		ypos = 0xff - ypos;

		xpos -= 38;//48;
		ypos -= 14;//16;

		if (end) break;
		if (prio == layer)
			drawgfx_transpen(bitmap, cliprect, gfx, number, colr, xflip, yflip, xpos, ypos, 0);

		source += 4;
	}
}

static TILE_GET_INFO( get_fof_bak_tile_info )
{
	fitfight_state *state = (fitfight_state *)machine->driver_data;
	int code = state->fof_bak_tileram[tile_index * 2 + 1];
	int colr = state->fof_bak_tileram[tile_index * 2] & 0x1f;
	int xflip = (state->fof_bak_tileram[tile_index * 2] & 0x0020) >> 5;
	xflip ^= 1;

	SET_TILE_INFO(2, code, colr, TILE_FLIPYX(xflip));
}

WRITE16_HANDLER(  fof_bak_tileram_w )
{
	fitfight_state *state = (fitfight_state *)space->machine->driver_data;

	COMBINE_DATA(&state->fof_bak_tileram[offset]);
	tilemap_mark_tile_dirty(state->fof_bak_tilemap, offset / 2);
}


static TILE_GET_INFO( get_fof_mid_tile_info )
{
	fitfight_state *state = (fitfight_state *)machine->driver_data;
	int code = state->fof_mid_tileram[tile_index * 2 + 1];
	int colr = state->fof_mid_tileram[tile_index * 2] & 0x1f;
	int xflip = (state->fof_mid_tileram[tile_index * 2] & 0x0020) >> 5;
	xflip ^= 1;

	SET_TILE_INFO(1, code, colr, TILE_FLIPYX(xflip));
}

WRITE16_HANDLER( fof_mid_tileram_w )
{
	fitfight_state *state = (fitfight_state *)space->machine->driver_data;

	COMBINE_DATA(&state->fof_mid_tileram[offset]);
	tilemap_mark_tile_dirty(state->fof_mid_tilemap, offset / 2);
}

static TILE_GET_INFO( get_fof_txt_tile_info )
{
	fitfight_state *state = (fitfight_state *)machine->driver_data;
	int code = state->fof_txt_tileram[tile_index * 2 + 1];
	int colr = state->fof_txt_tileram[tile_index * 2] & 0x1f;
	int xflip = (state->fof_txt_tileram[tile_index * 2] & 0x0020) >> 5;
	xflip ^= 1;

	SET_TILE_INFO(0, code, colr, TILE_FLIPYX(xflip));
}

WRITE16_HANDLER( fof_txt_tileram_w )
{
	fitfight_state *state = (fitfight_state *)space->machine->driver_data;

	COMBINE_DATA(&state->fof_txt_tileram[offset]);
	tilemap_mark_tile_dirty(state->fof_txt_tilemap, offset / 2);
}

/* video start / update */

VIDEO_START(fitfight)
{
	fitfight_state *state = (fitfight_state *)machine->driver_data;
	state->fof_bak_tilemap = tilemap_create(machine, get_fof_bak_tile_info, tilemap_scan_cols, 8, 8, 128, 32);
	/* opaque */

	state->fof_mid_tilemap = tilemap_create(machine, get_fof_mid_tile_info, tilemap_scan_cols, 8, 8, 128, 32);
	tilemap_set_transparent_pen(state->fof_mid_tilemap, 0);

	state->fof_txt_tilemap = tilemap_create(machine, get_fof_txt_tile_info, tilemap_scan_cols, 8, 8, 128, 32);
	tilemap_set_transparent_pen(state->fof_txt_tilemap, 0);
}

VIDEO_UPDATE(fitfight)
{
	fitfight_state *state = (fitfight_state *)screen->machine->driver_data;

	/* scroll isn't right */

	int vblank;
	int scrollbak, scrollmid;

	vblank = (state->fof_700000[0] & 0x8000);

	if (vblank > 0)
		bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));
	else {
//      if (input_code_pressed(screen->machine, KEYCODE_Q))
//          scrollbak = ((state->fof_a00000[0] & 0xff00) >> 5) - ((state->fof_700000[0] & 0x0038) >> 3);
//      else if (input_code_pressed(screen->machine, KEYCODE_W))
//          scrollbak = ((state->fof_a00000[0] & 0xff00) >> 5) + ((state->fof_700000[0] & 0x01c0) >> 6);
//      else if (input_code_pressed(screen->machine, KEYCODE_E))
//          scrollbak = ((state->fof_a00000[0] & 0xff00) >> 5) - ((state->fof_700000[0] & 0x01c0) >> 6);
//      else if (input_code_pressed(screen->machine, KEYCODE_R))
//          scrollbak = ((state->fof_a00000[0] & 0xff00) >> 5) + ((state->fof_700000[0] & 0x0038) >> 3);
//      else
		scrollbak = ((state->fof_a00000[0] & 0xff00) >> 5);
		tilemap_set_scrollx(state->fof_bak_tilemap,0, scrollbak );
		tilemap_set_scrolly(state->fof_bak_tilemap,0, state->fof_a00000[0] & 0xff);
		tilemap_draw(bitmap, cliprect, state->fof_bak_tilemap, 0, 0);

		draw_sprites(screen->machine, bitmap, cliprect, 0);

//      if (input_code_pressed(screen->machine, KEYCODE_A))
//          scrollmid = ((state->fof_900000[0] & 0xff00) >> 5) - ((state->fof_700000[0] & 0x01c0) >> 6);
//      else if (input_code_pressed(screen->machine, KEYCODE_S))
//          scrollmid = ((state->fof_900000[0] & 0xff00) >> 5) + ((state->fof_700000[0] & 0x0038) >> 3);
//      else if (input_code_pressed(screen->machine, KEYCODE_D))
//          scrollmid = ((state->fof_900000[0] & 0xff00) >> 5) - ((state->fof_700000[0] & 0x0038) >> 3);
//      else if (input_code_pressed(screen->machine, KEYCODE_F))
//          scrollmid = ((state->fof_900000[0] & 0xff00) >> 5) + ((state->fof_700000[0] & 0x01c0) >> 6);
//      else
		scrollmid = ((state->fof_900000[0] & 0xff00) >> 5);
		tilemap_set_scrollx(state->fof_mid_tilemap, 0, scrollmid );
		tilemap_set_scrolly(state->fof_mid_tilemap, 0, state->fof_900000[0] & 0xff);
//      if (!input_code_pressed(screen->machine, KEYCODE_F))
		tilemap_draw(bitmap, cliprect, state->fof_mid_tilemap, 0, 0);

		draw_sprites(screen->machine, bitmap, cliprect, 1);

		tilemap_draw(bitmap, cliprect, state->fof_txt_tilemap, 0, 0);
	}
/*  popmessage ("Regs %04x %04x %04x %04x %04x %04x",
            state->fof_100000[0], state->fof_600000[0], state->fof_700000[0],
            state->fof_800000[0], state->fof_900000[0],
            state->fof_a00000[0] );
*/
	return 0;
}
