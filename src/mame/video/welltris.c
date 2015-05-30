// license:BSD-3-Clause
// copyright-holders:David Haywood
#include "emu.h"
#include "includes/welltris.h"



void welltris_state::setbank(int num, int bank)
{
	if (m_gfxbank[num] != bank)
	{
		m_gfxbank[num] = bank;
		m_char_tilemap->mark_all_dirty();
	}
}


/* Not really enough evidence here */

WRITE16_MEMBER(welltris_state::palette_bank_w)
{
	if (ACCESSING_BITS_0_7)
	{
		if (m_charpalettebank != (data & 0x03))
		{
			m_charpalettebank = (data & 0x03);
			m_char_tilemap->mark_all_dirty();
		}

		flip_screen_set(data & 0x80);

		m_spritepalettebank = (data & 0x20) >> 5;
		m_pixelpalettebank = (data & 0x08) >> 3;
	}
}

WRITE16_MEMBER(welltris_state::gfxbank_w)
{
	if (ACCESSING_BITS_0_7)
	{
		setbank(0, (data & 0xf0) >> 4);
		setbank(1, data & 0x0f);
	}
}

WRITE16_MEMBER(welltris_state::scrollreg_w)
{
	switch (offset) {
		case 0: m_scrollx = data - 14; break;
		case 1: m_scrolly = data +  0; break;
	}
}

TILE_GET_INFO_MEMBER(welltris_state::get_tile_info)
{
	UINT16 code = m_charvideoram[tile_index];
	int bank = (code & 0x1000) >> 12;

	SET_TILE_INFO_MEMBER(0,
			(code & 0x0fff) + (m_gfxbank[bank] << 12),
			((code & 0xe000) >> 13) + (8 * m_charpalettebank),
			0);
}

WRITE16_MEMBER(welltris_state::charvideoram_w)
{
	COMBINE_DATA(&m_charvideoram[offset]);
	m_char_tilemap->mark_tile_dirty(offset);
}

void welltris_state::video_start()
{
	m_char_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(welltris_state::get_tile_info),this), TILEMAP_SCAN_ROWS,  8, 8, 64, 32);

	m_char_tilemap->set_transparent_pen(15);

	save_item(NAME(m_gfxbank));
	save_item(NAME(m_charpalettebank));
	save_item(NAME(m_spritepalettebank));
	save_item(NAME(m_pixelpalettebank));
	save_item(NAME(m_scrollx));
	save_item(NAME(m_scrolly));
}

void welltris_state::draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x, y;
	int pixdata;

	for (y = 0; y < 256; y++) {
		for (x = 0; x < 512 / 2; x++) {
			pixdata = m_pixelram[(x & 0xff) + (y & 0xff) * 256];

			bitmap.pix16(y, (x * 2) + 0) = (pixdata >> 8) + (0x100 * m_pixelpalettebank) + 0x400;
			bitmap.pix16(y, (x * 2) + 1) = (pixdata & 0xff) + (0x100 * m_pixelpalettebank) + 0x400;
		}
	}
}

UINT32 welltris_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_char_tilemap->set_scrollx(0, m_scrollx);
	m_char_tilemap->set_scrolly(0, m_scrolly);

	draw_background(bitmap, cliprect);
	m_char_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_spr_old->turbofrc_draw_sprites(m_spriteram, m_spriteram.bytes(), m_spritepalettebank, bitmap, cliprect, screen.priority(), 0);
	return 0;
}
