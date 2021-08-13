// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, Bryan McPhail
/***************************************************************

 Pro Yakyuu Nyuudan Test Tryout (JPN Ver.)
 video hardware emulation

****************************************************************/

#include "emu.h"
#include "includes/tryout.h"


void tryout_state::palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	for (int i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		// green component
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		// blue component
		bit0 = 0;
		bit1 = BIT(color_prom[i], 6);
		bit2 = BIT(color_prom[i], 7);
		int const b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

TILE_GET_INFO_MEMBER(tryout_state::get_fg_tile_info)
{
	int code = m_videoram[tile_index];
	int attr = m_videoram[tile_index + 0x400];
	code |= ((attr & 0x03) << 8);
	int color = ((attr & 0x4) >> 2) + 6;

	tileinfo.set(0, code, color, 0);
}

TILE_GET_INFO_MEMBER(tryout_state::get_bg_tile_info)
{
	tileinfo.set(2, m_vram[tile_index] & 0x7f, 2, 0);
}

uint8_t tryout_state::vram_r(offs_t offset)
{
	return m_vram[offset]; // debug only
}

void tryout_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

void tryout_state::vram_w(offs_t offset, uint8_t data)
{
	/*  There are eight banks of vram - in bank 0 the first 0x400 bytes
	are reserved for the tilemap.  In banks 2, 4 and 6 the game never
	writes to the first 0x400 bytes - I suspect it's either
	unused, or it actually mirrors the tilemap ram from the first bank.

	The rest of the vram is tile data which has the bitplanes arranged
	in a very strange format.  For MAME's sake we reformat this on
	the fly for easier gfx decode.

	Bit 0 of the bank register seems special - it's kept low when uploading
	gfx data and then set high from that point onwards.

	*/
	const uint8_t bank = (m_vram_bank >> 1) & 0x7;


	if ((bank == 0 || bank == 2 || bank == 4 || bank == 6) && (offset & 0x7ff) < 0x400)
	{
		int newoff = offset & 0x3ff;

		m_vram[newoff] = data;
		m_bg_tilemap->mark_tile_dirty(newoff);
		return;
	}

	/*
	    Bit planes for tiles are arranged as follows within vram (split into high/low nibbles):
	        0x0400 (0) + 0x0400 (4) + 0x0800(0) - tiles 0x00 to 0x0f
	        0x0800 (4) + 0x0c00 (0) + 0x0c00(4) - tiles 0x10 to 0x1f
	        0x1400 (0) + 0x1400 (4) + 0x1800(0) - tiles 0x20 to 0x2f
	        0x1800 (4) + 0x1c00 (0) + 0x1c00(4) - tiles 0x30 to 0x3f
	        etc.
	*/

	offset = (offset & 0x7ff) | (bank << 11);
	m_vram[offset] = data;

	switch (offset & 0x1c00)
	{
	case 0x0400:
		m_vram_gfx[(offset & 0x3ff) + 0x0000 + ((offset & 0x2000) >> 1)] = (~data & 0xf);
		m_vram_gfx[(offset & 0x3ff) + 0x2000 + ((offset & 0x2000) >> 1)] = (~data & 0xf0) >> 4;
		break;
	case 0x0800:
		m_vram_gfx[(offset & 0x3ff) + 0x4000 + ((offset & 0x2000) >> 1)] = (~data & 0xf);
		m_vram_gfx[(offset & 0x3ff) + 0x4400 + ((offset & 0x2000) >> 1)] = (~data & 0xf0) >> 4;
		break;
	case 0x0c00:
		m_vram_gfx[(offset & 0x3ff) + 0x0400 + ((offset & 0x2000) >> 1)] = (~data & 0xf);
		m_vram_gfx[(offset & 0x3ff) + 0x2400 + ((offset & 0x2000) >> 1)] = (~data & 0xf0) >> 4;
		break;
	case 0x1400:
		m_vram_gfx[(offset & 0x3ff) + 0x0800 + ((offset & 0x2000) >> 1)] = (~data & 0xf);
		m_vram_gfx[(offset & 0x3ff) + 0x2800 + ((offset & 0x2000) >> 1)] = (~data & 0xf0) >> 4;
		break;
	case 0x1800:
		m_vram_gfx[(offset & 0x3ff) + 0x4800 + ((offset & 0x2000) >> 1)] = (~data & 0xf);
		m_vram_gfx[(offset & 0x3ff) + 0x4c00 + ((offset & 0x2000) >> 1)] = (~data & 0xf0) >> 4;
		break;
	case 0x1c00:
		m_vram_gfx[(offset & 0x3ff) + 0x0c00 + ((offset & 0x2000) >> 1)] = (~data & 0xf);
		m_vram_gfx[(offset & 0x3ff) + 0x2c00 + ((offset & 0x2000) >> 1)] = (~data & 0xf0) >> 4;
		break;
	}

	m_gfxdecode->gfx(2)->mark_dirty((offset - 0x400 / 64) & 0x7f);
}

void tryout_state::vram_bankswitch_w(uint8_t data)
{
	m_vram_bank = data;
}

void tryout_state::flipscreen_w(uint8_t data)
{
	flip_screen_set(data & 1);
}

TILEMAP_MAPPER_MEMBER(tryout_state::get_fg_memory_offset)
{
	return (row ^ 0x1f) + (col << 5);
}

TILEMAP_MAPPER_MEMBER(tryout_state::get_bg_memory_offset)
{
	int a;
//  if (col&0x20)
//      a= (7 - (row & 7)) + ((0x8 - (row & 0x8)) << 4) + ((col & 0xf) << 3) + (( (  0x10 - (col & 0x10) ) ) << 4) + ((( (col & 0x20))) << 4);
//  else
		a= (7 - (row & 7)) + ((0x8 - (row & 0x8)) << 4) + ((col & 0xf) << 3) + ((col & 0x10) << 4) + ((col & 0x20) << 4);

//  osd_printf_debug("%d %d -> %d\n", col, row, a);
	return a;
}

void tryout_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tryout_state::get_fg_tile_info)), tilemap_mapper_delegate(*this, FUNC(tryout_state::get_fg_memory_offset)),  8, 8, 32,32);
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tryout_state::get_bg_tile_info)), tilemap_mapper_delegate(*this, FUNC(tryout_state::get_bg_memory_offset)), 16,16, 64,16);

	m_fg_tilemap->set_transparent_pen(0);

	save_item(NAME(m_vram_bank));
}

void tryout_state::draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	for (int offs = 0; offs < 0x7f; offs += 4)
	{
		if (!(m_spriteram[0][offs] & 1))
			continue;

		int sprite = m_spriteram[0][offs + 1] + ((m_spriteram[1][offs] & 7) << 8);
		int x = m_spriteram[0][offs + 3] - 3;
		int y = m_spriteram[0][offs + 2];
		int color = 0;//(m_spriteram[0][offs] & 8) >> 3;
		int fx = (m_spriteram[0][offs] & 8) >> 3;
		int fy = 0;
		int inc = 16;

		if (flip_screen())
		{
			x = 240 - x;
			fx = !fx;

			y = 240 - y;
			fy = !fy;

			inc = -inc;
		}

		// Double Height
		if(m_spriteram[0][offs] & 0x10)
		{
			m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
				sprite,
				color, fx, fy, x, y + inc, 0);

			m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
				sprite + 1,
				color, fx, fy, x, y, 0);
		}
		else
		{
			m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
				sprite,
				color, fx, fy, x, y, 0);
		}
	}
}

uint32_t tryout_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int scrollx = 0;

	if (!flip_screen())
		m_fg_tilemap->set_scrollx(0, 16); // Assumed hard-wired
	else
		m_fg_tilemap->set_scrollx(0, -8); // Assumed hard-wired

	scrollx = m_gfx_control[1] + ((m_gfx_control[0] & 1) << 8) + ((m_gfx_control[0] & 4) << 7) - ((m_gfx_control[0] & 2) ? 0 : 0x100);

	// wrap-around
	if (m_gfx_control[1] == 0) { scrollx += 0x100; }

	m_bg_tilemap->set_scrollx(0, scrollx + 2); // why +2? hard-wired?
	m_bg_tilemap->set_scrolly(0, -m_gfx_control[2]);

	if(!(m_gfx_control[0] & 0x8)) // screen disable
	{
		// TODO: Color might be different, needs a video from an original PCB.
		bitmap.fill(m_palette->pen(0x10), cliprect);
	}
	else
	{
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
		draw_sprites(bitmap, cliprect);
	}

//  popmessage("%02x %02x %02x %02x", m_gfx_control[0], m_gfx_control[1], m_gfx_control[2], scrollx);
	return 0;
}
