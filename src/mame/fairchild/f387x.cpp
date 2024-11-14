// license:BSD-3-Clause
// copyright-holders:AJR
/************************************************************************************************************

    Skeleton driver for Fairchild F387X PEP (Prototyping, Emulating and Programming) System.

************************************************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/f8/f8.h"
#include "machine/f3853.h"


namespace {

class f387x_state : public driver_device
{
public:
	f387x_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rs232c(*this, "rs232c")
		, m_keypad(*this, "ROW%u", 0U)
	{
	}

	void f387x(machine_config &config);

private:
	u8 p8_r();
	void p8_w(u8 data);
	u8 p9_r();
	void p9_w(u8 data);
	u8 ipor_r();
	void opor_w(u8 data);

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<f8_cpu_device> m_maincpu;
	required_device<rs232_port_device> m_rs232c;
	required_ioport_array<6> m_keypad;
};


u8 f387x_state::p8_r()
{
	return 0;
}

void f387x_state::p8_w(u8 data)
{
}

u8 f387x_state::p9_r()
{
	return 0;
}

void f387x_state::p9_w(u8 data)
{
}

u8 f387x_state::ipor_r()
{
	return m_rs232c->rxd_r() ? 0x80 : 0;
}

void f387x_state::opor_w(u8 data)
{
	m_rs232c->write_txd(BIT(data, 0));
}

void f387x_state::mem_map(address_map &map)
{
	map(0x2b80, 0x2bff).ram(); // F6810
	map(0x8000, 0x87ff).rom().region("pepbug", 0);
}

void f387x_state::io_map(address_map &map)
{
	map(0x20, 0x20).rw(FUNC(f387x_state::p8_r), FUNC(f387x_state::p8_w));
	map(0x21, 0x21).rw(FUNC(f387x_state::p9_r), FUNC(f387x_state::p9_w));
	map(0x24, 0x24).rw(FUNC(f387x_state::ipor_r), FUNC(f387x_state::opor_w));
	map(0xeb, 0xeb).nopw(); // code deliberately outputs to nonexistent port here
}


static INPUT_PORTS_START(f387x)
	PORT_START("ROW0")
	PORT_BIT(0x0f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("C/R") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("DEL") PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("CHG") PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("+") PORT_CODE(KEYCODE_EQUALS)

	PORT_START("ROW1")
	PORT_BIT(0x0f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("MEM") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("REG") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("PORT") PORT_CODE(KEYCODE_O)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS)

	PORT_START("ROW2")
	PORT_BIT(0x0f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("3  NEXT") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("7  ACC") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("B  FIND") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("F  GO") PORT_CODE(KEYCODE_F)

	PORT_START("ROW3")
	PORT_BIT(0x0f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("2  PREV") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("6  W") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("A  MOVE") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("E  LOAD") PORT_CODE(KEYCODE_E)

	PORT_START("ROW4")
	PORT_BIT(0x0f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("1  DC") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("5  ISAR") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("9  HEX") PORT_CODE(KEYCODE_9)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("D  B CLR") PORT_CODE(KEYCODE_D)

	PORT_START("ROW5")
	PORT_BIT(0x0f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("0  PC") PORT_CODE(KEYCODE_0)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("4  2716") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("8  E70") PORT_CODE(KEYCODE_8)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("C  BRPT") PORT_CODE(KEYCODE_C)

	PORT_START("SW3")
	PORT_DIPNAME(1, 1, DEF_STR(Unknown)) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(1, DEF_STR(Off))
	PORT_DIPSETTING(0, DEF_STR(On))
	PORT_DIPNAME(2, 2, DEF_STR(Unknown)) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(2, DEF_STR(Off))
	PORT_DIPSETTING(0, DEF_STR(On))
	PORT_DIPNAME(4, 4, DEF_STR(Unknown)) PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(4, DEF_STR(Off))
	PORT_DIPSETTING(0, DEF_STR(On))
	PORT_DIPNAME(8, 8, DEF_STR(Unknown)) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(8, DEF_STR(Off))
	PORT_DIPSETTING(0, DEF_STR(On))
INPUT_PORTS_END

static DEVICE_INPUT_DEFAULTS_START(terminal)
	DEVICE_INPUT_DEFAULTS("RS232_RXBAUD", 0xff, RS232_BAUD_110)
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD", 0xff, RS232_BAUD_110)
	DEVICE_INPUT_DEFAULTS("RS232_DATABITS", 0xff, RS232_DATABITS_8)
	DEVICE_INPUT_DEFAULTS("RS232_PARITY", 0xff, RS232_PARITY_NONE)
	DEVICE_INPUT_DEFAULTS("RS232_STOPBITS", 0xff, RS232_STOPBITS_2)
DEVICE_INPUT_DEFAULTS_END

void f387x_state::f387x(machine_config &config)
{
	F8(config, m_maincpu, 2_MHz_XTAL); // F3850PC
	m_maincpu->set_addrmap(AS_PROGRAM, &f387x_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &f387x_state::io_map);
	m_maincpu->romc08_callback().set_constant(0x80);

	F3853(config, "smi", 0);

	RS232_PORT(config, m_rs232c, default_rs232_devices, "terminal");
	m_rs232c->set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));
}

ROM_START(f387x)
	ROM_REGION(0x1800, "pepbug", 0)
	ROM_LOAD("pepbug.u16", 0x0000, 0x0800, CRC(de05ac6d) SHA1(220281a7016aae785417bbfe8383c76f72f8fac2))
	// 2716 sockets at U14 and U15 are not populated
ROM_END

} // anonymous namespace


COMP(1979, f387x, 0, 0, f387x, f387x, f387x_state, empty_init, "Fairchild Instrument & Camera Corporation", "F387X PEP System", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
