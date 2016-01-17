// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/**********************************************************************

    K7659 keyboard emulation

*********************************************************************/

#include "k7659kb.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************




//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type K7659_KEYBOARD = &device_creator<k7659_keyboard_device>;



//-------------------------------------------------
//  ROM( k7659_keyboard )
//-------------------------------------------------

ROM_START( k7659_keyboard )
	ROM_REGION(0x0800, "k7659",0)
	ROM_LOAD ("k7659n.bin", 0x0000, 0x0800, CRC(7454bf0a) SHA1(b97e7df93778fa371b96b6f4fb1a5b1c8b89d7ba) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *k7659_keyboard_device::device_rom_region() const
{
	return ROM_NAME( k7659_keyboard );
}


//-------------------------------------------------
//  ADDRESS_MAP( kb_io )
//-------------------------------------------------



//-------------------------------------------------
//  DISCRETE_SOUND_START( k7659kb )
//-------------------------------------------------



//-------------------------------------------------
//  MACHINE_DRIVER( k7659_keyboard )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( k7659_keyboard )

MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor k7659_keyboard_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( k7659_keyboard );
}


//-------------------------------------------------
//  INPUT_PORTS( k7659_keyboard )
//-------------------------------------------------

static INPUT_PORTS_START( k7659_keyboard )
	PORT_START("A1")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1 !") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q') PORT_CHAR(0x11)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A') PORT_CHAR(0x01)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y') PORT_CHAR(0x19)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2 \"") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W') PORT_CHAR(0x17)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S') PORT_CHAR(0x13)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X') PORT_CHAR(0x18)
	PORT_START("A2")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR(0xff)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E') PORT_CHAR(0x05)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D') PORT_CHAR(0x04)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C') PORT_CHAR(0x03)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4 $") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R') PORT_CHAR(0x12)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F') PORT_CHAR(0x06)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V') PORT_CHAR(0x16)
	PORT_START("A3")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5 %") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T') PORT_CHAR(0x14)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G') PORT_CHAR(0x07)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B') PORT_CHAR(0x02)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6 &") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z') PORT_CHAR(0x1a)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H') PORT_CHAR(0x08)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N') PORT_CHAR(0x0e)
	PORT_START("A4")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7 /") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('/')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U') PORT_CHAR(0x15)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J') PORT_CHAR(0x0a)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M') PORT_CHAR(0x0d)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8 (") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I') PORT_CHAR(0x09)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K') PORT_CHAR(0x0b)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(", ;") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR(';')
	PORT_START("A5")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9 )") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O') PORT_CHAR(0x0f)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L') PORT_CHAR(0x0c)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(". :") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR(':')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('=') PORT_CHAR('0')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P') PORT_CHAR(0x10)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("| \\") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('|') PORT_CHAR('\\') PORT_CHAR(0x1c)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("- _") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')
	PORT_START("A6")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("~ ?") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('~') PORT_CHAR('?') PORT_CHAR(0x1e)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("} ]") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('}') PORT_CHAR(']') PORT_CHAR(0x1d)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("{ [") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('{') PORT_CHAR('[') PORT_CHAR(0x1b)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("<") PORT_CODE(KEYCODE_0_PAD) PORT_CHAR('<')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("+ *") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('+') PORT_CHAR('*')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("# '") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('#') PORT_CHAR('\'') PORT_CHAR(0x1f)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("^ ~") PORT_CODE(KEYCODE_COLON) PORT_CHAR('^') PORT_CHAR('~') PORT_CHAR(0x1e)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB) PORT_CHAR(0x09)
	PORT_START("A7")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(">") PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR('>')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CR") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(0x0d)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("04") PORT_CODE(KEYCODE_TILDE) PORT_CHAR(0x04)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("01") PORT_CODE(KEYCODE_TILDE) PORT_CHAR(0x01)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ') PORT_CHAR(' ') PORT_CHAR('`')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Escape") PORT_CODE(KEYCODE_ESC) PORT_CHAR(0x1b) PORT_CHAR(0x1b) PORT_CHAR(0x1f)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("13") PORT_CODE(KEYCODE_TILDE) PORT_CHAR(0x13)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("06") PORT_CODE(KEYCODE_TILDE) PORT_CHAR(0x06)
	PORT_START("A8")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_RSHIFT)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT)
	PORT_START("A9")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CLS") PORT_CODE(KEYCODE_END) PORT_CHAR(0x0c)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL) PORT_CHAR(0x7f) PORT_CHAR(0x1e) PORT_CHAR(0x1f)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F01") PORT_CODE(KEYCODE_TILDE) PORT_CHAR(0x05)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F02") PORT_CODE(KEYCODE_TILDE) PORT_CHAR(0x18)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F03") PORT_CODE(KEYCODE_TILDE) PORT_CHAR(0x13)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F04") PORT_CODE(KEYCODE_TILDE) PORT_CHAR(0x04)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F06") PORT_CODE(KEYCODE_TILDE) PORT_CHAR(0x03)
	PORT_START("A10")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F07") PORT_CODE(KEYCODE_TILDE) PORT_CHAR(0x1a)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F08") PORT_CODE(KEYCODE_TILDE) PORT_CHAR(0x17)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F09") PORT_CODE(KEYCODE_TILDE) PORT_CHAR(0x07)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F10") PORT_CODE(KEYCODE_TILDE) PORT_CHAR(0x14)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F11") PORT_CODE(KEYCODE_TILDE) PORT_CHAR(0x19)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F12") PORT_CODE(KEYCODE_TILDE) PORT_CHAR(0x02)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F13") PORT_CODE(KEYCODE_TILDE) PORT_CHAR(0x0b)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F14") PORT_CODE(KEYCODE_TILDE) PORT_CHAR(0x11)
	PORT_START("A11")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F05") PORT_CODE(KEYCODE_TILDE) PORT_CHAR(0x12)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP) PORT_CHAR(0x1a)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(0x0a)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(0x15)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(0x08)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E95") PORT_CODE(KEYCODE_TILDE) PORT_CHAR(0xff)
	PORT_START("A12")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Reset") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("NMI") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor k7659_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( k7659_keyboard );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  k7659_keyboard_device - constructor
//-------------------------------------------------

k7659_keyboard_device::k7659_keyboard_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, K7659_KEYBOARD, "K7659 Keyboard", tag, owner, clock, "k7659kb", __FILE__)
{}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k7659_keyboard_device::device_start()
{
	m_timer = timer_alloc();
	// state saving
	//save_item(NAME(m_y));
	//save_item(NAME(m_so));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k7659_keyboard_device::device_reset()
{
	m_lookup = 0;
	m_p_rom = memregion("k7659")->base();
	m_timer->adjust(attotime::from_hz(200), 0, attotime::from_hz(200));
}

void k7659_keyboard_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	UINT8 retVal = 0;//m_lookup;
	UINT8 a1 = ioport("A1")->read();
	UINT8 a2 = ioport("A2")->read();
	UINT8 a3 = ioport("A3")->read();
	UINT8 a4 = ioport("A4")->read();
	UINT8 a5 = ioport("A5")->read();
	UINT8 a6 = ioport("A6")->read();
	UINT8 a7 = ioport("A7")->read();
	UINT8 a8 = ioport("A8")->read();
	UINT8 a9 = ioport("A9")->read();
	UINT8 a10 = ioport("A10")->read();
	UINT8 a11 = ioport("A11")->read();
	UINT8 a12 = ioport("A12")->read();
	UINT16 code = 0;
	if (a1)
		code = 0x10 + key_pos(a1);
	else if (a2)
		code = 0x20 + key_pos(a2);
	else if (a3)
		code = 0x30 + key_pos(a3);
	else if (a4)
		code = 0x40 + key_pos(a4);
	else if (a5)
		code = 0x50 + key_pos(a5);
	else if (a6)
		code = 0x60 + key_pos(a6);
	else if (a7)
		code = 0x70 + key_pos(a7);
	else if (a9)
		code = 0x80 + key_pos(a9);
	else if (a10)
		code = 0x90 + key_pos(a10);
	else if (a11)
		code = 0xA0 + key_pos(a11);

	if (code)
	{
		if (BIT(a8,6) || BIT(a8,7))
			code |= 0x100;
		else
		if (BIT(a12,6))
			code |= 0x200;

		m_lookup = m_p_rom[code];
		retVal = m_lookup | 0x80;
	}
	m_key = retVal;
}

UINT8 k7659_keyboard_device::key_pos(UINT8 val)
{
	if (BIT(val,0)) return 1;
	if (BIT(val,1)) return 2;
	if (BIT(val,2)) return 3;
	if (BIT(val,3)) return 4;
	if (BIT(val,4)) return 5;
	if (BIT(val,5)) return 6;
	if (BIT(val,6)) return 7;
	if (BIT(val,7)) return 8;
	return 0;
}

READ8_MEMBER(k7659_keyboard_device::read)
{
	return m_key;
}
