// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Informer 207/376 Keyboard (HLE)

***************************************************************************/

#include "emu.h"
#include "informer_207_376_kbd.h"
#include "machine/keyboard.ipp"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(INFORMER_207_376_KBD_HLE, informer_207_376_kbd_hle_device, "in207376kbd_hle", "Informer 207/376 Keyboard (HLE)")


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

// keys not mapped yet:
//
// PF13 PF14 PF15 PF16 PF17 PF18 PF19 PF20 PF21 PF22 PF23 (PF24 = cursor up?)
// ATTN

static INPUT_PORTS_START( keyboard )
	PORT_START("row_0")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 00 */
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 01 */
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 02 */
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 03 */
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 04 */
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 05 */
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 06 */
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 07 */
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 08 */ PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(0x0d)                 PORT_NAME("\xe2\x86\xb2") // ↲
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 09 */ PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('<') PORT_CHAR('>')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 0a */
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 0b */
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 0c */ PORT_CODE(KEYCODE_INSERT)     PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 0d */ PORT_CODE(KEYCODE_DEL)        PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 0e */ PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP))    PORT_NAME("\xe2\x86\x91")
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 0f */ PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('{') PORT_CHAR('}')

	PORT_START("row_1")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 10 */ PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 11 */ PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=')  PORT_CHAR('+')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 12 */ PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 13 */ PORT_CODE(KEYCODE_DOWN)       PORT_CHAR(UCHAR_MAMEKEY(DOWN))  PORT_NAME("\xe2\x86\x93")
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 14 */ PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/')  PORT_CHAR('?')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 15 */ PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('\\') PORT_NAME(u8"\\  ¦")
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 16 */ PORT_CODE(KEYCODE_LEFT)       PORT_CHAR(UCHAR_MAMEKEY(LEFT))  PORT_NAME("\xe2\x86\x90")
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 17 */
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 18 */ PORT_CODE(KEYCODE_ENTER_PAD)  PORT_NAME("Enter  Set Up")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 19 */
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 1a */ PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_NAME("\xe2\x86\x92")
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 1b */ PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR(0) PORT_CHAR('!')     PORT_NAME(u8"¢  !")
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 1c */
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 1d */
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 1e */
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 1f */

	PORT_START("row_2")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 20 */ PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 21 */ PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('|')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 22 */ PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 23 */ PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 24 */ PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 25 */ PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 26 */ PORT_CODE(KEYCODE_6) PORT_CHAR('6')                PORT_NAME(u8"6  ¬")
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 27 */ PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 28 */ PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 29 */ PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 2a */
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 2b */
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 2c */
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 2d */
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 2e */
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 2f */

	PORT_START("row_3")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 30 */ PORT_CODE(KEYCODE_MINUS)     PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 31 */ PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(0x08)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 32 */ PORT_CODE(KEYCODE_STOP)      PORT_CHAR('.') // same as 0x4b?
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 33 */ PORT_CODE(KEYCODE_COMMA)     PORT_CHAR(',') // same as 0x49?
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 34 */ PORT_CODE(KEYCODE_ESC)       PORT_NAME("Reset  Dev Cncl")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 35 */ PORT_CODE(KEYCODE_TAB)       PORT_NAME("Tab or Back-Tab")
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 36 */ PORT_CODE(KEYCODE_HOME)      PORT_NAME("Tab or Back-Tab")
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 37 */
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 38 */
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 39 */
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 3a */
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 3b */
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 3c */
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 3d */ PORT_CODE(KEYCODE_TILDE)     PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 3e */
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 3f */

	PORT_START("row_4")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 40 */ // 7
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 41 */ // 8
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 42 */ // 9
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 43 */ // 4
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 44 */ // 5
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 45 */ // 6
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 46 */ // 1
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 47 */ // 2
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 48 */ // 3
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 49 */ PORT_CODE(KEYCODE_F5) // ,
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 4a */ // 0
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 4b */ PORT_CODE(KEYCODE_F6) // . with alt key unknown function
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 4c */ PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_NAME("Lock")
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 4d */ PORT_CODE(KEYCODE_LSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)           PORT_NAME("Left Shift")  // or right?
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 4e */ PORT_CODE(KEYCODE_RSHIFT)                                      PORT_NAME("Right Shift") // or left?
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 4f */ PORT_CODE(KEYCODE_LALT)     PORT_CHAR(UCHAR_SHIFT_2)           PORT_NAME("Alt")

	PORT_START("row_5")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 50 */ PORT_CODE(KEYCODE_F2)        PORT_NAME("Cursor Sel  Clear")
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 51 */ PORT_CODE(KEYCODE_F1) // X + human? ATTN?
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 52 */
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 53 */
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 54 */ PORT_CODE(KEYCODE_F3)        PORT_NAME("Cursor Blink  Alt Cursor")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 55 */ PORT_CODE(KEYCODE_END)       PORT_NAME("Erase EOF")
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 56 */ PORT_CODE(KEYCODE_PRTSCR)    PORT_NAME("Print")
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 57 */ PORT_CODE(KEYCODE_F4)        PORT_NAME("Keyclick  Test")
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 58 */
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 59 */
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 5a */
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 5b */
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 5c */
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 5d */
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 5e */ PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("Field Mark")
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 5f */ PORT_CODE(KEYCODE_ASTERISK)  PORT_NAME("Dup")

	PORT_START("row_6")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 60 */ PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 61 */ PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 62 */ PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 63 */ PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 64 */ PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 65 */ PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 66 */ PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 67 */ PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 68 */ PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 69 */ PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 6a */ PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 6b */ PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 6c */ PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 6d */ PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 6e */ PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 6f */ PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')

	PORT_START("row_7")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 70 */ PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 71 */ PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 72 */ PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 73 */ PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 74 */ PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 75 */ PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 76 */ PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 77 */ PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 78 */ PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 79 */ PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 7a */
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 7b */
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 7c */
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 7d */
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 7e */ PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN)  /* 7f */
INPUT_PORTS_END

ioport_constructor informer_207_376_kbd_hle_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( keyboard );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  informer_207_376_kbd_hle_device - constructor
//-------------------------------------------------

informer_207_376_kbd_hle_device::informer_207_376_kbd_hle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, INFORMER_207_376_KBD_HLE, tag, owner, clock),
	device_buffered_serial_interface(mconfig, *this),
	device_matrix_keyboard_interface(mconfig, *this, "row_0", "row_1", "row_2", "row_3", "row_4", "row_5", "row_6", "row_7"),
	m_tx_handler(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void informer_207_376_kbd_hle_device::device_start()
{
	// resolve callbacks
	m_tx_handler.resolve_safe();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void informer_207_376_kbd_hle_device::device_reset()
{
	clear_fifo();

	receive_register_reset();
	transmit_register_reset();

	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_2);
	set_rcv_rate(2400);
	set_tra_rate(2400);

	reset_key_state();
	start_processing(attotime::from_hz(2400));
	typematic_stop();
}

//-------------------------------------------------
//  tra_callback - send bit to host
//-------------------------------------------------

void informer_207_376_kbd_hle_device::tra_callback()
{
	m_tx_handler(transmit_register_get_data_bit());
}

//-------------------------------------------------
//  received_byte - handle received byte
//-------------------------------------------------

void informer_207_376_kbd_hle_device::received_byte(uint8_t byte)
{
	logerror("Received from host: %02x\n", byte);

	// correct? also gets 0x10 from host
	if (byte == 0x0a)
		transmit_byte(0x34); // send RESET
}

//-------------------------------------------------
//  key_make - handle a key being pressed
//-------------------------------------------------

void informer_207_376_kbd_hle_device::key_make(uint8_t row, uint8_t column)
{
	// send the code
	uint8_t code = row * 16 + column;

	transmit_byte(code);

	// no typematic for modifier keys
	if (code != 0x4c && code != 0x4d && code != 0x4e && code != 0x4f)
		typematic_start(row, column, attotime::from_msec(750), attotime::from_msec(50));
}

//-------------------------------------------------
//  key_break - handle a key being released
//-------------------------------------------------

void informer_207_376_kbd_hle_device::key_break(uint8_t row, uint8_t column)
{
	// send the break code (only for modifier keys: shift and alt)
	uint8_t code = row * 16 + column;

	if (typematic_is(row, column))
		typematic_stop();

	if (code == 0x4d || code == 0x4e || code == 0x4f)
		transmit_byte(0x80 | code);
}

//-------------------------------------------------
//  key_repeat - handle a key being repeated
//-------------------------------------------------

void informer_207_376_kbd_hle_device::key_repeat(u8 row, u8 column)
{
	uint8_t code = row * 16 + column;
	transmit_byte(code);
}

//-------------------------------------------------
//  rx_w - receive bit from host
//-------------------------------------------------

void informer_207_376_kbd_hle_device::rx_w(int state)
{
	device_buffered_serial_interface::rx_w(state);
}
