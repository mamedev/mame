// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Sony NEWS keyboard and mouse (high-level emulation).
 *
 * Sources:
 *
 *   - https://github.com/tmk/tmk_keyboard/tree/master/converter/news_usb
 *   - https://github.com/NetBSD/src/blob/trunk/sys/dev/news/newskeymap.c
 *   - https://github.com/NetBSD/src/blob/trunk/sys/arch/newsmips/dev/ms_hb.c
 *
 * TODO:
 *   - other languages (esp. Japanese)
 *   - other variations
 *   - dip switches
 */

#include "emu.h"
#include "news_hid.h"

#include "machine/keyboard.ipp"

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(NEWS_HID_HLE, news_hid_hle_device, "news_hid_hle", "Sony NEWS Keyboard and Mouse (HLE)")

news_hid_hle_device::news_hid_hle_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, NEWS_HID_HLE, tag, owner, clock)
	, device_matrix_keyboard_interface(mconfig, *this, "ROW0", "ROW1", "ROW2", "ROW3", "ROW4", "ROW5", "ROW6", "ROW7")
	, m_mouse_x_axis(*this, "mouse_x_axis")
	, m_mouse_y_axis(*this, "mouse_y_axis")
	, m_mouse_buttons(*this, "mouse_buttons")
	, m_irq_out_cb(*this)
{
}

void news_hid_hle_device::map(address_map &map)
{
	map(0x0, 0x0).r(FUNC(news_hid_hle_device::data_r<KEYBOARD>));
	map(0x1, 0x1).r(FUNC(news_hid_hle_device::status_r<KEYBOARD>));
	map(0x2, 0x2).w(FUNC(news_hid_hle_device::reset_w<KEYBOARD>));
	map(0x3, 0x3).w(FUNC(news_hid_hle_device::init_w<KEYBOARD>));
	map(0x4, 0x4).r(FUNC(news_hid_hle_device::data_r<MOUSE>));
	map(0x5, 0x5).r(FUNC(news_hid_hle_device::status_r<MOUSE>));
	map(0x6, 0x6).w(FUNC(news_hid_hle_device::reset_w<MOUSE>));
	map(0x7, 0x7).w(FUNC(news_hid_hle_device::init_w<MOUSE>));
}

void news_hid_hle_device::map_68k(address_map &map)
{
	map(0x0, 0x0).r(FUNC(news_hid_hle_device::data_r<KEYBOARD>));
	map(0x1, 0x1).r(FUNC(news_hid_hle_device::status_68k_r));
	map(0x2, 0x2).w(FUNC(news_hid_hle_device::ien_w<KEYBOARD>));
	map(0x3, 0x3).w(FUNC(news_hid_hle_device::reset_w<KEYBOARD>));
	// TODO: keyboard buzzer
	map(0x5, 0x5).r(FUNC(news_hid_hle_device::data_r<MOUSE>));
	map(0x6, 0x6).w(FUNC(news_hid_hle_device::ien_w<MOUSE>));
	map(0x7, 0x7).w(FUNC(news_hid_hle_device::reset_w<MOUSE>));
}

void news_hid_hle_device::map_apbus(address_map &map)
{
	// see https://github.com/NetBSD/src/blob/trunk/sys/arch/newsmips/apbus/kb_ap.c#L44
	// Note that this isn't actually fully functional yet, but it doesn't matter since
	// the APbus driver (NWS-5000X/news_r4k) doesn't have the framebuffer emulated yet
	// so the graphical console can't be used anyways. This is enough to satisfy the
	// keyboard/mouse init routines in NEWS-OS.
	// TODO: speed, buzzer, keyboard tx, make it actually work
	map(0x0, 0x3).r(FUNC(news_hid_hle_device::data_r<KEYBOARD>));
	map(0x4, 0x7).r(FUNC(news_hid_hle_device::status_r<KEYBOARD>));
	map(0x8, 0xb).w(FUNC(news_hid_hle_device::init_w<KEYBOARD>));
	map(0xc, 0xf).w(FUNC(news_hid_hle_device::reset_w<KEYBOARD>));
	// 10-13 kb speed r
	map(0x14, 0x17).r(FUNC(news_hid_hle_device::data_r<MOUSE>));
	map(0x18, 0x1b).r(FUNC(news_hid_hle_device::status_r<MOUSE>));
	map(0x1c, 0x1f).w(FUNC(news_hid_hle_device::init_w<MOUSE>));
	map(0x20, 0x23).w(FUNC(news_hid_hle_device::reset_w<MOUSE>));
	// 24-27 ms speed r
	// 28-2b kb buzzf
	// 2c-2f kb buzz
	// 30-33 kb tx data
	map(0x34, 0x37).lr32(NAME([](offs_t offset) { return 0x1; })); // kb tx stat (pretend to accept anything, AFAICT)
	// 38-3b kb tx init
	// 3c-3f kb tx reset
	// 40-43 kb tx speed
}

void news_hid_hle_device::device_start()
{
	m_irq_out_cb.resolve_all_safe();

	//save_item(NAME(m_fifo));
	save_item(NAME(m_irq_enabled));
	save_item(NAME(m_irq_out_state));

	save_item(NAME(m_mouse_x));
	save_item(NAME(m_mouse_y));
	save_item(NAME(m_mouse_b));
}

void news_hid_hle_device::device_reset()
{
	m_fifo[KEYBOARD].clear();
	m_fifo[MOUSE].clear();

	m_irq_enabled[KEYBOARD] = false;
	m_irq_enabled[MOUSE] = false;
	out_irq<KEYBOARD>(false);
	out_irq<MOUSE>(false);

	reset_key_state();
	start_processing(attotime::from_hz(1'200));
}

void news_hid_hle_device::key_make(u8 row, u8 column)
{
	LOG("key_make row %d col %d\n", row, column);

	push_key((row << 4) | column);
}

void news_hid_hle_device::key_break(u8 row, u8 column)
{
	LOG("key_break row %d col %d\n", row, column);

	push_key(0x80 | (row << 4) | column);
}

void news_hid_hle_device::push_key(u8 code)
{
	m_fifo[KEYBOARD].enqueue(code);

	out_irq<KEYBOARD>(true);
}

// HACK: abuse the keyboard row scanner to sample the mouse too
void news_hid_hle_device::scan_complete()
{
	// read mouse state
	s16 const x = m_mouse_x_axis->read();
	s16 const y = m_mouse_y_axis->read();
	u8 const b = m_mouse_buttons->read();

	// compute delta
	s8 const dx = x - m_mouse_x;
	s8 const dy = y - m_mouse_y;
	u8 const db = b ^ m_mouse_b;

	// report if fifo has room and position or buttons changed
	if ((m_fifo[MOUSE].queue_length() < 6) && (dx || dy || db))
	{
		LOG("mouse dx %d dy %d db %d\n", dx, dy, db);

		// compute sign
		u8 const sx = (dx < 0) ? 0x08 : 0x00;
		u8 const sy = (dy < 0) ? 0x10 : 0x00;

		// transmit data
		m_fifo[MOUSE].enqueue(0x80 | sy | sx | b);
		m_fifo[MOUSE].enqueue(dx & 0x7f);
		m_fifo[MOUSE].enqueue(dy & 0x7f);

		// update mouse state
		m_mouse_x = x;
		m_mouse_y = y;
		m_mouse_b = b;

		out_irq<MOUSE>(true);
	}
}

template <news_hid_hle_device::news_hid_device Device> void news_hid_hle_device::out_irq(bool state)
{
	if (m_irq_out_state[Device] != state)
	{
		m_irq_out_state[Device] = state;
		m_irq_out_cb[Device](state && m_irq_enabled[Device]);
	}
}

template <news_hid_hle_device::news_hid_device Device> u8 news_hid_hle_device::data_r()
{
	if (m_fifo[Device].empty())
		return 0;

	if (!machine().side_effects_disabled())
	{
		u8 const data = m_fifo[Device].dequeue();

		if (m_fifo[Device].empty())
			out_irq<Device>(false);

		return data;
	}
	else
		return m_fifo[Device].peek();
}

template <news_hid_hle_device::news_hid_device Device> void news_hid_hle_device::reset_w(u8 data)
{
	LOG("reset_w<%d> 0x%02x\n", Device, data);
	m_fifo[Device].clear();

	out_irq<Device>(false);
}

template <news_hid_hle_device::news_hid_device Device> void news_hid_hle_device::init_w(u8 data)
{
	LOG("init_w<%d> 0x%02x\n", Device, data);

	ien_w<Device>(data);
}

template <news_hid_hle_device::news_hid_device Device> void news_hid_hle_device::ien_w(u8 data)
{
	LOG("ien_w<%d> 0x%02x\n", Device, data);

	m_irq_enabled[Device] = bool(data);
}

u8 news_hid_hle_device::status_68k_r()
{
	u8 const data =
		(!m_fifo[KEYBOARD].empty() ? 0x80 : 0) |
		(!m_fifo[MOUSE].empty()    ? 0x40 : 0) |
		(m_fifo[KEYBOARD].full()   ? 0x20 : 0) |
		(m_fifo[MOUSE].full()      ? 0x10 : 0) |
		(m_irq_out_state[KEYBOARD] ? 0 : 0x08) |
		(m_irq_out_state[MOUSE]    ? 0 : 0x04) |
		(m_irq_enabled[KEYBOARD]   ? 0x02 : 0) |
		(m_irq_enabled[MOUSE]      ? 0x01 : 0);

	LOG("status_r 0x%02x\n", data);

	return data;
}

INPUT_PORTS_START(news_hid_hle_device)
	PORT_START("ROW0")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F1")           PORT_CODE(KEYCODE_F1)         PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F2")           PORT_CODE(KEYCODE_F2)         PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F3")           PORT_CODE(KEYCODE_F3)         PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F4")           PORT_CODE(KEYCODE_F4)         PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F5")           PORT_CODE(KEYCODE_F5)         PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F6")           PORT_CODE(KEYCODE_F6)         PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F7")           PORT_CODE(KEYCODE_F7)         PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F8")           PORT_CODE(KEYCODE_F8)         PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F9")           PORT_CODE(KEYCODE_F9)         PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F10")          PORT_CODE(KEYCODE_F10)        PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Esc")          PORT_CODE(KEYCODE_ESC)        PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_3)          PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_4)          PORT_CHAR('4') PORT_CHAR('$')

	PORT_START("ROW1")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_5)          PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_6)          PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_7)          PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_8)          PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_0)          PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Backspace")    PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(8)
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Tab")          PORT_CODE(KEYCODE_TAB)        PORT_CHAR(9)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_Q)          PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_W)          PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_E)          PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_R)          PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_T)          PORT_CHAR('t') PORT_CHAR('T')

	PORT_START("ROW2")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_Y)          PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_U)          PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_I)          PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_O)          PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_P)          PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Delete")       PORT_CODE(KEYCODE_DEL)        PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Control")      PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_A)          PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_S)          PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_D)          PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_F)          PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_G)          PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_H)          PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_J)          PORT_CHAR('j') PORT_CHAR('J')

	PORT_START("ROW3")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_K)          PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_L)          PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Return")       PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L Shift")      PORT_CODE(KEYCODE_LSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_Z)          PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_X)          PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_C)          PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_V)          PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_B)          PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_N)          PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_M)          PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR('>')

	PORT_START("ROW4")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R Shift")      PORT_CODE(KEYCODE_RSHIFT)     PORT_CHAR(UCHAR_MAMEKEY(RSHIFT))
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Alternate")    PORT_CODE(KEYCODE_LALT)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Caps Lock")    PORT_CODE(KEYCODE_CAPSLOCK)   PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Nfer") // muhenkan?
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Space")        PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Xfer") // henkan?
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) // eisu?
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) // kana?
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) // Execute?
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 7")         PORT_CODE(KEYCODE_7_PAD)      PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 8")         PORT_CODE(KEYCODE_8_PAD)      PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 9")         PORT_CODE(KEYCODE_9_PAD)      PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP -")         PORT_CODE(KEYCODE_MINUS_PAD)  PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 4")         PORT_CODE(KEYCODE_4_PAD)      PORT_CHAR(UCHAR_MAMEKEY(4_PAD))

	PORT_START("ROW5")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 5")         PORT_CODE(KEYCODE_5_PAD)      PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 6")         PORT_CODE(KEYCODE_6_PAD)      PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP +")         PORT_CODE(KEYCODE_PLUS_PAD)   PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 1")         PORT_CODE(KEYCODE_1_PAD)      PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 2")         PORT_CODE(KEYCODE_2_PAD)      PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 3")         PORT_CODE(KEYCODE_3_PAD)      PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) // Separator?
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 0")         PORT_CODE(KEYCODE_0_PAD)      PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Up")           PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP .")         PORT_CODE(KEYCODE_DEL_PAD)    PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Enter")        PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left")         PORT_CODE(KEYCODE_LEFT)       PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Down")         PORT_CODE(KEYCODE_DOWN)       PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right")        PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP *")         PORT_CODE(KEYCODE_ASTERISK)   PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP /")         PORT_CODE(KEYCODE_SLASH_PAD)

	PORT_START("ROW6")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP Tab")
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F11")          PORT_CODE(KEYCODE_F11)        PORT_CHAR(UCHAR_MAMEKEY(F11))
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F12")          PORT_CODE(KEYCODE_F12)        PORT_CHAR(UCHAR_MAMEKEY(F12))
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Help")
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Insert")       PORT_CODE(KEYCODE_INSERT)     PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Clear")
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Prior")        PORT_CODE(KEYCODE_PGUP)       PORT_CHAR(UCHAR_MAMEKEY(PGUP))
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Next")         PORT_CODE(KEYCODE_PGDN)       PORT_CHAR(UCHAR_MAMEKEY(PGDN))
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("ROW7")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("mouse_x_axis")
	PORT_BIT(0xffff, 0, IPT_MOUSE_X) PORT_SENSITIVITY(100)

	PORT_START("mouse_y_axis")
	PORT_BIT(0xffff, 0, IPT_MOUSE_Y) PORT_SENSITIVITY(100)

	PORT_START("mouse_buttons")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_CODE(MOUSECODE_BUTTON1) PORT_NAME("Left Button")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_CODE(MOUSECODE_BUTTON2) PORT_NAME("Right Button")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_CODE(MOUSECODE_BUTTON3) PORT_NAME("Middle Button")
	PORT_BIT(0xf8, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

ioport_constructor news_hid_hle_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(news_hid_hle_device);
}
