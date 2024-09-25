// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Preliminary driver for Falco TS-series terminals.

***************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/input_merger.h"
#include "machine/nvram.h"
#include "machine/ripple_counter.h"
#include "machine/rstbuf.h"
#include "machine/scn_pci.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "sound/spkrdev.h"
#include "video/mc6845.h"
#include "screen.h"
#include "speaker.h"


namespace {

class falcots_state : public driver_device
{
public:
	falcots_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rstbuf(*this, "rstbuf")
		, m_lineint(*this, "lineint")
		, m_bell(*this, "bell")
		, m_crtc(*this, "crtc")
		, m_blinkcnt(*this, "blinkcnt")
		, m_vram(*this, "vram")
		, m_chargen(*this, "chargen")
		, m_keys(*this, "KEY%X", 0U)
	{
	}

	void ts1(machine_config &config);
	void ts2624(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	MC6845_UPDATE_ROW(ts1_update_row);
	MC6845_UPDATE_ROW(update_row);
	void load_counters();
	void row_clock_w(int state);
	void vsync_w(int state);
	void bell_toggle_w(int state);

	u8 key_status_r();
	void key_scan_w(u8 data);
	void video_reset_w(u8 data);
	void line_addr_upper_w(u8 data);
	void line_addr_lower_w(u8 data);
	void line_attr_w(u8 data);
	void line_int_clear_w(u8 data);
	void brightness_w(u8 data);
	void bell_w(u8 data);
	void control_w(u8 data);

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void ts1_mem_map(address_map &map) ATTR_COLD;
	void ts1_io_map(address_map &map) ATTR_COLD;

	required_device<z80_device> m_maincpu;
	optional_device<rst_pos_buffer_device> m_rstbuf;
	required_device<input_merger_device> m_lineint;
	optional_device<speaker_sound_device> m_bell;
	required_device<mc6845_device> m_crtc;
	required_device<ripple_counter_device> m_blinkcnt;
	required_shared_ptr<u8> m_vram;
	required_region_ptr<u8> m_chargen;
	optional_ioport_array<16> m_keys;

	u16 m_line_addr_latch = 0;
	u16 m_line_addr_base = 0;
	u8 m_lines_left = 0;
	u8 m_line_attr_latch = 0;
	u8 m_line_attr = 0;
	u8 m_brightness = 0;
	u8 m_key_scan = 0;
	bool m_bell_toggle = false;
};

void falcots_state::machine_start()
{
	save_item(NAME(m_line_addr_latch));
	save_item(NAME(m_line_addr_base));
	save_item(NAME(m_lines_left));
	save_item(NAME(m_line_attr_latch));
	save_item(NAME(m_line_attr));
	save_item(NAME(m_brightness));
	save_item(NAME(m_key_scan));
	if (!m_rstbuf.found())
		save_item(NAME(m_bell_toggle));
}

void falcots_state::machine_reset()
{
	control_w(0);
}

MC6845_UPDATE_ROW(falcots_state::ts1_update_row)
{
	rgb_t fg(m_brightness, m_brightness, m_brightness);
	u32 *pix = &bitmap.pix(y);

	if (BIT(m_line_attr, 5))
		x_count /= 2;
	for (int x = 0; x < x_count; ++x)
	{
		u8 char_code = BIT(m_line_attr, 4) ? m_vram[(m_line_addr_base + x) & 0x7ff] : 0;
		u8 attr_code = BIT(m_line_attr, 4) ? m_vram[0x800 + ((m_line_addr_base + x) & 0x7ff)] : 0;

		u8 dots = 0;
		if (!BIT(attr_code, 5))
		{
			if (!BIT(attr_code, 1) || !BIT(m_blinkcnt->count(), 4))
				dots = ~m_chargen[u16(char_code & 0x7f) << 4 | (m_line_attr & 0x0f)];
			if (BIT(attr_code, 0) && (m_line_attr & 0x0b) == 0x0b)
				dots = ~dots;
			if (BIT(attr_code, 2))
				dots = ~dots;
		}

		for (int n = 8; n-- != 0; )
		{
			*pix++ = BIT(dots, n) ? fg : rgb_t::black();
			if (BIT(m_line_attr, 5))
				*pix++ = BIT(dots, n) ? fg : rgb_t::black();
		}
	}
}

MC6845_UPDATE_ROW(falcots_state::update_row)
{
	rgb_t fg(m_brightness, m_brightness, m_brightness);
	u32 *pix = &bitmap.pix(y);

	if (!BIT(m_line_attr, 5))
		x_count *= 2;
	for (int x = 0; x < x_count; ++x)
	{
		u8 char_code = BIT(m_line_attr, 4) ? m_vram[(m_line_addr_base + x) & 0x0fff] : 0;
		u8 attr_code = BIT(m_line_attr, 4) ? m_vram[0x1000 + ((m_line_addr_base + x) & 0x0fff)] : 0;

		u8 dots = 0;
		if (!BIT(attr_code, 5))
		{
			if (!BIT(attr_code, 1) || !BIT(m_blinkcnt->count(), 4))
				dots = ~m_chargen[u16(attr_code & 0x18) << 8 | u16(char_code & 0x7f) << 4 | (m_line_attr & 0x0f)];
			if (BIT(attr_code, 0) && (m_line_attr & 0x0b) == 0x0b)
				dots = ~dots;
			if (BIT(attr_code, 2))
				dots = ~dots;
		}

		for (int n = 8; n-- != 0; )
		{
			*pix++ = BIT(dots, n) ? fg : rgb_t::black();
			if (BIT(m_line_attr, 5))
				*pix++ = BIT(dots, n) ? fg : rgb_t::black();
		}
	}
}

void falcots_state::load_counters()
{
	m_line_addr_base = m_line_addr_latch >> 4;
	if (m_rstbuf.found())
		m_lines_left = ((m_line_attr_latch & 0x0f) >= 0x0c ? 0x0f : 0x0b) - (m_line_attr_latch & 0x0f);
	else
		m_lines_left = ~m_line_addr_latch & 0x000f;
	if (m_rstbuf.found() && BIT(m_line_attr_latch, 6))
		m_line_attr = (m_line_attr_latch & 0xf0) | (m_line_attr & 0x0f);
	else
		m_line_attr = m_line_attr_latch;
}

void falcots_state::row_clock_w(int state)
{
	if (state)
	{
		if (m_lines_left == 0)
			m_lineint->in_w<1>(1);
	}
	else
	{
		if (m_lines_left == 0)
			load_counters();
		else
		{
			--m_lines_left;
			if (BIT(m_line_attr, 7) != m_rstbuf.found())
				m_line_attr = (m_line_attr & 0xf0) | ((m_line_attr + 1) & 0x0f);
			if (BIT(m_line_attr, 6))
				m_line_attr ^= 0x80;
		}
		if (!m_rstbuf.found())
			m_lineint->in_w<1>(0);
	}
}

void falcots_state::vsync_w(int state)
{
	if (state)
	{
		m_lineint->in_w<1>(1);
	}
	else
	{
		if (!m_rstbuf.found())
			m_lineint->in_w<1>(0);
		load_counters();
	}
}

void falcots_state::bell_toggle_w(int state)
{
	if (state)
	{
		m_bell_toggle = !m_bell_toggle;
		m_bell->level_w(m_bell_toggle);
	}
}

u8 falcots_state::key_status_r()
{
	u8 status = m_crtc->vsync_r() ? 0x00 : 0x02;

	u8 i = (m_key_scan & 0x70) >> 4;
	u8 j = m_key_scan & 0x0f;
	if (!BIT(m_keys[j].read_safe(0xff), i))
		status |= 0x01;

	return status;
}

void falcots_state::key_scan_w(u8 data)
{
	// TODO: this is a vastly oversimplification of the keyboard interface
	// (actual interface uses various shift registers driven by an unknown clock)
	m_key_scan = data;
}

void falcots_state::video_reset_w(u8 data)
{
	m_line_addr_latch = 0;
	m_line_attr_latch = 0;
}

void falcots_state::line_addr_upper_w(u8 data)
{
	m_line_addr_latch = u16(data) << 8 | (m_line_addr_latch & 0x00ff);
}

void falcots_state::line_addr_lower_w(u8 data)
{
	m_line_addr_latch = (m_line_addr_latch & 0xff00) | data;
}

void falcots_state::line_attr_w(u8 data)
{
	m_line_attr_latch = data;
}

void falcots_state::line_int_clear_w(u8 data)
{
	m_lineint->in_w<1>(0);
}

void falcots_state::brightness_w(u8 data)
{
	m_brightness = data;
}

void falcots_state::bell_w(u8 data)
{
	// TODO
	logerror("%s: Bell triggered\n", machine().describe_context());
}

void falcots_state::control_w(u8 data)
{
	m_lineint->in_w<0>(BIT(data, 0));
}

void falcots_state::ts1_mem_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("maincpu", 0);
	map(0x8000, 0xbfff).ram(); // 8x NEC D416C
	map(0xec00, 0xefff).ram().share("nvram");
	map(0xf000, 0xffff).ram().share("vram"); // 6x(!?) AM9114EPC
}

void falcots_state::ts1_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0xe0, 0xe0).w(FUNC(falcots_state::video_reset_w));
	map(0xe1, 0xe1).w(FUNC(falcots_state::key_scan_w));
	map(0xe2, 0xe2).w(FUNC(falcots_state::line_addr_upper_w));
	map(0xe3, 0xe3).w(FUNC(falcots_state::brightness_w));
	map(0xe4, 0xe4).w(FUNC(falcots_state::line_attr_w));
	map(0xe5, 0xe5).w(FUNC(falcots_state::line_int_clear_w));
	map(0xe6, 0xe6).w(FUNC(falcots_state::bell_w));
	map(0xe7, 0xe7).w(FUNC(falcots_state::control_w));
	map(0xe8, 0xe8).r(FUNC(falcots_state::key_status_r));
	map(0xf0, 0xf0).w(m_crtc, FUNC(mc6845_device::address_w));
	map(0xf1, 0xf1).w(m_crtc, FUNC(mc6845_device::register_w));
	map(0xf8, 0xfb).r("pci", FUNC(scn2651_device::read));
	map(0xfc, 0xff).w("pci", FUNC(scn2651_device::write));
}

void falcots_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
	map(0x8000, 0x87ff).ram().share("nvram");
	map(0xa000, 0xbfff).ram().share("vram"); // 4x HM6116P-3
	map(0xc000, 0xffff).ram(); // 8x AM9016EPC (4116)
}

void falcots_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0xe0, 0xe0).r(FUNC(falcots_state::key_status_r));
	map(0xe0, 0xe0).w(FUNC(falcots_state::video_reset_w));
	map(0xe1, 0xe1).w(FUNC(falcots_state::key_scan_w));
	map(0xe2, 0xe2).w(FUNC(falcots_state::line_addr_upper_w));
	map(0xe3, 0xe3).w(FUNC(falcots_state::brightness_w));
	map(0xe4, 0xe4).w(FUNC(falcots_state::line_attr_w));
	map(0xe5, 0xe5).w(FUNC(falcots_state::line_addr_lower_w));
	map(0xe7, 0xe7).w(FUNC(falcots_state::control_w));
	map(0xe8, 0xe8).w(m_crtc, FUNC(mc6845_device::address_w));
	map(0xe9, 0xe9).w(m_crtc, FUNC(mc6845_device::register_w));
	map(0xeb, 0xeb).r(m_crtc, FUNC(mc6845_device::register_r));
	map(0xf0, 0xf3).rw("dart", FUNC(z80dart_device::ba_cd_r), FUNC(z80dart_device::ba_cd_w));
	map(0xf8, 0xfb).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
}

static INPUT_PORTS_START(ts1)
	PORT_START("KEY0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1  !  F1") PORT_CHAR('1') PORT_CHAR('!') PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Escape") PORT_CHAR(0x1b) PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q  Set Tab") PORT_CHAR('q') PORT_CHAR('Q') PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Tab  Back Tab") PORT_CHAR(0x09) PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A  Block Mode") PORT_CHAR('a') PORT_CHAR('A') PORT_CODE(KEYCODE_A)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Control") PORT_CHAR(UCHAR_SHIFT_2) PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left Shift") PORT_CHAR(UCHAR_SHIFT_1) PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift Caps  Pad") PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_CODE(KEYCODE_CAPSLOCK)

	PORT_START("KEY1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"3  # £  F3") PORT_CHAR('3') PORT_CHAR('#') PORT_CODE(KEYCODE_3)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2  \"  F2") PORT_CHAR('2') PORT_CHAR('"') PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W  Clr Tab") PORT_CHAR('w') PORT_CHAR('W') PORT_CODE(KEYCODE_W)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S  Conv Mode") PORT_CHAR('s') PORT_CHAR('S') PORT_CODE(KEYCODE_S)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z  Ins Char") PORT_CHAR('z') PORT_CHAR('Z') PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X  Del Char") PORT_CHAR('x') PORT_CHAR('X') PORT_CODE(KEYCODE_X)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Function") PORT_CHAR(UCHAR_MAMEKEY(LALT)) PORT_CODE(KEYCODE_LALT)

	PORT_START("KEY2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5  %  F5") PORT_CHAR('5') PORT_CHAR('%') PORT_CODE(KEYCODE_5)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4  $  F4") PORT_CHAR('4') PORT_CHAR('$') PORT_CODE(KEYCODE_4)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R  Scroll Down") PORT_CHAR('r') PORT_CHAR('R') PORT_CODE(KEYCODE_R)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E  Clr All Tabs") PORT_CHAR('e') PORT_CHAR('E') PORT_CODE(KEYCODE_E)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F  Freeze") PORT_CHAR('f') PORT_CHAR('F') PORT_CODE(KEYCODE_F)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CHAR('d') PORT_CHAR('D') PORT_CODE(KEYCODE_D)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V  Del Line") PORT_CHAR('v') PORT_CHAR('V') PORT_CODE(KEYCODE_V)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C  Ins Line") PORT_CHAR('c') PORT_CHAR('C') PORT_CODE(KEYCODE_C)

	PORT_START("KEY3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6  &  F6") PORT_CHAR('6') PORT_CHAR('&') PORT_CODE(KEYCODE_6)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T  Scroll Up") PORT_CHAR('t') PORT_CHAR('T') PORT_CODE(KEYCODE_T)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CHAR('g') PORT_CHAR('G') PORT_CODE(KEYCODE_G)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CHAR('b') PORT_CHAR('B') PORT_CODE(KEYCODE_B)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(' ') PORT_CODE(KEYCODE_SPACE)

	PORT_START("KEY4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7  '  F7") PORT_CHAR('7') PORT_CHAR('\'') PORT_CODE(KEYCODE_7)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8  (  F8") PORT_CHAR('8') PORT_CHAR('(') PORT_CODE(KEYCODE_8)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y") PORT_CHAR('y') PORT_CHAR('Y') PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U") PORT_CHAR('u') PORT_CHAR('U') PORT_CODE(KEYCODE_U)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H") PORT_CHAR('h') PORT_CHAR('H') PORT_CODE(KEYCODE_H)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J") PORT_CHAR('j') PORT_CHAR('J') PORT_CODE(KEYCODE_J)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M  Send Page") PORT_CHAR('m') PORT_CHAR('M') PORT_CODE(KEYCODE_M)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N  Send Line") PORT_CHAR('n') PORT_CHAR('N') PORT_CODE(KEYCODE_N)

	PORT_START("KEY5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9  )  F9") PORT_CHAR('9') PORT_CHAR(')') PORT_CODE(KEYCODE_9)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0  _  F10") PORT_CHAR('0') PORT_CHAR('_') PORT_CODE(KEYCODE_0)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CHAR('i') PORT_CHAR('I') PORT_CODE(KEYCODE_I)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O") PORT_CHAR('o') PORT_CHAR('O') PORT_CODE(KEYCODE_O)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K") PORT_CHAR('k') PORT_CHAR('K') PORT_CODE(KEYCODE_K)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L  Erase Line") PORT_CHAR('l') PORT_CHAR('L') PORT_CODE(KEYCODE_L)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".  >  Print Line") PORT_CHAR('.') PORT_CHAR('>') PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(',') PORT_CHAR('<') PORT_CODE(KEYCODE_COMMA)

	PORT_START("KEY6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-  =  F11") PORT_CHAR('-') PORT_CHAR('=') PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("^  ~  F12") PORT_CHAR('^') PORT_CHAR('~') PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P") PORT_CHAR('p') PORT_CHAR('P') PORT_CODE(KEYCODE_P)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('[') PORT_CHAR('{') PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(";  +  Erase Page") PORT_CHAR(';') PORT_CHAR('+') PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(":  *  Erase") PORT_CHAR(':') PORT_CHAR('*') PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/  ?  Print Page") PORT_CHAR('/') PORT_CHAR('?') PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Back Space") PORT_CHAR(0x08) PORT_CODE(KEYCODE_RALT)

	PORT_START("KEY7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\\  |  Examine F") PORT_CHAR('\\') PORT_CHAR('|') PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Break") PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(']') PORT_CHAR('}') PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("@  `  Set F") PORT_CHAR('@') PORT_CHAR('`') PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Rub Out  Clear") PORT_CHAR(0x7f) PORT_CODE(KEYCODE_DEL)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CHAR(0x0d) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Line Feed") PORT_CHAR(0x0a) PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right Shift") PORT_CODE(KEYCODE_RSHIFT)

	PORT_START("KEY8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD)) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Keypad 8  \u2191 (Brite)") PORT_CHAR(UCHAR_MAMEKEY(8_PAD)) PORT_CHAR(UCHAR_MAMEKEY(UP)) PORT_CODE(KEYCODE_8_PAD) // U+2191 = ↑
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Keypad 4  \u2190") PORT_CHAR(UCHAR_MAMEKEY(4_PAD)) PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_CODE(KEYCODE_4_PAD) // U+2190 = ←
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 5  Home") PORT_CHAR(UCHAR_MAMEKEY(5_PAD)) PORT_CHAR(UCHAR_MAMEKEY(HOME)) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD)) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Keypad 2  \u2193 (Dim)") PORT_CHAR(UCHAR_MAMEKEY(2_PAD)) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_CODE(KEYCODE_2_PAD) // U+2193 = ↓
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0  Set Up") PORT_CHAR(UCHAR_MAMEKEY(0_PAD)) PORT_CODE(KEYCODE_0_PAD)

	PORT_START("KEY9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD)) PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(TAB_PAD)) PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Keypad 6  \u2192") PORT_CHAR(UCHAR_MAMEKEY(6_PAD)) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CODE(KEYCODE_6_PAD) // U+2192 = →
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD)) PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD)) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad Enter  +") PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD)) PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD)) PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD)) PORT_CODE(KEYCODE_DEL_PAD)
INPUT_PORTS_END

static INPUT_PORTS_START(ts2624)
	PORT_START("KEY0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Aids")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Escape") PORT_CHAR(0x1b) PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('1') PORT_CHAR('!') PORT_CODE(KEYCODE_1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(0x09) PORT_NAME("Tab  Back Tab") PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('q') PORT_CHAR('Q') PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Control") PORT_CHAR(UCHAR_SHIFT_2) PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift Caps") PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_CODE(KEYCODE_CAPSLOCK)

	PORT_START("KEY1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("User Keys")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Mode")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('2') PORT_CHAR('@') PORT_CODE(KEYCODE_2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('3') PORT_CHAR('#') PORT_CODE(KEYCODE_3)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('w') PORT_CHAR('W') PORT_CODE(KEYCODE_W)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('a') PORT_CHAR('A') PORT_CODE(KEYCODE_A)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('z') PORT_CHAR('Z') PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left Shift") PORT_CHAR(UCHAR_SHIFT_1) PORT_CODE(KEYCODE_LSHIFT)

	PORT_START("KEY2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F2)) PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('4') PORT_CHAR('$') PORT_CODE(KEYCODE_4)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('e') PORT_CHAR('E') PORT_CODE(KEYCODE_E)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('s') PORT_CHAR('S') PORT_CODE(KEYCODE_S)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('d') PORT_CHAR('D') PORT_CODE(KEYCODE_D)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('x') PORT_CHAR('X') PORT_CODE(KEYCODE_X)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Function") PORT_CHAR(UCHAR_MAMEKEY(LALT)) PORT_CODE(KEYCODE_LALT)

	PORT_START("KEY3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('5') PORT_CHAR('%') PORT_CODE(KEYCODE_5)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('r') PORT_CHAR('R') PORT_CODE(KEYCODE_R)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('t') PORT_CHAR('T') PORT_CODE(KEYCODE_T)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('f') PORT_CHAR('F') PORT_CODE(KEYCODE_F)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('g') PORT_CHAR('G') PORT_CODE(KEYCODE_G)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('v') PORT_CHAR('V') PORT_CODE(KEYCODE_V)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('c') PORT_CHAR('C') PORT_CODE(KEYCODE_C)

	PORT_START("KEY4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F4)) PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F5)) PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('6') PORT_CHAR('^') PORT_CODE(KEYCODE_6)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('7') PORT_CHAR('&') PORT_CODE(KEYCODE_7)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('y') PORT_CHAR('Y') PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('h') PORT_CHAR('H') PORT_CODE(KEYCODE_H)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('b') PORT_CHAR('B') PORT_CODE(KEYCODE_B)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(' ') PORT_CODE(KEYCODE_SPACE)

	PORT_START("KEY5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F6)) PORT_CODE(KEYCODE_F6)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F7)) PORT_CODE(KEYCODE_F7)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('8') PORT_CHAR('*') PORT_CODE(KEYCODE_8)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('i') PORT_CHAR('I') PORT_CODE(KEYCODE_I)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('u') PORT_CHAR('U') PORT_CODE(KEYCODE_U)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('j') PORT_CHAR('J') PORT_CODE(KEYCODE_J)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('m') PORT_CHAR('M') PORT_CODE(KEYCODE_M)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('n') PORT_CHAR('N') PORT_CODE(KEYCODE_N)

	PORT_START("KEY6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F8)) PORT_CODE(KEYCODE_F8)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('0') PORT_CHAR(')') PORT_CODE(KEYCODE_0)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('9') PORT_CHAR('(') PORT_CODE(KEYCODE_9)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('o') PORT_CHAR('O') PORT_CODE(KEYCODE_O)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('k') PORT_CHAR('K') PORT_CODE(KEYCODE_K)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('l') PORT_CHAR('L') PORT_CODE(KEYCODE_L)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('.') PORT_CHAR('>') PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(',') PORT_CHAR('<') PORT_CODE(KEYCODE_COMMA)

	PORT_START("KEY7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ins/Del Line")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ins/Del Char")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('-') PORT_CHAR('_') PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('[') PORT_CHAR('{') PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('p') PORT_CHAR('P') PORT_CODE(KEYCODE_P)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(';') PORT_CHAR(':') PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('/') PORT_CHAR('?') PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Back Space") PORT_CHAR(0x08) PORT_CODE(KEYCODE_RALT)

	PORT_START("KEY8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Line/Dsp Clear")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('=') PORT_CHAR('+') PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('`') PORT_CHAR('~') PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Delete") PORT_CHAR(0x7f) PORT_CODE(KEYCODE_DEL)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(']') PORT_CHAR('}') PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('\\') PORT_CHAR('|') PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('\'') PORT_CHAR('"') PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right Shift") PORT_CODE(KEYCODE_RSHIFT)

	PORT_START("KEY9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(HOME)) PORT_CODE(KEYCODE_HOME)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Back Space (Break)") PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD)) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Keypad 4  \u2190") PORT_CHAR(UCHAR_MAMEKEY(4_PAD)) PORT_CODE(KEYCODE_4_PAD) // U+2190 = ←
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CHAR(0x0d) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD)) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD)) PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD)) PORT_CODE(KEYCODE_0_PAD)

	PORT_START("KEYA")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"\u2190  Prev Page") PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_CODE(KEYCODE_LEFT) // U+2190 = ←
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"\u2191  Scroll Up") PORT_CHAR(UCHAR_MAMEKEY(UP)) PORT_CODE(KEYCODE_UP) // U+2191 = ↑
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Keypad 8  \u2191 (Brite)") PORT_CHAR(UCHAR_MAMEKEY(8_PAD)) PORT_CODE(KEYCODE_8_PAD) // U+2191 = ↑
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 5  Home") PORT_CHAR(UCHAR_MAMEKEY(5_PAD)) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Keypad 2  \u2193 (Dim)") PORT_CHAR(UCHAR_MAMEKEY(2_PAD)) PORT_CODE(KEYCODE_2_PAD) // U+2193 = ↓
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD)) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD)) PORT_CODE(KEYCODE_DEL_PAD)

	PORT_START("KEYB")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"\u2193  Scroll Down") PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_CODE(KEYCODE_DOWN) // U+2193 = ↓
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"\u2192  Next Page") PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CODE(KEYCODE_RIGHT) // U+2192 = →
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD)) PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD)) PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Keypad 6  \u2192") PORT_CHAR(UCHAR_MAMEKEY(6_PAD)) PORT_CODE(KEYCODE_6_PAD) // U+2192 = →
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad Back Tab") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad Forward Tab") PORT_CHAR(UCHAR_MAMEKEY(TAB_PAD)) PORT_CODE(KEYCODE_ENTER_PAD)
INPUT_PORTS_END

void falcots_state::ts1(machine_config &config)
{
	Z80(config, m_maincpu, 15.2064_MHz_XTAL / 4); // NEC D780C-1 (divider guessed)
	m_maincpu->set_addrmap(AS_PROGRAM, &falcots_state::ts1_mem_map);
	m_maincpu->set_addrmap(AS_IO, &falcots_state::ts1_io_map);
	m_maincpu->set_irq_acknowledge_callback("rstbuf", FUNC(rst_pos_buffer_device::inta_cb));

	RST_POS_BUFFER(config, m_rstbuf).int_callback().set_inputline(m_maincpu, 0);
	INPUT_MERGER_ALL_HIGH(config, m_lineint).output_handler().set(m_rstbuf, FUNC(rst_pos_buffer_device::rst4_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // 2x NEC D444C + battery?

	scn2651_device &pci(SCN2651(config, "pci", 15.2064_MHz_XTAL / 3)); // SCN2651N
	pci.txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	pci.dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));
	pci.rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	pci.txrdy_handler().set(m_rstbuf, FUNC(rst_pos_buffer_device::rst1_w));
	pci.rxrdy_handler().set(m_rstbuf, FUNC(rst_pos_buffer_device::rst2_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(15.2064_MHz_XTAL, 792, 0, 640, 320, 0, 300);
	screen.set_screen_update(m_crtc, FUNC(mc6845_device::screen_update));
	screen.set_video_attributes(VIDEO_UPDATE_SCANLINE);

	MC6845(config, m_crtc, 15.2064_MHz_XTAL / 8); // MC6845P
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(falcots_state::ts1_update_row));
	m_crtc->out_de_callback().set(FUNC(falcots_state::row_clock_w));
	m_crtc->out_vsync_callback().set(FUNC(falcots_state::vsync_w));
	m_crtc->out_vsync_callback().append(m_blinkcnt, FUNC(ripple_counter_device::clock_w)).invert();

	RIPPLE_COUNTER(config, m_blinkcnt).set_stages(5);

	// TODO: actually has two RS-232C ports (A & B)
	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "loopback"));
	rs232.rxd_handler().set("pci", FUNC(scn2651_device::rxd_w));
	rs232.cts_handler().set("pci", FUNC(scn2651_device::cts_w));
	rs232.dcd_handler().set("pci", FUNC(scn2651_device::dcd_w));
	rs232.dsr_handler().set("pci", FUNC(scn2651_device::dsr_w));
}

static const z80_daisy_config daisy_chain[] =
{
	{ "ctc" },
	{ "dart" },
	{ nullptr }
};

void falcots_state::ts2624(machine_config &config)
{
	Z80(config, m_maincpu, 14.7456_MHz_XTAL / 4); // Z8400AB1
	m_maincpu->set_addrmap(AS_PROGRAM, &falcots_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &falcots_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain);

	INPUT_MERGER_ALL_HIGH(config, m_lineint).output_handler().set_inputline(m_maincpu, INPUT_LINE_NMI);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // HM6116LP-4 + battery

	z80ctc_device &ctc(Z80CTC(config, "ctc", 14.7456_MHz_XTAL / 4)); // Z8430AB1
	ctc.set_clk<0>(14.7456_MHz_XTAL / 16);
	ctc.set_clk<1>(14.7456_MHz_XTAL / 16);
	ctc.zc_callback<0>().set("dart", FUNC(z80dart_device::rxca_w));
	ctc.zc_callback<0>().append("dart", FUNC(z80dart_device::txca_w));
	ctc.zc_callback<1>().set("dart", FUNC(z80dart_device::rxtxcb_w));
	ctc.zc_callback<2>().set(FUNC(falcots_state::bell_toggle_w));
	ctc.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	z80dart_device &dart(Z80DART(config, "dart", 14.7456_MHz_XTAL / 4)); // Z8470AB1
	dart.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	dart.out_txda_callback().set("rs232a", FUNC(rs232_port_device::write_txd));
	dart.out_dtra_callback().set("rs232a", FUNC(rs232_port_device::write_dtr));
	dart.out_rtsa_callback().set("rs232a", FUNC(rs232_port_device::write_rts));
	dart.out_txdb_callback().set("rs232b", FUNC(rs232_port_device::write_txd));
	dart.out_dtrb_callback().set("rs232b", FUNC(rs232_port_device::write_dtr));
	dart.out_rtsb_callback().set("rs232b", FUNC(rs232_port_device::write_rts));

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_bell).add_route(ALL_OUTPUTS, "mono", 0.05);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(14.7456_MHz_XTAL, 768, 0, 640, 320, 0, 286);
	//screen.set_raw(23.9616_MHz_XTAL, 1248, 0, 1056, 320, 0, 286);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));
	screen.set_video_attributes(VIDEO_UPDATE_SCANLINE);

	MC6845(config, m_crtc, 14.7456_MHz_XTAL / 16);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(16);
	m_crtc->set_update_row_callback(FUNC(falcots_state::update_row));
	m_crtc->out_de_callback().set(FUNC(falcots_state::row_clock_w));
	m_crtc->out_vsync_callback().set(FUNC(falcots_state::vsync_w));
	m_crtc->out_vsync_callback().append(m_blinkcnt, FUNC(ripple_counter_device::clock_w)).invert();

	RIPPLE_COUNTER(config, m_blinkcnt).set_stages(5); // 1/2 74LS393 + 1/2 74LS74

	rs232_port_device &rs232a(RS232_PORT(config, "rs232a", default_rs232_devices, nullptr));
	rs232a.rxd_handler().set("dart", FUNC(z80dart_device::rxa_w));
	rs232a.cts_handler().set("dart", FUNC(z80dart_device::ctsa_w));
	rs232a.dcd_handler().set("dart", FUNC(z80dart_device::dcda_w));

	rs232_port_device &rs232b(RS232_PORT(config, "rs232b", default_rs232_devices, nullptr));
	rs232b.rxd_handler().set("dart", FUNC(z80dart_device::rxb_w));
	rs232b.cts_handler().set("dart", FUNC(z80dart_device::ctsb_w));
	rs232b.dcd_handler().set("dart", FUNC(z80dart_device::dcdb_w));
}

ROM_START(ts1)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("v2_13_x.d9",  0x0000, 0x1000, CRC(420e1ecd) SHA1(748e3733858ba813b9d72dfe018ba4f918d8c0db)) // Chip Type: 2732
	ROM_LOAD("v2_13_0.d10", 0x1000, 0x1000, CRC(228e7321) SHA1(43e0d04c58ee7c71f5603222bf0aaaf7979d67a3)) // Chip Type: 2732

	ROM_REGION(0x2000, "chargen", ROMREGION_ERASEFF)
	ROM_LOAD("crom003.f4", 0x0000, 0x0800, CRC(557c8e0b) SHA1(b028f526bd92f957ee6242a7e0e6e0f16b0880a8)) // Chip Type: EA8316E517
ROM_END

ROM_START(ts2624)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("1.bin", 0x0000, 0x2000, CRC(14fb80aa) SHA1(93bf0d39f3e4bf092b6cd850f95ee6cbd322ad13))
	ROM_LOAD("2.bin", 0x2000, 0x2000, CRC(d4c74a06) SHA1(291357a296c45fccdbe8e395ea170d847a3a6f03))
	ROM_LOAD("3.bin", 0x4000, 0x2000, CRC(90d0d04b) SHA1(099d6741091b3abbe4187c8278e2c7ebe151531c))
	ROM_LOAD("4.bin", 0x6000, 0x2000, CRC(b0c59ec8) SHA1(099f6d6a7594e177bc668fd19fa19c3f0f4ab38e))

	ROM_REGION(0x2000, "chargen", 0)
	ROM_LOAD("chr.bin", 0x0000, 0x2000, CRC(38569fe2) SHA1(c666c596bb6326e4f41ccfd91154bcfd75f5c0a3))

	ROM_REGION(0x60, "proms", 0)
	ROM_LOAD("msel64b.9c", 0x00, 0x20, NO_DUMP) // 74S288 or equivalent
	ROM_LOAD("prom.13d",   0x20, 0x20, NO_DUMP) // 74S288 or equivalent
	ROM_LOAD("prom.12f",   0x20, 0x20, NO_DUMP) // 74S288 or equivalent
ROM_END

} // anonymous namespace


COMP(1980, ts1,    0, 0, ts1,    ts1,    falcots_state, empty_init, "Falco Data Products", "TS-1 (v2.13.0)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND)
COMP(1982, ts2624, 0, 0, ts2624, ts2624, falcots_state, empty_init, "Falco Data Products", "TS-2624", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS)
