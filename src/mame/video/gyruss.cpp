// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/gyruss.h"

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Gyruss has one 32x8 palette PROM and two 256x4 lookup table PROMs
  (one for characters, one for sprites).
  The palette PROM is connected to the RGB output this way:

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

***************************************************************************/

void gyruss_state::gyruss_palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();
	static constexpr int resistances_rg[3] = { 1000, 470, 220 };
	static constexpr int resistances_b [2] = { 470, 220 };

	// compute the color output resistor weights
	double weights_rg[3], weights_b[2];
	compute_resistor_weights(0, 255, -1.0,
			3, resistances_rg, weights_rg, 470, 0,
			2, resistances_b,  weights_b,  470, 0,
			0, nullptr, nullptr, 0, 0);

	// create a lookup table for the palette
	for (int i = 0; i < 0x20; i++)
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

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	// color_prom now points to the beginning of the lookup table
	color_prom += 32;

	// sprites map to the lower 16 palette entries
	for (int i = 0; i < 0x100; i++)
	{
		uint8_t const ctabentry = color_prom[i] & 0x0f;
		palette.set_pen_indirect(i, ctabentry);
	}

	// characters map to the upper 16 palette entries
	for (int i = 0x100; i < 0x140; i++)
	{
		uint8_t const ctabentry = color_prom[i] & 0x0f;
		palette.set_pen_indirect(i, ctabentry | 0x10);
	}
}



WRITE8_MEMBER(gyruss_state::gyruss_spriteram_w)
{
//  m_screen->update_now();
	m_screen->update_partial(m_screen->vpos());
	m_spriteram[offset] = data;
}


TILE_GET_INFO_MEMBER(gyruss_state::gyruss_get_tile_info)
{
	int code = ((m_colorram[tile_index] & 0x20) << 3) | m_videoram[tile_index];
	int color = m_colorram[tile_index] & 0x0f;
	int flags = TILE_FLIPYX(m_colorram[tile_index] >> 6);

	tileinfo.group = (m_colorram[tile_index] & 0x10) ? 0 : 1;

	SET_TILE_INFO_MEMBER(2, code, color, flags);
}


void gyruss_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gyruss_state::gyruss_get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_tilemap->set_transmask(0, 0x00, 0); // opaque
	m_tilemap->set_transmask(1, 0x0f, 0); // transparent

	save_item(NAME(m_flipscreen));
}



READ8_MEMBER(gyruss_state::gyruss_scanline_r)
{
	/* reads 1V - 128V */
	return m_screen->vpos();
}


WRITE_LINE_MEMBER(gyruss_state::flipscreen_w)
{
	m_flipscreen = state;
}


void gyruss_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int offs;

	for (offs = 0xbc; offs >= 0; offs -= 4)
	{
		int x = m_spriteram[offs];
		int y = 241 - m_spriteram[offs + 3];

		int gfx_bank = m_spriteram[offs + 1] & 0x01;
		int code = ((m_spriteram[offs + 2] & 0x20) << 2) | ( m_spriteram[offs + 1] >> 1);
		int color = m_spriteram[offs + 2] & 0x0f;
		int flip_x = ~m_spriteram[offs + 2] & 0x40;
		int flip_y =  m_spriteram[offs + 2] & 0x80;

			m_gfxdecode->gfx(gfx_bank)->transpen(bitmap,cliprect, code, color, flip_x, flip_y, x, y, 0);
	}
}


uint32_t gyruss_state::screen_update_gyruss(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (cliprect.min_y == screen.visible_area().min_y)
	{
		machine().tilemap().mark_all_dirty();
		machine().tilemap().set_flip_all(m_flipscreen ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0);
	}

	m_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	draw_sprites(bitmap, cliprect);
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}
