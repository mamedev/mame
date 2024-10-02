// license:BSD-3-Clause
// copyright-holders:AJR
/***********************************************************************************************************************************

Skeleton driver for Ann Arbor Ambassador terminal.

************************************************************************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/input_merger.h"
#include "machine/scn_pci.h"
#include "machine/nvram.h"

#include "screen.h"


namespace {

class aaa_state : public driver_device
{
public:
	aaa_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_usart(*this, "usart%u", 0U)
		, m_screen(*this, "screen")
		, m_key_row(*this, "KSL%u", 0U)
		, m_font(*this, "font")
		, m_display_ram(*this, "display")
		, m_linecount(*this, "linecount")
	{ }

	void aaa(machine_config &config);

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void display_ram_w(offs_t offset, u8 data);
	template<int N> u8 usart_r(offs_t offset);
	template<int N> void usart_w(offs_t offset, u8 data);
	u8 keyboard_r(offs_t offset);

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device_array<scn2651_device, 2> m_usart;
	required_device<screen_device> m_screen;
	optional_ioport_array<16> m_key_row;
	required_region_ptr<u8> m_font;
	required_shared_ptr<u8> m_display_ram;
	required_shared_ptr<u8> m_linecount;
};

u32 aaa_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void aaa_state::display_ram_w(offs_t offset, u8 data)
{
	// 6 bits wide (2 words per character)
	m_display_ram[offset] = data | 0xc0;
}

template<int N>
u8 aaa_state::usart_r(offs_t offset)
{
	return m_usart[N]->read(offset >> 1);
}

template<int N>
void aaa_state::usart_w(offs_t offset, u8 data)
{
	m_usart[N]->write(offset >> 1, data);
}

u8 aaa_state::keyboard_r(offs_t offset)
{
	// TODO: key click
	return m_key_row[offset & 15].read_safe(0xff);
}

void aaa_state::mem_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("loprog", 0);
	map(0x2000, 0x27ff).mirror(0x800).ram();
	map(0x3f00, 0x3f03).writeonly().share("linecount"); // LS670 at 7B on main board
	map(0x4000, 0x43ff).ram().share("nvram");
	map(0x8000, 0x9fff).rom().region("hiprog", 0);
	map(0xc000, 0xffff).ram().w(FUNC(aaa_state::display_ram_w)).share("display");
}

void aaa_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).select(6).r(FUNC(aaa_state::usart_r<0>));
	map(0x01, 0x01).select(6).w(FUNC(aaa_state::usart_w<0>));
	map(0x40, 0x40).select(6).r(FUNC(aaa_state::usart_r<1>));
	map(0x41, 0x41).select(6).w(FUNC(aaa_state::usart_w<1>));
	map(0x80, 0x87).w("ctrllatch", FUNC(ls259_device::write_d7));
	map(0xc0, 0xff).r(FUNC(aaa_state::keyboard_r));
}

static INPUT_PORTS_START(aaa)
	// KR0, KR1, KR2, KR3, KR4, KR5, KR6, KR7 = connector pins W, X, Y, Z, AA, BB, CC, DD

	PORT_START("KSL2") // connector pin T
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('[') PORT_CHAR('{') PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('a') PORT_CHAR('A') PORT_CODE(KEYCODE_A)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('b') PORT_CHAR('B') PORT_CODE(KEYCODE_B)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('c') PORT_CHAR('C') PORT_CODE(KEYCODE_C)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('d') PORT_CHAR('D') PORT_CODE(KEYCODE_D)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('e') PORT_CHAR('E') PORT_CODE(KEYCODE_E)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('f') PORT_CHAR('F') PORT_CODE(KEYCODE_F)

	PORT_START("KSL3") // connector pin S
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('g') PORT_CHAR('G') PORT_CODE(KEYCODE_G)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('h') PORT_CHAR('H') PORT_CODE(KEYCODE_H)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('i') PORT_CHAR('I') PORT_CODE(KEYCODE_I)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('j') PORT_CHAR('J') PORT_CODE(KEYCODE_J)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('k') PORT_CHAR('K') PORT_CODE(KEYCODE_K)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('l') PORT_CHAR('L') PORT_CODE(KEYCODE_L)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('m') PORT_CHAR('M') PORT_CODE(KEYCODE_M)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('n') PORT_CHAR('N') PORT_CODE(KEYCODE_N)

	PORT_START("KSL4") // connector pin N
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('o') PORT_CHAR('O') PORT_CODE(KEYCODE_O)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('p') PORT_CHAR('P') PORT_CODE(KEYCODE_P)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('q') PORT_CHAR('Q') PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('r') PORT_CHAR('R') PORT_CODE(KEYCODE_R)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('s') PORT_CHAR('S') PORT_CODE(KEYCODE_S)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('t') PORT_CHAR('T') PORT_CODE(KEYCODE_T)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('u') PORT_CHAR('U') PORT_CODE(KEYCODE_U)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('v') PORT_CHAR('V') PORT_CODE(KEYCODE_V)

	PORT_START("KSL5") // connector pin M
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('w') PORT_CHAR('W') PORT_CODE(KEYCODE_W)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('x') PORT_CHAR('X') PORT_CODE(KEYCODE_X)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('y') PORT_CHAR('Y') PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('z') PORT_CHAR('Z') PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('`') PORT_CHAR('~') PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('\'') PORT_CHAR('"') PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('\\') PORT_CHAR('|') PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Pause") PORT_CODE(KEYCODE_LALT)

	PORT_START("KSL6") // connector pin L
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Move Up") PORT_CHAR(UCHAR_MAMEKEY(PGUP)) PORT_CODE(KEYCODE_PGUP)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('1') PORT_CHAR('!') PORT_CODE(KEYCODE_1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('2') PORT_CHAR('@') PORT_CODE(KEYCODE_2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('3') PORT_CHAR('#') PORT_CODE(KEYCODE_3)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('4') PORT_CHAR('$') PORT_CODE(KEYCODE_4)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('5') PORT_CHAR('%') PORT_CODE(KEYCODE_5)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('6') PORT_CHAR('^') PORT_CODE(KEYCODE_6)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('7') PORT_CHAR('&') PORT_CODE(KEYCODE_7)

	PORT_START("KSL7") // connector pin K
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('8') PORT_CHAR('*') PORT_CODE(KEYCODE_8)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('9') PORT_CHAR('(') PORT_CODE(KEYCODE_9)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('-') PORT_CHAR('_') PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(';') PORT_CHAR(':') PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(',') PORT_CHAR('<') PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('=') PORT_CHAR('+') PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('.') PORT_CHAR('>') PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('/') PORT_CHAR('?') PORT_CODE(KEYCODE_SLASH)

	PORT_START("KSL8") // connector pin J
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD)) PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD)) PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD)) PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Move Down") PORT_CHAR(UCHAR_MAMEKEY(PGDN)) PORT_CODE(KEYCODE_PGDN)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('0') PORT_CHAR(')') PORT_CODE(KEYCODE_0)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 1  SSA") PORT_CHAR(UCHAR_MAMEKEY(1_PAD)) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD)) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 3  ESA") PORT_CHAR(UCHAR_MAMEKEY(3_PAD)) PORT_CODE(KEYCODE_3_PAD)

	PORT_START("KSL9") // connector pin H
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD)) PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD)) PORT_CHAR(UCHAR_MAMEKEY(HOME)) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD)) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 7  T-Clr") PORT_CHAR(UCHAR_MAMEKEY(7_PAD)) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD)) PORT_CHAR(UCHAR_MAMEKEY(UP)) PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 9  T-Set") PORT_CHAR(UCHAR_MAMEKEY(9_PAD)) PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Reset")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Setup")

	PORT_START("KSL10") // connector pin F
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PF1") PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PF2") PORT_CHAR(UCHAR_MAMEKEY(F2)) PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PF3") PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Back Space") PORT_CHAR(0x08) PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PF4") PORT_CHAR(UCHAR_MAMEKEY(F4)) PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Break")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PF5") PORT_CHAR(UCHAR_MAMEKEY(F5)) PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PF6") PORT_CHAR(UCHAR_MAMEKEY(F6)) PORT_CODE(KEYCODE_F6)

	PORT_START("KSL11") // connector pin E
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Del")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PF7") PORT_CHAR(UCHAR_MAMEKEY(F7)) PORT_CODE(KEYCODE_F7)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PF8") PORT_CHAR(UCHAR_MAMEKEY(F8)) PORT_CODE(KEYCODE_F8)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PF9") PORT_CHAR(UCHAR_MAMEKEY(F9)) PORT_CODE(KEYCODE_F9)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PF10") PORT_CHAR(UCHAR_MAMEKEY(F10)) PORT_CODE(KEYCODE_F10)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PF11") PORT_CHAR(UCHAR_MAMEKEY(F11)) PORT_CODE(KEYCODE_F11)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PF12") PORT_CHAR(UCHAR_MAMEKEY(F12)) PORT_CODE(KEYCODE_F12)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Erase")

	PORT_START("KSL12") // connector pin D
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Edit")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Line Feed") PORT_CHAR(0x0a) PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Delete") PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(']') PORT_CHAR('}') PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Insert") PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Print")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CHAR(0x0d) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Send")

	PORT_START("KSL13") // connector pin C
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KSL14") // connector pin B
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KSL15") // connector pin A
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space Bar") PORT_CHAR(' ') PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Tab \u2192  Tab \u2190") PORT_CHAR(0x09) PORT_CODE(KEYCODE_TAB) // Tab →  Tab ←
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CHAR(UCHAR_MAMEKEY(LCONTROL)) PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CHAR(0x1b) PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left Shift") PORT_CHAR(UCHAR_MAMEKEY(LSHIFT)) PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right Shift") PORT_CODE(KEYCODE_RSHIFT) // possibly the other way around
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

void aaa_state::aaa(machine_config &config)
{
	Z80(config, m_maincpu, 18.414_MHz_XTAL / 5);
	m_maincpu->set_addrmap(AS_PROGRAM, &aaa_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &aaa_state::io_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // NEC D444C x2 + 3V lithium battery

	LS259(config, "ctrllatch"); // 1B on main board

	input_merger_device &usartint(INPUT_MERGER_ANY_HIGH(config, "usartint")); // open collector
	usartint.output_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	SCN2651(config, m_usart[0], 5.0688_MHz_XTAL); // 16F on I/O board
	m_usart[0]->txrdy_handler().set("usartint", FUNC(input_merger_device::in_w<0>));
	m_usart[0]->rxrdy_handler().set("usartint", FUNC(input_merger_device::in_w<1>));
	m_usart[0]->txemt_dschg_handler().set("usartint", FUNC(input_merger_device::in_w<2>));
	m_usart[0]->txd_handler().set("printer", FUNC(rs232_port_device::write_txd));
	m_usart[0]->rts_handler().set("printer", FUNC(rs232_port_device::write_rts));
	m_usart[0]->dtr_handler().set("printer", FUNC(rs232_port_device::write_dtr));

	SCN2651(config, m_usart[1], 5.0688_MHz_XTAL); // 18F on I/O board
	m_usart[1]->txrdy_handler().set("usartint", FUNC(input_merger_device::in_w<3>));
	m_usart[1]->rxrdy_handler().set("usartint", FUNC(input_merger_device::in_w<4>));
	m_usart[1]->txemt_dschg_handler().set("usartint", FUNC(input_merger_device::in_w<5>));
	m_usart[1]->txd_handler().set("computer", FUNC(rs232_port_device::write_txd));
	m_usart[1]->rts_handler().set("computer", FUNC(rs232_port_device::write_rts));
	m_usart[1]->dtr_handler().set("computer", FUNC(rs232_port_device::write_dtr));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(18.414_MHz_XTAL, 990, 0, 800, 310, 0, 240);
	m_screen->set_screen_update(FUNC(aaa_state::screen_update));

	rs232_port_device &computer(RS232_PORT(config, "computer", default_rs232_devices, nullptr));
	computer.rxd_handler().set(m_usart[1], FUNC(scn2651_device::rxd_w));
	computer.cts_handler().set(m_usart[1], FUNC(scn2651_device::cts_w));
	computer.dsr_handler().set(m_usart[1], FUNC(scn2651_device::dsr_w));

	rs232_port_device &printer(RS232_PORT(config, "printer", default_rs232_devices, nullptr));
	printer.rxd_handler().set(m_usart[0], FUNC(scn2651_device::rxd_w));
	printer.cts_handler().set(m_usart[0], FUNC(scn2651_device::cts_w));
	printer.dsr_handler().set(m_usart[0], FUNC(scn2651_device::dsr_w));
}

/**************************************************************************************************************

Ann Arbor Ambassador.
Chips: Z80A, M58725P-15, 6x MK4116N-36P, 2x SCN2651C, nvram, button-battery
Crystals: 18.414, 5.0688

***************************************************************************************************************/

ROM_START( aaa )
	ROM_REGION(0x2000, "loprog", 0)
	ROM_LOAD( "459_1.bin",    0x0000, 0x1000, CRC(55fb3e3b) SHA1(349cd257b1468827e1b389be7c989d0e4a13a5f1) )
	ROM_LOAD( "459_3.bin",    0x1000, 0x1000, CRC(e1e84ca4) SHA1(42dc5f4211beee79178f0c03bb45c66833119eae) )

	ROM_REGION(0x2000, "hiprog", 0)
	ROM_LOAD( "459_4.bin",    0x0000, 0x2000, CRC(4038aa89) SHA1(caf33c1f87aa396860324b9c73b35e4221f03d2e) )

	ROM_REGION(0x1000, "font", 0)
	ROM_LOAD( "202510b.bin",  0x0000, 0x1000, CRC(deda4aa4) SHA1(0bce5a8dc260ba51f3e431d8da408eac1f41acf7) )
ROM_END

} // anonymous namespace


COMP( 1981, aaa, 0, 0, aaa, aaa, aaa_state, empty_init, "Ann Arbor", "Ambassador", MACHINE_IS_SKELETON )
