// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

    http://www2.odn.ne.jp/~haf09260/Pc80/EnrPc.htm
    http://home1.catvmics.ne.jp/~kanemoto/n80/inside.html
    http://www.geocities.jp/retro_zzz/machines/nec/8001/index.html

*/

/*

    TODO:

    - uPD3301 attributes
    - PCG1000
    - Intel 8251
    - cassette
    - floppy
    - PC-8011
    - PC-8021
    - PC-8031

*/

#include "emu.h"
#include "includes/pc8001.h"
#include "screen.h"
#include "speaker.h"

/* Read/Write Handlers */

WRITE8_MEMBER( pc8001_state::port10_w )
{
	/*

	    bit     description

	    0       RTC C0
	    1       RTC C1
	    2       RTC C2
	    3       RTC DATA IN
	    4
	    5
	    6
	    7

	*/

	// RTC
	m_rtc->c0_w(BIT(data, 0));
	m_rtc->c1_w(BIT(data, 1));
	m_rtc->c2_w(BIT(data, 2));
	m_rtc->data_in_w(BIT(data, 3));

	// centronics
	m_cent_data_out->write(data);
}

WRITE8_MEMBER( pc8001_state::port30_w )
{
	/*

	    bit     description

	    0       characters per line (0=40, 1=80)
	    1       color mode (0=color, 1=B&W)
	    2       CMT CHIN
	    3       CMT MOTOR (1=on)
	    4       CMT BS1
	    5       CMT BS2
	    6       unused
	    7       unused

	*/

	/* characters per line */
	m_width80 = BIT(data, 0);

	/* color mode */
	m_color = BIT(data, 1);

	/* cassette motor */
	m_cassette->change_state(BIT(data,3) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
}

WRITE8_MEMBER( pc8001mk2_state::port31_w )
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
}

WRITE_LINE_MEMBER( pc8001_state::write_centronics_busy )
{
	m_centronics_busy = state;
}

WRITE_LINE_MEMBER( pc8001_state::write_centronics_ack )
{
	m_centronics_ack = state;
}

READ8_MEMBER( pc8001_state::port40_r )
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

	uint8_t data = 0x08;

	data |= m_centronics_busy;
	data |= m_centronics_ack << 1;
	data |= m_rtc->data_out_r() << 4;
	data |= m_crtc->vrtc_r() << 5;

	return data;
}

WRITE8_MEMBER( pc8001_state::port40_w )
{
	/*

	    bit     description

	    0       STROBE
	    1       RTC STB
	    2       RTC CLK
	    3       CRT /CLDS CLK
	    4
	    5       SPEAKER
	    6
	    7

	*/

	m_centronics->write_strobe(BIT(data, 0));

	m_rtc->clk_w(BIT(data, 2));
	m_rtc->stb_w(BIT(data, 1));

	m_beep->set_state(BIT(data, 5));
}

/* Memory Maps */

void pc8001_state::pc8001_mem(address_map &map)
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
//  AM_RANGE(0x70, 0x7f) unused
//  AM_RANGE(0x80, 0x80) AM_MIRROR(0x0f) AM_WRITE(pc8011_ext0_w)
//  AM_RANGE(0x90, 0x90) AM_MIRROR(0x0f) AM_WRITE(pc8011_ext1_w)
//  AM_RANGE(0xa0, 0xa0) AM_MIRROR(0x0f) AM_WRITE(pc8011_ext2_w)
//  AM_RANGE(0xb0, 0xb0) AM_READ(pc8011_gpio8_r)
//  AM_RANGE(0xb1, 0xb1) AM_WRITE(pc8011_gpio8_w)
//  AM_RANGE(0xb2, 0xb2) AM_READ(pc8011_gpio4_r)
//  AM_RANGE(0xb3, 0xb3) AM_WRITE(pc8011_gpio4_w)
//  AM_RANGE(0xc0, 0xc0) AM_DEVREADWRITE(PC8011_CH1_I8251_TAG, i8251_device, data_r, data_w)
//  AM_RANGE(0xc1, 0xc1) AM_DEVREADWRITE(PC8011_CH1_I8251_TAG, i8251_device, status_r, control_w)
//  AM_RANGE(0xc2, 0xc2) AM_DEVREADWRITE(PC8011_CH2_I8251_TAG, i8251_device, data_r, data_w)
//  AM_RANGE(0xc3, 0xc3) AM_DEVREADWRITE(PC8011_CH2_I8251_TAG, i8251_device, status_r, control_w)
//  AM_RANGE(0xc8, 0xc8) RS-232 output enable?
//  AM_RANGE(0xca, 0xca) RS-232 output disable?
//  AM_RANGE(0xd0, 0xd3) AM_DEVREADWRITE(PC8011_IEEE488_I8255A_TAG, i8255_device, read, write)
//  AM_RANGE(0xd8, 0xd8) AM_READ(pc8011_ieee488_control_signal_input_r)
//  AM_RANGE(0xda, 0xda) AM_READ(pc8011_ieee488_bus_address_mode_r)
//  AM_RANGE(0xdc, 0xdc) AM_WRITE(pc8011_ieee488_nrfd_w)
//  AM_RANGE(0xde, 0xde) AM_WRITE(pc8011_ieee488_bus_mode_control_w)
//  AM_RANGE(0xe0, 0xe3) AM_WRITE(expansion_storage_mode_w)
//  AM_RANGE(0xe4, 0xe4) AM_MIRROR(0x01) AM_WRITE(irq_level_w)
//  AM_RANGE(0xe6, 0xe6) AM_WRITE(irq_mask_w)
//  AM_RANGE(0xe7, 0xe7) AM_WRITE(pc8012_memory_mode_w)
//  AM_RANGE(0xe8, 0xfb) unused
	map(0xfc, 0xff).rw(I8255A_TAG, FUNC(i8255_device::read), FUNC(i8255_device::write));
}

void pc8001mk2_state::pc8001mk2_mem(address_map &map)
{
	map(0x0000, 0x5fff).bankrw("bank1");
	map(0x6000, 0x7fff).bankrw("bank2");
	map(0x8000, 0xbfff).bankrw("bank3");
	map(0xc000, 0xffff).bankrw("bank4");
}

void pc8001mk2_state::pc8001mk2_io(address_map &map)
{
	pc8001_io(map);
	map(0x30, 0x30).w(FUNC(pc8001mk2_state::port30_w));
	map(0x31, 0x31).w(FUNC(pc8001mk2_state::port31_w));
//  AM_RANGE(0x5c, 0x5c) AM_WRITE(gram_on_w)
//  AM_RANGE(0x5f, 0x5f) AM_WRITE(gram_off_w)
//  AM_RANGE(0xe8, 0xe8) kanji_address_lo_w, kanji_data_lo_r
//  AM_RANGE(0xe9, 0xe9) kanji_address_hi_w, kanji_data_hi_r
//  AM_RANGE(0xea, 0xea) kanji_readout_start_w
//  AM_RANGE(0xeb, 0xeb) kanji_readout_end_w
//  AM_RANGE(0xf3, 0xf3) DMA type disk unit interface selection port
//  AM_RANGE(0xf4, 0xf4) DMA type 8 inch control
//  AM_RANGE(0xf5, 0xf5) DMA type 8 inch margin control
//  AM_RANGE(0xf6, 0xf6) DMA type 8 inch FDC status
//  AM_RANGE(0xf7, 0xf7) DMA type 8 inch FDC data register
//  AM_RANGE(0xf8, 0xf8) DMA type 5 inch control
//  AM_RANGE(0xf9, 0xf9) DMA type 5 inch margin control
//  AM_RANGE(0xfa, 0xfa) DMA type 5 inch FDC status
//  AM_RANGE(0xfb, 0xfb) DMA type 5 inch FDC data register
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
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR(0xA5) PORT_CHAR('|')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR(']') PORT_CHAR('}')
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
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("  _") PORT_CODE(KEYCODE_DEL)           PORT_CHAR(0) PORT_CHAR('_')

	PORT_START("Y8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Clr Home") PORT_CODE(KEYCODE_HOME)     PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP)  PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT)    PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Del Ins") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(UCHAR_MAMEKEY(DEL)) PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Grph") PORT_CODE(KEYCODE_LALT) PORT_CODE(KEYCODE_RALT) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Kana") PORT_CODE(KEYCODE_LCONTROL) PORT_TOGGLE PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RCONTROL)                       PORT_CHAR(UCHAR_SHIFT_2)

	PORT_START("Y9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Stop") PORT_CODE(KEYCODE_ESC)          PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1)                             PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2)                             PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F3)                             PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F4)                             PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F5)                             PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE)                          PORT_CHAR(' ')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ESC)                            PORT_CHAR(UCHAR_MAMEKEY(ESC))

	PORT_START("DSW1")
INPUT_PORTS_END

/* uPD3301 Interface */

static const rgb_t PALETTE_PC8001[] =
{
	rgb_t::black(),
	rgb_t(0x00, 0x00, 0xff),
	rgb_t(0xff, 0x00, 0x00),
	rgb_t(0xff, 0x00, 0xff),
	rgb_t(0x00, 0xff, 0x00),
	rgb_t(0x00, 0xff, 0xff),
	rgb_t(0xff, 0xff, 0x00),
	rgb_t::white()
};

UPD3301_DRAW_CHARACTER_MEMBER( pc8001_state::pc8001_display_pixels )
{
	uint8_t data = m_char_rom->base()[(cc << 3) | lc];
	int i;

	if (lc >= 8) return;
	if (csr) data = 0xff;

	if (m_width80)
	{
		for (i = 0; i < 8; i++)
		{
			int color = BIT(data, 7) ^ rvv;

			bitmap.pix32(y, (sx * 8) + i) = PALETTE_PC8001[color ? 7 : 0];

			data <<= 1;
		}
	}
	else
	{
		if (sx % 2) return;

		for (i = 0; i < 8; i++)
		{
			int color = BIT(data, 7) ^ rvv;

			bitmap.pix32(y, (sx/2 * 16) + (i * 2)) = PALETTE_PC8001[color ? 7 : 0];
			bitmap.pix32(y, (sx/2 * 16) + (i * 2) + 1) = PALETTE_PC8001[color ? 7 : 0];

			data <<= 1;
		}
	}
}

/* 8257 Interface */

WRITE_LINE_MEMBER( pc8001_state::hrq_w )
{
	/* HACK - this should be connected to the BUSREQ line of Z80 */
	m_maincpu->set_input_line(INPUT_LINE_HALT, state);

	/* HACK - this should be connected to the BUSACK line of Z80 */
	m_dma->hlda_w(state);
}

READ8_MEMBER( pc8001_state::dma_mem_r )
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	return program.read_byte(offset);
}

/* Machine Initialization */

void pc8001_state::machine_start()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	/* initialize RTC */
	m_rtc->cs_w(1);
	m_rtc->oe_w(1);

	/* initialize DMA */
	m_dma->ready_w(1);

	/* setup memory banking */
	uint8_t *ram = m_ram->pointer();

	membank("bank1")->configure_entry(1, m_rom->base());
	program.install_read_bank(0x0000, 0x5fff, "bank1");
	program.unmap_write(0x0000, 0x5fff);

	switch (m_ram->size())
	{
	case 16*1024:
		membank("bank3")->configure_entry(0, ram);
		program.unmap_readwrite(0x6000, 0xbfff);
		program.unmap_readwrite(0x8000, 0xbfff);
		program.install_readwrite_bank(0xc000, 0xffff, "bank3");
		break;

	case 32*1024:
		membank("bank3")->configure_entry(0, ram);
		program.unmap_readwrite(0x6000, 0xbfff);
		program.install_readwrite_bank(0x8000, 0xffff, "bank3");
		break;

	case 64*1024:
		membank("bank1")->configure_entry(0, ram);
		membank("bank2")->configure_entry(0, ram + 0x6000);
		membank("bank3")->configure_entry(0, ram + 0x8000);
		program.install_readwrite_bank(0x0000, 0x5fff, "bank1");
		program.install_readwrite_bank(0x6000, 0xbfff, "bank2");
		program.install_readwrite_bank(0x8000, 0xffff, "bank3");
		membank("bank2")->set_entry(0);
		break;
	}

	membank("bank1")->set_entry(1);
	membank("bank3")->set_entry(0);

	/* register for state saving */
	save_item(NAME(m_width80));
	save_item(NAME(m_color));
}

/* Machine Drivers */

void pc8001_state::pc8001(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(4'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &pc8001_state::pc8001_mem);
	m_maincpu->set_addrmap(AS_IO, &pc8001_state::pc8001_io);

	/* video hardware */
	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_screen_update(UPD3301_TAG, FUNC(upd3301_device::screen_update));
	screen.set_size(640, 220);
	screen.set_visarea(0, 640-1, 0, 200-1);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beep, 2000).add_route(ALL_OUTPUTS, "mono", 0.25);

	/* devices */
	I8251(config, I8251_TAG, 0);

	I8255A(config, I8255A_TAG, 0);

	I8257(config, m_dma, XTAL(4'000'000));
	m_dma->out_hrq_cb().set(FUNC(pc8001_state::hrq_w));
	m_dma->in_memr_cb().set(FUNC(pc8001_state::dma_mem_r));
	m_dma->out_iow_cb<2>().set(m_crtc, FUNC(upd3301_device::dack_w));

	UPD1990A(config, m_rtc);

	UPD3301(config, m_crtc, XTAL(14'318'181));
	m_crtc->set_character_width(8);
	m_crtc->set_display_callback(FUNC(pc8001_state::pc8001_display_pixels), this);
	m_crtc->drq_wr_callback().set(m_dma, FUNC(i8257_device::dreq2_w));
	m_crtc->set_screen(SCREEN_TAG);

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->ack_handler().set(FUNC(pc8001_state::write_centronics_ack));
	m_centronics->busy_handler().set(FUNC(pc8001_state::write_centronics_busy));

	OUTPUT_LATCH(config, m_cent_data_out);
	m_centronics->set_output_latch(*m_cent_data_out);

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);

	RAM(config, RAM_TAG).set_default_size("16K").set_extra_options("32K,64K");
}

void pc8001mk2_state::pc8001mk2(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(4'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &pc8001mk2_state::pc8001mk2_mem);
	m_maincpu->set_addrmap(AS_IO, &pc8001mk2_state::pc8001mk2_io);

	/* video hardware */
	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_screen_update(UPD3301_TAG, FUNC(upd3301_device::screen_update));
	screen.set_size(640, 220);
	screen.set_visarea(0, 640-1, 0, 200-1);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beep, 2000).add_route(ALL_OUTPUTS, "mono", 0.25);

	/* devices */
	I8251(config, I8251_TAG, 0);

	I8255A(config, I8255A_TAG, 0);

	I8257(config, m_dma, XTAL(4'000'000));
	m_dma->out_hrq_cb().set(FUNC(pc8001_state::hrq_w));
	m_dma->in_memr_cb().set(FUNC(pc8001_state::dma_mem_r));
	m_dma->out_iow_cb<2>().set(m_crtc, FUNC(upd3301_device::dack_w));

	UPD1990A(config, m_rtc);

	UPD3301(config, m_crtc, XTAL(14'318'181));
	m_crtc->set_character_width(8);
	m_crtc->set_display_callback(FUNC(pc8001_state::pc8001_display_pixels), this);
	m_crtc->drq_wr_callback().set(m_dma, FUNC(i8257_device::dreq2_w));
	m_crtc->set_screen(SCREEN_TAG);

	CENTRONICS(config, m_centronics, centronics_devices, "printer");

	OUTPUT_LATCH(config, m_cent_data_out);
	m_centronics->set_output_latch(*m_cent_data_out);

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);

	RAM(config, RAM_TAG).set_default_size("64K");
}

/* ROMs */

ROM_START( pc8001 )
	ROM_REGION( 0x6000, Z80_TAG, 0 )
	ROM_SYSTEM_BIOS( 0, "v101", "N-BASIC v1.01" )
	ROMX_LOAD( "n80v101.rom", 0x00000, 0x6000, CRC(a2cc9f22) SHA1(6d2d838de7fea20ddf6601660d0525d5b17bf8a3), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v102", "N-BASIC v1.02" )
	ROMX_LOAD( "n80v102.rom", 0x00000, 0x6000, CRC(ed01ca3f) SHA1(b34a98941499d5baf79e7c0e5578b81dbede4a58), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "v110", "N-BASIC v1.10" )
	ROMX_LOAD( "n80v110.rom", 0x00000, 0x6000, CRC(1e02d93f) SHA1(4603cdb7a3833e7feb257b29d8052c872369e713), ROM_BIOS(2) )

	ROM_REGION( 0x800, UPD3301_TAG, 0)
	ROM_LOAD( "font.rom", 0x000, 0x800, CRC(56653188) SHA1(84b90f69671d4b72e8f219e1fe7cd667e976cf7f) )
ROM_END

ROM_START( pc8001mk2 )
	ROM_REGION( 0x8000, Z80_TAG, 0 )
	ROM_LOAD( "n80_2.rom", 0x00000, 0x8000, CRC(03cce7b6) SHA1(c12d34e42021110930fed45a8af98db52136f1fb) )

	ROM_REGION( 0x800, UPD3301_TAG, 0)
	ROM_LOAD( "font.rom", 0x0000, 0x0800, CRC(56653188) SHA1(84b90f69671d4b72e8f219e1fe7cd667e976cf7f) )

	ROM_REGION( 0x20000, "kanji", 0)
	ROM_LOAD( "kanji1.rom", 0x00000, 0x20000, CRC(6178bd43) SHA1(82e11a177af6a5091dd67f50a2f4bafda84d6556) )
ROM_END

/* System Drivers */

//    YEAR  NAME       PARENT  COMPAT  MACHINE    INPUT   CLASS            INIT        COMPANY  FULLNAME       FLAGS
COMP( 1979, pc8001,    0,      0,      pc8001,    pc8001, pc8001_state,    empty_init, "NEC",   "PC-8001",     MACHINE_NOT_WORKING )
COMP( 1983, pc8001mk2, pc8001, 0,      pc8001mk2, pc8001, pc8001mk2_state, empty_init, "NEC",   "PC-8001mkII", MACHINE_NOT_WORKING )
