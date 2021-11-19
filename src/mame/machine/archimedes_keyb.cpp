// license:LGPL-2.1+
// copyright-holders:Angelo Salese, Sandro Ronco
/**********************************************************************

    Acorn Archimedes keyboard

*********************************************************************/

#include "emu.h"
#include "archimedes_keyb.h"


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

DEFINE_DEVICE_TYPE(ARCHIMEDES_KEYBOARD, archimedes_keyboard_device, "archimedes_keyboard", "Acorn Archimedes Keyboard")

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( archimedes_keyboard )
	PORT_START("ROW.0")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)      PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)      PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)      PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_NUMLOCK)    PORT_CHAR(UCHAR_MAMEKEY(NUMLOCK))
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGUP)       PORT_CHAR(UCHAR_MAMEKEY(PGUP))
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PAUSE)      PORT_CHAR(UCHAR_MAMEKEY(PAUSE)) PORT_NAME("Break")
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SCRLOCK)    PORT_CHAR(UCHAR_MAMEKEY(SCRLOCK))
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_OTHER)    // Keyboard ID bit 4
	PORT_BIT(0xfe00, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW.1")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)        PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('`')  PORT_CHAR('~')
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)          PORT_CHAR('z')  PORT_CHAR('Z')
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)        PORT_CHAR('\t')
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)          PORT_CHAR('1')  PORT_CHAR('!')
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)          PORT_CHAR('q')  PORT_CHAR('Q')
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)          PORT_CHAR('a')  PORT_CHAR('A')
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_OTHER)    // Keyboard ID bit 5
	PORT_BIT(0xfe00, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW.2")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK)   PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)          PORT_CHAR('x')  PORT_CHAR('X')
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)          PORT_CHAR('s')  PORT_CHAR('S')
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)         PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)         PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)          PORT_CHAR('2')  PORT_CHAR('@')
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)          PORT_CHAR('w')  PORT_CHAR('W')
	PORT_BIT(0xff00, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW.3")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)          PORT_CHAR('c')  PORT_CHAR('C')
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)          PORT_CHAR('d')  PORT_CHAR('D')
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)         PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)         PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)          PORT_CHAR('3')  PORT_CHAR('#')
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)          PORT_CHAR('e')  PORT_CHAR('E')
	PORT_BIT(0xff00, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW.4")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)          PORT_CHAR('b')  PORT_CHAR('B')
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)          PORT_CHAR('v')  PORT_CHAR('V')
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)          PORT_CHAR('g')  PORT_CHAR('G')
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)          PORT_CHAR('f')  PORT_CHAR('F')
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)          PORT_CHAR('4')  PORT_CHAR('$')
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)          PORT_CHAR('5')  PORT_CHAR('%')
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)          PORT_CHAR('r')  PORT_CHAR('R')
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)          PORT_CHAR('t')  PORT_CHAR('T')
	PORT_BIT(0xff00, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW.5")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)          PORT_CHAR('n')  PORT_CHAR('N')
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)          PORT_CHAR('m')  PORT_CHAR('M')
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)          PORT_CHAR('h')  PORT_CHAR('H')
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)          PORT_CHAR('j')  PORT_CHAR('J')
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)          PORT_CHAR('7')  PORT_CHAR('&')
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)          PORT_CHAR('6')  PORT_CHAR('^')
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)          PORT_CHAR('u')  PORT_CHAR('U')
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)          PORT_CHAR('y')  PORT_CHAR('Y')
	PORT_BIT(0xff00, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW.6")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)       PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',')  PORT_CHAR('<')
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)          PORT_CHAR('k')  PORT_CHAR('K')
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('8')  PORT_CHAR('*')
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)         PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)          PORT_CHAR('i')  PORT_CHAR('I')
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13) PORT_NAME("Return")
	PORT_BIT(0xff00, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW.7")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)      PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)       PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)    PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0xff00, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW.8")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD)  PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD)   PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD)  PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD)                               PORT_CHAR('#')
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)        PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_END)        PORT_CHAR(UCHAR_MAMEKEY(END)) PORT_NAME("Copy")
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGDN)       PORT_CHAR(UCHAR_MAMEKEY(PGDN))
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_OTHER)   // Keyboard ID bit 0
	PORT_BIT(0xfe00, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW.9")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)      PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)      PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)      PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK)   PORT_CHAR('*')
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR(0xa3) PORT_CHAR(0xa4)
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=')  PORT_CHAR('+')
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(UCHAR_MAMEKEY(BACKSPACE)) PORT_NAME("Back Space")
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_INSERT)     PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_OTHER)    // Keyboard ID bit 1
	PORT_BIT(0xfe00, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW.10")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/')  PORT_CHAR('?')
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.')  PORT_CHAR('>')
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)          PORT_CHAR('l')  PORT_CHAR('L')
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_CHAR('9')  PORT_CHAR('(')
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)         PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)          PORT_CHAR('o')  PORT_CHAR('O')
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']')  PORT_CHAR('}')
	PORT_BIT(0xff00, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW.11")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)      PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)      PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)      PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD)  PORT_CHAR('/')
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-')  PORT_CHAR('_')
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10)        PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F11)        PORT_CHAR(UCHAR_MAMEKEY(F11))
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F12)        PORT_CHAR(UCHAR_MAMEKEY(F12))
	PORT_BIT(0xff00, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW.12")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';')  PORT_CHAR(':')
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)          PORT_CHAR('p')  PORT_CHAR('P')
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[')  PORT_CHAR('{')
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)          PORT_CHAR('0')  PORT_CHAR(')')
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9)         PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7)         PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8)         PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0xff00, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW.13")
	PORT_BIT(0x00ff, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT)       PORT_CHAR(UCHAR_MAMEKEY(LALT))
	PORT_BIT(0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT(0x0400, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x0800, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME)       PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PRTSCR)     PORT_CHAR(UCHAR_MAMEKEY(PRTSCR)) PORT_NAME("Print")
	PORT_BIT(0x2000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT)     PORT_CHAR(UCHAR_MAMEKEY(RSHIFT))
	PORT_BIT(0x4000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(RCONTROL))
	PORT_BIT(0x8000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RALT)       PORT_CHAR(UCHAR_MAMEKEY(RALT))

	PORT_START("ROW.14")
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_OTHER)    // Keyboard ID bit 2
	PORT_BIT(0xfeff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW.15")
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_OTHER)    // Keyboard ID bit 3
	PORT_BIT(0xfeff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("MOUSE.0")
	PORT_BIT(0xffff, IP_ACTIVE_HIGH, IPT_MOUSE_X) PORT_SENSITIVITY(50) PORT_KEYDELTA(0) PORT_RESET PORT_CODE(MOUSECODE_X) PORT_CHANGED_MEMBER(DEVICE_SELF, archimedes_keyboard_device, update_mouse_input, 0)

	PORT_START("MOUSE.1")
	PORT_BIT(0xffff, IP_ACTIVE_HIGH, IPT_MOUSE_Y) PORT_SENSITIVITY(50) PORT_KEYDELTA(0) PORT_RESET PORT_CODE(MOUSECODE_Y) PORT_CHANGED_MEMBER(DEVICE_SELF, archimedes_keyboard_device, update_mouse_input, 1) PORT_REVERSE

	PORT_START("MOUSE.2")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(MOUSECODE_BUTTON1) PORT_NAME("Mouse Left")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(MOUSECODE_BUTTON3) PORT_NAME("Mouse Center")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(MOUSECODE_BUTTON2) PORT_NAME("Mouse Right")
INPUT_PORTS_END


ioport_constructor archimedes_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( archimedes_keyboard );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  archimedes_keyboard_device - constructor
//-------------------------------------------------

archimedes_keyboard_device::archimedes_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ARCHIMEDES_KEYBOARD, tag, owner, clock)
	, device_serial_interface(mconfig, *this)
	, m_mcu(*this, "mcu")
	, m_kout(*this)
	, m_keyboard(*this, "ROW.%u", 0U)
	, m_mouse(*this, "MOUSE.%u", 0U)
	, m_leds(*this, "keyb_led.%u", 0U)
{
}


void archimedes_keyboard_device::device_resolve_objects()
{
	m_kout.resolve_safe();
	m_leds.resolve();
}

void archimedes_keyboard_device::device_start()
{
	// KART interface
	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_2);
	set_rate(31250);

	m_mouse_timer = timer_alloc();

	save_item(NAME(m_mouse_x));
	save_item(NAME(m_mouse_y));
	save_item(NAME(m_mouse_phase));
	save_item(NAME(m_mouse_xref));
	save_item(NAME(m_mouse_xdir));
	save_item(NAME(m_mouse_yref));
	save_item(NAME(m_mouse_ydir));
	save_item(NAME(m_mux));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void archimedes_keyboard_device::device_reset()
{
	m_mouse_x     = 0;
	m_mouse_y     = 0;
	m_mouse_phase = 1;
	m_mouse_xref  = 1;
	m_mouse_xdir  = 1;
	m_mouse_yref  = 1;
	m_mouse_ydir  = 1;
	m_mux         = 0;

	transmit_register_reset();
	receive_register_reset();

	m_mouse_timer->adjust(attotime::from_hz(1000 * 4), 0, attotime::from_hz(1000 * 4));
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void archimedes_keyboard_device::device_add_mconfig(machine_config &config)
{
	I8051(config, m_mcu, 12_MHz_XTAL);  // Philips 8051AH-2
	m_mcu->port_in_cb<0>().set([this]() { return m_keyboard[m_mux]->read() & 0xff; });
	m_mcu->port_in_cb<1>().set([this]() { return (m_keyboard[m_mux]->read() >> 8) & 0xff; });
	m_mcu->port_in_cb<2>().set(FUNC(archimedes_keyboard_device::mouse_r));
	m_mcu->port_in_cb<3>().set([this]() { return m_mouse[2]->read() & 0xff; });
	m_mcu->port_out_cb<2>().set([this](uint8_t data) { m_mux = data & 0x0f; });
	m_mcu->port_out_cb<3>().set(FUNC(archimedes_keyboard_device::leds_w));
	m_mcu->serial_rx_cb().set([this]() { return get_received_char(); });
	m_mcu->serial_tx_cb().set(FUNC(archimedes_keyboard_device::tx_w));
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( archimedes_keyboard )
	ROM_REGION( 0x1000, "mcu", 0 )
	ROM_LOAD("acorn_0280,022-01_philips_8051ah-2.bin", 0x0000, 0x1000, CRC(95095d38) SHA1(b9fd0000d77987f76fd48348fe2c6988818f40cb))
ROM_END


const tiny_rom_entry *archimedes_keyboard_device::device_rom_region() const
{
	return ROM_NAME( archimedes_keyboard );
}

void archimedes_keyboard_device::tra_complete()
{
}

void archimedes_keyboard_device::rcv_complete()
{
	receive_register_extract();
	m_mcu->set_input_line(MCS51_RX_LINE, ASSERT_LINE);
	m_mcu->set_input_line(MCS51_RX_LINE, CLEAR_LINE);
}

void archimedes_keyboard_device::tra_callback()
{
	m_kout(transmit_register_get_data_bit());
}

void archimedes_keyboard_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	//  Mouse quadrature signals
	//  Increase      Decrease
	//  REF  DIR      REF  DIR
	//   1    1        1    1
	//   1    0        0    1
	//   0    0        0    0
	//   0    1        1    0

	switch (m_mouse_phase)
	{
	case 0:
		m_mouse_xdir = m_mouse_xref = 1;
		m_mouse_ydir = m_mouse_yref = 1;
		break;
	case 1:
		m_mouse_xref = m_mouse_x < 0 ? 0 : 1;
		m_mouse_xdir = m_mouse_x > 0 ? 0 : 1;
		m_mouse_yref = m_mouse_y < 0 ? 0 : 1;
		m_mouse_ydir = m_mouse_y > 0 ? 0 : 1;
		break;
	case 2:
		m_mouse_xdir = m_mouse_xref = m_mouse_x != 0 ? 0 : 1;
		m_mouse_ydir = m_mouse_yref = m_mouse_y != 0 ? 0 : 1;
		break;
	case 3:
		m_mouse_xref = m_mouse_x > 0 ? 0 : 1;
		m_mouse_xdir = m_mouse_x < 0 ? 0 : 1;
		m_mouse_yref = m_mouse_y > 0 ? 0 : 1;
		m_mouse_ydir = m_mouse_y < 0 ? 0 : 1;
		if (m_mouse_x > 0)    m_mouse_x--;
		if (m_mouse_x < 0)    m_mouse_x++;
		if (m_mouse_y > 0)    m_mouse_y--;
		if (m_mouse_y < 0)    m_mouse_y++;
		break;
	}

	m_mouse_phase = (m_mouse_phase + 1) & 3;
}

void archimedes_keyboard_device::leds_w(uint8_t data)
{
	// Keyboard LEDs
	// --- -x-- Caps Lock
	// --- x--- Scroll Lock
	// --x ---- Num Lock

	for (int i = 0; i < 3; i++)
		m_leds[i] = BIT(data, 2 + i);
}

uint8_t archimedes_keyboard_device::mouse_r()
{
	return (m_mouse_xref << 4) | (m_mouse_xdir << 5) | (m_mouse_yref << 6) | (m_mouse_ydir << 7);
}

void archimedes_keyboard_device::tx_w(uint8_t data)
{
	if (is_transmit_register_empty())
		transmit_register_setup(data);
}

INPUT_CHANGED_MEMBER(archimedes_keyboard_device::update_mouse_input)
{
	if (param == 0)
		m_mouse_x = (int16_t)m_mouse[0]->read();

	if (param == 1)
		m_mouse_y = (int16_t)m_mouse[1]->read();
}
