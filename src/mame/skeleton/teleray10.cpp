// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Preliminary driver for Teleray 10 series terminal.

****************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/m6502/m6502.h"
#include "machine/74259.h"
#include "machine/input_merger.h"
#include "machine/scn_pci.h"
#include "machine/timer.h"
#include "sound/spkrdev.h"
#include "screen.h"
#include "speaker.h"


namespace {

class teleray10_state : public driver_device
{
public:
	teleray10_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mainirq(*this, "mainirq")
		, m_outreg(*this, "outreg")
		, m_screen(*this, "screen")
		, m_bell(*this, "bell")
		, m_pci(*this, "pci")
		, m_serialio(*this, "serialio")
		, m_peripheral(*this, "peripheral")
		, m_scratchpad(*this, "scratchpad")
		, m_displayram(*this, "displayram")
		, m_chargen(*this, "chargen")
		, m_keys(*this, "KA%X", 0U)
		, m_swmisc(*this, "SWMISC")
	{
	}

	void teleray10(machine_config &config);

	void key_interrupt_w(int state);
	int timer_expired_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER(timer_expired);
	TIMER_CALLBACK_MEMBER(bell_toggle);
	void bell_update();

	void wide_mode_w(int state);
	void bell_off_w(int state);
	void reset_timer_w(int state);

	void scratchpad_w(offs_t offset, u8 data);
	u8 serial_io_r(offs_t offset);
	void serial_io_w(offs_t offset, u8 data);
	u8 kb_r(offs_t offset);

	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<input_merger_device> m_mainirq;
	required_device<ls259_device> m_outreg;
	required_device<screen_device> m_screen;
	required_device<speaker_sound_device> m_bell;
	required_device<scn2651_device> m_pci;
	required_device<rs232_port_device> m_serialio;
	required_device<rs232_port_device> m_peripheral;
	required_shared_ptr<u8> m_scratchpad;
	required_shared_ptr<u8> m_displayram;
	required_region_ptr<u8> m_chargen;
	required_ioport_array<16> m_keys;
	required_ioport m_swmisc;

	emu_timer *m_bell_timer;

	u8 m_topr;
	bool m_timer_expired;
};


void teleray10_state::machine_start()
{
	m_bell_timer = timer_alloc(FUNC(teleray10_state::bell_toggle), this);

	m_topr = 0;
	m_timer_expired = false;

	// HACK: force continuous CTS
	subdevice<input_merger_device>("cts")->in_w<2>(0);

	save_item(NAME(m_topr));
	save_item(NAME(m_timer_expired));
}

void teleray10_state::machine_reset()
{
	// Pins 5 (CTS), 6 (DSR), 8 (DCD) are pulled up to +12V on peripheral connector
	m_peripheral->write_rts(0);
	m_peripheral->write_dtr(0);
}


TIMER_DEVICE_CALLBACK_MEMBER(teleray10_state::timer_expired)
{
	// arbitrary VBLANK condition
	if (param >= 240 && param < 290)
	{
		int height = BIT(m_swmisc->read(), 2) ? 310 : 372;
		if (height != m_screen->height())
			m_screen->configure(1000, height, m_screen->visible_area(), attotime::from_ticks(1000 * height, 18.6_MHz_XTAL).as_attoseconds());
	}

	if (m_outreg->q7_r())
	{
		m_timer_expired = true;
		m_mainirq->in_w<1>(1);
	}
}

TIMER_CALLBACK_MEMBER(teleray10_state::bell_toggle)
{
	bell_update();
}

void teleray10_state::bell_update()
{
	int vpos = m_screen->vpos();
	int line = vpos % 12;
	if (line < 8)
	{
		m_bell->level_w(1);
		m_bell_timer->adjust(m_screen->time_until_pos(vpos - line + 8));
	}
	else
	{
		m_bell->level_w(0);
		if (vpos - line + 12 < m_screen->height())
			m_bell_timer->adjust(m_screen->time_until_pos(vpos - line + 12));
		else
			m_bell_timer->adjust(m_screen->time_until_pos(0));
	}
}


u32 teleray10_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	u16 offset = 0;

	for (unsigned y = 0; y < 24; y++)
	{
		for (unsigned scan = 0; scan < 12; scan++)
		{
			u32 *px = &bitmap.pix(y * 12 + scan);
			for (unsigned x = 0; x < 80; x++)
			{
				u8 ch = m_displayram[offset + x];
				u8 dots = m_chargen[u16(ch) << 4 | scan] & 0x7f;
				for (int i = 0; i < 10; i++)
				{
					*px++ = BIT(dots, 6) ? rgb_t::white() : rgb_t::black();
					dots <<= 1;
				}
			}
		}
		offset += 80;
	}
	return 0;
}


void teleray10_state::key_interrupt_w(int state)
{
	m_maincpu->set_input_line(m6502_device::NMI_LINE, state ? CLEAR_LINE : ASSERT_LINE);
}

void teleray10_state::wide_mode_w(int state)
{
}

void teleray10_state::bell_off_w(int state)
{
	if (state)
	{
		m_bell->level_w(0);
		m_bell_timer->adjust(attotime::never);
	}
	else
		bell_update();
}

void teleray10_state::reset_timer_w(int state)
{
	if (!state)
	{
		m_timer_expired = false;
		m_mainirq->in_w<1>(0);
	}
}

int teleray10_state::timer_expired_r()
{
	return m_timer_expired;
}

void teleray10_state::scratchpad_w(offs_t offset, u8 data)
{
	// Output control register and top of page register overlap with scratchpad RAM
	if (offset < 0x10)
		m_outreg->write_bit(~offset & 7, BIT(data, 0));
	else if (offset < 0x20)
		m_topr = data;

	m_scratchpad[offset] = data;
}

u8 teleray10_state::serial_io_r(offs_t offset)
{
	return m_pci->read(~offset & 3);
}

void teleray10_state::serial_io_w(offs_t offset, u8 data)
{
	m_pci->write(~offset & 3, data);
}

u8 teleray10_state::kb_r(offs_t offset)
{
	// DM74154 1 of 16 decoder on keyboard
	return m_keys[offset]->read();
}

void teleray10_state::mem_map(address_map &map)
{
	map.global_mask(0x9fff);
	map(0x0000, 0x000f).r(FUNC(teleray10_state::kb_r));
	map(0x0010, 0x0010).mirror(0xe).portr("SW1B");
	map(0x0011, 0x0011).mirror(0xe).portr("SW1A");
	map(0x0020, 0x0020).mirror(0xe).portr("SW2B");
	map(0x0021, 0x0021).mirror(0xe).portr("SW2A");
	map(0x0030, 0x0033).mirror(0xc).rw(FUNC(teleray10_state::serial_io_r), FUNC(teleray10_state::serial_io_w));
	map(0x0040, 0x03ff).ram().share("scratchpad").w(FUNC(teleray10_state::scratchpad_w));
	map(0x0400, 0x0bff).ram().share("displayram");
	// $0C00 to $0FFF = PROM #5 @ 1L (empty socket here)
	map(0x9000, 0x9fff).rom().region("proms", 0);
}


static INPUT_PORTS_START(teleray10)
	PORT_START("KA0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('0') PORT_CHAR(')') PORT_CODE(KEYCODE_0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('1') PORT_CHAR('!') PORT_CODE(KEYCODE_1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('2') PORT_CHAR('@') PORT_CODE(KEYCODE_2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('3') PORT_CHAR('#') PORT_CODE(KEYCODE_3)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('4') PORT_CHAR('$') PORT_CODE(KEYCODE_4)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('5') PORT_CHAR('%') PORT_CODE(KEYCODE_5)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('6') PORT_CHAR('^') PORT_CODE(KEYCODE_6)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('7') PORT_CHAR('&') PORT_CODE(KEYCODE_7)

	PORT_START("KA1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('8') PORT_CHAR('*') PORT_CODE(KEYCODE_8)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('9') PORT_CHAR('(') PORT_CODE(KEYCODE_9)
	PORT_BIT(0xfc, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KA2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(' ') PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('a') PORT_CHAR('A') PORT_CHAR(0x01) PORT_CODE(KEYCODE_A)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('b') PORT_CHAR('B') PORT_CHAR(0x02) PORT_CODE(KEYCODE_B)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('c') PORT_CHAR('C') PORT_CHAR(0x03) PORT_CODE(KEYCODE_C)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('d') PORT_CHAR('D') PORT_CHAR(0x04) PORT_CODE(KEYCODE_D)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('e') PORT_CHAR('E') PORT_CHAR(0x05) PORT_CODE(KEYCODE_E)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('f') PORT_CHAR('F') PORT_CHAR(0x06) PORT_CODE(KEYCODE_F)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('g') PORT_CHAR('G') PORT_CHAR(0x07) PORT_CODE(KEYCODE_G)

	PORT_START("KA3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('h') PORT_CHAR('H') PORT_CODE(KEYCODE_H)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('i') PORT_CHAR('I') PORT_CODE(KEYCODE_I)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('j') PORT_CHAR('J') PORT_CODE(KEYCODE_J)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('k') PORT_CHAR('K') PORT_CHAR(0x0b) PORT_CODE(KEYCODE_K)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('l') PORT_CHAR('L') PORT_CHAR(0x0c) PORT_CODE(KEYCODE_L)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('m') PORT_CHAR('M') PORT_CODE(KEYCODE_M)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('n') PORT_CHAR('N') PORT_CHAR(0x0e) PORT_CODE(KEYCODE_N)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('o') PORT_CHAR('O') PORT_CHAR(0x0f) PORT_CODE(KEYCODE_O)

	PORT_START("KA4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('p') PORT_CHAR('P') PORT_CHAR(0x10) PORT_CODE(KEYCODE_P)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('q') PORT_CHAR('Q') PORT_CHAR(0x11) PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('r') PORT_CHAR('R') PORT_CHAR(0x12) PORT_CODE(KEYCODE_R)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('s') PORT_CHAR('S') PORT_CHAR(0x13) PORT_CODE(KEYCODE_S)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('t') PORT_CHAR('T') PORT_CHAR(0x14) PORT_CODE(KEYCODE_T)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('u') PORT_CHAR('U') PORT_CHAR(0x15) PORT_CODE(KEYCODE_U)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('v') PORT_CHAR('V') PORT_CHAR(0x16) PORT_CODE(KEYCODE_V)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('w') PORT_CHAR('W') PORT_CHAR(0x17) PORT_CODE(KEYCODE_W)

	PORT_START("KA5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('x') PORT_CHAR('X') PORT_CHAR(0x18) PORT_CODE(KEYCODE_X)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('y') PORT_CHAR('Y') PORT_CHAR(0x19) PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('z') PORT_CHAR('Z') PORT_CHAR(0x1a) PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('[') PORT_CHAR('{') PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('\\') PORT_CHAR('|') PORT_CHAR(0x1c) PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(']') PORT_CHAR('}') PORT_CHAR(0x1d) PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('-') PORT_CHAR('_') PORT_CODE(KEYCODE_MINUS)

	PORT_START("KA6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('`') PORT_CHAR('~') PORT_CHAR(0x1e) PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('\'') PORT_CHAR('"') PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(';') PORT_CHAR(':') PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(',') PORT_CHAR('<') PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('=') PORT_CHAR('+') PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('.') PORT_CHAR('>') PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('/') PORT_CHAR('?') PORT_CHAR(0x1f) PORT_CODE(KEYCODE_SLASH)

	PORT_START("KA7")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KA8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Back Space") PORT_CHAR(0x08) PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Tab  Back Tab") PORT_CHAR(0x09) PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Line Feed") PORT_CHAR(0x0a) PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Escape") PORT_CHAR(0x1b) PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CHAR(0x0d) PORT_CODE(KEYCODE_ENTER) // also called CR
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Break")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Rub Out") PORT_CHAR(0x7f) // also called DEL

	PORT_START("KA9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_CHAR(UCHAR_MAMEKEY(F5)) PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F2)) PORT_CHAR(UCHAR_MAMEKEY(F6)) PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_CHAR(UCHAR_MAMEKEY(F7)) PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F4)) PORT_CHAR(UCHAR_MAMEKEY(F8)) PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Clear EOL") PORT_CODE(KEYCODE_F8)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Tab Set  Tab Clr") PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Clear Page") PORT_CODE(KEYCODE_F6)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Clear EOP") PORT_CODE(KEYCODE_F7)

	PORT_START("KAA")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Del Line") PORT_CODE(KEYCODE_F9)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Insrt Line") PORT_CODE(KEYCODE_F10)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Del Char") PORT_CHAR(UCHAR_MAMEKEY(DEL)) PORT_CODE(KEYCODE_F11)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Insrt Char") PORT_CHAR(UCHAR_MAMEKEY(INSERT)) PORT_CODE(KEYCODE_F12)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Xmit Line")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Xmit Msg")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Xmit Page")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Print") PORT_CHAR(UCHAR_MAMEKEY(PRTSCR)) PORT_CODE(KEYCODE_PRTSCR)

	PORT_START("KAB")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Home LF") PORT_CHAR(UCHAR_MAMEKEY(HOME)) PORT_CODE(KEYCODE_HOME)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(UP)) PORT_CODE(KEYCODE_UP)
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KAC")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD)) PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD)) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD)) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD)) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD)) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD)) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD)) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD)) PORT_CODE(KEYCODE_7_PAD)

	PORT_START("KAD")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD)) PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD)) PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD)) PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT(0x18, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad CR") PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD)) PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Control") PORT_CHAR(UCHAR_SHIFT_2) PORT_CODE(KEYCODE_LCONTROL)

	PORT_START("KAE")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right Shift") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x3e, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Caps Lock") PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left Shift") PORT_CHAR(UCHAR_SHIFT_1) PORT_CODE(KEYCODE_LSHIFT)

	PORT_START("KAF")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_TOGGLE PORT_NAME("Page") // also called Scroll
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_TOGGLE PORT_NAME("Block")
	PORT_BIT(0x3c, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_TOGGLE PORT_NAME("Monitor") // also called Xprnt
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_TOGGLE PORT_NAME("Local")

	PORT_START("KINT")
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Interrupt") PORT_WRITE_LINE_MEMBER(teleray10_state, key_interrupt_w)

	PORT_START("SW1A")
	PORT_DIPNAME(0x01, 0x00, "Xmit ETX") PORT_DIPLOCATION("7A:3")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x01, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x00, "Xmit CSR") PORT_DIPLOCATION("7A:2")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x02, DEF_STR(On))
	PORT_DIPNAME(0x04, 0x04, "Character Bits") PORT_DIPLOCATION("6A:8")
	PORT_DIPSETTING(0x00, "7")
	PORT_DIPSETTING(0x04, "8")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN) // "Reserved" jumper
	PORT_DIPNAME(0x30, 0x30, "Parity") PORT_DIPLOCATION("6A:6,5")
	PORT_DIPSETTING(0x00, "Even")
	PORT_DIPSETTING(0x10, "Odd")
	PORT_DIPSETTING(0x30, "None/High")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(teleray10_state, timer_expired_r)
	PORT_DIPNAME(0x80, 0x00, "Stop Bits") PORT_DIPLOCATION("6A:7")
	PORT_DIPSETTING(0x00, "1")
	PORT_DIPSETTING(0x80, "2")

	PORT_START("SW1B")
	PORT_DIPNAME(0x0f, 0x0e, "Baud Rate") PORT_DIPLOCATION("6A:4,3,2,1")
	PORT_DIPSETTING(0x00, "50")
	PORT_DIPSETTING(0x01, "75")
	PORT_DIPSETTING(0x02, "110")
	PORT_DIPSETTING(0x03, "134.5")
	PORT_DIPSETTING(0x04, "150")
	PORT_DIPSETTING(0x05, "300")
	PORT_DIPSETTING(0x06, "600")
	PORT_DIPSETTING(0x07, "1200")
	PORT_DIPSETTING(0x08, "1800")
	PORT_DIPSETTING(0x09, "2000")
	PORT_DIPSETTING(0x0a, "2400")
	PORT_DIPSETTING(0x0b, "3600")
	PORT_DIPSETTING(0x0c, "4800")
	PORT_DIPSETTING(0x0d, "7200")
	PORT_DIPSETTING(0x0e, "9600")
	PORT_DIPSETTING(0x0f, "Reserved")
	PORT_DIPNAME(0x10, 0x10, "Half/Full Duplex") PORT_DIPLOCATION("7A:4")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x10, DEF_STR(On))
	PORT_DIPNAME(0x20, 0x00, "Right Margin Wrap") PORT_DIPLOCATION("7A:5")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x20, DEF_STR(On))
	PORT_DIPNAME(0x40, 0x00, "LF NL/CR NL") PORT_DIPLOCATION("7A:7")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x40, DEF_STR(On))
	PORT_DIPNAME(0x80, 0x00, "New Line") PORT_DIPLOCATION("7A:6")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x80, DEF_STR(On))

	PORT_START("SW2A")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("SW2B")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("SWMISC")
	PORT_DIPNAME(0x01, 0x01, "Display Attributes") PORT_DIPLOCATION("7A:1")
	PORT_DIPSETTING(0x01, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x02, "Inverse Display") PORT_DIPLOCATION("INVRS:1") // internal jumper near 4M
	PORT_DIPSETTING(0x02, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x04, 0x04, "Refresh Rate") PORT_DIPLOCATION("50Hz:1") // internal jumper near 2J
	PORT_DIPSETTING(0x00, "50 Hz")
	PORT_DIPSETTING(0x04, "60 Hz")
	PORT_DIPNAME(0x08, 0x08, "Serial Loop") PORT_DIPLOCATION("7A:8")
	PORT_DIPSETTING(0x08, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
INPUT_PORTS_END


void teleray10_state::teleray10(machine_config &config)
{
	M6502(config, m_maincpu, 18.6_MHz_XTAL / 20); // 2 characters loaded during each ϕ1 cycle
	m_maincpu->set_addrmap(AS_PROGRAM, &teleray10_state::mem_map);

	TIMER(config, "timer").configure_scanline(FUNC(teleray10_state::timer_expired), "screen", 10, 12);

	INPUT_MERGER_ANY_HIGH(config, m_mainirq).output_handler().set_inputline(m_maincpu, m6502_device::IRQ_LINE);

	LS259(config, m_outreg); // Output Control Register @ 7D
	m_outreg->q_out_cb<0>().set("xmitser", FUNC(input_merger_device::in_w<0>)).invert();
	m_outreg->q_out_cb<1>().set("xmitperiph", FUNC(input_merger_device::in_w<1>)).invert();
	// Q2 officially "reserved"
	m_outreg->q_out_cb<3>().set("cts", FUNC(input_merger_device::in_w<0>));
	m_outreg->q_out_cb<4>().set(m_serialio, FUNC(rs232_port_device::write_dtr));
	m_outreg->q_out_cb<5>().set(FUNC(teleray10_state::wide_mode_w));
	m_outreg->q_out_cb<6>().set(FUNC(teleray10_state::bell_off_w));
	m_outreg->q_out_cb<7>().set(FUNC(teleray10_state::reset_timer_w));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(18.6_MHz_XTAL, 1000, 0, 800, 310, 0, 288); // 372 total lines in 50 Hz mode
	m_screen->set_screen_update(FUNC(teleray10_state::screen_update));

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_bell).add_route(ALL_OUTPUTS, "mono", 0.5);

	SCN2651(config, m_pci, 5.0688_MHz_XTAL); // Signetics 2651 or equivalent
	m_pci->rxrdy_handler().set(m_mainirq, FUNC(input_merger_device::in_w<0>));
	m_pci->txd_handler().set("xmitser", FUNC(input_merger_device::in_w<1>));
	m_pci->txd_handler().append("xmitperiph", FUNC(input_merger_device::in_w<0>));
	m_pci->rts_handler().set(m_serialio, FUNC(rs232_port_device::write_rts));

	RS232_PORT(config, m_serialio, default_rs232_devices, nullptr);
	m_serialio->rxd_handler().set(m_pci, FUNC(scn2651_device::rxd_w));
	m_serialio->cts_handler().set("cts", FUNC(input_merger_device::in_w<1>));
	m_serialio->dcd_handler().set(m_pci, FUNC(scn2651_device::dcd_w));

	RS232_PORT(config, m_peripheral, default_rs232_devices, nullptr);

	INPUT_MERGER_ANY_HIGH(config, "xmitser").output_handler().set(m_serialio, FUNC(rs232_port_device::write_txd));
	INPUT_MERGER_ANY_HIGH(config, "xmitperiph").output_handler().set(m_peripheral, FUNC(rs232_port_device::write_txd));
	INPUT_MERGER_ALL_HIGH(config, "cts").output_handler().set(m_pci, FUNC(scn2651_device::cts_w));
}


ROM_START(teleray10)
	ROM_REGION(0x1000, "proms", 0) // 2708
	ROM_LOAD("a053730_4g.1g", 0x000, 0x400, CRC(6b7cb60f) SHA1(3031a8c2eb20f4345dc7308e073a7ac0ebeefab0))
	ROM_LOAD("a053730_3g.1j", 0x400, 0x400, CRC(634d7a31) SHA1(2cccc3b199e3cd79b3f6b73a94f9349363a102e1))
	ROM_LOAD("a053730_2g.1k", 0x800, 0x400, CRC(dd8174c1) SHA1(605f53ed348b87f2bb3fa25344e68ad59dd21159))
	ROM_LOAD("a053730_1g.1l", 0xc00, 0x400, CRC(01283a2e) SHA1(09edee6ded6eb15932467f226b9a97e07304c99e))

	ROM_REGION(0x800, "chargen", 0)
	ROM_LOAD("ka53895.7n", 0x000, 0x800, CRC(437cf3cc) SHA1(4da7eea06b6b5f6c0a3d995b727d6f8d14bb8b30))
ROM_END

} // anonymous namespace


COMP(1978, teleray10, 0, 0, teleray10, teleray10, teleray10_state, empty_init, "Research Inc.", "Teleray Model 10", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS)
