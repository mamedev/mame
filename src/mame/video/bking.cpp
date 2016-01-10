// license:???
// copyright-holders:Ed Mueller, Mike Balfour, Zsolt Vasvari
/***************************************************************************

  bking.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/bking.h"


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

PALETTE_INIT_MEMBER(bking_state, bking)
{
	const UINT8 *color_prom = memregion("proms")->base();
	static const int resistances_rg[3] = { 220, 390, 820 };
	static const int resistances_b [2] = { 220, 390 };
	double rweights[3], gweights[3], bweights[2];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0, 255, -1.0,
			3, &resistances_rg[0], rweights, 0, 0,
			3, &resistances_rg[0], gweights, 0, 0,
			2, &resistances_b[0],  bweights, 0, 0);

	for (i = 0; i < palette.entries(); i++)
	{
		UINT16 pen;
		int bit0, bit1, bit2, r, g, b;

		/* color PROM A7-A8 is the palette select */
		if (i < 0x20)
			/* characters - image bits go to A0-A2 of the color PROM */
			pen = (((i - 0x00) << 4) & 0x180) | ((i - 0x00) & 0x07);
		else if (i < 0x30)
			/* crow - image bits go to A5-A6. */
			pen = (((i - 0x20) << 5) & 0x180) | (((i - 0x20) & 0x03) << 5);
		else if (i < 0x38)
			/* ball #1 - image bit goes to A3 */
			pen = (((i - 0x30) << 6) & 0x180) | (((i - 0x30) & 0x01) << 3);
		else
			/* ball #2 - image bit goes to A4 */
			pen = (((i - 0x38) << 6) & 0x180) | (((i - 0x38) & 0x01) << 4);

		/* red component */
		bit0 = (color_prom[pen] >> 0) & 0x01;
		bit1 = (color_prom[pen] >> 1) & 0x01;
		bit2 = (color_prom[pen] >> 2) & 0x01;
		r = combine_3_weights(rweights, bit0, bit1, bit2);

		/* green component */
		bit0 = (color_prom[pen] >> 3) & 0x01;
		bit1 = (color_prom[pen] >> 4) & 0x01;
		bit2 = (color_prom[pen] >> 5) & 0x01;
		g = combine_3_weights(gweights, bit0, bit1, bit2);

		/* blue component */
		bit0 = (color_prom[pen] >> 6) & 0x01;
		bit1 = (color_prom[pen] >> 7) & 0x01;
		b = combine_2_weights(gweights, bit0, bit1);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}


WRITE8_MEMBER(bking_state::bking_xld1_w)
{
	m_xld1 = -data;
}

WRITE8_MEMBER(bking_state::bking_yld1_w)
{
	m_yld1 = -data;
}

WRITE8_MEMBER(bking_state::bking_xld2_w)
{
	m_xld2 = -data;
}

WRITE8_MEMBER(bking_state::bking_yld2_w)
{
	m_yld2 = -data;
}

WRITE8_MEMBER(bking_state::bking_xld3_w)
{
	m_xld3 = -data;
}

WRITE8_MEMBER(bking_state::bking_yld3_w)
{
	m_yld3 = -data;
}


WRITE8_MEMBER(bking_state::bking_cont1_w)
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

WRITE8_MEMBER(bking_state::bking_cont2_w)
{
	/* D0-D2 = BALL10 - BALL12 (Selects player 1 ball picture) */
	/* D3-D5 = BALL20 - BALL22 (Selects player 2 ball picture) */
	/* D6 = HIT1 */
	/* D7 = HIT2 */

	m_ball1_pic = (data >> 0) & 0x07;
	m_ball2_pic = (data >> 3) & 0x07;

	m_hit = data >> 6;
}

WRITE8_MEMBER(bking_state::bking_cont3_w)
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


WRITE8_MEMBER(bking_state::bking_msk_w)
{
	m_pc3259_mask++;
}


WRITE8_MEMBER(bking_state::bking_hitclr_w)
{
	m_pc3259_mask = 0;

	m_pc3259_output[0] = 0;
	m_pc3259_output[1] = 0;
	m_pc3259_output[2] = 0;
	m_pc3259_output[3] = 0;
}


WRITE8_MEMBER(bking_state::bking_playfield_w)
{
	m_playfield_ram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}


READ8_MEMBER(bking_state::bking_input_port_5_r)
{
	return ioport(m_controller ? "TRACK1_X" : "TRACK0_X")->read();
}

READ8_MEMBER(bking_state::bking_input_port_6_r)
{
	return ioport(m_controller ? "TRACK1_Y" : "TRACK0_Y")->read();
}

READ8_MEMBER(bking_state::bking_pos_r)
{
	return m_pc3259_output[offset / 8] << 4;
}


TILE_GET_INFO_MEMBER(bking_state::get_tile_info)
{
	UINT8 code0 = m_playfield_ram[2 * tile_index + 0];
	UINT8 code1 = m_playfield_ram[2 * tile_index + 1];

	int flags = 0;

	if (code1 & 4) flags |= TILE_FLIPX;
	if (code1 & 8) flags |= TILE_FLIPY;

	SET_TILE_INFO_MEMBER(0, code0 + 256 * code1, m_palette_bank, flags);
}


void bking_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(bking_state::get_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_screen->register_screen_bitmap(m_colmap_bg);
	m_screen->register_screen_bitmap(m_colmap_ball);
}


UINT32 bking_state::screen_update_bking(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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


void bking_state::screen_eof_bking(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		const rectangle rect(0, 7, 0, 15);

		int xld = 0;
		int yld = 0;

		UINT32 latch = 0;

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

		m_bg_tilemap->draw(screen, m_colmap_bg, rect, 0, 0);

		m_bg_tilemap->set_scrollx(0, 0);
		m_bg_tilemap->set_scrolly(0, 0);

		// check for collision
		const UINT8* colmask = memregion("user1")->base() + 8 * m_hit;

		for (int y = rect.min_y; y <= rect.max_y; y++)
		{
			const UINT16* p0 = &m_colmap_bg.pix16(y);
			const UINT16* p1 = &m_colmap_ball.pix16(y);

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
