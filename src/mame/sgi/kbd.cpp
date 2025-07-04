// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, Patrick Mackinlay

#include "emu.h"
#include "kbd.h"

#include "sound/beep.h"
#include "speaker.h"

DEFINE_DEVICE_TYPE(SGI_KBD_PORT, sgi_kbd_port_device, "sgi_kbd_port", "SGI Keyboard Port")
DEFINE_DEVICE_TYPE(SGI_KBD, sgi_kbd_device, "sgi_kbd", "SGI Keyboard")

/*
 * An emulation of a keyboard compatible with the Silicon Graphics 4D series
 * (Professional IRIS, Personal IRIS, PowerSeries, Crimson), Indigo, and Onyx
 * systems. The physical interface may be DB-15, DB-9 or DIN-6 depending on the
 * system, however these are all logically and electrically compatible. A mouse
 * port is also present on the keyboard, but its signals are routed directly
 * to/from the host. Later SGI machines use a PS/2 compatible keyboard.
 *
 * The emulated keyboard is a Key Tronic design with an Intel 8031 MCU and an
 * Exar 22-00958-000 capacitive sensing chip. The 8-position DIP switch is not
 * used by the keyboard itself, but is read and returned as configuration data
 * to the host, which uses it to identify the keyboard layout. The keyboard
 * protocol is RS-423 asynchronous serial at 600 baud, with 1 start, 8 data,
 * odd parity and 1 stop bit. Modifier keys and LEDs are managed by the host.
 *
 *  Part                Function
 *  ----                --------
 *  P8031AH             MCS-51 series microcontroller
 *  D2764A              8KiB eprom
 *  SN74LS373N          octal d-type transparent latch
 *  SN74LS374N          octal d-type edge triggered flip-flops
 *  22-00958-000C5145B  capacitive sensing chip
 *  DS26LS32ACN         quad differential line receivers
 *  DS3692N             differential line drivers
 *  NE555N              beep timer?
 *  11059 KSSOH         11.0592 MHz crystal
 *  8-pos DIP           keyboard layout configuration
 *
 * Sources:
 *  - IRIX keyboard(7) manual page
 *
 * TODO:
 *  - port 1 outputs and beeper/keyclick
 *
 */

sgi_kbd_port_device::sgi_kbd_port_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SGI_KBD_PORT, tag, owner, clock)
	, device_single_card_slot_interface<device_sgi_kbd_port_interface>(mconfig, *this)
	, m_rxd_handler(*this)
	, m_kbd(nullptr)
{
}

sgi_kbd_port_device::~sgi_kbd_port_device()
{
}

void sgi_kbd_port_device::device_config_complete()
{
	m_kbd = get_card_device();
}

void sgi_kbd_port_device::device_start()
{
	m_rxd_handler(1);
}

void sgi_kbd_port_device::write_txd(int state)
{
	if (m_kbd)
		m_kbd->write_txd(state);
}

device_sgi_kbd_port_interface::device_sgi_kbd_port_interface(machine_config const &mconfig, device_t &device)
	: device_interface(device, "sgi_kbd")
	, m_port(dynamic_cast<sgi_kbd_port_device *>(device.owner()))
{
}

device_sgi_kbd_port_interface::~device_sgi_kbd_port_interface()
{
}

sgi_kbd_device::sgi_kbd_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SGI_KBD, tag, owner, clock)
	, device_sgi_kbd_port_interface(mconfig, *this)
	, m_mcu(*this, "mcu")
	, m_matrix(*this, "col.%x", 0U)
	, m_led(*this, "led.%u", 0U)
	, m_col(0)
	, m_row(0)
	, m_p3(0)
{
}

/*
 * MCU port 1 WIP
 * --------------
 * p1.0  o  scan matrix
 * p1.1  o  ?
 * p1.2
 * p1.3  o  beeper?
 * p1.4
 * p1.5
 * p1.6
 * p1.7
*/

enum p3_mask : u8
{
	P3_RXD  = 0x01,
	P3_TXD  = 0x02,
	P3_INT0 = 0x04,
	P3_INT1 = 0x08,
	P3_T0   = 0x10,
	P3_T1   = 0x20,
	P3_WR   = 0x40,
	P3_RD   = 0x80,
};

void sgi_kbd_device::device_add_mconfig(machine_config &config)
{
	speaker_device &speaker(SPEAKER(config, "speaker"));
	speaker.front_center();

	beep_device &beeper(BEEP(config, "beeper", 480));
	beeper.add_route(ALL_OUTPUTS, speaker, 0.25);

	I8031(config, m_mcu, 11.0592_MHz_XTAL);
	m_mcu->set_addrmap(AS_PROGRAM, &sgi_kbd_device::map_mem);
	m_mcu->set_addrmap(AS_IO, &sgi_kbd_device::map_pio);
	m_mcu->port_out_cb<1>().append(FUNC(sgi_kbd_device::scan_matrix)).bit(0);
	m_mcu->port_out_cb<1>().append(beeper, FUNC(beep_device::set_state)).bit(3);
	m_mcu->port_out_cb<3>().append(FUNC(sgi_kbd_device::write_rxd)).bit(1);
	m_mcu->port_in_cb<3>().set([this]() { return m_p3; });
}

void sgi_kbd_device::map_mem(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("mcu", 0);
}

void sgi_kbd_device::map_pio(address_map &map)
{
	map(0x2000, 0x2000).select(0x0f00).mirror(0x00ff).lr8(
		[this](offs_t offset)
		{
			m_col = offset >> 8;
			m_row = 0;
			return 0;
		}, "set_column");

	map(0xff40, 0xff40).w(FUNC(sgi_kbd_device::led_w));
}

void sgi_kbd_device::device_start()
{
	save_item(NAME(m_col));
	save_item(NAME(m_row));
	save_item(NAME(m_p3));

	m_led.resolve();
}

void sgi_kbd_device::scan_matrix(int state)
{
	if (!state)
	{
		if (BIT(m_matrix[m_col]->read(), m_row & 7))
			m_p3 &= ~P3_T0;
		else
			m_p3 |= P3_T0;

		m_row++;
	}
}

void sgi_kbd_device::write_txd(int state)
{
	if (state)
		m_p3 |= P3_RXD;
	else
		m_p3 &= ~P3_RXD;
}

void sgi_kbd_device::led_w(u8 data)
{
	/*
	 * bit  output
	 *  0   L1
	 *  1   Num Lock
	 *  2   L2
	 *  3   Caps Lock
	 *  4   L3
	 *  5   Scroll Lock
	 *  6   L4
	 *  7   not used
	 */
	for (unsigned i = 0; i < 7; i++)
		m_led[i] = BIT(data, i);
}

namespace {

INPUT_PORTS_START(sgi_kbd)
	PORT_START("col.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_4)            PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_5)            PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_T)            PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_R)            PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_G)            PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_F)            PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_B)            PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_V)            PORT_CHAR('v') PORT_CHAR('V')

	PORT_START("col.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F1")                PORT_CODE(KEYCODE_F1)           PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F3")                PORT_CODE(KEYCODE_F3)           PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F5")                PORT_CODE(KEYCODE_F5)           PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F7")                PORT_CODE(KEYCODE_F7)           PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F9")                PORT_CODE(KEYCODE_F9)           PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F11")               PORT_CODE(KEYCODE_F11)          PORT_CHAR(UCHAR_MAMEKEY(F11))
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Print Screen")      PORT_CODE(KEYCODE_PRTSCR)       PORT_CHAR(UCHAR_MAMEKEY(PRTSCR))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Pause")             PORT_CODE(KEYCODE_PAUSE)

	PORT_START("col.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_2)            PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_3)            PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_E)            PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_W)            PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_D)            PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_S)            PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_C)            PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_X)            PORT_CHAR('x') PORT_CHAR('X')

	PORT_START("col.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_TILDE)        PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_1)            PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_Q)            PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Tab")               PORT_CODE(KEYCODE_TAB)          PORT_CHAR(9)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_A)            PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Caps Lock")         PORT_CODE(KEYCODE_CAPSLOCK)     PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_Z)            PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L Shift")           PORT_CODE(KEYCODE_LSHIFT)       PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("col.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Esc")               PORT_CODE(KEYCODE_ESC)          PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F2")                PORT_CODE(KEYCODE_F2)           PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F4")                PORT_CODE(KEYCODE_F4)           PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F6")                PORT_CODE(KEYCODE_F6)           PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F8")                PORT_CODE(KEYCODE_F8)           PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F10")               PORT_CODE(KEYCODE_F10)          PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F12")               PORT_CODE(KEYCODE_F12)          PORT_CHAR(UCHAR_MAMEKEY(F12))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Scroll Lock")       PORT_CODE(KEYCODE_SCRLOCK)      PORT_CHAR(UCHAR_MAMEKEY(SCRLOCK))

	PORT_START("col.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_6)            PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_7)            PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_U)            PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_Y)            PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_J)            PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_H)            PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_M)            PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_N)            PORT_CHAR('n') PORT_CHAR('N')

	PORT_START("col.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_8)            PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_9)            PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_O)            PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_I)            PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_L)            PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_K)            PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_STOP)         PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_COMMA)        PORT_CHAR(',') PORT_CHAR('<')

	PORT_START("col.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_0)            PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_MINUS)        PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_OPENBRACE)    PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_P)            PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_QUOTE)        PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_COLON)        PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R Shift")           PORT_CODE(KEYCODE_RSHIFT)       PORT_CHAR(UCHAR_MAMEKEY(RSHIFT))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_SLASH)        PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("col.8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_EQUALS)       PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Backspace")         PORT_CODE(KEYCODE_BACKSPACE)    PORT_CHAR(8)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_BACKSLASH)    PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                PORT_CODE(KEYCODE_CLOSEBRACE)   PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Enter")             PORT_CODE(KEYCODE_ENTER)        PORT_CHAR(13)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("col.9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Home")              PORT_CODE(KEYCODE_HOME)         PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Insert")            PORT_CODE(KEYCODE_INSERT)       PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Delete")            PORT_CODE(KEYCODE_DEL)          PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("End")               PORT_CODE(KEYCODE_END)          PORT_CHAR(UCHAR_MAMEKEY(END))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Page Up")           PORT_CODE(KEYCODE_PGUP)         PORT_CHAR(UCHAR_MAMEKEY(PGUP))
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Page Down")         PORT_CODE(KEYCODE_PGDN)         PORT_CHAR(UCHAR_MAMEKEY(PGDN))
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 0")              PORT_CODE(KEYCODE_0_PAD)        PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Up")                PORT_CODE(KEYCODE_UP)           PORT_CHAR(UCHAR_MAMEKEY(UP))

	PORT_START("col.a")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L Control")         PORT_CODE(KEYCODE_LCONTROL)     PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L Alt")             PORT_CODE(KEYCODE_LALT)         PORT_CHAR(UCHAR_MAMEKEY(LALT))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Space")             PORT_CODE(KEYCODE_SPACE)        PORT_CHAR(' ')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R Alt")             PORT_CODE(KEYCODE_RALT)         PORT_CHAR(UCHAR_MAMEKEY(RALT))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right")             PORT_CODE(KEYCODE_RIGHT)        PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R Control")         PORT_CODE(KEYCODE_RCONTROL)     PORT_CHAR(UCHAR_MAMEKEY(RCONTROL))
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left")              PORT_CODE(KEYCODE_LEFT)         PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Down")              PORT_CODE(KEYCODE_DOWN)         PORT_CHAR(UCHAR_MAMEKEY(DOWN))

	PORT_START("col.b")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Num Lock")          PORT_CODE(KEYCODE_NUMLOCK)      PORT_CHAR(UCHAR_MAMEKEY(NUMLOCK))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP /")              PORT_CODE(KEYCODE_SLASH_PAD)    PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 8")              PORT_CODE(KEYCODE_8_PAD)        PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 7")              PORT_CODE(KEYCODE_7_PAD)        PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 5")              PORT_CODE(KEYCODE_5_PAD)        PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 4")              PORT_CODE(KEYCODE_4_PAD)        PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 2")              PORT_CODE(KEYCODE_2_PAD)        PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 1")              PORT_CODE(KEYCODE_1_PAD)        PORT_CHAR(UCHAR_MAMEKEY(1_PAD))

	PORT_START("col.c")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP *")              PORT_CODE(KEYCODE_ASTERISK)     PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP -")              PORT_CODE(KEYCODE_MINUS_PAD)    PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP +")              PORT_CODE(KEYCODE_PLUS_PAD)     PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 9")              PORT_CODE(KEYCODE_9_PAD)        PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Enter")             PORT_CODE(KEYCODE_ENTER_PAD)    PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 6")              PORT_CODE(KEYCODE_6_PAD)        PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP .")              PORT_CODE(KEYCODE_DEL_PAD)      PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 3")              PORT_CODE(KEYCODE_3_PAD)        PORT_CHAR(UCHAR_MAMEKEY(3_PAD))

	PORT_START("col.d")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("col.e")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("col.f")
	PORT_DIPNAME(0xff, 0x00, "Layout")
	PORT_DIPSETTING(0x00, "us") // United States
	PORT_DIPSETTING(0x01, "de") // Germany
	PORT_DIPSETTING(0x02, "fr") // France
	PORT_DIPSETTING(0x03, "it") // Italy
	PORT_DIPSETTING(0x04, "dk") // Denmark
	PORT_DIPSETTING(0x05, "sp") // Spain
	PORT_DIPSETTING(0x06, "nw") // Norway
	PORT_DIPSETTING(0x07, "sw") // Sweden
	PORT_DIPSETTING(0x08, "sf") // Swiss French
	PORT_DIPSETTING(0x09, "uk") // United Kingdom
	PORT_DIPSETTING(0x0a, "be") // Belgium
	PORT_DIPSETTING(0x0b, "sg") // Swiss German
	PORT_DIPSETTING(0x0c, "nl") // Netherlands
	PORT_DIPSETTING(0x0d, "fn") // Finland
	PORT_DIPSETTING(0x0e, "po") // Portugal
	PORT_DIPSETTING(0x0f, "gr") // Greece
INPUT_PORTS_END

ROM_START(sgi_kbd)
	ROM_REGION(0x2000, "mcu", 0)
	ROM_LOAD("17159__1987__key_tronic.z6", 0x0, 0x2000, CRC(5502cb77) SHA1(a000a2b29e8629c686b29a5af22beedcc94710dc))
ROM_END

} // anonymous namespace

ioport_constructor sgi_kbd_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(sgi_kbd);
}

tiny_rom_entry const *sgi_kbd_device::device_rom_region() const
{
	return ROM_NAME(sgi_kbd);
}

void default_sgi_kbd_devices(device_slot_interface &device)
{
	device.option_add("keyboard", SGI_KBD);
}
