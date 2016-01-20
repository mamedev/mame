// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Bozo Pail toss by ICE  (ice_tbd notes say Innovative Creations in Entertainment - same company?)

Devices are 27c080

U9 is version 2.07


PCB uses a 68HC11A1P for a processor/security......

could be related to (or the same thing as - our name could be incorrect)
http://www.highwaygames.com/arcade-machines/bozo-s-grand-prize-game-6751/


*/

#include "emu.h"

class ice_bozopail : public driver_device
{
public:
	ice_bozopail(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag)
	//  ,m_maincpu(*this, "maincpu")
	{ }

	virtual void machine_start() override;
	virtual void machine_reset() override;

//  required_device<mcs51_cpu_device> m_maincpu;
};

static INPUT_PORTS_START( ice_bozoice_bozo )
INPUT_PORTS_END



void ice_bozopail::machine_start()
{
}

void ice_bozopail::machine_reset()
{
}


static MACHINE_CONFIG_START( ice_bozoice_bozo, ice_bozopail )

	/* basic machine hardware */
//  MCFG_CPU_ADD("maincpu", ??, 8000000) // unknown
//  MCFG_CPU_PROGRAM_MAP(ice_bozoice_bozo_map)
//  MCFG_CPU_IO_MAP(ice_bozoice_bozo_io)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
MACHINE_CONFIG_END



ROM_START( ice_bozo )
	ROM_REGION( 0x200000, "maincpu", 0 ) // mostly sound data, some code, what CPU? x86? vectors at end of u9?
	ROM_LOAD( "ICE-BOZO.U18", 0x000000, 0x100000, CRC(00500a8b) SHA1(50b8a784ae61510a08cafbfb8529ec2a8ac1bf06) )
	ROM_LOAD( "ICE-BOZO.U9",  0x100000, 0x100000, CRC(26fd9d60) SHA1(41fe8d42db1eb16b413bd5a0f16bf0d081c3cc97) )
ROM_END

GAME( 1997?, ice_bozo,  0,    ice_bozoice_bozo, ice_bozoice_bozo, driver_device,  0, ROT0, "Innovative Creations in Entertainment", "Bozo's Pail Toss (v2.07)", MACHINE_IS_SKELETON_MECHANICAL )
