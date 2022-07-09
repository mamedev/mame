// license:BSD-3-Clause
// copyright-holders:Lee Taylor
// thanks-to:John Clegg
/***************************************************************************

    Irem M58 hardware

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "m58.h"



/*************************************
 *
 *  Palette configuration
 *
 *************************************/

void m58_state::m58_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();
	uint8_t const *const char_lopal = color_prom + 0x000;
	uint8_t const *const char_hipal = color_prom + 0x100;
	uint8_t const *const sprite_pal = color_prom + 0x200;
	uint8_t const *const sprite_table = color_prom + 0x220;
	uint8_t const *const radar_lopal = color_prom + 0x320;
	uint8_t const *const radar_hipal = color_prom + 0x420;
	static constexpr int resistances_3[3] = { 1000, 470, 220 };
	static constexpr int resistances_2[2]  = { 470, 220 };
	double weights_r[3], weights_g[3], weights_b[3], scale;

	// compute palette information for characters/radar
	scale = compute_resistor_weights(0, 255, -1.0,
			2, resistances_2, weights_r, 0, 0,
			3, resistances_3, weights_g, 0, 0,
			3, resistances_3, weights_b, 0, 0);

	// character palette
	for (int i = 0; i < 256; i++)
	{
		uint8_t const promval = (char_lopal[i] & 0x0f) | (char_hipal[i] << 4);
		int const r = combine_weights(weights_r, BIT(promval,6), BIT(promval,7));
		int const g = combine_weights(weights_g, BIT(promval,3), BIT(promval,4), BIT(promval,5));
		int const b = combine_weights(weights_b, BIT(promval,0), BIT(promval,1), BIT(promval,2));

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	// radar palette
	for (int i = 0; i < 256; i++)
	{
		uint8_t const promval = (radar_lopal[i] & 0x0f) | (radar_hipal[i] << 4);
		int const r = combine_weights(weights_r, BIT(promval,6), BIT(promval,7));
		int const g = combine_weights(weights_g, BIT(promval,3), BIT(promval,4), BIT(promval,5));
		int const b = combine_weights(weights_b, BIT(promval,0), BIT(promval,1), BIT(promval,2));

		palette.set_indirect_color(256 + i, rgb_t(r, g, b));
	}

	// compute palette information for sprites
	scale = compute_resistor_weights(0, 255, scale,
			2, resistances_2, weights_r, 470, 0,
			3, resistances_3, weights_g, 470, 0,
			3, resistances_3, weights_b, 470, 0);

	// sprite palette
	for (int i = 0; i < 16; i++)
	{
		uint8_t const promval = sprite_pal[i];
		int const r = combine_weights(weights_r, BIT(promval,6), BIT(promval,7));
		int const g = combine_weights(weights_g, BIT(promval,3), BIT(promval,4), BIT(promval,5));
		int const b = combine_weights(weights_b, BIT(promval,0), BIT(promval,1), BIT(promval,2));

		palette.set_indirect_color(256 + 256 + i, rgb_t(r, g, b));
	}

	// character lookup table
	for (int i = 0; i < 256; i++)
		palette.set_pen_indirect(i, i);

	// radar lookup table
	for (int i = 0; i < 256; i++)
		palette.set_pen_indirect(256 + i, 256 + i);

	// sprite lookup table
	for (int i = 0; i < 256; i++)
	{
		uint8_t const promval = sprite_table[i] & 0x0f;
		palette.set_pen_indirect(256 + 256 + i, 256 + 256 + promval);
	}
}



/*************************************
 *
 *  Video RAM access
 *
 *************************************/

void m58_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}


void m58_state::scroll_panel_w(offs_t offset, uint8_t data)
{
	int sx = ( offset % 16 );
	int sy = ( offset / 16 );

	if (sx < 1 || sx > 14)
		return;

	sx = 4 * (sx - 1);

	for (int i = 0;i < 4;i++)
	{
		int col;

		col = (data >> i) & 0x11;
		col = ((col >> 3) | col) & 3;

		m_scroll_panel_bitmap.pix(sy, sx + i) = 0x100 + (sy & 0xfc) + col;
		m_scroll_panel_bitmap.pix(sy, sx + i + 0x2c8) = 0x100 + (sy & 0xfc) + col; // for flipscreen
	}
}



/*************************************
 *
 *  Tilemap info callback
 *
 *************************************/

TILE_GET_INFO_MEMBER(m58_state::get_bg_tile_info)
{
	int offs = tile_index * 2;
	int attr = m_videoram[offs + 1];
	int code = m_videoram[offs] + ((attr & 0xc0) << 2);
	int color = attr & 0x1f;
	int flags = (attr & 0x20) ? TILE_FLIPX : 0;

	tileinfo.set(0, code, color, flags);
}


TILEMAP_MAPPER_MEMBER(m58_state::tilemap_scan_rows)
{
	/* logical (col,row) -> memory offset */
	if (col >= 32)
		return (row + 32) * 32 + col - 32;
	else
		return row * 32 + col;
}



/*************************************
 *
 *  Video startup
 *
 *************************************/

void m58_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(m58_state::get_bg_tile_info)), tilemap_mapper_delegate(*this, FUNC(m58_state::tilemap_scan_rows)), 8, 8, 64, 32);
	m_bg_tilemap->set_scrolldy(26, 26);

	m_screen->register_screen_bitmap(m_scroll_panel_bitmap);
	save_item(NAME(m_scroll_panel_bitmap));
}



/*************************************
 *
 *  Outputs
 *
 *************************************/

void m58_state::flipscreen_w(uint8_t data)
{
	/* screen flip is handled both by software and hardware */
	flip_screen_set(BIT(data, 0) ^ BIT(~ioport("DSW2")->read(), 0));

	machine().bookkeeping().coin_counter_w(0, data & 0x02);
	machine().bookkeeping().coin_counter_w(1, data & 0x20);
}



/*************************************
 *
 *  Sprite rendering
 *
 *************************************/

void m58_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	const rectangle &visarea = m_screen->visible_area();

	for (int offs = m_spriteram.bytes() - 4; offs >= 0; offs -= 4)
	{
		int attr = m_spriteram[offs + 1];
		int bank = (attr & 0x20) >> 5;
		int code1 = m_spriteram[offs + 2] & 0xbf;
		int code2 = 0;
		int color = attr & 0x1f;
		int flipx = attr & 0x40;
		int flipy = attr & 0x80;
		int sx = m_spriteram[offs + 3];
		int sy1 = 210 - m_spriteram[offs];
		int sy2 = 0;

		if (flipy)
		{
			code2 = code1;
			code1 += 0x40;
		}
		else
		{
			code2 = code1 + 0x40;
		}

		if (flip_screen())
		{
			sx = 240 - sx;
			sy2 = 192 - sy1;
			sy1 = sy2 + 0x10;
			flipx = !flipx;
			flipy = !flipy;
		}
		else
		{
			sy2 = sy1 + 0x10;
		}

		m_gfxdecode->gfx(1)->transmask(bitmap, cliprect,
			code1 + 256 * bank, color,
			flipx, flipy, sx, visarea.min_y + sy1,
			m_palette->transpen_mask(*m_gfxdecode->gfx(1), color, 512)
		);
		m_gfxdecode->gfx(1)->transmask(bitmap, cliprect,
			code2 + 256 * bank, color,
			flipx, flipy, sx, visarea.min_y + sy2,
			m_palette->transpen_mask(*m_gfxdecode->gfx(1), color, 512)
		);
	}
}



/*************************************
 *
 *  Radar panel rendering
 *
 *************************************/

void m58_state::draw_panel( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	if (!*m_score_panel_disabled)
	{
		const rectangle clippanel(26*8, 32*8-1, 1*8, 31*8-1);
		const rectangle clippanelflip(0*8, 6*8-1, 1*8, 31*8-1);
		rectangle clip = flip_screen() ? clippanelflip : clippanel;
		const rectangle &visarea = m_screen->visible_area();
		int sx = flip_screen() ? cliprect.min_x - 8 : cliprect.max_x + 1 - 14*4;
		int yoffs = flip_screen() ? -40 : -16;

		clip.min_y += visarea.min_y + yoffs;
		clip.max_y += visarea.max_y + yoffs;
		clip &= cliprect;

		copybitmap(bitmap, m_scroll_panel_bitmap, flip_screen(), flip_screen(), sx, visarea.min_y + yoffs, clip);
	}
}



/*************************************
 *
 *  Video update
 *
 *************************************/

uint32_t m58_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0, (*m_scroll_x_high * 0x100) + *m_scroll_x_low);
	m_bg_tilemap->set_scrolly(0, *m_scroll_y_low);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	draw_panel(bitmap, cliprect);
	return 0;
}
