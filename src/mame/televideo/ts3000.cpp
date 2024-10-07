// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

2017-10-28 Skeleton

Televideo TS-3000. CPU is 8088. Other chips are: 8259, 8253, 8237, 8255, NS8250, uPD765AC, MM58167AN.
Crystals are: 18.432, 16.000, 24.000, 14.31818, 4.7727266. There's a barrel-type backup battery, and a bank of 8 dipswitches.
There are 25-pin serial and parallel ports, and a FDC connector. There's an undumped prom labelled "U20 V1.0" at position U20.

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"


namespace {

class ts3000_state : public driver_device
{
public:
	ts3000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
//      , m_maincpu(*this, "maincpu")
	{ }

	void ts3000(machine_config &config);

private:
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

//  required_device<cpu_device> m_maincpu;
};

void ts3000_state::mem_map(address_map &map)
{
	map(0x00000, 0x0ffff).ram();
	map(0xfc000, 0xfffff).rom().region("roms", 0);
}

void ts3000_state::io_map(address_map &map)
{
}

static INPUT_PORTS_START( ts3000 )
INPUT_PORTS_END

void ts3000_state::ts3000(machine_config &config)
{
	i8088_cpu_device &maincpu(I8088(config, "maincpu", XTAL(14'318'181)/3));  // no idea of clock
	maincpu.set_addrmap(AS_PROGRAM, &ts3000_state::mem_map);
	maincpu.set_addrmap(AS_IO, &ts3000_state::io_map);
}

ROM_START( ts3000 )
	ROM_REGION( 0x4000, "roms", 0 )
	ROM_LOAD( "u25 ver 2.03 bios d.u25", 0x0000, 0x4000, CRC(abaff64c) SHA1(b2f0e73d2a25a03d5bac558580919bd0400f4fcf) ) // The D at the end is handwritten
ROM_END

} // anonymous namespace


COMP( 198?, ts3000, 0, 0, ts3000, ts3000, ts3000_state, empty_init, "Televideo", "TS-3000", MACHINE_IS_SKELETON )
