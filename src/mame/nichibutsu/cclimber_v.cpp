// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

  cclimber.cpp

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "cclimber.h"

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Crazy Climber has three 32x8 palette PROMs.
  The palette PROMs are connected to the RGB output this way:

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

***************************************************************************/

void cclimber_state::cclimber_palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();
	static constexpr int resistances_rg[3] = { 1000, 470, 220 };
	static constexpr int resistances_b [2] = { 470, 220 };

	// compute the color output resistor weights
	double weights_rg[3], weights_b[2];
	compute_resistor_weights(0, 255, -1.0,
			3, resistances_rg, weights_rg, 0, 0,
			2, resistances_b,  weights_b,  0, 0,
			0, nullptr, nullptr, 0, 0);

	for (int i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int const r = combine_weights(weights_rg, bit0, bit1, bit2);

		// green component
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = combine_weights(weights_rg, bit0, bit1, bit2);

		// blue component
		bit0 = BIT(color_prom[i], 6);
		bit1 = BIT(color_prom[i], 7);
		int const b = combine_weights(weights_b, bit0, bit1);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Swimmer has two 256x4 char/sprite palette PROMs and one 32x8 big sprite
  palette PROM.
  The palette PROMs are connected to the RGB output this way:
  (the 500 and 250 ohm resistors are made of 1 kohm resistors in parallel)

  bit 3 -- 250 ohm resistor  -- BLUE
        -- 500 ohm resistor  -- BLUE
        -- 250 ohm resistor  -- GREEN
  bit 0 -- 500 ohm resistor  -- GREEN
  bit 3 -- 1  kohm resistor  -- GREEN
        -- 250 ohm resistor  -- RED
        -- 500 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

  bit 7 -- 250 ohm resistor  -- BLUE
        -- 500 ohm resistor  -- BLUE
        -- 250 ohm resistor  -- GREEN
        -- 500 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 250 ohm resistor  -- RED
        -- 500 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

  Additionally, the background color of the score panel is determined by
  these resistors:

                  /--- tri-state --  470 -- BLUE
  +5V -- 1kohm ------- tri-state --  390 -- GREEN
                  \--- tri-state -- 1000 -- RED

***************************************************************************/

void swimmer_state::swimmer_palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();

	for (int i = 0; i < 0x100; i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(color_prom[i + 0x000], 0);
		bit1 = BIT(color_prom[i + 0x000], 1);
		bit2 = BIT(color_prom[i + 0x000], 2);
		int const r = 0x20 * bit0 + 0x40 * bit1 + 0x80 * bit2;

		// green component
		bit0 = BIT(color_prom[i + 0x000], 3);
		bit1 = BIT(color_prom[i + 0x100], 0);
		bit2 = BIT(color_prom[i + 0x100], 1);
		int const g = 0x20 * bit0 + 0x40 * bit1 + 0x80 * bit2;

		// blue component
		bit0 = 0;
		bit1 = BIT(color_prom[i + 0x100], 2);
		bit2 = BIT(color_prom[i + 0x100], 3);
		int const b = 0x20 * bit0 + 0x40 * bit1 + 0x80 * bit2;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}

	color_prom += 0x200;

	// big sprite
	for (int i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int const r = 0x20 * bit0 + 0x40 * bit1 + 0x80 * bit2;

		// green component
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = 0x20 * bit0 + 0x40 * bit1 + 0x80 * bit2;

		// blue component
		bit0 = 0;
		bit1 = BIT(color_prom[i], 6);
		bit2 = BIT(color_prom[i], 7);
		int const b = 0x20 * bit0 + 0x40 * bit1 + 0x80 * bit2;

		palette.set_pen_color(i + 0x100, rgb_t(r, g, b));
	}

	// side panel backgrond pen
#if 0
	// values calculated from the resistors don't seem to match the real board
	palette.set_pen_color(m_sidepen, rgb_t(0x24, 0x5d, 0x4e));
#endif
	palette.set_pen_color(m_sidepen, rgb_t(0x20, 0x98, 0x79));
}


void yamato_state::yamato_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	// chars - 12 bits RGB
	for (int i = 0; i < 0x40; i++)
	{
		int bit0, bit1, bit2, bit3;

		// red component
		bit0 = BIT(color_prom[i + 0x00], 0);
		bit1 = BIT(color_prom[i + 0x00], 1);
		bit2 = BIT(color_prom[i + 0x00], 2);
		bit3 = BIT(color_prom[i + 0x00], 3);
		int const r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		// green component
		bit0 = BIT(color_prom[i + 0x00], 4);
		bit1 = BIT(color_prom[i + 0x00], 5);
		bit2 = BIT(color_prom[i + 0x00], 6);
		bit3 = BIT(color_prom[i + 0x00], 7);
		int const g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		// blue component
		bit0 = BIT(color_prom[i + 0x40], 0);
		bit1 = BIT(color_prom[i + 0x40], 1);
		bit2 = BIT(color_prom[i + 0x40], 2);
		bit3 = BIT(color_prom[i + 0x40], 3);
		int const b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}

	// big sprite - 8 bits RGB
	for (int i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(color_prom[i + 0x80], 0);
		bit1 = BIT(color_prom[i + 0x80], 1);
		bit2 = BIT(color_prom[i + 0x80], 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// green component
		bit0 = BIT(color_prom[i + 0x80], 3);
		bit1 = BIT(color_prom[i + 0x80], 4);
		bit2 = BIT(color_prom[i + 0x80], 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// blue component
		bit0 = 0;
		bit1 = BIT(color_prom[i + 0x80], 6);
		bit2 = BIT(color_prom[i + 0x80], 7);
		int const b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i + 0x40, rgb_t(r, g, b));
	}

	// fake colors for bg gradient
	for (int i = 0; i < 0x100; i++)
		palette.set_pen_color(YAMATO_SKY_PEN_BASE + i, rgb_t(0, 0, i));
}


void toprollr_state::toprollr_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	for (int i = 0; i < 0xa0; i++)
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


/***************************************************************************

  Swimmer can directly set the background color.
  The latch is connected to the RGB output this way:
  (the 500 and 250 ohm resistors are made of 1 kohm resistors in parallel)

  bit 7 -- 250 ohm resistor  -- RED
        -- 500 ohm resistor  -- RED
        -- 250 ohm resistor  -- GREEN
        -- 500 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 250 ohm resistor  -- BLUE
        -- 500 ohm resistor  -- BLUE
  bit 0 -- 1  kohm resistor  -- BLUE

***************************************************************************/

void swimmer_state::set_background_pen()
{
	int bit0, bit1, bit2;

	/* red component */
	bit0 = 0;
	bit1 = (*m_swimmer_background_color >> 6) & 0x01;
	bit2 = (*m_swimmer_background_color >> 7) & 0x01;
	int const r = 0x20 * bit0 + 0x40 * bit1 + 0x80 * bit2;

	/* green component */
	bit0 = (*m_swimmer_background_color >> 3) & 0x01;
	bit1 = (*m_swimmer_background_color >> 4) & 0x01;
	bit2 = (*m_swimmer_background_color >> 5) & 0x01;
	int const g = 0x20 * bit0 + 0x40 * bit1 + 0x80 * bit2;

	/* blue component */
	bit0 = (*m_swimmer_background_color >> 0) & 0x01;
	bit1 = (*m_swimmer_background_color >> 1) & 0x01;
	bit2 = (*m_swimmer_background_color >> 2) & 0x01;
	int const b = 0x20 * bit0 + 0x40 * bit1 + 0x80 * bit2;

	m_palette->set_pen_color(0, rgb_t(r, g, b));
}


void cclimber_state::cclimber_colorram_w(offs_t offset, uint8_t data)
{
	/* A5 is not connected, there is only 0x200 bytes of RAM */
	m_colorram[offset & ~0x20] = data;
	m_colorram[offset |  0x20] = data;
}


void cclimber_state::flip_screen_x_w(int state)
{
	m_flip_x = state;
}


void cclimber_state::flip_screen_y_w(int state)
{
	m_flip_y = state;
}


void swimmer_state::sidebg_enable_w(int state)
{
	m_side_background_enabled = state;
}


void swimmer_state::palette_bank_w(int state)
{
	m_palettebank = state;
}


TILE_GET_INFO_MEMBER(cclimber_state::cclimber_get_pf_tile_info)
{
	const int flags = TILE_FLIPYX(m_colorram[tile_index] >> 6);

	/* vertical flipping flips two adjacent characters */
	if (flags & 0x02)
		tile_index = tile_index ^ 0x20;

	const int code = ((m_colorram[tile_index] & 0x10) << 5) |
			((m_colorram[tile_index] & 0x20) << 3) | m_videoram[tile_index];

	const int color = m_colorram[tile_index] & 0x0f;

	tileinfo.set(0, code, color, flags);
}


TILE_GET_INFO_MEMBER(swimmer_state::swimmer_get_pf_tile_info)
{
	const int flags = TILE_FLIPYX(m_colorram[tile_index] >> 6);

	/* vertical flipping flips two adjacent characters */
	if (flags & 0x02)
		tile_index = tile_index ^ 0x20;

	const int code = ((m_colorram[tile_index] & 0x30) << 4) | m_videoram[tile_index];
	const int color = (m_palettebank << 4) | (m_colorram[tile_index] & 0x0f);

	tileinfo.set(0, code, color, flags);
}


TILE_GET_INFO_MEMBER(toprollr_state::toprollr_get_pf_tile_info)
{
	const int attr = m_colorram[tile_index];
	const int code = ((attr & 0x10) << 5) | ((attr & 0x20) << 3) | m_videoram[tile_index];
	const int color = attr & 0x0f;

	tileinfo.set(0, code, color, 0);
}


TILE_GET_INFO_MEMBER(cclimber_state::cclimber_get_bs_tile_info)
{
	/* only the lower right is visible */
	tileinfo.group = ((tile_index & 0x210) == 0x210) ? 0 : 1;

	/* the address doesn't use A4 of the coordinates, giving a 16x16 map */
	tile_index = ((tile_index & 0x1e0) >> 1) | (tile_index & 0x0f);

	const int code = ((m_bigsprite_control[1] & 0x08) << 5) | m_bigsprite_videoram[tile_index];
	const int color = m_bigsprite_control[1] & 0x07;

	tileinfo.set(2, code, color, 0);
}


TILE_GET_INFO_MEMBER(toprollr_state::toprollr_get_bs_tile_info)
{
	/* only the lower right is visible */
	tileinfo.group = ((tile_index & 0x210) == 0x210) ? 0 : 1;

	/* the address doesn't use A4 of the coordinates, giving a 16x16 map */
	tile_index = ((tile_index & 0x1e0) >> 1) | (tile_index & 0x0f);

	const int code = ((m_bigsprite_control[1] & 0x18) << 5) | m_bigsprite_videoram[tile_index];
	const int color = m_bigsprite_control[1] & 0x07;

	tileinfo.set(2, code, color, 0);
}


TILE_GET_INFO_MEMBER(toprollr_state::toproller_get_bg_tile_info)
{
	const int code = ((m_toprollr_bg_coloram[tile_index] & 0x40) << 2) | m_toprollr_bg_videoram[tile_index];
	const int color = m_toprollr_bg_coloram[tile_index] & 0x0f;

	tileinfo.set(3, code, color, TILE_FLIPX);
}


void cclimber_state::video_start()
{
	m_pf_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cclimber_state::cclimber_get_pf_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_pf_tilemap->set_transparent_pen(0);
	m_pf_tilemap->set_scroll_cols(32);

	m_bs_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cclimber_state::cclimber_get_bs_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bs_tilemap->set_scroll_cols(1);
	m_bs_tilemap->set_scroll_rows(1);
	m_bs_tilemap->set_transmask(0, 0x01, 0); // pen 0 is transparent
	m_bs_tilemap->set_transmask(1, 0x0f, 0); // all 4 pens are transparent

	save_item(NAME(m_flip_x));
	save_item(NAME(m_flip_y));
}

void swimmer_state::video_start()
{
	m_pf_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(swimmer_state::swimmer_get_pf_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_pf_tilemap->set_transparent_pen(0);
	m_pf_tilemap->set_scroll_cols(32);

	m_bs_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(swimmer_state::cclimber_get_bs_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bs_tilemap->set_scroll_cols(1);
	m_bs_tilemap->set_scroll_rows(1);
	m_bs_tilemap->set_transmask(0, 0x01, 0); // pen 0 is transparent
	m_bs_tilemap->set_transmask(1, 0xff, 0); // all 8 pens are transparent

	save_item(NAME(m_flip_x));
	save_item(NAME(m_flip_y));
	save_item(NAME(m_side_background_enabled));
	save_item(NAME(m_palettebank));
}

void toprollr_state::video_start()
{
	m_pf_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(toprollr_state::toprollr_get_pf_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_pf_tilemap->set_transparent_pen(0);

	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(toprollr_state::toproller_get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap->set_scroll_rows(1);

	m_bs_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(toprollr_state::toprollr_get_bs_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bs_tilemap->set_scroll_cols(1);
	m_bs_tilemap->set_scroll_rows(1);
	m_bs_tilemap->set_transmask(0, 0x01, 0); // pen 0 is transparent
	m_bs_tilemap->set_transmask(1, 0x0f, 0); // all 4 pens are transparent

	save_item(NAME(m_flip_x));
	save_item(NAME(m_flip_y));
}


void cclimber_state::draw_playfield(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_pf_tilemap->mark_all_dirty();
	m_pf_tilemap->set_flip((m_flip_x ? TILEMAP_FLIPX : 0) | (m_flip_y ? TILEMAP_FLIPY : 0));
	for (int i = 0; i < 32; i++)
		m_pf_tilemap->set_scrolly(i, m_column_scroll[i]);

	m_pf_tilemap->draw(screen, bitmap, cliprect, 0, 0);
}


void cclimber_state::cclimber_draw_bigsprite(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t x = m_bigsprite_control[3] - 8;
	uint8_t y = m_bigsprite_control[2];
	int bigsprite_flip_x = (m_bigsprite_control[1] & 0x10) >> 4;
	int bigsprite_flip_y = (m_bigsprite_control[1] & 0x20) >> 5;

	if (bigsprite_flip_x)
		x = 0x80 - x;

	if (bigsprite_flip_y)
		y = 0x80 - y;

	m_bs_tilemap->mark_all_dirty();

	m_bs_tilemap->set_flip((bigsprite_flip_x ? TILEMAP_FLIPX : 0) | (m_flip_y ^ bigsprite_flip_y ? TILEMAP_FLIPY : 0));

	m_bs_tilemap->set_scrollx(0, x);
	m_bs_tilemap->set_scrolly(0, y);

	m_bs_tilemap->draw(screen, bitmap, cliprect, 0, 0);
}


void toprollr_state::toprollr_draw_bigsprite(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t x = m_bigsprite_control[3] - 8;
	uint8_t y = m_bigsprite_control[2];

	if (m_flip_x)
		x = 0x80 - x;

	m_bs_tilemap->mark_all_dirty();

	m_bs_tilemap->set_flip((m_flip_x ? TILEMAP_FLIPX : 0) | (m_flip_y ? TILEMAP_FLIPY : 0));

	m_bs_tilemap->set_scrollx(0, x);
	m_bs_tilemap->set_scrolly(0, y);

	m_bs_tilemap->draw(screen, bitmap, cliprect, 0, 0);
}


void cclimber_state::cclimber_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx)
{
	/* draw the sprites -- note that it is important to draw them exactly in this
	   order, to have the correct priorities. */
	for (int offs = 0x1c; offs >= 0; offs -= 4)
	{
		int x = m_spriteram[offs + 3] + 1;
		/* x + 1 is evident in cclimber and ckong. It looks worse,
		   but it has been confirmed on several PCBs. */

		int y = 240 - m_spriteram[offs + 2];

		int code = ((m_spriteram[offs + 1] & 0x10) << 3) |
					((m_spriteram[offs + 1] & 0x20) << 1) |
					( m_spriteram[offs + 0] & 0x3f);

		int color = m_spriteram[offs + 1] & 0x0f;

		int flipx = m_spriteram[offs + 0] & 0x40;
		int flipy = m_spriteram[offs + 0] & 0x80;

		if (m_flip_x)
		{
			x = 242 - x;
			flipx = !flipx;
		}

		if (m_flip_y)
		{
			y = 240 - y;
			flipy = !flipy;
		}

		gfx->transpen(bitmap,cliprect, code, color, flipx, flipy, x, y, 0);
	}
}


void cclimber_state::toprollr_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx)
{
	/* draw the sprites -- note that it is important to draw them exactly in this
	   order, to have the correct priorities. */
	for (int offs = m_spriteram.bytes() - 4; offs >= 0; offs -= 4)
	{
		int x = m_spriteram[offs + 3];
		int y = 240 - m_spriteram[offs + 2];

		const int code = ((m_spriteram[offs + 1] & 0x10) << 3) |
					((m_spriteram[offs + 1] & 0x20) << 1) |
					( m_spriteram[offs + 0] & 0x3f);

		const int color = m_spriteram[offs + 1] & 0x0f;

		int flipx = m_spriteram[offs + 0] & 0x40;
		int flipy = m_spriteram[offs + 0] & 0x80;

		if (m_flip_x)
		{
			x = 240 - x;
			flipx = !flipx;
		}

		if (m_flip_y)
		{
			y = 240 - y;
			flipy = !flipy;
		}

		gfx->transpen(bitmap,cliprect, code, color, flipx, flipy, x, y, 0);
	}
}


void swimmer_state::swimmer_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx)
{
	/* draw the sprites -- note that it is important to draw them exactly in this
	   order, to have the correct priorities. */
	for (int offs = 0x1c; offs >= 0; offs -= 4)
	{
		int x = m_spriteram[offs + 3];
		int y = 240 - m_spriteram[offs + 2];

		const int code = ((m_spriteram[offs + 1] & 0x30) << 2) |
					(m_spriteram[offs + 0] & 0x3f);

		const int color = (m_palettebank << 4) |
					(m_spriteram[offs + 1] & 0x0f);

		int flipx = m_spriteram[offs + 0] & 0x40;
		int flipy = m_spriteram[offs + 0] & 0x80;

		if (m_flip_x)
		{
			x = 240 - x;
			flipx = !flipx;
		}

		if (m_flip_y)
		{
			y = 240 - y;
			flipy = !flipy;
		}

		gfx->transpen(bitmap,cliprect, code, color, flipx, flipy, x, y, 0);
	}
}


uint32_t cclimber_state::screen_update_cclimber(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	draw_playfield(screen, bitmap, cliprect);

	// draw the "big sprite" under the regular sprites 
	if ((m_bigsprite_control[0] & 0x01))
	{
		cclimber_draw_bigsprite(screen, bitmap, cliprect);
		cclimber_draw_sprites(bitmap, cliprect, m_gfxdecode->gfx(1));
	}

	// draw the "big sprite" over the regular sprites 
	else
	{
		cclimber_draw_sprites(bitmap, cliprect, m_gfxdecode->gfx(1));
		cclimber_draw_bigsprite(screen, bitmap, cliprect);
	}

	return 0;
}


uint32_t yamato_state::screen_update_yamato(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t const *const sky_rom = memregion("user1")->base() + 0x1200;

	for (int i = 0; i < 0x100; i++)
	{
		pen_t pen = YAMATO_SKY_PEN_BASE + sky_rom[(m_flip_x ? 0x80 : 0) + (i >> 1)];

		for (int j = 0; j < 0x100; j++)
			bitmap.pix(j, (i - 8) & 0xff) = pen;
	}

	draw_playfield(screen, bitmap, cliprect);

	// draw the "big sprite" under the regular sprites 
	if ((m_bigsprite_control[0] & 0x01))
	{
		cclimber_draw_bigsprite(screen, bitmap, cliprect);
		toprollr_draw_sprites(bitmap, cliprect, m_gfxdecode->gfx(1));
	}

	// draw the "big sprite" over the regular sprites 
	else
	{
		toprollr_draw_sprites(bitmap, cliprect, m_gfxdecode->gfx(1));
		cclimber_draw_bigsprite(screen, bitmap, cliprect);
	}

	return 0;
}


uint32_t swimmer_state::screen_update_swimmer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_swimmer_background_color)
		set_background_pen();

	if (m_side_background_enabled)
	{
		if (m_flip_x)
		{
			rectangle split_rect_left(0, 0xff - SWIMMER_BG_SPLIT, 0, 0xff);
			rectangle split_rect_right(0x100 - SWIMMER_BG_SPLIT, 0xff, 0, 0xff);

			split_rect_left &= cliprect;
			bitmap.fill(m_sidepen, split_rect_left);

			split_rect_right &= cliprect;
			bitmap.fill(0, split_rect_right);
		}
		else
		{
			rectangle split_rect_left(0, SWIMMER_BG_SPLIT - 1, 0, 0xff);
			rectangle split_rect_right(SWIMMER_BG_SPLIT, 0xff, 0, 0xff);

			split_rect_left &= cliprect;
			bitmap.fill(0, split_rect_left);

			split_rect_right &= cliprect;
			bitmap.fill(m_sidepen, split_rect_right);
		}
	}
	else
		bitmap.fill(0, cliprect);

	draw_playfield(screen, bitmap, cliprect);

	// draw the "big sprite" under the regular sprites 
	if ((m_bigsprite_control[0] & 0x01))
	{
		cclimber_draw_bigsprite(screen, bitmap, cliprect);
		swimmer_draw_sprites(bitmap, cliprect, m_gfxdecode->gfx(1));
	}

	// draw the "big sprite" over the regular sprites 
	else
	{
		swimmer_draw_sprites(bitmap, cliprect, m_gfxdecode->gfx(1));
		cclimber_draw_bigsprite(screen, bitmap, cliprect);
	}

	return 0;
}


uint32_t toprollr_state::screen_update_toprollr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	rectangle scroll_area_clip = cliprect;
	scroll_area_clip.min_x = (m_flip_x ? 3 : 5) * 8;
	scroll_area_clip.max_x = (m_flip_x ? 27 : 29) * 8 - 1;
	scroll_area_clip &= cliprect;

	bitmap.fill(0, cliprect);

	m_bg_tilemap->set_scrollx(0, m_toprollr_bg_videoram[0]);
	m_bg_tilemap->set_flip((m_flip_x ? TILEMAP_FLIPX : 0) | (m_flip_y ? TILEMAP_FLIPY : 0));
	m_bg_tilemap->mark_all_dirty();
	m_bg_tilemap->draw(screen, bitmap, scroll_area_clip, 0, 0);

	// draw the "big sprite" over the regular sprites 
	if ((m_bigsprite_control[1] & 0x20))
	{
		toprollr_draw_sprites(bitmap, scroll_area_clip, m_gfxdecode->gfx(1));
		toprollr_draw_bigsprite(screen, bitmap, scroll_area_clip);
	}

	// draw the "big sprite" under the regular sprites 
	else
	{
		toprollr_draw_bigsprite(screen, bitmap, scroll_area_clip);
		toprollr_draw_sprites(bitmap, scroll_area_clip, m_gfxdecode->gfx(1));
	}

	m_pf_tilemap->mark_all_dirty();
	m_pf_tilemap->set_flip((m_flip_x ? TILEMAP_FLIPX : 0) | (m_flip_y ? TILEMAP_FLIPY : 0));
	m_pf_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}
