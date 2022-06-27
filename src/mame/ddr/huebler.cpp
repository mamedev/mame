// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    Ausbaufaehiger Mikrocomputer mit dem U 880
    electronica Band 227/228

****************************************************************************/

/*

TODO:

    - keyboard repeat
    - shift lock is not PORT_TOGGLE, instead RS flipflop
    - tone generator
    - cassette serial clock
    - eprom programmer
    - power off

Cassette considerations

    - It operates at 4883 baud but may not be fully compatible with real
      hardware. A partial translation of the available text seems to
      indicate an overcomplicated and unreliable interface. The one
      implemented is 100% reliable at the same speed. I've been unable
      to locate any real recordings to compare against.
    - To save a range of memory: S name start end
    - To load it back: L name
    - To save an unnamed file: S "" start end
    - To load an unnamed file: L
    - When loading, if the name doesn't match, there's no message, and no
      way to determine the name.
    - For some reason, the system saves the file 3 times. Maybe it was an
      attempt to guard against the inherent unreliable design.
*/

#include "emu.h"
#include "huebler.h"
#include "speaker.h"
#include "screen.h"

/* Keyboard */

void amu880_state::scan_keyboard()
{
	uint8_t data = m_key_row[m_key_a8 ? m_key_d6 : m_key_d7]->read();

	int a8 = (data & 0x0f) == 0x0f;

	if (m_key_a8 && !a8)
	{
		m_key_d7 = m_key_d6;
		m_key_a4 = !(BIT(data, 1) && BIT(data, 3));
		m_key_a5 = !(BIT(data, 2) && BIT(data, 3));
	}

	m_key_a8 = a8;

	m_key_d6++;

	if (m_key_d6 == 16)
	{
		m_key_d6 = 0;
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(amu880_state::keyboard_tick)
{
	scan_keyboard();
}

/* Read/Write Handlers */

uint8_t amu880_state::keyboard_r(offs_t offset)
{
	/*

	    A0..A3  Y
	    A4      !(X0&X3)
	    A5      !(X2&X3)
	    A6      shift
	    A7      ctrl
	    A8      key pressed
	    A9      AB0

	*/

	uint8_t special = m_special->read();

	int ctrl = BIT(special, 0);
	int shift = BIT(special, 2) & BIT(special, 1);
	int ab0 = BIT(offset, 0);

	uint16_t address = (ab0 << 9) | (m_key_a8 << 8) | (ctrl << 7) | (shift << 6) | (m_key_a5 << 5) | (m_key_a4 << 4) | m_key_d7;

	return m_kb_rom->base()[address];
}

/* Memory Maps */

void amu880_state::amu880_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xe7ff).ram();
	map(0xe800, 0xefff).ram().share("video_ram");
	map(0xf000, 0xfbff).rom();
	map(0xfc00, 0xffff).ram();
}

void amu880_state::amu880_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
//  map(0x00, 0x00).mirror(0x03).w(FUNC(amu880_state::power_off_w));
//  map(0x04, 0x04).mirror(0x02).w(FUNC(amu880_state::tone_off_w));
//  map(0x05, 0x05).mirror(0x02).w(FUNC(amu880_state::tone_on_w));
	map(0x08, 0x09).mirror(0x02).r(FUNC(amu880_state::keyboard_r));
	map(0x0c, 0x0f).rw(Z80PIO2_TAG, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0x10, 0x13).rw(Z80PIO1_TAG, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0x14, 0x17).rw(Z80CTC_TAG, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x18, 0x1b).rw(m_sio, FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
}

/* Input Ports */

static INPUT_PORTS_START( amu880 )
	PORT_START("Y0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("Y1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CLR") PORT_CODE(KEYCODE_END)

	PORT_START("Y2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))

	PORT_START("Y3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('?')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))

	PORT_START("Y4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))

	PORT_START("Y5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))

	PORT_START("Y6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR('\r')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) // <-|

	PORT_START("Y7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("HOME") PORT_CODE(KEYCODE_HOME)

	PORT_START("Y8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')

	PORT_START("Y9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y10")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CHAR('@') PORT_CHAR('\\')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y11")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y12")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CHAR('_') PORT_CHAR('|')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y13")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y14")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y15")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SPECIAL")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
INPUT_PORTS_END

/* Video */

uint32_t amu880_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	pen_t const *const pen = m_palette->pens();

	for (int y = 0; y < 240; y++)
	{
		int const line = y % 10;

		for (int sx = 0; sx < 64; sx++)
		{
			uint16_t const videoram_addr = ((y / 10) * 64) + sx;
			uint8_t const videoram_data = m_video_ram[videoram_addr & 0x7ff];

			uint16_t const charrom_addr = ((videoram_data & 0x7f) << 3) | line;
			uint8_t data = m_char_rom->base()[charrom_addr & 0x3ff];

			for (int x = 0; x < 6; x++)
			{
				int const color = ((line > 7) ? 0 : BIT(data, 7)) ^ BIT(videoram_data, 7);

				bitmap.pix(y, (sx * 6) + x) = pen[color];

				data <<= 1;
			}
		}
	}

	return 0;
}

/* Z80-CTC Interface */

WRITE_LINE_MEMBER(amu880_state::ctc_z0_w)
{
}

WRITE_LINE_MEMBER(amu880_state::ctc_z2_w)
{
	/* cassette clock @ 39kHz */
	if (state)
	{
		m_cnt++;
		switch (m_cnt & 7) // divide by 8 to get 4883Hz
		{
			case 0:
				m_cassette->output(m_cassbit ? 1.0 : -1.0);
				break;
			case 2:
				m_sio->txca_w(0);
				m_sio->rxca_w(0);
				break;
			case 4:
				m_sio->txca_w(1);
				m_sio->rxca_w(1);
				break;
			case 6:
				m_sio->rxa_w((m_cassette->input() > 0.04) ? 1 : 0);
				break;
			default:
				break;
		}
	}
}

/* Z80-SIO Interface */

WRITE_LINE_MEMBER(amu880_state::cassette_w)
{
	m_cassbit = state;
}

/* Z80 Daisy Chain */

static const z80_daisy_config amu880_daisy_chain[] =
{
	{ Z80CTC_TAG },
	{ Z80SIO_TAG },
	{ Z80PIO1_TAG },
	{ Z80PIO2_TAG },
	{ nullptr }
};

/* Machine Initialization */

void amu880_state::machine_start()
{
	/* register for state saving */
	save_item(NAME(m_key_d6));
	save_item(NAME(m_key_d7));
	save_item(NAME(m_key_a4));
	save_item(NAME(m_key_a5));
	save_item(NAME(m_key_a8));
}

/* Machine Driver */

/* F4 Character Displayer */
static const gfx_layout amu880_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	128,                    /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 8 bytes */
};

static GFXDECODE_START( gfx_amu880 )
	GFXDECODE_ENTRY( "chargen", 0x0000, amu880_charlayout, 0, 1 )
GFXDECODE_END


void amu880_state::amu880(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(10'000'000)/4); // U880D
	m_maincpu->set_addrmap(AS_PROGRAM, &amu880_state::amu880_mem);
	m_maincpu->set_addrmap(AS_IO, &amu880_state::amu880_io);
	m_maincpu->set_daisy_config(amu880_daisy_chain);

	TIMER(config, "keyboard").configure_periodic(FUNC(amu880_state::keyboard_tick), attotime::from_hz(1500));

	/* video hardware */
	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(amu880_state::screen_update));
	screen.set_raw(9000000, 576, 0*6, 64*6, 320, 0*10, 24*10);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_amu880);
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	/* devices */
	z80ctc_device& ctc(Z80CTC(config, Z80CTC_TAG, XTAL(10'000'000)/4));
	ctc.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	ctc.zc_callback<0>().set(FUNC(amu880_state::ctc_z0_w));
	ctc.zc_callback<1>().set(m_sio, FUNC(z80sio_device::rxtxcb_w));
	ctc.zc_callback<2>().set(FUNC(amu880_state::ctc_z2_w));

	z80pio_device& pio1(Z80PIO(config, Z80PIO1_TAG, XTAL(10'000'000)/4));
	pio1.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	z80pio_device& pio2(Z80PIO(config, Z80PIO2_TAG, XTAL(10'000'000)/4));
	pio2.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	Z80SIO(config, m_sio, XTAL(10'000'000)/4); // U856
	m_sio->out_txda_callback().set(FUNC(amu880_state::cassette_w));
	m_sio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("64K");
}

/* ROMs */

ROM_START( amu880 )
	ROM_REGION( 0x10000, Z80_TAG, 0 )
	ROM_DEFAULT_BIOS( "v21" )
	ROM_SYSTEM_BIOS( 0, "v21", "H.MON v2.1" )
	ROMX_LOAD( "mon21a.bin", 0xf000, 0x0400, NO_DUMP, ROM_BIOS(0) )
	ROMX_LOAD( "mon21b.bin", 0xf400, 0x0400, NO_DUMP, ROM_BIOS(0) )
	ROMX_LOAD( "mon21c.bin", 0xf800, 0x0400, NO_DUMP, ROM_BIOS(0) )
	ROMX_LOAD( "mon21.bin", 0xf000, 0x0bdf, BAD_DUMP CRC(ba905563) SHA1(1fa0aeab5428731756bdfa74efa3c664898bf083), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v30", "H.MON v3.0" )
	ROMX_LOAD( "mon30.bin", 0xf000, 0x1000, CRC(033f8112) SHA1(0c6ae7b9d310dec093652db6e8ae84f8ebfdcd29), ROM_BIOS(1) )

	ROM_REGION( 0x4800, "hbasic", 0 )
	ROM_LOAD( "mon30p_hbasic33p.bin", 0x0000, 0x4800, CRC(c927e7be) SHA1(2d1f3ff4d882c40438a1281872c6037b2f07fdf2) )

	ROM_REGION( 0x400, "chargen", 0 )
	ROM_LOAD( "hemcfont.bin", 0x0000, 0x0400, CRC(1074d103) SHA1(e558279cff5744acef4eccf30759a9508b7f8750) )

	ROM_REGION( 0x400, "keyboard", 0 )
	ROM_LOAD( "keyboard.bin", 0x0000, 0x0400, BAD_DUMP CRC(daa06361) SHA1(b0299bd8d1686e05dbeeaed54f6c41ae543be20c) ) // typed in from manual
ROM_END

/* System Drivers */

/*    YEAR  NAME    PARENT  COMPAT  MACHINE INPUT   CLASS         INIT        COMPANY                   FULLNAME                                      FLAGS */
COMP( 1983, amu880, 0,      0,      amu880, amu880, amu880_state, empty_init, "Militaerverlag der DDR", "Ausbaufaehiger Mikrocomputer mit dem U 880", MACHINE_NO_SOUND )
