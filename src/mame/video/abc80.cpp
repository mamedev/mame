// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*****************************************************************************
 *
 * video/abc80.c
 *
 ****************************************************************************/

#include "emu.h"
#include "includes/abc80.h"
#include "screen.h"



//-------------------------------------------------
//  gfx_layout charlayout
//-------------------------------------------------

static const gfx_layout charlayout =
{
	6, 10,
	128,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8 },
	10*8
};


//-------------------------------------------------
//  GFXDECODE( abc80 )
//-------------------------------------------------

static GFXDECODE_START( gfx_abc80 )
	GFXDECODE_ENTRY( "chargen", 0,     charlayout, 0, 1 ) // normal characters
	GFXDECODE_ENTRY( "chargen", 0x500, charlayout, 0, 1 ) // graphics characters
GFXDECODE_END


void abc80_state::draw_scanline(bitmap_rgb32 &bitmap, int y)
{
	uint8_t vsync_data = m_vsync_prom->base()[y];
	uint8_t l = m_line_prom->base()[y];
	int dv = (vsync_data & ABC80_K2_DV) ? 1 : 0;

	if (!(vsync_data & ABC80_K2_FRAME_RESET))
	{
		// reset F2
		m_r = 0;
	}

	for (int sx = 0; sx < 64; sx++)
	{
		uint8_t hsync_data = m_hsync_prom->base()[sx];
		int dh = (hsync_data & ABC80_K5_DH) ? 1 : 0;
		uint8_t data = 0;

		if (hsync_data & ABC80_K5_LINE_END)
		{
			// reset F4
			m_c = 0;

			// reset J5
			m_mode = 0;
		}

		/*

		    Video RAM Addressing Scheme

		    A9 A8 A7 A6 A5 A4 A3 A2 A1 A0
		    R2 R1 R0 xx xx xx xx C2 C1 C0

		    A6 A5 A4 A3 = 00 C5 C4 C3 + R4 R3 R4 R3

		*/

		int a = (m_c >> 3) & 0x07;
		int b = ((m_r >> 1) & 0x0c) | ((m_r >> 3) & 0x03);
		int s = (a + b) & 0x0f;
		uint16_t videoram_addr = ((m_r & 0x07) << 7) | (s << 3) | (m_c & 0x07);
		uint8_t videoram_data = m_latch;
		uint8_t attr_addr = ((dh & dv) << 7) | (videoram_data & 0x7f);
		uint8_t attr_data = m_attr_prom->base()[attr_addr];

		int blank = (attr_data & ABC80_J3_BLANK) ? 1 : 0;
		int j = (attr_data & ABC80_J3_TEXT) ? 1 : 0;
		int k = (attr_data & ABC80_J3_GRAPHICS) ? 1 : 0;
		int versal = (attr_data & ABC80_J3_VERSAL) ? 1 : 0;
		int cursor = (videoram_data & ABC80_CHAR_CURSOR) ? 1 : 0;

		if (!j && k) m_mode = 0;
		if (j && !k) m_mode = 1;
		if (j && k) m_mode = !m_mode;

		if (m_mode & versal)
		{
			// graphics mode
			int r0 = 1, r1 = 1, r2 = 1;

			if (l < 3) r0 = 0; else if (l < 7) r1 = 0; else r2 = 0;

			int c0 = BIT(videoram_data, 0) || r0;
			int c1 = BIT(videoram_data, 1) || r0;
			int c2 = BIT(videoram_data, 2) || r1;
			int c3 = BIT(videoram_data, 3) || r1;
			int c4 = BIT(videoram_data, 4) || r2;
			int c5 = BIT(videoram_data, 6) || r2;

			if (c0 && c2 && c4) data |= 0xe0;
			if (c1 && c3 && c5) data |= 0x1c;
		}
		else
		{
			// text mode
			uint16_t chargen_addr = ((videoram_data & 0x7f) * 10) + l;

			data = m_char_rom->base()[chargen_addr];
		}

		// shift out pixels
		for (int bit = 0; bit < 6; bit++)
		{
			int color = BIT(data, 7);
			int x = (sx * 6) + bit;

			color ^= (cursor & m_blink);
			color &= blank;

			bitmap.pix(y, x) = m_palette->pen(color);

			data <<= 1;
		}

		m_latch = m_video_ram[videoram_addr];

		if (hsync_data & ABC80_K5_ROW_START)
		{
			// clock F4
			m_c++;
		}
	}

	if (vsync_data & ABC80_K2_FRAME_END)
	{
		// clock F2
		m_r++;
	}
}


//-------------------------------------------------
//  video_start -
//-------------------------------------------------

void abc80_state::video_start()
{
	m_screen->register_screen_bitmap(m_bitmap);

	// start timers
	m_scanline_timer = timer_alloc(FUNC(abc80_state::scanline_tick), this);
	m_scanline_timer->adjust(m_screen->time_until_pos(0, ABC80_HBEND), 0, m_screen->scan_period());

	m_blink_timer = timer_alloc(FUNC(abc80_state::blink_tick), this);
	m_blink_timer->adjust(attotime::from_hz(XTAL(11'980'800)/2/6/64/312/16), 0, attotime::from_hz(XTAL(11'980'800)/2/6/64/312/16));

	m_vsync_on_timer = timer_alloc(FUNC(abc80_state::vsync_on), this);
	m_vsync_on_timer->adjust(m_screen->time_until_pos(0, 0), 0, m_screen->frame_period());

	m_vsync_off_timer = timer_alloc(FUNC(abc80_state::vsync_off), this);
	m_vsync_off_timer->adjust(m_screen->time_until_pos(16, 0), 0, m_screen->frame_period());
}


//-------------------------------------------------
//  screen_update -
//-------------------------------------------------

uint32_t abc80_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);

	return 0;
}


//-------------------------------------------------
//  machine_config( abc80_video )
//-------------------------------------------------

void abc80_state::abc80_video(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_screen_update(FUNC(abc80_state::screen_update));
	m_screen->set_raw(XTAL(11'980'800)/2, ABC80_HTOTAL, ABC80_HBEND, ABC80_HBSTART, ABC80_VTOTAL, ABC80_VBEND, ABC80_VBSTART);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_abc80);
	PALETTE(config, m_palette, palette_device::MONOCHROME);
}
