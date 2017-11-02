// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

2017-10-28 Skeleton

Televideo TS-3000. CPU is 8088. Other chips are: 8259, 8253, 8237, 8255, NS8250, uPD765AC, MM58167AN.
Crystals are: 18.432, 16.000, 24.000, 14.31818, 4.7727266. There's a barrel-type backup battery, and a bank of 8 dispswitches.
There are 25-pin serial and parallel ports, and a FDC connector. There's an undumped prom labelled "U20 V1.0" at position U20.

************************************************************************************************************************************/

#include "emu.h"
//#include "cpu/mcs51/mcs51.h"

class ts3000_state : public driver_device
{
public:
	ts3000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
//		, m_maincpu(*this, "maincpu")
	{ }

protected:
//	required_device<i80c52_device> m_maincpu;
};

static INPUT_PORTS_START( ts3000 )
INPUT_PORTS_END

//static ADDRESS_MAP_START( prg_map, AS_PROGRAM, 8, ts3000_state )
//ADDRESS_MAP_END

static MACHINE_CONFIG_START( ts3000 )
MACHINE_CONFIG_END

ROM_START( ts3000 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "U25 VER 2.03 BIOS D.u25", 0x0000, 0x4000, CRC(abaff64c) SHA1(b2f0e73d2a25a03d5bac558580919bd0400f4fcf) ) // The D at the end is handwritten
ROM_END

COMP( 198?, ts3000, 0, 0, ts3000, ts3000, ts3000_state, 0, "Televideo", "TS-3000", MACHINE_IS_SKELETON )
