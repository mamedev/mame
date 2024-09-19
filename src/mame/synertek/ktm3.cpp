// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

    KTM-3 (c) 1980 Synertek Systems Corp.

    This is a sort of single-board video display terminal. The second 6502,
    which uses part of the same program ROM as the first 6502 and runs on the
    inverse phases of its clock, appears to function as a crude CRTC,
    generating character addresses and sync signals with its address outputs.

    Only a few address lines are decoded at all. The resulting mirroring might
    not be accurately emulated yet.

*******************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/m6502/m6502.h"
#include "machine/clock.h"
#include "machine/mos6551.h"
#include "screen.h"


namespace {

class ktm3_state : public driver_device
{
public:
	ktm3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_pcpu(*this, "pcpu")
		, m_vcpu(*this, "vcpu")
		, m_acia(*this, "acia")
		, m_key_matrix(*this, "KEY%u", 0U)
		, m_option_sw(*this, "OPTION")
		, m_chargen(*this, "chargen")
		, m_ram(*this, "ram%u", 0U)
	{
	}

	void ktm3(machine_config &config);

	int ac_r();
	template <int N> int sw_r();

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	u8 allram_r(offs_t offset);
	void allram_w(offs_t offset, u8 data);
	u8 keyboard_r(offs_t offset);

	void pcpu_map(address_map &map) ATTR_COLD;
	void vcpu_map(address_map &map) ATTR_COLD;

	void signal_w(int state);

	required_device<cpu_device> m_pcpu;
	required_device<cpu_device> m_vcpu;
	required_device<mos6551_device> m_acia;
	required_ioport_array<10> m_key_matrix;
	required_ioport m_option_sw;
	required_region_ptr<u8> m_chargen;
	required_shared_ptr_array<u8, 2> m_ram;

	bool m_signal = false;
};

void ktm3_state::machine_start()
{
	m_acia->write_cts(0);

	save_item(NAME(m_signal));
}

u32 ktm3_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

u8 ktm3_state::allram_r(offs_t offset)
{
	return m_ram[0][offset] & m_ram[1][offset];
}

void ktm3_state::allram_w(offs_t offset, u8 data)
{
	m_ram[0][offset] = m_ram[1][offset] = data;
}

u8 ktm3_state::keyboard_r(offs_t offset)
{
	u8 ret = 0xff;

	for (int i = 0; i < 10; i++)
		if (BIT(offset, i))
			ret &= m_key_matrix[i]->read();

	return ret ^ 0xff;
}

void ktm3_state::pcpu_map(address_map &map)
{
	map(0x0000, 0x03ff).mirror(0xf000).rw(FUNC(ktm3_state::allram_r), FUNC(ktm3_state::allram_w));
	map(0x0400, 0x0400).mirror(0xe3ff).portr("SPECIAL");
	map(0x0800, 0x0bff).mirror(0xe000).r(FUNC(ktm3_state::keyboard_r));
	map(0x1400, 0x17ff).mirror(0xe000).ram().share("ram0"); // 2x SY2114L-3
	map(0x1800, 0x1bff).mirror(0xc000).ram().share("ram1"); // 2x SY2114L-3
	map(0x3800, 0x3fff).mirror(0xc000).rom().region("program", 0x000);
	map(0x5c00, 0x5c03).mirror(0x83fc).rw(m_acia, FUNC(mos6551_device::read), FUNC(mos6551_device::write));
}

void ktm3_state::vcpu_map(address_map &map)
{
	map(0x0000, 0x00ff).mirror(0xf800).rw(FUNC(ktm3_state::allram_r), FUNC(ktm3_state::allram_w));
	map(0x0100, 0x01ff).mirror(0xfc00).rom().region("program", 0x000);
	map(0x0300, 0x03ff).mirror(0xfc00).rom().region("program", 0x100);
}

void ktm3_state::signal_w(int state)
{
	m_signal = state;
}

int ktm3_state::ac_r()
{
	return m_signal;
}

template <int N>
int ktm3_state::sw_r()
{
	return BIT(m_option_sw->read(), N);
}

static INPUT_PORTS_START(ktm3)
	PORT_START("KEY0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('1') PORT_CHAR('!') PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(0x1b) PORT_NAME("Esc") PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('9') PORT_CHAR(')') PORT_CODE(KEYCODE_9)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('i') PORT_CHAR('I') PORT_CODE(KEYCODE_I)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('k') PORT_CHAR('K') PORT_CODE(KEYCODE_K)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('a') PORT_CHAR('A') PORT_CODE(KEYCODE_A)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('z') PORT_CHAR('Z') PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_SHIFT_1) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)

	PORT_START("KEY1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('2') PORT_CHAR('"') PORT_CODE(KEYCODE_2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('q') PORT_CHAR('Q') PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('0') PORT_CODE(KEYCODE_0)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('o') PORT_CHAR('O') PORT_CODE(KEYCODE_O)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('l') PORT_CHAR('L') PORT_CODE(KEYCODE_L)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('s') PORT_CHAR('S') PORT_CODE(KEYCODE_S)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('x') PORT_CHAR('X') PORT_CODE(KEYCODE_X)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE

	PORT_START("KEY2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('3') PORT_CHAR('#') PORT_CODE(KEYCODE_3)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('w') PORT_CHAR('W') PORT_CODE(KEYCODE_W)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(':') PORT_CHAR('*') PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('p') PORT_CHAR('P') PORT_CODE(KEYCODE_P)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(';') PORT_CHAR('+') PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('d') PORT_CHAR('D') PORT_CODE(KEYCODE_D)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('c') PORT_CHAR('C') PORT_CODE(KEYCODE_C)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL)) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL)

	PORT_START("KEY3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('4') PORT_CHAR('$') PORT_CODE(KEYCODE_4)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('e') PORT_CHAR('E') PORT_CODE(KEYCODE_E)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('-') PORT_CHAR('=') PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(0x0a) PORT_NAME("Line Feed") PORT_CODE(KEYCODE_RALT) // actually between P and Return key
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('@') PORT_CHAR('`') PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('f') PORT_CHAR('F') PORT_CODE(KEYCODE_F)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('v') PORT_CHAR('V') PORT_CODE(KEYCODE_V)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, ktm3_state, sw_r<0>)

	PORT_START("KEY4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('5') PORT_CHAR('%') PORT_CODE(KEYCODE_5)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('r') PORT_CHAR('R') PORT_CODE(KEYCODE_R)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('^') PORT_CHAR('~') PORT_CODE(KEYCODE_TILDE) // actually between - and Home/Clear keys
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(0x0d) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('[') PORT_CHAR('{') PORT_CODE(KEYCODE_OPENBRACE) // actually to right of @ `
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('g') PORT_CHAR('G') PORT_CODE(KEYCODE_G)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('b') PORT_CHAR('B') PORT_CODE(KEYCODE_B)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, ktm3_state, sw_r<1>)

	PORT_START("KEY5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('6') PORT_CHAR('&') PORT_CODE(KEYCODE_6)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('t') PORT_CHAR('T') PORT_CODE(KEYCODE_T)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(HOME)) PORT_CHAR(UCHAR_MAMEKEY(END)) PORT_NAME("Home  Clear") PORT_CODE(KEYCODE_HOME)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('_') PORT_CHAR(UCHAR_MAMEKEY(DEL)) PORT_CODE(KEYCODE_BACKSPACE) // actually to right of Return key
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(']') PORT_CHAR('}') PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('h') PORT_CHAR('H') PORT_CODE(KEYCODE_H)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('n') PORT_CHAR('N') PORT_CODE(KEYCODE_N)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, ktm3_state, sw_r<2>)

	PORT_START("KEY6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('7') PORT_CHAR('\'') PORT_CODE(KEYCODE_7)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('y') PORT_CHAR('Y') PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(' ') PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('/') PORT_CHAR('?') PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('\\') PORT_CHAR('|') PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('j') PORT_CHAR('J') PORT_CODE(KEYCODE_J)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('m') PORT_CHAR('M') PORT_CODE(KEYCODE_M)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, ktm3_state, sw_r<3>)

	PORT_START("KEY7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('8') PORT_CHAR('(') PORT_CODE(KEYCODE_8)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('u') PORT_CHAR('U') PORT_CODE(KEYCODE_U)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Break") PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('.') PORT_CHAR('>') PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(',') PORT_CHAR('<') PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, ktm3_state, sw_r<4>)

	PORT_START("KEY8")
	PORT_BIT(0x7f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, ktm3_state, sw_r<5>)

	PORT_START("KEY9")
	PORT_BIT(0x7f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, ktm3_state, sw_r<6>)

	PORT_START("OPTION")
	PORT_DIPNAME(0x01, 0x01, DEF_STR(Unused)) PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING(0x01, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x00, "Conversation Mode") PORT_DIPLOCATION("SW:2")
	PORT_DIPSETTING(0x02, "Half Duplex")
	PORT_DIPSETTING(0x00, "Full Duplex")
	PORT_DIPNAME(0x04, 0x04, "Stop Bits") PORT_DIPLOCATION("SW:3")
	PORT_DIPSETTING(0x04, "1")
	PORT_DIPSETTING(0x00, "2")
	PORT_DIPNAME(0x18, 0x18, "Parity") PORT_DIPLOCATION("SW:4,5")
	PORT_DIPSETTING(0x00, "Even")
	PORT_DIPSETTING(0x08, "Odd")
	PORT_DIPSETTING(0x10, "Mark")
	PORT_DIPSETTING(0x18, "Space")
	PORT_DIPNAME(0xe0, 0xc0, "Baud Rate") PORT_DIPLOCATION("SW:6,7,8")
	PORT_DIPSETTING(0x00, "109.92")
	PORT_DIPSETTING(0x20, "300")
	PORT_DIPSETTING(0x40, "600")
	PORT_DIPSETTING(0x60, "1200")
	PORT_DIPSETTING(0x80, "2400")
	PORT_DIPSETTING(0xa0, "4800")
	PORT_DIPSETTING(0xc0, "9600")
	PORT_DIPSETTING(0xe0, "19200")

	PORT_START("SPECIAL")
	PORT_BIT(0x3f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, ktm3_state, ac_r)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, ktm3_state, sw_r<7>)

	PORT_START("JUMPER")
	PORT_DIPNAME(0x01, 0x00, "Columns") PORT_DIPLOCATION("J1:1")
	PORT_DIPSETTING(0x01, "40")
	PORT_DIPSETTING(0x00, "80")
INPUT_PORTS_END

void ktm3_state::ktm3(machine_config &config)
{
	M6502(config, m_pcpu, 14.745_MHz_XTAL / 15); // SY6502 at U2; divider not verified
	m_pcpu->set_addrmap(AS_PROGRAM, &ktm3_state::pcpu_map);

	M6502(config, m_vcpu, 14.745_MHz_XTAL / 15); // SY6502 at U1; divider not verified
	m_vcpu->set_addrmap(AS_PROGRAM, &ktm3_state::vcpu_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(14.745_MHz_XTAL, 930, 0, 600, 262, 0, 240); // parameters guessed
	screen.set_screen_update(FUNC(ktm3_state::screen_update));

	MOS6551(config, m_acia, 14.745_MHz_XTAL / 15); // SY6551
	m_acia->set_xtal(14.745_MHz_XTAL / 8);
	m_acia->irq_handler().set_inputline(m_pcpu, m6502_device::IRQ_LINE);
	m_acia->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_acia->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	m_acia->dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "loopback"));
	rs232.rxd_handler().set(m_acia, FUNC(mos6551_device::write_rxd));
	rs232.cts_handler().set(m_acia, FUNC(mos6551_device::write_dsr));
	rs232.dcd_handler().set(m_acia, FUNC(mos6551_device::write_dcd));

	CLOCK(config, "60hz", 60).signal_handler().set(FUNC(ktm3_state::signal_w));
}

ROM_START(ktm3)
	ROM_REGION(0x800, "program", 0)
	ROM_LOAD("02-9001-126.bin", 0x000, 0x800, CRC(d7441e28) SHA1(bf0c05bfdcfd9083183325336d9702c67b7de63c))

	ROM_REGION(0x800, "chargen", 0)
	ROM_LOAD("02-0061-a.bin", 0x000, 0x800, CRC(9739e2ac) SHA1(672059b7618afb6c19632663d58a854ea9ec2401))
ROM_END

} // anonymous namespace


COMP(1980, ktm3, 0, 0, ktm3, ktm3, ktm3_state, empty_init, "Synertek Systems", "KTM-3", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
