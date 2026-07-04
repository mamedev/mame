// license:BSD-3-Clause
// copyright-holders:Dave Rand
/**********************************************************************

    Xerox Low Profile Keyboard (LPK, #G25) — HLE.  See xerox_lpk.h.

*********************************************************************/

#include "emu.h"
#include "xerox_lpk.h"

DEFINE_DEVICE_TYPE(XEROX_LPK, xerox_lpk_device, "xerox_lpk", "Xerox Low Profile Keyboard")

// Xerox LPK position codes per matrix bit (row*8 + bit), read from the v50
// monitor's tabl (position -> char) at ROM offset 0x1b29.  0xFF = no key.
// Shift/ctrl/lock are conveyed via the cmd/status byte (not separate codes),
// so the modifier key stations are NoKey here; their matrix bits set the
// cmd/status bits instead.  (NoKey, not 0, because position 0x00 = '8'.)
static constexpr uint8_t NK = 0xff; // no key / not yet mapped
static const uint8_t s_scancode[8 * 8] =
{
	// row 0: esc 1 2 3 4 5 6 7   (esc position TBD)
	NK,  0x02,0x03,0x04,0x05,0x06,0x07,0x08,
	// row 1: 8 9 0 - ^ bksp tab q   (bit-paired board: the key right of - is ^)
	0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10,
	// row 2: w e r t y u i o
	0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,
	// row 3: p @ [ enter a s d f   (after p: @ then [, per the keytop tables)
	0x19,0x1a,0x1b,0x1c,0x1e,0x1f,0x20,0x21,
	// row 4: g h j k l ; : -      (separate ; and : keys; : shifted = *)
	0x22,0x23,0x24,0x25,0x26,0x27,0x28,NK,
	// row 5: \ z x c v b n m
	0x01,0x2c,0x2d,0x2e,0x2f,0x30,0x31,0x32,
	// row 6: , . / space - - - -
	0x33,0x34,0x35,0x39,NK,  NK,  NK,  NK,
	// row 7 (modifiers - stations not sent; matrix bits drive cmd/status)
	NK,  NK,  NK,  NK,  NK,  NK,  NK,  NK,
};


static INPUT_PORTS_START( xerox_lpk )
	PORT_START("ROW0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CHAR(27)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("ROW1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('0') PORT_CHAR('=')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Back Space") PORT_CHAR(8)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Tab") PORT_CHAR(9)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('q') PORT_CHAR('Q')

	PORT_START("ROW2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('o') PORT_CHAR('O')

	PORT_START("ROW3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('@') PORT_CHAR('`')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Return") PORT_CHAR(13)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('f') PORT_CHAR('F')

	PORT_START("ROW4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("ROW5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('m') PORT_CHAR('M')

	PORT_START("ROW6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR(',')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('.')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR(' ')
	PORT_BIT(0xf0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("ROW7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R Shift") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Lock") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT(0xf0, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END


xerox_lpk_device::xerox_lpk_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	xerox820_keyboard_device(mconfig, XEROX_LPK, tag, owner, clock),
	m_rows(*this, "ROW%u", 0U),
	m_bus(0xff),
	m_lock(false),
	m_fifo_head(0),
	m_fifo_tail(0),
	m_scan_timer(nullptr)
{
}

ioport_constructor xerox_lpk_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(xerox_lpk);
}

void xerox_lpk_device::device_start()
{
	m_scan_timer = timer_alloc(FUNC(xerox_lpk_device::scan_tick), this);
	save_item(NAME(m_state));
	save_item(NAME(m_bus));
	save_item(NAME(m_lock));
	save_item(NAME(m_fifo));
	save_item(NAME(m_fifo_head));
	save_item(NAME(m_fifo_tail));
}

void xerox_lpk_device::device_reset()
{
	for (int r = 0; r < MATRIX_ROWS; r++)
		m_state[r] = 0;
	m_fifo_head = m_fifo_tail = 0;
	m_lock = false;
	m_bus = 0xff;
	m_strobe = true;
	m_kbstb_cb(ASSERT_LINE);
	// poll/pace at ~1 ms; comfortably faster than human typing, slow enough
	// that the host ISR consumes each strobed byte before the next arrives
	m_scan_timer->adjust(attotime::from_msec(1), 0, attotime::from_msec(1));
}

void xerox_lpk_device::queue(uint8_t b)
{
	int next = (m_fifo_tail + 1) % int(sizeof(m_fifo));
	if (next != m_fifo_head)
	{
		m_fifo[m_fifo_tail] = b;
		m_fifo_tail = next;
	}
}

uint8_t xerox_lpk_device::modifiers(bool shift, bool ctrl) const
{
	uint8_t m = 0;
	if (ctrl)   m |= CST_CTRL;
	if (shift)  m |= CST_SHIFT;
	if (m_lock) m |= CST_LOCK;
	return m;
}

void xerox_lpk_device::send_key(uint8_t scancode, bool make, bool shift, bool ctrl)
{
	// The v50 RX handler (u36.rx024 pekhdli @0x193) dispatches on bit 7 of the
	// un-complemented byte: SET = cmd/status byte, CLEAR = position byte (the
	// reverse of the 4.01/4.02 appendix listing).  cmd/status carries bit 7 as
	// the sync/valid marker; tblsel then indexes tabl[position] directly.
	uint8_t cst = CST_CMD | modifiers(shift, ctrl) | (make ? 0 : CST_BREAK);
	queue(cst);            // cmd/status byte (bit 7 set)
	queue(scancode);       // position byte (bit 7 clear)
}

TIMER_CALLBACK_MEMBER(xerox_lpk_device::scan_tick)
{
	// drain one byte per tick (paced strobe) before scanning for new events
	if (m_fifo_head != m_fifo_tail)
	{
		m_bus = m_fifo[m_fifo_head];
		m_fifo_head = (m_fifo_head + 1) % int(sizeof(m_fifo));
		// one latch per byte on the CLEAR->ASSERT (rising) edge
		m_kbstb_cb(CLEAR_LINE);
		m_kbstb_cb(ASSERT_LINE);
		return;
	}

	// scan the matrix for transitions
	ioport_value cur[MATRIX_ROWS];
	for (int r = 0; r < MATRIX_ROWS; r++)
		cur[r] = m_rows[r]->read();

	// aggregate modifier state from the current matrix (row 7: lshift/rshift/
	// lctrl/lock); these stations are not sent as keys -- they ride in the
	// cmd/status byte.  Lock is a PORT_TOGGLE so its bit is the latched state.
	bool shift = BIT(cur[7], 0) || BIT(cur[7], 1);
	bool ctrl  = BIT(cur[7], 2);
	m_lock = BIT(cur[7], 3);

	for (int r = 0; r < MATRIX_ROWS; r++)
	{
		ioport_value diff = cur[r] ^ m_state[r];
		if (!diff)
			continue;
		for (int b = 0; b < 8; b++)
		{
			if (!BIT(diff, b))
				continue;
			bool make = BIT(cur[r], b);
			uint8_t sc = s_scancode[r * 8 + b];
			if (sc == NK)
				continue;
			send_key(sc, make, shift, ctrl);
		}
	}

	for (int r = 0; r < MATRIX_ROWS; r++)
		m_state[r] = cur[r];
}
