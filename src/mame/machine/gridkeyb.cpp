// license:BSD-3-Clause
// copyright-holders:Vas Crabb, Sergey Svishchev
/***************************************************************************

    GRiD Compass keyboard HLE, derived from generic_keyboard

    Keycodes from "6 - Interceptor, CCProm, Utils, OS - Sysgen/UTILS/DVLEXEC.PLM"

    Multiple shift key combos not simulated yet.

***************************************************************************/

#include "emu.h"

#include "machine/gridkeyb.h"
#include "machine/keyboard.ipp"



namespace {
u16 const TRANSLATION_TABLE[][4][16] = {
	{   // plain
		{ '`',   '1',   '2',   '3',   '4',   '5',   '6',   '7',   '8',   '9',   '0',   '-',   '=',   0x08U, 0x7fU, 0x1bU },
		{ 0x09U, 'q',   'w',   'e',   'r',   't',   'y',   'u',   'i',   'o',   'p',   '[',   ']',   '\\',  0xffU, 0xffU },
		{ 0xffU, 'a',   's',   'd',   'f',   'g',   'h',   'j',   'k',   'l',   ';',   '\'',  0x0dU, 0xc5U, 0xc4U, 0x0aU },
		{ 0xffU, '\\',  'z',   'x',   'c',   'v',   'b',   'n',   'm',   ',',   '.',   '/',   0xffU, 0xc6U, 0xc7U, ' '   }
	},
	{   // SHIFT
		{ '~',   '!',   '@',   '#',   '$',   '%',   '^',   '&',   '*',   '(',   ')',   '_',   '+',   0x08U, 0x7fU, 0x1bU },
		{ 0x09U, 'Q',   'W',   'E',   'R',   'T',   'Y',   'U',   'I',   'O',   'P',   '{',   '}',   '|',   0xffU, 0xffU },
		{ 0xffU, 'A',   'S',   'D',   'F',   'G',   'H',   'J',   'K',   'L',   ':',   '"',   0x0dU, 0xffU, 0xffU, 0x0aU },
		{ 0xffU, '_',   'Z',   'X',   'C',   'V',   'B',   'N',   'M',   '<',   '>',   '?',   0xffU, 0xffU, 0xffU, ' '   }
	},
	{   // CODE -- partial
		{ 0xffU, '1',   '2',   '3',   '4',   '5',   '6',   '7',   '8',   '9',   '0',   0xffU, 0xffU, 0x88U, 0xffU, 0x9bU },
		{ 0x89U, 0xf1U, 0xf7U, 0xe5U, 0xf2U, 0xf4U, 0xf9U, 0xf5U, 0xe9U, 0xefU, 0xf0U, 0x9bU, 0xffU, 0xffU, 0xffU, 0xffU },
		{ 0xffU, 0xe1U, 0xf3U, 0xe4U, 0xe6U, 0xe7U, 0xe8U, 0xeaU, 0xebU, 0xecU, ';',   '\'',  0x8dU, 0xffU, 0xffU, 0xffU },
		{ 0xffU, 0x1cU, 0xfaU, 0xf8U, 0xe3U, 0xf6U, 0xe2U, 0xeeU, 0xedU, ',',   '.',   0xbfU, 0xffU, 0xd4U, 0xd5U, 0xffU }
	},
	{   // CTRL
		{ 0x00U, '1',   '2',   '3',   '4',   '5',   '6',   '7',   '8',   '9',   '0',   0x1fU, 0x1eU, 0x88U, 0x7fU, 0x1bU },
		{ 0x89U, 0x11U, 0x17U, 0x05U, 0x12U, 0x14U, 0x19U, 0x15U, 0x09U, 0x0fU, 0x10U, 0x9bU, 0x1dU, 0x1cU, 0xffU, 0xffU },
		{ 0xffU, 0x01U, 0x13U, 0x04U, 0x06U, 0x07U, 0x08U, 0x0aU, 0x0bU, 0x0cU, ';',   '\'',  0x8dU, 0xffU, 0xffU, 0x0aU },
		{ 0xffU, 0x1cU, 0x1aU, 0x18U, 0x03U, 0x16U, 0x02U, 0x0eU, 0x0dU, ',',   '.',   0xbfU, 0xffU, 0xffU, 0xffU, 0x00U }
	},
};


bool const CAPS_TABLE[4][16] = {
	{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false },
	{ false, true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  false, false, false, false, false },
	{ false, true,  true,  true,  true,  true,  true,  true,  true,  true,  false, false, false, false, false, false },
	{ false, false, true,  true,  true,  true,  true,  true,  true,  false, false, false, false, false, false, false }
};

} // anonymous namespace



/***************************************************************************
    REUSABLE I/O PORTS
***************************************************************************/

INPUT_PORTS_START( grid_keyboard )
	PORT_START("GRIDKBD_CFG")
	PORT_CONFNAME( 0x0006U, 0x0004U, "Typematic Delay" )
	PORT_CONFSETTING(       0x0000U, "0.25s" )
	PORT_CONFSETTING(       0x0002U, "0.5s"  )
	PORT_CONFSETTING(       0x0004U, "0.75s" )
	PORT_CONFSETTING(       0x0006U, "1.0s"  )
	PORT_CONFNAME( 0x00f8U, 0x0098U, "Typematic Rate" )
	PORT_CONFSETTING(       0x0000U,  "2.0cps" )
	PORT_CONFSETTING(       0x0008U,  "2.1cps" )
	PORT_CONFSETTING(       0x0010U,  "2.5cps" )
	PORT_CONFSETTING(       0x0018U,  "2.7cps" )
	PORT_CONFSETTING(       0x0020U,  "2.0cps" )
	PORT_CONFSETTING(       0x0028U,  "2.1cps" )
	PORT_CONFSETTING(       0x0030U,  "2.5cps" )
	PORT_CONFSETTING(       0x0038U,  "2.7cps" )
	PORT_CONFSETTING(       0x0040U,  "3.3cps" )
	PORT_CONFSETTING(       0x0048U,  "3.8cps" )
	PORT_CONFSETTING(       0x0050U,  "4.0cps" )
	PORT_CONFSETTING(       0x0058U,  "4.3cps" )
	PORT_CONFSETTING(       0x0060U,  "4.6cps" )
	PORT_CONFSETTING(       0x0068U,  "5.0cps" )
	PORT_CONFSETTING(       0x0070U,  "5.5cps" )
	PORT_CONFSETTING(       0x0078U,  "6.0cps" )
	PORT_CONFSETTING(       0x0080U,  "8.0cps" )
	PORT_CONFSETTING(       0x0088U,  "8.6cps" )
	PORT_CONFSETTING(       0x0090U,  "9.2cps" )
	PORT_CONFSETTING(       0x0098U, "10.0cps" )
	PORT_CONFSETTING(       0x00a0U, "10.9cps" )
	PORT_CONFSETTING(       0x00a8U, "12.0cps" )
	PORT_CONFSETTING(       0x00b0U, "13.3cps" )
	PORT_CONFSETTING(       0x00b8U, "15.0cps" )
	PORT_CONFSETTING(       0x00c0U, "16.0cps" )
	PORT_CONFSETTING(       0x00c8U, "17.1cps" )
	PORT_CONFSETTING(       0x00d0U, "18.5cps" )
	PORT_CONFSETTING(       0x00d8U, "20.0cps" )
	PORT_CONFSETTING(       0x00e0U, "21.8cps" )
	PORT_CONFSETTING(       0x00e8U, "24.0cps" )
	PORT_CONFSETTING(       0x00f0U, "26.7cps" )
	PORT_CONFSETTING(       0x00f8U, "30.0cps" )

	PORT_START("GRIDKBD_MOD")
	PORT_BIT( 0x01U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Ctrl")       PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)             PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x02U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Shift")      PORT_CODE(KEYCODE_LSHIFT)   PORT_CODE(KEYCODE_RSHIFT)               PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x04U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Caps Lock")  PORT_CODE(KEYCODE_CAPSLOCK)                             PORT_TOGGLE PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT( 0x08U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Code")       PORT_CODE(KEYCODE_LALT)     PORT_CODE(KEYCODE_RALT)                 PORT_CHAR(UCHAR_MAMEKEY(LALT))

	PORT_START("GRIDKBD_ROW0")
	PORT_BIT( 0x0001U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TILDE)                              PORT_CHAR('`')   PORT_CHAR('~')
	PORT_BIT( 0x0002U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1)                                 PORT_CHAR('1')   PORT_CHAR('!')
	PORT_BIT( 0x0004U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2)                                  PORT_CHAR('2')   PORT_CHAR('@')
	PORT_BIT( 0x0008U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3)                                 PORT_CHAR('3')   PORT_CHAR('#')
	PORT_BIT( 0x0010U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4)                                 PORT_CHAR('4')   PORT_CHAR('$')
	PORT_BIT( 0x0020U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5)                                 PORT_CHAR('5')   PORT_CHAR('%')
	PORT_BIT( 0x0040U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6)                                  PORT_CHAR('6')   PORT_CHAR('^')
	PORT_BIT( 0x0080U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7)                                  PORT_CHAR('7')   PORT_CHAR('&')
	PORT_BIT( 0x0100U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8)                                  PORT_CHAR('8')   PORT_CHAR('*')
	PORT_BIT( 0x0200U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9)                                  PORT_CHAR('9')   PORT_CHAR('(')
	PORT_BIT( 0x0400U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0)                                  PORT_CHAR('0')   PORT_CHAR('(')
	PORT_BIT( 0x0800U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS)                              PORT_CHAR('-')   PORT_CHAR('_')
	PORT_BIT( 0x1000U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS)                             PORT_CHAR('=')   PORT_CHAR('+')
	PORT_BIT( 0x2000U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSPACE)  PORT_NAME("Backspace") PORT_CHAR(0x08U)
	PORT_BIT( 0x4000U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL)        PORT_NAME("Del")       PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT( 0x8000U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ESC)        PORT_NAME("Escape")    PORT_CHAR(UCHAR_MAMEKEY(ESC))

	PORT_START("GRIDKBD_ROW1")
	PORT_BIT( 0x0001U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TAB)                               PORT_CHAR('\t')
	PORT_BIT( 0x0002U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)                                 PORT_CHAR('q')   PORT_CHAR('Q')
	PORT_BIT( 0x0004U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)                                 PORT_CHAR('w')   PORT_CHAR('W')
	PORT_BIT( 0x0008U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E)                                 PORT_CHAR('e')   PORT_CHAR('E')
	PORT_BIT( 0x0010U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)                                 PORT_CHAR('r')   PORT_CHAR('R')
	PORT_BIT( 0x0020U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T)                                 PORT_CHAR('t')   PORT_CHAR('T')
	PORT_BIT( 0x0040U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y)                                 PORT_CHAR('y')   PORT_CHAR('Y')
	PORT_BIT( 0x0080U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U)                                 PORT_CHAR('u')   PORT_CHAR('U')
	PORT_BIT( 0x0100U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I)                                 PORT_CHAR('i')   PORT_CHAR('I')
	PORT_BIT( 0x0200U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O)                                 PORT_CHAR('o')   PORT_CHAR('O')
	PORT_BIT( 0x0400U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P)                                 PORT_CHAR('p')   PORT_CHAR('P')
	PORT_BIT( 0x0800U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE)                          PORT_CHAR('[')   PORT_CHAR('{')
	PORT_BIT( 0x1000U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE)                         PORT_CHAR(']')   PORT_CHAR('}')
	PORT_BIT( 0x2000U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH)                          PORT_CHAR('\\')  PORT_CHAR('|')
	PORT_BIT( 0x4000U, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x8000U, IP_ACTIVE_HIGH, IPT_UNUSED   )

	PORT_START("GRIDKBD_ROW2")
	PORT_BIT( 0x0001U, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x0002U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A)                                 PORT_CHAR('a')   PORT_CHAR('A')
	PORT_BIT( 0x0004U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S)                                 PORT_CHAR('s')   PORT_CHAR('S')
	PORT_BIT( 0x0008U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D)                                 PORT_CHAR('d')   PORT_CHAR('D')
	PORT_BIT( 0x0010U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F)                                 PORT_CHAR('f')   PORT_CHAR('F')
	PORT_BIT( 0x0020U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G)                                 PORT_CHAR('g')   PORT_CHAR('G')
	PORT_BIT( 0x0040U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H)                                 PORT_CHAR('h')   PORT_CHAR('H')
	PORT_BIT( 0x0080U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J)                                 PORT_CHAR('j')   PORT_CHAR('J')
	PORT_BIT( 0x0100U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K)                                 PORT_CHAR('k')   PORT_CHAR('K')
	PORT_BIT( 0x0200U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L)                                 PORT_CHAR('l')   PORT_CHAR('L')
	PORT_BIT( 0x0400U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)                              PORT_CHAR(';')   PORT_CHAR(':')
	PORT_BIT( 0x0800U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE)                              PORT_CHAR('\'')  PORT_CHAR('"')
	PORT_BIT( 0x1000U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER)      PORT_NAME("Return")    PORT_CHAR(0x0dU)
	PORT_BIT( 0x2000U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_UP)         PORT_NAME("UpArrow")
	PORT_BIT( 0x4000U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DOWN)       PORT_NAME("DownArrow")
	PORT_BIT( 0x8000U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_INSERT)     PORT_NAME("Linefeed")  PORT_CHAR(UCHAR_MAMEKEY(INSERT))

	PORT_START("GRIDKBD_ROW3")
	PORT_BIT( 0x0001U, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x0002U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH2)                        PORT_CHAR('\\')  PORT_CHAR('_')
	PORT_BIT( 0x0004U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z)                                 PORT_CHAR('z')   PORT_CHAR('Z')
	PORT_BIT( 0x0008U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X)                                 PORT_CHAR('x')   PORT_CHAR('X')
	PORT_BIT( 0x0010U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C)                                 PORT_CHAR('c')   PORT_CHAR('C')
	PORT_BIT( 0x0020U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V)                                 PORT_CHAR('v')   PORT_CHAR('V')
	PORT_BIT( 0x0040U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B)                                 PORT_CHAR('b')   PORT_CHAR('B')
	PORT_BIT( 0x0080U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N)                                 PORT_CHAR('n')   PORT_CHAR('N')
	PORT_BIT( 0x0100U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M)                                 PORT_CHAR('m')   PORT_CHAR('M')
	PORT_BIT( 0x0200U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA)                             PORT_CHAR(',')   PORT_CHAR('<')
	PORT_BIT( 0x0400U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP)                              PORT_CHAR('.')   PORT_CHAR('>')
	PORT_BIT( 0x0800U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH)                             PORT_CHAR('/')   PORT_CHAR('?')
	PORT_BIT( 0x1000U, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x2000U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LEFT)       PORT_NAME("LeftArrow")
	PORT_BIT( 0x4000U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RIGHT)      PORT_NAME("RightArrow")
	PORT_BIT( 0x8000U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE)                             PORT_CHAR(' ')
INPUT_PORTS_END



/***************************************************************************
    DEVICE TYPE GLOBALS
***************************************************************************/

DEFINE_DEVICE_TYPE(GRID_KEYBOARD, grid_keyboard_device, "grid_keyboard", "GRiD Compass Keyboard")



/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

grid_keyboard_device::grid_keyboard_device(
		machine_config const &mconfig,
		device_type type,
		char const *tag,
		device_t *owner,
		u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_matrix_keyboard_interface(mconfig, *this, "GRIDKBD_ROW0", "GRIDKBD_ROW1", "GRIDKBD_ROW2", "GRIDKBD_ROW3")
	, m_config(*this, "GRIDKBD_CFG")
	, m_modifiers(*this, "GRIDKBD_MOD")
	, m_last_modifiers(0U)
	, m_keyboard_cb(*this)
{
}


grid_keyboard_device::grid_keyboard_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: grid_keyboard_device(mconfig, GRID_KEYBOARD, tag, owner, clock)
{
}


ioport_constructor grid_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(grid_keyboard);
}


void grid_keyboard_device::device_start()
{
	m_keyboard_cb.resolve();

	save_item(NAME(m_last_modifiers));
}


void grid_keyboard_device::device_reset()
{
	reset_key_state();
	m_last_modifiers = 0;

	start_processing(attotime::from_hz(2'400));
	typematic_stop();
}


void grid_keyboard_device::key_make(u8 row, u8 column)
{
	send_translated((row << 4) | column);
	typematic_start(row, column, typematic_delay(), typematic_period());
}


void grid_keyboard_device::key_repeat(u8 row, u8 column)
{
	send_translated((row << 4) | column);
}


void grid_keyboard_device::send_key(u16 code)
{
	m_keyboard_cb(code);
}


bool grid_keyboard_device::translate(u8 code, u16 &translated) const
{
	unsigned const row((code >> 4) & 0x03U);
	unsigned const col((code >> 0) & 0x0fU);

	u16 const modifiers(m_modifiers->read());
	bool const shift(bool(modifiers & 0x02U) != (bool(modifiers & 0x04U) && CAPS_TABLE[row][col]));
	bool const ctrl(modifiers & 0x01U);
	bool const meta(modifiers & 0x08U);

	unsigned const map(ctrl ? 3U : meta ? 2U : shift ? 1U : 0U);
	u16 const result(TRANSLATION_TABLE[map][row][col]);
	if (result == u8(~0U))
	{
		return false;
	}
	else
	{
		translated = result;
		return true;
	}
}


void grid_keyboard_device::will_scan_row(u8 row)
{
	u16 const modifiers(m_modifiers->read());
	if (modifiers != m_last_modifiers)
		typematic_restart(typematic_delay(), typematic_period());

	m_last_modifiers = modifiers;
}


void grid_keyboard_device::send_translated(u8 code)
{
	u16 translated;
	if (translate(code, translated))
		send_key(translated);
}


attotime grid_keyboard_device::typematic_delay() const
{
	return attotime::from_msec(250 * (1 + ((m_config->read() >> 1) & 0x03U)));
}


attotime grid_keyboard_device::typematic_period() const
{
	unsigned const rate(~(m_config->read() >> 3) & 0x1fU);
	return attotime::from_ticks((8U + (rate & 0x07U)) * (1 << ((rate >> 3) & 0x03)), 240U);
}
