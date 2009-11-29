/***************************************************************************

    video/aeroboto.c

***************************************************************************/


#include "driver.h"
#include "aeroboto.h"


// how the starfield ROM is interpreted: 0=256x256x1 linear bitmap, 1=8x8x1x1024 tilemap
#define STARS_LAYOUT 1

// scroll speed of the stars: 1=normal, 2=half, 3=one-third...etc.(possitive integers only)
#define SCROLL_SPEED 1

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_tile_info )
{
	aeroboto_state *state = (aeroboto_state *)machine->driver_data;
	UINT8 code = state->videoram[tile_index];
	SET_TILE_INFO(
			0,
			code + (state->charbank << 8),
			state->tilecolor[code],
			(state->tilecolor[code] >= 0x33) ? 0 : TILE_FORCE_LAYER0);
}
// transparency should only affect tiles with color 0x33 or higher


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( aeroboto )
{
	aeroboto_state *state = (aeroboto_state *)machine->driver_data;

	state->bg_tilemap = tilemap_create(machine, get_tile_info, tilemap_scan_rows, 8, 8, 32, 64);
	tilemap_set_transparent_pen(state->bg_tilemap, 0);
	tilemap_set_scroll_rows(state->bg_tilemap, 64);

	state_save_register_global(machine, state->charbank);
	state_save_register_global(machine, state->starsoff);
	state_save_register_global(machine, state->sx);
	state_save_register_global(machine, state->sy);
	state_save_register_global(machine, state->ox);
	state_save_register_global(machine, state->oy);

	#if STARS_LAYOUT
	{
		UINT8 *temp;
		int i;

		temp = alloc_array_or_die(UINT8, state->stars_length);
		memcpy(temp, state->stars_rom, state->stars_length);

		for (i = 0; i < state->stars_length; i++)
			state->stars_rom[(i & ~0xff) + (i << 5 & 0xe0) + (i >> 3 & 0x1f)] = temp[i];

		free(temp);
	}
	#endif
}



/***************************************************************************

  Memory handlers

***************************************************************************/

READ8_HANDLER( aeroboto_in0_r )
{
	return input_port_read(space->machine, flip_screen_get(space->machine) ? "P2" : "P1");
}

WRITE8_HANDLER( aeroboto_3000_w )
{
	aeroboto_state *state = (aeroboto_state *)space->machine->driver_data;

	/* bit 0 selects both flip screen and player1/player2 controls */
	flip_screen_set(space->machine, data & 0x01);

	/* bit 1 = char bank select */
	if (state->charbank != ((data & 0x02) >> 1))
	{
		tilemap_mark_all_tiles_dirty(state->bg_tilemap);
		state->charbank = (data & 0x02) >> 1;
	}

	/* bit 2 = disable star field? */
	state->starsoff = data & 0x4;
}

WRITE8_HANDLER( aeroboto_videoram_w )
{
	aeroboto_state *state = (aeroboto_state *)space->machine->driver_data;

	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap,offset);
}

WRITE8_HANDLER( aeroboto_tilecolor_w )
{
	aeroboto_state *state = (aeroboto_state *)space->machine->driver_data;

	if (state->tilecolor[offset] != data)
	{
		state->tilecolor[offset] = data;
		tilemap_mark_all_tiles_dirty(state->bg_tilemap);
	}
}



/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	aeroboto_state *state = (aeroboto_state *)machine->driver_data;
	int offs;

	for (offs = 0; offs < state->spriteram_size; offs += 4)
	{
		int x = state->spriteram[offs + 3];
		int y = 240 - state->spriteram[offs];

		if (flip_screen_get(machine))
		{
			x = 248 - x;
			y = 240 - y;
		}

		drawgfx_transpen(bitmap, cliprect, machine->gfx[1],
				state->spriteram[offs + 1],
				state->spriteram[offs + 2] & 0x07,
				flip_screen_get(machine), flip_screen_get(machine),
				((x + 8) & 0xff) - 8, y, 0);
	}
}


VIDEO_UPDATE( aeroboto )
{
	aeroboto_state *state = (aeroboto_state *)screen->machine->driver_data;

	static const rectangle splitrect1 = { 0, 255, 0, 39 };
	static const rectangle splitrect2 = { 0, 255, 40, 255 };
	UINT8 *src_base, *src_colptr, *src_rowptr;
	int src_offsx, src_colmask, sky_color, star_color, x, y, i, j, pen;

	sky_color = star_color = *state->bgcolor << 2;

	// the star field is supposed to be seen through tile pen 0 when active
	if (!state->starsoff)
	{
		if (star_color < 0xd0)
		{
			star_color = 0xd0;
			sky_color = 0;
		}

		star_color += 2;

		bitmap_fill(bitmap, cliprect, sky_color);

		// actual scroll speed is unknown but it can be adjusted by changing the SCROLL_SPEED constant
		state->sx += (char)(*state->starx - state->ox);
		state->ox = *state->starx;
		x = state->sx / SCROLL_SPEED;

		if (*state->vscroll != 0xff)
			state->sy += (char)(*state->stary - state->oy);
		state->oy = *state->stary;
		y = state->sy / SCROLL_SPEED;

		src_base = state->stars_rom;

		for (i = 0; i < 256; i++)
		{
			src_offsx = (x + i) & 0xff;
			src_colmask = 1 << (src_offsx & 7);
			src_offsx >>= 3;
			src_colptr = src_base + src_offsx;
			pen = star_color + ((i + 8) >> 4 & 1);

			for (j = 0; j < 256; j++)
			{
				src_rowptr = src_colptr + (((y + j) & 0xff) << 5 );
				if (!((unsigned)*src_rowptr & src_colmask))
					*BITMAP_ADDR16(bitmap, j, i) = pen;
			}
		}
	}
	else
	{
		state->sx = state->ox = *state->starx;
		state->sy = state->oy = *state->stary;
		bitmap_fill(bitmap, cliprect, sky_color);
	}

	for (y = 0; y < 64; y++)
		tilemap_set_scrollx(state->bg_tilemap, y, state->hscroll[y]);

	// the playfield is part of a splitscreen and should not overlap with status display
	tilemap_set_scrolly(state->bg_tilemap, 0, *state->vscroll);
	tilemap_draw(bitmap, &splitrect2, state->bg_tilemap, 0, 0);

	draw_sprites(screen->machine, bitmap, cliprect);

	// the status display behaves more closely to a 40-line splitscreen than an overlay
	tilemap_set_scrolly(state->bg_tilemap, 0, 0);
	tilemap_draw(bitmap, &splitrect1, state->bg_tilemap, 0, 0);
	return 0;
}
