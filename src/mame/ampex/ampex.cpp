// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

2017-11-05 Skeleton

Ampex Dialogue 80 terminal

Chips: CRT-5037, COM8017, SMC (COM)5016-5, MK3880N (Z80), SN74LS424N (TIM8224)
Crystals: 4.9152, 23.814
Other: Beeper, 5x 10sw-dips.

The program code seems to have been designed with a 8080 CPU in mind, using no
Z80-specific opcodes. This impression is reinforced by the IC types present on
the PCB, which go so far as to include the standard 8224 clock generator.

*******************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/ay31015.h"
#include "machine/com8116.h"
#include "video/tms9927.h"
#include "screen.h"


namespace {

#define AMPEX_CH_WIDTH 7

class ampex_state : public driver_device
{
public:
	ampex_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_vtac(*this, "vtac")
		, m_uart(*this, "uart")
		, m_dbrg(*this, "dbrg")
		, m_p_chargen(*this, "chargen")
	{ }

	void ampex(machine_config &config);

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void write_54xx(u8 data);
	u8 read_5840();
	void write_5840(u8 data);
	u8 read_5841();
	void write_5841(u8 data);
	u8 read_5842();
	void write_5843(u8 data);
	u8 read_5846();
	u8 read_5847();

	u8 page_r(offs_t offset);
	void page_w(offs_t offset, u8 data);

	void vsyn_w(int state);
	void so_w(int state);
	void dav_w(int state);

	void mem_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;

	u8 m_page = 0;
	u8 m_attr = 0;
	bool m_attr_readback = false;
	bool m_uart_loopback = false;
	std::unique_ptr<u16[]> m_paged_ram{};

	required_device<cpu_device> m_maincpu;
	required_device<crt5037_device> m_vtac;
	required_device<ay31015_device> m_uart;
	required_device<com8116_device> m_dbrg;
	required_region_ptr<u8> m_p_chargen;
};

u32 ampex_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void ampex_state::write_54xx(u8 data)
{
	// Written during keyboard polling
}

u8 ampex_state::read_5840()
{
	logerror("%s: Read from 5840\n", machine().describe_context());
	return 0;
}

void ampex_state::write_5840(u8 data)
{
	m_page = (data & 0x30) >> 4;

	logerror("%s: Write %02X to 5840\n", machine().describe_context(), data);
}

u8 ampex_state::read_5841()
{
	u8 result = m_uart->dav_r() << 3;
	result |= m_uart->or_r() << 4;
	result |= m_uart->pe_r() << 5;
	result |= m_uart->fe_r() << 6;
	return result;
}

void ampex_state::write_5841(u8 data)
{
	m_uart_loopback = BIT(data, 7);
	m_attr_readback = BIT(data, 5);
}

u8 ampex_state::read_5842()
{
	//logerror("%s: Read from 5842\n", machine().describe_context());
	return 0;
}

void ampex_state::write_5843(u8 data)
{
	//logerror("%s: Write %02X to 5843\n", machine().describe_context(), data);
	m_attr = (data & 0x78) >> 3;
}

u8 ampex_state::read_5846()
{
	// probably acknowledges RST 6 interrupt (value not used)
	return 0;
}

u8 ampex_state::read_5847()
{
	// acknowledges RST 4/5 interrupt (value not used)
	return 0;
}

u8 ampex_state::page_r(offs_t offset)
{
	if (m_attr_readback)
		return 0x87 | m_paged_ram[m_page * 0x1800 + offset] >> 5;
	else
		return 0xff & m_paged_ram[m_page * 0x1800 + offset];
}

void ampex_state::page_w(offs_t offset, u8 data)
{
	m_paged_ram[m_page * 0x1800 + offset] = data | m_attr << 8;
}

void ampex_state::vsyn_w(int state)
{
	// should generate RST 6 interrupt
}

void ampex_state::so_w(int state)
{
	if (m_uart_loopback)
		m_uart->write_si(state);
}

void ampex_state::dav_w(int state)
{
	// DAV should generate RST 7
}

void ampex_state::mem_map(address_map &map)
{
	map(0x0000, 0x2fff).rom().region("roms", 0);
	map(0x4000, 0x43ff).ram(); // main RAM
	map(0x4400, 0x53ff).ram(); // expansion RAM
	map(0x5400, 0x5400).portr("IN0");
	map(0x5401, 0x5401).portr("IN1");
	map(0x5402, 0x5402).portr("IN2");
	map(0x5403, 0x5403).portr("IN3");
	map(0x5404, 0x5404).portr("IN4");
	map(0x5405, 0x5405).portr("IN5");
	map(0x5406, 0x5406).portr("IN6");
	map(0x5407, 0x5407).portr("IN7");
	map(0x5408, 0x5408).portr("IN8");
	map(0x5409, 0x5409).portr("IN9");
	map(0x540a, 0x540a).portr("INA");
	map(0x540b, 0x540b).portr("INB");
	map(0x540c, 0x540c).portr("INC");
	map(0x5400, 0x54ff).w(FUNC(ampex_state::write_54xx));
	map(0x5840, 0x5840).rw(FUNC(ampex_state::read_5840), FUNC(ampex_state::write_5840));
	map(0x5841, 0x5841).rw(FUNC(ampex_state::read_5841), FUNC(ampex_state::write_5841));
	map(0x5842, 0x5842).r(FUNC(ampex_state::read_5842)).w(m_uart, FUNC(ay31015_device::transmit));
	map(0x5843, 0x5843).r(m_uart, FUNC(ay31015_device::receive)).w(FUNC(ampex_state::write_5843));
	map(0x5846, 0x5846).r(FUNC(ampex_state::read_5846));
	map(0x5847, 0x5847).r(FUNC(ampex_state::read_5847));
	map(0x5c00, 0x5c0f).rw(m_vtac, FUNC(crt5037_device::read), FUNC(crt5037_device::write));
	map(0x8000, 0x97ff).rw(FUNC(ampex_state::page_r), FUNC(ampex_state::page_w));
	map(0xc000, 0xcfff).ram(); // video RAM
}

static INPUT_PORTS_START( ampex )
	PORT_START("IN0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('\\') PORT_CHAR('|') PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('1') PORT_CHAR('!') PORT_CODE(KEYCODE_1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('2') PORT_CHAR('"') PORT_CODE(KEYCODE_2)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('3') PORT_CHAR('#') PORT_CODE(KEYCODE_3)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('4') PORT_CHAR('$') PORT_CODE(KEYCODE_4)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('5') PORT_CHAR('%') PORT_CODE(KEYCODE_5)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('6') PORT_CHAR('&') PORT_CODE(KEYCODE_6)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('7') PORT_CHAR('\'') PORT_CODE(KEYCODE_7)

	PORT_START("IN1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR(0x1b) PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('q') PORT_CHAR('Q') PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('w') PORT_CHAR('W') PORT_CODE(KEYCODE_W)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('e') PORT_CHAR('E') PORT_CODE(KEYCODE_E)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('r') PORT_CHAR('R') PORT_CODE(KEYCODE_R)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('t') PORT_CHAR('T') PORT_CODE(KEYCODE_T)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('y') PORT_CHAR('Y') PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('u') PORT_CHAR('U') PORT_CODE(KEYCODE_U)

	PORT_START("IN2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CHAR(UCHAR_SHIFT_2) PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('a') PORT_CHAR('A') PORT_CODE(KEYCODE_A)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('s') PORT_CHAR('S') PORT_CODE(KEYCODE_S)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('d') PORT_CHAR('D') PORT_CODE(KEYCODE_D)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('f') PORT_CHAR('F') PORT_CODE(KEYCODE_F)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('g') PORT_CHAR('G') PORT_CODE(KEYCODE_G)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('h') PORT_CHAR('H') PORT_CODE(KEYCODE_H)

	PORT_START("IN3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN) // 0x83 (Break?)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift (Left)") PORT_CHAR(UCHAR_SHIFT_1) PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('z') PORT_CHAR('Z') PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('x') PORT_CHAR('X') PORT_CODE(KEYCODE_X)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('c') PORT_CHAR('C') PORT_CODE(KEYCODE_C)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('v') PORT_CHAR('V') PORT_CODE(KEYCODE_V)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('b') PORT_CHAR('B') PORT_CODE(KEYCODE_B)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('n') PORT_CHAR('N') PORT_CODE(KEYCODE_N)

	PORT_START("IN4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('m') PORT_CHAR('M') PORT_CODE(KEYCODE_M)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR(',') PORT_CHAR('<') PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('.') PORT_CHAR('>') PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('/') PORT_CHAR('?') PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift (Right)") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR(' ') PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IN5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('j') PORT_CHAR('J') PORT_CODE(KEYCODE_J)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('k') PORT_CHAR('K') PORT_CODE(KEYCODE_K)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('l') PORT_CHAR('L') PORT_CODE(KEYCODE_L)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR(';') PORT_CHAR('+') PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('[') PORT_CHAR('{') PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR(']') PORT_CHAR('}') PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN) // 0x1f (shifted: 0x8a) (Page New Line?)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN) // 0x94 (Line Ins?)

	PORT_START("IN6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('i') PORT_CHAR('I') PORT_CODE(KEYCODE_I)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('o') PORT_CHAR('O') PORT_CODE(KEYCODE_O)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('p') PORT_CHAR('P') PORT_CODE(KEYCODE_P)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('^') PORT_CHAR('~') PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR(0x0a) PORT_CODE(KEYCODE_RALT) PORT_NAME("Line Feed")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR(0x0d) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN) // 0xd4 (shifted: 0xf4)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN) // 0xd9 (shifted: 0xf9)

	PORT_START("IN7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('8') PORT_CHAR('(') PORT_CODE(KEYCODE_8)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('9') PORT_CHAR(')') PORT_CODE(KEYCODE_9)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('0') PORT_CHAR('_') PORT_CODE(KEYCODE_0)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR(':') PORT_CHAR('*') PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('-') PORT_CHAR('=') PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('@') PORT_CHAR('`') PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR(0x09) PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN) // 0x90 (shifted: 0x84)

	PORT_START("IN8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD)) PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD)) PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD)) PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(COMMA_PAD)) PORT_CODE(KEYCODE_COMMA_PAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN) // 0x97 (Char Del?)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN) // 0x96 (Char Ins?)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IN9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN) // 0x0d (top half of keypad Enter?)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD)) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD)) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD)) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(HOME)) PORT_CODE(KEYCODE_HOME)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Del") PORT_CODE(KEYCODE_RCONTROL) // 0x7f
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("INA")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(TAB_PAD)) PORT_CODE(KEYCODE_TAB_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD)) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD)) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD)) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN) // 0x95 (Line Del?)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(UP)) PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("INB")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD)) PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD)) PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD)) PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD)) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN) // 0x8b (shifted: 0x8c) (Clear?)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN) // 0xd0 (Page Erase?)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN) // 0xb4 (shifted: 0xb6) (Line Erase?)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("INC")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN) // 0x87 (shifted: 0x86)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN) // 0x85 (shifted: 0x84)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN) // 0x89 (shifted: 0x88)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN) // 0x92 (shifted: 0x93)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN) // 0x91
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN) // 0xb5 (shifted: 0xb7)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

void ampex_state::machine_start()
{
	m_page = 0;
	m_attr = 0;
	m_attr_readback = false;
	m_uart_loopback = false;
	m_paged_ram = std::make_unique<u16[]>(0x1800 * 4);

	m_uart->write_swe(0);

	// Are rates hardwired to DIP switches? They don't seem to be software-controlled...
	m_dbrg->str_w(0xe);
	m_dbrg->stt_w(0xe);

	// Make up some settings for the UART (probably also actually controlled by DIP switches)
	m_uart->write_nb1(1);
	m_uart->write_nb2(1);
	m_uart->write_np(1);
	m_uart->write_eps(1);
	m_uart->write_tsb(0);
	m_uart->write_cs(1);

	save_item(NAME(m_page));
	save_item(NAME(m_attr));
	save_item(NAME(m_attr_readback));
	save_item(NAME(m_uart_loopback));
	save_pointer(NAME(m_paged_ram), 0x1800 * 4);
}

void ampex_state::ampex(machine_config &config)
{
	Z80(config, m_maincpu, 23.814_MHz_XTAL / 9); // clocked by 8224?
	m_maincpu->set_addrmap(AS_PROGRAM, &ampex_state::mem_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(23.814_MHz_XTAL / 2, 105 * AMPEX_CH_WIDTH, 0, 80 * AMPEX_CH_WIDTH, 270, 0, 250);
	screen.set_screen_update(FUNC(ampex_state::screen_update));

	CRT5037(config, m_vtac, 23.814_MHz_XTAL / 2 / AMPEX_CH_WIDTH);
	m_vtac->set_char_width(AMPEX_CH_WIDTH);
	m_vtac->vsyn_callback().set(FUNC(ampex_state::vsyn_w));
	m_vtac->set_screen("screen");

	AY31015(config, m_uart, 0); // COM8017, actually
	m_uart->write_so_callback().set(FUNC(ampex_state::so_w));
	m_uart->write_dav_callback().set(FUNC(ampex_state::dav_w));
	m_uart->set_auto_rdav(true);

	COM5016_5(config, m_dbrg, 4.9152_MHz_XTAL);
	m_dbrg->fr_handler().set(m_uart, FUNC(ay31015_device::write_rcp));
	m_dbrg->ft_handler().set(m_uart, FUNC(ay31015_device::write_tcp));
}

ROM_START( dialog80 )
	ROM_REGION( 0x3000, "roms", 0 )
	ROM_LOAD( "3505240-01.u102", 0x0000, 0x0800, CRC(c5315780) SHA1(f2a8924f277d04bf4407f9b71b8d2788df0b1dc2) )
	ROM_LOAD( "3505240-02.u104", 0x0800, 0x0800, CRC(3fefa114) SHA1(d83c00605ae6c02d3aac7b572eb2bf615f0d4f3a) )
	ROM_LOAD( "3505240-03.u103", 0x1000, 0x0800, CRC(03abbcb2) SHA1(e5d382eefc3baff8f3e4d6b13219cb5eb1ca32f2) )
	ROM_LOAD( "3505240-04.u105", 0x1800, 0x0800, CRC(c051e15f) SHA1(16a066c39743ddf9a7da54bb8c03e2090d461862) )
	ROM_LOAD( "3505240-05.u100", 0x2000, 0x0800, CRC(6db6365b) SHA1(a68c83e554c2493645287e369749a07474723452) )
	ROM_LOAD( "3505240-06.u101", 0x2800, 0x0800, CRC(8f9a4969) SHA1(f9cd434f8d287c584cda429b45ca2537fdfb436b) )

	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD( "3505240-07.u69",  0x0000, 0x0800, CRC(838a16cb) SHA1(4301324b9fe9453c2d277972f9464c4214c6793d) )

	ROM_REGION( 0x0200, "proms", 0 ) // unknown TI 16-pin DIPs
	ROM_LOAD( "417129-010.u16",  0x0000, 0x0100, NO_DUMP )
	ROM_LOAD( "417129-010.u87",  0x0100, 0x0100, NO_DUMP )
ROM_END

} // anonymous namespace


COMP( 1980, dialog80, 0, 0, ampex, ampex, ampex_state, empty_init, "Ampex", "Dialogue 80", MACHINE_IS_SKELETON )
