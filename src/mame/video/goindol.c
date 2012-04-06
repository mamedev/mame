/***************************************************************************
  Goindol

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/goindol.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_fg_tile_info )
{
	goindol_state *state = machine.driver_data<goindol_state>();
	int code = state->m_fg_videoram[2 * tile_index + 1];
	int attr = state->m_fg_videoram[2 * tile_index];
	SET_TILE_INFO(
			0,
			code | ((attr & 0x7) << 8) | (state->m_char_bank << 11),
			(attr & 0xf8) >> 3,
			0);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	goindol_state *state = machine.driver_data<goindol_state>();
	int code = state->m_bg_videoram[2 * tile_index + 1];
	int attr = state->m_bg_videoram[2 * tile_index];
	SET_TILE_INFO(
			1,
			code | ((attr & 0x7) << 8) | (state->m_char_bank << 11),
			(attr & 0xf8) >> 3,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( goindol )
{
	goindol_state *state = machine.driver_data<goindol_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);

	state->m_fg_tilemap->set_transparent_pen(0);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_MEMBER(goindol_state::goindol_fg_videoram_w)
{
	m_fg_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset / 2);
}

WRITE8_MEMBER(goindol_state::goindol_bg_videoram_w)
{
	m_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}



/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int gfxbank, UINT8 *sprite_ram )
{
	goindol_state *state = machine.driver_data<goindol_state>();
	int offs, sx, sy, tile, palette;

	for (offs = 0; offs < state->m_spriteram_size; offs += 4)
	{
		sx = sprite_ram[offs];
		sy = 240 - sprite_ram[offs + 1];

		if (flip_screen_get(machine))
		{
			sx = 248 - sx;
			sy = 248 - sy;
		}

		if ((sprite_ram[offs + 1] >> 3) && (sx < 248))
		{
			tile = ((sprite_ram[offs + 3]) + ((sprite_ram[offs + 2] & 7) << 8));
			tile += tile;
			palette = sprite_ram[offs + 2] >> 3;

			drawgfx_transpen(bitmap,cliprect,
						machine.gfx[gfxbank],
						tile,
						palette,
						flip_screen_get(machine),flip_screen_get(machine),
						sx,sy, 0);
			drawgfx_transpen(bitmap,cliprect,
						machine.gfx[gfxbank],
						tile+1,
						palette,
						flip_screen_get(machine),flip_screen_get(machine),
						sx,sy + (flip_screen_get(machine) ? -8 : 8), 0);
		}
	}
}

SCREEN_UPDATE_IND16( goindol )
{
	goindol_state *state = screen.machine().driver_data<goindol_state>();
	state->m_fg_tilemap->set_scrollx(0, *state->m_fg_scrollx);
	state->m_fg_tilemap->set_scrolly(0, *state->m_fg_scrolly);

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_sprites(screen.machine(), bitmap, cliprect, 1, state->m_spriteram);
	draw_sprites(screen.machine(), bitmap, cliprect, 0, state->m_spriteram2);
	return 0;
}
