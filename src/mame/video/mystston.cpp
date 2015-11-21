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

#define PIXEL_CLOCK     (MYSTSTON_MASTER_CLOCK / 2)
#define HTOTAL          (0x180)
#define HBEND           (0x000)
#define HBSTART         (0x100)
#define VTOTAL          (0x110)  /* counts from 0x08-0xff, then from 0xe8-0xff */
#define VBEND           (0x008)
#define VBSTART         (0x0f8)
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

	mystston_on_scanline_interrupt();

	scanline = scanline + 16;
	if (scanline >= VTOTAL)
		scanline = FIRST_INT_VPOS;

	/* the vertical synch chain is clocked by H256 -- this is probably not important, but oh well */
	m_interrupt_timer->adjust(m_screen->time_until_pos(scanline - 1, INT_HPOS), scanline);
}



/*************************************
 *
 *  Palette handling
 *
 *************************************/

void mystston_state::set_palette()
{
	int i;
	static const int resistances_rg[3] = { 4700, 3300, 1500 };
	static const int resistances_b [2] = { 3300, 1500 };
	double weights_rg[3], weights_b[2];

	UINT8 *color_prom = memregion("proms")->base();

	compute_resistor_weights(0, 255, -1.0,
			3, resistances_rg, weights_rg, 0, 4700,
			2, resistances_b,  weights_b,  0, 4700,
			0, 0, 0, 0, 0);

	for (i = 0; i < 0x40; i++)
	{
		UINT8 data;
		int r, g, b;
		int bit0, bit1, bit2;

		/* first half is dynamic, second half is from the PROM */
		if (i & 0x20)
			data = color_prom[i & 0x1f];
		else
			data = m_paletteram[i];

		/* red component */
		bit0 = (data >> 0) & 0x01;
		bit1 = (data >> 1) & 0x01;
		bit2 = (data >> 2) & 0x01;
		r = combine_3_weights(weights_rg, bit0, bit1, bit2);

		/* green component */
		bit0 = (data >> 3) & 0x01;
		bit1 = (data >> 4) & 0x01;
		bit2 = (data >> 5) & 0x01;
		g = combine_3_weights(weights_rg, bit0, bit1, bit2);

		/* blue component */
		bit0 = (data >> 6) & 0x01;
		bit1 = (data >> 7) & 0x01;
		b = combine_2_weights(weights_b, bit0, bit1);

		m_palette->set_pen_color(i, rgb_t(r, g, b));
	}
}



/*************************************
 *
 *  Video control register
 *
 *************************************/

WRITE8_MEMBER(mystston_state::mystston_video_control_w)
{
	*m_video_control = data;

	/* D0-D1 - foreground text color */
	/* D2 - background page select */
	/* D3 - unused */

	/* D4-D5 - coin counters in flipped order */
	coin_counter_w(machine(), 0, data & 0x20);
	coin_counter_w(machine(), 1, data & 0x10);

	/* D6 - unused */
	/* D7 - screen flip */
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

	SET_TILE_INFO_MEMBER(1, code, 0, flags);
}


TILE_GET_INFO_MEMBER(mystston_state::get_fg_tile_info)
{
	int code = ((m_fg_videoram[0x400 | tile_index] & 0x07) << 8) | m_fg_videoram[tile_index];
	int color = ((*m_video_control & 0x01) << 1) | ((*m_video_control & 0x02) >> 1);

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}



/*************************************
 *
 *  Sprite drawing
 *
 *************************************/

void mystston_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx, int flip)
{
	int offs;

	for (offs = 0; offs < 0x60; offs += 4)
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

VIDEO_START_MEMBER(mystston_state,mystston)
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(mystston_state::get_bg_tile_info),this), TILEMAP_SCAN_COLS_FLIP_X, 16, 16, 16, 32);

	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(mystston_state::get_fg_tile_info),this), TILEMAP_SCAN_COLS_FLIP_X,  8,  8, 32, 32);
	m_fg_tilemap->set_transparent_pen(0);

	/* create the interrupt timer */
	m_interrupt_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(mystston_state::interrupt_callback),this));
}



/*************************************
 *
 *  Reset
 *
 *************************************/

VIDEO_RESET_MEMBER(mystston_state,mystston)
{
	m_interrupt_timer->adjust(m_screen->time_until_pos(FIRST_INT_VPOS - 1, INT_HPOS), FIRST_INT_VPOS);
}



/*************************************
 *
 *  Update
 *
 *************************************/

UINT32 mystston_state::screen_update_mystston(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int flip = (*m_video_control & 0x80) ^ ((ioport("DSW1")->read() & 0x20) << 2);

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

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


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


static GFXDECODE_START( mystston )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   4*8, 4 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 2*8, 1 )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout, 0*8, 2 )
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

MACHINE_CONFIG_FRAGMENT( mystston_video )
	MCFG_VIDEO_START_OVERRIDE(mystston_state,mystston)
	MCFG_VIDEO_RESET_OVERRIDE(mystston_state,mystston)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", mystston)
	MCFG_PALETTE_ADD("palette", 0x40)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART)
	MCFG_SCREEN_UPDATE_DRIVER(mystston_state, screen_update_mystston)
	MCFG_SCREEN_PALETTE("palette")
MACHINE_CONFIG_END
