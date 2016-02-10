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

*/

#include "includes/huebler.h"

/* Keyboard */

void amu880_state::scan_keyboard()
{
	UINT8 data = m_key_row[m_key_a8 ? m_key_d6 : m_key_d7]->read();

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

READ8_MEMBER( amu880_state::keyboard_r )
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

	UINT8 special = m_special->read();

	int ctrl = BIT(special, 0);
	int shift = BIT(special, 2) & BIT(special, 1);
	int ab0 = BIT(offset, 0);

	UINT16 address = (ab0 << 9) | (m_key_a8 << 8) | (ctrl << 7) | (shift << 6) | (m_key_a5 << 5) | (m_key_a4 << 4) | m_key_d7;

	return m_kb_rom->base()[address];
}

/* Memory Maps */

static ADDRESS_MAP_START( amu880_mem, AS_PROGRAM, 8, amu880_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xe7ff) AM_RAM
	AM_RANGE(0xe800, 0xefff) AM_RAM AM_SHARE("video_ram")
	AM_RANGE(0xf000, 0xfbff) AM_ROM
	AM_RANGE(0xfc00, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( amu880_io, AS_IO, 8, amu880_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
//  AM_RANGE(0x00, 0x00) AM_MIRROR(0x03) AM_WRITE(power_off_w)
//  AM_RANGE(0x04, 0x04) AM_MIRROR(0x02) AM_WRITE(tone_off_w)
//  AM_RANGE(0x05, 0x05) AM_MIRROR(0x02) AM_WRITE(tone_on_w)
	AM_RANGE(0x08, 0x09) AM_MIRROR(0x02) AM_READ(keyboard_r)
	AM_RANGE(0x0c, 0x0f) AM_DEVREADWRITE(Z80PIO2_TAG, z80pio_device, read_alt, write_alt)
	AM_RANGE(0x10, 0x13) AM_DEVREADWRITE(Z80PIO1_TAG, z80pio_device, read_alt, write_alt)
	AM_RANGE(0x14, 0x17) AM_DEVREADWRITE(Z80CTC_TAG, z80ctc_device, read, write)
	AM_RANGE(0x18, 0x1b) AM_DEVREADWRITE(Z80SIO_TAG, z80sio0_device, ba_cd_r, ba_cd_w)
ADDRESS_MAP_END

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

UINT32 amu880_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int y, sx, x, line;
	const pen_t *pen = m_palette->pens();

	for (y = 0; y < 240; y++)
	{
		line = y % 10;

		for (sx = 0; sx < 64; sx++)
		{
			UINT16 videoram_addr = ((y / 10) * 64) + sx;
			UINT8 videoram_data = m_video_ram[videoram_addr & 0x7ff];

			UINT16 charrom_addr = ((videoram_data & 0x7f) << 3) | line;
			UINT8 data = m_char_rom->base()[charrom_addr & 0x3ff];

			for (x = 0; x < 6; x++)
			{
				int color = ((line > 7) ? 0 : BIT(data, 7)) ^ BIT(videoram_data, 7);

				bitmap.pix32(y, (sx * 6) + x) = pen[color];

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
	/* cassette transmit/receive clock */
}

/* Z80-SIO Interface */

TIMER_DEVICE_CALLBACK_MEMBER( amu880_state::tape_tick )
{
	m_z80sio->rxa_w(m_cassette->input() < 0.0);
}

WRITE_LINE_MEMBER(amu880_state::cassette_w)
{
	m_cassette->output(state ? -1.0 : +1.0);
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
	// find keyboard rows
	m_key_row[0] = m_y0;
	m_key_row[1] = m_y1;
	m_key_row[2] = m_y2;
	m_key_row[3] = m_y3;
	m_key_row[4] = m_y4;
	m_key_row[5] = m_y5;
	m_key_row[6] = m_y6;
	m_key_row[7] = m_y7;
	m_key_row[8] = m_y8;
	m_key_row[9] = m_y9;
	m_key_row[10] = m_y10;
	m_key_row[11] = m_y11;
	m_key_row[12] = m_y12;
	m_key_row[13] = m_y13;
	m_key_row[14] = m_y14;
	m_key_row[15] = m_y15;

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

static GFXDECODE_START( amu880 )
	GFXDECODE_ENTRY( "chargen", 0x0000, amu880_charlayout, 0, 1 )
GFXDECODE_END


static MACHINE_CONFIG_START( amu880, amu880_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_10MHz/4) // U880D
	MCFG_CPU_PROGRAM_MAP(amu880_mem)
	MCFG_CPU_IO_MAP(amu880_io)
	MCFG_CPU_CONFIG(amu880_daisy_chain)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("keyboard", amu880_state, keyboard_tick, attotime::from_hz(1500))

	/* video hardware */
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(amu880_state, screen_update)
	MCFG_SCREEN_RAW_PARAMS(9000000, 576, 0*6, 64*6, 320, 0*10, 24*10)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", amu880)
	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")

	/* devices */
	MCFG_DEVICE_ADD(Z80CTC_TAG, Z80CTC, XTAL_10MHz/4)
	MCFG_Z80CTC_INTR_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))
	MCFG_Z80CTC_ZC0_CB(WRITELINE(amu880_state, ctc_z0_w))
	MCFG_Z80CTC_ZC1_CB(DEVWRITELINE(Z80SIO_TAG, z80dart_device, rxtxcb_w))
	MCFG_Z80CTC_ZC2_CB(WRITELINE(amu880_state, ctc_z2_w))

	MCFG_DEVICE_ADD(Z80PIO1_TAG, Z80PIO, XTAL_10MHz/4)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))

	MCFG_DEVICE_ADD(Z80PIO2_TAG, Z80PIO, XTAL_10MHz/4)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))

	MCFG_Z80SIO0_ADD(Z80SIO_TAG, XTAL_10MHz/4, 0, 0, 0, 0) // U856
	MCFG_Z80DART_OUT_TXDA_CB(WRITELINE(amu880_state, cassette_w))
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))

	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_MUTED)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("tape", amu880_state, tape_tick, attotime::from_hz(44100))

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
MACHINE_CONFIG_END

/* ROMs */

ROM_START( amu880 )
	ROM_REGION( 0x10000, Z80_TAG, 0 )
	ROM_DEFAULT_BIOS( "v21" )
	ROM_SYSTEM_BIOS( 0, "v21", "H.MON v2.1" )
	ROMX_LOAD( "mon21a.bin", 0xf000, 0x0400, NO_DUMP, ROM_BIOS(1) )
	ROMX_LOAD( "mon21b.bin", 0xf400, 0x0400, NO_DUMP, ROM_BIOS(1) )
	ROMX_LOAD( "mon21c.bin", 0xf800, 0x0400, NO_DUMP, ROM_BIOS(1) )
	ROMX_LOAD( "mon21.bin", 0xf000, 0x0bdf, BAD_DUMP CRC(ba905563) SHA1(1fa0aeab5428731756bdfa74efa3c664898bf083), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "v30", "H.MON v3.0" )
	ROMX_LOAD( "mon30.bin", 0xf000, 0x1000, CRC(033f8112) SHA1(0c6ae7b9d310dec093652db6e8ae84f8ebfdcd29), ROM_BIOS(2) )

	ROM_REGION( 0x4800, "hbasic", 0 )
	ROM_LOAD( "mon30p_hbasic33p.bin", 0x0000, 0x4800, CRC(c927e7be) SHA1(2d1f3ff4d882c40438a1281872c6037b2f07fdf2) )

	ROM_REGION( 0x400, "chargen", 0 )
	ROM_LOAD( "hemcfont.bin", 0x0000, 0x0400, CRC(1074d103) SHA1(e558279cff5744acef4eccf30759a9508b7f8750) )

	ROM_REGION( 0x400, "keyboard", 0 )
	ROM_LOAD( "keyboard.bin", 0x0000, 0x0400, BAD_DUMP CRC(daa06361) SHA1(b0299bd8d1686e05dbeeaed54f6c41ae543be20c) ) // typed in from manual
ROM_END

/* System Drivers */

/*    YEAR  NAME    PARENT  COMPAT  MACHINE INPUT   INIT    COMPANY                     FULLNAME                                        FLAGS */
COMP( 1983, amu880, 0,      0,      amu880, amu880, driver_device,  0,      "Militaerverlag der DDR",   "Ausbaufaehiger Mikrocomputer mit dem U 880",   MACHINE_NO_SOUND )
