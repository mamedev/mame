// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

    Skeleton driver for Intel MCS-51 System Design Kit (SDK-51).

*******************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/mcs51/mcs51.h"
#include "cpu/mcs48/mcs48.h"
//#include "imagedev/cassette.h"
#include "machine/bankdev.h"
#include "machine/i8155.h"
#include "machine/i8243.h"
#include "machine/i8251.h"


namespace {

class sdk51_state : public driver_device
{
public:
	sdk51_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_progmem(*this, "progmem")
		, m_datamem(*this, "datamem")
		, m_mem0(*this, "mem0")
		, m_upi(*this, "upi")
		, m_usart(*this, "usart")
		, m_cycles(*this, "cycles")
		, m_kb(*this, "KB%u", 0U)
	{
	}

	void sdk51(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void psen_map(address_map &map) ATTR_COLD;
	void movx_map(address_map &map) ATTR_COLD;
	void progmem_map(address_map &map) ATTR_COLD;
	void datamem_map(address_map &map) ATTR_COLD;
	void mem0_map(address_map &map) ATTR_COLD;

	u8 psen_r(offs_t offset);
	u8 datamem_r(offs_t offset);
	void datamem_w(offs_t offset, u8 data);

	u8 brkmem_r(offs_t offset);
	void brkmem_w(offs_t offset, u8 data);

	u8 upibus_r();
	void upibus_w(u8 data);
	void display_clock_w(int state);
	void upiobf_w(int state);
	void serial_control_w(u8 data);

	required_device<mcs51_cpu_device> m_maincpu;
	required_device<address_map_bank_device> m_progmem;
	required_device<address_map_bank_device> m_datamem;
	required_device<address_map_bank_device> m_mem0;
	required_device<upi41_cpu_device> m_upi;
	required_device<i8251_device> m_usart;
	required_region_ptr<u8> m_cycles;
	required_ioport_array<7> m_kb;

	bool m_upiobf = false;
	u8 m_serial_control = 0;
	u32 m_kdtime = 0;
	bool m_display_clock = false;
};

u8 sdk51_state::psen_r(offs_t offset)
{
	return m_progmem->read8(offset);
}

u8 sdk51_state::datamem_r(offs_t offset)
{
	return m_datamem->read8(offset);
}

void sdk51_state::datamem_w(offs_t offset, u8 data)
{
	m_datamem->write8(offset, data);
}

u8 sdk51_state::brkmem_r(offs_t offset)
{
	return 1 | (m_upiobf ? 2 : 0);
}

void sdk51_state::brkmem_w(offs_t offset, u8 data)
{
	m_mem0->set_bank(0);
}

u8 sdk51_state::upibus_r()
{
	u8 result = 0xff;

	if (!BIT(m_serial_control, 0))
		result &= m_usart->read(BIT(m_serial_control, 2));

	if (!BIT(m_upi->p2_r(), 6))
		for (int n = 0; n < 7; n++)
			if (BIT(m_kdtime, n))
				result &= m_kb[n]->read();

	return result;
}

void sdk51_state::upibus_w(u8 data)
{
	if (!BIT(m_serial_control, 1))
		m_usart->write(BIT(m_serial_control, 2), data);
}

void sdk51_state::serial_control_w(u8 data)
{
	m_serial_control = data;
}

void sdk51_state::display_clock_w(int state)
{
	if (!m_display_clock && state)
		m_kdtime = ((m_kdtime << 1) & 0xfffffe) | BIT(m_upi->p1_r(), 6);

	m_display_clock = state;
}

void sdk51_state::upiobf_w(int state)
{
	if (m_upiobf != bool(state))
	{
		m_upiobf = state;
		m_maincpu->set_input_line(MCS51_INT0_LINE, state ? ASSERT_LINE : CLEAR_LINE);
	}
}

void sdk51_state::psen_map(address_map &map)
{
	map(0x0000, 0xffff).r(FUNC(sdk51_state::psen_r));
}

void sdk51_state::movx_map(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(sdk51_state::datamem_r), FUNC(sdk51_state::datamem_w));
}

void sdk51_state::progmem_map(address_map &map)
{
	map(0x0000, 0x1fff).m(m_mem0, FUNC(address_map_bank_device::amap8));
	map(0x2000, 0x3fff).ram();
	map(0xe000, 0xffff).rom().region("monitor", 0);
}

void sdk51_state::datamem_map(address_map &map)
{
	progmem_map(map);
	map(0xa000, 0xa001).mirror(0xffe).rw(m_upi, FUNC(upi41_cpu_device::upi41_master_r), FUNC(upi41_cpu_device::upi41_master_w));
	map(0xb000, 0xb0ff).mirror(0x700).rw("io", FUNC(i8155_device::memory_r), FUNC(i8155_device::memory_w));
	map(0xb800, 0xb807).mirror(0x7f8).rw("io", FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
	map(0xc000, 0xdfff).rw(FUNC(sdk51_state::brkmem_r), FUNC(sdk51_state::brkmem_w));
}

void sdk51_state::mem0_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x2000, 0x2fff).mirror(0x1000).rom().region("monitor", 0);
}

static INPUT_PORTS_START(sdk51)
	PORT_START("KB0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CHAR(UCHAR_SHIFT_1) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Control") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(0x09) PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(0x1b) PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Rubout") PORT_CHAR(0x7f) PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(' ') PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('@') PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CHAR(0x0d) PORT_CODE(KEYCODE_ENTER)

	PORT_START("KB1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('0') PORT_CHAR('\\') PORT_CODE(KEYCODE_0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('1') PORT_CHAR('!') PORT_CODE(KEYCODE_1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('2') PORT_CHAR('"') PORT_CODE(KEYCODE_2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('3') PORT_CHAR('#') PORT_CODE(KEYCODE_3)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('4') PORT_CHAR('$') PORT_CODE(KEYCODE_4)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('5') PORT_CHAR('%') PORT_CODE(KEYCODE_5)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('6') PORT_CHAR('&') PORT_CODE(KEYCODE_6)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('7') PORT_CHAR('\'') PORT_CODE(KEYCODE_7)

	PORT_START("KB2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('8') PORT_CHAR('(') PORT_CODE(KEYCODE_8)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('9') PORT_CHAR(')') PORT_CODE(KEYCODE_9)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('-') PORT_CHAR('=') PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(';') PORT_CHAR('+') PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(':') PORT_CHAR('*') PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('/') PORT_CHAR('?') PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('.') PORT_CHAR(']') PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(',') PORT_CHAR('[') PORT_CODE(KEYCODE_COMMA)

	PORT_START("KB3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('A') PORT_CODE(KEYCODE_A)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('B') PORT_CODE(KEYCODE_B)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('C') PORT_CODE(KEYCODE_C)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('D') PORT_CODE(KEYCODE_D)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('E') PORT_CODE(KEYCODE_E)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('F') PORT_CODE(KEYCODE_F)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('G') PORT_CODE(KEYCODE_G)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('H') PORT_CODE(KEYCODE_H)

	PORT_START("KB4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('I') PORT_CODE(KEYCODE_I)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('J') PORT_CODE(KEYCODE_J)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('K') PORT_CODE(KEYCODE_K)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('L') PORT_CODE(KEYCODE_L)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('M') PORT_CODE(KEYCODE_M)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('N') PORT_CODE(KEYCODE_N)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('O') PORT_CODE(KEYCODE_O)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('P') PORT_CODE(KEYCODE_P)

	PORT_START("KB5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('Q') PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('R') PORT_CODE(KEYCODE_R)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('S') PORT_CODE(KEYCODE_S)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('T') PORT_CODE(KEYCODE_T)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('U') PORT_CODE(KEYCODE_U)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('V') PORT_CODE(KEYCODE_V)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('W') PORT_CODE(KEYCODE_W)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('X') PORT_CODE(KEYCODE_X)

	PORT_START("KB6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('Y') PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('Z') PORT_CODE(KEYCODE_Z)
	PORT_BIT(0xfc, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

void sdk51_state::machine_start()
{
	m_upiobf = false;
	m_serial_control = 0x0f;
	m_display_clock = true;

	save_item(NAME(m_upiobf));
	save_item(NAME(m_serial_control));
	save_item(NAME(m_kdtime));
	save_item(NAME(m_display_clock));
}

void sdk51_state::machine_reset()
{
	m_mem0->set_bank(1);
	m_kdtime = 0;
}

void sdk51_state::sdk51(machine_config &config)
{
	I8031(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &sdk51_state::psen_map);
	m_maincpu->set_addrmap(AS_IO, &sdk51_state::movx_map);

	ADDRESS_MAP_BANK(config, m_progmem);
	m_progmem->set_addrmap(0, &sdk51_state::progmem_map);
	m_progmem->set_data_width(8);
	m_progmem->set_addr_width(16);

	ADDRESS_MAP_BANK(config, m_datamem);
	m_datamem->set_addrmap(0, &sdk51_state::datamem_map);
	m_datamem->set_data_width(8);
	m_datamem->set_addr_width(16);

	ADDRESS_MAP_BANK(config, m_mem0);
	m_mem0->set_addrmap(0, &sdk51_state::mem0_map);
	m_mem0->set_data_width(8);
	m_mem0->set_addr_width(14);
	m_mem0->set_stride(0x2000);

	I8041A(config, m_upi, 6_MHz_XTAL);
	m_upi->p1_in_cb().set(FUNC(sdk51_state::upibus_r));
	m_upi->p1_out_cb().set(FUNC(sdk51_state::upibus_w));
	m_upi->p2_in_cb().set("upiexp", FUNC(i8243_device::p2_r));
	m_upi->p2_out_cb().set(FUNC(sdk51_state::display_clock_w)).bit(7);
	m_upi->p2_out_cb().append(FUNC(sdk51_state::upiobf_w)).bit(4);
	m_upi->p2_out_cb().append("upiexp", FUNC(i8243_device::p2_w)).mask(0x0f);
	m_upi->prog_out_cb().set("upiexp", FUNC(i8243_device::prog_w));
	m_upi->t1_in_cb().set(m_usart, FUNC(i8251_device::txrdy_r));

	i8243_device &upiexp(I8243(config, "upiexp"));
	upiexp.p4_out_cb().set(FUNC(sdk51_state::serial_control_w));

	i8155_device &io(I8155(config, "io", 6_MHz_XTAL / 3));
	io.out_to_callback().set(m_usart, FUNC(i8251_device::write_txc));
	io.out_to_callback().append(m_usart, FUNC(i8251_device::write_txc));

	I8251(config, m_usart, 6_MHz_XTAL / 3);
	m_usart->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_usart->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_usart, FUNC(i8251_device::write_rxd));
	rs232.cts_handler().set(m_usart, FUNC(i8251_device::write_cts));
}

ROM_START(sdk51)
	ROM_REGION(0x2000, "monitor", 0) // "SDK-51 MONITOR VER. 1.03"
	ROM_LOAD("u59-e000.bin", 0x0000, 0x1000, CRC(cc6c7b05) SHA1(ead75920347ff19487e730e90e1e1f7207d44601))
	ROM_LOAD("u60-f000.bin", 0x1000, 0x1000, CRC(da6e664d) SHA1(18416106307f37dba6dbb789f3d39fe6d5294755))

	ROM_REGION(0x0400, "upi", 0)
	ROM_LOAD("u41-8041a.bin", 0x000, 0x400, CRC(02f38b69) SHA1(ab2ac73b69b3297572583242ed5bd717eb116c37))

	ROM_REGION(0x0200, "cycles", 0)
	ROM_LOAD("u63-3622a.bin", 0x000, 0x200, CRC(85cbd498) SHA1(f0214b6d02d6d153b5fafd9adf5a23013373c9c4)) // pin 9 output stuck high but not used
ROM_END

} // anonymous namespace


COMP(1981, sdk51, 0, 0, sdk51, sdk51, sdk51_state, empty_init, "Intel", "MCS-51 System Design Kit", MACHINE_IS_SKELETON)
