// license:BSD-3-Clause
// copyright-holders:R. Belmont

// High-level Apple Desktop Bus extended keyboard

#include "emu.h"
#include "adbhlekeyboard.h"

#include "adbhle.h"

namespace
{

class adb_hle_keyboard_device : public adb_hle_device
{
public:
	adb_hle_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	DECLARE_INPUT_CHANGED_MEMBER(power_changed);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual unsigned adb_talk(u8 reg, std::span<u8> data) override;
	virtual void adb_talk_complete(u8 reg, bool success) override;
	virtual void adb_poll() override;
	virtual void adb_bus_reset() override;

private:
	static constexpr unsigned KEY_QUEUE_SIZE = 32;

	void reset_keyboard();
	void enqueue_key(u8 code);
	void update_modifier(u8 code, bool pressed);

	required_ioport_array<8> m_keys;

	u16 m_key_matrix[8];
	u8 m_key_queue[KEY_QUEUE_SIZE];
	u8 m_queue_start;
	u8 m_queue_count;
	u8 m_pending_count;
	u8 m_modifiers;
};

static INPUT_PORTS_START(adb_hle_keyboard)
	PORT_START("KEY0")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)             PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)             PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)             PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)             PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)             PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)             PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)             PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)             PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)             PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)             PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)             PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)             PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)             PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)             PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)             PORT_CHAR('r') PORT_CHAR('R')

	PORT_START("KEY1")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)             PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)             PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)             PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)             PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)             PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)             PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)             PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)             PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)        PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)             PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)             PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)         PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)             PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)             PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)    PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)             PORT_CHAR('o') PORT_CHAR('O')

	PORT_START("KEY2")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)             PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)     PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)             PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)             PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR('\r')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)             PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)             PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)         PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)             PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)         PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)     PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)         PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)         PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)             PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)             PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)          PORT_CHAR('.') PORT_CHAR('>')

	PORT_START("KEY3")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)           PORT_CHAR('\t')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)         PORT_CHAR(' ')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)         PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)     PORT_CHAR(8)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Control") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Command / Open Apple") PORT_CODE(KEYCODE_LALT)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Caps Lock") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Option / Solid Apple") PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left Arrow") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right Arrow") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Down Arrow") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Up Arrow") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("KEY4")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad Clear") PORT_CODE(KEYCODE_NUMLOCK) PORT_CHAR(UCHAR_MAMEKEY(NUMLOCK))
	PORT_BIT(0x0700, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("KEY5")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS_PAD) PORT_CHAR(UCHAR_MAMEKEY(EQUALS_PAD))
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Reset / Power") PORT_CODE(KEYCODE_F12) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(adb_hle_keyboard_device::power_changed), 0)
	PORT_BIT(0xc000, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("KEY6")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9) PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F11) PORT_CHAR(UCHAR_MAMEKEY(F11))
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F13) PORT_CHAR(UCHAR_MAMEKEY(F13))
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F14) PORT_CHAR(UCHAR_MAMEKEY(F14))
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10) PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F12) PORT_CHAR(UCHAR_MAMEKEY(F12))

	PORT_START("KEY7")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F15) PORT_CHAR(UCHAR_MAMEKEY(F15))
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_INSERT) PORT_NAME("Insert/Help") PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGUP) PORT_CHAR(UCHAR_MAMEKEY(PGUP))
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL) PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_END) PORT_CHAR(UCHAR_MAMEKEY(END))
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGDN) PORT_CHAR(UCHAR_MAMEKEY(PGDN))
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0xf800, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

adb_hle_keyboard_device::adb_hle_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	adb_hle_device(mconfig, ADB_HLE_KEYBOARD, tag, owner, clock, 2, 1),
	m_keys(*this, "KEY%u", 0),
	m_queue_start(0),
	m_queue_count(0),
	m_pending_count(0),
	m_modifiers(0xff)
{
	std::fill(std::begin(m_key_matrix), std::end(m_key_matrix), 0);
	std::fill(std::begin(m_key_queue), std::end(m_key_queue), 0);
}

ioport_constructor adb_hle_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(adb_hle_keyboard);
}

void adb_hle_keyboard_device::device_start()
{
	adb_hle_device::device_start();

	save_item(NAME(m_key_matrix));
	save_item(NAME(m_key_queue));
	save_item(NAME(m_queue_start));
	save_item(NAME(m_queue_count));
	save_item(NAME(m_pending_count));
	save_item(NAME(m_modifiers));
}

void adb_hle_keyboard_device::device_reset()
{
	adb_hle_device::device_reset();
	reset_keyboard();
}

INPUT_CHANGED_MEMBER(adb_hle_keyboard_device::power_changed)
{
	adb_poweron_w(newval ? CLEAR_LINE : ASSERT_LINE);
}

unsigned adb_hle_keyboard_device::adb_talk(u8 reg, std::span<u8> data)
{
	m_pending_count = 0;
	if (reg == 0)
	{
		if (!m_queue_count)
		{
			return 0;
		}

		data[1] = m_key_queue[m_queue_start];
		m_pending_count = 1;
		if (m_queue_count > 1)
		{
			data[0] = m_key_queue[(m_queue_start + 1) % KEY_QUEUE_SIZE];
			m_pending_count = 2;
		}
		else
		{
			data[0] = 0xff;
		}
		return 2;
	}

	if (reg == 2)
	{
		data[0] = m_modifiers;
		data[1] = 0xff;
		return 2;
	}

	return 0;
}

void adb_hle_keyboard_device::adb_talk_complete(u8 reg, bool success)
{
	if ((reg == 0) && success)
	{
		m_queue_start = (m_queue_start + m_pending_count) % KEY_QUEUE_SIZE;
		m_queue_count -= m_pending_count;
	}
	m_pending_count = 0;
	set_service_request(m_queue_count != 0);
}

void adb_hle_keyboard_device::adb_poll()
{
	bool const power_transition = bool((m_keys[5]->read() ^ m_key_matrix[5]) & 0x2000);
	bool any_key_down = false;
	for (unsigned row = 0; row < std::size(m_key_matrix); row++)
	{
		u16 const keys = m_keys[row]->read();
		any_key_down = any_key_down || bool(keys);
		u16 changes = keys ^ m_key_matrix[row];
		for (unsigned column = 0; changes && (column < 16); column++)
		{
			u16 const mask = u16(1) << column;
			if (changes & mask)
			{
				bool const pressed = bool(keys & mask);
				u8 const key = (row << 4) | column;
				u8 const code = key | (pressed ? 0x00 : 0x80);
				update_modifier(code, pressed);

				if (key == 0x5d)
				{
					adb_poweron_w(pressed ? CLEAR_LINE : ASSERT_LINE);
					enqueue_key(pressed ? 0x7f : 0xff);
					enqueue_key(pressed ? 0x7f : 0xff);
				}
				else if ((key != 0x6f) || !power_transition)
				{
					enqueue_key(code);
				}
				changes &= ~mask;
			}
		}
		m_key_matrix[row] = keys;
	}

	adb_anykeydown_w(any_key_down ? ASSERT_LINE : CLEAR_LINE);
	set_service_request(m_queue_count != 0);
}

void adb_hle_keyboard_device::adb_bus_reset()
{
	reset_keyboard();
}

void adb_hle_keyboard_device::reset_keyboard()
{
	std::fill(std::begin(m_key_matrix), std::end(m_key_matrix), 0);
	std::fill(std::begin(m_key_queue), std::end(m_key_queue), 0);
	m_queue_start = 0;
	m_queue_count = 0;
	m_pending_count = 0;
	m_modifiers = 0xff;
	set_service_request(false);
	adb_poweron_w((m_keys[5]->read() & 0x2000) ? CLEAR_LINE : ASSERT_LINE);
	adb_anykeydown_w(CLEAR_LINE);
}

void adb_hle_keyboard_device::enqueue_key(u8 code)
{
	if (m_queue_count == KEY_QUEUE_SIZE)
	{
		return;
	}

	u8 const queue_end = (m_queue_start + m_queue_count) % KEY_QUEUE_SIZE;
	m_key_queue[queue_end] = code;
	m_queue_count++;
}

void adb_hle_keyboard_device::update_modifier(u8 code, bool pressed)
{
	u8 mask = 0;
	switch (code & 0x7f)
	{
	case 0x39:
		mask = 0x20;
		break;

	case 0x36:
		mask = 0x08;
		break;

	case 0x38:
		mask = 0x04;
		break;

	case 0x3a:
		mask = 0x02;
		break;

	case 0x37:
		mask = 0x01;
		break;
	}

	if (mask)
	{
		if (pressed)
		{
			m_modifiers |= mask;
		}
		else
		{
			m_modifiers &= ~mask;
		}
	}
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(ADB_HLE_KEYBOARD, adb_slot_card_interface, adb_hle_keyboard_device, "adbhlekeyboard", "ADB HLE Extended Keyboard")
