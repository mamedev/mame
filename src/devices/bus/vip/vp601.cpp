// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA VP-601/VP-611 ASCII Keyboard emulation

    The keyboard is scanned by a CD4520 counter through two CD4051
    multiplexers and encoded by a CDP1832 mask ROM (U4), which is not
    dumped. The encoding below follows the Key / Output Character Table
    and the timing diagram in the VP-601/VP-611 instruction manual.

    TODO:

    - dump the CDP1832 encoder ROM
    - three VP-611 numeric keypad keys are not documented
    - USER 1/USER 2 switches
    - key click

**********************************************************************/

#include "emu.h"
#include "vp601.h"

#include <bit>



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

namespace {

constexpr int DEBOUNCE_MS = 15;
constexpr int STROBE_MS = 15;

enum : uint8_t
{
	NUL = 0x00,
	BS = 0x08,
	LF = 0x0a,
	CR = 0x0d,
	ESC = 0x1b,
	FS = 0x1c,
	GS = 0x1d,
	RS = 0x1e,
	US = 0x1f,
	SP = 0x20,
	DEL = 0x7f
};

// normal, shift, control
constexpr uint8_t KEY_CODES[9 * 8][3] =
{
	{ '1', '!', NUL }, { '2', '"', NUL }, { '3', '#', NUL }, { '4', '$', NUL }, { '5', '%', NUL }, { '6', '&', NUL }, { '7', '\'', NUL }, { '8', '(', NUL },
	{ '9', ')', NUL }, { '0', '0', NUL }, { ':', '*', NUL }, { '-', '=', GS }, { '^', '~', NUL }, { DEL, '_', DEL }, { BS, BS, BS }, { ESC, ESC, ESC },
	{ 'q', 'Q', 0x11 }, { 'w', 'W', 0x17 }, { 'e', 'E', 0x05 }, { 'r', 'R', 0x12 }, { 't', 'T', 0x14 }, { 'y', 'Y', 0x19 }, { 'u', 'U', 0x15 }, { 'i', 'I', 0x09 },
	{ 'o', 'O', 0x0f }, { 'p', 'P', 0x10 }, { '@', '`', NUL }, { '[', '{', ESC }, { LF, LF, LF }, { 'a', 'A', 0x01 }, { 's', 'S', 0x13 }, { 'd', 'D', 0x04 },
	{ 'f', 'F', 0x06 }, { 'g', 'G', 0x07 }, { 'h', 'H', 0x08 }, { 'j', 'J', 0x0a }, { 'k', 'K', 0x0b }, { 'l', 'L', 0x0c }, { ';', '+', NUL }, { ']', '}', NUL },
	{ 'z', 'Z', 0x1a }, { 'x', 'X', 0x18 }, { 'c', 'C', 0x03 }, { 'v', 'V', 0x16 }, { 'b', 'B', 0x02 }, { 'n', 'N', 0x0e }, { 'm', 'M', 0x0d }, { ',', '<', FS },
	{ '.', '>', RS }, { '/', '?', US }, { '\\', '|', NUL }, { SP, SP, SP }, { CR, CR, CR }, { NUL, NUL, NUL }, { NUL, NUL, NUL }, { NUL, NUL, NUL },
	{ '0', '0', NUL }, { '1', '!', NUL }, { '2', '"', NUL }, { '3', '#', NUL }, { '4', '$', NUL }, { '5', '%', NUL }, { '6', '&', NUL }, { '7', '\'', NUL },
	{ '8', '(', NUL }, { '9', ')', NUL }, { '+', '+', '+' }, { '*', '*', '*' }, { '=', '=', '=' }, { NUL, NUL, NUL }, { NUL, NUL, NUL }, { NUL, NUL, NUL }
};

} // anonymous namespace



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VIP_ASCII_KEYBOARD_PORT, vip_ascii_keyboard_port_device, "vip_ascii_keyboard_port", "VIP ASCII keyboard port")
DEFINE_DEVICE_TYPE(VP601, vp601_device, "vp601", "VP-601 ASCII Keyboard")
DEFINE_DEVICE_TYPE(VP611, vp611_device, "vp611", "VP-611 ASCII/Numeric Keyboard")



//**************************************************************************
//  PORT DEVICE
//**************************************************************************

vip_ascii_keyboard_port_device::vip_ascii_keyboard_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VIP_ASCII_KEYBOARD_PORT, tag, owner, clock),
	device_single_card_slot_interface<device_vip_ascii_keyboard_interface>(mconfig, *this),
	m_write_data(*this),
	m_write_st(*this),
	m_write_kd(*this)
{
}

void vip_ascii_keyboard_port_device::device_start()
{
}

device_vip_ascii_keyboard_interface::device_vip_ascii_keyboard_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "vipasciikbd")
{
	m_port = dynamic_cast<vip_ascii_keyboard_port_device *>(device.owner());
}



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

static INPUT_PORTS_START( vp601 )
	PORT_START("ROW0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')

	PORT_START("ROW1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL) PORT_CHAR(DEL) PORT_CHAR('_')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("BACK SPACE") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(BS)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ESCAPE") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))

	PORT_START("ROW2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')

	PORT_START("ROW3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('@') PORT_CHAR('`')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("LINE FEED") PORT_CODE(KEYCODE_PGDN) PORT_CHAR(LF)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')

	PORT_START("ROW4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')

	PORT_START("ROW5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')

	PORT_START("ROW6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(CR)
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("MODIFIERS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CONTROL") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("CONFIG")
	PORT_CONFNAME( 0x01, 0x01, "U&L CASE" )
	PORT_CONFSETTING(    0x01, "Open (Upper Case Only)" )
	PORT_CONFSETTING(    0x00, "Grounded (Upper & Lower Case)" )
INPUT_PORTS_END

static INPUT_PORTS_START( vp611 )
	PORT_INCLUDE( vp601 )

	PORT_START("ROW7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 0") PORT_CODE(KEYCODE_0_PAD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 1") PORT_CODE(KEYCODE_1_PAD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 2") PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 3") PORT_CODE(KEYCODE_3_PAD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 4") PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 5") PORT_CODE(KEYCODE_5_PAD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 6") PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 7") PORT_CODE(KEYCODE_7_PAD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD))

	PORT_START("ROW8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 8") PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 9") PORT_CODE(KEYCODE_9_PAD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad +") PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad *") PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad =") PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

ioport_constructor vp601_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( vp601 );
}

ioport_constructor vp611_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( vp611 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

vp601_device::vp601_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_vip_ascii_keyboard_interface(mconfig, *this),
	m_rows(*this, "ROW%u", 0U),
	m_modifiers(*this, "MODIFIERS"),
	m_config(*this, "CONFIG"),
	m_scan_timer(nullptr)
{
}

vp601_device::vp601_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	vp601_device(mconfig, VP601, tag, owner, clock)
{
}

vp611_device::vp611_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	vp601_device(mconfig, VP611, tag, owner, clock)
{
}

void vp601_device::device_start()
{
	std::fill(std::begin(m_prev), std::end(m_prev), 0);

	m_scan_timer = timer_alloc(FUNC(vp601_device::scan_tick), this);

	save_item(NAME(m_prev));
	save_item(NAME(m_key));
	save_item(NAME(m_count));
}

void vp601_device::device_reset()
{
	m_key = -1;
	m_count = 0;

	m_port->st_w(0);
	m_port->kd_w(0);

	m_scan_timer->adjust(attotime::from_msec(1), 0, attotime::from_msec(1));
}

uint8_t vp601_device::encode(int key)
{
	uint8_t const modifiers = m_modifiers->read();
	uint8_t data;

	if (BIT(modifiers, 0))
	{
		data = KEY_CODES[key][2];
	}
	else if (BIT(modifiers, 1))
	{
		data = KEY_CODES[key][1];
	}
	else
	{
		data = KEY_CODES[key][0];

		if (BIT(m_config->read(), 0) && data >= 'a' && data <= 'z')
			data -= 0x20;
	}

	return data | (std::popcount(data) & 1) << 7;
}

TIMER_CALLBACK_MEMBER(vp601_device::scan_tick)
{
	int pressed = -1;

	for (int row = 0; row < 9; row++)
	{
		uint8_t const data = m_rows[row].read_safe(0);
		uint8_t const edge = data & ~m_prev[row];

		if (edge)
			pressed = row * 8 + std::countr_zero(edge);

		if (m_key >= 0 && (m_key >> 3) == row && !BIT(data, m_key & 7))
		{
			m_key = -1;
			m_port->st_w(0);
			m_port->kd_w(0);
		}

		m_prev[row] = data;
	}

	if (pressed >= 0)
	{
		m_key = pressed;
		m_count = 0;
		m_port->st_w(0);
	}
	else if (m_key >= 0 && m_count <= DEBOUNCE_MS + STROBE_MS)
	{
		m_count++;

		if (m_count == DEBOUNCE_MS)
		{
			m_port->data_w(encode(m_key));
			m_port->st_w(1);
			m_port->kd_w(1);
		}
		else if (m_count == DEBOUNCE_MS + STROBE_MS)
		{
			m_port->st_w(0);
		}
	}
}



//**************************************************************************
//  SLOT INTERFACE
//**************************************************************************

void vip_ascii_keyboards(device_slot_interface &device)
{
	device.option_add("vp601", VP601);
	device.option_add("vp611", VP611);
}
