// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*****************************************************************************
 *
 * video/abc80.c
 *
 ****************************************************************************/

#include "emu.h"
#include "abc80.h"
#include "screen.h"

void tkn80_state::set_screen_params(void)
{
	if (m_80)
	{
		m_screen->set_raw(XTAL(11'980'800), ABC80_HTOTAL*2, ABC80_HBEND*2, ABC80_HBSTART*2, ABC80_VTOTAL, ABC80_VBEND, ABC80_VBSTART);
		m_scanline_timer->adjust(m_screen->time_until_pos(0, ABC80_HBEND*2), 0, m_screen->scan_period());
	}
	else
	{
		m_screen->set_raw(XTAL(11'980'800)/2, ABC80_HTOTAL, ABC80_HBEND, ABC80_HBSTART, ABC80_VTOTAL, ABC80_VBEND, ABC80_VBSTART);
		m_scanline_timer->adjust(m_screen->time_until_pos(0, ABC80_HBEND), 0, m_screen->scan_period());
	}
}

void tkn80_state::set_80(bool state)
{
	u8 config = m_config->read();
	int view = BIT(config, 2) + 1;

	/*

	    TKN 000-3ff -> ZA3506 000-3ff (9913/10042)
	    TKN 400-7ff -> ZA3507 000-3ff (9913/10042)
	    TKN 800-cff -> ZA3506 000-3ff (11273)
	    TKN c00-FFF -> ZA3507 000-3ff (11273)

	*/

	if (state)
	{
		m_80 = true;

		m_view_rom0.select(view);
		m_view_rom2.select(view);
		
		set_screen_params();
	}
	else
	{
		m_80 = false;

		m_view_rom0.select(0);
		m_view_rom2.select(0);

		set_screen_params();
	}
}

uint8_t tkn80_state::in3_r()
{
	if (!machine().side_effects_disabled() && m_80)
	{
		set_80(false);
	}

	return 0xff;
};

uint8_t tkn80_state::in4_r()
{
	if (!machine().side_effects_disabled() && !m_80)
	{
		set_80(true);
	}

	return 0xff;
};

void abc80_state::draw_scanline(bitmap_rgb32 &bitmap, int y)
{
	u8 vsync_data = m_vsync_prom->base()[y];
	bool dv = (vsync_data & ABC80_K2_DV) ? 1 : 0;

	if (!(vsync_data & ABC80_K2_FRAME_RESET))
	{
		// reset F2
		m_r = 0;
	}

	for (int sx = 0; sx < 64; sx++)
	{
		u8 hsync_data = m_hsync_prom->base()[sx];

		if (hsync_data & ABC80_K5_LINE_END)
		{
			// reset F4
			m_c = 0;

			// reset J5
			m_mode = 0;
		}

		draw_character(bitmap, y, sx, dv, hsync_data);

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

void tkn80_state::draw_scanline(bitmap_rgb32 &bitmap, int y)
{
	u8 vsync_data = m_vsync_prom->base()[y];
	bool dv = (vsync_data & ABC80_K2_DV) ? 1 : 0;

	if (!(vsync_data & ABC80_K2_FRAME_RESET))
	{
		// reset F2
		m_r = 0;
	}

	int cols = m_80 ? 128 : 64;
	for (int sx = 0; sx < cols; sx++)
	{
		u8 hsync_data = m_hsync_prom->base()[m_80 ? (sx / 2) : sx];

		if (hsync_data & ABC80_K5_LINE_END)
		{
			// reset F4
			m_c = 0;

			// reset J5
			m_mode = 0;
		}

		draw_character(bitmap, y, sx, dv, hsync_data);

		if (hsync_data & ABC80_K5_ROW_START)
		{
			if (!m_80 || sx > 30) {
				// clock F4
				m_c++;
			}
		}
	}

	if (vsync_data & ABC80_K2_FRAME_END)
	{
		// clock F2
		m_r++;
	}
}

void abc80_state::draw_character(bitmap_rgb32 &bitmap, int y, int sx, bool dv, u8 hsync_data)
{
	u8 l = m_line_prom->base()[y];
	bool dh = (hsync_data & ABC80_K5_DH) ? 1 : 0;
	u8 data = 0;

	u16 videoram_addr = get_videoram_addr();
	u8 videoram_data = m_latch;
	u8 attr_addr = ((dh & dv) << 7) | (videoram_data & 0x7f);
	u8 attr_data = m_attr_prom->base()[attr_addr];

	bool blank = (attr_data & ABC80_J3_BLANK) ? 1 : 0;
	bool j = (attr_data & ABC80_J3_TEXT) ? 1 : 0;
	bool k = (attr_data & ABC80_J3_GRAPHICS) ? 1 : 0;
	bool versal = (attr_data & ABC80_J3_VERSAL) ? 1 : 0;
	bool cursor = (videoram_data & ABC80_CHAR_CURSOR) ? 1 : 0;

	if (!j && k) m_mode = 0;
	if (j && !k) m_mode = 1;
	if (j && k) m_mode = !m_mode;

	if (m_mode & versal)
	{
		// graphics mode
		bool r0 = 1, r1 = 1, r2 = 1;

		if (l < 3) r0 = 0; else if (l < 7) r1 = 0; else r2 = 0;

		bool c0 = BIT(videoram_data, 0) || r0;
		bool c1 = BIT(videoram_data, 1) || r0;
		bool c2 = BIT(videoram_data, 2) || r1;
		bool c3 = BIT(videoram_data, 3) || r1;
		bool c4 = BIT(videoram_data, 4) || r2;
		bool c5 = BIT(videoram_data, 6) || r2;

		if (c0 && c2 && c4) data |= 0xe0;
		if (c1 && c3 && c5) data |= 0x1c;
	}
	else
	{
		// text mode
		data = m_rocg->read(videoram_data & 0x7f, l);
	}

	// shift out pixels
	for (int bit = 0; bit < 6; bit++)
	{
		bool color = BIT(data, 7);
		int x = (sx * 6) + bit;

		color ^= (cursor & m_blink);
		color &= blank;

		bitmap.pix(y, x) = m_palette->pen(color);

		data <<= 1;
	}

	m_latch = read_videoram(videoram_addr);
}

offs_t abc80_state::get_videoram_addr()
{
	/*

	    Video RAM Addressing Scheme

	    A9 A8 A7 A6 A5 A4 A3 A2 A1 A0
	    R2 R1 R0 xx xx xx xx C2 C1 C0

	             A6 A5 A4 A3 = 00 C5 C4 C3 + R4 R3 R4 R3

	*/

	int a = (m_c >> 3) & 0x07;
	int b = ((m_r >> 1) & 0x0c) | ((m_r >> 3) & 0x03);
	int s = (a + b) & 0x0f;

	return ((m_r & 0x07) << 7) | (s << 3) | (m_c & 0x07);
}

offs_t tkn80_state::get_videoram_addr()
{
	if (m_80)
	{
		/*

		    Video RAM Addressing Scheme

		    A10 A9 A8 A7 A6 A5 A4 A3 A2 A1 A0
		    R2 R1 R0 xx xx xx xx C3 C2 C1 C0

		             A7 A6 A5 A4 = 00 C6 C5 C4 + R4 R3 R4 R3

		*/

		int a = (m_c >> 4) & 0x07;
		int b = ((m_r >> 1) & 0x0c) | ((m_r >> 3) & 0x03);
		int s = (a + b) & 0x1f;

		return ((m_r & 0x07) << 8) | (s << 4) | (m_c & 0x0f);
	}
	else
	{
		/*

		    Video RAM Addressing Scheme

		    A10 A9 A8 A7 A6 A5 A4 A3 A2 A1 A0
		      1 R2 R1 R0 xx xx xx xx C2 C1 C0

		                 A6 A5 A4 A3 = 00 C5 C4 C3 + R4 R3 R4 R3

		*/

		int a = (m_c >> 3) & 0x07;
		int b = ((m_r >> 1) & 0x0c) | ((m_r >> 3) & 0x03);
		int s = (a + b) & 0x0f;

		return 0x400 | ((m_r & 0x07) << 7) | (s << 3) | (m_c & 0x07);
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
//  video_reset -
//-------------------------------------------------

void tkn80_state::video_reset()
{
	u8 config = m_config->read();

	m_80 = BIT(config, 0);
	set_80(m_80);

	m_blink = 1;
	m_blink_timer->enable(BIT(config, 1));
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
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_screen_update(FUNC(abc80_state::screen_update));
	m_screen->set_raw(XTAL(11'980'800)/2, ABC80_HTOTAL, ABC80_HBEND, ABC80_HBSTART, ABC80_VTOTAL, ABC80_VBEND, ABC80_VBSTART);

	SN74S263(config, m_rocg, 0);
	m_rocg->set_palette(m_palette);
}


//-------------------------------------------------
//  machine_config( tkn80_video )
//-------------------------------------------------

void tkn80_state::tkn80_video(machine_config &config)
{
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_screen_update(FUNC(tkn80_state::screen_update));
	m_screen->set_raw(XTAL(11'980'800), ABC80_HTOTAL*2, ABC80_HBEND*2, ABC80_HBSTART*2, ABC80_VTOTAL, ABC80_VBEND, ABC80_VBSTART);

	SN74S263(config, m_rocg, 0);
	m_rocg->set_palette(m_palette);
}
