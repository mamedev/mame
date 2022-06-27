// license:BSD-3-Clause
// copyright-holders:David Haywood, Phil Bennett
/*************************************************************************

    Kyuukoukabakugekitai - Dive Bomber Squad

*************************************************************************/

#include "emu.h"
#include "divebomb.h"
#include "video/resnet.h"



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(divebomb_state::get_fg_tile_info)
{
	uint32_t code = m_fgram[tile_index + 0x000];
	uint32_t attr = m_fgram[tile_index + 0x400];
	uint32_t colour = attr >> 4;

	code |= (attr & 0x3) << 8;

	tileinfo.set(0, code, colour, 0);
}



/*************************************
 *
 *  K051316 callbacks
 *
 *************************************/

K051316_CB_MEMBER(divebomb_state::zoom_callback_1)
{
	*code |= (*color & 0x03) << 8;
	*color = 0 + ((m_roz_pal >> 4) & 3);
}


K051316_CB_MEMBER(divebomb_state::zoom_callback_2)
{
	*code |= (*color & 0x03) << 8;
	*color = 4 + (m_roz_pal & 3);
}



/*************************************
 *
 *  Video hardware handlers
 *
 *************************************/

void divebomb_state::fgram_w(offs_t offset, uint8_t data)
{
	m_fgram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset & 0x3ff);
}


void divebomb_state::rozcpu_pal_w(uint8_t data)
{
	//.... ..xx  K051316 1 palette select
	//..xx ....  K051316 2 palette select

	uint8_t old_pal = m_roz_pal;
	m_roz_pal = data;

	if ((old_pal & 0x03) != (data & 0x03))
		m_k051316[1]->mark_tmap_dirty();

	if ((old_pal & 0x30) != (data & 0x30))
		m_k051316[0]->mark_tmap_dirty();

	if (data & 0xcc)
		logerror("rozcpu_port50_w %02x\n", data);
}



/*************************************
 *
 *  Video hardware init
 *
 *************************************/

void divebomb_state::decode_proms(palette_device &palette, const uint8_t * rgn, int size, int index, bool inv)
{
	static constexpr int resistances[4] = { 2000, 1000, 470, 220 };

	// compute the color output resistor weights
	double rweights[4], gweights[4], bweights[4];
	compute_resistor_weights(0, 255, -1.0,
			4, resistances, rweights, 0, 0,
			4, resistances, gweights, 0, 0,
			4, resistances, bweights, 0, 0);

	// create a lookup table for the palette
	for (uint32_t i = 0; i < size; ++i)
	{
		uint32_t const rdata = rgn[i + size*2] & 0x0f;
		uint32_t const r = combine_weights(rweights, BIT(rdata, 0), BIT(rdata, 1), BIT(rdata, 2), BIT(rdata, 3));

		uint32_t const gdata = rgn[i + size] & 0x0f;
		uint32_t const g = combine_weights(gweights, BIT(gdata, 0), BIT(gdata, 1), BIT(gdata, 2), BIT(gdata, 3));

		uint32_t const bdata = rgn[i] & 0x0f;
		uint32_t const b = combine_weights(bweights, BIT(bdata, 0), BIT(bdata, 1), BIT(bdata, 2), BIT(bdata, 3));

		if (!inv)
			palette.set_pen_color(index + i, rgb_t(r, g, b));
		else
			palette.set_pen_color(index + (i ^ 0xff), rgb_t(r, g, b));
	}
}


void divebomb_state::divebomb_palette(palette_device &palette) const
{
	decode_proms(palette, memregion("spr_proms")->base(), 0x100, 0x400 + 0x400 + 0x400, false);
	decode_proms(palette, memregion("fg_proms")->base(), 0x400, 0x400 + 0x400, false);
	decode_proms(palette, memregion("k051316_1_pr")->base(), 0x400, 0, true);
	decode_proms(palette, memregion("k051316_2_pr")->base(), 0x400, 0x400, true);
}


void divebomb_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(divebomb_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_fg_tilemap->set_transparent_pen(0);
	m_fg_tilemap->set_scrolly(0, 16);
}



/*************************************
 *
 *  Main update
 *
 *************************************/

void divebomb_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const uint8_t *spriteram = m_spriteram;

	for (uint32_t i = 0; i < m_spriteram.bytes(); i += 4)
	{
		uint32_t sy = spriteram[i + 3];
		uint32_t sx = spriteram[i + 0];
		uint32_t code = spriteram[i + 2];
		uint32_t attr = spriteram[i + 1];

		code += (attr & 0x0f) << 8;

		uint32_t colour = attr >> 4;

		m_gfxdecode->gfx(1)->transpen(bitmap, cliprect, code, colour, 0, 0, sx, sy, 0);
		m_gfxdecode->gfx(1)->transpen(bitmap, cliprect, code, colour, 0, 0, sx, sy-256, 0);
	}
}


uint32_t divebomb_state::screen_update_divebomb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	for (int chip = 1; chip >= 0; chip--)
	{
		if (m_roz_enable[chip])
		{
			m_k051316[chip]->zoom_draw(screen, bitmap, cliprect, 0, 0);
		}
	}

	draw_sprites(bitmap, cliprect);

	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}
