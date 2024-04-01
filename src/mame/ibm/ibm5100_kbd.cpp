// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * A high level emulation of the IBM 5100 keyboard.
 *
 * Sources:
 *  - IBM 5100 Maintenance Information Manual, SY31-0405-3, Fourth Edition (October 1979), International Business Machines Corporation
 *
 * TODO:
 *  - missing key names and chars
 *
 * For columns 0..15, the scan code format is SrrCcccc where:
 *
 *  S     = shift modifier (1=active)
 *  rr    = matrix row number
 *  C     = command modifier (1=active)
 *  cccc  = matrix column number
 *
 * For columns > 15, the scan code format is 1rrccCSc where:
 *
 *  S     = shift modifier (0=active)
 *  rr    = matrix row number
 *  C     = command modifier (1=active)
 *  cc..c = matrix column number
 *
 * This scheme disallows modifier combinations, with shift taking priority over
 * command. Bits 5 and 6 of the scan code are swapped when output.
 *
 * Regular alpha, numeric and keypad keys are easily mapped 1:1 with a standard
 * modern keyboard. Additional keys are mapped by default as follows:
 *
 *  5100 Key    Mapping        Rationale
 *  --------    -------        ---------
 *  +/-         -              closest key position, matching shifted symbol
 *  ×/÷         =              closest key position
 *  ←/→         ;              moved to row with two non-alpha keys
 *  =           '              moved to row with two non-alpha keys
 *  [/(         [              moved to row with three non-alpha keys, matching unshifted symbol
 *  ]/)         ]              moved to row with three non-alpha keys, matching unshifted symbol
 *  #/@         \              moved to row with three non-alpha keys
 *  &/$         `              available standard symbol key
 *
 *  l/r/u/d     arrow keys     conceptual match
 *  Attention   Delete         available non-symbol, non-modal key
 *  Hold        Backspace      available non-symbol, non-modal key
 *  Execute     Enter          conceptual match
 *  Command     L/R Control    conceptual match
 */

#include "emu.h"
#include "ibm5100_kbd.h"

#include "machine/keyboard.ipp"

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(IBM5100_KEYBOARD, ibm5100_keyboard_device, "ibm5100_keyboard", "IBM 5100 Keyboard")

INPUT_PORTS_START(ibm5100_keyboard)
	PORT_START("modifiers")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)
												 PORT_CODE(KEYCODE_RSHIFT)     PORT_NAME("Shift")            PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)
												 PORT_CODE(KEYCODE_RCONTROL)   PORT_NAME("Command")          PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))

	PORT_START("col.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_NAME(u8"9 \u2228")       PORT_CHAR('9')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)          PORT_NAME(u8"I \u2373")       PORT_CHAR('I')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)          PORT_NAME(u8"K '")            PORT_CHAR('K') PORT_CHAR('\'')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)                                    PORT_CHAR(',') PORT_CHAR(';')

	PORT_START("col.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_NAME(u8"8 \u2260")       PORT_CHAR('8')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)          PORT_NAME(u8"U \u2193")       PORT_CHAR('U')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)          PORT_NAME(u8"J \u2218")       PORT_CHAR('J')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)          PORT_NAME(u8"M \u2223")       PORT_CHAR('M')

	PORT_START("col.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_NAME(u8"× ÷")            // FIXME: PORT_CHAR?
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_NAME(u8"\u2190 \u2192")  // FIXME: PORT_CHAR?
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)                               PORT_CHAR(']')  PORT_CHAR(')')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)                                    PORT_CHAR('/')  PORT_CHAR('\\')

	PORT_START("col.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)                                    PORT_CHAR('+') PORT_CHAR('-')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)          PORT_NAME(u8"P \u22c6")       PORT_CHAR('P')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)                                PORT_CHAR('[')  PORT_CHAR('(')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)      PORT_NAME("Space")            PORT_CHAR(UCHAR_MAMEKEY(SPACE))

	PORT_START("col.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)         PORT_NAME("Up")               PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)                                    PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)                                    PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)                                    PORT_CHAR(UCHAR_MAMEKEY(2_PAD))

	PORT_START("col.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)                                    PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)                                    PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)                                    PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)                                    PORT_CHAR(UCHAR_MAMEKEY(0_PAD))

	PORT_START("col.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD)                                PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK)   PORT_NAME("Keypad *")         PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD)                                PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD)                                 PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))

	PORT_START("col.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)                                    PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)                                    PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)                                    PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)    PORT_NAME("Keypad .")         PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))

	PORT_START("col.8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)          PORT_NAME(u8"6 \u2265")       PORT_CHAR('6')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)          PORT_NAME(u8"T \u223c")       PORT_CHAR('T')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)          PORT_NAME(u8"G \u2207")       PORT_CHAR('G')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)          PORT_NAME(u8"B \u22a5")       PORT_CHAR('B')

	PORT_START("col.9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)          PORT_NAME(u8"7 >")            PORT_CHAR('7') PORT_CHAR('>')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)          PORT_NAME(u8"Y \u2191")       PORT_CHAR('Y')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)          PORT_NAME(u8"H \u2206")       PORT_CHAR('H')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)          PORT_NAME(u8"N \u22a4")       PORT_CHAR('N')

	PORT_START("col.a")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)          PORT_NAME(u8"4 \u2264")       PORT_CHAR('4')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)          PORT_NAME(u8"E \u220a")       PORT_CHAR('E')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)          PORT_NAME(u8"D \u230a")       PORT_CHAR('D')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)          PORT_NAME(u8"C \u2229")       PORT_CHAR('C')

	PORT_START("col.b")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)          PORT_NAME(u8"5 =")            PORT_CHAR('5')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)          PORT_NAME(u8"R \u2374")       PORT_CHAR('R')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)          PORT_NAME(u8"F _")            PORT_CHAR('F') PORT_CHAR('_')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)          PORT_NAME(u8"V \u222a")       PORT_CHAR('V')

	PORT_START("col.c")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)          PORT_NAME(u8"3 <")            PORT_CHAR('3') PORT_CHAR('<')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)          PORT_NAME(u8"W \u2375")       PORT_CHAR('W')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)          PORT_NAME(u8"S \u2308")       PORT_CHAR('S')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)          PORT_NAME(u8"X \u2283")       PORT_CHAR('X')

	PORT_START("col.d")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)       PORT_NAME("Down")             PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)          PORT_NAME(u8"1 ¨")            PORT_CHAR('1')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)      PORT_NAME("&")                PORT_CHAR('&') PORT_CHAR('$')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("col.e")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)          PORT_NAME(u8"0 \u2227")       PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)          PORT_NAME(u8"O \u25cb")       PORT_CHAR('O')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)          PORT_NAME(u8"L \u2395")       PORT_CHAR('L')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)                                     PORT_CHAR('.') PORT_CHAR(':')

	PORT_START("col.f")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)          PORT_NAME(u8"2 \u203e")       PORT_CHAR('2')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)          PORT_NAME(u8"Q ?")            PORT_CHAR('Q') PORT_CHAR('?')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)          PORT_NAME(u8"A \u237a")       PORT_CHAR('A')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)          PORT_NAME(u8"Z \u2282")       PORT_CHAR('Z')

	PORT_START("col.10")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)        PORT_NAME("Attention")        PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)      PORT_NAME("Right")            PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)
												 PORT_CODE(KEYCODE_ENTER_PAD)  PORT_NAME("Execute")          PORT_CHAR(13)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("col.11")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)  PORT_NAME("Hold")             PORT_CHAR(8)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)       PORT_NAME("Left")             PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)      PORT_NAME("Equals")           PORT_CHAR('=')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)  PORT_NAME("# @")              PORT_CHAR('#') PORT_CHAR('@')
INPUT_PORTS_END

static const std::pair<u8, u8>typamatic_keys[] =
{
	std::make_pair( 3, 3), // space
	std::make_pair( 4, 0), // cursor up
	std::make_pair(13, 0), // cursor down
	std::make_pair(16, 1), // cursor right
	std::make_pair(17, 1), // cursor left
};

ibm5100_keyboard_device::ibm5100_keyboard_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, IBM5100_KEYBOARD, tag, owner, clock)
	, device_matrix_keyboard_interface(mconfig, *this
		, "col.0", "col.1", "col.2", "col.3", "col.4", "col.5", "col.6", "col.7"
		, "col.8", "col.9", "col.a", "col.b", "col.c", "col.d", "col.e", "col.f"
		, "col.10", "col.11")
	, m_strobe(*this)
	, m_modifiers(*this, "modifiers")
	, m_lock(false)
{
}

void ibm5100_keyboard_device::device_start()
{
	save_item(NAME(m_scan));
	save_item(NAME(m_typamatic));
	save_item(NAME(m_lock));
}

void ibm5100_keyboard_device::device_reset()
{
	m_scan = 0;
	m_strobe(1);
	m_typamatic = false;

	reset_key_state();
	start_processing(attotime::from_hz(25'000));
	typematic_stop();
}

void ibm5100_keyboard_device::key_make(u8 row, u8 column)
{
	if (m_lock)
		return;

	m_scan = translate(row, column);

	LOG("key_make row %d column %d scan 0x%02x\n", row, column, m_scan);

	m_strobe(0);
	m_strobe(1);

	// only some keys have typamatic action
	if (std::find(std::begin(typamatic_keys), std::end(typamatic_keys), std::make_pair(row, column)) != std::end(typamatic_keys))
		typematic_start(row, column, attotime::from_msec(700), attotime::from_msec(100));
}

void ibm5100_keyboard_device::key_break(u8 row, u8 column)
{
	if (typematic_is(row, column))
		typematic_stop();

	m_scan = 0;
}

void ibm5100_keyboard_device::key_repeat(u8 row, u8 column)
{
	if (m_typamatic)
	{
		m_scan = translate(row, column);

		m_strobe(0);
		m_strobe(1);
	}
}

// column and row are swapped with respect device_matrix_keyboard_interface arguments
u8 ibm5100_keyboard_device::translate(u8 column, u8 row)
{
	// compute basic scan code with bits 5 and 6 swapped
	u8 data = bitswap<8>(row << 5 | column, 7, 5, 6, 4, 3, 2, 1, 0);

	u8 const modifiers = m_modifiers->read();

	// modifiers are applied differently for columns > 15
	if (column < 16)
	{
		if (BIT(modifiers, 0))
			// shift
			data |= 0x80;
		else if (BIT(modifiers, 1))
			// command
			data |= 0x10;
	}
	else
	{
		data |= 0x82;

		if (BIT(modifiers, 0))
			// shift
			data &= ~0x02;
		else if (BIT(modifiers, 1))
			// command
			data |= 0x04;
	}

	return data;
}

void ibm5100_keyboard_device::typamatic_w(int state)
{
	m_typamatic = state;
}

void ibm5100_keyboard_device::lock_w(int state)
{
	m_lock = !state;
}

ioport_constructor ibm5100_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(ibm5100_keyboard);
}
