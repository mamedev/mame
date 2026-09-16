// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

        Dual Systems 68000

        09/12/2009 Skeleton driver.

****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/i8085/i8085.h"
#include "bus/rs232/rs232.h"
//#include "bus/s100/s100.h"
#include "machine/input_merger.h"
#include "machine/scn_pci.h"


namespace {

class dual68_state : public driver_device
{
public:
	dual68_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_usart(*this, "usart%u", 1U)
		, m_p_ram(*this, "ram")
	{ }

	void dual68(machine_config &config);

private:
	uint8_t sio_direct_r(offs_t offset);
	void sio_direct_w(offs_t offset, uint8_t data);
	uint8_t sio_status_r();
	uint8_t fdc_status_r();

	void dual68_mem(address_map &map) ATTR_COLD;
	void sio4_io(address_map &map) ATTR_COLD;
	void sio4_mem(address_map &map) ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	required_device<cpu_device> m_maincpu;
	required_device_array<scn_pci_device, 4> m_usart;
	required_shared_ptr<uint16_t> m_p_ram;
};



uint8_t dual68_state::sio_direct_r(offs_t offset)
{
	return m_usart[0]->read(offset);
}

void dual68_state::sio_direct_w(offs_t offset, uint8_t data)
{
	m_usart[0]->write(offset, data);
}

uint8_t dual68_state::sio_status_r()
{
	return 0x00;
}

uint8_t dual68_state::fdc_status_r()
{
	return 0x40;
}

void dual68_state::dual68_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x00ffff).ram().share("ram");
	map(0x080000, 0x081fff).rom().region("mainbios", 0);
	map(0x7f0000, 0x7f0001).rw(FUNC(dual68_state::sio_direct_r), FUNC(dual68_state::sio_direct_w));
	map(0x7f00c0, 0x7f00c0).r(FUNC(dual68_state::fdc_status_r));
	map(0x800000, 0x801fff).rom().region("mainbios", 0);
}

void dual68_state::sio4_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x07ff).rom().region("siocpu", 0);
	map(0x8000, 0x87ff).mirror(0x7800).ram();
	//map(0xffe3, 0xffe3).lr8("usart0_rx_hack", []() { return 0x02; });
}

void dual68_state::sio4_io(address_map &map)
{
	map.unmap_value_high();
	map(0x06, 0x06).r(FUNC(dual68_state::sio_status_r));
	map(0x18, 0x18).nopw();
	map(0x20, 0x23).rw("usart1", FUNC(scn_pci_device::read), FUNC(scn_pci_device::write));
	map(0x28, 0x2b).rw("usart2", FUNC(scn_pci_device::read), FUNC(scn_pci_device::write));
	map(0x30, 0x33).rw("usart3", FUNC(scn_pci_device::read), FUNC(scn_pci_device::write));
	map(0x38, 0x3b).rw("usart4", FUNC(scn_pci_device::read), FUNC(scn_pci_device::write));
}

/* Input ports */
static INPUT_PORTS_START( dual68 )
INPUT_PORTS_END

void dual68_state::machine_reset()
{
	uint8_t *mainbios = memregion("mainbios")->base();

	memcpy((uint8_t*)m_p_ram.target(),mainbios,0x2000);
}

void dual68_state::dual68(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 16_MHz_XTAL / 2); // MC68000L8
	m_maincpu->set_addrmap(AS_PROGRAM, &dual68_state::dual68_mem);

	i8085a_cpu_device &siocpu(I8085A(config, "siocpu", 9.8304_MHz_XTAL)); // NEC D8085AC-2
	siocpu.set_addrmap(AS_PROGRAM, &dual68_state::sio4_mem);
	siocpu.set_addrmap(AS_IO, &dual68_state::sio4_io);

	for (auto &usart : m_usart)
		SCN2661B(config, usart, 9.8304_MHz_XTAL / 2);
	m_usart[0]->rxrdy_handler().set("usartint", FUNC(input_merger_device::in_w<0>));
	m_usart[1]->rxrdy_handler().set("usartint", FUNC(input_merger_device::in_w<1>));
	m_usart[2]->rxrdy_handler().set("usartint", FUNC(input_merger_device::in_w<2>));
	m_usart[3]->rxrdy_handler().set("usartint", FUNC(input_merger_device::in_w<3>));
	m_usart[0]->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_usart[0]->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	m_usart[0]->dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));

	INPUT_MERGER_ANY_HIGH(config, "usartint").output_handler().set_inputline("siocpu", I8085_RST65_LINE);

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set(m_usart[0], FUNC(scn_pci_device::rxd_w));
	rs232.dsr_handler().set(m_usart[0], FUNC(scn_pci_device::dsr_w));
	rs232.dcd_handler().set(m_usart[0], FUNC(scn_pci_device::dcd_w));
	rs232.cts_handler().set(m_usart[0], FUNC(scn_pci_device::cts_w));
}

/* ROM definition */
ROM_START( dual68 )
	ROM_REGION16_BE( 0x2000, "mainbios", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "v1", "2 * 4KB" )
	ROMX_LOAD("dual_cpu68000_1.bin", 0x0000, 0x1000, CRC(d1785c08) SHA1(73c1f68875f1d8eb5e92f4347f509c61103da90f),ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("dual_cpu68000_2.bin", 0x0001, 0x1000, CRC(b9f1ba3c) SHA1(8fd02936ad06d5a22d435d96f06e2442fc7d00ec),ROM_SKIP(1) | ROM_BIOS(0))

	ROM_SYSTEM_BIOS( 1, "v2", "2 * 2KB" )
	ROMX_LOAD("dual.u2.bin", 0x0000, 0x0800, CRC(e9c44fcd) SHA1(d5cc609d6f5e6745d5f0af1aa6dc66012333ed60),ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("dual.u3.bin", 0x0001, 0x0800, CRC(827b049f) SHA1(8209f8ab3d1068e5bab51e7eb12be46d4ea28354),ROM_SKIP(1) | ROM_BIOS(1))

	ROM_REGION( 0x10000, "siocpu", ROMREGION_ERASEFF )
	ROM_LOAD("dual_sio4.bin", 0x0000, 0x0800, CRC(6b0a1965) SHA1(5d2dc6c6a315293ded4b9fc95c8ac1599bf31dd3))
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY                     FULLNAME              FLAGS
COMP( 1981, dual68, 0,      0,      dual68,  dual68, dual68_state, empty_init, "Dual Systems Corporation", "Dual Systems 68000", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
