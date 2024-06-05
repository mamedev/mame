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
|           4013 3.57MHz|-------------|       4050   5114   5114                            |
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
    CDP1802 - RCA CDP1802BE CMOS 8-Bit Microprocessor running at 3.57MHz
    CDP1852 - RCA CDP1852CE Byte-Wide Input/Output Port
    CDP1853 - RCA CDP1853CE N-Bit 1 of 8 Decoder
    CDP1856 - RCA CDP1856CE 4-Bit Memory Buffer
    CDP1869 - RCA CDP1869CE Video Interface System (VIS) Address and Sound Generator
    CDP1870 - RCA CDP1870CE Video Interface System (VIS) Color Video (DOT XTAL at 5.6260MHz, CHROM XTAL at 8.867238MHz)
    CN1     - RF connector [TMC-700]
    CN2     - 10x2 pin printer connector [TMC-700]
                    GND   1  2  D0
                    GND   3  4  D1
                    GND   5  6  D2
                    GND   7  8  D3
                    GND   9 10  D4
                    GND  11 12  D5
                    GND  13 14  D6
                    GND  15 16  D7
                    GND  17 18  BUSY
                    GND  19 20  _STROBE
    CN3     - 32x3 pin EURO connector
    CN4     - DIN5D tape connector
                1   input (500 mV / 47 Kohm)
                2   GND
                3   output (580 mV / 47 Kohm)
                4   input (500 mV / 47 Kohm)
                5   output (580 mV / 47 Kohm)
    CN5     - DIN5X video connector
                1   GND
                2   GND
                3   composite video output (75 ohm)
                4   GND
                5   composite video output (75 ohm)
    CN6     - DIN2 power connector
                1   input 8..12V DC..400Hz 300mA
                2   GND
    CN7     - DIN5D audio connector [TMCP-300]
                1   N/C
                2   GND
                3   mono audio output
                4   N/C
                5   mono audio output
    CN8     - 10x2 pin keyboard connector
    SW1     - RUN/STOP switch (left=run, right=stop)
    SW2     - internal speaker/external audio switch [TMCP-300]
    P1      - color phase lock adjustment potentiometer
    C1      - dot oscillator adjustment variable capacitor
    C2      - chroma oscillator adjustment variable capacitor
    T1      - RF signal strength adjustment potentiometer [TMC-700]
    T2      - tape recording level adjustment potentiometer (0.57 V p-p)
    T3      - video output level adjustment potentiometer (1 V p-p)
    T4      - video synchronization pulse adjustment potentiometer
    K1      - RF signal quality adjustment variable inductor [TMC-700]
    K2      - RF channel adjustment variable inductor (VHF I) [TMC-700]
    LS1     - loudspeaker

*/

/*

    TODO

    - screen update is too fast
    - cursor on text should blink as dark blue
    - PRWNOISE and PRBEEP return wrong values
    - CDP1869 white noise
    - connect expansion bus
    - series I ROMs
    - DOS ROMs

*/

#include "emu.h"
#include "tmc600.h"

#include "utf8.h"


//**************************************************************************
//  I/O
//**************************************************************************

uint8_t tmc600_state::rtc_r()
{
	m_rtc_int = m_vismac_reg_latch >> 3;

	return 0;
}

void tmc600_state::printer_w(uint8_t data)
{
	m_centronics->write_data0(BIT(data, 0));
	m_centronics->write_data1(BIT(data, 1));
	m_centronics->write_data2(BIT(data, 2));
	m_centronics->write_data3(BIT(data, 3));
	m_centronics->write_data4(BIT(data, 4));
	m_centronics->write_data5(BIT(data, 5));
	m_centronics->write_data6(BIT(data, 6));
	m_centronics->write_data7(BIT(data, 7));

	m_centronics->write_strobe(0);
	m_centronics->write_strobe(1);
}

int tmc600_state::ef2_r()
{
	return m_cassette->input() < 0;
}

int tmc600_state::ef3_r()
{
	return !BIT(m_key_row[(m_out3 >> 3) & 0x07]->read(), m_out3 & 0x07);
}

void tmc600_state::q_w(int state)
{
	m_cassette->output(state ? +1.0 : -1.0);
}

void tmc600_state::sc_w(uint8_t data)
{
	if (data == COSMAC_STATE_CODE_S3_INTERRUPT) {
		m_maincpu->int_w(CLEAR_LINE);
	}
}

void tmc600_state::out3_w(uint8_t data)
{
	m_out3 = data;
}

QUICKLOAD_LOAD_MEMBER(tmc600_state::quickload_cb)
{
	int size = image.length();

	if (size < 16)
		return std::make_pair(image_error::INVALIDLENGTH, "Image is too short");

	if ((size - 16) > (m_ram->size() - 0x300))
		return std::make_pair(image_error::INVALIDLENGTH, "Image is larger than RAM");

	address_space &program = m_maincpu->space(AS_PROGRAM);

	image.fseek(0x5, SEEK_SET);
	image.fread(program.get_write_ptr(0x6181), 4); // DEFUS and EOP
	image.fread(program.get_write_ptr(0x6192), 4); // STRING and ARRAY

	image.fseek(0x9, SEEK_SET);
	image.fread(program.get_write_ptr(0x6199), 2); // EOD

	image.fseek(0xf, SEEK_SET);
	image.fread(program.get_write_ptr(0x6300), size); // program

	return std::make_pair(std::error_condition(), std::string());
}


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void tmc600_state::tmc600_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x6000, 0x7fff).ram();
	map(0xf400, 0xf7ff).m(m_vis, FUNC(cdp1869_device::char_map));
	map(0xf800, 0xffff).m(m_vis, FUNC(cdp1869_device::page_map));
}

void tmc600_state::tmc600_io_map(address_map &map)
{
	map(0x03, 0x03).w(m_bwio, FUNC(cdp1852_device::write));
	map(0x04, 0x04).w(CDP1852_TMC700_TAG, FUNC(cdp1852_device::write));
	map(0x05, 0x05).rw(FUNC(tmc600_state::rtc_r), FUNC(tmc600_state::vismac_data_w));
//  map(0x06, 0x06).w(FUNC(tmc600_state::floppy_w);
	map(0x07, 0x07).w(FUNC(tmc600_state::vismac_register_w));
}


//**************************************************************************
//  INPUT PORTS
//**************************************************************************

static INPUT_PORTS_START( tmc600 )
	PORT_START("Y0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_CHAR('0') PORT_CHAR('_')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CHAR('7') PORT_CHAR('\'')

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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('@')
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
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(U'Å')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(U'Ä')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(U'Ö')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("BREAK") PORT_CODE(KEYCODE_END) PORT_CHAR(UCHAR_MAMEKEY(END),3)

	PORT_START("Y6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("DEL") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC),27)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("E2") PORT_CODE(KEYCODE_RALT) PORT_CHAR(UCHAR_MAMEKEY(RALT))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CTRL1") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CTRL2") PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(RCONTROL))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("E1") PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(LALT))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("Y7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("(unknown)") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LINE FEED") PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME)) PORT_CHAR(10)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\u2191") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\u2192") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(13)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\u2193") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\u2190") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))

	PORT_START("RUN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Run/Stop") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_TOGGLE PORT_WRITE_LINE_DEVICE_MEMBER(CDP1802_TAG, cosmac_device, clear_w)
INPUT_PORTS_END


//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

void tmc600_state::tmc600(machine_config &config)
{
	// CPU
	cdp1802_device &cpu(CDP1802(config, CDP1802_TAG, 3.57_MHz_XTAL));
	cpu.set_addrmap(AS_PROGRAM, &tmc600_state::tmc600_map);
	cpu.set_addrmap(AS_IO, &tmc600_state::tmc600_io_map);
	cpu.wait_cb().set_constant(1);
	cpu.ef2_cb().set(FUNC(tmc600_state::ef2_r));
	cpu.ef3_cb().set(FUNC(tmc600_state::ef3_r));
	cpu.q_cb().set(FUNC(tmc600_state::q_w));
	cpu.sc_cb().set(FUNC(tmc600_state::sc_w));
	cpu.tpb_cb().set(CDP1852_KB_TAG, FUNC(cdp1852_device::clock_w));
	cpu.tpb_cb().append(CDP1852_TMC700_TAG, FUNC(cdp1852_device::clock_w));

	// sound and video hardware
	tmc600_video(config);

	// keyboard output latch
	CDP1852(config, m_bwio); // clock is CDP1802 TPB
	m_bwio->mode_cb().set_constant(1);
	m_bwio->do_cb().set(FUNC(tmc600_state::out3_w));

	// address bus demux for expansion bus
	cdp1852_device &demux(CDP1852(config, CDP1852_BUS_TAG)); // clock is expansion bus TPA
	demux.mode_cb().set_constant(0);

	// printer output latch
	cdp1852_device &prtout(CDP1852(config, CDP1852_TMC700_TAG)); // clock is CDP1802 TPB
	prtout.mode_cb().set_constant(1);
	prtout.do_cb().set(FUNC(tmc600_state::printer_w));

	// printer connector
	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(CDP1802_TAG, FUNC(cosmac_device::ef4_w)).exor(1);

	// cassette
	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);

	// quickload
	quickload_image_device &quickload(QUICKLOAD(config, "quickload", "tmc600"));
	quickload.set_load_callback(FUNC(tmc600_state::quickload_cb));
	quickload.set_interface("tmc600_quik");

	// expansion bus connector
	TMC600_EUROBUS_SLOT(config, m_bus, tmc600_eurobus_cards, nullptr);

	// internal RAM
	RAM(config, RAM_TAG).set_default_size("8K");

	// software lists
	SOFTWARE_LIST(config, "quik_list").set_original("tmc600_quik");
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

#if 0
ROM_START( tmc600s1 )
	ROM_REGION( 0x6000, CDP1802_TAG, 0 )
	ROM_LOAD( "sb20",       0x0000, 0x1000, NO_DUMP )
	ROM_LOAD( "sb21",       0x1000, 0x1000, NO_DUMP )
	ROM_LOAD( "sb22",       0x2000, 0x1000, NO_DUMP )
	ROM_LOAD( "sb23",       0x3000, 0x1000, NO_DUMP )
	ROM_SYSTEM_BIOS( 0, "sb040282", "SB040282" )
	ROMX_LOAD( "190482",    0x4000, 0x1000, NO_DUMP, ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "sbdos", "SBDOS" )
	ROMX_LOAD( "190482_",   0x4000, 0x1000, NO_DUMP, ROM_BIOS(1) )
	ROMX_LOAD( "190482_v",  0x5000, 0x1000, NO_DUMP, ROM_BIOS(1) )

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "chargen",    0x0000, 0x1000, CRC(93f92cbf) SHA1(371156fb38fa5319c6fde537ccf14eed94e7adfb) )
ROM_END
#endif

ROM_START( tmc600s2 )
	ROM_REGION( 0x6000, CDP1802_TAG, 0 )
	ROM_LOAD( "sb30",       0x0000, 0x1000, CRC(95d1292a) SHA1(1fa52d59d3005f8ac74a32c2164fdb22947c2748) )
	ROM_LOAD( "sb31",       0x1000, 0x1000, CRC(2c8f3d17) SHA1(f14e8adbcddeaeaa29b1e7f3dfa741f4e230f599) )
	ROM_LOAD( "sb32",       0x2000, 0x1000, CRC(dd58a128) SHA1(be9bdb0fc5e0cc3dcc7f2fb7ccab69bf5b043803) )
	ROM_LOAD( "sb33",       0x3000, 0x1000, CRC(b7d241fa) SHA1(6f3eadf86c4e3aaf93d123e302a18dc4d9db964b) )
	ROM_SYSTEM_BIOS( 0, "sb040282", "SB040282" )
	ROMX_LOAD( "151182",    0x4000, 0x1000, CRC(c1a8d9d8) SHA1(4552e1f06d0e338ba7b0f1c3a20b8a51c27dafde), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "sbdos", "SBDOS" )
	ROMX_LOAD( "151182_",   0x4000, 0x1000, NO_DUMP, ROM_BIOS(1) )
	ROMX_LOAD( "151182_v",  0x5000, 0x1000, NO_DUMP, ROM_BIOS(1) )

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "chargen",    0x0000, 0x1000, CRC(93f92cbf) SHA1(371156fb38fa5319c6fde537ccf14eed94e7adfb) )
ROM_END


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME      PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY        FULLNAME                     FLAGS
//COMP( 1982, tmc600s1, 0,      0,      tmc600,  tmc600, tmc600_state, empty_init, "Telercas Oy", "Telmac TMC-600 (Sarja I)",  MACHINE_NOT_WORKING )
COMP( 1982, tmc600s2, 0,      0,      tmc600,  tmc600, tmc600_state, empty_init, "Telercas Oy", "Telmac TMC-600 (Sarja II)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
