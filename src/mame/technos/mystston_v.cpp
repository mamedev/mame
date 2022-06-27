// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Technos Mysterious Stones hardware

    driver by Nicola Salmoria

    There are only a few differences between the video hardware of Mysterious
    Stones and Mat Mania. The tile bank select bit is different and the sprite
    selection seems to be different as well. Additionally, the palette is stored
    differently. I'm also not sure that the 2nd tile page is really used in
    Mysterious Stones.

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/mystston.h"



/*************************************
 *
 *  Video timing constants
 *
 *************************************/
// HMC20
// set_raw(MASTER_CLOCK / 2, 384, 0, 256, 272, 8, 248)
#define PIXEL_CLOCK     (MASTER_CLOCK / 2)
#define HTOTAL          (384)
#define HBEND           (0)
#define HBSTART         (256)
#define VTOTAL          (272)  /* counts from 0x08-0xff, then from 0xe8-0xff */
#define VBEND           (8)
#define VBSTART         (248)
#define FIRST_INT_VPOS  (0x008)
#define INT_HPOS        (0x100)



/*************************************
 *
 *  Scanline interrupt system
 *
 *  There is an interrupt every 16
 *  scanlines, starting with 8.
 *
 *************************************/

TIMER_CALLBACK_MEMBER(mystston_state::interrupt_callback)
{
	int scanline = param;

	on_scanline_interrupt();

	scanline = scanline + 16;
	if (scanline >= VTOTAL)
		scanline = FIRST_INT_VPOS;

	// the vertical synch chain is clocked by H256 -- this is probably not important, but oh well
	m_interrupt_timer->adjust(m_screen->time_until_pos(scanline - 1, INT_HPOS), scanline);
}



/*************************************
 *
 *  Palette handling
 *
 *************************************/

void mystston_state::set_palette()
{
	static const int resistances_rg[3] = { 4700, 3300, 1500 };
	static const int resistances_b [2] = { 3300, 1500 };
	double weights_rg[3], weights_b[2];

	compute_resistor_weights(0, 255, -1.0,
			3, resistances_rg, weights_rg, 0, 4700,
			2, resistances_b,  weights_b,  0, 4700,
			0, nullptr, nullptr, 0, 0);

	for (int i = 0; i < 0x40; i++)
	{
		uint8_t data;

		// first half is dynamic, second half is from the PROM
		if (i & 0x20)
			data = m_color_prom[i & 0x1f];
		else
			data = m_paletteram[i];

		// red component
		int bit0 = (data >> 0) & 0x01;
		int bit1 = (data >> 1) & 0x01;
		int bit2 = (data >> 2) & 0x01;
		int r = combine_weights(weights_rg, bit0, bit1, bit2);

		// green component
		bit0 = (data >> 3) & 0x01;
		bit1 = (data >> 4) & 0x01;
		bit2 = (data >> 5) & 0x01;
		int g = combine_weights(weights_rg, bit0, bit1, bit2);

		// blue component
		bit0 = (data >> 6) & 0x01;
		bit1 = (data >> 7) & 0x01;
		int b = combine_weights(weights_b, bit0, bit1);

		m_palette->set_pen_color(i, rgb_t(r, g, b));
	}
}



/*************************************
 *
 *  Video control register
 *
 *************************************/

void mystston_state::video_control_w(uint8_t data)
{
	*m_video_control = data;

	// D0-D1 - foreground text color
	// D2 - background page select
	// D3 - unused

	// D4-D5 - coin counters in flipped order
	machine().bookkeeping().coin_counter_w(0, data & 0x20);
	machine().bookkeeping().coin_counter_w(1, data & 0x10);

	// D6 - unused
	// D7 - screen flip
}



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

TILE_GET_INFO_MEMBER(mystston_state::get_bg_tile_info)
{
	int page = (*m_video_control & 0x04) << 8;
	int code = ((m_bg_videoram[page | 0x200 | tile_index] & 0x01) << 8) | m_bg_videoram[page | tile_index];
	int flags = (tile_index & 0x10) ? TILE_FLIPY : 0;

	tileinfo.set(1, code, 0, flags);
}


TILE_GET_INFO_MEMBER(mystston_state::get_fg_tile_info)
{
	int code = ((m_fg_videoram[0x400 | tile_index] & 0x07) << 8) | m_fg_videoram[tile_index];
	int color = ((*m_video_control & 0x01) << 1) | ((*m_video_control & 0x02) >> 1);

	tileinfo.set(0, code, color, 0);
}



/*************************************
 *
 *  Sprite drawing
 *
 *************************************/

void mystston_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx, int flip)
{
	for (int offs = 0; offs < 0x60; offs += 4)
	{
		int attr = m_spriteram[offs];

		if (attr & 0x01)
		{
			int code = ((attr & 0x10) << 4) | m_spriteram[offs + 1];
			int color = (attr & 0x08) >> 3;
			int flipx = attr & 0x04;
			int flipy = attr & 0x02;
			int x = 240 - m_spriteram[offs + 3];
			int y = (240 - m_spriteram[offs + 2]) & 0xff;

			if (flip)
			{
				x = 240 - x;
				y = 240 - y;
				flipx = !flipx;
				flipy = !flipy;
			}

			gfx->transpen(bitmap,cliprect, code, color, flipx, flipy, x, y, 0);
		}
	}
}



/*************************************
 *
 *  Start
 *
 *************************************/

void mystston_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(mystston_state::get_bg_tile_info)), TILEMAP_SCAN_COLS_FLIP_X, 16, 16, 16, 32);

	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(mystston_state::get_fg_tile_info)), TILEMAP_SCAN_COLS_FLIP_X,  8,  8, 32, 32);
	m_fg_tilemap->set_transparent_pen(0);

	// create the interrupt timer
	m_interrupt_timer = timer_alloc(FUNC(mystston_state::interrupt_callback), this);
}



/*************************************
 *
 *  Reset
 *
 *************************************/

void mystston_state::video_reset()
{
	m_interrupt_timer->adjust(m_screen->time_until_pos(FIRST_INT_VPOS - 1, INT_HPOS), FIRST_INT_VPOS);
}



/*************************************
 *
 *  Update
 *
 *************************************/

uint32_t mystston_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int flip = (*m_video_control & 0x80) ^ ((m_dsw1->read() & 0x20) << 2);

	set_palette();

	machine().tilemap().mark_all_dirty();
	m_bg_tilemap->set_scrolly(0, *m_scroll);
	machine().tilemap().set_flip_all(flip ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect, m_gfxdecode->gfx(2), flip);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}



/*************************************
 *
 *  Graphics decoding
 *
 *************************************/

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7,
		16*0+0, 16*0+1, 16*0+2, 16*0+3, 16*0+4, 16*0+5, 16*0+6, 16*0+7 },
	{ 0*8, 1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8,
		8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8
};


static GFXDECODE_START( gfx_mystston )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x3_planar, 4*8, 4 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 2*8, 1 )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout, 0*8, 2 )
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void mystston_state::video(machine_config &config)
{
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_mystston);
	PALETTE(config, m_palette).set_entries(0x40);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART);
	m_screen->set_screen_update(FUNC(mystston_state::screen_update));
	m_screen->set_palette(m_palette);
}
