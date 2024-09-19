// license:BSD-3-Clause
// copyright-holders:
/****************************************************************************

    Skeleton driver for 4th Generation "Super Chexx" bubble hockey
    electromechanical machines

****************************************************************************/

#include "emu.h"
#include "cpu/mc68hc11/mc68hc11.h"

namespace {

class schexx_state : public driver_device
{

public:
	schexx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void schexx(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<mc68hc11_cpu_device> m_maincpu;
};

void schexx_state::mem_map(address_map &map)
{
	map(0xe000, 0xffff).rom().region("program", 0x7e000);
}

INPUT_PORTS_START(schexx)
INPUT_PORTS_END

void schexx_state::schexx(machine_config &config)
{
	MC68HC11A1(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &schexx_state::mem_map);
}

ROM_START(schexx)
	ROM_REGION(0x80000, "program", 0)
	ROM_LOAD("schexx1_2_040.u4", 0x0000, 0x80000, CRC(8cbb7172) SHA1(ba59f7d8dd7e08c837181e18ce15e3c976f8f00b))
ROM_END

} // Anonymous namespace

GAME(1996, schexx, 0, schexx, schexx, schexx_state, empty_init, ROT0, "ICE", "Super Chexx (EM Bubble Hockey)", MACHINE_IS_SKELETON_MECHANICAL)
