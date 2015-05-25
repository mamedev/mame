// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/***************************************************************************

    Atari Runaway video emulation

****************************************************************************/

#include "emu.h"
#include "includes/runaway.h"


WRITE8_MEMBER(runaway_state::runaway_paletteram_w)
{
	int R =
		0x21 * ((~data >> 2) & 1) +
		0x47 * ((~data >> 3) & 1) +
		0x97 * ((~data >> 4) & 1);

	int G =
		0x21 * ((~data >> 5) & 1) +
		0x47 * ((~data >> 6) & 1) +
		0x97 * ((~data >> 7) & 1);

	int B =
		0x21 * 0 +
		0x47 * ((~data >> 0) & 1) +
		0x97 * ((~data >> 1) & 1);

	m_palette->set_pen_color(offset, rgb_t(R, G, B));
}



WRITE8_MEMBER(runaway_state::runaway_video_ram_w)
{
	m_video_ram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}



WRITE8_MEMBER(runaway_state::runaway_tile_bank_w)
{
	if ((data & 1) != m_tile_bank)
	{
		m_bg_tilemap->mark_all_dirty();
	}

	m_tile_bank = data & 1;
}


TILE_GET_INFO_MEMBER(runaway_state::runaway_get_tile_info)
{
	UINT8 code = m_video_ram[tile_index];

	SET_TILE_INFO_MEMBER(0, ((code & 0x3f) << 1) | ((code & 0x40) >> 6) | (m_tile_bank << 7), 0, (code & 0x80) ? TILE_FLIPY : 0);
}


TILE_GET_INFO_MEMBER(runaway_state::qwak_get_tile_info)
{
	UINT8 code = m_video_ram[tile_index];

	SET_TILE_INFO_MEMBER(0, ((code & 0x7f) << 1) | ((code & 0x80) >> 7), 0, 0);
}



void runaway_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(runaway_state::runaway_get_tile_info),this), TILEMAP_SCAN_ROWS,  8, 8, 32, 30);

	save_item(NAME(m_tile_bank));
}


VIDEO_START_MEMBER(runaway_state,qwak)
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(runaway_state::qwak_get_tile_info),this), TILEMAP_SCAN_ROWS,  8, 8, 32, 30);

	save_item(NAME(m_tile_bank));
}



UINT32 runaway_state::screen_update_runaway(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i;

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	for (i = 0; i < 16; i++)
	{
		unsigned code = m_sprite_ram[i] & 0x3f;

		int x = m_sprite_ram[i + 0x20];
		int y = m_sprite_ram[i + 0x10];

		int flipx = m_sprite_ram[i] & 0x40;
		int flipy = m_sprite_ram[i] & 0x80;

		code |= (m_sprite_ram[i + 0x30] << 2) & 0x1c0;

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
			code,
			0,
			flipx, flipy,
			x, 240 - y, 0);

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
			code,
			0,
			flipx, flipy,
			x - 256, 240 - y, 0);
	}
	return 0;
}


UINT32 runaway_state::screen_update_qwak(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i;

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	for (i = 0; i < 16; i++)
	{
		unsigned code = m_sprite_ram[i] & 0x7f;

		int x = m_sprite_ram[i + 0x20];
		int y = m_sprite_ram[i + 0x10];

		int flipx = 0;
		int flipy = m_sprite_ram[i] & 0x80;

		code |= (m_sprite_ram[i + 0x30] << 2) & 0x1c0;

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
			code,
			0,
			flipx, flipy,
			x, 240 - y, 0);

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
			code,
			0,
			flipx, flipy,
			x - 256, 240 - y, 0);
	}
	return 0;
}
