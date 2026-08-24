// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    Luxor ABC 800/802/806 video

***************************************************************************/

#include "emu.h"
#include "abc80x.h"
#include "screen.h"

//#define VERBOSE 1
#include "logmacro.h"


//**************************************************************************
//  HIGH RESOLUTION GRAPHICS
//**************************************************************************

//-------------------------------------------------
//  hrs_w - high resolution scanline write
//-------------------------------------------------

void abc800_state::hrs_w(uint8_t data)
{
	m_hrs = data;
}


//-------------------------------------------------
//  hrc_w - high resolution color write
//-------------------------------------------------

void abc800_state::hrc_w(uint8_t data)
{
	m_fgctl = data;
}



//**************************************************************************
//  ABC 800 COLOR
//**************************************************************************

//-------------------------------------------------
//  hr_update - high resolution screen update
//-------------------------------------------------

void abc800c_state::hr_update(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const pen_t *pen = m_palette->pens();

	uint16_t addr = 0;

	for (int y = m_hrs; y < std::min(cliprect.max_y + 1, m_hrs + 480); y += 2)
	{
		int x = 0;

		for (int sx = 0; sx < 64; sx++)
		{
			uint8_t data = m_video_ram[addr++];

			for (int dot = 0; dot < 4; dot++)
			{
				uint16_t fgctl_addr = ((m_fgctl & 0x7f) << 2) | ((data >> 6) & 0x03);
				uint8_t fgctl = m_fgctl_prom->base()[fgctl_addr];
				int color = fgctl & 0x07;

				if (color)
				{
					bool black = bitmap.pix(y, x) == rgb_t::black();
					bool opaque = !BIT(fgctl, 3);

					if (black || opaque)
					{
						bitmap.pix(y, x) = pen[color];
						bitmap.pix(y, x + 1) = pen[color];

						bitmap.pix(y + 1, x) = pen[color];
						bitmap.pix(y + 1, x + 1) = pen[color];
					}
				}

				data <<= 2;
				x += 2;
			}
		}
	}
}


//-------------------------------------------------
//  SCREEN_UPDATE( abc800c )
//-------------------------------------------------

uint32_t abc800c_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// clear screen
	bitmap.fill(rgb_t::black(), cliprect);

	// draw text
	if (!BIT(m_fgctl, 7))
	{
		m_trom->screen_update(screen, bitmap, cliprect);
	}

	// draw HR graphics
	hr_update(bitmap, cliprect);

	return 0;
}


//-------------------------------------------------
//  SAA5050_INTERFACE( trom_intf )
//-------------------------------------------------

uint8_t abc800c_state::char_ram_r(offs_t offset)
{
	int row = offset / 40;
	int col = offset % 40;

	offset = ((row & 0x07) * 0x80) + col;

	if (row & 0x08) offset += 0x28;
	if (row & 0x10) offset += 0x50;

	return m_char_ram[offset];
}


//-------------------------------------------------
//  PALETTE_INIT( abc800c )
//-------------------------------------------------

void abc800c_state::abc800c_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t::black());
	palette.set_pen_color(1, rgb_t(0xff, 0x00, 0x00)); // red
	palette.set_pen_color(2, rgb_t(0x00, 0xff, 0x00)); // green
	palette.set_pen_color(3, rgb_t(0xff, 0xff, 0x00)); // yellow
	palette.set_pen_color(4, rgb_t(0x00, 0x00, 0xff)); // blue
	palette.set_pen_color(5, rgb_t(0xff, 0x00, 0xff)); // magenta
	palette.set_pen_color(6, rgb_t(0x00, 0xff, 0xff)); // cyan
	palette.set_pen_color(7, rgb_t::white());
}


//-------------------------------------------------
//  machine_config( abc800c_video )
//-------------------------------------------------

void abc800c_state::abc800c_video(machine_config &config)
{
	screen_device &screen(SCREEN(config, SCREEN_TAG));
	screen.set_screen_update(FUNC(abc800c_state::screen_update));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(480, 480);
	screen.set_visarea(0, 480-1, 0, 480-1);

	PALETTE(config, m_palette, FUNC(abc800c_state::abc800c_palette), 8);

	SAA5052(config, m_trom, XTAL(12'000'000)/2);
	m_trom->d_cb().set(FUNC(abc800c_state::char_ram_r));
	m_trom->set_screen_size(40, 24, 40);
}



//**************************************************************************
//  ABC 800 MONOCHROME
//**************************************************************************

//-------------------------------------------------
//  hr_update - high resolution screen update
//-------------------------------------------------

void abc800m_state::hr_update(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// this is needed because the MC6845 emulation does
	// not position the active display area correctly
	constexpr int HORIZONTAL_PORCH_HACK = 115;

	uint16_t addr = 0;

	const pen_t *pen = m_palette->pens();

	for (int y = m_hrs; y < std::min(cliprect.max_y + 1, m_hrs + 240); y++)
	{
		int x = HORIZONTAL_PORCH_HACK;

		for (int sx = 0; sx < 64; sx++)
		{
			uint8_t data = m_video_ram[addr++];

			for (int dot = 0; dot < 4; dot++)
			{
				uint16_t fgctl_addr = ((m_fgctl & 0x7f) << 2) | ((data >> 6) & 0x03);
				int color = (m_fgctl_prom->base()[fgctl_addr] & 0x07) ? 1 : 0;

				bitmap.pix(y, x++) = pen[color];
				bitmap.pix(y, x++) = pen[color];

				data <<= 2;
			}
		}
	}
}


//-------------------------------------------------
//  MC6845_UPDATE_ROW( abc800m_update_row )
//-------------------------------------------------

MC6845_UPDATE_ROW( abc800m_state::abc800m_update_row )
{
	int column;
	rgb_t fgpen = m_palette->pen(1);

	y += vbp;

	for (column = 0; column < x_count; column++)
	{
		int bit;

		uint16_t address = (m_char_ram[(ma + column) & 0x7ff] << 4) | (ra & 0x0f);
		uint8_t data = (m_char_rom->base()[address & 0x7ff] & 0x3f);

		if (column == cursor_x)
		{
			data = 0x3f;
		}

		data <<= 2;

		for (bit = 0; bit < ABC800_CHAR_WIDTH; bit++)
		{
			int x = hbp + (column * ABC800_CHAR_WIDTH) + bit;

			if (BIT(data, 7) && de)
			{
				bitmap.pix(y, x) = fgpen;
			}

			data <<= 1;
		}
	}
}


//-------------------------------------------------
//  SCREEN_UPDATE( abc800m )
//-------------------------------------------------

uint32_t abc800m_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// clear screen
	bitmap.fill(rgb_t::black(), cliprect);

	// draw HR graphics
	hr_update(bitmap, cliprect);

	// draw text
	if (!BIT(m_fgctl, 7))
	{
		m_crtc->screen_update(screen, bitmap, cliprect);
	}

	return 0;
}


void abc800_state::video_start()
{
	// register for state saving
	save_item(NAME(m_hrs));
	save_item(NAME(m_fgctl));
}



//-------------------------------------------------
//  machine_config( abc800m_video )
//-------------------------------------------------

void abc800m_state::abc800m_video(machine_config &config)
{
	mc6845_device &mc6845(MC6845(config, MC6845_TAG, ABC800_CCLK));
	mc6845.set_screen(SCREEN_TAG);
	mc6845.set_show_border_area(true);
	mc6845.set_char_width(ABC800_CHAR_WIDTH);
	mc6845.set_update_row_callback(FUNC(abc800m_state::abc800m_update_row));
	mc6845.out_vsync_callback().set(m_dart, FUNC(z80dart_device::rib_w)).invert();

	screen_device &screen(SCREEN(config, SCREEN_TAG).set_color(rgb_t(0xff, 0xff, 0x00)));
	screen.set_screen_update(FUNC(abc800m_state::screen_update));
	screen.set_raw(XTAL(12'000'000), 0x300, 0, 0x1e0, 0x13a, 0, 0xf0);

	PALETTE(config, m_palette, palette_device::MONOCHROME);
}



//-------------------------------------------------
//  MC6845_UPDATE_ROW( abc802_update_row )
//-------------------------------------------------

MC6845_UPDATE_ROW( abc802_state::abc802_update_row )
{
	/*

	    PAL16R4 equation:

	    IF (VCC)    *OS   = FC + RF / RC
	                *RG:  = HS / *RG + *ATE / *RG + ATD / *RG + LL /
	                        *RG + AT1 / *RG + AT0 / ATE + *ATD + *LL +
	                        *AT1 + *AT0
	                *RI:  = *RI + *INV / *RI + LL / *INV + *LL
	                *RF:  = HS / *RF + *ATE / *RF + ATD / *RF + LL /
	                        *RF + AT1 / *RF + AT0 / ATE + *ATD + *LL +
	                        *AT1 + AT0
	                *RC:  = HS / *RC + *ATE / *RC + *ATD / *RC + LL /
	                        *RC + *ATI / *RC + AT0 / ATE + *LL + *AT1 +
	                        *AT0
	    IF (VCC)    *O0   = *CUR + *AT0 / *CUR + ATE
	                *O1   = *CUR + *AT1 / *CUR + ATE


	    + = AND
	    / = OR
	    * = Inverted

	    ATD     Attribute data
	    ATE     Attribute enable
	    AT0,AT1 Attribute address
	    CUR     Cursor
	    FC      FLSH clock
	    HS      Horizontal sync
	    INV     Inverted signal input
	    LL      Load when Low
	    OEL     Output Enable when Low
	    RC      Row clear
	    RF      Row flash
	    RG      Row graphic
	    RI      Row inverted

	*/

	const pen_t *pen = m_palette->pens();

	int rf = 0, rc = 0, rg = 0;

	y += vbp;

	for (int column = 0; column < x_count; column++)
	{
		uint8_t code = m_char_ram[(ma + column) & 0x7ff];
		uint16_t address = code << 4;
		uint8_t ra_latch = ra;
		uint8_t data;

		int ri = (code & ABC802_INV) ? 1 : 0;

		if (column == cursor_x)
		{
			ra_latch = 0x0f;
		}

		if ((m_flshclk && rf) || rc)
		{
			ra_latch = 0x0e;
		}

		if (rg)
		{
			address |= 0x800;
		}

		data = m_char_rom->base()[(address + ra_latch) & 0xfff];

		if (data & ABC802_ATE)
		{
			int attr = data & 0x03;
			int value = (data & ABC802_ATD) ? 1 : 0;

			switch (attr)
			{
			case 0x00:
				// Row Graphic
				rg = value;
				break;

			case 0x01:
				// Row Flash
				rf = value;
				break;

			case 0x02:
				// Row Clear
				rc = value;
				break;

			case 0x03:
				// undefined
				break;
			}
		}
		else
		{
			data <<= 2;

			if (m_80_40_mux)
			{
				for (int bit = 0; bit < ABC800_CHAR_WIDTH; bit++)
				{
					int x = hbp + ((column + 3) * ABC800_CHAR_WIDTH) + bit;
					int color = (BIT(data, 7) ^ ri) && de;

					bitmap.pix(y, x) = pen[color];

					data <<= 1;
				}
			}
			else
			{
				for (int bit = 0; bit < ABC800_CHAR_WIDTH; bit++)
				{
					int x = hbp + ((column + 3) * ABC800_CHAR_WIDTH) + (bit << 1);
					int color = (BIT(data, 7) ^ ri) && de;

					bitmap.pix(y, x) = pen[color];
					bitmap.pix(y, x + 1) = pen[color];

					data <<= 1;
				}

				column++;
			}
		}
	}
}


//-------------------------------------------------
//  vs_w - vertical sync write
//-------------------------------------------------

void abc802_state::vs_w(int state)
{
	if (!state)
	{
		// flash clock
		if (m_flshclk_ctr & 0x20)
		{
			m_flshclk = !m_flshclk;
			m_flshclk_ctr = 0;
		}
		else
		{
			m_flshclk_ctr++;
		}
	}
}


void abc802_state::video_start()
{
	// register for state saving
	save_item(NAME(m_flshclk_ctr));
	save_item(NAME(m_flshclk));
	save_item(NAME(m_80_40_mux));
}


//-------------------------------------------------
//  machine_config( abc802_video )
//-------------------------------------------------

void abc802_state::abc802_video(machine_config &config)
{
	mc6845_device &mc6845(MC6845(config, MC6845_TAG, ABC800_CCLK));
	mc6845.set_screen(SCREEN_TAG);
	mc6845.set_show_border_area(true);
	mc6845.set_char_width(ABC800_CHAR_WIDTH);
	mc6845.set_update_row_callback(FUNC(abc802_state::abc802_update_row));
	mc6845.out_vsync_callback().set(FUNC(abc802_state::vs_w));
	mc6845.out_vsync_callback().append(m_dart, FUNC(z80dart_device::rib_w)).invert();

	screen_device &screen(SCREEN(config, SCREEN_TAG).set_color(rgb_t::amber()));
	screen.set_screen_update(MC6845_TAG, FUNC(mc6845_device::screen_update));
	screen.set_raw(XTAL(12'000'000), 0x300, 0, 0x1e0, 0x13a, 0, 0xf0);

	PALETTE(config, m_palette, palette_device::MONOCHROME);
}


//-------------------------------------------------
//  hrs_w - high resolution memory banking
//-------------------------------------------------

void abc806_state::hrs_w(uint8_t data)
{
	/*

	    bit     signal  description

	    0       VM15    visible screen memory area bit 0
	    1       VM16    visible screen memory area bit 1
	    2       VM17    visible screen memory area bit 2
	    3       VM18    visible screen memory area bit 3
	    4       F15     cpu accessible screen memory area bit 0
	    5       F16     cpu accessible screen memory area bit 1
	    6       F17     cpu accessible screen memory area bit 2
	    7       F18     cpu accessible screen memory area bit 3

	*/

	LOG("%s HRS %02x\n", machine().describe_context(), data);

	m_hrs = data;
}


//-------------------------------------------------
//  hrc_w - high resolution color write
//-------------------------------------------------

void abc806_state::hrc_w(offs_t offset, uint8_t data)
{
	int reg = (offset >> 8) & 0x0f;

	m_hrc[reg] = data;
}


//-------------------------------------------------
//  charram_r - character RAM read
//-------------------------------------------------

uint8_t abc806_state::charram_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		m_attr_data = m_attr_ram[offset];

	return m_char_ram[offset];
}


//-------------------------------------------------
//  charram_w - character RAM write
//-------------------------------------------------

void abc806_state::charram_w(offs_t offset, uint8_t data)
{
	if (!machine().side_effects_disabled())
		m_attr_ram[offset] = m_attr_data;

	m_char_ram[offset] = data;
}


//-------------------------------------------------
//  ami_r - attribute memory read
//-------------------------------------------------

uint8_t abc806_state::ami_r()
{
	return m_attr_data;
}


//-------------------------------------------------
//  amo_w - attribute memory write
//-------------------------------------------------

void abc806_state::amo_w(uint8_t data)
{
	m_attr_data = data;
}


//-------------------------------------------------
//  cli_r - palette PROM read
//-------------------------------------------------

uint8_t abc806_state::cli_r(offs_t offset)
{
	/*

	    bit     description

	    0       HRU II data bit 0
	    1       HRU II data bit 1
	    2       HRU II data bit 2
	    3       HRU II data bit 3
	    4
	    5
	    6
	    7       RTC data output

	*/

	uint16_t hru2_addr = (m_hru2_a8 << 8) | (offset >> 8);
	uint8_t data = m_hru2_prom->base()[hru2_addr] & 0x0f;

	LOG("HRU II %03x : %01x\n", hru2_addr, data);

	data |= m_rtc->dio_r() << 7;

	return data;
}


//-------------------------------------------------
//  sti_r - protection device read
//-------------------------------------------------

uint8_t abc806_state::sti_r()
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4
	    5
	    6
	    7       PROT DOUT

	*/

	return 0x7f;
}


//-------------------------------------------------
//  sto_w -
//-------------------------------------------------

void abc806_state::sto_w(uint8_t data)
{
	m_sto->write_bit(data & 0x07, BIT(data, 7));
}


//-------------------------------------------------
//  eme_w - external memory enable
//-------------------------------------------------

void abc806_state::eme_w(int state)
{
	LOG("%s EME %u\n", machine().describe_context(), state);
	m_eme = state;
}


//-------------------------------------------------
//  _40_w - 40/80 column display
//-------------------------------------------------

void abc806_state::_40_w(int state)
{
	m_40 = state;
}


//-------------------------------------------------
//  hru2_a8_w - HRU II address line 8, PROT A0
//-------------------------------------------------

void abc806_state::hru2_a8_w(int state)
{
	m_hru2_a8 = state;
}


//-------------------------------------------------
//  prot_ini_w - PROT INI
//-------------------------------------------------

void abc806_state::prot_ini_w(int state)
{
}


//-------------------------------------------------
//  txoff_w - text display enable
//-------------------------------------------------

void abc806_state::txoff_w(int state)
{
	m_txoff = state;
}


//-------------------------------------------------
//  prot_din_w - PROT DIN
//-------------------------------------------------

void abc806_state::prot_din_w(int state)
{
}


//-------------------------------------------------
//  sso_w - sync offset write
//-------------------------------------------------

void abc806_state::sso_w(uint8_t data)
{
	m_sync = data & 0x3f;
}


//-------------------------------------------------
//  MC6845_UPDATE_ROW( abc806_update_row )
//-------------------------------------------------

MC6845_UPDATE_ROW( abc806_state::abc806_update_row )
{
	const pen_t *pen = m_palette->pens();

	int fg_color = 7;
	int bg_color = 0;
	int underline = 0;
	int flash = 0;
	int e5 = m_40;
	int e6 = m_40;
	int th = 0;

	y += m_sync + vbp;

	for (int column = 0; column < x_count; column++)
	{
		uint8_t data = m_char_ram[(ma + column) & 0x7ff];
		uint8_t attr = m_attr_ram[(ma + column) & 0x7ff];
		uint8_t rad_data;

		if ((attr & 0x07) == ((attr >> 3) & 0x07))
		{
			// special case
			switch (attr >> 6)
			{
			case 0:
				// use previously selected attributes
				break;

			case 1:
				// reserved for future use
				break;

			case 2:
				// blank
				fg_color = 0;
				bg_color = 0;
				underline = 0;
				flash = 0;
				break;

			case 3:
				// double width
				e5 = BIT(attr, 0);
				e6 = BIT(attr, 1);

				// read attributes from next byte
				attr = m_attr_ram[(ma + column + 1) & 0x7ff];

				if (attr != 0x00)
				{
					fg_color = attr & 0x07;
					bg_color = (attr >> 3) & 0x07;
					underline = BIT(attr, 6);
					flash = BIT(attr, 7);
				}
				break;
			}
		}
		else
		{
			// normal case
			fg_color = attr & 0x07;
			bg_color = (attr >> 3) & 0x07;
			underline = BIT(attr, 6);
			flash = BIT(attr, 7);
			e5 = m_40;
			e6 = m_40;
		}

		if (column == cursor_x)
		{
			rad_data = 0x0f;
		}
		else
		{
			uint16_t rad_addr = (e6 << 8) | (e5 << 7) | (flash << 6) | (underline << 4) | (m_flshclk << 5) | (ra & 0x0f);
			rad_data = m_rad_prom->base()[rad_addr] & 0x0f;
		}

		uint16_t chargen_addr = (th << 12) | (data << 4) | rad_data;
		uint8_t chargen_data = m_char_rom->base()[chargen_addr & 0xfff] << 2;
		int x = hbp + (column + 4) * ABC800_CHAR_WIDTH;

		for (int bit = 0; bit < ABC800_CHAR_WIDTH; bit++)
		{
			int color = BIT(chargen_data, 7) ? fg_color : bg_color;
			if (!de) color = 0;

			bitmap.pix(y, x++) = pen[color];

			if (e5 || e6)
			{
				bitmap.pix(y, x++) = pen[color];
			}

			chargen_data <<= 1;
		}

		if (e5 || e6)
		{
			column++;
		}
	}
}


//-------------------------------------------------
//  hs_w - horizontal sync write
//-------------------------------------------------

void abc806_state::hs_w(int state)
{
	int vsync;

	if (!state)
	{
		m_v50_addr++;

		// clock current vsync value into the shift register
		m_vsync_shift <<= 1;
		m_vsync_shift |= m_vsync;

		vsync = BIT(m_vsync_shift, m_sync);

		if (!m_d_vsync && vsync)
		{
			// clear V50 address
			m_v50_addr = 0;
		}
		else if (m_d_vsync && !vsync)
		{
			// flash clock
			if (m_flshclk_ctr & 0x20)
			{
				m_flshclk = !m_flshclk;
				m_flshclk_ctr = 0;
			}
			else
			{
				m_flshclk_ctr++;
			}
		}

		if (m_d_vsync != vsync)
		{
			// signal _DEW to DART
			m_dart->rib_w(!vsync);
		}

		m_d_vsync = vsync;
	}
}


//-------------------------------------------------
//  vs_w - vertical sync write
//-------------------------------------------------

void abc806_state::vs_w(int state)
{
	m_vsync = state;
}


//-------------------------------------------------
//  hr_update - high resolution screen update
//-------------------------------------------------

void abc806_state::hr_update(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	constexpr int HORIZONTAL_PORCH_HACK   = 109;
	constexpr int VERTICAL_PORCH_HACK     = 27;

	const pen_t *pen = m_palette->pens();

	uint32_t addr = (m_hrs & 0x0f) << 15;

	for (int y = m_sync + VERTICAL_PORCH_HACK; y < std::min(cliprect.max_y + 1, m_sync + VERTICAL_PORCH_HACK + 240); y++)
	{
		for (int sx = 0; sx < 128; sx++)
		{
			uint8_t data = videoram_read(addr++);
			uint16_t dot = (m_hrc[data >> 4] << 8) | m_hrc[data & 0x0f];

			for (int pixel = 0; pixel < 4; pixel++)
			{
				int x = HORIZONTAL_PORCH_HACK + (ABC800_CHAR_WIDTH * 4) - 16 + (sx * 4) + pixel;

				if (BIT(dot, 15) || (bitmap.pix(y, x) == rgb_t::black()))
				{
					bitmap.pix(y, x) = pen[(dot >> 12) & 0x07];
				}

				dot <<= 4;
			}
		}
	}
}


void abc806_state::video_start()
{
	// register for state saving
	save_item(NAME(m_txoff));
	save_item(NAME(m_40));
	save_item(NAME(m_flshclk_ctr));
	save_item(NAME(m_flshclk));
	save_item(NAME(m_attr_data));
	save_item(NAME(m_hrs));
	save_item(NAME(m_hrc));
	save_item(NAME(m_sync));
	save_item(NAME(m_v50_addr));
	save_item(NAME(m_hru2_a8));
	save_item(NAME(m_vsync_shift));
	save_item(NAME(m_vsync));
	save_item(NAME(m_d_vsync));

	// initialize variables
	for (auto & elem : m_hrc)
	{
		elem = 0;
	}

	m_sync = 10;
	m_d_vsync = 1;
	m_vsync = 1;
	m_40 = 1;
}


//-------------------------------------------------
//  SCREEN_UPDATE( abc806 )
//-------------------------------------------------

uint32_t abc806_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// clear screen
	bitmap.fill(rgb_t::black(), cliprect);

	if (!m_txoff)
	{
		// draw text
		m_crtc->screen_update(screen, bitmap, cliprect);
	}

	// draw HR graphics
	hr_update(bitmap, cliprect);

	return 0;
}


//-------------------------------------------------
//  PALETTE_INIT( abc806 )
//-------------------------------------------------

void abc806_state::abc806_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t::black());
	palette.set_pen_color(1, rgb_t(0xff, 0x00, 0x00)); // red
	palette.set_pen_color(2, rgb_t(0x00, 0xff, 0x00)); // green
	palette.set_pen_color(3, rgb_t(0xff, 0xff, 0x00)); // yellow
	palette.set_pen_color(4, rgb_t(0x00, 0x00, 0xff)); // blue
	palette.set_pen_color(5, rgb_t(0xff, 0x00, 0xff)); // magenta
	palette.set_pen_color(6, rgb_t(0x00, 0xff, 0xff)); // cyan
	palette.set_pen_color(7, rgb_t::white());
}


//-------------------------------------------------
//  machine_config( abc806_video )
//-------------------------------------------------

void abc806_state::abc806_video(machine_config &config)
{
	MC6845(config, m_crtc, ABC800_CCLK);
	m_crtc->set_screen(SCREEN_TAG);
	m_crtc->set_show_border_area(true);
	m_crtc->set_char_width(ABC800_CHAR_WIDTH);
	m_crtc->set_update_row_callback(FUNC(abc806_state::abc806_update_row));
	m_crtc->out_hsync_callback().set(FUNC(abc806_state::hs_w));
	m_crtc->out_vsync_callback().set(FUNC(abc806_state::vs_w));

	screen_device &screen(SCREEN(config, SCREEN_TAG));
	screen.set_screen_update(FUNC(abc806_state::screen_update));
	screen.set_raw(XTAL(12'000'000), 0x300, 0, 0x1e0, 0x13a, 0, 0xfa);

	PALETTE(config, m_palette, FUNC(abc806_state::abc806_palette), 8);
}
