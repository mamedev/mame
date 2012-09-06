/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/arkanoid.h"


WRITE8_MEMBER(arkanoid_state::arkanoid_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

WRITE8_MEMBER(arkanoid_state::arkanoid_d008_w)
{
	int bank;

	/* bits 0 and 1 flip X and Y, I don't know which is which */
	if (flip_screen_x() != (data & 0x01))
	{
		flip_screen_x_set(data & 0x01);
		m_bg_tilemap->mark_all_dirty();
	}

	if (flip_screen_y() != (data & 0x02))
	{
		flip_screen_y_set(data & 0x02);
		m_bg_tilemap->mark_all_dirty();
	}

	/* bit 2 selects the input paddle */
	m_paddle_select = data & 0x04;

	/* bit 3 is coin lockout (but not the service coin) */
	coin_lockout_w(machine(), 0, !(data & 0x08));
	coin_lockout_w(machine(), 1, !(data & 0x08));

	/* bit 4 is unknown */

	/* bits 5 and 6 control gfx bank and palette bank. They are used together */
	/* so I don't know which is which. */
	bank = (data & 0x20) >> 5;

	if (m_gfxbank != bank)
	{
		m_gfxbank = bank;
		m_bg_tilemap->mark_all_dirty();
	}

	bank = (data & 0x40) >> 6;

	if (m_palettebank != bank)
	{
		m_palettebank = bank;
		m_bg_tilemap->mark_all_dirty();
	}

	/* BM:  bit 7 is suspected to be MCU reset, the evidence for this is that
     the games tilt mode reset sequence shows the main CPU must be able to
     directly control the reset line of the MCU, else the game will crash
     leaving the tilt screen (as the MCU is now out of sync with main CPU
     which resets itself).  This bit is the likely candidate as it is flipped
     early in bootup just prior to accessing the MCU for the first time. */
	if (m_mcu != NULL)	// Bootlegs don't have the MCU but still set this bit
		device_set_input_line(m_mcu, INPUT_LINE_RESET, (data & 0x80) ? CLEAR_LINE : ASSERT_LINE);
}

/* different hook-up, everything except for bits 0-1 and 7 aren't tested afaik. */
WRITE8_MEMBER(arkanoid_state::tetrsark_d008_w)
{
	int bank;

	/* bits 0 and 1 flip X and Y, I don't know which is which */
	if (flip_screen_x() != (data & 0x01))
	{
		flip_screen_x_set(data & 0x01);
		m_bg_tilemap->mark_all_dirty();
	}

	if (flip_screen_y() != (data & 0x02))
	{
		flip_screen_y_set(data & 0x02);
		m_bg_tilemap->mark_all_dirty();
	}

	/* bit 2 selects the input paddle? */
	m_paddle_select = data & 0x04;

	/* bit 3-4 is unknown? */

	/* bits 5 and 6 control gfx bank and palette bank. They are used together */
	/* so I don't know which is which.? */
	bank = (data & 0x20) >> 5;

	if (m_gfxbank != bank)
	{
		m_gfxbank = bank;
		m_bg_tilemap->mark_all_dirty();
	}

	bank = (data & 0x40) >> 6;

	if (m_palettebank != bank)
	{
		m_palettebank = bank;
		m_bg_tilemap->mark_all_dirty();
	}

	/* bit 7 is coin lockout (but not the service coin) */
	coin_lockout_w(machine(), 0, !(data & 0x80));
	coin_lockout_w(machine(), 1, !(data & 0x80));
}


WRITE8_MEMBER(arkanoid_state::hexa_d008_w)
{

	/* bit 0 = flipx (or y?) */
	if (flip_screen_x() != (data & 0x01))
	{
		flip_screen_x_set(data & 0x01);
		m_bg_tilemap->mark_all_dirty();
	}

	/* bit 1 = flipy (or x?) */
	if (flip_screen_y() != (data & 0x02))
	{
		flip_screen_y_set(data & 0x02);
		m_bg_tilemap->mark_all_dirty();
	}

	/* bit 2 - 3 unknown */

	/* bit 4 could be the ROM bank selector for 8000-bfff (not sure) */
	membank("bank1")->set_entry(((data & 0x10) >> 4));

	/* bit 5 = gfx bank */
	if (m_gfxbank != ((data & 0x20) >> 5))
	{
		m_gfxbank = (data & 0x20) >> 5;
		m_bg_tilemap->mark_all_dirty();
	}

	/* bit 6 - 7 unknown */
}

TILE_GET_INFO_MEMBER(arkanoid_state::get_bg_tile_info)
{
	int offs = tile_index * 2;
	int code = m_videoram[offs + 1] + ((m_videoram[offs] & 0x07) << 8) + 2048 * m_gfxbank;
	int color = ((m_videoram[offs] & 0xf8) >> 3) + 32 * m_palettebank;

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}

VIDEO_START( arkanoid )
{
	arkanoid_state *state = machine.driver_data<arkanoid_state>();

	state->m_bg_tilemap = &machine.tilemap().create(tilemap_get_info_delegate(FUNC(arkanoid_state::get_bg_tile_info),state), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

static void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	arkanoid_state *state = machine.driver_data<arkanoid_state>();
	int offs;

	for (offs = 0; offs < state->m_spriteram.bytes(); offs += 4)
	{
		int sx, sy, code;

		sx = state->m_spriteram[offs];
		sy = 248 - state->m_spriteram[offs + 1];
		if (state->flip_screen_x())
			sx = 248 - sx;
		if (state->flip_screen_y())
			sy = 248 - sy;

		code = state->m_spriteram[offs + 3] + ((state->m_spriteram[offs + 2] & 0x03) << 8) + 1024 * state->m_gfxbank;

		drawgfx_transpen(bitmap,cliprect,machine.gfx[0],
				2 * code,
				((state->m_spriteram[offs + 2] & 0xf8) >> 3) + 32 * state->m_palettebank,
				state->flip_screen_x(),state->flip_screen_y(),
				sx,sy + (state->flip_screen_y() ? 8 : -8),0);
		drawgfx_transpen(bitmap,cliprect,machine.gfx[0],
				2 * code + 1,
				((state->m_spriteram[offs + 2] & 0xf8) >> 3) + 32 * state->m_palettebank,
				state->flip_screen_x(),state->flip_screen_y(),
				sx,sy,0);
	}
}


SCREEN_UPDATE_IND16( arkanoid )
{
	arkanoid_state *state = screen.machine().driver_data<arkanoid_state>();

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_sprites(screen.machine(), bitmap, cliprect);
	return 0;
}

SCREEN_UPDATE_IND16( hexa )
{
	arkanoid_state *state = screen.machine().driver_data<arkanoid_state>();

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	return 0;
}
