// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

2017-11-02 Skeleton

Hazeltine Esprit terminals.

2 zipfiles were found: "Hazeltine_Esprit" and "Hazeltine_EaspritIII".


************************************************************************************************************************************/

#include "emu.h"
//#include "cpu/mcs51/mcs51.h"

class hazeltine_state : public driver_device
{
public:
	hazeltine_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
//		, m_maincpu(*this, "maincpu")
	{ }

protected:
//	required_device<i80c52_device> m_maincpu;
};

static INPUT_PORTS_START( hazeltine )
INPUT_PORTS_END

//static ADDRESS_MAP_START( prg_map, AS_PROGRAM, 8, hazeltine_state )
//ADDRESS_MAP_END

static MACHINE_CONFIG_START( hazeltine )
MACHINE_CONFIG_END

ROM_START( hazeltine )
	ROM_REGION( 0x10000, "maincpu", 0 )
	// Esprit
	ROM_LOAD( "hazeltine_esprit.u26",    0x0000, 0x0804, CRC(93f45f13) SHA1(1f493b44124c348759469e24fdfa8b7c52fe6fac) )
	ROM_LOAD( "hazeltine_esprit.u19",    0x0000, 0x1000, CRC(6fdec792) SHA1(a1d1d68c8793e7e15ab5cd17682c299dff3985cb) )
	// Esprit III
	ROM_LOAD( "hazeltine_espritiii.u5",  0x0000, 0x2000, CRC(fd63dad1) SHA1(b2a3e7db8480b28cab2b2834ad89fb6257f13cba) )
	ROM_LOAD( "hazeltine_espritiii.u19", 0x0000, 0x1000, CRC(33e4a8ef) SHA1(e19c84a3c5f94812928ea84bab3ede7970dd5e72) )
ROM_END

COMP( 1981, hazeltine, 0, 0, hazeltine, hazeltine, hazeltine_state, 0, "Hazeltine", "Esprit", MACHINE_IS_SKELETON )
