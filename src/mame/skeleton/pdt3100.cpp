// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

    Skeleton driver for Symbol PDT 3100 handheld computer.

    Parts side
MCU NEC V25 D70320GJ-8
3 x Flash Intel N28F010
Xtal R160SHAA7
SRAM Hitachi 658512ALFP-8
    LCD side
CMOS Pseudo Static RAM Toshiba TC518128CFTL-80
Display controller? STI 13130-018
Real-Time Clock plus RAM with Serial Interface MC68HC68T1

Installed program: "Scan3000" from the Spanish company "CB IBERSOFT".

*******************************************************************************/

#include "emu.h"
#include "cpu/nec/v25.h"


namespace {

class pdt3100_state : public driver_device
{
public:
	pdt3100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void pdt3100(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<v25_device> m_maincpu;
};

void pdt3100_state::mem_map(address_map &map)
{
	map(0x00000, 0x9ffff).ram();
	map(0xa0000, 0xbffff).rom().region("flash3", 0);
	map(0xc0000, 0xdffff).rom().region("flash2", 0);
	map(0xe0000, 0xfffff).rom().region("flash1", 0);
}

void pdt3100_state::io_map(address_map &map)
{
	map(0x01e7, 0x01e7).lr8([]() { return 0xff; }, "ff_r");
}

static INPUT_PORTS_START(pdt3100)
INPUT_PORTS_END

void pdt3100_state::pdt3100(machine_config &config)
{
	V25(config, m_maincpu, 8'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &pdt3100_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &pdt3100_state::io_map);
}

ROM_START(pdt3100)
	ROM_REGION(0x20000, "flash1", 0)
	ROM_LOAD("03-11061-24_e973_j._l_62605-12.hex_03-11-98.u9", 0x00000, 0x20000, CRC(527831fc) SHA1(370b62491564107b468f43824d21d898eaaf5967))

	ROM_REGION(0x20000, "flash2", 0)
	ROM_LOAD("03-13466-06_0de5_12-30-97.u8", 0x00000, 0x20000, CRC(c2bb5418) SHA1(418cbbfef609e61f3fb474d83b4faf75c68211c8))

	ROM_REGION(0x20000, "flash3", 0)
	ROM_LOAD("03-13467-06_c62e_02-02-98.u11", 0x00000, 0x20000, CRC(4ff6396c) SHA1(c4bc8e8e0991fa7f7852726c280f9aba57fef3f4))
ROM_END

} // anonymous namespace


COMP(1998, pdt3100, 0, 0, pdt3100, pdt3100, pdt3100_state, empty_init, "Symbol", "PDT 3100 (v1.10-00)", MACHINE_IS_SKELETON)
