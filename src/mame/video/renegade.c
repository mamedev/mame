/***************************************************************************

    Renegade Video Hardware

***************************************************************************/

#include "emu.h"
#include "includes/renegade.h"


WRITE8_MEMBER(renegade_state::renegade_videoram_w)
{
	UINT8 *videoram = m_videoram;
	videoram[offset] = data;
	offset = offset % (64 * 16);
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(renegade_state::renegade_videoram2_w)
{
	m_videoram2[offset] = data;
	offset = offset % (32 * 32);
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(renegade_state::renegade_flipscreen_w)
{
	flip_screen_set(machine(), ~data & 0x01);
}

WRITE8_MEMBER(renegade_state::renegade_scroll0_w)
{
	m_scrollx = (m_scrollx & 0xff00) | data;
}

WRITE8_MEMBER(renegade_state::renegade_scroll1_w)
{
	m_scrollx = (m_scrollx & 0xff) | (data << 8);
}

static TILE_GET_INFO( get_bg_tilemap_info )
{
	renegade_state *state = machine.driver_data<renegade_state>();
	UINT8 *videoram = state->m_videoram;
	const UINT8 *source = &videoram[tile_index];
	UINT8 attributes = source[0x400]; /* CCC??BBB */
	SET_TILE_INFO(
		1 + (attributes & 0x7),
		source[0],
		attributes >> 5,
		0);
}

static TILE_GET_INFO( get_fg_tilemap_info )
{
	renegade_state *state = machine.driver_data<renegade_state>();
	const UINT8 *source = &state->m_videoram2[tile_index];
	UINT8 attributes = source[0x400];
	SET_TILE_INFO(
		0,
		(attributes & 3) * 256 + source[0],
		attributes >> 6,
		0);
}

VIDEO_START( renegade )
{
	renegade_state *state = machine.driver_data<renegade_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tilemap_info, tilemap_scan_rows,      16, 16, 64, 16);
	state->m_fg_tilemap = tilemap_create(machine, get_fg_tilemap_info, tilemap_scan_rows,   8, 8, 32, 32);

	state->m_fg_tilemap->set_transparent_pen(0);
	state->m_bg_tilemap->set_scrolldx(256, 0);

	state_save_register_global(machine, state->m_scrollx);
}

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	renegade_state *state = machine.driver_data<renegade_state>();
	UINT8 *source = state->m_spriteram;
	UINT8 *finish = source + 96 * 4;

	while (source < finish)
	{
		int sy = 240 - source[0];

		if (sy >= 16)
		{
			int attributes = source[1]; /* SFCCBBBB */
			int sx = source[3];
			int sprite_number = source[2];
			int sprite_bank = 9 + (attributes & 0xf);
			int color = (attributes >> 4) & 0x3;
			int xflip = attributes & 0x40;

			if (sx > 248)
				sx -= 256;

			if (flip_screen_get(machine))
			{
				sx = 240 - sx;
				sy = 240 - sy;
				xflip = !xflip;
			}

			if (attributes & 0x80) /* big sprite */
			{
				sprite_number &= ~1;
				drawgfx_transpen(bitmap, cliprect, machine.gfx[sprite_bank],
					sprite_number + 1,
					color,
					xflip, flip_screen_get(machine),
					sx, sy + (flip_screen_get(machine) ? -16 : 16), 0);
			}
			else
			{
				sy += (flip_screen_get(machine) ? -16 : 16);
			}
			drawgfx_transpen(bitmap, cliprect, machine.gfx[sprite_bank],
				sprite_number,
				color,
				xflip, flip_screen_get(machine),
				sx, sy, 0);
		}
		source += 4;
	}
}

SCREEN_UPDATE_IND16( renegade )
{
	renegade_state *state = screen.machine().driver_data<renegade_state>();
	state->m_bg_tilemap->set_scrollx(0, state->m_scrollx);
	state->m_bg_tilemap->draw(bitmap, cliprect, 0 , 0);
	draw_sprites(screen.machine(), bitmap, cliprect);
	state->m_fg_tilemap->draw(bitmap, cliprect, 0 , 0);
	return 0;
}
