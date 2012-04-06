/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/commando.h"


WRITE8_MEMBER(commando_state::commando_videoram_w)
{

	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(commando_state::commando_colorram_w)
{

	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(commando_state::commando_videoram2_w)
{

	m_videoram2[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(commando_state::commando_colorram2_w)
{

	m_colorram2[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(commando_state::commando_scrollx_w)
{

	m_scroll_x[offset] = data;
	m_bg_tilemap->set_scrollx(0, m_scroll_x[0] | (m_scroll_x[1] << 8));
}

WRITE8_MEMBER(commando_state::commando_scrolly_w)
{

	m_scroll_y[offset] = data;
	m_bg_tilemap->set_scrolly(0, m_scroll_y[0] | (m_scroll_y[1] << 8));
}

WRITE8_MEMBER(commando_state::commando_c804_w)
{

	// bits 0 and 1 are coin counters
	coin_counter_w(machine(), 0, data & 0x01);
	coin_counter_w(machine(), 1, data & 0x02);

	// bit 4 resets the sound CPU
	device_set_input_line(m_audiocpu, INPUT_LINE_RESET, (data & 0x10) ? ASSERT_LINE : CLEAR_LINE);

	// bit 7 flips screen
	flip_screen_set(machine(), data & 0x80);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	commando_state *state = machine.driver_data<commando_state>();
	int attr = state->m_colorram[tile_index];
	int code = state->m_videoram[tile_index] + ((attr & 0xc0) << 2);
	int color = attr & 0x0f;
	int flags = TILE_FLIPYX((attr & 0x30) >> 4);

	SET_TILE_INFO(1, code, color, flags);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	commando_state *state = machine.driver_data<commando_state>();
	int attr = state->m_colorram2[tile_index];
	int code = state->m_videoram2[tile_index] + ((attr & 0xc0) << 2);
	int color = attr & 0x0f;
	int flags = TILE_FLIPYX((attr & 0x30) >> 4);

	SET_TILE_INFO(0, code, color, flags);
}

VIDEO_START( commando )
{
	commando_state *state = machine.driver_data<commando_state>();

	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_cols, 16, 16, 32, 32);
	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);

	state->m_fg_tilemap->set_transparent_pen(3);
}

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	commando_state *state = machine.driver_data<commando_state>();
	UINT8 *buffered_spriteram = state->m_spriteram->buffer();
	int offs;

	for (offs = state->m_spriteram->bytes() - 4; offs >= 0; offs -= 4)
	{
		// bit 1 of attr is not used
		int attr = buffered_spriteram[offs + 1];
		int bank = (attr & 0xc0) >> 6;
		int code = buffered_spriteram[offs] + 256 * bank;
		int color = (attr & 0x30) >> 4;
		int flipx = attr & 0x04;
		int flipy = attr & 0x08;
		int sx = buffered_spriteram[offs + 3] - ((attr & 0x01) << 8);
		int sy = buffered_spriteram[offs + 2];

		if (flip_screen_get(machine))
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		if (bank < 3)
			drawgfx_transpen(bitmap, cliprect, machine.gfx[2], code, color, flipx, flipy, sx, sy, 15);
	}
}

SCREEN_UPDATE_IND16( commando )
{
	commando_state *state = screen.machine().driver_data<commando_state>();

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_sprites(screen.machine(), bitmap, cliprect);
	state->m_fg_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}
