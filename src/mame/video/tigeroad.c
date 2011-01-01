#include "emu.h"
#include "includes/tigeroad.h"

static int bgcharbank;
static tilemap_t *bg_tilemap, *fg_tilemap;

WRITE16_HANDLER( tigeroad_videoram_w )
{
	tigeroad_state *state = space->machine->driver_data<tigeroad_state>();
	UINT16 *videoram = state->videoram;
	COMBINE_DATA(&videoram[offset]);
	tilemap_mark_tile_dirty(fg_tilemap, offset);
}

WRITE16_HANDLER( tigeroad_videoctrl_w )
{
	int bank;

	if (ACCESSING_BITS_8_15)
	{
		data = (data >> 8) & 0xff;

		/* bit 1 flips screen */

		if (flip_screen_get(space->machine) != (data & 0x02))
		{
			flip_screen_set(space->machine, data & 0x02);
			tilemap_mark_all_tiles_dirty_all(space->machine);
		}

		/* bit 2 selects bg char bank */

		bank = (data & 0x04) >> 2;

		if (bgcharbank != bank)
		{
			bgcharbank = bank;
			tilemap_mark_all_tiles_dirty(bg_tilemap);
		}

		/* bits 4-5 are coin lockouts */

		coin_lockout_w(space->machine, 0, !(data & 0x10));
		coin_lockout_w(space->machine, 1, !(data & 0x20));

		/* bits 6-7 are coin counters */

		coin_counter_w(space->machine, 0, data & 0x40);
		coin_counter_w(space->machine, 1, data & 0x80);
	}
}

WRITE16_HANDLER( tigeroad_scroll_w )
{
	int scroll = 0;

	COMBINE_DATA(&scroll);

	switch (offset)
	{
	case 0:
		tilemap_set_scrollx(bg_tilemap, 0, scroll);
		break;
	case 1:
		tilemap_set_scrolly(bg_tilemap, 0, -scroll - 32 * 8);
		break;
	}
}

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int priority )
{
	UINT16 *source = &machine->generic.buffered_spriteram.u16[machine->generic.spriteram_size/2] - 4;
	UINT16 *finish = machine->generic.buffered_spriteram.u16;

	// TODO: The Track Map should probably be drawn on top of the background tilemap...
	//       Also convert the below into a for loop!

	while (source >= finish)
	{
		int tile_number = source[0];

		if (tile_number != 0xfff) {
			int attr = source[1];
			int sy = source[2] & 0x1ff;
			int sx = source[3] & 0x1ff;

			int flipx = attr & 0x02;
			int flipy = attr & 0x01;
			int color = (attr >> 2) & 0x0f;

			if (sx > 0x100) sx -= 0x200;
			if (sy > 0x100) sy -= 0x200;

			if (flip_screen_get(machine))
			{
				sx = 240 - sx;
				sy = 240 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			drawgfx_transpen(bitmap, cliprect,
				machine->gfx[2],
				tile_number,
				color,
				flipx, flipy,
				sx, 240 - sy, 15);
		}

		source -= 4;
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	UINT8 *tilerom = machine->region("gfx4")->base();

	int data = tilerom[tile_index];
	int attr = tilerom[tile_index + 1];
	int code = data + ((attr & 0xc0) << 2) + (bgcharbank << 10);
	int color = attr & 0x0f;
	int flags = (attr & 0x20) ? TILE_FLIPX : 0;

	SET_TILE_INFO(1, code, color, flags);
	tileinfo->group = (attr & 0x10) ? 1 : 0;
}

static TILE_GET_INFO( get_fg_tile_info )
{
	tigeroad_state *state = machine->driver_data<tigeroad_state>();
	UINT16 *videoram = state->videoram;
	int data = videoram[tile_index];
	int attr = data >> 8;
	int code = (data & 0xff) + ((attr & 0xc0) << 2) + ((attr & 0x20) << 5);
	int color = attr & 0x0f;
	int flags = (attr & 0x10) ? TILE_FLIPY : 0;

	SET_TILE_INFO(0, code, color, flags);
}

static TILEMAP_MAPPER( tigeroad_tilemap_scan )
{
	/* logical (col,row) -> memory offset */
	return 2 * (col % 8) + 16 * ((127 - row) % 8) + 128 * (col / 8) + 2048 * ((127 - row) / 8);
}

VIDEO_START( tigeroad )
{
	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tigeroad_tilemap_scan,
		 32, 32, 128, 128);

	fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows,
		 8, 8, 32, 32);

	tilemap_set_transmask(bg_tilemap, 0, 0xffff, 0);
	tilemap_set_transmask(bg_tilemap, 1, 0x1ff, 0xfe00);

	tilemap_set_transparent_pen(fg_tilemap, 3);
}

VIDEO_UPDATE( tigeroad )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, TILEMAP_DRAW_LAYER1, 0);
	draw_sprites(screen->machine, bitmap, cliprect, 0);
	tilemap_draw(bitmap, cliprect, bg_tilemap, TILEMAP_DRAW_LAYER0, 1);
	//draw_sprites(screen->machine, bitmap, cliprect, 1); draw priority sprites?
	tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 2);
	return 0;
}

VIDEO_EOF( tigeroad )
{
	address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);

	buffer_spriteram16_w(space, 0, 0, 0xffff);
}
