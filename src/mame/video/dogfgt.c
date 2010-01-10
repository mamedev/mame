#include "emu.h"
#include "includes/dogfgt.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Dog-Fight has both palette RAM and PROMs. The PROMs are used for tiles &
  pixmap, RAM for sprites.

***************************************************************************/

PALETTE_INIT( dogfgt )
{
	int i;

	/* first 16 colors are RAM */
	for (i = 0; i < 64; i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */
		bit0 = (*color_prom >> 0) & 0x01;
		bit1 = (*color_prom >> 1) & 0x01;
		bit2 = (*color_prom >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (*color_prom >> 3) & 0x01;
		bit1 = (*color_prom >> 4) & 0x01;
		bit2 = (*color_prom >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = (*color_prom >> 6) & 0x01;
		bit2 = (*color_prom >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine, i + 16, MAKE_RGB(r,g,b));
		color_prom++;
	}
}


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_tile_info )
{
	dogfgt_state *state = (dogfgt_state *)machine->driver_data;
	SET_TILE_INFO(
			0,
			state->bgvideoram[tile_index],
			state->bgvideoram[tile_index + 0x400] & 0x03,
			0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( dogfgt )
{
	dogfgt_state *state = (dogfgt_state *)machine->driver_data;
	state->bg_tilemap = tilemap_create(machine, get_tile_info, tilemap_scan_rows, 16, 16, 32, 32);

	state->bitmapram = auto_alloc_array(machine, UINT8, BITMAPRAM_SIZE);
	state_save_register_global_pointer(machine, state->bitmapram, BITMAPRAM_SIZE);

	state->pixbitmap = video_screen_auto_bitmap_alloc(machine->primary_screen);
	state_save_register_global_bitmap(machine, state->pixbitmap);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( dogfgt_plane_select_w )
{
	dogfgt_state *state = (dogfgt_state *)space->machine->driver_data;

	state->bm_plane = data;
}

READ8_HANDLER( dogfgt_bitmapram_r )
{
	dogfgt_state *state = (dogfgt_state *)space->machine->driver_data;

	if (state->bm_plane > 2)
	{
		popmessage("bitmapram_r offs %04x plane %d\n", offset, state->bm_plane);
		return 0;
	}

	return state->bitmapram[offset + BITMAPRAM_SIZE / 3 * state->bm_plane];
}

static WRITE8_HANDLER( internal_bitmapram_w )
{
	dogfgt_state *state = (dogfgt_state *)space->machine->driver_data;
	int x, y, subx;

	state->bitmapram[offset] = data;

	offset &= (BITMAPRAM_SIZE / 3 - 1);
	x = 8 * (offset / 256);
	y = offset % 256;

	for (subx = 0; subx < 8; subx++)
	{
		int i, color = 0;

		for (i = 0; i < 3; i++)
			color |= ((state->bitmapram[offset + BITMAPRAM_SIZE / 3 * i] >> subx) & 1) << i;

		if (flip_screen_get(space->machine))
			*BITMAP_ADDR16(state->pixbitmap, y ^ 0xff, (x + subx) ^ 0xff) = PIXMAP_COLOR_BASE + 8 * state->pixcolor + color;
		else
			*BITMAP_ADDR16(state->pixbitmap, y, x + subx) = PIXMAP_COLOR_BASE + 8 * state->pixcolor + color;
	}
}

WRITE8_HANDLER( dogfgt_bitmapram_w )
{
	dogfgt_state *state = (dogfgt_state *)space->machine->driver_data;

	if (state->bm_plane > 2)
	{
		popmessage("bitmapram_w offs %04x plane %d\n", offset, state->bm_plane);
		return;
	}

	internal_bitmapram_w(space, offset + BITMAPRAM_SIZE / 3 * state->bm_plane, data);
}

WRITE8_HANDLER( dogfgt_bgvideoram_w )
{
	dogfgt_state *state = (dogfgt_state *)space->machine->driver_data;

	state->bgvideoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset & 0x3ff);
}

WRITE8_HANDLER( dogfgt_scroll_w )
{
	dogfgt_state *state = (dogfgt_state *)space->machine->driver_data;

	state->scroll[offset] = data;
	tilemap_set_scrollx(state->bg_tilemap, 0, state->scroll[0] + 256 * state->scroll[1] + 256);
	tilemap_set_scrolly(state->bg_tilemap, 0, state->scroll[2] + 256 * state->scroll[3]);
}

WRITE8_HANDLER( dogfgt_1800_w )
{
	dogfgt_state *state = (dogfgt_state *)space->machine->driver_data;

	/* bits 0 and 1 are probably text color (not verified because PROM is missing) */
	state->pixcolor = ((data & 0x01) << 1) | ((data & 0x02) >> 1);

	/* bits 4 and 5 are coin counters */
	coin_counter_w(space->machine, 0, data & 0x10);
	coin_counter_w(space->machine, 1, data & 0x20);

	/* bit 7 is screen flip */
	flip_screen_set(space->machine, data & 0x80);

	/* other bits unused? */
	logerror("PC %04x: 1800 = %02x\n", cpu_get_pc(space->cpu), data);
}


/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites( running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect )
{
	dogfgt_state *state = (dogfgt_state *)machine->driver_data;
	int offs;

	for (offs = 0; offs < state->spriteram_size; offs += 4)
	{
		if (state->spriteram[offs] & 0x01)
		{
			int sx, sy, flipx, flipy;

			sx = state->spriteram[offs + 3];
			sy = (240 - state->spriteram[offs + 2]) & 0xff;
			flipx = state->spriteram[offs] & 0x04;
			flipy = state->spriteram[offs] & 0x02;
			if (flip_screen_get(machine))
			{
				sx = 240 - sx;
				sy = 240 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
					state->spriteram[offs + 1] + ((state->spriteram[offs] & 0x30) << 4),
					(state->spriteram[offs] & 0x08) >> 3,
					flipx,flipy,
					sx,sy,0);
		}
	}
}


VIDEO_UPDATE( dogfgt )
{
	dogfgt_state *state = (dogfgt_state *)screen->machine->driver_data;
	int offs;

	if (state->lastflip != flip_screen_get(screen->machine) || state->lastpixcolor != state->pixcolor)
	{
		const address_space *space = cputag_get_address_space(screen->machine, "maincpu", ADDRESS_SPACE_PROGRAM);

		state->lastflip = flip_screen_get(screen->machine);
		state->lastpixcolor = state->pixcolor;

		for (offs = 0; offs < BITMAPRAM_SIZE; offs++)
			internal_bitmapram_w(space, offs, state->bitmapram[offs]);
	}


	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);

	draw_sprites(screen->machine, bitmap, cliprect);

	copybitmap_trans(bitmap, state->pixbitmap, 0, 0, 0, 0, cliprect, PIXMAP_COLOR_BASE + 8 * state->pixcolor);
	return 0;
}
