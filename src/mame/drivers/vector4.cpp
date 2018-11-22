// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Vector 4

        08/12/2009 Skeleton driver.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "bus/rs232/rs232.h"


class vector4_state : public driver_device
{
public:
	vector4_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void vector4(machine_config &config);

private:
	void vector4_io(address_map &map);
	void vector4_mem(address_map &map);

	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
};


void vector4_state::vector4_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xdfff).ram();
	map(0xe000, 0xefff).rom();
	map(0xf000, 0xf7ff).ram();
	map(0xf800, 0xf8ff).rom();
	map(0xfc00, 0xffff).ram();
}

void vector4_state::vector4_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x02, 0x03).rw("uart1", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x04, 0x05).rw("uart2", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x06, 0x07).rw("uart3", FUNC(i8251_device::read), FUNC(i8251_device::write));
}

/* Input ports */
static INPUT_PORTS_START( vector4 )
INPUT_PORTS_END


void vector4_state::machine_reset()
{
	m_maincpu->set_state_int(Z80_PC, 0xe000);
}


void vector4_state::vector4(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(4'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &vector4_state::vector4_mem);
	m_maincpu->set_addrmap(AS_IO, &vector4_state::vector4_io);

	/* video hardware */
	clock_device &uart_clock(CLOCK(config, "uart_clock", 153600));
	uart_clock.signal_handler().set("uart1", FUNC(i8251_device::write_txc));
	uart_clock.signal_handler().append("uart1", FUNC(i8251_device::write_rxc));
	uart_clock.signal_handler().append("uart2", FUNC(i8251_device::write_txc));
	uart_clock.signal_handler().append("uart2", FUNC(i8251_device::write_rxc));
	uart_clock.signal_handler().append("uart3", FUNC(i8251_device::write_txc));
	uart_clock.signal_handler().append("uart3", FUNC(i8251_device::write_rxc));

	i8251_device &uart1(I8251(config, "uart1", 0));
	uart1.txd_handler().set("rs232a", FUNC(rs232_port_device::write_txd));
	uart1.dtr_handler().set("rs232a", FUNC(rs232_port_device::write_dtr));
	uart1.rts_handler().set("rs232a", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232a(RS232_PORT(config, "rs232a", default_rs232_devices, "terminal"));
	rs232a.rxd_handler().set("uart1", FUNC(i8251_device::write_rxd));
	rs232a.dsr_handler().set("uart1", FUNC(i8251_device::write_dsr));
	rs232a.cts_handler().set("uart1", FUNC(i8251_device::write_cts));

	i8251_device &uart2(I8251(config, "uart2", 0));
	uart2.txd_handler().set("rs232b", FUNC(rs232_port_device::write_txd));
	uart2.dtr_handler().set("rs232b", FUNC(rs232_port_device::write_dtr));
	uart2.rts_handler().set("rs232b", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232b(RS232_PORT(config, "rs232b", default_rs232_devices, nullptr));
	rs232b.rxd_handler().set("uart2", FUNC(i8251_device::write_rxd));
	rs232b.dsr_handler().set("uart2", FUNC(i8251_device::write_dsr));
	rs232b.cts_handler().set("uart2", FUNC(i8251_device::write_cts));

	i8251_device &uart3(I8251(config, "uart3", 0));
	uart3.txd_handler().set("rs232c", FUNC(rs232_port_device::write_txd));
	uart3.dtr_handler().set("rs232c", FUNC(rs232_port_device::write_dtr));
	uart3.rts_handler().set("rs232c", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232c(RS232_PORT(config, "rs232c", default_rs232_devices, nullptr));
	rs232c.rxd_handler().set("uart3", FUNC(i8251_device::write_rxd));
	rs232c.dsr_handler().set("uart3", FUNC(i8251_device::write_dsr));
	rs232c.cts_handler().set("uart3", FUNC(i8251_device::write_cts));
}

/* ROM definition */
ROM_START( vector4 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )

	ROM_SYSTEM_BIOS( 0, "v4c", "ver 4.0c" )
	ROMX_LOAD( "vg40cl_ihl.bin", 0xe000, 0x0400, CRC(dcaf79e6) SHA1(63619ddb12ff51e5862902fb1b33a6630f555ad7), ROM_BIOS(0))
	ROMX_LOAD( "vg40ch_ihl.bin", 0xe400, 0x0400, CRC(3ff97d70) SHA1(b401e49aa97ac106c2fd5ee72d89e683ebe34e34), ROM_BIOS(0))

	ROM_SYSTEM_BIOS( 1, "v43", "ver 4.3" )
	ROMX_LOAD( "vg-em-43.bin",   0xe000, 0x1000, CRC(29a0fcee) SHA1(ca44de527f525b72f78b1c084c39aa6ce21731b5), ROM_BIOS(1))

	ROM_SYSTEM_BIOS( 2, "v5", "ver 5.0" )
	ROMX_LOAD( "vg-zcb50.bin",   0xe000, 0x1000, CRC(22d692ce) SHA1(cbb21b0acc98983bf5febd59ff67615d71596e36), ROM_BIOS(2))
	ROM_LOAD( "mfdc.bin", 0xf800, 0x0100, CRC(d82a40d6) SHA1(cd1ef5fb0312cd1640e0853d2442d7d858bc3e3b))
ROM_END

/* Driver */

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY            FULLNAME    FLAGS
COMP( 19??, vector4, 0,      0,      vector4, vector4, vector4_state, empty_init, "Vector Graphics", "Vector 4", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
