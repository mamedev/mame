// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    IBM Model F PC/AT 84-key / 3270PC 122-key keyboard emulation

*********************************************************************/

/*

    TODO:

    - cpu LC timing
    - 3270PC keys

*/

/*

    Part No     Layout
    -------------------
    6450225     UK 84-key
    6110344     UK 122-key

*/

#include "emu.h"
#include "pcat84.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define I8048_TAG       "m5"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(PC_KBD_IBM_PC_AT_84,   ibm_pc_at_84_keyboard_device,   "kb_pcat84", "IBM PC/AT Keyboard")
DEFINE_DEVICE_TYPE(PC_KBD_IBM_3270PC_122, ibm_3270pc_122_keyboard_device, "kb_3270pc", "IBM 3270PC Keyboard")


//-------------------------------------------------
//  ROM( ibm_pc_at_84_keyboard )
//-------------------------------------------------

ROM_START( ibm_pc_at_84_keyboard )
	ROM_REGION( 0x400, I8048_TAG, 0 )
	/*
	Keyboard Part No 6450225

	PH 1503099
	D 878154
	8441 D H
	*/
	ROM_LOAD( "1503099.m5", 0x000, 0x400, CRC(1e921f37) SHA1(5f722bdb3b57f5a532c02a5c3f78f30d785796f2) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *ibm_pc_at_84_keyboard_device::device_rom_region() const
{
	return ROM_NAME( ibm_pc_at_84_keyboard );
}


//-------------------------------------------------
//  ROM( ibm_3270pc_122_keyboard )
//-------------------------------------------------

ROM_START( ibm_3270pc_122_keyboard )
	ROM_REGION( 0x400, I8048_TAG, 0 )
	/*
	Keyboard Part No 6110344

	PH 1385001
	D
	8512 D H
	*/
	ROM_LOAD( "1385001.m5", 0x000, 0x400, CRC(c19767e9) SHA1(a3701e4617383a4de0fd5e2e86c4b74beaf94a7b) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *ibm_3270pc_122_keyboard_device::device_rom_region() const
{
	return ROM_NAME( ibm_3270pc_122_keyboard );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void ibm_pc_at_84_keyboard_device::device_add_mconfig(machine_config &config)
{
	I8048(config, m_maincpu, 5364000);
	m_maincpu->bus_out_cb().set(FUNC(ibm_pc_at_84_keyboard_device::bus_w));
	m_maincpu->p1_in_cb().set(FUNC(ibm_pc_at_84_keyboard_device::p1_r));
	m_maincpu->p1_out_cb().set(FUNC(ibm_pc_at_84_keyboard_device::p1_w));
	m_maincpu->p2_in_cb().set(FUNC(ibm_pc_at_84_keyboard_device::p2_r));
	m_maincpu->p2_out_cb().set(FUNC(ibm_pc_at_84_keyboard_device::p2_w));
	m_maincpu->t0_in_cb().set(FUNC(ibm_pc_at_84_keyboard_device::t0_r));
	m_maincpu->t1_in_cb().set(FUNC(ibm_pc_at_84_keyboard_device::t1_r));
}


//-------------------------------------------------
//  INPUT_PORTS( ibm_pc_at_84_keyboard )
//-------------------------------------------------

INPUT_PORTS_START( ibm_pc_at_84_keyboard )
	PORT_START("DR00")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F9) PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DR01")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F10) PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TAB) PORT_CHAR(UCHAR_MAMEKEY(TAB))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DR02")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(LALT))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DR03")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DR04")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DR05")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DR06")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DR07")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DR08")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DR09")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DR10")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("DR11")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DR12")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DR13")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 1 End") PORT_CODE(KEYCODE_1_PAD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 4 " UTF8_LEFT) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 7 Home") PORT_CODE(KEYCODE_7_PAD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DR14")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 0 Ins") PORT_CODE(KEYCODE_0_PAD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad . Del") PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 2 " UTF8_DOWN) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5_PAD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 6 " UTF8_RIGHT) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 8 " UTF8_UP) PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_NUMLOCK) PORT_CHAR(UCHAR_MAMEKEY(NUMLOCK))

	PORT_START("DR15")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 3 PgDn") PORT_CODE(KEYCODE_3_PAD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad * PrtSc") PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad 9 PgUp") PORT_CODE(KEYCODE_9_PAD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Scroll Lock Break") PORT_CODE(KEYCODE_SCRLOCK) PORT_CHAR(UCHAR_MAMEKEY(SCRLOCK))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Sys Req")
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor ibm_pc_at_84_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( ibm_pc_at_84_keyboard );
}


//-------------------------------------------------
//  INPUT_PORTS( ibm_3270pc_122_keyboard )
//-------------------------------------------------

INPUT_PORTS_START( ibm_3270pc_122_keyboard )
	PORT_INCLUDE(ibm_pc_at_84_keyboard)

	PORT_START("KBDIDA")
	PORT_DIPUNUSED( 0x01, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x02, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x04, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )

	PORT_START("KBDIDB")
	PORT_DIPUNUSED( 0x01, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x02, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x04, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor ibm_3270pc_122_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( ibm_3270pc_122_keyboard );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ibm_pc_at_84_keyboard_device - constructor
//-------------------------------------------------

ibm_pc_at_84_keyboard_device::ibm_pc_at_84_keyboard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_pc_kbd_interface(mconfig, *this),
	m_maincpu(*this, I8048_TAG),
	m_dr(*this, "DR%02u", 0),
	m_kbdida(*this, "KBDIDA"),
	m_kbdidb(*this, "KBDIDB"),
	m_leds(*this, "led%u", 0U),
	m_db(0),
	m_cnt(0),
	m_sense(0),
	m_t1(1)
{
}

ibm_pc_at_84_keyboard_device::ibm_pc_at_84_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ibm_pc_at_84_keyboard_device(mconfig, PC_KBD_IBM_PC_AT_84, tag, owner, clock)
{
}

ibm_3270pc_122_keyboard_device::ibm_3270pc_122_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ibm_pc_at_84_keyboard_device(mconfig, PC_KBD_IBM_3270PC_122, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ibm_pc_at_84_keyboard_device::device_start()
{
	set_pc_kbdc_device();

	m_leds.resolve();

	// state saving
	save_item(NAME(m_db));
	save_item(NAME(m_cnt));
	save_item(NAME(m_sense));
	save_item(NAME(m_t1));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ibm_pc_at_84_keyboard_device::device_reset()
{
	m_maincpu->reset();
}


//-------------------------------------------------
//  bus_w -
//-------------------------------------------------

WRITE8_MEMBER( ibm_pc_at_84_keyboard_device::bus_w )
{
	/*

	    bit     description

	    0       SENSE 0
	    1       SENSE 1
	    2       SENSE 2
	    3       CNT 0
	    4       CNT 1
	    5       CNT 2
	    6       CNT 3
	    7       CNT G

	*/

	m_db = data;

	if (!BIT(data, 7))
	{
		m_cnt = (data >> 3) & 0x0f;
	}
}


//-------------------------------------------------
//  p1_r -
//-------------------------------------------------

READ8_MEMBER( ibm_pc_at_84_keyboard_device::p1_r )
{
	/*

	    bit     description

	    0
	    1
	    2       KBDID A1
	    3       KBDID A2
	    4       KBDID A3
	    5       KBDID A4
	    6       KBDID A5
	    7       KBDID A6

	*/

	uint8_t data = 0;

	data |= m_kbdida->read() << 2;

	return data;
}


//-------------------------------------------------
//  p1_w -
//-------------------------------------------------

WRITE8_MEMBER( ibm_pc_at_84_keyboard_device::p1_w )
{
	/*

	    bit     description

	    0       SENSE G
	    1       T1
	    2
	    3
	    4
	    5
	    6
	    7

	*/

	if (!BIT(data, 0))
	{
		m_sense = m_db & 0x07;
	}

	m_t1 = BIT(data, 1);
}


//-------------------------------------------------
//  p2_r -
//-------------------------------------------------

READ8_MEMBER( ibm_pc_at_84_keyboard_device::p2_r )
{
	/*

	    bit     description

	    0       KBDID B1
	    1       KBDID B2
	    2       KBDID B3
	    3       KBDID B4
	    4       KBDID B5
	    5       KBDID B6
	    6
	    7

	*/

	uint8_t data = 0xc0;

	data |= m_kbdidb->read();

	return data;
}


//-------------------------------------------------
//  p2_w -
//-------------------------------------------------

WRITE8_MEMBER( ibm_pc_at_84_keyboard_device::p2_w )
{
	/*

	    bit     description

	    0       SCROLL LED
	    1       NUM LED
	    2       CAPS LED
	    3
	    4
	    5
	    6       CLOCK
	    7       DATA

	*/

	m_leds[LED_SCROLL] = BIT(data, 0);
	m_leds[LED_NUM] = BIT(data, 1);
	m_leds[LED_CAPS] = BIT(data, 2);

	m_pc_kbdc->data_write_from_kb(!BIT(data, 7));
	m_pc_kbdc->clock_write_from_kb(!BIT(data, 6));
}


//-------------------------------------------------
//  t0_r -
//-------------------------------------------------

READ_LINE_MEMBER( ibm_pc_at_84_keyboard_device::t0_r )
{
	return !data_signal();
}


//-------------------------------------------------
//  t1_r -
//-------------------------------------------------

READ_LINE_MEMBER( ibm_pc_at_84_keyboard_device::t1_r )
{
	return key_depressed();
}


//-------------------------------------------------
//  key_depressed -
//-------------------------------------------------

int ibm_pc_at_84_keyboard_device::key_depressed()
{
	uint8_t data = m_dr[m_cnt]->read();

	return m_t1 && BIT(data, m_sense);
}
