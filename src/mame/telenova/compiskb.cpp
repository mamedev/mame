// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Telenova Compis keyboard emulation

*********************************************************************/

#include "emu.h"
#include "compiskb.h"

#include "speaker.h"

#include "utf8.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define I8748_TAG       "i8748"
#define SPEAKER_TAG     "speaker"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(COMPIS_KEYBOARD, compis_keyboard_device, "compiskb", "Compis Keyboard")


//-------------------------------------------------
//  ROM( compis_keyboard )
//-------------------------------------------------

ROM_START( compis_keyboard )
	ROM_REGION( 0x800, I8748_TAG, 0 )
	ROM_LOAD( "cmpkey13.u1", 0x000, 0x800, CRC(3f87d138) SHA1(c04e2d325b9c04818bc7c47c3bf32b13862b11ec) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *compis_keyboard_device::device_rom_region() const
{
	return ROM_NAME( compis_keyboard );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void compis_keyboard_device::device_add_mconfig(machine_config &config)
{
	I8748(config, m_maincpu, 2016000); // XTAL(4'032'000)/2 ???
	m_maincpu->bus_in_cb().set(FUNC(compis_keyboard_device::bus_r));
	m_maincpu->bus_out_cb().set(FUNC(compis_keyboard_device::bus_w));
	m_maincpu->p1_in_cb().set(FUNC(compis_keyboard_device::p1_r));
	m_maincpu->p2_in_cb().set(FUNC(compis_keyboard_device::p2_r));
	m_maincpu->t0_in_cb().set_constant(0); // ???
	m_maincpu->t1_in_cb().set_constant(0); // ???

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);
}


//-------------------------------------------------
//  INPUT_PORTS( compis_keyboard )
//-------------------------------------------------

INPUT_PORTS_START( compis_keyboard )
	PORT_START("Y1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"ü  Ü") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(0x00fc,'~') PORT_CHAR(0x00dc,'^')
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("TABR") PORT_CODE(KEYCODE_PGDN) PORT_CHAR(UCHAR_MAMEKEY(PGDN))
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1_PAD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("TABL") PORT_CODE(KEYCODE_PGUP) PORT_CHAR(UCHAR_MAMEKEY(PGUP))
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3_PAD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad +") PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR(':')
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("''  *") PORT_CODE(KEYCODE_TILDE) PORT_CHAR(39) PORT_CHAR('*')
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("COMPIS !") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("COMPIS ?") PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("COMPIS |") PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0_PAD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR(UCHAR_MAMEKEY(000_PAD))
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHAR(UCHAR_MAMEKEY(00_PAD))
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED ) // 49
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNUSED ) // 52
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('<') PORT_CHAR('>')
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED ) // 41
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR(';')
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y4")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED ) // 4b
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y5")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t')
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y6")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('/')
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y7")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(0x00f6) PORT_CHAR(0x00d6) // ü Ü
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("INPASSA") PORT_CODE(KEYCODE_INSERT) PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"SÖK") PORT_CODE(KEYCODE_PRTSCR) PORT_CHAR(UCHAR_MAMEKEY(PRTSCR))
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"UTPLÅNA") PORT_CODE(KEYCODE_DEL) PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("COMPIS S") PORT_CODE(KEYCODE_NUMLOCK) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad C") PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u232b") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8) // ⌫ (erase left)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('+') PORT_CHAR('?')
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y8")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13) //TODO utf8
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("AVBRYT") PORT_CODE(KEYCODE_SCRLOCK) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7_PAD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("START-STOP") PORT_CODE(KEYCODE_PAUSE) PORT_CHAR(UCHAR_MAMEKEY(PAUSE))
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9_PAD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad ,") PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR(UCHAR_MAMEKEY(COMMA_PAD))
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(0x00e5) PORT_CHAR(0x00c5) // å Å
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y9")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('=')
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("COMPIS +")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5_PAD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad -") PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(0x00e4) PORT_CHAR(0x00c4) // ä Ä
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR(0x00b4) PORT_CHAR('`') // ´
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SPECIAL")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u21e9") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) // ⇩
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u21e7\u21e7") PORT_CODE(KEYCODE_LALT) PORT_CODE(KEYCODE_RALT) // ⇧⇧
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u21e7") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1) // ⇧
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor compis_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( compis_keyboard );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  compis_keyboard_device - constructor
//-------------------------------------------------

compis_keyboard_device::compis_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, COMPIS_KEYBOARD, tag, owner, clock),
		m_maincpu(*this, I8748_TAG),
		m_speaker(*this, SPEAKER_TAG),
		m_y(*this, "Y%u", 1),
		m_special(*this, "SPECIAL"),
		m_out_tx_handler(*this),
		m_led_caps(*this, "led_caps"),
		m_keylatch(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void compis_keyboard_device::device_start()
{
	m_out_tx_handler(1);

	// resolve output finder
	m_led_caps.resolve();
}


//-------------------------------------------------
//  si_w - serial input write
//-------------------------------------------------

void compis_keyboard_device::si_w(int state)
{
	m_maincpu->set_input_line(MCS48_INPUT_IRQ, state ? CLEAR_LINE : ASSERT_LINE);
}


//-------------------------------------------------
//  bus_r -
//-------------------------------------------------

uint8_t compis_keyboard_device::bus_r()
{
	// HACK this should be handled in mcs48.cpp
	return m_bus;
}


//-------------------------------------------------
//  bus_w -
//-------------------------------------------------

void compis_keyboard_device::bus_w(uint8_t data)
{
	/*

	    bit     description

	    0       keyboard row bit 0
	    1       keyboard row bit 1
	    2       keyboard row bit 2
	    3       keyboard row bit 3
	    4
	    5       speaker
	    6       CAPS LED
	    7       data out

	*/

	m_bus = data;

	// keyboard column
	m_keylatch = (data & 0x0f);

	// speaker
	m_speaker->level_w(BIT(data, 5));

	// LEDs
	m_led_caps = BIT(data, 6);

	// serial data out
	m_out_tx_handler(BIT(data, 7));
}


//-------------------------------------------------
//  bus_w -
//-------------------------------------------------

uint8_t compis_keyboard_device::p1_r()
{
	uint8_t data = 0xff;

	switch (m_keylatch)
	{
	case 1: data &= m_y[0]->read(); break;
	case 2: data &= m_y[1]->read(); break;
	case 3: data &= m_y[2]->read(); break;
	case 4: data &= m_y[3]->read(); break;
	case 5: data &= m_y[4]->read(); break;
	case 6: data &= m_y[5]->read(); break;
	case 7: data &= m_y[6]->read(); break;
	case 8: data &= m_y[7]->read(); break;
	case 9: data &= m_y[8]->read(); break;
	}

	return data;
}


//-------------------------------------------------
//  p2_r -
//-------------------------------------------------

uint8_t compis_keyboard_device::p2_r()
{
	uint8_t data = 0xff;

	switch (m_keylatch)
	{
	case 1: data &= m_y[0]->read() >> 8; break;
	case 2: data &= m_y[1]->read() >> 8; break;
	case 3: data &= m_y[2]->read() >> 8; break;
	case 4: data &= m_y[3]->read() >> 8; break;
	case 5: data &= m_y[4]->read() >> 8; break;
	case 6: data &= m_y[5]->read() >> 8; break;
	case 7: data &= m_y[6]->read() >> 8; break;
	case 8: data &= m_y[7]->read() >> 8; break;
	case 9: data &= m_y[8]->read() >> 8; break;
	}

	data &= m_special->read();

	return data;
}
