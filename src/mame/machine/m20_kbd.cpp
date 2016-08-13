// license:BSD-3-Clause
// copyright-holders:Carl,Vas Crabb

#include "machine/m20_kbd.h"

#include "machine/keyboard.ipp"


namespace {
/*
    TODO: dump 8048 mcu

    There seem to be a lot of guesses in this code - actual scan rate,
    FIFO length, command processing, etc. don't seem to be evidence-
    based.  The modifier handling seems very odd as it has aliasing and
    overflow all over the place.

    Layout is selected with a set of four jumpers on the keyboard MCU.
    We need to bring these out to DIP switches, work out which command
    is the layout request, and respond appropriately.

    Available layouts:
    Italian
    German
    French
    British
    USA ASCII
    Spanish
    Portuguese
    Swedish - Finnish
    Danish
    Katakana
    Yugoslavian
    Norwegian
    Greek
    Swiss - French
    Swiss - German

    The keyboard MCU drives a buzzer.  We should work out what the bell
    on/off commands are and emulate it.

    There are apparently no break codes.
    The scancodes are arranged to read in alphabetical order with the QWERTY layout.
    Modifiers apparently modify the make codes.
    We are using a matrix here that corresponds to the logical scan codes - the physical matrix apparently doesn't match this.

    00  1c  1d  1e  1f  20  21  22  23  24  25  26  27   c3     cd  ce  cf  d0
     ??   12  18  06  13  15  1a  16  0a  10  11  28  29  c2    ca  cb  cc  d1
       ??  02  14  05  07  08  09  0b  0c  0d  2a  2b  2c  c1   c7  c8  c9  d2
    ??   01  1b  19  04  17  03  0f  0e  2d  2e  2f   ??        c4  c5  c6  d3
                               c0

    Ths is the 72-key version of the keyboard.
    The katakana kayout has 75 keys, but we don't have a diagram for it.
    ?? are modifiers, which are read directly, not through the 8x9 scan matrix.
*/
// Italian layout (QZERTY typewriter)
static INPUT_PORTS_START( m20_keyboard )
	PORT_START("LINE0")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("RESET")    PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_LALT)        PORT_CHAR('<') PORT_CHAR('>')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_A)           PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_B)           PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_C)           PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_D)           PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_E)           PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_F)           PORT_CHAR('f') PORT_CHAR('F')

	PORT_START("LINE1")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_G)           PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_H)           PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_I)           PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_J)           PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_K)           PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_L)           PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_M)           PORT_CHAR(',') PORT_CHAR('?')
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_N)           PORT_CHAR('n') PORT_CHAR('N')

	PORT_START("LINE2")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_O)           PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_P)           PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_Q)           PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_R)           PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_S)           PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_T)           PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_U)           PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_V)           PORT_CHAR('v') PORT_CHAR('V')

	PORT_START("LINE3")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_W)           PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_X)           PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_Y)           PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_Z)           PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_0)           PORT_CHAR(0x00e0U) PORT_CHAR('0') // a grave
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_1)           PORT_CHAR(0x00a3U) PORT_CHAR('1') // pound
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_2)           PORT_CHAR(0x00e9U) PORT_CHAR('2') // e acute
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_3)           PORT_CHAR('"') PORT_CHAR('3')

	PORT_START("LINE4")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_4)           PORT_CHAR('\'') PORT_CHAR('4')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_5)           PORT_CHAR('(') PORT_CHAR('5')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_6)           PORT_CHAR('_') PORT_CHAR('6')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_7)           PORT_CHAR(0x00e8U) PORT_CHAR('7') // e grave
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_8)           PORT_CHAR('^') PORT_CHAR('8')
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_9)           PORT_CHAR(0x00e7U) PORT_CHAR('9') // c cedilla
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_MINUS)       PORT_CHAR(')') PORT_CHAR(0x00b0U) // degree
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_EQUALS)      PORT_CHAR('-') PORT_CHAR('+')

	PORT_START("LINE5")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_OPENBRACE)   PORT_CHAR(0x00ecU) PORT_CHAR('=') // i grave
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_CLOSEBRACE)  PORT_CHAR('$') PORT_CHAR('&')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_COLON)       PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_QUOTE)       PORT_CHAR(0x00f9U) PORT_CHAR('%') // u grave
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_TILDE)       PORT_CHAR('*') PORT_CHAR(0x00a7U) // section
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_COMMA)       PORT_CHAR(';') PORT_CHAR('.')
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_STOP)        PORT_CHAR(':') PORT_CHAR('/')
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD)                       PORT_CODE(KEYCODE_SLASH)       PORT_CHAR(0x00f2U) PORT_CHAR('!') // o grave

	PORT_START("LINE6")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD)                        PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD)                        PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(0x000dU)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("S1")        PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("S2")        PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad .")  PORT_CODE(KEYCODE_DEL_PAD)    PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD)                        PORT_CODE(KEYCODE_0_PAD)      PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad 00") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD)                        PORT_CODE(KEYCODE_1_PAD)      PORT_CHAR(UCHAR_MAMEKEY(1_PAD))

	PORT_START("LINE7")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD)                        PORT_CODE(KEYCODE_2_PAD)      PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD)                        PORT_CODE(KEYCODE_3_PAD)      PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD)                        PORT_CODE(KEYCODE_4_PAD)      PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD)                        PORT_CODE(KEYCODE_5_PAD)      PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD)                        PORT_CODE(KEYCODE_6_PAD)      PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD)                        PORT_CODE(KEYCODE_7_PAD)      PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD)                        PORT_CODE(KEYCODE_8_PAD)      PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD)                        PORT_CODE(KEYCODE_9_PAD)      PORT_CHAR(UCHAR_MAMEKEY(9_PAD))

	PORT_START("LINE8")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD)                        PORT_CODE(KEYCODE_PLUS_PAD)   PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD)                        PORT_CODE(KEYCODE_MINUS_PAD)  PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD)                        PORT_CODE(KEYCODE_ASTERISK)   PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD)                        PORT_CODE(KEYCODE_SLASH_PAD)  PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))
	PORT_BIT(0xf0,IP_ACTIVE_HIGH,IPT_UNUSED)

	PORT_START("MODIFIERS")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("COMMAND")   PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("CTRL")      PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("R SHIFT")   PORT_CODE(KEYCODE_RSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("L SHIFT")   PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0xf0,IP_ACTIVE_HIGH,IPT_UNUSED)
INPUT_PORTS_END
} // anonymous namespace


m20_keyboard_device::m20_keyboard_device(const machine_config& mconfig, const char* tag, device_t* owner, UINT32 clock)
	: buffered_rs232_device(mconfig, M20_KEYBOARD, "M20 Keyboard", tag, owner, 0, "m20_keyboard", __FILE__)
	, device_matrix_keyboard_interface(mconfig, *this, "LINE0", "LINE1", "LINE2", "LINE3", "LINE4", "LINE5", "LINE6", "LINE7", "LINE8")
	, m_modifiers(*this, "MODIFIERS")
{
}


ioport_constructor m20_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(m20_keyboard);
}


void m20_keyboard_device::device_reset()
{
	buffered_rs232_device::device_reset();

	reset_key_state();
	clear_fifo();

	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_2);
	set_rate(1'200);
	receive_register_reset();
	transmit_register_reset();

	output_dcd(0);
	output_dsr(0);
	output_cts(0);
	output_rxd(1);

	start_processing(attotime::from_hz(1'200));
}


void m20_keyboard_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	device_matrix_keyboard_interface::device_timer(timer, id, param, ptr);
	buffered_rs232_device::device_timer(timer, id, param, ptr);
}


void m20_keyboard_device::key_make(UINT8 row, UINT8 column)
{
	UINT8 const row_code(((row < 6U) ? row : (0x18U | (row - 6U))) << 3);
	UINT8 const modifiers(m_modifiers->read());
	UINT8 mod_code(0U);
	switch (modifiers)
	{
	case 0x01U: // COMMAND
		mod_code = 0x90;
		break;
	case 0x02U: // CTRL
		mod_code = 0x60;
		break;
	case 0x04U: // RSHIFT
	case 0x08U: // LSHIFT
	case 0x0cU: // LSHIFT|RSHIFT
		mod_code = 0x30;
		break;
	}
	transmit_byte((row_code | column) + mod_code);
}


void m20_keyboard_device::received_byte(UINT8 byte)
{
	switch (byte)
	{
	case 0x03U:
		transmit_byte(0x02U);
		break;
	case 0x80U:
		transmit_byte(0x80U);
		break;
	default:
		logerror("received unknown command %02x", byte);
	}
}


const device_type M20_KEYBOARD = &device_creator<m20_keyboard_device>;
