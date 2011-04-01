/***************************************************************************

    video/aeroboto.c

***************************************************************************/


#include "emu.h"
#include "includes/aeroboto.h"


// how the starfield ROM is interpreted: 0=256x256x1 linear bitmap, 1=8x8x1x1024 tilemap
#define STARS_LAYOUT 1

// scroll speed of the stars: 1=normal, 2=half, 3=one-third...etc.(possitive integers only)
#define SCROLL_SPEED 1

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_tile_info )
{
	aeroboto_state *state = machine.driver_data<aeroboto_state>();
	UINT8 code = state->m_videoram[tile_index];
	SET_TILE_INFO(
			0,
			code + (state->m_charbank << 8),
			state->m_tilecolor[code],
			(state->m_tilecolor[code] >= 0x33) ? 0 : TILE_FORCE_LAYER0);
}
// transparency should only affect tiles with color 0x33 or higher


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( aeroboto )
{
	aeroboto_state *state = machine.driver_data<aeroboto_state>();

	state->m_bg_tilemap = tilemap_create(machine, get_tile_info, tilemap_scan_rows, 8, 8, 32, 64);
	tilemap_set_transparent_pen(state->m_bg_tilemap, 0);
	tilemap_set_scroll_rows(state->m_bg_tilemap, 64);

	state->save_item(NAME(state->m_charbank));
	state->save_item(NAME(state->m_starsoff));
	state->save_item(NAME(state->m_sx));
	state->save_item(NAME(state->m_sy));
	state->save_item(NAME(state->m_ox));
	state->save_item(NAME(state->m_oy));

	#if STARS_LAYOUT
	{
		UINT8 *temp;
		int i;

		temp = auto_alloc_array(machine, UINT8, state->m_stars_length);
		memcpy(temp, state->m_stars_rom, state->m_stars_length);

		for (i = 0; i < state->m_stars_length; i++)
			state->m_stars_rom[(i & ~0xff) + (i << 5 & 0xe0) + (i >> 3 & 0x1f)] = temp[i];

		auto_free(machine, temp);
	}
	#endif
}



/***************************************************************************

  Memory handlers

***************************************************************************/

READ8_HANDLER( aeroboto_in0_r )
{
	return input_port_read(space->machine(), flip_screen_get(space->machine()) ? "P2" : "P1");
}

WRITE8_HANDLER( aeroboto_3000_w )
{
	aeroboto_state *state = space->machine().driver_data<aeroboto_state>();

	/* bit 0 selects both flip screen and player1/player2 controls */
	flip_screen_set(space->machine(), data & 0x01);

	/* bit 1 = char bank select */
	if (state->m_charbank != ((data & 0x02) >> 1))
	{
		tilemap_mark_all_tiles_dirty(state->m_bg_tilemap);
		state->m_charbank = (data & 0x02) >> 1;
	}

	/* bit 2 = disable star field? */
	state->m_starsoff = data & 0x4;
}

WRITE8_HANDLER( aeroboto_videoram_w )
{
	aeroboto_state *state = space->machine().driver_data<aeroboto_state>();

	state->m_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->m_bg_tilemap,offset);
}

WRITE8_HANDLER( aeroboto_tilecolor_w )
{
	aeroboto_state *state = space->machine().driver_data<aeroboto_state>();

	if (state->m_tilecolor[offset] != data)
	{
		state->m_tilecolor[offset] = data;
		tilemap_mark_all_tiles_dirty(state->m_bg_tilemap);
	}
}



/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites( running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	aeroboto_state *state = machine.driver_data<aeroboto_state>();
	int offs;

	for (offs = 0; offs < state->m_spriteram_size; offs += 4)
	{
		int x = state->m_spriteram[offs + 3];
		int y = 240 - state->m_spriteram[offs];

		if (flip_screen_get(machine))
		{
			x = 248 - x;
			y = 240 - y;
		}

		drawgfx_transpen(bitmap, cliprect, machine.gfx[1],
				state->m_spriteram[offs + 1],
				state->m_spriteram[offs + 2] & 0x07,
				flip_screen_get(machine), flip_screen_get(machine),
				((x + 8) & 0xff) - 8, y, 0);
	}
}


SCREEN_UPDATE( aeroboto )
{
	aeroboto_state *state = screen->machine().driver_data<aeroboto_state>();

	static const rectangle splitrect1 = { 0, 255, 0, 39 };
	static const rectangle splitrect2 = { 0, 255, 40, 255 };
	UINT8 *src_base, *src_colptr, *src_rowptr;
	int src_offsx, src_colmask, sky_color, star_color, x, y, i, j, pen;

	sky_color = star_color = *state->m_bgcolor << 2;

	// the star field is supposed to be seen through tile pen 0 when active
	if (!state->m_starsoff)
	{
		if (star_color < 0xd0)
		{
			star_color = 0xd0;
			sky_color = 0;
		}

		star_color += 2;

		bitmap_fill(bitmap, cliprect, sky_color);

		// actual scroll speed is unknown but it can be adjusted by changing the SCROLL_SPEED constant
		state->m_sx += (char)(*state->m_starx - state->m_ox);
		state->m_ox = *state->m_starx;
		x = state->m_sx / SCROLL_SPEED;

		if (*state->m_vscroll != 0xff)
			state->m_sy += (char)(*state->m_stary - state->m_oy);
		state->m_oy = *state->m_stary;
		y = state->m_sy / SCROLL_SPEED;

		src_base = state->m_stars_rom;

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
		state->m_sx = state->m_ox = *state->m_starx;
		state->m_sy = state->m_oy = *state->m_stary;
		bitmap_fill(bitmap, cliprect, sky_color);
	}

	for (y = 0; y < 64; y++)
		tilemap_set_scrollx(state->m_bg_tilemap, y, state->m_hscroll[y]);

	// the playfield is part of a splitscreen and should not overlap with status display
	tilemap_set_scrolly(state->m_bg_tilemap, 0, *state->m_vscroll);
	tilemap_draw(bitmap, &splitrect2, state->m_bg_tilemap, 0, 0);

	draw_sprites(screen->machine(), bitmap, cliprect);

	// the status display behaves more closely to a 40-line splitscreen than an overlay
	tilemap_set_scrolly(state->m_bg_tilemap, 0, 0);
	tilemap_draw(bitmap, &splitrect1, state->m_bg_tilemap, 0, 0);
	return 0;
}
