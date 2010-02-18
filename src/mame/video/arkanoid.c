/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/arkanoid.h"


WRITE8_HANDLER( arkanoid_videoram_w )
{
	arkanoid_state *state = (arkanoid_state *)space->machine->driver_data;
	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset / 2);
}

WRITE8_HANDLER( arkanoid_d008_w )
{
	arkanoid_state *state = (arkanoid_state *)space->machine->driver_data;
	int bank;

	/* bits 0 and 1 flip X and Y, I don't know which is which */
	if (flip_screen_x_get(space->machine) != (data & 0x01))
	{
		flip_screen_x_set(space->machine, data & 0x01);
		tilemap_mark_all_tiles_dirty(state->bg_tilemap);
	}

	if (flip_screen_y_get(space->machine) != (data & 0x02))
	{
		flip_screen_y_set(space->machine, data & 0x02);
		tilemap_mark_all_tiles_dirty(state->bg_tilemap);
	}

	/* bit 2 selects the input paddle */
	state->paddle_select = data & 0x04;

	/* bit 3 is coin lockout (but not the service coin) */
	coin_lockout_w(space->machine, 0, !(data & 0x08));
	coin_lockout_w(space->machine, 1, !(data & 0x08));

	/* bit 4 is unknown */

	/* bits 5 and 6 control gfx bank and palette bank. They are used together */
	/* so I don't know which is which. */
	bank = (data & 0x20) >> 5;

	if (state->gfxbank != bank)
	{
		state->gfxbank = bank;
		tilemap_mark_all_tiles_dirty(state->bg_tilemap);
	}

	bank = (data & 0x40) >> 6;

	if (state->palettebank != bank)
	{
		state->palettebank = bank;
		tilemap_mark_all_tiles_dirty(state->bg_tilemap);
	}

	/* BM:  bit 7 is suspected to be MCU reset, the evidence for this is that
     the games tilt mode reset sequence shows the main CPU must be able to
     directly control the reset line of the MCU, else the game will crash
     leaving the tilt screen (as the MCU is now out of sync with main CPU
     which resets itself).  This bit is the likely candidate as it is flipped
     early in bootup just prior to accessing the MCU for the first time. */
	if (state->mcu != NULL)	// Bootlegs don't have the MCU but still set this bit
		cpu_set_input_line(state->mcu, INPUT_LINE_RESET, (data & 0x80) ? CLEAR_LINE : ASSERT_LINE);
}

/* different hook-up, everything except for bits 0-1 and 7 aren't tested afaik. */
WRITE8_HANDLER( tetrsark_d008_w )
{
	arkanoid_state *state = (arkanoid_state *)space->machine->driver_data;
	int bank;

	/* bits 0 and 1 flip X and Y, I don't know which is which */
	if (flip_screen_x_get(space->machine) != (data & 0x01))
	{
		flip_screen_x_set(space->machine, data & 0x01);
		tilemap_mark_all_tiles_dirty(state->bg_tilemap);
	}

	if (flip_screen_y_get(space->machine) != (data & 0x02))
	{
		flip_screen_y_set(space->machine, data & 0x02);
		tilemap_mark_all_tiles_dirty(state->bg_tilemap);
	}

	/* bit 2 selects the input paddle? */
	state->paddle_select = data & 0x04;

	/* bit 3-4 is unknown? */

	/* bits 5 and 6 control gfx bank and palette bank. They are used together */
	/* so I don't know which is which.? */
	bank = (data & 0x20) >> 5;

	if (state->gfxbank != bank)
	{
		state->gfxbank = bank;
		tilemap_mark_all_tiles_dirty(state->bg_tilemap);
	}

	bank = (data & 0x40) >> 6;

	if (state->palettebank != bank)
	{
		state->palettebank = bank;
		tilemap_mark_all_tiles_dirty(state->bg_tilemap);
	}

	/* bit 7 is coin lockout (but not the service coin) */
	coin_lockout_w(space->machine, 0, !(data & 0x80));
	coin_lockout_w(space->machine, 1, !(data & 0x80));
}


WRITE8_HANDLER( hexa_d008_w )
{
	arkanoid_state *state = (arkanoid_state *)space->machine->driver_data;

	/* bit 0 = flipx (or y?) */
	if (flip_screen_x_get(space->machine) != (data & 0x01))
	{
		flip_screen_x_set(space->machine, data & 0x01);
		tilemap_mark_all_tiles_dirty(state->bg_tilemap);
	}

	/* bit 1 = flipy (or x?) */
	if (flip_screen_y_get(space->machine) != (data & 0x02))
	{
		flip_screen_y_set(space->machine, data & 0x02);
		tilemap_mark_all_tiles_dirty(state->bg_tilemap);
	}

	/* bit 2 - 3 unknown */

	/* bit 4 could be the ROM bank selector for 8000-bfff (not sure) */
	memory_set_bank(space->machine, "bank1", ((data & 0x10) >> 4));

	/* bit 5 = gfx bank */
	if (state->gfxbank != ((data & 0x20) >> 5))
	{
		state->gfxbank = (data & 0x20) >> 5;
		tilemap_mark_all_tiles_dirty(state->bg_tilemap);
	}

	/* bit 6 - 7 unknown */
}

static TILE_GET_INFO( get_bg_tile_info )
{
	arkanoid_state *state = (arkanoid_state *)machine->driver_data;
	int offs = tile_index * 2;
	int code = state->videoram[offs + 1] + ((state->videoram[offs] & 0x07) << 8) + 2048 * state->gfxbank;
	int color = ((state->videoram[offs] & 0xf8) >> 3) + 32 * state->palettebank;

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( arkanoid )
{
	arkanoid_state *state = (arkanoid_state *)machine->driver_data;

	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
}

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	arkanoid_state *state = (arkanoid_state *)machine->driver_data;
	int offs;

	for (offs = 0; offs < state->spriteram_size; offs += 4)
	{
		int sx, sy, code;

		sx = state->spriteram[offs];
		sy = 248 - state->spriteram[offs + 1];
		if (flip_screen_x_get(machine))
			sx = 248 - sx;
		if (flip_screen_y_get(machine))
			sy = 248 - sy;

		code = state->spriteram[offs + 3] + ((state->spriteram[offs + 2] & 0x03) << 8) + 1024 * state->gfxbank;

		drawgfx_transpen(bitmap,cliprect,machine->gfx[0],
				2 * code,
				((state->spriteram[offs + 2] & 0xf8) >> 3) + 32 * state->palettebank,
				flip_screen_x_get(machine),flip_screen_y_get(machine),
				sx,sy + (flip_screen_y_get(machine) ? 8 : -8),0);
		drawgfx_transpen(bitmap,cliprect,machine->gfx[0],
				2 * code + 1,
				((state->spriteram[offs + 2] & 0xf8) >> 3) + 32 * state->palettebank,
				flip_screen_x_get(machine),flip_screen_y_get(machine),
				sx,sy,0);
	}
}


VIDEO_UPDATE( arkanoid )
{
	arkanoid_state *state = (arkanoid_state *)screen->machine->driver_data;

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}

VIDEO_UPDATE( hexa )
{
	arkanoid_state *state = (arkanoid_state *)screen->machine->driver_data;

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	return 0;
}
