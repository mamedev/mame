// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
 * TTL-level serial matrix keyboard
 *
 * The keyboard MCU handles scanning the matrix and converting key down/
 * up events to high-level characters for the host computer.  The DIP
 * switches are only read on startup.  The row select outputs are
 * decoded by a 74159 so only one row can be driven at a time.
 *
 * The keyboard has a PCB location for the DIP switches, but they're
 * unpopulated so all the switches are always off unless you add them or
 * solder in wire links.  The keyclick/bell beeper also appears to be
 * unpopulated.  The 300 Baud option is probably to allow the same MCU
 * program to be used with other CP/M systems (e.g. Kaypro expects this
 * Baud rate).
 *
 * In addition to the asynchronous serial output, the program supports
 * synchronous output.  If enabled with DIP switches, characters are
 * sent out MSB first on PB2.  PB3 is pulsed low for each bit, and PB4
 * is pulsed low after sending a complete byte.  PB2 only changes while
 * PB3 and PB4 are both high; PB4 is pulsed after the pulse on PB3 for
 * the final bit.  There are no delays - data is sent as fast as
 * possible.  Characters are still sent on the asynchronous serial
 * output when synchronous output is enabled.
 *
 * The keyboard understands a very simple single-byte command format.
 * - If the MSB of the command is set, it will run one of the diagnostic
 *   modes, depending on the three LSBs (same options provided by the
 *   three low DIP switches), and the rest of the command processing is
 *   skipped.
 * - If bit 6 of the command is set, it will spin until it receives a
 *   command with bit 6 clear (diagnostic mode commands are ignored
 *   while in this state).
 * - Bit 5 controls the keyboard bell (active high).
 * - If the /INT pin is tied high, bits 0 and 1 control the Shift Lock
 *   and Caps Lock LEDs, respectively.  Note that only the LEDs are
 *   affected, not the keyboard's shift/caps lock state.  If /INT is
 *   tied low, these bits are ignored.
 * - Bits 4, 3 and 2 control PB4, PB3 and PB2 (setting the bit pulls the
 *   output low).  This is presumably used to set the state of the
 *   F17/F18/F19 LEDs when synchronous output is not being used.
 *
 * The Zorba manual describes a keyboard PROM with enough space to map
 * all eight modifier combinations independently, however this keyboard
 * contains no PROMs (just the MCU and demultiplexer), and the internal
 * MCU EPROM only contains three tables for normal, shift, and control
 * (caps lock is computed off the normal table by checking for letter
 * characters).
 *
 * The host sets the keyboard USART to 1200 8N2.  The MCU generates
 * serial timings off timer interrupts.  Receive synchronisation is
 * acquired at four times the Baud rate (4800Hz or 1200Hz).
 *
 *       0         1         2         3         4         5         6         7
 *  0    F1        F13       3 #       KP8       i I       s S       KP2       , <
 *  1    F2        F14       4 $       KP9       o O       d D       KP3       . >
 *  2    F3        F15       5 %       break     p P       f F       up        / ?
 *  3    F4        F16       6 ^       home      [ {       g G       down      KP0
 *  4    F5        F17       7 &       tab       ] }       h H       shift     KP.
 *  5    F6        F18       8 *       q Q       KP4       j J       z Z       left
 *  6    F7        F19       9 (       w W       KP5       k K       x X       right
 *  7    F8        ~         0 )       e E       KP6       l L       c C       space
 *  8    F9        \ |       - _       r R       s-lock    ; :       v V       newline
 *  9    F10       ESC       = +       t T       ctrl      ' "       b B       KP-
 * 10    F11       1 !       del       y Y       c-lock    return    n N
 * 11    F12       2 @       KP7       u U       a A       KP1       m M
 *
 * TODO: The two "holes" generate control characters, possibly soft
 * capslock and shiftlock codes to be used when /INT is tied high.  They
 * don't correspond to physical keys on the Zorba keyboard.
 */
#include "emu.h"
#include "zorbakbd.h"

#include "cpu/m6805/m68705.h"
#include "speaker.h"


namespace {

INPUT_PORTS_START(zorba_keyboard)
	PORT_START("ROW0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F1")           PORT_CODE(KEYCODE_F1)         PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F13")          PORT_CODE(KEYCODE_F13)        PORT_CHAR(UCHAR_MAMEKEY(F13))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_3)          PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("KP 8")         PORT_CODE(KEYCODE_8_PAD)      PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_I)          PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_S)          PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("KP 2")         PORT_CODE(KEYCODE_2_PAD)      PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR('<')

	PORT_START("ROW1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F2")           PORT_CODE(KEYCODE_F2)         PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F14")          PORT_CODE(KEYCODE_F14)        PORT_CHAR(UCHAR_MAMEKEY(F14))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_4)          PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("KP 9")         PORT_CODE(KEYCODE_9_PAD)      PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_O)          PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_D)          PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("KP 3")         PORT_CODE(KEYCODE_3_PAD)      PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR('>')

	PORT_START("ROW2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F3")           PORT_CODE(KEYCODE_F3)         PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F15")          PORT_CODE(KEYCODE_F15)        PORT_CHAR(UCHAR_MAMEKEY(F15))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_5)          PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("BREAK")        PORT_CODE(KEYCODE_PAUSE)      PORT_CHAR(UCHAR_MAMEKEY(PAUSE))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_P)          PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_F)          PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("ROW3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F4")           PORT_CODE(KEYCODE_F4)         PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F16")          PORT_CODE(KEYCODE_F16)        PORT_CHAR(UCHAR_MAMEKEY(F16))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_6)          PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("HOME")         PORT_CODE(KEYCODE_HOME)       PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_G)          PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_DOWN)       PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("KP 0")         PORT_CODE(KEYCODE_0_PAD)      PORT_CHAR(UCHAR_MAMEKEY(0_PAD))

	PORT_START("ROW4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F5")           PORT_CODE(KEYCODE_F5)         PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F17")          PORT_CODE(KEYCODE_F17)        PORT_CHAR(UCHAR_MAMEKEY(F17))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_7)          PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("TAB")          PORT_CODE(KEYCODE_TAB)        PORT_CHAR(9)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_H)          PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT")        PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("KP .")         PORT_CODE(KEYCODE_DEL_PAD)    PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))

	PORT_START("ROW5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F6")           PORT_CODE(KEYCODE_F6)         PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F18")          PORT_CODE(KEYCODE_F18)        PORT_CHAR(UCHAR_MAMEKEY(F18))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_8)          PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_Q)          PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("KP 4")         PORT_CODE(KEYCODE_4_PAD)      PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_J)          PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_Z)          PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_LEFT)       PORT_CHAR(UCHAR_MAMEKEY(LEFT))

	PORT_START("ROW6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F7")           PORT_CODE(KEYCODE_F7)         PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F19")          PORT_CODE(KEYCODE_F19)        PORT_CHAR(UCHAR_MAMEKEY(F19))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_W)          PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("KP 5")         PORT_CODE(KEYCODE_5_PAD)      PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_K)          PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_X)          PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT))

	PORT_START("ROW7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F8")           PORT_CODE(KEYCODE_F8)         PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("~ PRINT")      PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('~') PORT_CHAR(UCHAR_MAMEKEY(PRTSCR))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_0)          PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_E)          PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("KP 6")         PORT_CODE(KEYCODE_6_PAD)      PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_L)          PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_C)          PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')

	PORT_START("ROW8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F9")           PORT_CODE(KEYCODE_F9)         PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_R)          PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT LOCK")   PORT_CODE(KEYCODE_LALT)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_V)          PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("NEW LINE")     PORT_CODE(KEYCODE_ENTER_PAD)  PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))

	PORT_START("ROW9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F10")          PORT_CODE(KEYCODE_F10)        PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ESC")          PORT_CODE(KEYCODE_ESC)        PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_T)          PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CTRL")         PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_B)          PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("KP -")         PORT_CODE(KEYCODE_MINUS_PAD)  PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))

	PORT_START("ROW10")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F11")          PORT_CODE(KEYCODE_F11)        PORT_CHAR(UCHAR_MAMEKEY(F11))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("BS DEL")       PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(8) PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_Y)          PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CAPS LOCK")    PORT_CODE(KEYCODE_CAPSLOCK)   PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RETURN")       PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_N)          PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  ) // e0/e2/e3

	PORT_START("ROW11")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F12")          PORT_CODE(KEYCODE_F12)        PORT_CHAR(UCHAR_MAMEKEY(F12))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("KP 7")         PORT_CODE(KEYCODE_7_PAD)      PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_U)          PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_A)          PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("KP 1")         PORT_CODE(KEYCODE_1_PAD)      PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_M)          PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  ) // e1/e3/e4

	PORT_START("ROW12")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED   ) // scanned but the tables are too small so it would post garbage

	PORT_START("ROW13")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED   ) // scanned but the tables are too small so it would post garbage

	PORT_START("ROW14")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED   ) // scanned but the tables are too small so it would post garbage

	PORT_START("ROW15")
	PORT_DIPNAME( 0x07, 0x07, "Diagnostic Mode" )       PORT_DIPLOCATION("DIP:1,2,3")
	PORT_DIPSETTING(    0x07, "0 (Normal)" )
	PORT_DIPSETTING(    0x06, "1 (Dump Tables)" )
	PORT_DIPSETTING(    0x05, "2 (Report Revision)" )
	PORT_DIPSETTING(    0x04, "3 (Serial Test)" )
	PORT_DIPSETTING(    0x03, "4 (Serial Echo)" )
	PORT_DIPSETTING(    0x02, "5 (Output Test)" )
	PORT_DIPSETTING(    0x01, "6 (Normal)" )
	PORT_DIPSETTING(    0x00, "7 (Dump Tables)" )
	PORT_DIPNAME( 0x08, 0x08, "Key Repeat" )            PORT_DIPLOCATION("DIP:4")
	PORT_DIPSETTING(    0x00, DEF_STR(Off) )
	PORT_DIPSETTING(    0x08, DEF_STR(On) )
	PORT_DIPNAME( 0x10, 0x10, "Baud Rate" )             PORT_DIPLOCATION("DIP:5")
	PORT_DIPSETTING(    0x00, "300" )
	PORT_DIPSETTING(    0x10, "1200" )
	PORT_DIPNAME( 0x20, 0x20, "Key Click" )             PORT_DIPLOCATION("DIP:6")
	PORT_DIPSETTING(    0x00, DEF_STR(Off) )
	PORT_DIPSETTING(    0x20, DEF_STR(On) )
	PORT_DIPNAME( 0x40, 0x40, "Synchronous Output" )    PORT_DIPLOCATION("DIP:7")
	PORT_DIPSETTING(    0x40, DEF_STR(Off) )
	PORT_DIPSETTING(    0x00, DEF_STR(On) )
	PORT_DIPNAME( 0x80, 0x80, "Key Repeat Delay/Rate" ) PORT_DIPLOCATION("DIP:8")
	PORT_DIPSETTING(    0x80, "0.75s/15cps" )
	PORT_DIPSETTING(    0x00, "0.5s/21cps" )
INPUT_PORTS_END


ROM_START(zorba_keyboard)
	ROM_REGION(0x0800, "mcu", 0)
	ROM_LOAD( "8999-1 3-28-83", 0x080, 0x780, CRC(79fe6c0d) SHA1(4b6fca9379d5199d1347ad1187cbfdebfc4c73e7) )
ROM_END

} // anonymous namespace


DEFINE_DEVICE_TYPE(ZORBA_KEYBOARD, zorba_keyboard_device, "zorba_kbd", "Zorba Keyboard")


zorba_keyboard_device::zorba_keyboard_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		uint32_t clock)
	: device_t(mconfig, ZORBA_KEYBOARD, tag, owner, clock)
	, m_rows(*this, "ROW%u", 0)
	, m_beeper(*this, "beeper")
	, m_led_key_caps_lock(*this, "led_key_caps_lock")
	, m_led_key_shift_lock(*this, "led_key_shift_lock")
	, m_rxd_cb(*this)
	, m_txd_high(true)
	, m_row_select(0)
{
}


void zorba_keyboard_device::txd_w(int state)
{
	m_txd_high = CLEAR_LINE != state;
}


u8 zorba_keyboard_device::mcu_pa_r()
{
	return m_rows[m_row_select]->read();
}


u8 zorba_keyboard_device::mcu_pb_r()
{
	return m_txd_high ? 0x7f : 0xff;
}


void zorba_keyboard_device::mcu_pb_w(u8 data)
{
	// TODO: bits 2/3/4 do something; some photos show F17/F18/F19 with LED windows
	m_rxd_cb(BIT(data, 6) ? 0 : 1);
	m_beeper->set_state(BIT(data, 5) ? 0 : 1);
	m_led_key_caps_lock  = BIT(data, 1) ? 0 : 1;
	m_led_key_shift_lock = BIT(data, 0) ? 0 : 1;
}


void zorba_keyboard_device::mcu_pc_w(u8 data)
{
	m_row_select = data & 0x0f;
}


void zorba_keyboard_device::device_start()
{
	m_led_key_caps_lock.resolve();
	m_led_key_shift_lock.resolve();

	save_item(NAME(m_txd_high));
	save_item(NAME(m_row_select));

	m_txd_high = true;
	m_row_select = 0;
}


void zorba_keyboard_device::device_add_mconfig(machine_config &config)
{
	// MC68705P3S
	m68705p3_device &mcu(M68705P3(config, "mcu", 3.579'545_MHz_XTAL));
	mcu.porta_r().set(FUNC(zorba_keyboard_device::mcu_pa_r));
	mcu.portb_r().set(FUNC(zorba_keyboard_device::mcu_pb_r));
	mcu.portb_w().set(FUNC(zorba_keyboard_device::mcu_pb_w));
	mcu.portc_w().set(FUNC(zorba_keyboard_device::mcu_pc_w));

	// TODO: beeper frequency is unknown, using value from Sun keyboard for now
	SPEAKER(config, "bell").front_center();
	BEEP(config, m_beeper, ATTOSECONDS_TO_HZ(480 * ATTOSECONDS_PER_MICROSECOND));
	m_beeper->add_route(ALL_OUTPUTS, "bell", 0.4);
}


ioport_constructor zorba_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(zorba_keyboard);
}


tiny_rom_entry const *zorba_keyboard_device::device_rom_region() const
{
	return ROM_NAME(zorba_keyboard);
}
