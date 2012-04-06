/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/sauro.h"

/* General */

WRITE8_MEMBER(sauro_state::tecfri_videoram_w)
{

	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(sauro_state::tecfri_colorram_w)
{

	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(sauro_state::tecfri_videoram2_w)
{

	m_videoram2[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(sauro_state::tecfri_colorram2_w)
{

	m_colorram2[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(sauro_state::tecfri_scroll_bg_w)
{

	m_bg_tilemap->set_scrollx(0, data);
}

static TILE_GET_INFO( get_tile_info_bg )
{
	sauro_state *state = machine.driver_data<sauro_state>();
	int code = state->m_videoram[tile_index] + ((state->m_colorram[tile_index] & 0x07) << 8);
	int color = ((state->m_colorram[tile_index] >> 4) & 0x0f) | state->m_palette_bank;
	int flags = state->m_colorram[tile_index] & 0x08 ? TILE_FLIPX : 0;

	SET_TILE_INFO(0, code, color, flags);
}

static TILE_GET_INFO( get_tile_info_fg )
{
	sauro_state *state = machine.driver_data<sauro_state>();
	int code = state->m_videoram2[tile_index] + ((state->m_colorram2[tile_index] & 0x07) << 8);
	int color = ((state->m_colorram2[tile_index] >> 4) & 0x0f) | state->m_palette_bank;
	int flags = state->m_colorram2[tile_index] & 0x08 ? TILE_FLIPX : 0;

	SET_TILE_INFO(1, code, color, flags);
}

/* Sauro */

static const int scroll2_map[8] = {2, 1, 4, 3, 6, 5, 0, 7};
static const int scroll2_map_flip[8] = {0, 7, 2, 1, 4, 3, 6, 5};

WRITE8_MEMBER(sauro_state::sauro_palette_bank_w)
{

	m_palette_bank = (data & 0x03) << 4;
	machine().tilemap().mark_all_dirty();
}

WRITE8_MEMBER(sauro_state::sauro_scroll_fg_w)
{
	const int *map = (flip_screen_get(machine()) ? scroll2_map_flip : scroll2_map);
	int scroll = (data & 0xf8) | map[data & 7];

	m_fg_tilemap->set_scrollx(0, scroll);
}

VIDEO_START( sauro )
{
	sauro_state *state = machine.driver_data<sauro_state>();

	state->m_bg_tilemap = tilemap_create(machine, get_tile_info_bg, tilemap_scan_cols,
		 8, 8, 32, 32);

	state->m_fg_tilemap = tilemap_create(machine, get_tile_info_fg, tilemap_scan_cols,
		 8, 8, 32, 32);

	state->m_fg_tilemap->set_transparent_pen(0);
	state->m_palette_bank = 0;
}

static void sauro_draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	sauro_state *state = machine.driver_data<sauro_state>();
	UINT8 *spriteram = state->m_spriteram;
	int offs,code,sx,sy,color,flipx;
	int flipy = flip_screen_get(machine);

	for (offs = 3; offs < state->m_spriteram_size - 1; offs += 4)
	{
		sy = spriteram[offs];
		if (sy == 0xf8) continue;

		code = spriteram[offs+1] + ((spriteram[offs+3] & 0x03) << 8);
		sx = spriteram[offs+2];
		sy = 236 - sy;
		color = ((spriteram[offs+3] >> 4) & 0x0f) | state->m_palette_bank;

		// I'm not really sure how this bit works
		if (spriteram[offs+3] & 0x08)
		{
			if (sx > 0xc0)
			{
				// Sign extend
				sx = (signed int)(signed char)sx;
			}
		}
		else
		{
			if (sx < 0x40) continue;
		}

		flipx = spriteram[offs+3] & 0x04;

		if (flipy)
		{
			flipx = !flipx;
			sx = (235 - sx) & 0xff;  // The &0xff is not 100% percent correct
			sy = 240 - sy;
		}

		drawgfx_transpen(bitmap, cliprect, machine.gfx[2],
				code,
				color,
				flipx, flipy,
				sx,sy,0);
	}
}

SCREEN_UPDATE_IND16( sauro )
{
	sauro_state *state = screen.machine().driver_data<sauro_state>();

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);
	sauro_draw_sprites(screen.machine(), bitmap, cliprect);
	return 0;
}

/* Tricky Doc */

VIDEO_START( trckydoc )
{
	sauro_state *state = machine.driver_data<sauro_state>();

	state->m_bg_tilemap = tilemap_create(machine, get_tile_info_bg, tilemap_scan_cols,
		 8, 8, 32, 32);
}

static void trckydoc_draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	sauro_state *state = machine.driver_data<sauro_state>();
	UINT8 *spriteram = state->m_spriteram;
	int offs,code,sy,color,flipx,sx;
	int flipy = flip_screen_get(machine);

	/* Weird, sprites entries don't start on DWORD boundary */
	for (offs = 3; offs < state->m_spriteram_size - 1; offs += 4)
	{
		sy = spriteram[offs];

		if(spriteram[offs+3] & 0x08)
		{
			/* needed by the elevator cable (2nd stage), balls bouncing (3rd stage) and maybe other things */
			sy += 6;
		}

		code = spriteram[offs+1] + ((spriteram[offs+3] & 0x01) << 8);

		sx = spriteram[offs+2]-2;
		color = (spriteram[offs+3] >> 4) & 0x0f;

		sy = 236 - sy;

		/* similar to sauro but different bit is used .. */
		if (spriteram[offs+3] & 0x02)
		{
			if (sx > 0xc0)
			{
				/* Sign extend */
				sx = (signed int)(signed char)sx;
			}
		}
		else
		{
			if (sx < 0x40) continue;
		}

		flipx = spriteram[offs+3] & 0x04;

		if (flipy)
		{
			flipx = !flipx;
			sx = (235 - sx) & 0xff;  /* The &0xff is not 100% percent correct */
			sy = 240 - sy;
		}

		drawgfx_transpen(bitmap, cliprect,machine.gfx[1],
				code,
				color,
				flipx, flipy,
				sx,sy,0);
	}
}

SCREEN_UPDATE_IND16( trckydoc )
{
	sauro_state *state = screen.machine().driver_data<sauro_state>();
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	trckydoc_draw_sprites(screen.machine(), bitmap, cliprect);
	return 0;
}
