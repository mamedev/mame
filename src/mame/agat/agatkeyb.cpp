// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    Agat keyboard HLE

***************************************************************************/

#include "emu.h"

#include "agatkeyb.h"
#include "machine/keyboard.ipp"
#include "utf8.h"


/***************************************************************************
    REUSABLE I/O PORTS
***************************************************************************/

INPUT_PORTS_START( agat_keyboard )
	PORT_START("AGATKBD_MOD")
	PORT_BIT(0x0001U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Ctrl")  PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)             PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT(0x0002U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT)   PORT_CODE(KEYCODE_RSHIFT)               PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x0004U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Meta")  PORT_CODE(KEYCODE_LALT)     PORT_CHAR(UCHAR_MAMEKEY(LALT)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(agat_keyboard_device::meta_changed), 0)
	PORT_BIT(0x0008U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F10)      PORT_CHAR(UCHAR_MAMEKEY(F10)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(agat_keyboard_device::reset_changed), 0)

	PORT_START("AGATKBD_ROW0")
	PORT_BIT(0x0001U, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT(0x0002U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num 0") PORT_CODE(KEYCODE_0_PAD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x0004U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num .") PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT(0x0008U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num =") PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT(0x0010U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F1") PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))
	PORT_BIT(0x0020U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F2") PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))
	PORT_BIT(0x0040U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F3") PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(0x0080U, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT(0x0100U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(UTF8_LEFT)  PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x0200U, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT(0x0400U, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT(0x0800U, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT(0x1000U, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT(0x2000U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Return")   PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x4000U, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT(0x8000U, IP_ACTIVE_HIGH, IPT_UNUSED   )

	PORT_START("AGATKBD_ROW1")
	PORT_BIT(0x0001U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num 1") PORT_CODE(KEYCODE_7_PAD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x0002U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num 2") PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x0004U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num 3") PORT_CODE(KEYCODE_9_PAD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x0008U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num 4") PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x0010U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num 5") PORT_CODE(KEYCODE_5_PAD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x0020U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x0040U, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT(0x0080U, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT(0x0100U, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT(0x0200U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(UTF8_UP)    PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x0400U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(UTF8_DOWN)  PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x0800U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ESC)        PORT_NAME("Escape")    PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x1000U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num 6") PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x2000U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num 7") PORT_CODE(KEYCODE_1_PAD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x4000U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num 8") PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x8000U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num 9") PORT_CODE(KEYCODE_3_PAD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD))

	PORT_START("AGATKBD_ROW2")
	PORT_BIT(0x0001U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE)                             PORT_CHAR(' ')
	PORT_BIT(0x0002U, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT(0x0004U, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT(0x0008U, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT(0x0010U, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT(0x0020U, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT(0x0040U, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT(0x0080U, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT(0x0100U, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT(0x0200U, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT(0x0400U, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT(0x0800U, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT(0x1000U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA)                             PORT_CHAR(',')   PORT_CHAR('<')
	PORT_BIT(0x2000U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS)                             PORT_CHAR('-')   PORT_CHAR('=')
	PORT_BIT(0x4000U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP)                              PORT_CHAR('.')   PORT_CHAR('>')
	PORT_BIT(0x8000U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH)                             PORT_CHAR('/')   PORT_CHAR('?')

	PORT_START("AGATKBD_ROW3")
	PORT_BIT(0x0001U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0)                                 PORT_CHAR('0')
	PORT_BIT(0x0002U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1)                                 PORT_CHAR('1')   PORT_CHAR('!')
	PORT_BIT(0x0004U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2)                                 PORT_CHAR('2')   PORT_CHAR('"')
	PORT_BIT(0x0008U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3)                                 PORT_CHAR('3')   PORT_CHAR('#')
	PORT_BIT(0x0010U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4)                                 PORT_CHAR('4')   PORT_CHAR('$')
	PORT_BIT(0x0020U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5)                                 PORT_CHAR('5')   PORT_CHAR('%')
	PORT_BIT(0x0040U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6)                                 PORT_CHAR('6')   PORT_CHAR('&')
	PORT_BIT(0x0080U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7)                                 PORT_CHAR('7')   PORT_CHAR('\'')
	PORT_BIT(0x0100U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8)                                 PORT_CHAR('8')   PORT_CHAR('(')
	PORT_BIT(0x0200U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9)                                 PORT_CHAR('9')   PORT_CHAR(')')
	PORT_BIT(0x0400U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE)                             PORT_CHAR(':')   PORT_CHAR('*')
	PORT_BIT(0x0800U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)                             PORT_CHAR(';')   PORT_CHAR('+')
	PORT_BIT(0x1000U, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT(0x2000U, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT(0x4000U, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT(0x8000U, IP_ACTIVE_HIGH, IPT_UNUSED   )

	PORT_START("AGATKBD_ROW4")
	PORT_BIT(0x0001U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TAB)                               PORT_CHAR('@')
	PORT_BIT(0x0002U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A)                                 PORT_CHAR('A')   PORT_CHAR('a')
	PORT_BIT(0x0004U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B)                                 PORT_CHAR('B')   PORT_CHAR('b')
	PORT_BIT(0x0008U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C)                                 PORT_CHAR('C')   PORT_CHAR('c')
	PORT_BIT(0x0010U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D)                                 PORT_CHAR('D')   PORT_CHAR('d')
	PORT_BIT(0x0020U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E)                                 PORT_CHAR('E')   PORT_CHAR('e')
	PORT_BIT(0x0040U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F)                                 PORT_CHAR('F')   PORT_CHAR('f')
	PORT_BIT(0x0080U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G)                                 PORT_CHAR('G')   PORT_CHAR('g')
	PORT_BIT(0x0100U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H)                                 PORT_CHAR('H')   PORT_CHAR('h')
	PORT_BIT(0x0200U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I)                                 PORT_CHAR('I')   PORT_CHAR('i')
	PORT_BIT(0x0400U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J)                                 PORT_CHAR('J')   PORT_CHAR('j')
	PORT_BIT(0x0800U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K)                                 PORT_CHAR('K')   PORT_CHAR('k')
	PORT_BIT(0x1000U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L)                                 PORT_CHAR('L')   PORT_CHAR('l')
	PORT_BIT(0x2000U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M)                                 PORT_CHAR('M')   PORT_CHAR('m')
	PORT_BIT(0x4000U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N)                                 PORT_CHAR('N')   PORT_CHAR('n')
	PORT_BIT(0x8000U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O)                                 PORT_CHAR('O')   PORT_CHAR('o')

	PORT_START("AGATKBD_ROW5")
	PORT_BIT(0x0001U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P)                                 PORT_CHAR('P')   PORT_CHAR('p')
	PORT_BIT(0x0002U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)                                 PORT_CHAR('Q')   PORT_CHAR('q')
	PORT_BIT(0x0004U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)                                 PORT_CHAR('R')   PORT_CHAR('r')
	PORT_BIT(0x0008U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S)                                 PORT_CHAR('S')   PORT_CHAR('s')
	PORT_BIT(0x0010U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T)                                 PORT_CHAR('T')   PORT_CHAR('t')
	PORT_BIT(0x0020U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U)                                 PORT_CHAR('U')   PORT_CHAR('u')
	PORT_BIT(0x0040U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V)                                 PORT_CHAR('V')   PORT_CHAR('v')
	PORT_BIT(0x0080U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)                                 PORT_CHAR('W')   PORT_CHAR('w')
	PORT_BIT(0x0100U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X)                                 PORT_CHAR('X')   PORT_CHAR('x')
	PORT_BIT(0x0200U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y)                                 PORT_CHAR('Y')   PORT_CHAR('y')
	PORT_BIT(0x0400U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z)                                 PORT_CHAR('Z')   PORT_CHAR('z')
	PORT_BIT(0x0800U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE)                         PORT_CHAR('[')   PORT_CHAR('{')
	PORT_BIT(0x1000U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH)                         PORT_CHAR('\\')  PORT_CHAR('|')
	PORT_BIT(0x2000U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE)                        PORT_CHAR(']')   PORT_CHAR('}')
	PORT_BIT(0x4000U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TILDE)                             PORT_CHAR('^')
	PORT_BIT(0x8000U, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS)                            PORT_CHAR('_')
INPUT_PORTS_END



/***************************************************************************
    DEVICE TYPE GLOBALS
***************************************************************************/

DEFINE_DEVICE_TYPE(AGAT_KEYBOARD, agat_keyboard_device, "agat_keyboard", "Agat Keyboard")



/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

agat_keyboard_device::agat_keyboard_device(
		machine_config const &mconfig,
		device_type type,
		char const *tag,
		device_t *owner,
		u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_matrix_keyboard_interface(mconfig, *this,
		"AGATKBD_ROW0", "AGATKBD_ROW1", "AGATKBD_ROW2", "AGATKBD_ROW3", "AGATKBD_ROW4", "AGATKBD_ROW5")
	, m_modifiers(*this, "AGATKBD_MOD")
	, m_last_modifiers(0U)
	, m_keyboard_cb(*this)
	, m_out_meta_cb(*this)
	, m_out_reset_cb(*this)
{
}


agat_keyboard_device::agat_keyboard_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: agat_keyboard_device(mconfig, AGAT_KEYBOARD, tag, owner, clock)
{
}


ioport_constructor agat_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(agat_keyboard);
}


void agat_keyboard_device::device_start()
{
	save_item(NAME(m_last_modifiers));
	save_item(NAME(m_meta));
}


void agat_keyboard_device::device_reset()
{
	reset_key_state();
	m_last_modifiers = 0;
	m_meta = false;

	start_processing(attotime::from_hz(1'000)); // XXX
	typematic_stop();
}


void agat_keyboard_device::key_make(u8 row, u8 column)
{
	send_translated((row << 4) | column);
	typematic_start(row, column, typematic_delay(), typematic_period());
}


void agat_keyboard_device::key_repeat(u8 row, u8 column)
{
	send_translated((row << 4) | column);
}


INPUT_CHANGED_MEMBER(agat_keyboard_device::meta_changed)
{
	if (oldval)
	{
		m_meta ^= 1;
		m_out_meta_cb(m_meta);
	}
}

INPUT_CHANGED_MEMBER(agat_keyboard_device::reset_changed)
{
	if ((m_modifiers->read() & 9) == 9)
	{
		m_out_reset_cb(true);
	}
}


bool agat_keyboard_device::translate(u8 code, u16 &translated) const
{
	u16 const modifiers(m_modifiers->read());
	bool const ctrl(modifiers & 0x01U);
	bool const shift(modifiers & 0x02U);

	if (shift)
	{
		if (code == 0x20)
		{
			translated = code;
		}
		else
		{
			translated = code < 0x40 ? (code ^ 0x10) : (code ^ 0x20);
		}
	}
	else
	{
		translated = (code < 0x20) ? code : (ctrl ? (code & 0x1f) : code);
	}
//  logerror("code %02x c %d s %d m %d -> %02x\n", code, ctrl, shift, m_meta, translated);
	return true;
}


void agat_keyboard_device::will_scan_row(u8 row)
{
	u16 const modifiers(m_modifiers->read());
	if (modifiers != m_last_modifiers)
		typematic_restart(typematic_delay(), typematic_period());

	m_last_modifiers = modifiers;
}


void agat_keyboard_device::send_translated(u8 code)
{
	u16 translated;
	if (translate(code, translated))
		m_keyboard_cb(translated);
}


attotime agat_keyboard_device::typematic_delay() const
{
	return attotime::from_msec(250); // XXX
}


attotime agat_keyboard_device::typematic_period() const
{
	return attotime::from_hz(10);
}
