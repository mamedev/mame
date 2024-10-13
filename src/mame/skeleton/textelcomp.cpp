// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

    Skeleton driver for Textel Compact portable digital teletype machine.

*******************************************************************************/

#include "emu.h"
#include "cpu/m6502/m65sc02.h"
#include "machine/input_merger.h"
#include "machine/6522via.h"
#include "machine/mos6551.h"
#include "machine/msm58321.h"
#include "video/sed1330.h"
#include "emupal.h"
#include "screen.h"


namespace {

class textelcomp_state : public driver_device
{
public:
	textelcomp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rtc(*this, "rtc")
		, m_chargen(*this, "chargen")
		, m_keys(*this, "KEY%X", 0U)
		, m_leds(*this, "led%u", 0U)
		, m_keyscan(0)
		, m_shift_register(0)
		, m_shift_data(true)
		, m_shift_clock(true)
	{ }

	void textelcomp(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	void keyscan_w(u8 data);
	u8 keyboard_r();
	void shift_data_w(int state);
	void shift_clock_w(int state);
	void update_shift_output();
	void rtc_w(u8 data);

	void mem_map(address_map &map) ATTR_COLD;
	void lcdc_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<msm58321_device> m_rtc;
	required_region_ptr<u8> m_chargen;
	required_ioport_array<16> m_keys;
	output_finder<16> m_leds;

	u8 m_keyscan;
	u16 m_shift_register;
	bool m_shift_data;
	bool m_shift_clock;
};


void textelcomp_state::machine_start()
{
	m_rtc->cs1_w(1);
	subdevice<mos6551_device>("acia")->write_cts(0);

	m_leds.resolve();

	save_item(NAME(m_keyscan));
	save_item(NAME(m_shift_register));
	save_item(NAME(m_shift_data));
	save_item(NAME(m_shift_clock));
}

void textelcomp_state::keyscan_w(u8 data)
{
	// 2x TC4094BP driving keyboard lights
	if (BIT(data, 4) && !BIT(m_keyscan, 4))
		update_shift_output();

	m_keyscan = data;
}

u8 textelcomp_state::keyboard_r()
{
	return m_keys[m_keyscan & 0x0f]->read();
}

void textelcomp_state::shift_data_w(int state)
{
	m_shift_data = state;
}

void textelcomp_state::shift_clock_w(int state)
{
	if (state && !m_shift_clock)
	{
		m_shift_clock = true;
		m_shift_register = (m_shift_register << 1) | m_shift_data;
		if (BIT(m_keyscan, 4))
			update_shift_output();
	}
	else if (!state && m_shift_clock)
		m_shift_clock = false;
}

void textelcomp_state::update_shift_output()
{
	for (int i = 0; i < 16; i++)
		m_leds[i] = BIT(m_shift_register, i);
}

void textelcomp_state::rtc_w(u8 data)
{
	// Minimum address/data setup time is given as 0 µs in Oki and Epson datasheets
	// Address and data are written to the VIA at the same time as the control strobes
	if (!BIT(data, 5))
		m_rtc->write_w(0);
	if (!BIT(data, 6))
		m_rtc->read_w(0);
	if (!BIT(data, 7))
		m_rtc->address_write_w(0);

	m_rtc->d0_w(BIT(data, 0));
	m_rtc->d1_w(BIT(data, 1));
	m_rtc->d2_w(BIT(data, 2));
	m_rtc->d3_w(BIT(data, 3));

	if (BIT(data, 5))
		m_rtc->write_w(1);
	if (BIT(data, 6))
		m_rtc->read_w(1);
	if (BIT(data, 7))
		m_rtc->address_write_w(1);
}


void textelcomp_state::mem_map(address_map &map)
{
	map(0x0000, 0x1eff).ram(); // MB8464A-10L (battery backed?)
	map(0x1f00, 0x1f0f).m("via0", FUNC(via6522_device::map));
	map(0x1f10, 0x1f1f).m("via1", FUNC(via6522_device::map));
	map(0x1f20, 0x1f2f).m("via2", FUNC(via6522_device::map));
	map(0x1f30, 0x1f3f).m("via3", FUNC(via6522_device::map));
	map(0x1f40, 0x1f43).rw("acia", FUNC(mos6551_device::read), FUNC(mos6551_device::write));
	map(0x1f70, 0x1f70).rw("lcdc", FUNC(sed1330_device::status_r), FUNC(sed1330_device::data_w));
	map(0x1f71, 0x1f71).rw("lcdc", FUNC(sed1330_device::data_r), FUNC(sed1330_device::command_w));
	map(0x4000, 0x7fff).ram(); // HY62256ALP-10 (battery backed?)
	map(0x8000, 0x9fff).ram(); // MB8464A-10L (battery backed?)
	map(0xa000, 0xffff).rom().region("maincpu", 0x2000);
}

void textelcomp_state::lcdc_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0xf000, 0xffff).rom().region("chargen", 0x1000);
}


static INPUT_PORTS_START(textelcomp)
	PORT_START("KEY0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_SHIFT_1) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN) // code 0x10
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN) // code 0x13
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN) // code 0x80/0x81
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN) // code 0x90/0x91
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNKNOWN) // code 0xa0
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN) // code 0xa1
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN) // code 0xa4

	PORT_START("KEY1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift Lock") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN) // code 0x11
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN) // code 0x14
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('1') PORT_CHAR('!') PORT_CODE(KEYCODE_1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Lösch") PORT_CHAR(0x08) PORT_CODE(KEYCODE_TAB) // left of Q
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('a') PORT_CHAR('A') PORT_CHAR(0x01) PORT_CODE(KEYCODE_A)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('y') PORT_CHAR('Y') PORT_CHAR(0x19) PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN) // code 0xa5

	PORT_START("KEY2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN) // modifier
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN) // code 0x12
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN) // code 0x15
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('2') PORT_CHAR('"') PORT_CODE(KEYCODE_2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('q') PORT_CHAR('Q') PORT_CHAR(0x11) PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('s') PORT_CHAR('S') PORT_CHAR(0x13) PORT_CODE(KEYCODE_S)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('x') PORT_CHAR('X') PORT_CHAR(0x18) PORT_CODE(KEYCODE_X)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN) // code 0xa8

	PORT_START("KEY3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_SHIFT_2) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('3') PORT_CHAR('#') PORT_CODE(KEYCODE_3)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('w') PORT_CHAR('W') PORT_CHAR(0x17) PORT_CODE(KEYCODE_W)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('d') PORT_CHAR('D') PORT_CHAR(0x04) PORT_CODE(KEYCODE_D)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('c') PORT_CHAR('C') PORT_CHAR(0x03) PORT_CODE(KEYCODE_C)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN) // code 0xa9

	PORT_START("KEY4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN) // code 0x1b
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN) // code 0x20
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN) // code 0x21
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('4') PORT_CHAR('+') PORT_CODE(KEYCODE_4)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('e') PORT_CHAR('E') PORT_CHAR(0x05) PORT_CODE(KEYCODE_E)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('f') PORT_CHAR('F') PORT_CHAR(0x06) PORT_CODE(KEYCODE_F)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('v') PORT_CHAR('V') PORT_CHAR(0x16) PORT_CODE(KEYCODE_V)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(0x0d) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN) // code 0x22
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('5') PORT_CHAR('%') PORT_CODE(KEYCODE_5)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('r') PORT_CHAR('R') PORT_CHAR(0x12) PORT_CODE(KEYCODE_R)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('g') PORT_CHAR('G') PORT_CHAR(0x07) PORT_CODE(KEYCODE_G)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('b') PORT_CHAR('B') PORT_CODE(KEYCODE_B)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(' ') PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('6') PORT_CHAR('&') PORT_CODE(KEYCODE_6)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('t') PORT_CHAR('T') PORT_CHAR(0x14) PORT_CODE(KEYCODE_T)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('h') PORT_CHAR('H') PORT_CODE(KEYCODE_H)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('n') PORT_CHAR('N') PORT_CHAR(0x0e) PORT_CODE(KEYCODE_N)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN) // code 0x40
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN) // code 0x44
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('7') PORT_CHAR('/') PORT_CODE(KEYCODE_7)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('z') PORT_CHAR('Z') PORT_CHAR(0x1a) PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('j') PORT_CHAR('J') PORT_CHAR(0x0a) PORT_CODE(KEYCODE_J)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('m') PORT_CHAR('M') PORT_CODE(KEYCODE_M)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN) // code 0x41
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN) // code 0x45
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('8') PORT_CHAR('(') PORT_CODE(KEYCODE_8)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('u') PORT_CHAR('U') PORT_CHAR(0x15) PORT_CODE(KEYCODE_U)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('k') PORT_CHAR('K') PORT_CHAR(0x0b) PORT_CODE(KEYCODE_K)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(',') PORT_CHAR(';') PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN) // code 0x50
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN) // code 0x46
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('9') PORT_CHAR(')') PORT_CODE(KEYCODE_9)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('i') PORT_CHAR('I') PORT_CHAR(0x09) PORT_CODE(KEYCODE_I)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('l') PORT_CHAR('L') PORT_CHAR(0x0c) PORT_CODE(KEYCODE_L)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('.') PORT_CHAR(':') PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEYA")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN) // code 0x43
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('0') PORT_CODE(KEYCODE_0)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('o') PORT_CHAR('O') PORT_CHAR(0x0f) PORT_CODE(KEYCODE_O)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Ö") PORT_CHAR(0xf6, 0x7c) PORT_CHAR(0xd6, 0x5c) PORT_CHAR(0x1c) PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('-') PORT_CHAR('=') PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEYB")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN) // code 0x60
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(0xdf, 0x7e) PORT_CHAR('?') PORT_CODE(KEYCODE_MINUS) // ß
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('p') PORT_CHAR('P') PORT_CHAR(0x10) PORT_CODE(KEYCODE_P)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Ä") PORT_CHAR(0xe4, 0x7b) PORT_CHAR(0xc4, 0x5b) PORT_CHAR(0x1b) PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEYC")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN) // code 0x30/0x31
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('*') PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Ü") PORT_CHAR(0xfc, 0x7d) PORT_CHAR(0xdc, 0x5d) PORT_CHAR(0x1d) PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEYD")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN) // code 0x71
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEYE")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN) // code 0x70
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEYF")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


void textelcomp_state::textelcomp(machine_config &config)
{
	M65SC02(config, m_maincpu, 3.6864_MHz_XTAL / 2); // G65SC02P-2 (clock not verified)
	m_maincpu->set_addrmap(AS_PROGRAM, &textelcomp_state::mem_map);

	INPUT_MERGER_ANY_HIGH(config, "mainirq").output_handler().set_inputline(m_maincpu, m65sc02_device::IRQ_LINE);

	via6522_device &via0(R65C22(config, "via0", 3.6864_MHz_XTAL / 2)); // G65SC22P-2
	via0.irq_handler().set("mainirq", FUNC(input_merger_device::in_w<0>));

	R65C22(config, "via1", 3.6864_MHz_XTAL / 2); // G65SC22P-2
	// IRQ might be connected on hardware, but is never enabled

	via6522_device &via2(R65C22(config, "via2", 3.6864_MHz_XTAL / 2)); // G65SC22P-2
	via2.readpa_handler().set(FUNC(textelcomp_state::keyboard_r));
	via2.writepb_handler().set(FUNC(textelcomp_state::keyscan_w));
	via2.cb1_handler().set(FUNC(textelcomp_state::shift_clock_w));
	via2.cb2_handler().set(FUNC(textelcomp_state::shift_data_w));

	via6522_device &via3(R65C22(config, "via3", 3.6864_MHz_XTAL / 2)); // G65SC22P-2
	via3.writepa_handler().set(FUNC(textelcomp_state::rtc_w));
	via3.ca2_handler().set(m_rtc, FUNC(msm58321_device::cs2_w)).invert();
	via3.ca2_handler().append(m_rtc, FUNC(msm58321_device::stop_w)).invert();
	// TODO: PB7 toggling generates beeps?

	mos6551_device &acia(MOS6551(config, "acia", 3.6864_MHz_XTAL / 2)); // G65SC51P-2
	acia.set_xtal(3.6864_MHz_XTAL / 2);
	acia.irq_handler().set("mainirq", FUNC(input_merger_device::in_w<1>));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_color(rgb_t(0x00, 0xff, 0x80));
	screen.set_refresh_hz(50);
	screen.set_size(640, 201);
	screen.set_visarea(0, 640-1, 0, 201-1);
	screen.set_palette("palette");
	screen.set_screen_update("lcdc", FUNC(sed1330_device::screen_update));

	PALETTE(config, "palette", palette_device::MONOCHROME_INVERTED);

	sed1330_device &lcdc(SED1330(config, "lcdc", 6.4_MHz_XTAL)); // SED1330F + B&W LCD
	lcdc.set_addrmap(0, &textelcomp_state::lcdc_map);
	lcdc.set_screen("screen");

	MSM58321(config, m_rtc, 32.768_kHz_XTAL); // RTC58321A
	m_rtc->set_default_24h(true);
	m_rtc->d0_handler().set("via3", FUNC(via6522_device::write_pa0));
	m_rtc->d1_handler().set("via3", FUNC(via6522_device::write_pa1));
	m_rtc->d2_handler().set("via3", FUNC(via6522_device::write_pa2));
	m_rtc->d3_handler().set("via3", FUNC(via6522_device::write_pa3));
	m_rtc->busy_handler().set("via0", FUNC(via6522_device::write_ca1)); // source of periodic falling edge interrupt?
}


ROM_START(a1010)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("d15_31.bin",  0x0000, 0x8000, CRC(5ee1175d) SHA1(87ff6a3d5c64a53b0ab23d54aa343365c44d0407))

	ROM_REGION(0x8000, "chargen", 0)
	ROM_LOAD("chargen.bin", 0x0000, 0x8000, CRC(07daa70e) SHA1(8066a0ac238b06fbeeb99c3a2a8a9e70a27db7a9))
ROM_END

} // anonymous namespace


COMP(1993, a1010, 0, 0, textelcomp, textelcomp, textelcomp_state, empty_init, "Humantechnik", "Textel Compact A1010-0", MACHINE_IS_SKELETON)
