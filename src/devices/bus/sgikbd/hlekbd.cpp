// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
#include "emu.h"
#include "hlekbd.h"

#include "machine/keyboard.ipp"
#include "speaker.h"

DEFINE_DEVICE_TYPE_NS(SGI_HLE_KEYBOARD, bus::sgikbd, hle_device, "hlekbd", "SGI Indigo Keyboard (HLE)")

namespace bus { namespace sgikbd {

namespace {

INPUT_PORTS_START( hle_device )
	PORT_START("ROW0")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("L Control")    PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Caps Lock")    PORT_CODE(KEYCODE_CAPSLOCK)   PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("R Shift")      PORT_CODE(KEYCODE_RSHIFT)     PORT_CHAR(UCHAR_MAMEKEY(RSHIFT))
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("L Shift")      PORT_CODE(KEYCODE_LSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Esc")          PORT_CODE(KEYCODE_ESC)        PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Tab")          PORT_CODE(KEYCODE_TAB)        PORT_CHAR(9)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_Q)          PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_A)          PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_S)          PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_3)          PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_W)          PORT_CHAR('w') PORT_CHAR('W')

	PORT_START("ROW1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_E)          PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_D)          PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_F)          PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_Z)          PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_X)          PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_4)          PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_5)          PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_R)          PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_T)          PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_G)          PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_H)          PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_C)          PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_V)          PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_6)          PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_7)          PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_Y)          PORT_CHAR('y') PORT_CHAR('Y')

	PORT_START("ROW2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_U)          PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_J)          PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_K)          PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_B)          PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_N)          PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_8)          PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_I)          PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_O)          PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_L)          PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_M)          PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_0)          PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_P)          PORT_CHAR('p') PORT_CHAR('P')

	PORT_START("ROW3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Return")       PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("KP 1")         PORT_CODE(KEYCODE_1_PAD)      PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("KP 0")         PORT_CODE(KEYCODE_0_PAD)      PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Backspace")    PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(8)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("KP 4")         PORT_CODE(KEYCODE_4_PAD)      PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("KP 2")         PORT_CODE(KEYCODE_2_PAD)      PORT_CHAR(UCHAR_MAMEKEY(2_PAD))

	PORT_START("ROW4")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("KP 3")         PORT_CODE(KEYCODE_3_PAD)      PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("KP .")         PORT_CODE(KEYCODE_DEL_PAD)    PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("KP 7")         PORT_CODE(KEYCODE_7_PAD)      PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("KP 8")         PORT_CODE(KEYCODE_8_PAD)      PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("KP 5")         PORT_CODE(KEYCODE_5_PAD)      PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("KP 6")         PORT_CODE(KEYCODE_6_PAD)      PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Left")         PORT_CODE(KEYCODE_LEFT)       PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Down")         PORT_CODE(KEYCODE_DOWN)       PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("KP 9")         PORT_CODE(KEYCODE_9_PAD)      PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("KP -")         PORT_CODE(KEYCODE_MINUS_PAD)  PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Right")        PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT))

	PORT_START("ROW5")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Up")           PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Enter")        PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Space")        PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("L Alt")        PORT_CODE(KEYCODE_LALT)       PORT_CHAR(UCHAR_MAMEKEY(LALT))
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("R Alt")        PORT_CODE(KEYCODE_RALT)       PORT_CHAR(UCHAR_MAMEKEY(RALT))
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("R Control")    PORT_CODE(KEYCODE_RCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(RCONTROL))
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F1")           PORT_CODE(KEYCODE_F1)         PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F2")           PORT_CODE(KEYCODE_F2)         PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F3")           PORT_CODE(KEYCODE_F3)         PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F4")           PORT_CODE(KEYCODE_F4)         PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F5")           PORT_CODE(KEYCODE_F5)         PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F6")           PORT_CODE(KEYCODE_F6)         PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F7")           PORT_CODE(KEYCODE_F7)         PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F8")           PORT_CODE(KEYCODE_F8)         PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F9")           PORT_CODE(KEYCODE_F9)         PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F10")          PORT_CODE(KEYCODE_F10)        PORT_CHAR(UCHAR_MAMEKEY(F10))

	PORT_START("ROW6")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F11")          PORT_CODE(KEYCODE_F11)        PORT_CHAR(UCHAR_MAMEKEY(F11))
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F12")          PORT_CODE(KEYCODE_F12)        PORT_CHAR(UCHAR_MAMEKEY(F12))
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Print Screen") PORT_CODE(KEYCODE_PRTSCR)     PORT_CHAR(UCHAR_MAMEKEY(PRTSCR))
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Scroll Lock")  PORT_CODE(KEYCODE_SCRLOCK)    PORT_CHAR(UCHAR_MAMEKEY(SCRLOCK))
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Pause")        PORT_CODE(KEYCODE_PAUSE)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Insert")       PORT_CODE(KEYCODE_INSERT)     PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Home")         PORT_CODE(KEYCODE_HOME)       PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Page Up")      PORT_CODE(KEYCODE_PGUP)       PORT_CHAR(UCHAR_MAMEKEY(PGUP))
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("End")          PORT_CODE(KEYCODE_END)        PORT_CHAR(UCHAR_MAMEKEY(END))
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Page Down")    PORT_CODE(KEYCODE_PGDN)       PORT_CHAR(UCHAR_MAMEKEY(PGDN))
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num Lock")     PORT_CODE(KEYCODE_NUMLOCK)    PORT_CHAR(UCHAR_MAMEKEY(NUMLOCK))
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("KP /")         PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("KP *")         PORT_CODE(KEYCODE_ASTERISK)   PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("KP +")         PORT_CODE(KEYCODE_PLUS_PAD)   PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ROW7")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

} // anonymous namespace

hle_device::hle_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SGI_HLE_KEYBOARD, tag, owner, clock)
	, device_buffered_serial_interface(mconfig, *this)
	, device_sgi_keyboard_port_interface(mconfig, *this)
	, device_matrix_keyboard_interface(mconfig, *this, "ROW0", "ROW1", "ROW2", "ROW3", "ROW4", "ROW5", "ROW6", "ROW7")
	, m_click_timer(nullptr)
	, m_beep_timer(nullptr)
	, m_beeper(*this, "beeper")
	, m_leds(*this, "led%u", 0U)
	, m_make_count(0)
	, m_keyclick(true)
	, m_auto_repeat(false)
	, m_beeper_state(0)
	, m_led_state(0)
{
}

hle_device::~hle_device()
{
}

ioport_constructor hle_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(hle_device);
}

WRITE_LINE_MEMBER(hle_device::input_txd)
{
	device_buffered_serial_interface::rx_w(state);
}

void hle_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "bell").front_center();
	BEEP(config, m_beeper, ATTOSECONDS_TO_HZ(480 * ATTOSECONDS_PER_MICROSECOND));
	m_beeper->add_route(ALL_OUTPUTS, "bell", 1.0);
}

void hle_device::device_start()
{
	m_leds.resolve();
	m_click_timer = timer_alloc(TIMER_CLICK);
	m_beep_timer = timer_alloc(TIMER_BEEP);

	save_item(NAME(m_make_count));
	save_item(NAME(m_keyclick));
	save_item(NAME(m_auto_repeat));
	save_item(NAME(m_beeper_state));
	save_item(NAME(m_led_state));
}

void hle_device::device_reset()
{
	// initialise state
	clear_fifo();
	m_make_count = 0;
	m_keyclick = true;
	m_auto_repeat = false;
	m_beeper_state = 0;
	m_led_state = 0;

	// configure device_buffered_serial_interface
	set_data_frame(START_BIT_COUNT, DATA_BIT_COUNT, PARITY, STOP_BITS);
	set_rate(BAUD);
	receive_register_reset();
	transmit_register_reset();

	// set device_sgi_keyboard_port_interface lines
	output_rxd(1);

	// start with keyboard LEDs off
	m_leds[LED_NUM] = 0;
	m_leds[LED_CAPS] = 0;
	m_leds[LED_SCROLL] = 0;
	m_leds[LED_USER1] = 0;
	m_leds[LED_USER2] = 0;
	m_leds[LED_USER3] = 0;
	m_leds[LED_USER4] = 0;

	m_click_timer->adjust(attotime::never);
	m_beep_timer->adjust(attotime::never);

	// kick the base
	reset_key_state();
	start_processing(attotime::from_hz(600));
}

void hle_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_CLICK:
		m_beeper_state &= ~BEEPER_CLICK;
		m_beeper->set_state(m_beeper_state ? 1 : 0);
		break;

	case TIMER_BEEP:
		m_beeper_state &= ~BEEPER_BELL;
		m_beeper->set_state(m_beeper_state ? 1 : 0);
		break;

	default:
		break;
	}
}

void hle_device::tra_callback()
{
	output_rxd(transmit_register_get_data_bit());
}

void hle_device::tra_complete()
{
	if (fifo_full())
		start_processing(attotime::from_hz(600));

	device_buffered_serial_interface::tra_complete();
}

void hle_device::key_make(uint8_t row, uint8_t column)
{
	// we should have stopped processing if we filled the FIFO
	assert(!fifo_full());

	// send the make code, click if desired
	transmit_byte((row << 4) | column);
	if (m_keyclick)
	{
		m_beeper_state |= uint8_t(BEEPER_CLICK);
		m_beeper->set_state(m_beeper_state ? 1 : 0);
		m_click_timer->reset(attotime::from_msec(5));
	}

	// count keys
	++m_make_count;
	assert(m_make_count);
}

void hle_device::key_break(uint8_t row, uint8_t column)
{
	// we should have stopped processing if we filled the FIFO
	assert(!fifo_full());
	assert(m_make_count);

	// send the break code, and the idle code if no other keys are down
	transmit_byte(0x80 | (row << 4) | column);
	if (!--m_make_count)
		transmit_byte(0xf0);
}

void hle_device::transmit_byte(uint8_t byte)
{
	device_buffered_serial_interface::transmit_byte(byte);
	if (fifo_full())
		stop_processing();
}

void hle_device::received_byte(uint8_t byte)
{
	if (BIT(byte, CTRL_B))
	{
		if (BIT(byte, CTRL_B_CMPL_DS1_2))
		{
			logerror("Not Yet Implemented: Complement DS1/2\n");
		}

		if (BIT(byte, CTRL_B_SCRLK))
		{
			logerror("Toggle Scroll Lock LED\n");
			m_led_state ^= (1 << LED_SCROLL);
			m_leds[LED_SCROLL] = BIT(m_led_state, LED_SCROLL);
		}
		if (BIT(byte, CTRL_B_L1))
		{
			logerror("Toggle User LED 1\n");
			m_led_state ^= (1 << LED_USER1);
			m_leds[LED_USER1] = BIT(m_led_state, LED_USER1);
		}
		if (BIT(byte, CTRL_B_L2))
		{
			logerror("Toggle User LED 2\n");
			m_led_state ^= (1 << LED_USER2);
			m_leds[LED_USER2] = BIT(m_led_state, LED_USER2);
		}
		if (BIT(byte, CTRL_B_L3))
		{
			logerror("Toggle User LED 3\n");
			m_led_state ^= (1 << LED_USER3);
			m_leds[LED_USER3] = BIT(m_led_state, LED_USER3);
		}
		if (BIT(byte, CTRL_B_L4))
		{
			logerror("Toggle User LED 4\n");
			m_led_state ^= (1 << LED_USER4);
			m_leds[LED_USER3] = BIT(m_led_state, LED_USER4);
		}
	}
	else
	{
		if (BIT(byte, CTRL_A_SBEEP))
		{
			logerror("Short Beep\n");
			m_beeper_state |= uint8_t(BEEPER_BELL);
			m_beeper->set_state(m_beeper_state ? 1 : 0);
			m_click_timer->adjust(attotime::from_msec(200));
		}

		if (BIT(byte, CTRL_A_LBEEP))
		{
			logerror("Long Beep\n");
			m_beeper_state |= uint8_t(BEEPER_BELL);
			m_beeper->set_state(m_beeper_state ? 1 : 0);
			m_click_timer->adjust(attotime::from_msec(1000));
		}

		if (BIT(byte, CTRL_A_NOCLICK))
		{
			logerror("Keyclick Off\n");
			m_keyclick = false;
		}

		if (BIT(byte, CTRL_A_RCB))
		{
			logerror("Receive Config Byte\n");
			transmit_byte(0x6e);
			transmit_byte(0x00); // US layout
		}

		if (BIT(byte, CTRL_A_NUMLK))
		{
			logerror("Toggle Num Lock LED\n");
			m_led_state ^= (1 << LED_NUM);
			m_leds[LED_NUM] = BIT(m_led_state, LED_NUM);
		}

		if (BIT(byte, CTRL_A_CAPSLK))
		{
			logerror("Toggle Caps Lock LED\n");
			m_led_state ^= (1 << LED_CAPS);
			m_leds[LED_CAPS] = BIT(m_led_state, LED_CAPS);
		}

		if (BIT(byte, CTRL_A_AUTOREP))
		{
			logerror("Toggle Auto Repeat\n");
			m_auto_repeat = !m_auto_repeat;
		}
	}
}

} } // namespace bus::sgikbd
