// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Sergey Svishchev
/***************************************************************************

Robotron A7150

2009-10-04 Skeleton driver.

http://www.robotrontechnik.de/index.htm?/html/computer/a7150.htm

http://www.tiffe.de/Robotron/MMS16/
- Confidence test is documented in A7150_Rechner...pdf, pp. 112-119
- Internal test of KGS -- in KGS-K7070.pdf, pp. 19-23

After about a minute, the self-test will appear.

To do:
- native keyboard -- K7637 and/or K7672
- interrupts
- ABS K7071 (text-only video)
- ABG K7075 (CGA compatible video)
- KES K5170 media controller (derived from iSBC 215A but not 100% compatible)
- boot BOS (iRMX clone)
- boot DCP (DOS clone)
- boot MUTOS (V7 UNIX) to multiuser
- boot SCP (CP/M clone)
- A7100 model (KES at 0x100 etc.)

****************************************************************************/

#include "emu.h"

#include "isbc_215g.h"

#include "bus/multibus/multibus.h"
#include "bus/multibus/robotron_k7070.h"
#include "bus/multibus/robotron_k7071.h"
#include "bus/rs232/rs232.h"
#include "cpu/i86/i86.h"
#include "machine/i8087.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/keyboard.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"


namespace {


class a7150_state : public driver_device
{
public:
	a7150_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_bus(*this, "slot")
		, m_maincpu(*this, "maincpu")
		, m_uart8251(*this, "uart8251")
		, m_pit8253(*this, "pit8253")
		, m_pic8259(*this, "pic8259")
		, m_rs232(*this, "rs232")
	{ }

	void a7150(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

private:
	void ppi_c_w(uint8_t data);

	bool m_ifss_loopback = 0;

	required_device<multibus_device> m_bus;
	required_device<i8086_cpu_device> m_maincpu;
	required_device<i8251_device> m_uart8251;
	required_device<pit8253_device> m_pit8253;
	required_device<pic8259_device> m_pic8259;
	required_device<rs232_port_device> m_rs232;

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	u8 bus_pio_r(offs_t offset) { return m_bus->space(AS_IO).read_byte(offset); }
	void bus_pio_w(offs_t offset, u8 data) { m_bus->space(AS_IO).write_byte(offset, data); }
};


void a7150_state::ppi_c_w(uint8_t data)
{
	// b0 -- INTR(B)
	// b1 -- /OBF(B)
	// m_centronics->write_ack(BIT(data, 2));
	// m_centronics->write_strobe(BIT(data, 3));
	// b7 -- /NMIDIS
}

void a7150_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0xf7fff).ram();
	map(0xf8000, 0xfffff).rom().region("maincpu", 0);
}

void a7150_state::io_map(address_map &map)
{
	map.unmap_value_high();
	// map PIO to Multibus by default
	map(0x0000, 0xffff).rw(FUNC(a7150_state::bus_pio_r), FUNC(a7150_state::bus_pio_w));
//  map(0x0000, 0x0003).unmaprw(); // memory parity 1-2
//  map(0x0040, 0x0043).unmaprw(); // memory parity 3-4
	map(0x004a, 0x004a).w("isbc_215g", FUNC(isbc_215g_device::write)); // KES board
	map(0x00c0, 0x00c3).rw(m_pic8259, FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0x00c8, 0x00cf).rw("ppi8255", FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff);
	map(0x00d0, 0x00d7).rw(m_pit8253, FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask16(0x00ff);
	map(0x00d8, 0x00db).rw(m_uart8251, FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff);
//  map(0x0300, 0x031f).unmaprw(); // ASP board #1
//  map(0x0320, 0x033f).unmaprw(); // ASP board #2
}

static DEVICE_INPUT_DEFAULTS_START( kbd_rs232_defaults )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_28800 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_28800 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_2 )
	DEVICE_INPUT_DEFAULTS( "FLOW_CONTROL", 0x01, 0x01 )
DEVICE_INPUT_DEFAULTS_END

void a7150_state::machine_reset()
{
	m_ifss_loopback = false;
}

void a7150_state::machine_start()
{
	save_item(NAME(m_ifss_loopback));
}

static void a7150_cards(device_slot_interface &device)
{
	device.option_add("kgs", ROBOTRON_K7070);
	device.option_add("abs", ROBOTRON_K7071);
}

/*
 * K2771.30 ZRE - processor board
 * K3571    OPS - 256KB RAM board (x4)
 * K7070    KGS - graphics terminal, running firmware from A7100
 * K7072    ABG - dumb grayscale framebuffer
 * K5170    KES - media controller
 */
void a7150_state::a7150(machine_config &config)
{
	MULTIBUS(config, m_bus, 10_MHz_XTAL); // FIXME: clock driven by bus master
	m_bus->int_callback<1>().set(m_pic8259, FUNC(pic8259_device::ir1_w));
	m_bus->int_callback<6>().set(m_pic8259, FUNC(pic8259_device::ir6_w));
	m_bus->int_callback<7>().set(m_pic8259, FUNC(pic8259_device::ir7_w));
	MULTIBUS_SLOT(config, "slot:1", m_bus, a7150_cards, "kgs", false);

	// ZRE board

	I8086(config, m_maincpu, XTAL(9'832'000) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &a7150_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &a7150_state::io_map);
	m_maincpu->set_irq_acknowledge_callback("pic8259", FUNC(pic8259_device::inta_cb));
	m_maincpu->esc_opcode_handler().set("i8087", FUNC(i8087_device::insn_w));
	m_maincpu->esc_data_handler().set("i8087", FUNC(i8087_device::addr_w));

	i8087_device &i8087(I8087(config, "i8087", XTAL(9'832'000) / 2));
	i8087.set_space_86(m_maincpu, AS_PROGRAM);
	i8087.irq().set(m_pic8259, FUNC(pic8259_device::ir0_w));
	i8087.busy().set_inputline("maincpu", INPUT_LINE_TEST);

	PIC8259(config, m_pic8259, 0);
	m_pic8259->out_int_callback().set_inputline(m_maincpu, 0);

	// IFSP port (IRQ 4)
	i8255_device &ppi(I8255(config, "ppi8255"));
//  ppi.in_pa_callback().set("cent_status_in", FUNC(input_buffer_device::read));
//  ppi.out_pb_callback().set("cent_data_out", output_latch_device::write));
	ppi.out_pc_callback().set(FUNC(a7150_state::ppi_c_w));

	PIT8253(config, m_pit8253, 0);
	m_pit8253->set_clk<0>(14.7456_MHz_XTAL / 4);
	m_pit8253->out_handler<0>().set(m_pic8259, FUNC(pic8259_device::ir2_w));
	m_pit8253->set_clk<1>(14.7456_MHz_XTAL / 4);
	m_pit8253->set_clk<2>(14.7456_MHz_XTAL / 4);
	m_pit8253->out_handler<2>().set([this] (bool state) { m_uart8251->write_rxc(state); m_uart8251->write_txc(state); });

	// IFSS port -- keyboard runs at 28800 8N2
	I8251(config, m_uart8251, 0);
	m_uart8251->txd_handler().set([this] (bool state) { if (m_ifss_loopback) m_uart8251->write_rxd(state); else m_rs232->write_txd(state); });
	m_uart8251->dtr_handler().set([this] (bool state) { if (m_ifss_loopback) m_uart8251->write_dsr(state); else m_rs232->write_dtr(state); });
	m_uart8251->rts_handler().set([this] (bool state) { m_ifss_loopback = !state; });
	m_uart8251->rxrdy_handler().set(m_pic8259, FUNC(pic8259_device::ir6_w));

	RS232_PORT(config, m_rs232, default_rs232_devices, "keyboard");
	m_rs232->rxd_handler().set(m_uart8251, FUNC(i8251_device::write_rxd));
	m_rs232->cts_handler().set(m_uart8251, FUNC(i8251_device::write_cts));
	m_rs232->dsr_handler().set(m_uart8251, FUNC(i8251_device::write_dsr));
	m_rs232->set_option_device_input_defaults("keyboard", DEVICE_INPUT_DEFAULTS_NAME(kbd_rs232_defaults));

	// KES board

	ISBC_215G(config, "isbc_215g", 0, 0x4a, m_maincpu).irq_callback().set(m_pic8259, FUNC(pic8259_device::ir5_w));
}

/* ROM definition */
ROM_START( a7150 )
	ROM_REGION16_LE( 0x8000, "maincpu", 0 )
	ROM_DEFAULT_BIOS("2.3")

	// A7100
	ROM_SYSTEM_BIOS(0, "1.1", "ACT 1.1")
	ROMX_LOAD("q259.bin", 0x4001, 0x2000, CRC(fb5b547b) SHA1(1d17fcededa91cad321a7b237a46a308142d902b), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("q260.bin", 0x0001, 0x2000, CRC(b51f8ed6) SHA1(9aa6291bf8ab49a343741717366992649e2957b3), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("q261.bin", 0x4000, 0x2000, CRC(43c08ea3) SHA1(ea697180b415b71d834968be84431a6efe9490c2), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("q262.bin", 0x0000, 0x2000, CRC(9df1c396) SHA1(a627889e1162e5b2fe95804de52bb78e41aaf7cc), ROM_BIOS(0) | ROM_SKIP(1))

	// A7150
	ROM_SYSTEM_BIOS(1, "2.1", "ACT 2.1") // requires K7075 video card
	ROMX_LOAD("265.bin",  0x4001, 0x2000, CRC(a5fb5f35) SHA1(9d9501441cad0ef724dec7b5ffb52b17a678a9f8), ROM_BIOS(1) | ROM_SKIP(1))
	ROMX_LOAD("266.bin",  0x0001, 0x2000, CRC(f5898eb7) SHA1(af3fd82813fbea7883dea4d7e23a9b5e5b2b844a), ROM_BIOS(1) | ROM_SKIP(1))
	ROMX_LOAD("267.bin",  0x4000, 0x2000, CRC(c1873a01) SHA1(77f15cc217cd854732fbe33d395e1ea9867fedd7), ROM_BIOS(1) | ROM_SKIP(1))
	ROMX_LOAD("268.bin",  0x0000, 0x2000, CRC(e3f09213) SHA1(1e2d69061f8e84697440b219181e0b870fe21835), ROM_BIOS(1) | ROM_SKIP(1))

	ROM_SYSTEM_BIOS(2, "2.2", "ACT 2.2")
	ROMX_LOAD("269.bin",  0x4001, 0x2000, CRC(f137f94b) SHA1(7cb79f332db48cb66dae04c1ce1bdd169a6ab561), ROM_BIOS(2) | ROM_SKIP(1))
	ROMX_LOAD("270.bin",  0x0001, 0x2000, CRC(1ea44a33) SHA1(f5708d1f6a9dc109979a9a91a80f2a4e4956d1eb), ROM_BIOS(2) | ROM_SKIP(1))
	ROMX_LOAD("271.bin",  0x4000, 0x2000, CRC(de2222c9) SHA1(e02225c93b49f0380dfb2d996b63370141359199), ROM_BIOS(2) | ROM_SKIP(1))
	ROMX_LOAD("272.bin",  0x0000, 0x2000, CRC(5001c528) SHA1(ce67c35326fbfd17f086a37ffe81b79aefaef0cb), ROM_BIOS(2) | ROM_SKIP(1))

	ROM_SYSTEM_BIOS(3, "2.3", "ACT 2.3")
	ROMX_LOAD("273.rom",  0x4001, 0x2000, CRC(67ca9b78) SHA1(bcb6221f6df28b24b602846b149ac12e93b5e356), ROM_BIOS(3) | ROM_SKIP(1))
	ROMX_LOAD("274.rom",  0x0001, 0x2000, CRC(6fa68834) SHA1(49abe48bbb5ae151f977a9c63b27336c15e8a08d), ROM_BIOS(3) | ROM_SKIP(1))
	ROMX_LOAD("275.rom",  0x4000, 0x2000, CRC(0da54426) SHA1(7492caff98b1d1a896c5964942b17beadf996b60), ROM_BIOS(3) | ROM_SKIP(1))
	ROMX_LOAD("276.rom",  0x0000, 0x2000, CRC(5924192a) SHA1(eb494d9f96a0b3ea69f4b9cb2b7add66a8c16946), ROM_BIOS(3) | ROM_SKIP(1))
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY         FULLNAME  FLAGS
COMP( 1986, a7150, 0,      0,      a7150,   0,     a7150_state, empty_init, "VEB Robotron", "A7150",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
