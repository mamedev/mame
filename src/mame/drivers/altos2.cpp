// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

2017-11-03 Skeleton

Altos II terminal. Green screen.

Chips: Z80A, 2x Z80DART, Z80CTC, X2210D, 2x CRT9006, CRT9007, CRT9021A, 8x 6116

Other: Beeper.  Crystals: 4.9152, 8.000, 40.000

Keyboard: P8035L CPU, undumped 2716 labelled "358_2758"

************************************************************************************************************************************/

#include "emu.h"
//#include "cpu/mcs51/mcs51.h"

class altos2_state : public driver_device
{
public:
	altos2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
//		, m_maincpu(*this, "maincpu")
	{ }

protected:
//	required_device<i80c52_device> m_maincpu;
};

static INPUT_PORTS_START( altos2 )
INPUT_PORTS_END

//static ADDRESS_MAP_START( prg_map, AS_PROGRAM, 8, altos2_state )
//ADDRESS_MAP_END

static MACHINE_CONFIG_START( altos2 )
MACHINE_CONFIG_END

ROM_START( altos2 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "us_v1.1_14410.u34", 0x000000, 0x002000, CRC(0ebb78bf) SHA1(96a1f7d34ff35037cbbc93049c0e2b9c9f11f1db) )
	ROM_LOAD( "us_v1.2_15732.u32", 0x000000, 0x002000, CRC(a85f7be0) SHA1(3cfa954c916258d86f7f745d10ec2ff5e33261b3) )
	ROM_LOAD( "us_v1.2_15733.u19", 0x000000, 0x002000, CRC(45ebe88a) SHA1(33f16b382a2b365122ebf5e5f7312f8afa45ad15) )
ROM_END

COMP( 198?, altos2, 0, 0, altos2, altos2, altos2_state, 0, "Altos", "Altos II Terminal", MACHINE_IS_SKELETON )
