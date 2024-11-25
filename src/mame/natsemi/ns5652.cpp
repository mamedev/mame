// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    unknown National Semiconductor INS8900 Multibus card (980305652)

***************************************************************************/

#include "emu.h"
#include "cpu/pace/pace.h"
#include "machine/ins8250.h"


namespace {

class ns5652_state : public driver_device
{
public:
	ns5652_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ecprom(*this, "ecprom")
	{
	}

	void ns5652(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<pace_device> m_maincpu;
	required_region_ptr<u8> m_ecprom;
};


void ns5652_state::mem_map(address_map &map)
{
	map(0x0000, 0x0bff).mirror(0xd000).rom().region("program", 0);
	map(0xe800, 0xebff).ram(); // 4x MM2114J-3
	map(0xec00, 0xecff).lr16([this](offs_t offset) { return m_ecprom[offset]; }, "ecprom_r");
	map(0xf600, 0xf600).nopw(); // shift register outputs?
	map(0xf610, 0xf610).nopw();
	map(0xf620, 0xf620).nopw();
	map(0xf630, 0xf630).nopw();
	map(0xf640, 0xf640).nopw();
	map(0xf650, 0xf650).nopw();
	map(0xf660, 0xf660).nopw();
	map(0xf670, 0xf670).nopw();
	map(0xf680, 0xf680).nopw();
	map(0xf690, 0xf690).nopw();
	map(0xf6a0, 0xf6a0).nopw();
	map(0xf6b0, 0xf6b0).nopw();
	map(0xf6c0, 0xf6c0).nopw();
	map(0xf6d0, 0xf6d0).nopw();
	map(0xf6e0, 0xf6e0).nopw();
	map(0xf6f0, 0xf6f0).nopw();
	map(0xf700, 0xf700).nopw();
	map(0xf710, 0xf710).nopw();
	map(0xf720, 0xf720).nopw();
	map(0xf730, 0xf730).nopw();
	map(0xf740, 0xf740).nopw();
	map(0xf750, 0xf750).nopw();
	map(0xf760, 0xf760).nopw();
	map(0xf770, 0xf770).nopw();
	map(0xf780, 0xf780).nopw();
	map(0xf790, 0xf790).nopw();
}


static INPUT_PORTS_START(ns5652)
INPUT_PORTS_END

void ns5652_state::ns5652(machine_config &config)
{
	INS8900(config, m_maincpu, 1.8432_MHz_XTAL); // no other XTAL visible
	m_maincpu->set_addrmap(AS_PROGRAM, &ns5652_state::mem_map);

	INS8250(config, "ace", 1.8432_MHz_XTAL);
}

ROM_START(ns5652)
	ROM_REGION16_LE(0x1800, "program", 0) // all MM2708Q
	ROM_LOAD16_BYTE("5652_001b.bin", 0x0000, 0x0400, CRC(03acf738) SHA1(e512ccf64473e0b7291d8cc14f44858cac2048e6))
	ROM_LOAD16_BYTE("5652_004b.bin", 0x0001, 0x0400, CRC(b238b1ba) SHA1(90735194cc7f111fc7c1cdde1a9aab4945b00a7e))
	ROM_LOAD16_BYTE("5652_002b.bin", 0x0800, 0x0400, CRC(2fd33c25) SHA1(5f1bab6c149c19b8c57f9f014d7aecd5d287fae0))
	ROM_LOAD16_BYTE("5652_005b.bin", 0x0801, 0x0400, CRC(e1d559ed) SHA1(3093d28b661275c00de8145f8424f584a4854072))
	ROM_LOAD16_BYTE("5652_003b.bin", 0x1000, 0x0400, CRC(24abf1f8) SHA1(ef22ca58e59d8301aab9175ef7ac9dc97feae9ec))
	ROM_LOAD16_BYTE("5652_006b.bin", 0x1001, 0x0400, CRC(db1dca74) SHA1(05149e85237a742850446c01249c83ba373e66b3))

	ROM_REGION(0x100, "ecprom", 0) // 256 bytes to be packed into top 128 words of RAM
	ROM_LOAD("5930_001a.bin", 0x000, 0x100, NO_DUMP) // MM5203Q (256x8 organization)
ROM_END

} // anonymous namespace


COMP(19??, ns5652, 0, 0, ns5652, ns5652, ns5652_state, empty_init, "National Semiconductor", "unknown INS8900 Multibus card (980305652)", MACHINE_IS_SKELETON)
