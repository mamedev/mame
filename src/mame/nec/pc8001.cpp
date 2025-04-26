// license:BSD-3-Clause
// copyright-holders:Curt Coder, Angelo Salese
/*

References:
- http://www2.odn.ne.jp/~haf09260/Pc80/EnrPc.htm
- http://home1.catvmics.ne.jp/~kanemoto/n80/inside.html
- http://www.geocities.jp/retro_zzz/machines/nec/8001/index.html
- https://oldcrap.org/2024/07/08/nec-pc-8001/

*/

/*

    TODO:

    - uPD3301 attributes;
    - PCG-1000;
    - Intel 8251;
    - cassette;
    - dip-switches;
    - PC-8011 (expansion unit)
    - PC-8021;
    - PC-8031 (mini disk unit, in progress)
    - pc8001mk2sr: verify how much needs to be ported from pc8801.cpp code
      (Has 3 bitplane GVRAM like PC-8801 V1 mode);
    - waitstates & DMA penalty (some games are suspciously fast);
    - buzzer has pretty ugly aliasing in places;

    Notes:
    - pc8001 v1.01 / v1.02 sports a buggy readout of the expansion ROM at PC=17a1:
      It expects an header read of 0x41-0x42 at offset $6000-6001, but second read at
      PC=0x17aa is just a comparison to $6000 == 0x42, which is impossible at that point
      unless external aid is given. This has been fixed in v1.10;
    - Color Magical (pc8001gp:flop5 option 7) transfers two 8 color screens at
      even/odd frame intervals, effectively boosting the number of available colors to 27.
      This trick is kinda flickery even on real HW, no wonder it looks ugly in MAME,
      can it be improved?

*/

#include "emu.h"
#include "pc8001.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"
#include "utf8.h"

void pc8001_base_state::crtc_reverse_w(int state)
{
	// rvv acts as a global flip for reverse attribute meaning
	// (does not act on underlying palette)
	// TODO: what happens if RVV changes mid-frame?
	// I suspect monitor resync more likely than rasters.
	m_screen_reverse = state;
}

UPD3301_FETCH_ATTRIBUTE( pc8001_base_state::attr_fetch )
{
	const u8 attr_max_size = 80;
	const bool is_color_mode = gfx_mode == 0x2;
	std::array<u16, attr_max_size> attr_extend_info = m_crtc->default_attr_fetch(attr_row, gfx_mode, y, attr_fifo_size, row_size);

	// further extend the attributes if we are in color mode
	if (is_color_mode)
	{
		// flgworld (pc8001) gameplay sets up:
		// - 0x00 0x00 0x02 0x88 on playfield
		// \- (wanting the default from the first defined color)
		// - 0x00 0x00 0x00 0x48 0x12 0x88 for first row
		// \- (Expecting "FLAG WORLD" wording to be red while the "P"s in green)
		// undermon (pc8001) instruction screen sets up:
		// - 0x00 0x00 0x06 0xb8
		// \- (expecting blue fill up to 0x06)
		// xak2 & cancanb (pc8801) really expects that the color / decoration is implictly
		// latched from previous line (uses semigfx black tiles for masking)
		if (y == 0)
		{
			// TODO: default values for line 0
			m_attr_color = 0xe8;
			m_attr_decoration = 0x00;
		}

		for (int ex = 0; ex < row_size; ex++)
		{
			u16 cur_attr = attr_extend_info[ex];
			if (BIT(cur_attr, 3))
				m_attr_color = cur_attr;
			else
				m_attr_decoration = cur_attr;
			attr_extend_info[ex] = (m_attr_color << 8) | m_attr_decoration;
		}
	}

	return attr_extend_info;
}

UPD3301_DRAW_CHARACTER_MEMBER( pc8001_base_state::draw_text )
{
	// punt if we are in width 40 (discarded on this end)
	if (sx % 2 && !m_width80)
		return;

	const bool is_color_mode = gfx_mode == 0x2;
	u8 tile;
	const u8 tile_width = m_width80 ? 8 : 16;
	const u8 dot_width = (m_width80 ^ 1) + 1;
	const u8 y_double = m_screen_is_24KHz ? 2 : 1;
	const u8 y_height = y_double * 8;

	bool semigfx_tile, reverse, attr_blink, secret;
	bool upperline, lowerline;
	u8 color;

	if (is_color_mode)
	{
		color = (attr & 0xe000) >> 13;
		semigfx_tile = bool(BIT(attr, 12));
		// bit 7 is used by 2001spc and many others, no effect?
	}
	else
	{
		color = 7;
		semigfx_tile = bool(BIT(attr, 7));
	}

	lowerline = bool(BIT(attr, 5));
	// FIXME: ninjakmb (pc8801) sets 0x50 0x50 for all attribute strips
	// bit 6 is undefined in the specs, is it for masking decoration(s)?
	upperline = bool(BIT(attr, 4));
	reverse = bool(BIT(attr, 2));
	attr_blink = bool(BIT(attr, 1));
	secret = bool(BIT(attr, 0));

	if (semigfx_tile)
		tile = cc;
	else
	{
		if (lc > y_height - 1)
			tile = 0;
		else
			tile = m_cgrom->base()[(cc << 3) | (lc >> (y_double-1))];
	}

	// secret blacks out every tile connections,
	// has lower priority over blinking and other attribute decorations
	if (secret)
		tile = 0;

	if (csr)
		tile ^= 0xff;
	else if (attr_blink_on && attr_blink)
		tile = 0;

	// upper/lower line aren't affected by secret and blinking, only reverse
	// TODO: should downshift chars by one
	if (lc == 0 && upperline)
		tile = 0xff;

	if (is_lowestline && lowerline)
		tile = 0xff;

	if (reverse ^ m_screen_reverse)
		tile ^= 0xff;

//  if (m_width80)
	{
		u8 pen_dot;

		for (int xi = 0; xi < tile_width; xi += dot_width)
		{
			int res_x = (sx * 8) + xi;
			if (semigfx_tile)
			{
				u8 mask = (xi & (4 << (dot_width - 1))) ? 0x10 : 0x01;
				mask <<= (lc & (0x3 << y_double)) >> y_double;
				pen_dot = tile & mask;
			}
			else
			{
				pen_dot = tile;
				pen_dot = (pen_dot >> (7 - (xi >> (dot_width - 1)))) & 1;
			}

			if (!pen_dot)
				continue;

			for (int di = 0; di < dot_width; di++)
				bitmap.pix(y, res_x + di) = m_crtc_palette->pen(color);
		}
	}
}

uint32_t pc8001_state::screen_update( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	// TODO: superimposing
	// TODO: merging with previous frame for Color Magical (is it driver area?)
	m_crtc->screen_update(screen, bitmap, cliprect);
	return 0;
}

/* Read/Write Handlers */

/*
 * ---- ---x RTC C0
 * ---- --x- RTC C1
 * ---- -x-- RTC C2
 * ---- x--- RTC DATA IN
 */
void pc8001_base_state::port10_w(uint8_t data)
{
	// RTC
	m_rtc->c0_w(BIT(data, 0));
	m_rtc->c1_w(BIT(data, 1));
	m_rtc->c2_w(BIT(data, 2));
	m_rtc->data_in_w(BIT(data, 3));

	// centronics
	m_cent_data_out->write(data);
}

/*
 * I/O Port $30 (w/o) "System Control Port (1)"
 * N88-BASIC buffer port $e6c0
 *
 * Virtually same between PC-8001 and PC-8801
 *
 * --xx ---- BS2, BS1: USART channel control
 * --00 ----           CMT 600 bps
 * --01 ----           CMT 1200 bps
 * --10 ----           RS-232C async mode
 * --11 ----           RS-232C sync mode
 * ---- x--- MTON: CMT motor control (active high)
 * ---- -x-- CDS: CMT carrier control (1) mark (0) space
 * ---- --x- /COLOR: CRT display mode control (0) color mode (1) monochrome
 * ---- ---x /40: CRT display format control (1) 80 chars per line (0) 40 chars
 *
 */
void pc8001_base_state::port30_w(uint8_t data)
{
	m_width80 = BIT(data, 0);
	m_color = BIT(data, 1);

	m_cassette->change_state(BIT(data, 3) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
}

void pc8001mk2_state::port31_w(uint8_t data)
{
	/*

	    bit     description

	    0       expansion ROM (0=expansion, 1=N80)
	    1       memory mode (0=ROM, 1=RAM)
	    2       color mode (0=attribute, 1=B&W)
	    3       graphics enable
	    4       resolution (0=640x200, 1=320x200)
	    5       background color
	    6       background color
	    7       background color

	*/
	membank("bank2")->set_entry(data & 1);
}

void pc8001_base_state::write_centronics_busy(int state)
{
	m_centronics_busy = state;
}

void pc8001_base_state::write_centronics_ack(int state)
{
	m_centronics_ack = state;
}

uint8_t pc8001_state::port40_r()
{
	/*

	    bit     description

	    0       BUSY
	    1       ACK
	    2       CMT CDIN
	    3       EXP /EXTON
	    4       RTC DATA OUT
	    5       VRTC
	    6
	    7

	*/

	uint8_t data = 0x00;

	data |= m_centronics_busy;
	data |= m_centronics_ack << 1;
	data |= m_rtc->data_out_r() << 4;
	data |= m_crtc->vrtc_r() << 5;
	// TODO: enable line from pc80s31k (bit 3, active_low)

	return data;
}

/*
 * --x- ---- SPEAKER
 * ---- x--- CRT /CLDS CLK
 * ---- -x-- RTC CLK
 * ---- --x- RTC STB
 * ---- ---x Centronics STROBE
 */
void pc8001_state::port40_w(uint8_t data)
{
	m_centronics->write_strobe(BIT(data, 0));

	m_rtc->stb_w(BIT(data, 1));
	m_rtc->clk_w(BIT(data, 2));

	m_beep->set_state(BIT(data, 5));
}

/* Memory Maps */

void pc8001_state::pc8001_map(address_map &map)
{
	map(0x0000, 0x5fff).bankrw("bank1");
	map(0x6000, 0x7fff).bankrw("bank2");
	map(0x8000, 0xffff).bankrw("bank3");
}

void pc8001_state::pc8001_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x00, 0x00).portr("Y0");
	map(0x01, 0x01).portr("Y1");
	map(0x02, 0x02).portr("Y2");
	map(0x03, 0x03).portr("Y3");
	map(0x04, 0x04).portr("Y4");
	map(0x05, 0x05).portr("Y5");
	map(0x06, 0x06).portr("Y6");
	map(0x07, 0x07).portr("Y7");
	map(0x08, 0x08).portr("Y8");
	map(0x09, 0x09).portr("Y9");
	map(0x10, 0x10).mirror(0x0f).w(FUNC(pc8001_state::port10_w));
	map(0x20, 0x21).mirror(0x0e).rw(I8251_TAG, FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x30, 0x30).mirror(0x0f).w(FUNC(pc8001_state::port30_w));
	map(0x40, 0x40).mirror(0x0f).rw(FUNC(pc8001_state::port40_r), FUNC(pc8001_state::port40_w));
	map(0x50, 0x51).rw(m_crtc, FUNC(upd3301_device::read), FUNC(upd3301_device::write));
	map(0x60, 0x68).rw(m_dma, FUNC(i8257_device::read), FUNC(i8257_device::write));
//  map(0x70, 0x7f) unused
//  map(0x80, 0x80).mirror(0x0f).w(FUNC(pc8001_state::pc8011_ext0_w));
//  map(0x90, 0x90).mirror(0x0f).w(FUNC(pc8001_state::pc8011_ext1_w));
//  map(0xa0, 0xa0).mirror(0x0f).w(FUNC(pc8001_state::pc8011_ext2_w));
//  map(0xb0, 0xb0).r(FUNC(pc8001_state::pc8011_gpio8_r));
//  map(0xb1, 0xb1).w(FUNC(pc8001_state::pc8011_gpio8_w));
//  map(0xb2, 0xb2).r(FUNC(pc8001_state::pc8011_gpio4_r));
//  map(0xb3, 0xb3).w(FUNC(pc8001_state::pc8011_gpio4_w));
//  map(0xc0, 0xc0).rw(PC8011_CH1_I8251_TAG, FUNC(i8251_device::data_r), FUNC(i8251_device::data_w));
//  map(0xc1, 0xc1).rw(PC8011_CH1_I8251_TAG, FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
//  map(0xc2, 0xc2).rw(PC8011_CH2_I8251_TAG, FUNC(i8251_device::data_r), FUNC(i8251_device::data_w));
//  map(0xc3, 0xc3).rw(PC8011_CH2_I8251_TAG, FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
//  map(0xc8, 0xc8) RS-232 output enable?
//  map(0xca, 0xca) RS-232 output disable?
//  map(0xd0, 0xd3).rw(PC8011_IEEE488_I8255A_TAG, FUNC(i8255_device::read), FUNC(i8255_device::write));
//  map(0xd8, 0xd8).r(FUNC(pc8001_state::pc8011_ieee488_control_signal_input_r));
//  map(0xda, 0xda).r(FUNC(pc8001_state::pc8011_ieee488_bus_address_mode_r));
//  map(0xdc, 0xdc).w(FUNC(pc8001_state::pc8011_ieee488_nrfd_w));
//  map(0xde, 0xde).w(FUNC(pc8001_state::pc8011_ieee488_bus_mode_control_w));
//  map(0xe0, 0xe3).w(FUNC(pc8001_state::expansion_storage_mode_w));
//  map(0xe4, 0xe4).mirror(0x01).w(FUNC(pc8001_state::irq_level_w));
//  map(0xe6, 0xe6).w(FUNC(pc8001_state::irq_mask_w));
//  map(0xe7, 0xe7).w(FUNC(pc8001_state::pc8012_memory_mode_w));
//  map(0xe8, 0xfb) unused
	map(0xfc, 0xff).m(m_pc80s31, FUNC(pc80s31_device::host_map));
}

void pc8001mk2_state::pc8001mk2_map(address_map &map)
{
	map(0x0000, 0x5fff).bankrw("bank1");
	map(0x6000, 0x7fff).bankrw("bank2");
	map(0x8000, 0xbfff).bankrw("bank3");
	map(0xc000, 0xffff).bankrw("bank4");
}

void pc8001mk2_state::pc8001mk2_io(address_map &map)
{
	pc8001_io(map);
	map(0x30, 0x30).portr("DSW1").w(FUNC(pc8001mk2_state::port30_w));
	map(0x31, 0x31).portr("DSW2").w(FUNC(pc8001mk2_state::port31_w));
//  map(0x5c, 0x5c).w(FUNC(pc8001mk2_state::gram_on_w));
//  map(0x5f, 0x5f).w(FUNC(pc8001mk2_state::gram_off_w));
//  map(0xe8, 0xe8) kanji_address_lo_w, kanji_data_lo_r
//  map(0xe9, 0xe9) kanji_address_hi_w, kanji_data_hi_r
//  map(0xea, 0xea) kanji_readout_start_w
//  map(0xeb, 0xeb) kanji_readout_end_w
//  map(0xf3, 0xf3) DMA type disk unit interface selection port
//  map(0xf4, 0xf4) DMA type 8 inch control
//  map(0xf5, 0xf5) DMA type 8 inch margin control
//  map(0xf6, 0xf6) DMA type 8 inch FDC status
//  map(0xf7, 0xf7) DMA type 8 inch FDC data register
//  map(0xf8, 0xf8) DMA type 5 inch control
//  map(0xf9, 0xf9) DMA type 5 inch margin control
//  map(0xfa, 0xfa) DMA type 5 inch FDC status
//  map(0xfb, 0xfb) DMA type 5 inch FDC data register
}

void pc8001mk2sr_state::port33_w(u8 data)
{
	// TODO: needs progressive flush
#ifdef UNUSED_FUNCTION
	if (data & 0x80)
	{
		membank("bank1")->set_entry(2);
		membank("bank2")->set_entry(2 | (m_n80sr_bank & 1));
	}
	else
	{
		membank("bank1")->set_entry(0);
		membank("bank2")->set_entry(0);
	}
#endif
}

u8 pc8001mk2sr_state::port71_r()
{
	return m_n80sr_bank;
}

void pc8001mk2sr_state::port71_w(u8 data)
{
	m_n80sr_bank = data;
}

void pc8001mk2sr_state::pc8001mk2sr_io(address_map &map)
{
	pc8001mk2_io(map);
	map(0x33, 0x33).w(FUNC(pc8001mk2sr_state::port33_w));
	map(0x71, 0x71).rw(FUNC(pc8001mk2sr_state::port71_r), FUNC(pc8001mk2sr_state::port71_w));
}

/* Input Ports */

static INPUT_PORTS_START( pc8001 )
	PORT_START("Y0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0_PAD)      PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1_PAD)      PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2_PAD)      PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3_PAD)      PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4_PAD)      PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5_PAD)      PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6_PAD)      PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7_PAD)      PORT_CHAR(UCHAR_MAMEKEY(7_PAD))

	PORT_START("Y1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8_PAD)      PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9_PAD)      PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ASTERISK)   PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PLUS_PAD)   PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PGUP)       PORT_CHAR(UCHAR_MAMEKEY(EQUALS_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PGDN)       PORT_CHAR(UCHAR_MAMEKEY(COMMA_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL_PAD)    PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(13)

	PORT_START("Y2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('@')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A)          PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B)          PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C)          PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D)          PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E)          PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F)          PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G)          PORT_CHAR('g') PORT_CHAR('G')

	PORT_START("Y3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H)          PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I)          PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J)          PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K)          PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L)          PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M)          PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N)          PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O)          PORT_CHAR('o') PORT_CHAR('O')

	PORT_START("Y4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P)          PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)          PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)          PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S)          PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T)          PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U)          PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V)          PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)          PORT_CHAR('w') PORT_CHAR('W')

	PORT_START("Y5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X)          PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y)          PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z)          PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('[')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR(0xA5)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR(']')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('^')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-') PORT_CHAR('=')

	PORT_START("Y6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0)          PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3)          PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4)          PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5)          PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6)          PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7)          PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("Y7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8)          PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("  _") PORT_CODE(KEYCODE_DEL)           PORT_CHAR(0xff) PORT_CHAR('_')

	PORT_START("Y8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Clr Home") PORT_CODE(KEYCODE_HOME)     PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP)  PORT_CHAR(UCHAR_MAMEKEY(UP)) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT)    PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Del Ins") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(UCHAR_MAMEKEY(DEL)) PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Grph") PORT_CODE(KEYCODE_LALT) PORT_CODE(KEYCODE_RALT)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Kana") PORT_CODE(KEYCODE_LCONTROL) PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RCONTROL)                       PORT_CHAR(UCHAR_SHIFT_2)

	PORT_START("Y9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Stop") PORT_CODE(KEYCODE_TILDE)        PORT_CHAR(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1)                             PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2)                             PORT_CHAR(UCHAR_MAMEKEY(F2)) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F3)                             PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F4)                             PORT_CHAR(UCHAR_MAMEKEY(F4)) PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F5)                             PORT_CHAR(UCHAR_MAMEKEY(F5)) PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE)                          PORT_CHAR(' ')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ESC)                            PORT_CHAR(27)

//  PORT_START("DSW1")
INPUT_PORTS_END

static INPUT_PORTS_START( pc8001mk2 )
	PORT_INCLUDE( pc8001 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "Boot Mode" )
	PORT_DIPSETTING(    0x00, "N-BASIC" )
	PORT_DIPSETTING(    0x01, "N80-BASIC" )
	PORT_DIPNAME( 0x02, 0x02, "DSW1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "DSW2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( pc8001mk2sr )
	PORT_INCLUDE( pc8001mk2 )

	PORT_MODIFY("DSW1")
	// This is really a tri-state dip on front panel
	// BIOS just expects bit 1 to be off for SR mode
	PORT_DIPNAME( 0x03, 0x02, "Boot Mode" )
	PORT_DIPSETTING(    0x00, "N80SR-BASIC (duplicate)")
	PORT_DIPSETTING(    0x01, "N80SR-BASIC" )
	PORT_DIPSETTING(    0x02, "N-BASIC" )
	PORT_DIPSETTING(    0x03, "N80-BASIC" )
	PORT_DIPNAME( 0x04, 0x04, "DSW1" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

/* 8257 Interface */

void pc8001_base_state::hrq_w(int state)
{
	/* HACK - this should be connected to the BUSREQ line of Z80 */
	m_maincpu->set_input_line(INPUT_LINE_HALT, state);

	/* HACK - this should be connected to the BUSACK line of Z80 */
	m_dma->hlda_w(state);
}

uint8_t pc8001_base_state::dma_mem_r(offs_t offset)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	return program.read_byte(offset);
}

/* Machine Initialization */

void pc8001_base_state::machine_start()
{
	m_screen_reverse = false;

	/* initialize RTC */
	m_rtc->cs_w(1);
	m_rtc->oe_w(1);

	save_item(NAME(m_width80));
	save_item(NAME(m_color));
	save_item(NAME(m_screen_reverse));
	save_item(NAME(m_screen_is_24KHz));
}

void pc8001_state::machine_start()
{
	pc8001_base_state::machine_start();

	address_space &program = m_maincpu->space(AS_PROGRAM);

	/* initialize DMA */
	m_dma->ready_w(1);

	/* setup memory banking */
	uint8_t *ram = m_ram->pointer();

	membank("bank1")->configure_entry(1, m_rom->base());
	program.install_read_bank(0x0000, 0x5fff, membank("bank1"));
	program.unmap_write(0x0000, 0x5fff);
	membank("bank2")->configure_entry(1, m_rom->base() + 0x6000);

	switch (m_ram->size())
	{
	case 16*1024:
		membank("bank3")->configure_entry(0, ram);
		program.unmap_readwrite(0x8000, 0xbfff);
		program.install_readwrite_bank(0xc000, 0xffff, membank("bank3"));
		break;

	case 32*1024:
		membank("bank3")->configure_entry(0, ram);
		program.unmap_readwrite(0x8000, 0xbfff);
		program.install_readwrite_bank(0x8000, 0xffff, membank("bank3"));
		break;

	case 64*1024:
		membank("bank1")->configure_entry(0, ram);
		membank("bank2")->configure_entry(0, ram + 0x6000);
		membank("bank3")->configure_entry(0, ram + 0x8000);
		program.install_readwrite_bank(0x0000, 0x5fff, membank("bank1"));
		program.install_readwrite_bank(0x6000, 0xbfff, membank("bank2"));
		program.install_readwrite_bank(0x8000, 0xffff, membank("bank3"));
//      membank("bank2")->set_entry(0);
		break;
	}

	// PC8001 is 15KHz only
	set_screen_frequency(false);
}

void pc8001_state::machine_reset()
{
	membank("bank1")->set_entry(1);
	membank("bank2")->set_entry(1);
	membank("bank3")->set_entry(0);
}

void pc8001mk2sr_state::machine_start()
{
	pc8001_state::machine_start();

	membank("bank1")->configure_entry(2, m_n80sr_rom->base());
	membank("bank2")->configure_entry(2, m_n80sr_rom->base() + 0x6000);
	membank("bank2")->configure_entry(3, m_n80sr_rom->base() + 0x8000);

	save_item(NAME(m_n80sr_bank));
}

void pc8001mk2sr_state::machine_reset()
{
	pc8001_state::machine_reset();

	//membank("bank1")->set_entry(2);
	//membank("bank2")->set_entry(2);
}

/* Machine Drivers */

void pc8001_state::pc8001(machine_config &config)
{
	constexpr XTAL MASTER_CLOCK = XTAL(4'000'000);
	constexpr XTAL VIDEO_CLOCK = XTAL(14'318'181);

	/* basic machine hardware */
	Z80(config, m_maincpu, MASTER_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &pc8001_state::pc8001_map);
	m_maincpu->set_addrmap(AS_IO, &pc8001_state::pc8001_io);

	PC80S31(config, m_pc80s31, MASTER_CLOCK);
	config.set_perfect_quantum(m_maincpu);
	config.set_perfect_quantum("pc80s31:fdc_cpu");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(VIDEO_CLOCK, 896, 0, 640, 260, 0, 200);
	m_screen->set_screen_update(FUNC(pc8001_state::screen_update));
//  m_screen->set_palette(m_crtc_palette);

	PALETTE(config, m_crtc_palette, palette_device::BRG_3BIT);

	UPD3301(config, m_crtc, VIDEO_CLOCK);
	m_crtc->set_character_width(8);
	m_crtc->set_display_callback(FUNC(pc8001_state::draw_text));
	m_crtc->set_attribute_fetch_callback(FUNC(pc8001_state::attr_fetch));
	m_crtc->drq_wr_callback().set(m_dma, FUNC(i8257_device::dreq2_w));
	m_crtc->rvv_wr_callback().set(FUNC(pc8001_state::crtc_reverse_w));
	m_crtc->set_screen(m_screen);

	I8257(config, m_dma, MASTER_CLOCK);
	m_dma->out_hrq_cb().set(FUNC(pc8001_state::hrq_w));
	m_dma->in_memr_cb().set(FUNC(pc8001_state::dma_mem_r));
	m_dma->out_iow_cb<2>().set(m_crtc, FUNC(upd3301_device::dack_w));

	/* devices */
	I8251(config, I8251_TAG, 0);

	UPD1990A(config, m_rtc);

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->ack_handler().set(FUNC(pc8001_state::write_centronics_ack));
	m_centronics->busy_handler().set(FUNC(pc8001_state::write_centronics_busy));

	OUTPUT_LATCH(config, m_cent_data_out);
	m_centronics->set_output_latch(*m_cent_data_out);

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);

	RAM(config, RAM_TAG).set_default_size("16K").set_extra_options("32K,64K");

	SOFTWARE_LIST(config, "disk_n_list").set_original("pc8001_flop");

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	// TODO: unknown clock, is it really a beeper?
	BEEP(config, m_beep, 2400).add_route(ALL_OUTPUTS, "mono", 0.25);
}

void pc8001mk2_state::pc8001mk2(machine_config &config)
{
	pc8001(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &pc8001mk2_state::pc8001mk2_map);
	m_maincpu->set_addrmap(AS_IO, &pc8001mk2_state::pc8001mk2_io);

	// TODO: video HW has extra GVRAM setup

	RAM(config.replace(), RAM_TAG).set_default_size("64K");

	SOFTWARE_LIST(config, "disk_n80_list").set_original("pc8001mk2_flop");
}

void pc8001mk2sr_state::pc8001mk2sr(machine_config &config)
{
	pc8001mk2(config);
	m_maincpu->set_addrmap(AS_IO, &pc8001mk2sr_state::pc8001mk2sr_io);

	// TODO: mods for SR mode support

	SOFTWARE_LIST(config, "disk_n80sr_list").set_original("pc8001mk2sr_flop");
}

/* ROMs */

ROM_START( pc8001 )
	ROM_REGION( 0x8000, Z80_TAG, ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS("v110")
	// PCB pictures shows divided by 3 ROMs (and 4th socket unpopulated)
	// D2364C ROMs from a pc8001b PCB:
	// - p12019-106.ic10 072NBASIC
	// - p11219-105.ic11 073NBASIC
	// - p12029-106.ic12 171NBASIC
	ROM_SYSTEM_BIOS( 0, "v101", "N-BASIC v1.01" )
	ROMX_LOAD( "n80v101.rom", 0x00000, 0x6000, BAD_DUMP CRC(a2cc9f22) SHA1(6d2d838de7fea20ddf6601660d0525d5b17bf8a3), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v102", "N-BASIC v1.02" )
	ROMX_LOAD( "n80v102.rom", 0x00000, 0x6000, BAD_DUMP CRC(ed01ca3f) SHA1(b34a98941499d5baf79e7c0e5578b81dbede4a58), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "v110", "N-BASIC v1.10" )
	ROMX_LOAD( "n80v110.rom", 0x00000, 0x6000, BAD_DUMP CRC(1e02d93f) SHA1(4603cdb7a3833e7feb257b29d8052c872369e713), ROM_BIOS(2) )
	// empty socket, cfr. notes in header for usage instructions
	ROM_LOAD_OPTIONAL( "exprom.ic13", 0x6000, 0x2000, NO_DUMP )

	ROM_REGION( 0x800, CGROM_TAG, 0)
	ROM_LOAD( "font.rom", 0x000, 0x800, CRC(56653188) SHA1(84b90f69671d4b72e8f219e1fe7cd667e976cf7f) )
ROM_END

ROM_START( pc8001mk2 )
	ROM_REGION( 0x8000, Z80_TAG, 0 )
	// N-BASIC v1.3
	// N80-BASIC v1.0
	ROM_LOAD( "n80_2.rom", 0x0000, 0x8000, CRC(03cce7b6) SHA1(c12d34e42021110930fed45a8af98db52136f1fb) )

	ROM_REGION( 0x800, CGROM_TAG, 0)
	ROM_LOAD( "font.rom", 0x0000, 0x0800, CRC(56653188) SHA1(84b90f69671d4b72e8f219e1fe7cd667e976cf7f) )

	ROM_REGION( 0x20000, "kanji", 0)
	ROM_LOAD( "kanji1.rom", 0x00000, 0x20000, CRC(6178bd43) SHA1(82e11a177af6a5091dd67f50a2f4bafda84d6556) )
ROM_END

ROM_START( pc8001mk2sr )
	ROM_REGION( 0x8000, Z80_TAG, 0 )
	// N-BASIC v1.6
	// N80-BASIC v1.2
	ROM_LOAD( "n80_2sr.rom", 0x0000, 0x8000, CRC(dcb71282) SHA1(e8db5dc5eae11da14e48656d324874e59f2e3844) )

	ROM_REGION (0x10000, N80SR_ROM_TAG, ROMREGION_ERASEFF )
	// N80SR-BASIC v1.0
	ROM_LOAD( "n80_3.rom",    0x0000, 0xa000, BAD_DUMP CRC(d99ef247) SHA1(9bfa5009d703cd31caa734d932d2a847d74cbfa6) )
	// TODO: empty socket at ic77

	ROM_REGION( 0x2000, CGROM_TAG, 0)
	ROM_LOAD( "font80sr.rom", 0x000000, 0x001000, CRC(784c0b17) SHA1(565dc8e5e46b1633cb434d12b4d8b3a662546b33) )
	ROM_LOAD( "fonthira.rom", 0x001000, 0x000800, CRC(fe7059d5) SHA1(10c5f85adcce540cbd0a11352e2c38a84c989a26) )
	ROM_LOAD( "fontkata.rom", 0x001800, 0x000800, CRC(56653188) SHA1(84b90f69671d4b72e8f219e1fe7cd667e976cf7f) )

	ROM_REGION( 0x20000, "kanji", 0)
	ROM_LOAD( "kanji1.rom", 0x00000, 0x20000, CRC(6178bd43) SHA1(82e11a177af6a5091dd67f50a2f4bafda84d6556) )
ROM_END

/* System Drivers */

//    YEAR  NAME       PARENT  COMPAT  MACHINE    INPUT   CLASS            INIT        COMPANY  FULLNAME       FLAGS
// 1978?, pc8001g, Wirewrapped prototype version
COMP( 1979, pc8001,      0,      0,      pc8001,      pc8001,      pc8001_state,      empty_init, "NEC",   "PC-8001",     MACHINE_NOT_WORKING )
// 1981 pc8001a, US version of PC-8001 with Greek alphabet instead of Kana
COMP( 1983, pc8001mk2,   pc8001, 0,      pc8001mk2,   pc8001mk2,   pc8001mk2_state,   empty_init, "NEC",   "PC-8001mkII", MACHINE_NOT_WORKING )
COMP( 1985, pc8001mk2sr, pc8001, 0,      pc8001mk2sr, pc8001mk2sr, pc8001mk2sr_state, empty_init, "NEC",   "PC-8001mkIISR", MACHINE_NOT_WORKING )
