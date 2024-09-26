// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

    Driver for Micro-Term ACT-5A and other F8-based terminals.

*******************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/f8/f8.h"
#include "machine/ay31015.h"
#include "machine/f3853.h"
#include "machine/input_merger.h"
#include "machine/ripple_counter.h"
#include "sound/beep.h"
#include "screen.h"
#include "speaker.h"


namespace {

class microterm_f8_state : public driver_device
{
public:
	microterm_f8_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_uart(*this, "uart")
		, m_io(*this, "io")
		, m_screen(*this, "screen")
		, m_bell(*this, "bell")
		, m_blinkcount(*this, "blinkcount")
		, m_kbdecode(*this, "kbdecode")
		, m_chargen(*this, "chargen")
		, m_keys(*this, "KEY%u", 0U)
		, m_modifiers(*this, "MODIFIERS")
		, m_special(*this, "SPECIAL")
		, m_dsw(*this, "DSW%u", 1U)
		, m_jumpers(*this, "JUMPERS")
	{ }

	void act5a(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(baud_clock);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void vblank_w(int state);

	u8 bell_r();
	void scroll_w(u8 data);
	u8 vram_r(offs_t offset);
	void vram_w(offs_t offset, u8 data);
	void uart_transmit_w(u8 data);
	bool poll_keyboard();
	u8 key_r();
	u8 port00_r();
	void port00_w(u8 data);
	u8 port01_r();

	void f8_mem(address_map &map) ATTR_COLD;
	void f8_io(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<ay51013_device> m_uart;
	required_device<rs232_port_device> m_io;
	//required_device<rs232_port_device> m_aux;
	required_device<screen_device> m_screen;
	required_device<beep_device> m_bell;
	required_device<ripple_counter_device> m_blinkcount;
	required_region_ptr<u8> m_kbdecode;
	required_region_ptr<u8> m_chargen;
	required_ioport_array<11> m_keys;
	required_ioport m_modifiers;
	required_ioport m_special;
	required_ioport_array<3> m_dsw;
	required_ioport m_jumpers;

	u8 m_port00 = 0;
	u8 m_keylatch = 0;
	u8 m_attrlatch = 0;
	u8 m_scroll = 0;
	std::unique_ptr<u16[]> m_vram;

	emu_timer *m_baud_clock = nullptr;
};

void microterm_f8_state::machine_start()
{
	m_keylatch = 0;
	m_attrlatch = 0;
	m_vram = make_unique_clear<u16[]>(0x800); // 6x MM2114 with weird addressing

	m_baud_clock = timer_alloc(FUNC(microterm_f8_state::baud_clock), this);
	m_baud_clock->adjust(attotime::zero, 0);

	save_item(NAME(m_port00));
	save_item(NAME(m_keylatch));
	save_item(NAME(m_attrlatch));
	save_item(NAME(m_scroll));
	save_pointer(NAME(m_vram), 0x800);
}

void microterm_f8_state::machine_reset()
{
	m_scroll = 0;
	m_port00 = 0xff;

	// UART parameters
	ioport_value dsw3 = m_dsw[2]->read();
	m_uart->write_np(BIT(dsw3, 9));
	m_uart->write_tsb(BIT(dsw3, 8));
	m_uart->write_eps(BIT(dsw3, 7));
	m_uart->write_nb1(BIT(dsw3, 6));
	m_uart->write_nb2(BIT(dsw3, 5));

	m_uart->write_cs(1);
	m_uart->write_swe(0);
}

TIMER_CALLBACK_MEMBER(microterm_f8_state::baud_clock)
{
	m_uart->write_tcp(param);
	m_uart->write_rcp(param);

	ioport_value rate = m_dsw[1]->read() ^ 0xff;
	if (BIT(rate, 7))
		m_baud_clock->adjust(attotime::from_hz(110 * 32), !param);
	else
		m_baud_clock->adjust(attotime::from_hz(19200 * 32 / rate), !param);
}

u32 microterm_f8_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (BIT(m_port00, 2))
	{
		// Display blanked?
		bitmap.fill(rgb_t::black(), cliprect);
		return 0;
	}

	const ioport_value jumpers = m_jumpers->read();
	unsigned y = cliprect.top();
	offs_t rowbase = (((y / 12) + m_scroll) % 24) * 80;
	unsigned line = y % 12;
	if (line >= 6)
		line += 2;

	const bool cursor_on = (m_blinkcount->count() & (~jumpers << 1) & 0x38) == 0;
	const bool blink_on = (m_blinkcount->count() & (~jumpers >> 3) & 0x38) == 0;

	while (y <= cliprect.bottom())
	{
		const bool allow_underline = (line == 13) || (line == 12 && !BIT(jumpers, 1));

		for (unsigned x = cliprect.left(); x <= cliprect.right(); x++)
		{
			u16 ch = m_vram[rowbase + (x / 9)];
			u8 chdata = m_chargen[(line << 7) | (ch & 0x7f)];
			bool dot = (!BIT(ch, 8) && allow_underline) || ((BIT(ch, 10) || blink_on) && BIT(chdata, 8 - (x % 9)));
			if ((BIT(ch, 7) && cursor_on) == BIT(ch, 9))
				dot = !dot;
			bitmap.pix(y, x) = dot ? rgb_t::white() : rgb_t::black();
		}

		y++;
		line++;
		if ((line & 7) >= 6)
		{
			line = (line + 2) & 15;
			if (line == 0)
			{
				rowbase += 80;
				if (rowbase == 24 * 80)
					rowbase = 0;
			}
		}
	}

	return 0;
}

void microterm_f8_state::vblank_w(int state)
{
	if (state)
		m_bell->set_state(0);
}

u8 microterm_f8_state::bell_r()
{
	if (!machine().side_effects_disabled())
		m_bell->set_state(1);
	return 0;
}

void microterm_f8_state::scroll_w(u8 data)
{
	m_scroll++;
	if (m_scroll == 24)
		m_scroll = 0;
}

u8 microterm_f8_state::vram_r(offs_t offset)
{
	offs_t vaddr = (offset >> 8) * 80 + (offset & 0x007f);
	assert(vaddr < 0x800);

	u16 vdata = m_vram[vaddr];
	if (!machine().side_effects_disabled())
		m_attrlatch = (vdata & 0xf00) >> 4;

	// Bit 7 indicates protected attribute
	if ((~vdata & (~m_jumpers->read() >> 1) & 0xf00) != 0)
		return vdata | 0x80;
	else
		return vdata & 0x7f;
}

void microterm_f8_state::vram_w(offs_t offset, u8 data)
{
	offs_t vaddr = (offset >> 8) * 80 + (offset & 0x007f);
	assert(vaddr < 0x800);

	m_vram[vaddr] = data | u16(m_port00 & 0xf0) << 4;
	if (BIT(m_port00, 0))
		m_vram[vaddr] |= u16(m_attrlatch) << 4;
}

void microterm_f8_state::uart_transmit_w(u8 data)
{
	m_uart->transmit((data & 0x7f) | (m_dsw[2]->read() & 0x10) << 3);
}

bool microterm_f8_state::poll_keyboard()
{
	for (int row = 0; row < 11; row++)
	{
		u8 polled = m_keys[row]->read();
		if (polled == 0xff)
			continue;

		offs_t keyaddr = (m_modifiers->read() << 7) | (row << 3);
		while (BIT(polled, 0))
		{
			polled >>= 1;
			keyaddr++;
		}
		m_keylatch = m_kbdecode[keyaddr];
		return true;
	}

	return false;
}

u8 microterm_f8_state::key_r()
{
	// Cursor is supposed to stop blinking temporarily when keys are depressed
	// This implementation suspends cursor blinking when keys are actually read
	// It also suspends blinking text, which perhaps is supposed to happen
	if (!machine().side_effects_disabled())
	{
		m_blinkcount->reset_w(1);
		m_blinkcount->reset_w(0);
	}

	return m_keylatch;
}

u8 microterm_f8_state::port00_r()
{
	u8 flags = m_port00;

	// Full duplex switch
	if (!BIT(m_dsw[2]->read(), 0))
		flags |= 0x02;

	if (BIT(m_port00, 0))
		flags |= m_attrlatch;

	return flags;
}

void microterm_f8_state::port00_w(u8 data)
{
	m_port00 = data;
}

u8 microterm_f8_state::port01_r()
{
	u8 flags = 0;

	// Some timing flag (not necessarily HBLANK?)
	if (!m_screen->hblank())
		flags |= 0x80;

	// Keyboard polling
	if (poll_keyboard())
		flags |= 0x40;

	// ???
	if (!m_screen->vblank())
		flags |= 0x20;

	// Local mode switch
	if (!BIT(m_special->read(), 1))
		flags |= 0x10;

	// Protected field setting
	flags |= ~m_dsw[2]->read() & 0x06;

	return flags;
}

void microterm_f8_state::f8_mem(address_map &map)
{
	map(0x0000, 0x0bff).rom().region("maincpu", 0);
	map(0x0c00, 0x0c00).r(FUNC(microterm_f8_state::bell_r));
	map(0x2000, 0x2000).mirror(0x1fff).w(FUNC(microterm_f8_state::scroll_w));
	map(0x4000, 0x407f).select(0x1f00).rw(FUNC(microterm_f8_state::vram_r), FUNC(microterm_f8_state::vram_w));
	map(0x5800, 0x5fff).unmaprw();
	map(0x8000, 0x8000).r(m_uart, FUNC(ay51013_device::receive)).w(FUNC(microterm_f8_state::uart_transmit_w));
	map(0xf000, 0xf000).r(FUNC(microterm_f8_state::key_r));
}

void microterm_f8_state::f8_io(address_map &map)
{
	map.unmap_value_high();
	map(0x00, 0x00).rw(FUNC(microterm_f8_state::port00_r), FUNC(microterm_f8_state::port00_w));
	map(0x01, 0x01).r(FUNC(microterm_f8_state::port01_r)).nopw();
	map(0x0c, 0x0f).rw("smi", FUNC(f3853_device::read), FUNC(f3853_device::write));
}

static INPUT_PORTS_START(act5a)
	PORT_START("KEY0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(' ') PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N") PORT_CHAR('n') PORT_CHAR('N') PORT_CHAR(0x0e) PORT_CODE(KEYCODE_N)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CHAR('b') PORT_CHAR('B') PORT_CHAR(0x02) PORT_CODE(KEYCODE_B)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CHAR('v') PORT_CHAR('V') PORT_CHAR(0x16) PORT_CODE(KEYCODE_V)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CHAR('c') PORT_CHAR('C') PORT_CHAR(0x03) PORT_CODE(KEYCODE_C)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X") PORT_CHAR('x') PORT_CHAR('X') PORT_CHAR(0x18) PORT_CODE(KEYCODE_X)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z") PORT_CHAR('z') PORT_CHAR('Z') PORT_CHAR(0x1a) PORT_CODE(KEYCODE_Z)

	PORT_START("KEY1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(COMMA_PAD)) PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD)) PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H") PORT_CHAR('h') PORT_CHAR('H') PORT_CHAR(0x08) PORT_CODE(KEYCODE_H)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CHAR('g') PORT_CHAR('G') PORT_CHAR(0x07) PORT_CODE(KEYCODE_G)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CHAR('f') PORT_CHAR('F') PORT_CHAR(0x06) PORT_CODE(KEYCODE_F)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CHAR('d') PORT_CHAR('D') PORT_CHAR(0x04) PORT_CODE(KEYCODE_D)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CHAR('s') PORT_CHAR('S') PORT_CHAR(0x13) PORT_CODE(KEYCODE_S)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CHAR('a') PORT_CHAR('A') PORT_CHAR(0x01) PORT_CODE(KEYCODE_A)

	PORT_START("KEY2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8  *  -") PORT_CHAR('8') PORT_CHAR('*') PORT_CODE(KEYCODE_8)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y") PORT_CHAR('y') PORT_CHAR('Y') PORT_CHAR(0x19) PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T") PORT_CHAR('t') PORT_CHAR('T') PORT_CHAR(0x14) PORT_CODE(KEYCODE_T)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CHAR('r') PORT_CHAR('R') PORT_CHAR(0x12) PORT_CODE(KEYCODE_R)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CHAR('e') PORT_CHAR('E') PORT_CHAR(0x05) PORT_CODE(KEYCODE_E)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W") PORT_CHAR('w') PORT_CHAR('W') PORT_CHAR(0x17) PORT_CODE(KEYCODE_W)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q") PORT_CHAR('q') PORT_CHAR('Q') PORT_CHAR(0x11) PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(0x09) PORT_CODE(KEYCODE_TAB)

	PORT_START("KEY3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('7') PORT_CHAR('&') PORT_CODE(KEYCODE_7)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('6') PORT_CHAR('^') PORT_CODE(KEYCODE_6)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('5') PORT_CHAR('%') PORT_CODE(KEYCODE_5)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('4') PORT_CHAR('$') PORT_CODE(KEYCODE_4)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('3') PORT_CHAR('#') PORT_CODE(KEYCODE_3)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('2') PORT_CHAR('@') PORT_CODE(KEYCODE_2)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('1') PORT_CHAR('!') PORT_CODE(KEYCODE_1)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(0x1b) PORT_CODE(KEYCODE_ESC)

	PORT_START("KEY4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("]  }") PORT_CHAR(']') PORT_CHAR('}') PORT_CHAR(0x1d) PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('-') PORT_CHAR('_') PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[  {") PORT_CHAR('[') PORT_CHAR('{') PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P") PORT_CHAR('p') PORT_CHAR('P') PORT_CHAR(0x10) PORT_CODE(KEYCODE_P)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('0') PORT_CHAR(')') PORT_CODE(KEYCODE_0)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\\  |") PORT_CHAR('\\') PORT_CHAR('|') PORT_CHAR(0x1c) PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('`') PORT_CHAR('~') PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('=') PORT_CHAR('+') PORT_CODE(KEYCODE_EQUALS)

	PORT_START("KEY5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('\'') PORT_CHAR('"') PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(';') PORT_CHAR(':') PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('/') PORT_CHAR('?') PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".  >  9") PORT_CHAR('.') PORT_CHAR('>') PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(",  <  8") PORT_CHAR(',') PORT_CHAR('<') PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M  7") PORT_CHAR('m') PORT_CHAR('M') PORT_CODE(KEYCODE_M)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L  6") PORT_CHAR('l') PORT_CHAR('L') PORT_CHAR(0x0c) PORT_CODE(KEYCODE_L)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K  5") PORT_CHAR('k') PORT_CHAR('K') PORT_CHAR(0x0b) PORT_CODE(KEYCODE_K)

	PORT_START("KEY6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J  4") PORT_CHAR('j') PORT_CHAR('J') PORT_CODE(KEYCODE_J)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O  3") PORT_CHAR('o') PORT_CHAR('O') PORT_CHAR(0x0f) PORT_CODE(KEYCODE_O)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I  2") PORT_CHAR('i') PORT_CHAR('I') PORT_CODE(KEYCODE_I)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U  1") PORT_CHAR('u') PORT_CHAR('U') PORT_CHAR(0x15) PORT_CODE(KEYCODE_U)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9  (  0") PORT_CHAR('9') PORT_CHAR('(') PORT_CODE(KEYCODE_9)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Line Feed") PORT_CHAR(0x0a) PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Delete") PORT_CHAR(0x7f) PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CHAR(0x0d) PORT_CODE(KEYCODE_ENTER)

	PORT_START("KEY7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD)) PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD)) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD)) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD)) PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD)) PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD)) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD)) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD)) PORT_CODE(KEYCODE_8_PAD)

	PORT_START("KEY8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD)) PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD)) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD)) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD)) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(UP)) PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_CODE(KEYCODE_LEFT)

	PORT_START("KEY9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B/W  Blink") PORT_CODE(KEYCODE_F8)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Intens  Undl") PORT_CODE(KEYCODE_F7)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Format On/Off") PORT_CODE(KEYCODE_F6)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Split On/Off") PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Clear Home") PORT_CHAR(UCHAR_MAMEKEY(HOME)) PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Clear EOL/EOF") PORT_CHAR(UCHAR_MAMEKEY(END)) PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ins Char/Line") PORT_CHAR(UCHAR_MAMEKEY(INSERT)) PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Del Char/Line") PORT_CHAR(UCHAR_MAMEKEY(DEL)) PORT_CODE(KEYCODE_F1)

	PORT_START("KEY10")
	PORT_BIT(0x3f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Send Line/Page") PORT_CODE(KEYCODE_F11)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Print Line/Page") PORT_CHAR(UCHAR_MAMEKEY(PRTSCR)) PORT_CODE(KEYCODE_F12)

	PORT_START("MODIFIERS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CHAR(UCHAR_SHIFT_1) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num") PORT_CHAR(UCHAR_MAMEKEY(NUMLOCK)) PORT_CODE(KEYCODE_LALT) PORT_TOGGLE
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CHAR(UCHAR_SHIFT_2) PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Caps Lock") PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE

	PORT_START("SPECIAL")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Break") PORT_CODE(KEYCODE_F9) PORT_WRITE_LINE_DEVICE_MEMBER("txd", input_merger_device, in_w<1>)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Line/Loc") PORT_CODE(KEYCODE_F10) PORT_TOGGLE

	PORT_START("DSW1")
	PORT_DIPNAME(0xff, 0xfd, "Printer Data Rate") PORT_DIPLOCATION("S1:8,7,6,5,4,3,2,1")
	PORT_DIPSETTING(0x7f, "110")
	PORT_DIPSETTING(0xbf, "300")
	PORT_DIPSETTING(0xdf, "600")
	PORT_DIPSETTING(0xef, "1,200")
	PORT_DIPSETTING(0xf7, "2,400")
	PORT_DIPSETTING(0xfb, "4,800")
	PORT_DIPSETTING(0xfd, "9,600")
	PORT_DIPSETTING(0xfe, "19,200")

	PORT_START("DSW2")
	PORT_DIPNAME(0xff, 0xfd, "I/O Data Rate") PORT_DIPLOCATION("S2:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(0x7f, "110")
	PORT_DIPSETTING(0xbf, "300")
	PORT_DIPSETTING(0xdf, "600")
	PORT_DIPSETTING(0xef, "1,200")
	PORT_DIPSETTING(0xf7, "2,400")
	PORT_DIPSETTING(0xfb, "4,800")
	PORT_DIPSETTING(0xfd, "9,600")
	PORT_DIPSETTING(0xfe, "19,200")

	PORT_START("DSW3")
	PORT_DIPNAME(0x001, 0x000, "Conversation Mode") PORT_DIPLOCATION("S3:1")
	PORT_DIPSETTING(0x001, "Half Duplex")
	PORT_DIPSETTING(0x000, "Full Duplex")
	PORT_DIPNAME(0x006, 0x006, "Protected Field Attribute") PORT_DIPLOCATION("S3:2,3")
	PORT_DIPSETTING(0x006, "Reduced Intensity")
	PORT_DIPSETTING(0x002, "Blinking")
	PORT_DIPSETTING(0x004, "Reverse Video")
	PORT_DIPSETTING(0x000, "Underline")
	PORT_DIPNAME(0x008, 0x008, "Display Null Character") PORT_DIPLOCATION("S3:4")
	PORT_DIPSETTING(0x000, DEF_STR(Off))
	PORT_DIPSETTING(0x008, DEF_STR(On))
	PORT_DIPNAME(0x010, 0x010, "8th Bit Transmit") PORT_DIPLOCATION("S3:5")
	PORT_DIPSETTING(0x000, "0 (Space)")
	PORT_DIPSETTING(0x010, "1 (Mark)")
	PORT_DIPNAME(0x060, 0x060, "Word Length") PORT_DIPLOCATION("S3:6,7")
	PORT_DIPSETTING(0x000, "5 Bits")
	PORT_DIPSETTING(0x040, "6 Bits")
	PORT_DIPSETTING(0x020, "7 Bits")
	PORT_DIPSETTING(0x060, "8 Bits")
	PORT_DIPNAME(0x280, 0x280, "Parity") PORT_DIPLOCATION("S3:8,10")
	PORT_DIPSETTING(0x280, "None")
	PORT_DIPSETTING(0x080, "Even")
	PORT_DIPSETTING(0x000, "Odd")
	PORT_DIPNAME(0x100, 0x100, "Number of Stop Bits") PORT_DIPLOCATION("S3:9")
	PORT_DIPSETTING(0x000, "1")
	PORT_DIPSETTING(0x100, "2")

	PORT_START("JUMPERS")
	PORT_DIPNAME(0x0003, 0x0002, "Underline") PORT_DIPLOCATION("W1:1,2")
	PORT_DIPSETTING(0x0002, "Single")
	PORT_DIPSETTING(0x0001, "Double")
	PORT_DIPNAME(0x003c, 0x0034, "Cursor Rate") PORT_DIPLOCATION("W2:1,2,3,4")
	PORT_DIPSETTING(0x001c, "0 Hz")
	PORT_DIPSETTING(0x002c, "1 Hz")
	PORT_DIPSETTING(0x0034, "2 Hz")
	PORT_DIPSETTING(0x0038, "4 Hz")
	PORT_DIPNAME(0x01c0, 0x00c0, "Blinking Rate") PORT_DIPLOCATION("W3:1,2,3")
	PORT_DIPSETTING(0x00c0, "1 Hz")
	PORT_DIPSETTING(0x0140, "2 Hz")
	PORT_DIPSETTING(0x0180, "4 Hz")
	PORT_DIPNAME(0x1e00, 0x0e00, "Protected Video Attribute") PORT_DIPLOCATION("W4:4,3,2,1")
	PORT_DIPSETTING(0x0e00, "Reduced Intensity")
	PORT_DIPSETTING(0x1600, "Blinking")
	PORT_DIPSETTING(0x1a00, "Reverse Video")
	PORT_DIPSETTING(0x1c00, "Underline")
	PORT_DIPNAME(0xe000, 0x6000, "Keyboard Auto Repeat Rate") PORT_DIPLOCATION("W5:1,2,3")
	PORT_DIPSETTING(0x6000, "7.5 cps")
	PORT_DIPSETTING(0xa000, "15 cps")
	PORT_DIPSETTING(0xc000, "30 cps")
INPUT_PORTS_END

void microterm_f8_state::act5a(machine_config &config)
{
	cpu_device &maincpu(F8(config, "maincpu", 2_MHz_XTAL));
	maincpu.set_addrmap(AS_PROGRAM, &microterm_f8_state::f8_mem);
	maincpu.set_addrmap(AS_IO, &microterm_f8_state::f8_io);
	maincpu.set_irq_acknowledge_callback("smi", FUNC(f3853_device::int_acknowledge));

	f3853_device &smi(F3853(config, "smi", 2_MHz_XTAL));
	smi.int_req_callback().set_inputline("maincpu", F8_INPUT_LINE_INT_REQ);

	AY51013(config, m_uart);
	m_uart->read_si_callback().set(m_io, FUNC(rs232_port_device::rxd_r));
	m_uart->write_so_callback().set("txd", FUNC(input_merger_device::in_w<0>));
	m_uart->write_dav_callback().set("smi", FUNC(f3853_device::ext_int_w));
	m_uart->set_auto_rdav(true);

	RS232_PORT(config, m_io, default_rs232_devices, nullptr);

	INPUT_MERGER_ALL_HIGH(config, "txd").output_handler().set(m_io, FUNC(rs232_port_device::write_txd));

	//RS232_PORT(config, m_aux, default_rs232_devices, nullptr);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(16.572_MHz_XTAL, 918, 0, 720, 301, 0, 288); // more or less guessed
	screen.set_screen_update(FUNC(microterm_f8_state::screen_update));
	screen.screen_vblank().set(FUNC(microterm_f8_state::vblank_w));
	screen.screen_vblank().append(m_blinkcount, FUNC(ripple_counter_device::clock_w)).invert();

	RIPPLE_COUNTER(config, m_blinkcount).set_stages(6);

	SPEAKER(config, "mono").front_center();
	BEEP(config, m_bell, 1760);
	m_bell->add_route(ALL_OUTPUTS, "mono", 0.50);
}

ROM_START(act5a)
	ROM_REGION(0x0c00, "maincpu", 0) // xtals 16.572MHz, 2.000MHz
	ROM_LOAD("act5a_2708.u1",  0x0000, 0x0400, CRC(ad3bfa5a) SHA1(723c7bfefbf96177171b8e58a8e20ee69daa27f0))
	ROM_LOAD("act5a_2708.u12", 0x0400, 0x0400, CRC(be4a148d) SHA1(69bf838ed9fccdf7f225bc380bcce8e7e0bd88bc))
	ROM_LOAD("act5a_2708.u22", 0x0800, 0x0400, CRC(f0ec3b9f) SHA1(7785eba9993c23a767b84fff2c44d5cb6210ad80))

	ROM_REGION(0x2000, "kbdecode", 0)
	ROM_LOAD("act5a_9316.u39", 0x0000, 0x2000, CRC(6f9eac71) SHA1(0488bdd19a6bff4e869a3480c7e9d8c5ca9938eb))

	ROM_REGION(0x2000, "chargen", 0)
	ROM_LOAD("act5a_9316.u55", 0x0000, 0x2000, CRC(8f96b7c8) SHA1(652d420ab5be9412cae322cd1799f8a9e3959c44))
ROM_END

} // anonymous namespace


//COMP(1976, act4, 0, 0, act5a, act5a, microterm_f8_state, empty_init, "Micro-Term", "ACT-IV", MACHINE_NOT_WORKING)
//COMP(1978, act5, 0, 0, act5a, act5a, microterm_f8_state, empty_init, "Micro-Term", "ACT-V", MACHINE_NOT_WORKING)
COMP(1980, act5a, 0, 0, act5a, act5a, microterm_f8_state, empty_init, "Micro-Term", "ACT-5A", MACHINE_NOT_WORKING)
