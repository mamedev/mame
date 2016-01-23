// license:BSD-3-Clause
// copyright-holders:Lee Taylor
// thanks-to:John Clegg
/***************************************************************************

    Irem M58 hardware

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/m58.h"



/*************************************
 *
 *  Palette configuration
 *
 *************************************/

PALETTE_INIT_MEMBER(m58_state, m58)
{
	const UINT8 *color_prom = memregion("proms")->base();
	const UINT8 *char_lopal = color_prom + 0x000;
	const UINT8 *char_hipal = color_prom + 0x100;
	const UINT8 *sprite_pal = color_prom + 0x200;
	const UINT8 *sprite_table = color_prom + 0x220;
	const UINT8 *radar_lopal = color_prom + 0x320;
	const UINT8 *radar_hipal = color_prom + 0x420;
	static const int resistances_3[3] = { 1000, 470, 220 };
	static const int resistances_2[2]  = { 470, 220 };
	double weights_r[3], weights_g[3], weights_b[3], scale;
	int i;

	/* compute palette information for characters/radar */
	scale = compute_resistor_weights(0, 255, -1.0,
			2, resistances_2, weights_r, 0, 0,
			3, resistances_3, weights_g, 0, 0,
			3, resistances_3, weights_b, 0, 0);

	/* character palette */
	for (i = 0; i < 256; i++)
	{
		UINT8 promval = (char_lopal[i] & 0x0f) | (char_hipal[i] << 4);
		int r = combine_2_weights(weights_r, BIT(promval,6), BIT(promval,7));
		int g = combine_3_weights(weights_g, BIT(promval,3), BIT(promval,4), BIT(promval,5));
		int b = combine_3_weights(weights_b, BIT(promval,0), BIT(promval,1), BIT(promval,2));

		palette.set_indirect_color(i, rgb_t(r,g,b));
	}

	/* radar palette */
	for (i = 0; i < 256; i++)
	{
		UINT8 promval = (radar_lopal[i] & 0x0f) | (radar_hipal[i] << 4);
		int r = combine_2_weights(weights_r, BIT(promval,6), BIT(promval,7));
		int g = combine_3_weights(weights_g, BIT(promval,3), BIT(promval,4), BIT(promval,5));
		int b = combine_3_weights(weights_b, BIT(promval,0), BIT(promval,1), BIT(promval,2));

		palette.set_indirect_color(256+i, rgb_t(r,g,b));
	}

	/* compute palette information for sprites */
	scale = compute_resistor_weights(0, 255, scale,
			2, resistances_2, weights_r, 470, 0,
			3, resistances_3, weights_g, 470, 0,
			3, resistances_3, weights_b, 470, 0);

	/* sprite palette */
	for (i = 0; i < 16; i++)
	{
		UINT8 promval = sprite_pal[i];
		int r = combine_2_weights(weights_r, BIT(promval,6), BIT(promval,7));
		int g = combine_3_weights(weights_g, BIT(promval,3), BIT(promval,4), BIT(promval,5));
		int b = combine_3_weights(weights_b, BIT(promval,0), BIT(promval,1), BIT(promval,2));

		palette.set_indirect_color(256+256+i, rgb_t(r,g,b));
	}

	/* character lookup table */
	for (i = 0; i < 256; i++)
		palette.set_pen_indirect(i, i);

	/* radar lookup table */
	for (i = 0; i < 256; i++)
		palette.set_pen_indirect(256+i, 256+i);

	/* sprite lookup table */
	for (i = 0; i < 256; i++)
	{
		UINT8 promval = sprite_table[i] & 0x0f;
		palette.set_pen_indirect(256+256+i, 256+256+promval);
	}
}



/*************************************
 *
 *  Video RAM access
 *
 *************************************/

WRITE8_MEMBER(m58_state::videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}


WRITE8_MEMBER(m58_state::scroll_panel_w)
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

		m_scroll_panel_bitmap.pix16(sy, sx + i) = 0x100 + (sy & 0xfc) + col;
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

	SET_TILE_INFO_MEMBER(0, code, color, flags);
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
	int width = m_screen->width();
	int height = m_screen->height();
	const rectangle &visarea = m_screen->visible_area();

	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(m58_state::get_bg_tile_info),this), tilemap_mapper_delegate(FUNC(m58_state::tilemap_scan_rows),this), 8, 8, 64, 32);
	m_bg_tilemap->set_scrolldx(visarea.min_x, width - (visarea.max_x + 1));
	m_bg_tilemap->set_scrolldy(visarea.min_y - 8, height + 16 - (visarea.max_y + 1));

	m_screen->register_screen_bitmap(m_scroll_panel_bitmap);
	save_item(NAME(m_scroll_panel_bitmap));
}



/*************************************
 *
 *  Outputs
 *
 *************************************/

WRITE8_MEMBER(m58_state::flipscreen_w)
{
	/* screen flip is handled both by software and hardware */
	flip_screen_set((data & 0x01) ^ (~ioport("DSW2")->read() & 0x01));

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
		int sy1 = 233 - m_spriteram[offs];
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

UINT32 m58_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0, (*m_scroll_x_high * 0x100) + *m_scroll_x_low);
	m_bg_tilemap->set_scrolly(0, *m_scroll_y_low);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	draw_panel(bitmap, cliprect);
	return 0;
}
