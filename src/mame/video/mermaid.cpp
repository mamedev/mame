// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
#include "emu.h"
#include "includes/mermaid.h"


void mermaid_state::common_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();
	for (int i = 0; i < 0x40; i++)
	{
		int const r = 0x21 * BIT(color_prom[i], 0) + 0x47 * BIT(color_prom[i], 1) + 0x97 * BIT(color_prom[i], 2);
		int const g = 0x21 * BIT(color_prom[i], 3) + 0x47 * BIT(color_prom[i], 4) + 0x97 * BIT(color_prom[i], 5);
		int const b =                                0x47 * BIT(color_prom[i], 6) + 0x97 * BIT(color_prom[i], 7);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}
}

void mermaid_state::mermaid_palette(palette_device &palette) const
{
	common_palette(palette);

	// blue background
	palette.set_indirect_color(0x40, rgb_t(0, 0, 0xff));

	// char/sprite palette
	for (int i = 0; i < 0x40; i++)
		palette.set_pen_indirect(i, i);

	// background palette
	palette.set_pen_indirect(0x40, 0x20);
	palette.set_pen_indirect(0x41, 0x21);
	palette.set_pen_indirect(0x42, 0x40);
	palette.set_pen_indirect(0x43, 0x21);
}

void mermaid_state::rougien_palette(palette_device &palette) const
{
	common_palette(palette);

	// black background
	palette.set_indirect_color(0x40, rgb_t(0, 0, 0));

	// char/sprite palette
	for (int i = 0; i < 0x40; i++)
		palette.set_pen_indirect(i, i);

	// background palette
	palette.set_pen_indirect(0x40, 0x40);
	palette.set_pen_indirect(0x41, 0x00);
	palette.set_pen_indirect(0x42, 0x00);
	palette.set_pen_indirect(0x43, 0x02);
}


WRITE8_MEMBER(mermaid_state::mermaid_videoram2_w)
{
	m_videoram2[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(mermaid_state::mermaid_videoram_w)
{
	m_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(mermaid_state::mermaid_colorram_w)
{
	m_colorram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

WRITE_LINE_MEMBER(mermaid_state::flip_screen_x_w)
{
	flip_screen_x_set(state);
}

WRITE_LINE_MEMBER(mermaid_state::flip_screen_y_w)
{
	flip_screen_y_set(state);
}

WRITE8_MEMBER(mermaid_state::mermaid_bg_scroll_w)
{
	m_bg_scrollram[offset] = data;
	m_bg_tilemap->set_scrolly(offset, data);
}

WRITE8_MEMBER(mermaid_state::mermaid_fg_scroll_w)
{
	m_fg_scrollram[offset] = data;
	m_fg_tilemap->set_scrolly(offset, data);
}

WRITE_LINE_MEMBER(mermaid_state::rougien_gfxbankswitch1_w)
{
	m_rougien_gfxbank1 = state;
}

WRITE_LINE_MEMBER(mermaid_state::rougien_gfxbankswitch2_w)
{
	m_rougien_gfxbank2 = state;
}

READ8_MEMBER(mermaid_state::mermaid_collision_r)
{
	/*
	    collision register active LOW:

	with coll = spriteram[offs + 2] & 0xc0

	    Bit 0 - Sprite (coll = 0x40) - Sprite (coll = 0x00)
	    Bit 1 - Sprite (coll = 0x40) - Foreground
	    Bit 2 - Sprite (coll = 0x40) - Background
	    Bit 3 - Sprite (coll = 0x80) - Sprite (coll = 0x00)
	    Bit 4
	    Bit 5
	    Bit 6 - Sprite (coll = 0x40) - Sprite (coll = 0x80)
	    Bit 7
	*/

	int collision = 0xff;

	if (m_coll_bit0) collision ^= 0x01;
	if (m_coll_bit1) collision ^= 0x02;
	if (m_coll_bit2) collision ^= 0x04;
	if (m_coll_bit3) collision ^= 0x08;
	if (m_coll_bit6) collision ^= 0x40;

	return collision;
}

TILE_GET_INFO_MEMBER(mermaid_state::get_bg_tile_info)
{
	int code = m_videoram2[tile_index];
	int sx = tile_index % 32;
	int color = (sx >= 26) ? 0 : 1;

	SET_TILE_INFO_MEMBER(2, code, color, 0);
}

TILE_GET_INFO_MEMBER(mermaid_state::get_fg_tile_info)
{
	int attr = m_colorram[tile_index];
	int code = m_videoram[tile_index] + ((attr & 0x30) << 4);
	int color = attr & 0x0f;
	int flags = TILE_FLIPYX((attr & 0xc0) >> 6);

	code |= m_rougien_gfxbank1 * 0x2800;
	code |= m_rougien_gfxbank2 * 0x2400;

	SET_TILE_INFO_MEMBER(0, code, color, flags);
}

void mermaid_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(mermaid_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap->set_scroll_cols(32);

	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(mermaid_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_fg_tilemap->set_scroll_cols(32);
	m_fg_tilemap->set_transparent_pen(0);

	m_screen->register_screen_bitmap(m_helper);
	m_screen->register_screen_bitmap(m_helper2);
}

void mermaid_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	const rectangle spritevisiblearea(0 * 8, 26 * 8 - 1, 2 * 8, 30 * 8 - 1);
	const rectangle flip_spritevisiblearea(6 * 8, 31 * 8 - 1, 2 * 8, 30 * 8 - 1);

	uint8_t *spriteram = m_spriteram;
	int offs;

	for (offs = m_spriteram.bytes() - 4; offs >= 0; offs -= 4)
	{
		int attr = spriteram[offs + 2];
		int bank = (attr & 0x30) >> 4;
		int code = (spriteram[offs] & 0x3f) | (bank << 6);
		int color = attr & 0x0f;
		int flipx = spriteram[offs] & 0x40;
		int flipy = spriteram[offs] & 0x80;
		int sx = spriteram[offs + 3] + 1;
		int sy = 240 - spriteram[offs + 1];

		if (sx >= 0xf0) sx -= 256;

		code |= m_rougien_gfxbank1 * 0x2800;
		code |= m_rougien_gfxbank2 * 0x2400;

		if (flip_screen_x())
		{
			flipx = !flipx;
			sx = 240 - sx;
		}

		if (flip_screen_y())
		{
			flipy = !flipy;
			sy = 240 - sy;
		}


			m_gfxdecode->gfx(1)->transpen(bitmap,(flip_screen_x() ? flip_spritevisiblearea : spritevisiblearea), code, color, flipx, flipy, sx, sy, 0);
	}
}

uint32_t mermaid_state::screen_update_mermaid(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}

uint8_t mermaid_state::collision_check( rectangle& rect )
{
	uint8_t data = 0;

	int x;
	int y;

	for (y = rect.top(); y <= rect.bottom(); y++)
		for (x = rect.left(); x <= rect.right(); x++)
		{
			uint16_t a = m_palette->pen_indirect(m_helper.pix16(y, x)) & 0x3f;
			uint16_t b = m_palette->pen_indirect(m_helper2.pix16(y, x)) & 0x3f;

			if (b)
				if (a)
					data |= 0x01;
		}

	return data;
}

void mermaid_state::collision_update()
{
	const rectangle &visarea = m_screen->visible_area();
	uint8_t *spriteram = m_spriteram;

	int offs, offs2;

	m_coll_bit0 = 0;
	m_coll_bit1 = 0;
	m_coll_bit2 = 0;
	m_coll_bit3 = 0;
	m_coll_bit6 = 0;

	// check for bit 0 (sprite-sprite), 1 (sprite-foreground), 2 (sprite-background)

	for (offs = m_spriteram.bytes() - 4; offs >= 0; offs -= 4)
	{
		int attr = spriteram[offs + 2];
		int bank = (attr & 0x30) >> 4;
		int coll = (attr & 0xc0) >> 6;
		int code = (spriteram[offs] & 0x3f) | (bank << 6);
		int flipx = spriteram[offs] & 0x40;
		int flipy = spriteram[offs] & 0x80;
		int sx = spriteram[offs + 3] + 1;
		int sy = 240 - spriteram[offs + 1];

		rectangle rect;

		if (coll != 1) continue;

		code |= m_rougien_gfxbank1 * 0x2800;
		code |= m_rougien_gfxbank2 * 0x2400;

		if (flip_screen_x())
		{
			flipx = !flipx;
			sx = 240 - sx;
		}

		if (flip_screen_y())
		{
			flipy = !flipy;
			sy = 240 - sy;
		}

		rect.set(
				sx, sx + m_gfxdecode->gfx(1)->width() - 1,
				sy, sy + m_gfxdecode->gfx(1)->height() - 1);
		rect &= visarea;

		// check collision sprite - background

		m_helper.fill(0, rect);
		m_helper2.fill(0, rect);

		m_bg_tilemap->draw(*m_screen, m_helper, rect, 0, 0);

		m_gfxdecode->gfx(1)->transpen(m_helper2,rect, code, 0, flipx, flipy, sx, sy, 0);

		m_coll_bit2 |= collision_check(rect);

		// check collision sprite - foreground

		m_helper.fill(0, rect);
		m_helper2.fill(0, rect);

		m_fg_tilemap->draw(*m_screen, m_helper, rect, 0, 0);

		m_gfxdecode->gfx(1)->transpen(m_helper2,rect, code, 0, flipx, flipy, sx, sy, 0);

		m_coll_bit1 |= collision_check(rect);

		// check collision sprite - sprite

		m_helper.fill(0, rect);
		m_helper2.fill(0, rect);

		for (offs2 = m_spriteram.bytes() - 4; offs2 >= 0; offs2 -= 4)
			if (offs != offs2)
			{
				int attr2 = spriteram[offs2 + 2];
				int bank2 = (attr2 & 0x30) >> 4;
				int coll2 = (attr2 & 0xc0) >> 6;
				int code2 = (spriteram[offs2] & 0x3f) | (bank2 << 6);
				int flipx2 = spriteram[offs2] & 0x40;
				int flipy2 = spriteram[offs2] & 0x80;
				int sx2 = spriteram[offs2 + 3] + 1;
				int sy2 = 240 - spriteram[offs2 + 1];

				if (coll2 != 0) continue;

				code2 |= m_rougien_gfxbank1 * 0x2800;
				code2 |= m_rougien_gfxbank2 * 0x2400;

				if (flip_screen_x())
				{
					flipx2 = !flipx2;
					sx2 = 240 - sx2;
				}

				if (flip_screen_y())
				{
					flipy2 = !flipy2;
					sy2 = 240 - sy2;
				}

				m_gfxdecode->gfx(1)->transpen(m_helper,rect, code2, 0, flipx2, flipy2, sx2, sy2, 0);
			}

		m_gfxdecode->gfx(1)->transpen(m_helper2,rect, code, 0, flipx, flipy, sx, sy, 0);

		m_coll_bit0 |= collision_check(rect);
	}

	// check for bit 3 (sprite-sprite)

	for (offs = m_spriteram.bytes() - 4; offs >= 0; offs -= 4)
	{
		int attr = spriteram[offs + 2];
		int bank = (attr & 0x30) >> 4;
		int coll = (attr & 0xc0) >> 6;
		int code = (spriteram[offs] & 0x3f) | (bank << 6);
		int flipx = spriteram[offs] & 0x40;
		int flipy = spriteram[offs] & 0x80;
		int sx = spriteram[offs + 3] + 1;
		int sy = 240 - spriteram[offs + 1];

		rectangle rect;

		if (coll != 2) continue;

		code |= m_rougien_gfxbank1 * 0x2800;
		code |= m_rougien_gfxbank2 * 0x2400;

		if (flip_screen_x())
		{
			flipx = !flipx;
			sx = 240 - sx;
		}

		if (flip_screen_y())
		{
			flipy = !flipy;
			sy = 240 - sy;
		}

		rect.set(
				sx, sx + m_gfxdecode->gfx(1)->width() - 1,
				sy, sy + m_gfxdecode->gfx(1)->height() - 1);
		rect &= visarea;

		// check collision sprite - sprite

		m_helper.fill(0, rect);
		m_helper2.fill(0, rect);

		for (offs2 = m_spriteram.bytes() - 4; offs2 >= 0; offs2 -= 4)
			if (offs != offs2)
			{
				int attr2 = spriteram[offs2 + 2];
				int bank2 = (attr2 & 0x30) >> 4;
				int coll2 = (attr2 & 0xc0) >> 6;
				int code2 = (spriteram[offs2] & 0x3f) | (bank2 << 6);
				int flipx2 = spriteram[offs2] & 0x40;
				int flipy2 = spriteram[offs2] & 0x80;
				int sx2 = spriteram[offs2 + 3] + 1;
				int sy2 = 240 - spriteram[offs2 + 1];

				if (coll2 != 0) continue;

				code2 |= m_rougien_gfxbank1 * 0x2800;
				code2 |= m_rougien_gfxbank2 * 0x2400;

				if (flip_screen_x())
				{
					flipx2 = !flipx2;
					sx2 = 240 - sx2;
				}

				if (flip_screen_y())
				{
					flipy2 = !flipy2;
					sy2 = 240 - sy2;
				}

				m_gfxdecode->gfx(1)->transpen(m_helper,rect, code2, 0, flipx2, flipy2, sx2, sy2, 0);
			}

		m_gfxdecode->gfx(1)->transpen(m_helper2,rect, code, 0, flipx, flipy, sx, sy, 0);

		m_coll_bit3 |= collision_check(rect);
	}

	// check for bit 6

	for (offs = m_spriteram.bytes() - 4; offs >= 0; offs -= 4)
	{
		int attr = spriteram[offs + 2];
		int bank = (attr & 0x30) >> 4;
		int coll = (attr & 0xc0) >> 6;
		int code = (spriteram[offs] & 0x3f) | (bank << 6);
		int flipx = spriteram[offs] & 0x40;
		int flipy = spriteram[offs] & 0x80;
		int sx = spriteram[offs + 3] + 1;
		int sy = 240 - spriteram[offs + 1];

		rectangle rect;

		if (coll != 1) continue;

		code |= m_rougien_gfxbank1 * 0x2800;
		code |= m_rougien_gfxbank2 * 0x2400;

		if (flip_screen_x())
		{
			flipx = !flipx;
			sx = 240 - sx;
		}

		if (flip_screen_y())
		{
			flipy = !flipy;
			sy = 240 - sy;
		}

		rect.set(
				sx, sx + m_gfxdecode->gfx(1)->width() - 1,
				sy, sy + m_gfxdecode->gfx(1)->height() - 1);
		rect &= visarea;

		// check collision sprite - sprite

		m_helper.fill(0, rect);
		m_helper2.fill(0, rect);

		for (offs2 = m_spriteram.bytes() - 4; offs2 >= 0; offs2 -= 4)
			if (offs != offs2)
			{
				int attr2 = spriteram[offs2 + 2];
				int bank2 = (attr2 & 0x30) >> 4;
				int coll2 = (attr2 & 0xc0) >> 6;
				int code2 = (spriteram[offs2] & 0x3f) | (bank2 << 6);
				int flipx2 = spriteram[offs2] & 0x40;
				int flipy2 = spriteram[offs2] & 0x80;
				int sx2 = spriteram[offs2 + 3] + 1;
				int sy2 = 240 - spriteram[offs2 + 1];

				if (coll2 != 2) continue;

				code2 |= m_rougien_gfxbank1 * 0x2800;
				code2 |= m_rougien_gfxbank2 * 0x2400;

				if (flip_screen_x())
				{
					flipx2 = !flipx2;
					sx2 = 240 - sx2;
				}

				if (flip_screen_y())
				{
					flipy2 = !flipy2;
					sy2 = 240 - sy2;
				}

				m_gfxdecode->gfx(1)->transpen(m_helper,rect, code2, 0, flipx2, flipy2, sx2, sy2, 0);
			}

		m_gfxdecode->gfx(1)->transpen(m_helper2,rect, code, 0, flipx, flipy, sx, sy, 0);

		m_coll_bit6 |= collision_check(rect);
	}
}

WRITE_LINE_MEMBER(mermaid_state::screen_vblank_mermaid)
{
	// rising edge
	if (state)
	{
		collision_update();

		if (m_nmi_mask)
			m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	}
}
