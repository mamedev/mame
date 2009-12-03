#include "driver.h"
#include "includes/homerun.h"


#define half_screen 116

WRITE8_DEVICE_HANDLER(homerun_banking_w)
{
	homerun_state *state = (homerun_state *)device->machine->driver_data;
	if (video_screen_get_vpos(device->machine->primary_screen) > half_screen)
		state->gc_down = data & 3;
	else
		state->gc_up = data & 3;

  	tilemap_mark_all_tiles_dirty(state->tilemap);

	data >>= 5;
	memory_set_bank(device->machine, "bank1", data & 0x07);
}

WRITE8_HANDLER( homerun_videoram_w )
{
	homerun_state *state = (homerun_state *)space->machine->driver_data;
	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->tilemap, offset & 0xfff);
}

WRITE8_HANDLER(homerun_color_w)
{
	int r, g, b;
	int bit0, bit1, bit2;
	bit0 = (data >> 0) & 0x01;
	bit1 = (data >> 1) & 0x01;
	bit2 = (data >> 2) & 0x01;
	r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
	bit0 = (data >> 3) & 0x01;
	bit1 = (data >> 4) & 0x01;
	bit2 = (data >> 5) & 0x01;
	g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
	bit0 = 0;
	bit1 = (data >> 6) & 0x01;
	bit2 = (data >> 7) & 0x01;
	b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
	palette_set_color(space->machine, offset, MAKE_RGB(r,g,b));
}

static TILE_GET_INFO( get_homerun_tile_info )
{
	homerun_state *state = (homerun_state *)machine->driver_data;
	int tileno = (state->videoram[tile_index]) + ((state->videoram[tile_index + 0x1000] & 0x38) << 5) + ((state->gfx_ctrl & 1) << 11);
	int palno = (state->videoram[tile_index + 0x1000] & 0x07);

	SET_TILE_INFO(0, tileno, palno, 0);
}

VIDEO_START( homerun )
{
	homerun_state *state = (homerun_state *)machine->driver_data;
	state->tilemap = tilemap_create(machine, get_homerun_tile_info, tilemap_scan_rows, 8, 8, 64, 64);
}

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	homerun_state *state = (homerun_state *)machine->driver_data;
	UINT8 *spriteram = state->spriteram;
	int offs;

	for (offs = state->spriteram_size - 4; offs >=0; offs -= 4)
	{
		int code, color, sx, sy, flipx, flipy;
		sx = spriteram[offs + 3];
		sy = spriteram[offs + 0] - 16;
		code = (spriteram[offs + 1]) + ((spriteram[offs + 2] & 0x8) << 5) + (state->gfx_ctrl << 9);
		color = (spriteram[offs + 2] & 0x7) + 8 ;
		flipx=(spriteram[offs + 2] & 0x40) ;
		flipy=(spriteram[offs + 2] & 0x80) ;
		drawgfx_transpen(bitmap, cliprect, machine->gfx[1],
				code,
				color,
				flipx,flipy,
				sx,sy,0);
	}
}

VIDEO_UPDATE(homerun)
{
	homerun_state *state = (homerun_state *)screen->machine->driver_data;
	rectangle myclip = *cliprect;

	/* upper part */
	tilemap_set_scrollx(state->tilemap, 0, state->xpc + ((state->xpa & 2) << 7) );
	tilemap_set_scrolly(state->tilemap, 0, state->xpb + ((state->xpa & 1) << 8) );

	myclip.max_y /= 2;
	state->gfx_ctrl = state->gc_up;
	tilemap_draw(bitmap, &myclip, state->tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap, &myclip);

	/* lower part */
	myclip.min_y += myclip.max_y;
	myclip.max_y *= 2;
	state->gfx_ctrl = state->gc_down;
	tilemap_draw(bitmap, &myclip, state->tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap, &myclip);

	state->gc_down = state->gc_up;
	return 0;
}
