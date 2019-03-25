// license:BSD-3-Clause
// copyright-holders:AJR
/***********************************************************************************************************************************

Skeleton driver for Ann Arbor Ambassador terminal.

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/input_merger.h"
#include "machine/mc2661.h"

class aaa_state : public driver_device
{
public:
	aaa_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pci(*this, "pci%u", 0U)
		, m_chargen(*this, "chargen")
	{ }

	void aaa(machine_config &config);

private:
	template<int N> u8 pci_r(offs_t offset);
	template<int N> void pci_w(offs_t offset, u8 data);

	void mem_map(address_map &map);
	void io_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device_array<mc2661_device, 2> m_pci;
	required_region_ptr<u8> m_chargen;
};

template<int N>
u8 aaa_state::pci_r(offs_t offset)
{
	return m_pci[N]->read(offset >> 1);
}

template<int N>
void aaa_state::pci_w(offs_t offset, u8 data)
{
	m_pci[N]->write(offset >> 1, data);
}

void aaa_state::mem_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("maincpu", 0);
	map(0x2000, 0x27ff).ram();
	map(0x2800, 0x2fff).ram(); // NVRAM?
	map(0x8000, 0x9fff).rom().region("maincpu", 0x8000);
	map(0xc000, 0xffff).ram();
}

void aaa_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).select(6).r(FUNC(aaa_state::pci_r<0>));
	map(0x01, 0x01).select(6).w(FUNC(aaa_state::pci_w<0>));
	map(0x40, 0x40).select(6).r(FUNC(aaa_state::pci_r<1>));
	map(0x41, 0x41).select(6).w(FUNC(aaa_state::pci_w<1>));
	map(0x87, 0x87).nopw();
}

static INPUT_PORTS_START( aaa )
INPUT_PORTS_END

void aaa_state::aaa(machine_config &config)
{
	Z80(config, m_maincpu, 2'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &aaa_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &aaa_state::io_map);

	input_merger_device &pciint(INPUT_MERGER_ANY_HIGH(config, "pciint")); // open collector?
	pciint.output_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	MC2661(config, m_pci[0], 5.0688_MHz_XTAL);
	m_pci[0]->txrdy_handler().set("pciint", FUNC(input_merger_device::in_w<0>));
	m_pci[0]->rxrdy_handler().set("pciint", FUNC(input_merger_device::in_w<1>));
	m_pci[0]->txemt_dschg_handler().set("pciint", FUNC(input_merger_device::in_w<2>));

	MC2661(config, m_pci[1], 5.0688_MHz_XTAL);
	m_pci[1]->txrdy_handler().set("pciint", FUNC(input_merger_device::in_w<3>));
	m_pci[1]->rxrdy_handler().set("pciint", FUNC(input_merger_device::in_w<4>));
	m_pci[1]->txemt_dschg_handler().set("pciint", FUNC(input_merger_device::in_w<5>));
}

/**************************************************************************************************************

Ann Arbor Ambassador.
Chips: Z80A, M58725P (16k RAM), 2x SCN2651C, nvram, button-battery
Crystals: 18.414, 6.0688

***************************************************************************************************************/

ROM_START( aaa )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "459_1.bin",    0x0000, 0x1000, CRC(55fb3e3b) SHA1(349cd257b1468827e1b389be7c989d0e4a13a5f1) )
	ROM_LOAD( "459_3.bin",    0x1000, 0x1000, CRC(e1e84ca4) SHA1(42dc5f4211beee79178f0c03bb45c66833119eae) )
	ROM_LOAD( "459_4.bin",    0x8000, 0x2000, CRC(4038aa89) SHA1(caf33c1f87aa396860324b9c73b35e4221f03d2e) )

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "202510b.bin",  0x0000, 0x1000, CRC(deda4aa4) SHA1(0bce5a8dc260ba51f3e431d8da408eac1f41acf7) )
ROM_END

COMP( 1981, aaa, 0, 0, aaa, aaa, aaa_state, empty_init, "Ann Arbor", "Ambassador", MACHINE_IS_SKELETON )
