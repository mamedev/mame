// license:BSD-3-Clause
// copyright-holders:Mike Balfour, Zsolt Vasvari
/***************************************************************************

  bking.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "bking.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

  The palette PROM is connected to the RGB output this way:

  bit 7 -- 390 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- BLUE
        -- 820 ohm resistor  -- GREEN
        -- 390 ohm resistor  -- GREEN
        -- 220 ohm resistor  -- GREEN
        -- 820 ohm resistor  -- RED
        -- 390 ohm resistor  -- RED
  bit 0 -- 220 ohm resistor  -- RED

***************************************************************************/

void bking_state::bking_palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();
	static constexpr int resistances_rg[3] = { 220, 390, 820 };
	static constexpr int resistances_b [2] = { 220, 390 };

	// compute the color output resistor weights
	double rweights[3], gweights[3], bweights[2];
	compute_resistor_weights(0, 255, -1.0,
			3, &resistances_rg[0], rweights, 0, 0,
			3, &resistances_rg[0], gweights, 0, 0,
			2, &resistances_b[0],  bweights, 0, 0);

	for (int i = 0; i < palette.entries(); i++)
	{
		uint16_t pen;
		int bit0, bit1, bit2;

		// color PROM A7-A8 is the palette select
		if (i < 0x20) // characters - image bits go to A0-A2 of the color PROM
			pen = (((i - 0x00) << 4) & 0x180) | ((i - 0x00) & 0x07);
		else if (i < 0x30) // crow - image bits go to A5-A6.
			pen = (((i - 0x20) << 5) & 0x180) | (((i - 0x20) & 0x03) << 5);
		else if (i < 0x38) // ball #1 - image bit goes to A3
			pen = (((i - 0x30) << 6) & 0x180) | (((i - 0x30) & 0x01) << 3);
		else // ball #2 - image bit goes to A4
			pen = (((i - 0x38) << 6) & 0x180) | (((i - 0x38) & 0x01) << 4);

		// red component
		bit0 = (color_prom[pen] >> 0) & 0x01;
		bit1 = (color_prom[pen] >> 1) & 0x01;
		bit2 = (color_prom[pen] >> 2) & 0x01;
		int const r = combine_weights(rweights, bit0, bit1, bit2);

		// green component
		bit0 = (color_prom[pen] >> 3) & 0x01;
		bit1 = (color_prom[pen] >> 4) & 0x01;
		bit2 = (color_prom[pen] >> 5) & 0x01;
		int const g = combine_weights(gweights, bit0, bit1, bit2);

		// blue component
		bit0 = (color_prom[pen] >> 6) & 0x01;
		bit1 = (color_prom[pen] >> 7) & 0x01;
		int const b = combine_weights(gweights, bit0, bit1);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}


void bking_state::bking_xld1_w(uint8_t data)
{
	m_xld1 = -data;
}

void bking_state::bking_yld1_w(uint8_t data)
{
	m_yld1 = -data;
}

void bking_state::bking_xld2_w(uint8_t data)
{
	m_xld2 = -data;
}

void bking_state::bking_yld2_w(uint8_t data)
{
	m_yld2 = -data;
}

void bking_state::bking_xld3_w(uint8_t data)
{
	m_xld3 = -data;
}

void bking_state::bking_yld3_w(uint8_t data)
{
	m_yld3 = -data;
}


void bking_state::bking_cont1_w(uint8_t data)
{
	/* D0 = COIN LOCK */
	/* D1 = BALL 5 (Controller selection) */
	/* D2 = VINV (flip screen) */
	/* D3 = Not Connected */
	/* D4-D7 = CROW0-CROW3 (selects crow picture) */

	machine().bookkeeping().coin_lockout_global_w(~data & 0x01);

	flip_screen_set(data & 0x04);

	m_controller = data & 0x02;

	m_crow_pic = (data >> 4) & 0x0f;
}

void bking_state::bking_cont2_w(uint8_t data)
{
	/* D0-D2 = BALL10 - BALL12 (Selects player 1 ball picture) */
	/* D3-D5 = BALL20 - BALL22 (Selects player 2 ball picture) */
	/* D6 = HIT1 */
	/* D7 = HIT2 */

	m_ball1_pic = (data >> 0) & 0x07;
	m_ball2_pic = (data >> 3) & 0x07;

	m_hit = data >> 6;
}

void bking_state::bking_cont3_w(uint8_t data)
{
	/* D0 = CROW INV (inverts Crow picture and coordinates) */
	/* D1-D2 = COLOR 0 - COLOR 1 (switches 4 color palettes, global across all graphics) */
	/* D3 = SOUND STOP */

	m_crow_flip = ~data & 0x01;

	if (m_palette_bank != ((data >> 1) & 0x03))
	{
		m_palette_bank = (data >> 1) & 0x03;
		m_bg_tilemap->mark_all_dirty();
	}

	machine().sound().system_mute(data & 0x08);
}


void bking_state::bking_msk_w(uint8_t data)
{
	m_pc3259_mask++;
}


void bking_state::bking_hitclr_w(uint8_t data)
{
	m_pc3259_mask = 0;

	m_pc3259_output[0] = 0;
	m_pc3259_output[1] = 0;
	m_pc3259_output[2] = 0;
	m_pc3259_output[3] = 0;
}


void bking_state::bking_playfield_w(offs_t offset, uint8_t data)
{
	m_playfield_ram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}


uint8_t bking_state::bking_input_port_5_r()
{
	return ioport(m_controller ? "TRACK1_X" : "TRACK0_X")->read();
}

uint8_t bking_state::bking_input_port_6_r()
{
	return ioport(m_controller ? "TRACK1_Y" : "TRACK0_Y")->read();
}

uint8_t bking_state::bking_pos_r(offs_t offset)
{
	return m_pc3259_output[offset / 8] << 4;
}


TILE_GET_INFO_MEMBER(bking_state::get_tile_info)
{
	uint8_t code0 = m_playfield_ram[2 * tile_index + 0];
	uint8_t code1 = m_playfield_ram[2 * tile_index + 1];

	int flags = 0;

	if (code1 & 4) flags |= TILE_FLIPX;
	if (code1 & 8) flags |= TILE_FLIPY;

	tileinfo.set(0, code0 + 256 * code1, m_palette_bank, flags);
}


void bking_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(bking_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_screen->register_screen_bitmap(m_colmap_bg);
	m_screen->register_screen_bitmap(m_colmap_ball);
}


uint32_t bking_state::screen_update_bking(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	/* draw the balls */
	m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
		m_ball1_pic,
		m_palette_bank,
		0, 0,
		m_xld1, m_yld1, 0);

	m_gfxdecode->gfx(3)->transpen(bitmap,cliprect,
		m_ball2_pic,
		m_palette_bank,
		0, 0,
		m_xld2, m_yld2, 0);

	/* draw the crow */
	m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
		m_crow_pic,
		m_palette_bank,
		m_crow_flip, m_crow_flip,
		m_crow_flip ? m_xld3 - 16 : 256 - m_xld3, m_crow_flip ? m_yld3 - 16 : 256 - m_yld3, 0);

	return 0;
}


WRITE_LINE_MEMBER(bking_state::screen_vblank_bking)
{
	// rising edge
	if (state)
	{
		const rectangle rect(0, 7, 0, 15);

		int xld = 0;
		int yld = 0;

		uint32_t latch = 0;

		if (m_pc3259_mask == 6) /* player 1 */
		{
			xld = m_xld1;
			yld = m_yld1;

			m_gfxdecode->gfx(2)->opaque(m_colmap_ball,rect, m_ball1_pic, 0, 0, 0, 0, 0);

			latch = 0x0c00;
		}
		else if (m_pc3259_mask == 3) /* player 2 */
		{
			xld = m_xld2;
			yld = m_yld2;

			m_gfxdecode->gfx(3)->opaque(m_colmap_ball,rect, m_ball2_pic, 0, 0, 0, 0, 0);

			latch = 0x0400;
		}
		else
			return;

		m_bg_tilemap->set_scrollx(0, flip_screen() ? -xld : xld);
		m_bg_tilemap->set_scrolly(0, flip_screen() ? -yld : yld);

		m_bg_tilemap->draw(*m_screen, m_colmap_bg, rect, 0, 0);

		m_bg_tilemap->set_scrollx(0, 0);
		m_bg_tilemap->set_scrolly(0, 0);

		// check for collision
		uint8_t const *const colmask = memregion("user1")->base() + 8 * m_hit;

		for (int y = rect.min_y; y <= rect.max_y; y++)
		{
			uint16_t const *const p0 = &m_colmap_bg.pix(y);
			uint16_t const *const p1 = &m_colmap_ball.pix(y);

			for (int x = rect.min_x; x <= rect.max_x; x++)
			{
				if (colmask[p0[x] & 7] && p1[x] & 1)
				{
					int col = (xld + x) / 8 + 1;
					int row = (yld + y) / 8 + 0;

					latch |= (flip_screen() ? 31 - col : col) << 0;
					latch |= (flip_screen() ? 31 - row : row) << 5;

					m_pc3259_output[0] = (latch >> 0x0) & 0xf;
					m_pc3259_output[1] = (latch >> 0x4) & 0xf;
					m_pc3259_output[2] = (latch >> 0x8) & 0xf;
					m_pc3259_output[3] = (latch >> 0xc) & 0xf;

					return;
				}
			}
		}
	}
}
