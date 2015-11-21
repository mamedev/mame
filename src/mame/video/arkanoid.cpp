// license:BSD-3-Clause
// copyright-holders:Brad Oliver
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
	flip_screen_x_set(data & 0x01);
	flip_screen_y_set(data & 0x02);

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
	if (m_mcu != NULL)  // Bootlegs don't have the MCU but still set this bit
		m_mcu->set_input_line(INPUT_LINE_RESET, (data & 0x80) ? CLEAR_LINE : ASSERT_LINE);
}


WRITE8_MEMBER(arkanoid_state::brixian_d008_w)
{
	int bank;

	/* bits 0 and 1 flip X and Y, I don't know which is which */
	flip_screen_x_set(data & 0x01);
	flip_screen_y_set(data & 0x02);

	/* bit 2 selects the input paddle */
	/*  - not relevant to brixian */

	/* bit 3 is coin lockout (but not the service coin) */
	/*  - not here, means you can only play 1 game */

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

	/* bit 7 is MCU reset on Arkanoid */
	/*  - does it reset the Brixian MCU too? */
}


/* different hook-up, everything except for bits 0-1 and 7 aren't tested afaik. */
WRITE8_MEMBER(arkanoid_state::tetrsark_d008_w)
{
	int bank;

	/* bits 0 and 1 flip X and Y, I don't know which is which */
	flip_screen_x_set(data & 0x01);
	flip_screen_y_set(data & 0x02);

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
	flip_screen_x_set(data & 0x01);
	flip_screen_y_set(data & 0x02);

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

void arkanoid_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(arkanoid_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

void arkanoid_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int offs;

	for (offs = 0; offs < m_spriteram.bytes(); offs += 4)
	{
		int sx, sy, code;

		sx = m_spriteram[offs];
		sy = 248 - m_spriteram[offs + 1];
		if (flip_screen_x())
			sx = 248 - sx;
		if (flip_screen_y())
			sy = 248 - sy;

		code = m_spriteram[offs + 3] + ((m_spriteram[offs + 2] & 0x03) << 8) + 1024 * m_gfxbank;

		m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
				2 * code,
				((m_spriteram[offs + 2] & 0xf8) >> 3) + 32 * m_palettebank,
				flip_screen_x(),flip_screen_y(),
				sx,sy + (flip_screen_y() ? 8 : -8),0);
		m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
				2 * code + 1,
				((m_spriteram[offs + 2] & 0xf8) >> 3) + 32 * m_palettebank,
				flip_screen_x(),flip_screen_y(),
				sx,sy,0);
	}
}


UINT32 arkanoid_state::screen_update_arkanoid(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}

UINT32 arkanoid_state::screen_update_hexa(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
