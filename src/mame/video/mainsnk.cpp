// license:BSD-3-Clause
// copyright-holders:David Haywood, Tomasz Slanina
#include "emu.h"
#include "includes/mainsnk.h"


void mainsnk_state::mainsnk_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();
	constexpr int num_colors = 0x400;

	for (int i = 0; i < num_colors; i++)
	{
		int bit0, bit1, bit2, bit3;

		bit0 = BIT(color_prom[i + 2*num_colors], 3);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		bit3 = BIT(color_prom[i], 3);
		int const r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		bit0 = BIT(color_prom[i + 2*num_colors], 2);
		bit1 = BIT(color_prom[i + num_colors], 2);
		bit2 = BIT(color_prom[i + num_colors], 3);
		bit3 = BIT(color_prom[i], 0);
		int const g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		bit0 = BIT(color_prom[i + 2*num_colors], 0);
		bit1 = BIT(color_prom[i + 2*num_colors], 1);
		bit2 = BIT(color_prom[i + num_colors], 0);
		bit3 = BIT(color_prom[i + num_colors], 1);
		int const b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

TILEMAP_MAPPER_MEMBER(mainsnk_state::marvins_tx_scan_cols)
{
	// tilemap is 36x28, the central part is from the first RAM page and the
	// extra 4 columns are from the second page
	col -= 2;
	if (col & 0x20)
		return 0x400 + row + ((col & 0x1f) << 5);
	else
		return row + (col << 5);
}

TILE_GET_INFO_MEMBER(mainsnk_state::get_tx_tile_info)
{
	int code = m_fgram[tile_index];

	SET_TILE_INFO_MEMBER(0,
			code,
			0,
			tile_index & 0x400 ? TILE_FORCE_LAYER0 : 0);
}

TILE_GET_INFO_MEMBER(mainsnk_state::get_bg_tile_info)
{
	int code = (m_bgram[tile_index]);

	SET_TILE_INFO_MEMBER(0,
			m_bg_tile_offset + code,
			0,
			0);
}


void mainsnk_state::video_start()
{
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(mainsnk_state::get_tx_tile_info),this), tilemap_mapper_delegate(FUNC(mainsnk_state::marvins_tx_scan_cols),this), 8, 8, 36, 28);
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(mainsnk_state::get_bg_tile_info),this), TILEMAP_SCAN_COLS,    8, 8, 32, 32);

	m_tx_tilemap->set_transparent_pen(15);
	m_tx_tilemap->set_scrolldy(8, 8);

	m_bg_tilemap->set_scrolldx(16, 16);
	m_bg_tilemap->set_scrolldy(8,  8);

	save_item(NAME(m_bg_tile_offset));
}


WRITE8_MEMBER(mainsnk_state::c600_w)
{
	int total_elements = m_gfxdecode->gfx(0)->elements();

	flip_screen_set(BIT(data, 7));

	m_bg_tilemap->set_palette_offset((data & 0x07) << 4);
	m_tx_tilemap->set_palette_offset((data & 0x07) << 4);

	int bank = 0;
	if (total_elements == 0x400)    // mainsnk
		bank = ((data & 0x30) >> 4);
	else if (total_elements == 0x800)   // canvas
		bank = ((data & 0x40) >> 6) | ((data & 0x30) >> 3);

	if (m_bg_tile_offset != (bank << 8))
	{
		m_bg_tile_offset = bank << 8;
		m_bg_tilemap->mark_all_dirty();
	}
}

WRITE8_MEMBER(mainsnk_state::fgram_w)
{
	m_fgram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(mainsnk_state::bgram_w)
{
	m_bgram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}



void mainsnk_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int scrollx, int scrolly )
{
	gfx_element *gfx = m_gfxdecode->gfx(1);
	const uint8_t *source, *finish;
	source =  m_spriteram;
	finish =  source + 25*4;

	while( source<finish )
	{
		int attributes = source[3];
		int tile_number = source[1];
		int sy = source[0];
		int sx = source[2];
		int color = attributes&0xf;
		int flipx = 0;
		int flipy = 0;
		if( sy>240 ) sy -= 256;

		tile_number |= attributes<<4 & 0x300;

		sx = 288-16 - sx;
		sy += 8;

		if (flip_screen())
		{
			sx = 288-16 - sx;
			sy = 224+8-16 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		gfx->transpen(bitmap,cliprect,
			tile_number,
			color,
			flipx,flipy,
			sx,sy,7);

		source+=4;
	}
}


uint32_t mainsnk_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect, 0, 0);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}
