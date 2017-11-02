// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

2017-10-29 Skeleton

Motorola Powerstack II. CPU is a PowerPC 604e @ 300MHz.

************************************************************************************************************************************/

#include "emu.h"
//#include "cpu/mcs51/mcs51.h"

class powerstack_state : public driver_device
{
public:
	powerstack_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
//		, m_maincpu(*this, "maincpu")
	{ }

protected:
//	required_device<i80c52_device> m_maincpu;
};

static INPUT_PORTS_START( powerstack )
INPUT_PORTS_END

//static ADDRESS_MAP_START( prg_map, AS_PROGRAM, 8, powerstack_state )
//ADDRESS_MAP_END

static MACHINE_CONFIG_START( powerstack )
MACHINE_CONFIG_END

ROM_START( powerstk )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "motorola_powerstack2.bin", 0x0000, 0x80000, CRC(948e8fcd) SHA1(9a8c32b621c98bc33ee525f66747c34d39851685) )
ROM_END

COMP( 1996, powerstk, 0, 0, powerstack, powerstack, powerstack_state, 0, "Motorola", "Powerstack II", MACHINE_IS_SKELETON )
