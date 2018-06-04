// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

Skeleton driver for Ann Arbor Ambassador terminal.

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"

class aaa_state : public driver_device
{
public:
	aaa_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_chargen(*this, "chargen")
	{ }

	void aaa(machine_config &config);
	void mem_map(address_map &map);
	void io_map(address_map &map);
private:
	required_device<cpu_device> m_maincpu;
	required_region_ptr<u8> m_p_chargen;
};

void aaa_state::mem_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("maincpu", 0);
	map(0x2000, 0x27ff).ram();
	map(0x8000, 0x9fff).rom().region("maincpu", 0x8000);
	map(0xc000, 0xffff).ram();
}

void aaa_state::io_map(address_map &map)
{
	map.global_mask(0xff);
}

static INPUT_PORTS_START( aaa )
INPUT_PORTS_END

MACHINE_CONFIG_START(aaa_state::aaa)
	MCFG_DEVICE_ADD("maincpu", Z80, 2'000'000)
	MCFG_DEVICE_PROGRAM_MAP(mem_map)
	MCFG_DEVICE_IO_MAP(io_map)
MACHINE_CONFIG_END

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
