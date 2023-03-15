// license:BSD-3-Clause
// copyright-holders:Brad Oliver
/***************************************************************************

  arkanoid.cpp

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "arkanoid.h"


void arkanoid_state::arkanoid_videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

void arkanoid_state::arkanoid_d008_w(uint8_t data)
{
	int bank;

	/* bits 0 and 1 flip X and Y */
	flip_screen_x_set(data & 0x01);
	flip_screen_y_set(data & 0x02);

	/* bit 2 selects the input paddle */
	m_paddle_select = data & 0x04;

	/* bit 3 is coin lockout (but not the service coin) */
	machine().bookkeeping().coin_lockout_w(0, !(data & 0x08));
	machine().bookkeeping().coin_lockout_w(1, !(data & 0x08));

	/* bit 4 is unknown */

	/* bit 5 controls the graphics rom bank */
	bank = (data & 0x20) >> 5;

	if (m_gfxbank != bank)
	{
		m_gfxbank = bank;
		m_bg_tilemap->mark_all_dirty();
	}

	/* bit 6 controls the palette bank */
	bank = (data & 0x40) >> 6;

	if (m_palettebank != bank)
	{
		m_palettebank = bank;
		m_bg_tilemap->mark_all_dirty();
	}

	// bit 7 resets the MCU and semaphore flipflops
	// This bit is flipped early in bootup just prior to accessing the MCU for the first time.
	if (m_mcuintf.found()) // Bootlegs don't have the MCU but still set this bit
		m_mcuintf->reset_w(BIT(data, 7) ? CLEAR_LINE : ASSERT_LINE);
}


void arkanoid_state::brixian_d008_w(uint8_t data)
{
	int bank;

	/* bits 0 and 1 flip X and Y */
	flip_screen_x_set(data & 0x01);
	flip_screen_y_set(data & 0x02);

	/* bit 2 selects the input paddle */
	/*  - not relevant to brixian */

	/* bit 3 is coin lockout (but not the service coin) */
	/*  - not here, means you can only play 1 game */

	/* bit 4 is unknown */

	/* bit 5 controls the graphics rom bank */
	bank = (data & 0x20) >> 5;

	if (m_gfxbank != bank)
	{
		m_gfxbank = bank;
		m_bg_tilemap->mark_all_dirty();
	}

	/* bit 6 controls the palette bank */
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
void arkanoid_state::tetrsark_d008_w(uint8_t data)
{
	int bank;

	/* bits 0 and 1 flip X and Y */
	flip_screen_x_set(data & 0x01);
	flip_screen_y_set(data & 0x02);

	/* bit 2 selects the input paddle? */
	m_paddle_select = data & 0x04;

	/* bit 3-4 is unknown? */

	/* bit 5 controls the graphics rom bank */
	bank = (data & 0x20) >> 5;

	if (m_gfxbank != bank)
	{
		m_gfxbank = bank;
		m_bg_tilemap->mark_all_dirty();
	}

	/* bit 6 controls the palette bank */
	bank = (data & 0x40) >> 6;

	if (m_palettebank != bank)
	{
		m_palettebank = bank;
		m_bg_tilemap->mark_all_dirty();
	}

	/* bit 7 is coin lockout (but not the service coin) */
	machine().bookkeeping().coin_lockout_w(0, !(data & 0x80));
	machine().bookkeeping().coin_lockout_w(1, !(data & 0x80));
}


void arkanoid_state::hexa_d008_w(uint8_t data)
{
	/* bits 0 and 1 flip X and Y */
	flip_screen_x_set(data & 0x01);
	flip_screen_y_set(data & 0x02);

	/* bit 2 - 3 unknown */

	/* bit 4 could be the ROM bank selector for 8000-bfff (not sure) */
	membank("bank1")->set_entry(((data & 0x10) >> 4));

	/* bit 5 controls the graphics rom bank */
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

	tileinfo.set(0, code, color, 0);
}

void arkanoid_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(arkanoid_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
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


uint32_t arkanoid_state::screen_update_arkanoid(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}

uint32_t arkanoid_state::screen_update_hexa(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
