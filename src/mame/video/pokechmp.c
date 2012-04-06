/* Poke Champ */

#include "emu.h"
#include "includes/pokechmp.h"


WRITE8_MEMBER(pokechmp_state::pokechmp_videoram_w)
{
	UINT8 *videoram = m_videoram;
	videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

WRITE8_MEMBER(pokechmp_state::pokechmp_flipscreen_w)
{
	if (flip_screen_get(machine()) != (data & 0x80))
	{
		flip_screen_set(machine(), data & 0x80);
		machine().tilemap().mark_all_dirty();
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	pokechmp_state *state = machine.driver_data<pokechmp_state>();
	UINT8 *videoram = state->m_videoram;
	int code = videoram[tile_index*2+1] + ((videoram[tile_index*2] & 0x3f) << 8);
	int color = videoram[tile_index*2] >> 6;

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( pokechmp )
{
	pokechmp_state *state = machine.driver_data<pokechmp_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows,
		 8, 8, 32, 32);
}

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	pokechmp_state *state = machine.driver_data<pokechmp_state>();
	UINT8 *spriteram = state->m_spriteram;
	int offs;

	for (offs = 0;offs < state->m_spriteram_size;offs += 4)
	{
		if (spriteram[offs] != 0xf8)
		{
			int sx,sy,flipx,flipy;


			sx = 240 - spriteram[offs+2];
			sy = 240 - spriteram[offs];

			flipx = spriteram[offs+1] & 0x04;
			flipy = spriteram[offs+1] & 0x02;
			if (flip_screen_get(machine)) {
				sx=240-sx;
				sy=240-sy;
				if (flipx) flipx=0; else flipx=1;
				if (flipy) flipy=0; else flipy=1;
			}

			drawgfx_transpen(bitmap,cliprect,machine.gfx[1],
					spriteram[offs+3] + ((spriteram[offs+1] & 1) << 8),
					(spriteram[offs+1] & 0xf0) >> 4,
					flipx,flipy,
					sx,sy,0);
		}
	}
}

SCREEN_UPDATE_IND16( pokechmp )
{
	pokechmp_state *state = screen.machine().driver_data<pokechmp_state>();
	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_sprites(screen.machine(), bitmap, cliprect);
	return 0;
}
