// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Turbo Drive by Innovative Creations in Entertainment (ICE)

http://www.arcade-museum.com/game_detail.php?game_id=10658


Slot car type race game, coin operated.

Device is a 27c128

--
not sure what the actual inputs / outputs would be on this, maybe just track position / lap sensors?


*/

#include "emu.h"
#include "cpu/z80/z80.h"

class ice_tbd_state : public driver_device
{
public:
	ice_tbd_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		,m_maincpu(*this, "maincpu")
		{ }

	virtual void machine_start();
	virtual void machine_reset();
	required_device<cpu_device> m_maincpu;
};



static ADDRESS_MAP_START( ice_tbd_map, AS_PROGRAM, 8, ice_tbd_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
ADDRESS_MAP_END


static INPUT_PORTS_START( ice_tbd )
INPUT_PORTS_END

void ice_tbd_state::machine_start()
{
}

void ice_tbd_state::machine_reset()
{
}

static MACHINE_CONFIG_START( ice_tbd, ice_tbd_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,8000000)         /* ? MHz */
	MCFG_CPU_PROGRAM_MAP(ice_tbd_map)

MACHINE_CONFIG_END


ROM_START( ice_tbd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "TURBO-DR.IVE", 0x0000, 0x4000, CRC(d7c79ac4) SHA1(a01d93411e604e36a3ced58063f2ab81e431b82a)  )
ROM_END


GAME( 1988, ice_tbd,  0,    ice_tbd, ice_tbd, driver_device,  0, ROT0, "Innovative Creations in Entertainment", "Turbo Drive (ICE)", MACHINE_IS_SKELETON_MECHANICAL )
