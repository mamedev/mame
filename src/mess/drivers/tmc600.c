// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

Telmac TMC-600

PCB Layout
----------

HP14782-1

            CN2                    CN3
        |---------|     |------------------------|
|-------|---------|-----|------------------------|------------------------------------------|
|  CN1                                                  CN4 SW1       CN5 CN6 P1 SW2 CN7    |
|         |-------|                                4049         T3                          |
| T1 K1   |CDP1852|         4050  4050  4050            40107 T2         C1  C2 8.867238MHz |
|         |-------|         |-------|                                     5.6260MHz T4      |
| MC1374                    |CDP1852|                               ROM5    |---------|     |
|                           |-------|                                       | CDP1870 |     |
|    K2                                                             ROM4    |---------|     |
|                       |-------------|       4050   5114   5114                            |
|           4013        |   CDP1802   |       4050   5114   5114    ROM3    4030  ROM6  4060|
|           4013        |-------------|       4050   5114   5114                            |
|           4081  40106                       4051   5114   5114    ROM2    4076  5114  4011|
|                                             4556   5114   5114          CDP1856 5114      |
|                   |-------|                        5114   5114    ROM1  CDP1856 5114  4076|
|           LS1     |CDP1852| CDP1853       CDP1856  5114   5114                            |
|                   |-------|               CDP1856  5114   5114    ROM0    |---------| 4011|
|                                                                           | CDP1869 |     |
|   7805            4051      4051                                          |---------|     |
|                                                                                           |
|                     |--CN8--|                                                             |
|-------------------------------------------------------------------------------------------|

Notes:
    All IC's shown. TMCP-300 and TMC-700 expansions have been installed.

    ROM0-5  - Toshiba TMM2732DI 4Kx8 EPROM
    ROM6    - Hitachi HN462732G 4Kx8 EPROM
    5114    - RCA MWS5114E1 1024-Word x 4-Bit LSI Static RAM
    MC1374  - Motorola MC1374P TV Modulator
    CDP1802 - RCA CDP1802BE CMOS 8-Bit Microprocessor running at ? MHz
    CDP1852 - RCA CDP1852CE Byte-Wide Input/Output Port
    CDP1853 - RCA CDP1853CE N-Bit 1 of 8 Decoder
    CDP1856 - RCA CDP1856CE 4-Bit Memory Buffer
    CDP1869 - RCA CDP1869CE Video Interface System (VIS) Address and Sound Generator
    CDP1870 - RCA CDP1870CE Video Interface System (VIS) Color Video (DOT XTAL at 5.6260MHz, CHROM XTAL at 8.867238MHz)
    CN1     - RF connector [TMC-700]
    CN2     - printer connector [TMC-700]
    CN3     - EURO connector
    CN4     - tape connector
    CN5     - video connector
    CN6     - power connector
    CN7     - audio connector [TMCP-300]
    CN8     - keyboard connector
    SW1     - RUN/STOP switch
    SW2     - internal speaker/external audio switch [TMCP-300]
    P1      - color phase lock adjustment
    C1      - dot oscillator adjustment
    C2      - chrom oscillator adjustment
    T1      - RF signal strength adjustment [TMC-700]
    T2      - tape recording level adjustment (0.57 Vpp)
    T3      - video output level adjustment (1 Vpp)
    T4      - video synchronization pulse adjustment
    K1      - RF signal quality adjustment [TMC-700]
    K2      - RF channel adjustment (VHF I) [TMC-700]
    LS1     - loudspeaker

*/

/*

    TODO:

    - proper emulation of the VISMAC interface (cursor blinking, color RAM), schematics are needed
    - disk interface
    - CPU frequency needs to be derived from the schematics
    - serial interface expansion card
    - centronics printer handshaking

*/

#include "includes/tmc600.h"

/* Read/Write Handlers */

WRITE8_MEMBER( tmc600_state::keyboard_latch_w )
{
	m_keylatch = data;
}

/* Memory Maps */

static ADDRESS_MAP_START( tmc600_map, AS_PROGRAM, 8, tmc600_state )
	AM_RANGE(0x0000, 0x4fff) AM_ROM
	AM_RANGE(0x6000, 0xbfff) AM_RAM
	AM_RANGE(0xf400, 0xf7ff) AM_DEVICE(CDP1869_TAG, cdp1869_device, char_map)
	AM_RANGE(0xf800, 0xffff) AM_DEVICE(CDP1869_TAG, cdp1869_device, page_map)
ADDRESS_MAP_END

static ADDRESS_MAP_START( tmc600_io_map, AS_IO, 8, tmc600_state )
	AM_RANGE(0x03, 0x03) AM_WRITE(keyboard_latch_w)
	AM_RANGE(0x04, 0x04) AM_DEVWRITE("cent_data_out", output_latch_device, write)
	AM_RANGE(0x05, 0x05) AM_WRITE(vismac_data_w)
//  AM_RANGE(0x06, 0x06) AM_WRITE(floppy_w)
	AM_RANGE(0x07, 0x07) AM_WRITE(vismac_register_w)
ADDRESS_MAP_END

/* Input Ports */

static INPUT_PORTS_START( tmc600 )
	PORT_START("Y0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CHAR('7') PORT_CHAR('/')

	PORT_START("Y1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("Y2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('@') PORT_CHAR('\\')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G')

	PORT_START("Y3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O')

	PORT_START("Y4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W')

	PORT_START("Y5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xC3\x85") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(0x00C5)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xC3\x84") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(0x00C4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xC3\x96") PORT_CODE(KEYCODE_COLON) PORT_CHAR(0x00D6)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("BREAK") PORT_CODE(KEYCODE_END) PORT_CHAR(UCHAR_MAMEKEY(END))

	PORT_START("Y6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("DEL") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("E2") PORT_CODE(KEYCODE_RALT) PORT_CHAR(UCHAR_MAMEKEY(RALT))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CTRL1") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CTRL2") PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(RCONTROL))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("E1") PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(LALT))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("Y7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("(unknown)") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LINE FEED") PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME)) PORT_CHAR(10)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(13)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))

	PORT_START("RUN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Run/Stop") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_TOGGLE
INPUT_PORTS_END

/* CDP1802 Interface */

READ_LINE_MEMBER( tmc600_state::clear_r )
{
	return BIT(m_run->read(), 0);
}

READ_LINE_MEMBER( tmc600_state::ef2_r )
{
	return m_cassette->input() < 0;
}

READ_LINE_MEMBER( tmc600_state::ef3_r )
{
	UINT8 data = ~m_key_row[m_keylatch / 8]->read();

	return BIT(data, m_keylatch % 8);
}

WRITE_LINE_MEMBER( tmc600_state::q_w )
{
	m_cassette->output(state ? +1.0 : -1.0);
}

/* Machine Initialization */

void tmc600_state::machine_start()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	/* configure RAM */
	switch (m_ram->size())
	{
	case 8*1024:
		program.unmap_readwrite(0x8000, 0xbfff);
		break;

	case 16*1024:
		program.unmap_readwrite(0xa000, 0xbfff);
		break;
	}

	// find keyboard rows
	m_key_row[0] = m_y0;
	m_key_row[1] = m_y1;
	m_key_row[2] = m_y2;
	m_key_row[3] = m_y3;
	m_key_row[4] = m_y4;
	m_key_row[5] = m_y5;
	m_key_row[6] = m_y6;
	m_key_row[7] = m_y7;

	/* register for state saving */
	save_item(NAME(m_keylatch));
}

/* Machine Drivers */

static MACHINE_CONFIG_START( tmc600, tmc600_state )
	// basic system hardware
	MCFG_CPU_ADD(CDP1802_TAG, CDP1802, 3579545)  // ???
	MCFG_CPU_PROGRAM_MAP(tmc600_map)
	MCFG_CPU_IO_MAP(tmc600_io_map)
	MCFG_COSMAC_WAIT_CALLBACK(VCC)
	MCFG_COSMAC_CLEAR_CALLBACK(READLINE(tmc600_state, clear_r))
	MCFG_COSMAC_EF2_CALLBACK(READLINE(tmc600_state, ef2_r))
	MCFG_COSMAC_EF3_CALLBACK(READLINE(tmc600_state, ef3_r))
	MCFG_COSMAC_Q_CALLBACK(WRITELINE(tmc600_state, q_w))

	// sound and video hardware
	MCFG_FRAGMENT_ADD(tmc600_video)

	/* devices */
	MCFG_CENTRONICS_ADD(CENTRONICS_TAG, centronics_devices, "printer")

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", CENTRONICS_TAG)

	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_MUTED)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("8K")
	MCFG_RAM_EXTRA_OPTIONS("16K,24K")
MACHINE_CONFIG_END

/* ROMs */

#if 0
ROM_START( tmc600s1 )
	ROM_REGION( 0x5000, CDP1802_TAG, 0 )
	ROM_LOAD( "sb20",       0x0000, 0x1000, NO_DUMP )
	ROM_LOAD( "sb21",       0x1000, 0x1000, NO_DUMP )
	ROM_LOAD( "sb22",       0x2000, 0x1000, NO_DUMP )
	ROM_LOAD( "sb23",       0x3000, 0x1000, NO_DUMP )
	ROM_LOAD( "190482_2",   0x4000, 0x1000, NO_DUMP )

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "chargen",    0x0000, 0x1000, NO_DUMP )
ROM_END
#endif

ROM_START( tmc600s2 )
	ROM_REGION( 0x5000, CDP1802_TAG, 0 )
	ROM_LOAD( "sb30",       0x0000, 0x1000, CRC(95d1292a) SHA1(1fa52d59d3005f8ac74a32c2164fdb22947c2748) )
	ROM_LOAD( "sb31",       0x1000, 0x1000, CRC(2c8f3d17) SHA1(f14e8adbcddeaeaa29b1e7f3dfa741f4e230f599) )
	ROM_LOAD( "sb32",       0x2000, 0x1000, CRC(dd58a128) SHA1(be9bdb0fc5e0cc3dcc7f2fb7ccab69bf5b043803) )
	ROM_LOAD( "sb33",       0x3000, 0x1000, CRC(b7d241fa) SHA1(6f3eadf86c4e3aaf93d123e302a18dc4d9db964b) )
	ROM_LOAD( "151182",     0x4000, 0x1000, CRC(c1a8d9d8) SHA1(4552e1f06d0e338ba7b0f1c3a20b8a51c27dafde) )

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "chargen",    0x0000, 0x1000, CRC(93f92cbf) SHA1(371156fb38fa5319c6fde537ccf14eed94e7adfb) )
ROM_END

/* System Drivers */
//    YEAR  NAME      PARENT    COMPAT   MACHINE   INPUT     INIT    COMPANY        FULLNAME
//COMP( 1982, tmc600s1, 0,  0,       tmc600,   tmc600, driver_device,   0,        "Telercas Oy", "Telmac TMC-600 (Sarja I)",  MACHINE_NOT_WORKING )
COMP( 1982, tmc600s2, 0,    0,       tmc600,   tmc600, driver_device,   0,     "Telercas Oy", "Telmac TMC-600 (Sarja II)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
