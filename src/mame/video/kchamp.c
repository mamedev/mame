/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/kchamp.h"


PALETTE_INIT( kchamp )
{
	int i, red, green, blue;

	for (i = 0; i < machine.total_colors(); i++)
	{
		red = color_prom[i];
		green = color_prom[machine.total_colors() + i];
		blue = color_prom[2 * machine.total_colors() + i];

		palette_set_color_rgb(machine, i, pal4bit(red), pal4bit(green), pal4bit(blue));
	}
}

WRITE8_MEMBER(kchamp_state::kchamp_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(kchamp_state::kchamp_colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(kchamp_state::kchamp_flipscreen_w)
{
	flip_screen_set(machine(), data & 0x01);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	kchamp_state *state = machine.driver_data<kchamp_state>();
	int code = state->m_videoram[tile_index] + ((state->m_colorram[tile_index] & 7) << 8);
	int color = (state->m_colorram[tile_index] >> 3) & 0x1f;

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( kchamp )
{
	kchamp_state *state = machine.driver_data<kchamp_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
}

/*
        Sprites
        -------
        Offset        Encoding
             0        YYYYYYYY
             1        TTTTTTTT
             2        FGGTCCCC
             3        XXXXXXXX
*/

static void kchamp_draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	kchamp_state *state = machine.driver_data<kchamp_state>();
	UINT8 *spriteram = state->m_spriteram;
	int offs;

	for (offs = 0; offs < 0x100; offs += 4)
	{
		int attr = spriteram[offs + 2];
		int bank = 1 + ((attr & 0x60) >> 5);
		int code = spriteram[offs + 1] + ((attr & 0x10) << 4);
		int color = attr & 0x0f;
		int flipx = 0;
		int flipy = attr & 0x80;
		int sx = spriteram[offs + 3] - 8;
		int sy = 247 - spriteram[offs];

		if (flip_screen_get(machine))
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap, cliprect, machine.gfx[bank], code, color, flipx, flipy, sx, sy, 0);
	}
}

static void kchampvs_draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	kchamp_state *state = machine.driver_data<kchamp_state>();
	UINT8 *spriteram = state->m_spriteram;
	int offs;

	for (offs = 0; offs < 0x100; offs += 4)
	{
		int attr = spriteram[offs + 2];
		int bank = 1 + ((attr & 0x60) >> 5);
		int code = spriteram[offs + 1] + ((attr & 0x10) << 4);
		int color = attr & 0x0f;
		int flipx = 0;
		int flipy = attr & 0x80;
		int sx = spriteram[offs + 3];
		int sy = 240 - spriteram[offs];

		if (flip_screen_get(machine))
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap, cliprect, machine.gfx[bank], code, color, flipx, flipy, sx, sy, 0);
	}
}


SCREEN_UPDATE_IND16( kchamp )
{
	kchamp_state *state = screen.machine().driver_data<kchamp_state>();

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	kchamp_draw_sprites(screen.machine(), bitmap, cliprect);
	return 0;
}

SCREEN_UPDATE_IND16( kchampvs )
{
	kchamp_state *state = screen.machine().driver_data<kchamp_state>();

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	kchampvs_draw_sprites(screen.machine(), bitmap, cliprect);
	return 0;
}
