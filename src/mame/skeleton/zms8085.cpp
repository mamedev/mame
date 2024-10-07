// license:BSD-3-Clause
// copyright-holders:AJR
/***********************************************************************************************************************************

Skeleton driver for Zentec's 8085-based Zephyr/ZMS-series terminals.

This represents the second generation of display terminals by Zentec Corporation. The
first, which included the ZMS-50, ZMS-70 & ZMS-90, had an 8080 CPU and different video
timings.

U.S. Patent No. 4,203,107 looks like the best available description of the Zephyr video
hardware, since no schematics or manuals have been found.

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "bus/rs232/rs232.h"
#include "machine/ay31015.h"
#include "machine/clock.h"
#include "screen.h"


namespace {

class zms8085_state : public driver_device
{
public:
	zms8085_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_uart(*this, "uart")
		, m_uart_clock(*this, "uart_clock")
		, m_rs232(*this, "rs232")
		, m_screen(*this, "screen")
		, m_mainram(*this, "mainram")
		, m_chargen(*this, "chargen")
		, m_keys(*this, "KEY%X", 0U)
	{ }

	void zephyr(machine_config &config);

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	u8 special_r();
	u8 uart_status_r();
	void key_l_w(u8 data);
	void key_h_w(u8 data);
	u8 key_r();

	virtual void machine_start() override ATTR_COLD;

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<ay51013_device> m_uart;
	required_device<clock_device> m_uart_clock;
	required_device<rs232_port_device> m_rs232;
	required_device<screen_device> m_screen;
	required_shared_ptr<u8> m_mainram;
	required_region_ptr<u8> m_chargen;
	required_ioport_array<14> m_keys;

	u16 m_key_row = 0U;
};


u32 zms8085_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	u8 cursor_y = m_mainram[0x000];
	u8 cursor_x = m_mainram[0x001];
	u8 scroll = m_mainram[0x005];

	u16 pos = 0x030 + (scroll & 0x1f) * 0x050;
	u8 row = scroll & 0x1f;

	for (unsigned y = 0; y < 250; y++)
	{
		for (unsigned ch = 0; ch < 80; ch++)
		{
			unsigned x = ch * 10;
			u8 code = m_mainram[(ch + pos) & 0xfff];
			u16 data = (m_chargen[(code & 0x7f) * 16 + (y + 9) % 10] << 2) | 0x003;
			if (ch == cursor_x && row == cursor_y)
				data ^= 0x3ff;
			for (int bit = 10; bit > 0; bit--, data <<= 1)
				bitmap.pix(y, x++) = BIT(data, 9) ? rgb_t::black() : rgb_t::white();
		}

		if ((y % 10) == 9)
		{
			row++;
			if (y == 239)
			{
				pos = 0x030;
				cursor_y = 0xff;
			}
			else
			{
				pos += 80;
				if (pos >= 0x800)
				{
					pos = 0x080;
					row -= 24;
				}
			}
		}
	}
	return 0;
}


u8 zms8085_state::special_r()
{
	return 0x40;
}

u8 zms8085_state::uart_status_r()
{
	return m_uart->tbmt_r() | (m_uart->dav_r() << 1);
}

void zms8085_state::key_l_w(u8 data)
{
	m_key_row = (m_key_row & 0x3f00) | data;
}

void zms8085_state::key_h_w(u8 data)
{
	m_key_row = (m_key_row & 0x00ff) | ((data & 0x3f) << 8);
}

u8 zms8085_state::key_r()
{
	if (!machine().side_effects_disabled())
		m_maincpu->set_input_line(I8085_RST55_LINE, CLEAR_LINE);

	u8 result = 0xff;
	for (int n = 0; n < 14; n++)
		if (BIT(m_key_row, n))
			result &= m_keys[n]->read();

	return result;
}


void zms8085_state::mem_map(address_map &map)
{
	map(0x0000, 0x0fff).rom().region("maincpu", 0).nopw();
	map(0x1000, 0x1fff).ram().share("mainram");
}

void zms8085_state::io_map(address_map &map)
{
	map(0x61, 0x61).r(FUNC(zms8085_state::special_r)).w(m_uart, FUNC(ay51013_device::transmit));
	map(0x62, 0x62).r(m_uart, FUNC(ay51013_device::receive));
	map(0x63, 0x63).r(FUNC(zms8085_state::uart_status_r));
	map(0x64, 0x64).w(FUNC(zms8085_state::key_l_w));
	map(0x65, 0x65).w(FUNC(zms8085_state::key_h_w));
	map(0x66, 0x66).r(FUNC(zms8085_state::key_r));
	map(0x67, 0x67).portr("MODIFIERS");
	map(0x68, 0x68).nopw();
}


static INPUT_PORTS_START(zephyr)
	PORT_START("KEY0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('g') PORT_CHAR('G') PORT_CODE(KEYCODE_G)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('t') PORT_CHAR('T') PORT_CODE(KEYCODE_T)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('6') PORT_CHAR('^') PORT_CODE(KEYCODE_6)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('=') PORT_CHAR('+') PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('`') PORT_CHAR('~') PORT_CODE(KEYCODE_TILDE) // to right of right shift
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('m') PORT_CHAR('M') PORT_CODE(KEYCODE_M)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("KEY1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('j') PORT_CHAR('J') PORT_CODE(KEYCODE_J)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('u') PORT_CHAR('U') PORT_CODE(KEYCODE_U)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('8') PORT_CHAR('*') PORT_CODE(KEYCODE_8)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('{') PORT_CHAR('}') PORT_CODE(KEYCODE_BACKSLASH) // to right of Back Space
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD)) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('.') PORT_CHAR('>') PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD)) PORT_CODE(KEYCODE_7_PAD)

	PORT_START("KEY2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('l') PORT_CHAR('L') PORT_CODE(KEYCODE_L)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('o') PORT_CHAR('O') PORT_CODE(KEYCODE_O)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('0') PORT_CHAR(')') PORT_CODE(KEYCODE_0)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(HOME)) PORT_CODE(KEYCODE_HOME)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD)) PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('[') PORT_CHAR(']') PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD)) PORT_CODE(KEYCODE_9_PAD)

	PORT_START("KEY3")
	PORT_BIT(0x03, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('e') PORT_CHAR('E') PORT_CODE(KEYCODE_E)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('4') PORT_CHAR('$') PORT_CODE(KEYCODE_4)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(COMMA_PAD)) PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('b') PORT_CHAR('B') PORT_CODE(KEYCODE_B)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_CODE(KEYCODE_DOWN)

	PORT_START("KEY4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('f') PORT_CHAR('F') PORT_CODE(KEYCODE_F)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('r') PORT_CHAR('R') PORT_CODE(KEYCODE_R)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('5') PORT_CHAR('%') PORT_CODE(KEYCODE_5)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD)) PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('n') PORT_CHAR('N') PORT_CODE(KEYCODE_N)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("KEY5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(';') PORT_CHAR(':') PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('p') PORT_CHAR('P') PORT_CODE(KEYCODE_P)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('-') PORT_CHAR('_') PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD)) PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('\'') PORT_CHAR('"') PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('\\') PORT_CHAR('|') PORT_CODE(KEYCODE_CLOSEBRACE)

	PORT_START("KEY6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('k') PORT_CHAR('K') PORT_CODE(KEYCODE_K)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('i') PORT_CHAR('I') PORT_CODE(KEYCODE_I)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('9') PORT_CHAR('(') PORT_CODE(KEYCODE_9)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD)) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('/') PORT_CHAR('?') PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD)) PORT_CODE(KEYCODE_8_PAD)

	PORT_START("KEY7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('h') PORT_CHAR('H') PORT_CODE(KEYCODE_H)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('y') PORT_CHAR('Y') PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('7') PORT_CHAR('&') PORT_CODE(KEYCODE_7)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Back Space") PORT_CHAR(0x08) PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD)) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(',') PORT_CHAR('<') PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("KEY8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('d') PORT_CHAR('D') PORT_CODE(KEYCODE_D)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('w') PORT_CHAR('W') PORT_CODE(KEYCODE_W)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('3') PORT_CHAR('#') PORT_CODE(KEYCODE_3)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(UP)) PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD)) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('v') PORT_CHAR('V') PORT_CODE(KEYCODE_V)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("KEY9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('s') PORT_CHAR('S') PORT_CODE(KEYCODE_S)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('q') PORT_CHAR('Q') PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('2') PORT_CHAR('@') PORT_CODE(KEYCODE_2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD)) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('c') PORT_CHAR('C') PORT_CODE(KEYCODE_C)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Line Feed") // to right of backslash

	PORT_START("KEYA")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('a') PORT_CHAR('A') PORT_CODE(KEYCODE_A)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(0x09) PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('1') PORT_CHAR('!') PORT_CODE(KEYCODE_1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F2)) PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD)) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('x') PORT_CHAR('X') PORT_CODE(KEYCODE_X)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CHAR(0x0d) PORT_CODE(KEYCODE_ENTER)

	PORT_START("KEYB")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc  Del") PORT_CHAR(0x1b) PORT_CHAR(0x7f) PORT_CODE(KEYCODE_LALT) // to left of Caps Lock
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("New Line/Page") // to right of Return?
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('z') PORT_CHAR('Z') PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(' ') PORT_CODE(KEYCODE_SPACE)

	PORT_START("KEYC")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD)) PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F14)) PORT_CODE(KEYCODE_F14)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F13)) PORT_CODE(KEYCODE_F13)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F12)) PORT_CODE(KEYCODE_F12)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F11)) PORT_CODE(KEYCODE_F11)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F15)) PORT_CODE(KEYCODE_F15)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F16)) PORT_CODE(KEYCODE_F16)

	PORT_START("KEYD")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F10)) PORT_CODE(KEYCODE_F10)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F6)) PORT_CODE(KEYCODE_F6)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F5)) PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F4)) PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F9)) PORT_CODE(KEYCODE_F9)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F7)) PORT_CODE(KEYCODE_F7)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F8)) PORT_CODE(KEYCODE_F8)

	PORT_START("MODIFIERS")
	PORT_BIT(0x1f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CHAR(UCHAR_SHIFT_2) PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CHAR(UCHAR_SHIFT_1) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
INPUT_PORTS_END


void zms8085_state::machine_start()
{
	m_uart->write_swe(0);

	// most of these settings should be configurable through switches
	m_uart->write_nb1(1);
	m_uart->write_nb2(1);
	m_uart->write_np(1);
	m_uart->write_eps(1);
	m_uart->write_tsb(0);
	m_uart->write_cs(1);

	m_key_row = 0;
	save_item(NAME(m_key_row));
}

void zms8085_state::zephyr(machine_config &config)
{
	I8085A(config, m_maincpu, 15.582_MHz_XTAL / 2); // divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &zms8085_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &zms8085_state::io_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(15.582_MHz_XTAL, 980, 0, 800, 265, 0, 250);
	screen.set_screen_update(FUNC(zms8085_state::screen_update));
	screen.screen_vblank().set_inputline(m_maincpu, I8085_RST55_LINE, ASSERT_LINE);

	AY51013(config, m_uart); // SMC COM2017
	m_uart->read_si_callback().set("rs232", FUNC(rs232_port_device::rxd_r));
	m_uart->write_so_callback().set("rs232", FUNC(rs232_port_device::write_txd));
	m_uart->write_dav_callback().set_inputline("maincpu", I8085_RST65_LINE);
	m_uart->set_auto_rdav(true);

	CLOCK(config, m_uart_clock, 153600); // should actually be configurable somehow
	m_uart_clock->signal_handler().set(m_uart, FUNC(ay51013_device::write_tcp));
	m_uart_clock->signal_handler().append(m_uart, FUNC(ay51013_device::write_rcp));

	RS232_PORT(config, "rs232", default_rs232_devices, nullptr);
}

/**************************************************************************************************************

Zentec Zephyr (Model 00-441-01).
Chips: COM2017, i8085A, 2x unreadable (40-pin AMI DIP), Beeper
Crystal: 15.582000

***************************************************************************************************************/

ROM_START( zephyr )
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "main", "Main program" )
	ROMX_LOAD( "23-067-03b.bin",  0x0000, 0x0800, CRC(29cfa003) SHA1(9de7a8402173a2c448e54ee433ba3050db7b70bb), ROM_BIOS(0) )
	ROMX_LOAD( "23-067-004b.bin", 0x0800, 0x0800, CRC(37741104) SHA1(52b9998e0a8d4949e0dc7c3349b3681e13345061), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "test", "Test program" )
	ROMX_LOAD( "23-006-32c.bin",  0x0000, 0x0800, CRC(0a3a5447) SHA1(a8c25730a1d7e5b9c86e0d504afc923e931f9025), ROM_BIOS(1) )

	ROM_REGION(0x0800, "chargen", 0)
	ROM_LOAD( "23-066-02a.bin",  0x0000, 0x0800, CRC(d5650b6c) SHA1(e6333e59018d9904f12abb270db4ba28aeff1995) )
ROM_END

} // anonymous namespace


COMP( 1979, zephyr, 0, 0, zephyr, zephyr, zms8085_state, empty_init, "Zentec", "Zephyr (00-441-01)", MACHINE_IS_SKELETON )
