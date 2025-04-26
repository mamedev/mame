// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

PC-9801 Keyboard simulation

Resources:
- PC-98 Bible;
- PC-9800 Technical DataBook;
- https://github.com/tmk/tmk_keyboard/wiki/PC-9801-Keyboard;

TODO:
- RTY behaviour
\- triggered in bokosuka when it starts losing a key break along the way.
- key repeat: alternates break and make keys when typematic kicks in, 30ms per swap?
\- Most keyboards don't have a method for disabling typematic, depends on RTY?
\- pc9821ap2: specifically wants F3 to be unmapped for calling setup mode with soft reset:
   HELP key is recognized but code overrides with the F3 path, any way to avoid it?
- Undumped i8048 MCU;
- GRPH + SHIFT scancodes;
- Subclass keyboard variants (cfr. PC-9801-115 Bungo);
- Verify untested keys:
    STOP, COPY, and vf-1 through vf-5
    STOP is correct, verified with branmar2
- Problems with natural keyboard (most nonprinting keys don't work);
- pc9801fs: doesn't capture HELP key even with -119, need to mash for entering setup mode (verify)
- slotify;

**************************************************************************************************/

#include "emu.h"
#include "pc98_kbd.h"
#include "machine/keyboard.ipp"

#define LOG_COMMAND     (1U << 1)

//#include <iostream>

#define VERBOSE (LOG_GENERAL | LOG_COMMAND)
//#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGCOMMAND(...)       LOGMASKED(LOG_COMMAND, __VA_ARGS__)


DEFINE_DEVICE_TYPE(PC98_KBD,     pc98_kbd_device,     "pc98_kbd",     "NEC PC-98 Keyboard")
DEFINE_DEVICE_TYPE(PC98_119_KBD, pc98_119_kbd_device, "pc98_119_kbd", "NEC PC-9801-119 Keyboard")

pc98_kbd_device::pc98_kbd_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_buffered_serial_interface(mconfig, *this)
	, device_matrix_keyboard_interface(mconfig, *this, "KEY0", "KEY1", "KEY2", "KEY3", "KEY4", "KEY5", "KEY6", "KEY7", "KEY8", "KEY9", "KEYA", "KEYB", "KEYC", "KEYD", "KEYE", "KEYF")
	, m_tx_cb(*this)
	, m_kbde_state(0)
	, m_rty_state(0)
{
}

pc98_kbd_device::pc98_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pc98_kbd_device(mconfig, PC98_KBD, tag, owner, clock)
{
}


void pc98_kbd_device::device_validity_check(validity_checker &valid) const
{
}


void pc98_kbd_device::device_start()
{
	// ...
}


void pc98_kbd_device::device_reset()
{
	clear_fifo();
	set_data_frame(START_BIT_COUNT, DATA_BIT_COUNT, PARITY, STOP_BITS);
	set_rcv_rate(BAUD);
	set_tra_rate(BAUD);
	receive_register_reset();
	transmit_register_reset();

	reset_key_state();
	start_processing(attotime::from_hz(BAUD));
	typematic_stop();

//  m_repeat_state = false;
}

uint8_t pc98_kbd_device::translate(uint8_t row, uint8_t column)
{
	return row * 8 + column;
}

static INPUT_PORTS_START( pc9801_kbd )
	PORT_START("KEY0") // 0x00 - 0x07
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("KEY1") // 0x08 - 0x0f
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("- / =") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("^ / `") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^') PORT_CHAR('`')
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(u8"¥ / ¦") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(U'¥','\\') PORT_CHAR(U'¦','|')
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("BACKSPACE") PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(8)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("TAB") PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)

	PORT_START("KEY2") // 0x10 - 0x17
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')

	PORT_START("KEY3") // 0x18 - 0x1f
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("@ / ~") PORT_CODE(KEYCODE_TILDE) PORT_CHAR('@') PORT_CHAR('~')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("[ / {") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')

	PORT_START("KEY4") // 0x20 - 0x27
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("; / +") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(": / *") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')

	PORT_START("KEY5") // 0x28 - 0x2f
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("] / }") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')

	// i feel like PORT_CHAR(UCHAR_MAMEKEY(INVALID)) shouldn't work...
	PORT_START("KEY6") // 0x30 - 0x37
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(", / <") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(". / >") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("/ / ?") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/')  PORT_CHAR('?')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("_") PORT_CHAR(UCHAR_MAMEKEY(INVALID)) PORT_CHAR('_')
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE)  PORT_CHAR(' ')
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("XFER")
	// "ROLL UP / DOWN" are marked as PgDn / PgUp on key sides on most if not all PC-98 keyboards
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("ROLL UP / PgDn") PORT_CODE(KEYCODE_PGDN)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("ROLL DOWN / PgUp") PORT_CODE(KEYCODE_PGUP)

	PORT_START("KEY7") // 0x38 - 0x3f
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("INS") PORT_CODE(KEYCODE_INSERT)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("HOME / CLR") PORT_CODE(KEYCODE_HOME)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("HELP") PORT_CODE(KEYCODE_END)

	PORT_START("KEY8") // 0x40 - 0x47
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("- (PAD)") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("/ (PAD)") PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7 (PAD)") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("8 (PAD)") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("9 (PAD)") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("* (PAD)") PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4 (PAD)") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5 (PAD)") PORT_CODE(KEYCODE_5_PAD)

	PORT_START("KEY9") // 0x48 - 0x4f
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("6 (PAD)") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("+ (PAD)") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1 (PAD)") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2 (PAD)") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3 (PAD)") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("= (PAD)") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0 (PAD)") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(", (PAD)")

	PORT_START("KEYA") // 0x50 - 0x57
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(". (PAD)") PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("NFER")
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("VF1")
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("VF2")
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("VF3")
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("VF4")
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("VF5")
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_UNUSED) //IPT_KEYBOARD) PORT_NAME(" un 2-8")

	PORT_START("KEYB") // 0x58 - 0x5f
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_UNUSED) //IPT_KEYBOARD) PORT_NAME(" un 3-1")
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_UNUSED) //IPT_KEYBOARD) PORT_NAME(" un 3-2")
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_UNUSED) //IPT_KEYBOARD) PORT_NAME(" un 3-3")
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_UNUSED) //IPT_KEYBOARD) PORT_NAME(" un 3-4")
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_UNUSED) //IPT_KEYBOARD) PORT_NAME(" un 3-5")
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_UNUSED) //IPT_KEYBOARD) PORT_NAME(" un 3-6")
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_UNUSED) //IPT_KEYBOARD) PORT_NAME(" un 3-7")
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_UNUSED) //IPT_KEYBOARD) PORT_NAME(" un 3-8")

	PORT_START("KEYC") // 0x60 - 0x67
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("STOP")
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("COPY")
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F6") PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6))

	PORT_START("KEYD") // 0x68 - 0x6f
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F7") PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F8") PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F9") PORT_CODE(KEYCODE_F9) PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F10") PORT_CODE(KEYCODE_F10) PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_UNUSED) //IPT_KEYBOARD) PORT_NAME(" un 5-5")
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_UNUSED) //IPT_KEYBOARD) PORT_NAME(" un 5-6")
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_UNUSED) //IPT_KEYBOARD) PORT_NAME(" un 5-7")
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_UNUSED) //IPT_KEYBOARD) PORT_NAME(" un 5-8")

	PORT_START("KEYE") // 0x70 - 0x77
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("CAPS LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\xe3\x82\xab\xe3\x83\x8a / KANA LOCK") PORT_TOGGLE
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("GRPH / ALT") PORT_CODE(KEYCODE_LALT) PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_UNUSED) //IPT_KEYBOARD) PORT_NAME(" un 6-6")
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_UNUSED) //IPT_KEYBOARD) PORT_NAME(" un 6-7")
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_UNUSED) //IPT_KEYBOARD) PORT_NAME(" un 6-8")

	PORT_START("KEYF") // 0x78 - 0x7f
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_UNUSED) //IPT_KEYBOARD) PORT_NAME(" un 7-1")
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_UNUSED) //IPT_KEYBOARD) PORT_NAME(" un 7-2")
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_UNUSED) //IPT_KEYBOARD) PORT_NAME(" un 7-3")
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_UNUSED) //IPT_KEYBOARD) PORT_NAME(" un 7-4")
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_UNUSED) //IPT_KEYBOARD) PORT_NAME(" un 7-5")
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_UNUSED) //IPT_KEYBOARD) PORT_NAME(" un 7-6")
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_UNUSED) //IPT_KEYBOARD) PORT_NAME(" un 7-7")
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_UNUSED) //IPT_KEYBOARD) PORT_NAME(" un 7-8")
INPUT_PORTS_END

ioport_constructor pc98_kbd_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( pc9801_kbd );
}


//**************************************************************************
//  device_matrix_keyboard
//**************************************************************************

void pc98_kbd_device::key_make(uint8_t row, uint8_t column)
{
	uint8_t code = translate(row, column);

	send_key(code);

	// no typematic for caps and kana locks
	// TODO: does it applies to the whole E row?
	// page 345 of the Technical DataBook for timings
	if (code != 0x71 && code != 0x72)
	{
		//m_repeat_state = 0;
		typematic_start(row, column, attotime::from_msec(500), attotime::from_msec(60));
	}
}

void pc98_kbd_device::key_break(uint8_t row, uint8_t column)
{
	if (typematic_is(row, column))
		typematic_stop();

	uint8_t code = translate(row, column);

	send_key(code | 0x80);
}

void pc98_kbd_device::key_repeat(uint8_t row, uint8_t column)
{
//  uint8_t code = translate(row, column);

//    m_repeat_state ^= 1;
//    code |= m_repeat_state << 7;
//
//    send_key(code);
}

void pc98_kbd_device::send_key(uint8_t code)
{
	transmit_byte(code);
	if (fifo_full())
		stop_processing();
}

//**************************************************************************
//  Serial implementation
//**************************************************************************

void pc98_kbd_device::tra_complete()
{
	if (fifo_full())
		start_processing(attotime::from_hz(BAUD));

	device_buffered_serial_interface::tra_complete();
}

void pc98_kbd_device::transmit_byte(u8 byte)
{
	device_buffered_serial_interface::transmit_byte(byte);
	if (fifo_full())
		stop_processing();
}

/*
 * 0xff: reset
 * everything else: implementation specific, TBD (0x9* command, some have extra parameters)
 */
void pc98_kbd_device::received_byte(u8 byte)
{
	logerror("received_byte 0x%02x\n", byte);
	if (byte == 0xff)
	{
		clear_fifo();
		receive_register_reset();
		transmit_register_reset();

		reset_key_state();
		start_processing(attotime::from_hz(BAUD));
		typematic_stop();
	}
}

void pc98_kbd_device::rcv_complete()
{
	receive_register_extract();
	received_byte(get_received_char());
}

void pc98_kbd_device::input_kbde(int state)
{
	if (!m_kbde_state && state)
		start_processing(attotime::from_hz(BAUD));
	if (m_kbde_state && !state)
		stop_processing();
	m_kbde_state = state;
}

void pc98_kbd_device::input_rty(int state)
{
	m_rty_state = state;
}

/*
 * PC-9801-119 overrides
 */

 pc98_119_kbd_device::pc98_119_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pc98_kbd_device(mconfig, PC98_119_KBD, tag, owner, clock)
{
}

void pc98_119_kbd_device::device_start()
{
	pc98_kbd_device::device_start();

	save_item(NAME(m_cmd_state));
	save_item(NAME(m_key_delay));
	save_pointer(NAME(m_repeat_state), 0x80);
}

void pc98_119_kbd_device::device_reset()
{
	pc98_kbd_device::device_reset();

	m_cmd_state = 0;
	m_key_delay = 500;
	std::fill(std::begin(m_repeat_state), std::end(m_repeat_state), 0);
}

void pc98_119_kbd_device::key_make(uint8_t row, uint8_t column)
{
	uint8_t code = translate(row, column);

	send_key(code);

	// HACK: enable key repeat for the only key that matters for now (HELP -> setup mode)
	// acts wild in any game that uses numpad and keys only (cfr. runners, weaponsf)
	// as if it's expecting typematic being entirely disabled there
	//if (code != 0x71 && code != 0x72)
	if (code == 0x3f)
	{
		m_repeat_state[code] = 0;
		typematic_start(row, column, attotime::from_msec(m_key_delay), attotime::from_msec(60));
	}
}

void pc98_119_kbd_device::key_repeat(uint8_t row, uint8_t column)
{
	uint8_t code = translate(row, column);

	m_repeat_state[code] ^= 1;
	code |= m_repeat_state[code] << 7;

	send_key(code);
}

void pc98_119_kbd_device::received_byte(u8 byte)
{
	const u8 ACK = 0xfa;
	const u8 NACK = 0xfc;

	if (m_cmd_state)
	{
		// ignore same byte, we expect a 0 in bit 7 anyway
		if (m_cmd_state == byte)
			return;

		LOGCOMMAND("Command: [%02x] %02x\n", m_cmd_state, byte);

		switch(m_cmd_state)
		{
			// -xx- ---- key delay
			// -11- ---- 1000 ms
			// -10- ---- 500 ms
			// -01- ---- 500 ms (default)
			// -00- ---- 250 ms
			// ---x xxxx repeat rate (slow 11111 -> 00001 fast)
			// win95: sets 0x70 at startup by default, 0x51 at shutdown
			case 0x9c:
			{
				static const u16 key_delay_ms[] = {250, 500, 500, 1000 };
				m_key_delay = key_delay_ms[(byte >> 5) & 3];
				// TODO: repeat rate
				send_key(ACK);
				break;
			}

			// TODO: caps/kana/num lock handling
			// 0110 ---- reads back LEDs
			// 0111 ---- sets lock state
			// ---- x--- Kana lock
			// ---- -x-- CAPS lock
			// ---- ---x Num lock
			case 0x9d:
				send_key(ACK);
				break;
		}

		m_cmd_state = 0;
		return;
	}

	LOGCOMMAND("Command: %02x\n", byte);

	switch(byte)
	{
		// 0x95 Extended key settings (expects param byte)
		// 0x00 normal mode
		// 0x03 windows & application keys enabled (bitwise?)

		// 0x96 Mode identification, PC9801-98 only?
		// ACK -> 0xa0 -> 0x86 automatic conversion mode
		// ACK -> 0xa0 -> 0x85 normal mode

		// 0x99 <unknown>
		//      returns 0xfa ACK -> 0xfb (not ready?)

		case 0x9c:
			LOGCOMMAND("\t$9c Key Repeat rate settings\n");
			m_cmd_state = byte;
			send_key(ACK);
			break;

		case 0x9d:
			// NOTE: different for PC-9801NS/T
			LOGCOMMAND("\t$9d Keyboard LED settings\n");
			m_cmd_state = byte;
			send_key(ACK);
			break;

		// 0x9e ?, assume it returns ACK

		case 0x9f:
			LOGCOMMAND("\t$9f Keyboard Type ID\n");
			send_key(ACK);
			send_key(0xa0);
			send_key(0x80);
			break;

		default:
			// return a NACK for any unrecognized command
			send_key(NACK);
			if ((byte & 0xf0) == 0x90)
				popmessage("pc98_kbd: unemulated command %02x", byte);
			break;
	}
}
