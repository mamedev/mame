// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

    Compucolor II

    http://www.compucolor.org/index.html

*/

#include "emu.h"
#include "bus/compucolor/exp.h"
#include "bus/compucolor/floppy.h"
#include "bus/compucolor/soundware.h"
#include "bus/rs232/rs232.h"
#include "cpu/i8085/i8085.h"
#include "machine/ram.h"
#include "machine/ripple_counter.h"
#include "machine/tms5501.h"
#include "video/tms9927.h"
#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "utf8.h"

#include <algorithm>
#include <iterator>


namespace {

#define I8080_TAG   "ua2"
#define TMS5501_TAG "ud2"
#define CRT5027_TAG "uf9"
#define RS232_TAG   "rs232"

class compucolor2_state : public driver_device
{
public:
	compucolor2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, I8080_TAG),
		m_mioc(*this, TMS5501_TAG),
		m_vtac(*this, CRT5027_TAG),
		m_palette(*this, "palette"),
		m_blink(*this, "blink"),
		m_rs232(*this, RS232_TAG),
		m_ram(*this, RAM_TAG),
		m_floppy(*this, "cd%u", 0U),
		m_char_rom(*this, "chargen"),
		m_video_ram(*this, "videoram"),
		m_y(*this, "Y%u", 0),
		m_y128(*this, "Y128"),
		m_xo(0)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<tms5501_device> m_mioc;
	required_device<crt5027_device> m_vtac;
	required_device<palette_device> m_palette;
	required_device<ripple_counter_device> m_blink;
	required_device<rs232_port_device> m_rs232;
	required_device<ram_device> m_ram;
	required_device_array<compucolor_floppy_port_device, 2> m_floppy;
	required_memory_region m_char_rom;
	required_shared_ptr<uint8_t> m_video_ram;
	required_ioport_array<16> m_y;
	required_ioport m_y128;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	uint8_t xi_r();
	void xo_w(uint8_t data);
	void xmt_w(int state);
	template <unsigned N> void rxd_w(int state);
	void update_rxd();

	IRQ_CALLBACK_MEMBER( int_ack );

	uint8_t m_xo;
	uint8_t m_rxd[3];
	
	void compucolor2(machine_config &config) ATTR_COLD;
	void compucolor2a(machine_config &config) ATTR_COLD;
	void compucolor2_io(address_map &map) ATTR_COLD;
	void compucolor2_mem(address_map &map) ATTR_COLD;
};

void compucolor2_state::compucolor2_mem(address_map &map)
{
	map(0x0000, 0x3fff).rom().region(I8080_TAG, 0);
	map(0x6000, 0x6fff).mirror(0x1000).ram().share("videoram");
}

void compucolor2_state::compucolor2_io(address_map &map)
{
	// nothing drives the data bus on an unselected read, so it still holds the port number fetched with the IN instruction
	map(0x00, 0xff).lr8(NAME([] (offs_t offset) { return uint8_t(offset); }));
	map(0x00, 0x0f).mirror(0x10).m(m_mioc, FUNC(tms5501_device::io_map));
	map(0x60, 0x6f).mirror(0x10).w(m_vtac, FUNC(crt5027_device::write));
	map(0x80, 0x9f).mirror(0x60).rom().region("ua1", 0);
}

static INPUT_PORTS_START( compucolor2 )
	PORT_START("Y0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O', 'o')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("_ CRT") PORT_CHAR('_')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F15 PLOT ESC")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("BL/A7 OFF")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("BLINK ON")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N', 'n')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("^ USER") PORT_CHAR('^')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F14 CHAR RECT")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("A7 ON")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("BG ON")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("M TERM MODE") PORT_CODE(KEYCODE_M) PORT_CHAR('M', 'm')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("] BLCK") PORT_CHAR(']')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F13 POINT X")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RETURN ENTER") PORT_CODE(KEYCODE_ENTER) PORT_CHAR('\r')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("FG ON")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(UCHAR_MAMEKEY(EQUALS_PAD))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("L LOCL") PORT_CODE(KEYCODE_L) PORT_CHAR('L', 'l')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\\ 45 UP") PORT_CHAR('\\')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F12 POINT Y")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ERASE PAGE")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("K ROLL") PORT_CODE(KEYCODE_K) PORT_CHAR('K', 'k')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("[ VIS") PORT_CHAR('[')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F11 POINT INC")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ERASE LINE")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J', 'j')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Z 45 DW") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z', 'z')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F10 X BAR XO")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"Keypad ×") PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I', 'i')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Y TEST") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y', 'y')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F9 X BAR Y")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TAB) PORT_CHAR(UCHAR_MAMEKEY(TAB))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("H HALF") PORT_CODE(KEYCODE_H) PORT_CHAR('H', 'h')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("X XMIT") PORT_CODE(KEYCODE_X) PORT_CHAR('X', 'x')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F8 X BAR XM")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("HOME")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("G BELL") PORT_CODE(KEYCODE_G) PORT_CHAR('G', 'g')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("W BASIC") PORT_CODE(KEYCODE_W) PORT_CHAR('W', 'w')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F7 X BAR INC")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LIST")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F', 'f')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("V DEL LINE") PORT_CODE(KEYCODE_V) PORT_CHAR('V', 'v')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F6 Y BAR YO")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("POKE")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y10")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("E BSC RST") PORT_CODE(KEYCODE_E) PORT_CHAR('E', 'e')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("U INS LINE") PORT_CODE(KEYCODE_U) PORT_CHAR('U', 'u')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F5 Y BAR X")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("AUTO") PORT_CODE(KEYCODE_PGDN)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PRINT")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y11")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D DISK FCS") PORT_CODE(KEYCODE_D) PORT_CHAR('D', 'd')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("T TEXT EDIT") PORT_CODE(KEYCODE_T) PORT_CHAR('T', 't')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F4 Y BAR YM")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("DELETE CHAR")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LOAD")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y12")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("C CURSOR X-Y") PORT_CODE(KEYCODE_C) PORT_CHAR('C', 'c')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("S ASMB") PORT_CODE(KEYCODE_S) PORT_CHAR('S', 's')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F3 Y BAR INC")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("INSERT LINE")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SAVE")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y13")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("B PLOT") PORT_CODE(KEYCODE_B) PORT_CHAR('B', 'b')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("R BAUD RATE") PORT_CODE(KEYCODE_R) PORT_CHAR('R', 'r')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F2 VECT X1")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("DELETE LINE")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PLOT")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y14")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("A PROT BLND") PORT_CODE(KEYCODE_A) PORT_CHAR('A', 'a')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Q INS CHAR") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q', 'q')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F1 VECT Y1")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("INSERT CHAR")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PUT")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y15")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("@ NULL") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("P CPU OP SYS") PORT_CODE(KEYCODE_P) PORT_CHAR('P', 'p')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F0 VECT INC")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("BREAK ATTN")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("OUT")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y128")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CPU RESET")
	PORT_BIT( 0x0e, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("REPEAT")
	PORT_BIT( 0x80, 0x00, IPT_KEYBOARD ) PORT_NAME("CAPS LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
INPUT_PORTS_END

uint32_t compucolor2_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	rectangle cursor;
	bool const cursor_on = !BIT(m_blink->count(), 4) && m_vtac->cursor_bounds(cursor);

	for (int y = 0; y < 32*8; y++)
	{
		int const row = ((y / 8) + m_vtac->upscroll_offset()) & 0x1f;
		offs_t offset = row * 128;
		bool const cursor_line = cursor_on && (y == cursor.min_y || y == cursor.max_y);

		for (int sx = 0; sx < 64; sx++)
		{
			uint8_t code = m_video_ram[offset++];
			uint8_t attr = m_video_ram[offset++];

			offs_t char_offs = ((code & 0x7f) << 3) | (y & 0x07);
			if (BIT(code, 7)) char_offs = ((code & 0x7f) << 3) | (BIT(row, 0) << 2) | ((y >> 1) & 0x03);

			uint8_t data = m_char_rom->base()[char_offs];

			rgb_t fg = m_palette->pen_color(attr & 0x07);
			rgb_t const bg = m_palette->pen_color((attr >> 3) & 0x07);

			if (cursor_line && (sx * 6) == cursor.min_x)
			{
				fg = m_palette->pen_color(7);
				data = 0xff;
			}

			for (int x = 0; x < 6; x++)
			{
				bitmap.pix(y, (sx * 6) + x) = BIT(data, 7) ? fg : bg;

				data <<= 1;
			}
		}
	}

	return 0;
}

uint8_t compucolor2_state::xi_r()
{
	uint8_t data = 0xff;

	switch ((m_xo >> 4) & 0x03)
	{
	case 0:
		data &= m_y[m_xo & 0x0f]->read();

		if (BIT(m_xo, 7))
		{
			data = (m_y128->read() & 0xf0) | (data & 0x0f);
		}
		break;

	default:
		data = 0;
	}

	return data;
}

void compucolor2_state::xo_w(uint8_t data)
{
	/*

	    bit     description

	    0       Keyboard A0, Stepper Phase 0
	    1       Keyboard A1, Stepper Phase 1
	    2       Keyboard A2, Stepper Phase 2
	    3       Keyboard A3, Disk _R/W
	    4       00=Keyboard/Modem, 01=Internal Drive (CD0), 10=External Drive (CD1)
	    5       ^
	    6       LED
	    7       0=Normal Keyboard, 1=SHIFT/CONTROL/REPEAT/CAPS LOCK

	*/

	m_xo = data;

	switch ((m_xo >> 4) & 0x03)
	{
	case 0:
	case 3:
		m_floppy[0]->select_w(1);
		m_floppy[1]->select_w(1);
		break;

	case 1:
		m_floppy[1]->select_w(1);

		m_floppy[0]->rw_w(BIT(data, 3));
		m_floppy[0]->select_w(0);
		m_floppy[0]->stepper_w(data & 0x07);
		break;

	case 2:
		m_floppy[0]->select_w(1);

		m_floppy[1]->rw_w(BIT(data, 3));
		m_floppy[1]->select_w(0);
		m_floppy[1]->stepper_w(data & 0x07);
		break;
	}

	update_rxd();
}

template <unsigned N> void compucolor2_state::rxd_w(int state)
{
	m_rxd[N] = state;

	update_rxd();
}

void compucolor2_state::update_rxd()
{
	uint8_t const sel = (m_xo >> 4) & 0x03;

	m_mioc->rcv_w((sel < 3) ? m_rxd[sel] : 1);
}

void compucolor2_state::xmt_w(int state)
{
	switch ((m_xo >> 4) & 0x03)
	{
	case 0:
		m_rs232->write_txd(state);
		break;

	case 1:
		m_floppy[0]->write_txd(state);
		break;

	case 2:
		m_floppy[1]->write_txd(state);
		break;
	}
}

IRQ_CALLBACK_MEMBER( compucolor2_state::int_ack )
{
	return m_mioc->get_vector();
}

void compucolor2_state::machine_start()
{
	m_maincpu->space(AS_PROGRAM).install_ram(0x8000, 0x8000 + m_ram->mask(), m_ram->pointer());

	std::fill(std::begin(m_rxd), std::end(m_rxd), 1);

	// state saving
	save_item(NAME(m_xo));
	save_item(NAME(m_rxd));
}

void compucolor2_state::machine_reset()
{
	m_rs232->write_rts(1);
	m_rs232->write_dtr(1);
}

void compucolor_rs232_devices(device_slot_interface &device)
{
	default_rs232_devices(device);
	device.option_add("soundware", COMPUCOLOR_SOUNDWARE);
}

void compucolor2_state::compucolor2(machine_config &config)
{
	// basic machine hardware
	I8080(config, m_maincpu, XTAL(17'971'200)/9);
	m_maincpu->set_addrmap(AS_PROGRAM, &compucolor2_state::compucolor2_mem);
	m_maincpu->set_addrmap(AS_IO, &compucolor2_state::compucolor2_io);
	m_maincpu->set_irq_acknowledge_callback(FUNC(compucolor2_state::int_ack));

	// video hardware
	screen_device &screen(SCREEN(config, "screen"));
	screen.set_raw(XTAL(17'971'200)/2, 93*6, 0, 64*6, 268, 0, 256);
	screen.set_screen_update(FUNC(compucolor2_state::screen_update));

	PALETTE(config, m_palette, palette_device::RGB_3BIT);

	CRT5027(config, m_vtac, XTAL(17'971'200)/2/6);
	m_vtac->set_char_width(6);
	m_vtac->vsyn_callback().set("blink", FUNC(ripple_counter_device::clock_w));
	m_vtac->set_screen("screen");

	ripple_counter_device &blink(RIPPLE_COUNTER(config, "blink")); // 74LS393 at UG10
	blink.set_stages(8);
	blink.count_out_cb().set(m_mioc, FUNC(tms5501_device::sens_w)).bit(4);

	// devices
	TMS5501(config, m_mioc, XTAL(17'971'200)/9);
	m_mioc->int_callback().set_inputline(I8080_TAG, I8085_INTR_LINE);
	m_mioc->xmt_callback().set(FUNC(compucolor2_state::xmt_w));
	m_mioc->xi_callback().set(FUNC(compucolor2_state::xi_r));
	m_mioc->xo_callback().set(FUNC(compucolor2_state::xo_w));

	RS232_PORT(config, m_rs232, compucolor_rs232_devices, nullptr);
	m_rs232->rxd_handler().set(FUNC(compucolor2_state::rxd_w<0>));

	COMPUCOLOR_FLOPPY_PORT(config, m_floppy[0], compucolor_floppy_port_devices, "floppy_4ph");
	m_floppy[0]->rxd_handler().set(FUNC(compucolor2_state::rxd_w<1>));

	COMPUCOLOR_FLOPPY_PORT(config, m_floppy[1], compucolor_floppy_port_devices, nullptr);
	m_floppy[1]->rxd_handler().set(FUNC(compucolor2_state::rxd_w<2>));

	COMPUCOLOR_EXPANSION_SLOT(config, "exp", XTAL(17'971'200)/9, compucolor_expansion_cards, nullptr).set_io_space(m_maincpu, AS_IO);

	// internal ram
	RAM(config, RAM_TAG).set_default_size("32K").set_extra_options("4K,8K,16K").set_default_value(0x00);

	// software list
	SOFTWARE_LIST(config, "flop_list").set_original("compclr2_flop");
}

void compucolor2_state::compucolor2a(machine_config &config)
{
	compucolor2(config);

	m_floppy[0]->set_default_option("floppy");
}

ROM_START( compclr2 )
	ROM_REGION( 0x4000, I8080_TAG, 0 )
	ROM_LOAD( "v879.rom", 0x0000, 0x4000, BAD_DUMP CRC(4de8e652) SHA1(e5c55da3ac893b8a5a99c8795af3ca72b1645f3f) )

	ROM_REGION( 0x800, "chargen", 0 )
	ROM_LOAD( "chargen.uf6", 0x000, 0x400, BAD_DUMP CRC(7eef135a) SHA1(be488ef32f54c6e5f551fb84ab12b881aef72dd9) )
	ROM_LOAD( "chargen.uf7", 0x400, 0x400, BAD_DUMP CRC(2bee7cf6) SHA1(808e0fc6f2332b4297de190eafcf84668703e2f4) )

	ROM_REGION( 0x20, "ua1", 0 )
	ROM_LOAD( "82s123.ua1", 0x00, 0x20, BAD_DUMP CRC(27ae54bc) SHA1(ccb056fbc1ec2132f2602217af64d77237494afb) ) // I/O PROM

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "82s129.ue2", 0x00, 0x20, NO_DUMP ) // Address Decoder/Timer for RAM
	ROM_LOAD( "82s129.uf3", 0x00, 0x20, NO_DUMP ) // System Decoder
	ROM_LOAD( "82s123.ub3", 0x00, 0x20, NO_DUMP ) // ROM/PROM Decoder
	ROM_LOAD( "82s129.uf8", 0x00, 0x20, NO_DUMP ) // CPU & Horizontal Decoder
	ROM_LOAD( "82s129.ug9", 0x00, 0x20, NO_DUMP ) // Scan Decoder
	ROM_LOAD( "82s129.ug5", 0x00, 0x20, NO_DUMP ) // Color PROM
ROM_END

ROM_START( compclr2a )
	ROM_REGION( 0x4000, I8080_TAG, 0 )
	ROM_LOAD( "v678.rom", 0x0000, 0x4000, BAD_DUMP CRC(5e559469) SHA1(fe308774aae1294c852fe24017e58d892d880cd3) )

	ROM_REGION( 0x800, "chargen", 0 )
	ROM_LOAD( "chargen.uf6", 0x000, 0x400, BAD_DUMP CRC(7eef135a) SHA1(be488ef32f54c6e5f551fb84ab12b881aef72dd9) )
	ROM_LOAD( "chargen.uf7", 0x400, 0x400, BAD_DUMP CRC(2bee7cf6) SHA1(808e0fc6f2332b4297de190eafcf84668703e2f4) )

	ROM_REGION( 0x20, "ua1", 0 )
	ROM_LOAD( "82s123.ua1", 0x00, 0x20, BAD_DUMP CRC(27ae54bc) SHA1(ccb056fbc1ec2132f2602217af64d77237494afb) ) // I/O PROM

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "82s129.ue2", 0x00, 0x20, NO_DUMP ) // Address Decoder/Timer for RAM
	ROM_LOAD( "82s129.uf3", 0x00, 0x20, NO_DUMP ) // System Decoder
	ROM_LOAD( "82s123.ub3", 0x00, 0x20, NO_DUMP ) // ROM/PROM Decoder
	ROM_LOAD( "82s129.uf8", 0x00, 0x20, NO_DUMP ) // CPU & Horizontal Decoder
	ROM_LOAD( "82s129.ug9", 0x00, 0x20, NO_DUMP ) // Scan Decoder
	ROM_LOAD( "82s129.ug5", 0x00, 0x20, NO_DUMP ) // Color PROM
ROM_END

} // anonymous namespace


COMP( 1979, compclr2,  0,        0, compucolor2,  compucolor2, compucolor2_state, empty_init, "Intelligent Systems Corporation", "Compucolor II (v8.79)", MACHINE_NO_SOUND_HW )
COMP( 1977, compclr2a, compclr2, 0, compucolor2a, compucolor2, compucolor2_state, empty_init, "Intelligent Systems Corporation", "Compucolor II (v6.78)", MACHINE_NO_SOUND_HW )
