// license:BSD-3-Clause
// copyright-holders:Paul Leaman, Couriersud
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "1942.h"

/***************************************************************************

  Convert the color PROMs into a more useable format.

  1942 has three 256x4 palette PROMs (one per gun) and three 256x4 lookup
  table PROMs (one for characters, one for sprites, one for background tiles).
  The palette PROMs are connected to the RGB output this way:

  bit 3 -- 220 ohm resistor  -- RED/GREEN/BLUE
        -- 470 ohm resistor  -- RED/GREEN/BLUE
        -- 1  kohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 2.2kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/

void _1942_state::create_palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("palproms")->base();

	for (int i = 0; i < 256; i++)
	{
		// red component
		int bit0 = BIT(color_prom[i + 0 * 256], 0);
		int bit1 = BIT(color_prom[i + 0 * 256], 1);
		int bit2 = BIT(color_prom[i + 0 * 256], 2);
		int bit3 = BIT(color_prom[i + 0 * 256], 3);
		int const r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		// green component
		bit0 = BIT(color_prom[i + 1 * 256], 0);
		bit1 = BIT(color_prom[i + 1 * 256], 1);
		bit2 = BIT(color_prom[i + 1 * 256], 2);
		bit3 = BIT(color_prom[i + 1 * 256], 3);
		int const g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		// blue component
		bit0 = BIT(color_prom[i + 2 * 256], 0);
		bit1 = BIT(color_prom[i + 2 * 256], 1);
		bit2 = BIT(color_prom[i + 2 * 256], 2);
		bit3 = BIT(color_prom[i + 2 * 256], 3);
		int const b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}
}

void _1942_state::_1942_palette(palette_device &palette) const
{
	create_palette(palette);

	/* characters use palette entries 128-143 */
	int colorbase = 0;
	const uint8_t *charlut_prom = memregion("charprom")->base();
	for (int i = 0; i < 64 * 4; i++)
		palette.set_pen_indirect(colorbase + i, 0x80 | charlut_prom[i]);

	// background tiles use palette entries 0-63 in four banks
	colorbase += 64 * 4;
	const uint8_t *tilelut_prom = memregion("tileprom")->base();
	for (int i = 0; i < 32 * 8; i++)
	{
		palette.set_pen_indirect(colorbase + 0 * 32 * 8 + i, 0x00 | tilelut_prom[i]);
		palette.set_pen_indirect(colorbase + 1 * 32 * 8 + i, 0x10 | tilelut_prom[i]);
		palette.set_pen_indirect(colorbase + 2 * 32 * 8 + i, 0x20 | tilelut_prom[i]);
		palette.set_pen_indirect(colorbase + 3 * 32 * 8 + i, 0x30 | tilelut_prom[i]);
	}

	// sprites use palette entries 64-79
	colorbase += 4 * 32 * 8;
	const uint8_t *sprlut_prom = memregion("sprprom")->base();
	for (int i = 0; i < 16 * 16; i++)
		palette.set_pen_indirect(colorbase + i, 0x40 | sprlut_prom[i]);
}

void _1942p_state::_1942p_palette(palette_device &palette) const
{
	for (int i = 0; i < 0x400; i++)
		palette.set_pen_indirect(i, i);

	uint8_t const *const color_prom = memregion("proms")->base();
	for (int i = 0; i < 0x100; i++)
		palette.set_pen_indirect(i + 0x400, color_prom[i] | 0x240);
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(_1942_state::get_fg_tile_info)
{
	int code = m_fg_videoram[tile_index];
	int color = m_fg_videoram[tile_index + 0x400];
	tileinfo.set(0,
			code + ((color & 0x80) << 1),
			color & 0x3f,
			0);
}

TILE_GET_INFO_MEMBER(_1942_state::get_bg_tile_info)
{
	tile_index = (tile_index & 0x0f) | ((tile_index & 0x01f0) << 1);

	int code = m_bg_videoram[tile_index];
	int color = m_bg_videoram[tile_index + 0x10];
	tileinfo.set(1,
			code + ((color & 0x80) << 1),
			(color & 0x1f) + (0x20 * m_palette_bank),
			TILE_FLIPYX((color & 0x60) >> 5));
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/
void _1942_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(_1942_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(_1942_state::get_bg_tile_info)), TILEMAP_SCAN_COLS, 16, 16, 32, 16);

	m_fg_tilemap->set_transparent_pen(0);

	m_bg_tilemap->set_scrolldx(128, 128);
	m_bg_tilemap->set_scrolldy(  6,   6);
	m_fg_tilemap->set_scrolldx(128, 128);
	m_fg_tilemap->set_scrolldy(  6,   6);
}

void _1942p_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(_1942_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(_1942_state::get_bg_tile_info)), TILEMAP_SCAN_COLS, 16, 16, 32, 16);

	m_fg_tilemap->set_transparent_pen(3);

	m_bg_tilemap->set_scrolldx(128, 128);
	m_bg_tilemap->set_scrolldy(  6,   6);
	m_fg_tilemap->set_scrolldx(128, 128);
	m_fg_tilemap->set_scrolldy(  6,   6);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

void _1942_state::fgvideoram_w(offs_t offset, uint8_t data)
{
	m_fg_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

void _1942_state::bgvideoram_w(offs_t offset, uint8_t data)
{
	m_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty((offset & 0x0f) | ((offset >> 1) & 0x01f0));
}


void _1942_state::palette_bank_w(uint8_t data)
{
	if (m_palette_bank != data)
	{
		m_palette_bank = data & 3;
		m_bg_tilemap->mark_all_dirty();
	}
}

void _1942_state::scroll_w(offs_t offset, uint8_t data)
{
	m_scroll[offset] = data;
	m_bg_tilemap->set_scrollx(0, m_scroll[0] | (m_scroll[1] << 8));
}


void _1942_state::control_w(uint8_t data)
{
	/* bit 7: flip screen
	   bit 4: cpu B reset
	   bit 0: coin counter */

	machine().bookkeeping().coin_counter_w(0,data & 0x01);

	m_audiocpu->set_input_line(INPUT_LINE_RESET, (data & 0x10) ? ASSERT_LINE : CLEAR_LINE);

	flip_screen_set(data & 0x80);
}


/***************************************************************************

  Display refresh

***************************************************************************/

void _1942_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// Sprites 0 to 15 are drawn on all scanlines.
	// Sprites 16 to 23 are drawn on scanlines 16 to 127.
	// Sprites 24 to 31 are drawn on scanlines 128 to 239.
	//
	// The reason for this is ostensibly so that the back half of the sprite list can
	// be used to selectively mask sprites along the midpoint of the screen.
	//
	// Moreover, the H counter runs from 128 to 511 for a total of 384 horizontal
	// clocks per scanline. With an effective 6MHz pixel clock, this produces a
	// horizontal scan rate of exactly 15.625kHz, a standard scan rate for games
	// of this era.
	//
	// Sprites are drawn by MAME in reverse order, as the actual hardware only
	// permits a transparent pixel to be overwritten by an opaque pixel, and does
	// not support opaque-opaque overwriting - i.e., the first sprite to draw wins
	// control over its horizontal range. If MAME drew in forward order, it would
	// instead produce a last-sprite-wins behavior.

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		const rectangle cliprecty(cliprect.min_x, cliprect.max_x, y, y);
		uint8_t objdata[4];
		uint8_t v = flip_screen() ? ~(y - 1 - 6) : y - 1 - 6;
		for (int h = 496; h >= 128; h -= 16)
		{
			const bool objcnt4 = BIT(h, 8) != BIT(~h, 7);
			const bool objcnt3 = (BIT(v, 7) && objcnt4) != BIT(~h, 7);
			uint8_t obj_idx = (h >> 4) & 7;
			obj_idx |= objcnt3 ? 0x08 : 0x00;
			obj_idx |= objcnt4 ? 0x10 : 0x00;
			obj_idx <<= 2;
			for (int i = 0; i < 4; i++)
				objdata[i] = m_spriteram[obj_idx | i];

			int code = (objdata[0] & 0x7f) + ((objdata[1] & 0x20) << 2) + ((objdata[0] & 0x80) << 1);
			int col = objdata[1] & 0x0f;
			int sx = objdata[3] - 0x10 * (objdata[1] & 0x10);
			int sy = objdata[2];
			int dir = 1;

			uint8_t valpha = (uint8_t)sy;
			uint8_t v2c = (uint8_t)(~v) + (flip_screen() ? 0x01 : 0xff);
			uint8_t lvbeta = v2c + valpha;
			uint8_t vbeta = ~lvbeta;
			bool vleq = vbeta <= ((~valpha) & 0xff);
			bool vinlen = true;
			uint8_t vlen = objdata[1] >> 6;
			switch (vlen & 3)
			{
			case 0:
				vinlen = BIT(lvbeta, 7) && BIT(lvbeta, 6) && BIT(lvbeta, 5) && BIT(lvbeta, 4);
				break;
			case 1:
				vinlen = BIT(lvbeta, 7) && BIT(lvbeta, 6) && BIT(lvbeta, 5);
				break;
			case 2:
				vinlen = BIT(lvbeta, 7) && BIT(lvbeta, 6);
				break;
			case 3:
				vinlen = true;
				break;
			}
			bool vinzone = !(vleq && vinlen);

			if (flip_screen())
			{
				sx = 240 - sx;
				sy = 240 - sy;
				dir = -1;
			}

			/* handle double / quadruple height */
			int i = (objdata[1] & 0xc0) >> 6;
			if (i == 2)
				i = 3;

			if (!vinzone)
			{
				do
				{
					m_gfxdecode->gfx(2)->transpen(bitmap, cliprecty, code + i, col, flip_screen(), flip_screen(), sx+128, sy + 6 + 16 * i * dir, 15);
				} while (i-- > 0);
			}
		}
	}
}

uint32_t _1942_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


void _1942p_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = m_spriteram.bytes() - 4; offs >= 0; offs -= 4)
	{
		int code = (m_spriteram[offs] & 0x7f) + 4 * (m_spriteram[offs + 3] & 0x20)
					+ 2 * (m_spriteram[offs] & 0x80);
		int col = m_spriteram[offs + 3] & 0x0f;

		int sx = m_spriteram[offs + 2] - 0x10 * (m_spriteram[offs + 3] & 0x10);
		int sy = m_spriteram[offs + 1];

		if (flip_screen())
		{
			sx = 240 - sx;
		}
		else
		{
			sy = 240 - sy;
		}

		m_gfxdecode->gfx(2)->transpen(bitmap, cliprect, code, col, flip_screen(), flip_screen(), sx+128, sy+6, 15);
	}
}
